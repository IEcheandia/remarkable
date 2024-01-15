import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.general 1.0

Item {
    property alias attributeModel: scanLabController.attributeModel
    property alias screenshotTool: image.screenshotTool
    property alias figureSimulationVisible: figureSimulation.visible

    id: root

    function pageBecameVisible()
    {
        liveModeEnableTimer.restart();
        scanLabController.weldheadDeviceProxy = HardwareModule.weldHeadDeviceProxy;
    }

    onVisibleChanged: {
        if (visible)
        {
            root.pageBecameVisible();
            xPositionField.text = "";
            yPositionField.text = "";
        } else
        {
            liveModeEnableTimer.stop();
            scanLabController.liveMode = false;
            scanLabController.weldheadDeviceProxy = null;
        }
    }

    Component.onCompleted: {
        if (visible)
        {
            root.pageBecameVisible();
        }
    }

    Timer {
        id: liveModeEnableTimer
        interval: 500
        repeat: false
        onTriggered: {
            scanLabController.liveMode = true;
        }
    }

    ScanLabController {
        id: scanLabController

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel
        grabberDeviceProxy: SystemConfiguration.CameraInterfaceType == 1 ? HardwareModule.grabberDeviceProxy : null
        deviceNotificationServer: HardwareModule.deviceNotificationServer

    }

    RowLayout {
        anchors.fill: parent

        ImageWithAxisControl {
            Layout.fillHeight: true
            Layout.fillWidth: true

            id: image
            scanLabController: scanLabController
            updating: scanLabController.updating
            handlersEnabled: !figureSimulation.dialogVisible

            clip: true
        }

        ColumnLayout {
            Layout.maximumWidth: 0.2 * root.width
            Layout.fillWidth: true
            Layout.fillHeight: true

            PrecitecApplication.ActiveFocusControlAwareFlickable {
                Layout.fillHeight: true
                Layout.fillWidth: true
                flickableDirection: Flickable.VerticalFlick

                ColumnLayout {
                    anchors.fill: parent
                    enabled: scanLabController.ready && !scanLabController.updating

                    GroupBox {
                        Layout.fillWidth: true
                        id: scanLabControllerGroup
                        title: qsTr("Change scanner position")

                        ColumnLayout {
                            spacing: 50
                            PrecitecApplication.ArrowButtonGrid {
                                Layout.alignment: Qt.AlignHCenter

                                leftEnabled: scanLabController.canDecrementX
                                rightEnabled: scanLabController.canIncrementX
                                upEnabled: scanLabController.canIncrementY
                                downEnabled: scanLabController.canDecrementY
                                enabled: scanLabController.ready && !scanLabController.updating

                                onMoveLeft: scanLabController.decrementXPosition()
                                onMoveRight: scanLabController.incrementXPosition()
                                onMoveUp: scanLabController.incrementYPosition()
                                onMoveDown: scanLabController.decrementYPosition()
                            }
                            GridLayout {
                                Layout.alignment: Qt.AlignHCenter

                                columns: 3

                                Label {
                                    text: "X:"
                                }
                                TextField {
                                    id: xPositionField

                                    placeholderText: qsTr("Position to jump to.");
                                    selectByMouse: true
                                    enabled: scanLabController.ready && !scanLabController.updating
                                    validator: DoubleValidator {
                                        bottom: scanLabController.xMinLimit
                                        top: scanLabController.xMaxLimit
                                    }
                                    onEditingFinished: {
                                        scanLabController.scannerXPosition = Number.fromLocaleString(locale, text)
                                    }
                                    palette.text: xPositionField.acceptableInput || xPositionField.text == "" ? "black" : "red"
                                }
                                Label {
                                    text: "mm"
                                }
                                Label {
                                    text: "Y:"
                                }
                                TextField {
                                    id: yPositionField

                                    placeholderText: qsTr("Position to jump to.");
                                    selectByMouse: true
                                    enabled: scanLabController.ready && !scanLabController.updating
                                    validator: DoubleValidator {
                                        bottom: scanLabController.yMinLimit
                                        top: scanLabController.yMaxLimit
                                    }
                                    onEditingFinished: {
                                        scanLabController.scannerYPosition = Number.fromLocaleString(locale, text)
                                    }
                                    palette.text: yPositionField.acceptableInput || xPositionField.text == ""  ? "black" : "red"
                                }
                                Label {
                                    text: "mm"
                                }
                            }

                            GroupBox {
                                Layout.fillWidth: true
                                title: qsTr("Actual Scanner Position:")
                                ColumnLayout {
                                    Label {
                                        text: qsTr("X: %2 mm".arg(scanLabController.scannerXPosition))
                                    }
                                    Label {
                                        text: qsTr("Y: %2 mm".arg(scanLabController.scannerYPosition))
                                    }
                                }
                            }
                            Button {
                                Layout.alignment: Qt.AlignHCenter
                                text: qsTr("Jump to center")
                                onClicked: scanLabController.resetToZero();
                            }

                        }
                    }
                    FigureSimulationPilotLaser {
                        id: figureSimulation
                        Layout.fillWidth: true
                    }
                    Item {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }
        }
    }
}
