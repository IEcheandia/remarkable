import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtQml 2.8
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.userManagement 1.0
import precitec.gui 1.0

import Precitec.AppGui 1.0

/**
 * Page to set the Tool Center Point.
 **/
Item {
    id: tcpPage

    property alias screenshotTool: image.screenshotTool

    enabled: UserManagement.currentUser && UserManagement.hasPermission(App.SetToolCenterPoint)

    function pageBecameVisible()
    {
        liveModeEnableTimer.restart();
        controller.calibrationDevice = HardwareModule.calibrationDeviceProxy;
        controller.grabberDevice = HardwareModule.grabberDeviceProxy;
    }

    onVisibleChanged: {
        if (visible)
        {
            tcpPage.pageBecameVisible();
        } else
        {
            liveModeEnableTimer.stop();
            controller.liveMode = false;
            controller.calibrationDevice = null;
            controller.grabberDevice = null;
        }
    }

    Component.onCompleted: {
        if (visible)
        {
            tcpPage.pageBecameVisible();
        }
    }

    ToolCenterPointController {
        id: controller

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        productModel: HardwareModule.productModel
        systemStatus: HardwareModule.systemStatus
    }

    Timer {
        id: liveModeEnableTimer
        interval: 500
        repeat: false
        onTriggered: {
            controller.liveMode = true;
        }
    }

    RowLayout {
        anchors{
            fill: parent
            margins: spacing
        }

        PrecitecImage.Image {
            id: image
            clip: true
            mirroringSupported: false
            opacity: controller.updating ? 0.5 : 1.0

            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.8
            Layout.minimumWidth: parent.width * 0.25

            PrecitecApplication.CrossCursor {
                id: originalTcpCursor
                enabled: false
                visible: controller.changes
                opacity: 0.5
                color2: "gray"

                function mapToImage()
                {
                    var mapped = image.mapToPaintedImage(controller.originalTcp);
                    xPosition = mapped.x;
                    yPosition = mapped.y;
                }

                x: image.x + image.paintedRect.x
                y: image.y + image.paintedRect.y
                width: image.paintedRect.width
                height: image.paintedRect.height

                Connections {
                    target: controller
                    function onOriginalTcpChanged() {
                        originalTcpCursor.mapToImage()
                    }
                }
                Component.onCompleted: originalTcpCursor.mapToImage()
                onWidthChanged: originalTcpCursor.mapToImage()
                onHeightChanged: originalTcpCursor.mapToImage()

            }

            PrecitecApplication.CrossCursor {
                id: crossCursor
                property bool updateBlocked: true
                enabled: HardwareModule.systemStatus.state == SystemStatusServer.Live && !controller.updating && image.inputHandlersActive

                function mapToImage()
                {
                    updateBlocked = true;
                    var mapped = image.mapToPaintedImage(controller.tcp);
                    xPosition = mapped.x;
                    yPosition = mapped.y;
                    updateBlocked = false;
                }

                function updateTcp()
                {
                    if (updateBlocked)
                    {
                        return;
                    }
                    controller.tcp = image.mapFromPaintedImage(Qt.point(Math.round(crossCursor.xPosition), Math.round(crossCursor.yPosition)));
                }

                x: image.x + image.paintedRect.x
                y: image.y + image.paintedRect.y
                width: image.paintedRect.width
                height: image.paintedRect.height

                Connections {
                    target: controller
                    function onTcpChanged() {
                        crossCursor.mapToImage()
                    }
                }
                Component.onCompleted: crossCursor.mapToImage()
                onWidthChanged: crossCursor.mapToImage()
                onHeightChanged: crossCursor.mapToImage()

                onXPositionChanged: crossCursor.updateTcp()
                onYPositionChanged: crossCursor.updateTcp()
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            enabled: !controller.updating
            RowLayout {
                Layout.fillWidth: true
                ToolButton {
                    display: AbstractButton.IconOnly
                    enabled: controller.changes
                    icon {
                        name: "edit-undo"
                        width: 64
                        height: 64
                    }
                    onClicked: controller.discardChanges()

                    ToolTip.text: qsTr("Discard changes.")
                    ToolTip.visible: hovered
                    ToolTip.delay: 200
                    ToolTip.timeout: 5000
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    enabled: controller.changes
                    icon {
                        name: "document-save"
                        width: 64
                        height: 64
                    }
                    onClicked: controller.saveChanges()

                    ToolTip.text: qsTr("Save changes.")
                    ToolTip.visible: hovered
                    ToolTip.delay: 200
                    ToolTip.timeout: 5000
                }
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            PrecitecApplication.ArrowButtonGrid {
                leftEnabled: crossCursor.canMoveLeft
                rightEnabled: crossCursor.canMoveRight
                upEnabled: crossCursor.canMoveUp
                downEnabled: crossCursor.canMoveDown
                enabled: crossCursor.enabled

                onMoveLeft: {
                    controller.tcp.x--;
                }
                onMoveDown: {
                    controller.tcp.y++;
                }
                onMoveUp: {
                    controller.tcp.y--;
                }
                onMoveRight: {
                    controller.tcp.x++;
                }

                Layout.alignment: Qt.AlignHCenter
            }
            Label {
                enabled: crossCursor.enabled
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("X [pixel]:   %1").arg(Math.trunc(controller.tcp.x))
            }
            Label {
                enabled: crossCursor.enabled
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Y [pixel]:   %1").arg(Math.trunc(controller.tcp.y))
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }

    }
    BusyIndicator {
        anchors.centerIn: parent
        running: controller.updating
    }
}
