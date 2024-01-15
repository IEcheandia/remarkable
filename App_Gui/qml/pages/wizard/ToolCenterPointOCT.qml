import QtQuick 2.5
import QtQml 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.userManagement 1.0
import precitec.gui 1.0

import Precitec.AppGui 1.0

/**
 * Page to set the Tool Center Point.
 **/
Item {
    id: tcpOctPage

    property bool scanRequested: false
    property alias screenshotTool: image.screenshotTool

    function pageBecameVisible()
    {
        octTCPCalibrationTimer.restart();

        controller.calibrationDevice = HardwareModule.calibrationDeviceProxy;
        controller.grabberDevice = HardwareModule.grabberDeviceProxy;
    }

    enabled: UserManagement.currentUser && UserManagement.hasPermission(App.SetToolCenterPoint)
    Timer {
        id: octTCPCalibrationTimer
        interval: 500
        repeat: false
        onTriggered: {
           if (HardwareModule.systemStatus.state == SystemStatusServer.Normal)
           {
               idmCalibrationController.octTCPCalibration();
           }
        }
    }
    onVisibleChanged: {
        if (visible)
        {
            tcpOctPage.pageBecameVisible();
        } else
        {
            octTCPCalibrationTimer.stop()
            controller.calibrationDevice = null;
            controller.grabberDevice = null;
        }
    }

    Component.onCompleted: {
        if (visible)
        {
            tcpOctPage.pageBecameVisible();
        }
    }

    ToolCenterPointController {
        id: controller
        isOCT: true
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        calibrationCoordinatesRequestProxy: HardwareModule.calibrationCoordinatesRequestProxy
        systemStatus: HardwareModule.systemStatus
    }

    IDMCalibrationController {
        id: idmCalibrationController
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        onCalibratingChanged: {
            if (calibrating)
            {
                scanRequested = true;
            }
        }
    }

    RowLayout {
        anchors.fill: parent

        PrecitecImage.Image {
            id: image
            clip: true
            debugMeasurement: true

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: parent.width * 0.25

            PrecitecApplication.CrossCursor {
                id: crossCursor
                property bool updateBlocked: true
                enabled: HardwareModule.systemStatus.state == SystemStatusServer.Normal
                visible: scanRequested && HardwareModule.systemStatus.state == SystemStatusServer.Normal

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
                    if (updateBlocked || !visible)
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
                onVisibleChanged:  { 
                    if (visible && controller.isOCT)
                    {
                        // refresh calibration device, get tcp coordinates
                        controller.calibrationDevice = null;
                        controller.calibrationDevice = HardwareModule.calibrationDeviceProxy;
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            RowLayout {
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
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


                ToolButton {
                    text: qsTr("Refresh 2D scan")
                    enabled: HardwareModule.systemStatus.state == SystemStatusServer.Normal
                    onClicked: {
                        octTCPCalibrationTimer.stop()
                        idmCalibrationController.octTCPCalibration();
                    }
                }
            }


            Item {
                Layout.fillHeight: true
            }
            GridLayout {
                columns: 2
                enabled: crossCursor.enabled

                Label {
                    text: qsTr("X:")
                }
                SpinBox {
                    id: tcpXSpinbox
                    editable: true
                    value: controller.tcp.x
                    from: 0
                    to: 1024
                    onValueModified: {
                        controller.tcp.x = value;
                    }
                    Binding {
                        target: tcpXSpinbox
                        property: "to"
                        restoreMode: Binding.RestoreBinding
                        value: image.imageSize.width
                        when: image.imageSize.width != 0
                    }
                    Component.onCompleted: {
                        contentItem.selectByMouse = true;
                    }
                }
                Label {
                    text: qsTr("Y:")
                }
                SpinBox {
                    id: tcpYSpinbox
                    editable: true
                    value: controller.tcp.y
                    from: 0
                    to: 1024
                    onValueModified: {
                        controller.tcp.y = value;
                    }
                    Binding {
                        target: tcpYSpinbox
                        property: "to"
                        restoreMode: Binding.RestoreBinding
                        value: image.imageSize.height
                        when: image.imageSize.height != 0
                    }
                    Component.onCompleted: {
                        contentItem.selectByMouse = true;
                    }
                }

                PrecitecApplication.ArrowButtonGrid {
                    leftEnabled: crossCursor.canMoveLeft
                    rightEnabled: crossCursor.canMoveRight
                    upEnabled: crossCursor.canMoveUp
                    downEnabled: crossCursor.canMoveDown

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
                    Layout.columnSpan: 2
                }
            }
            Item {
                Layout.fillHeight: true
            }
        }

    }
}
