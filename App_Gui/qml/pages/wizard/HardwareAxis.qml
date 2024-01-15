import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui 1.0
import precitec.gui.filterparametereditor 1.0

Item {
    id: hardwareAxisPage
    property alias screenshotTool: image.screenshotTool

    enabled: yAxisInformation.axisEnabled && visible
    onEnabledChanged: {
        if (enabled)
        {
            liveModeEnableTimer.restart();
        } else
        {
            liveModeEnableTimer.stop();
            axisController.liveMode = false;
        }
    }
    onVisibleChanged: {
        if (visible)
        {
            lineLaserModel.deviceProxy = HardwareModule.weldHeadDeviceProxy;
            fieldIlluminationModel.deviceProxy = HardwareModule.weldHeadDeviceProxy;
        } else
        {
            lineLaserModel.deviceProxy = null;
            fieldIlluminationModel.deviceProxy = null;
        }
    }
    Component.onCompleted: {
        if (visible)
        {
            lineLaserModel.deviceProxy = HardwareModule.weldHeadDeviceProxy;
            fieldIlluminationModel.deviceProxy = HardwareModule.weldHeadDeviceProxy;
            if (enabled)
            {
                liveModeEnableTimer.restart();
            }
        }
    }

    AxisController {
        id: axisController
        axisInformation: yAxisInformation
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        productModel: HardwareModule.productModel
        systemStatus: HardwareModule.systemStatus
    }

    LineLaserModel {
        id: lineLaserModel
        notificationServer: HardwareModule.deviceNotificationServer
    }

    FieldIlluminationModel {
        id: fieldIlluminationModel
        notificationServer: HardwareModule.deviceNotificationServer
    }

    Timer {
        id: liveModeEnableTimer
        interval: 500
        repeat: false
        onTriggered: {
            axisController.liveMode = true;
        }
    }

    Connections {
        target: HardwareModule.systemStatus
        function onReturnedFromCalibration() {
            axisController.endReferenceRun()
        }
    }

    RowLayout {
        anchors {
            fill: parent
            margins: spacing
        }
        enabled: !axisController.calibrating && !axisController.updating
        opacity: enabled ? 1.0 : 0.5

        ImageWithAxisControl {
            Layout.preferredWidth: 0.8 * parent.width
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: parent.width * 0.25
            id: image
            axisController: axisController
        }

        PrecitecApplication.ActiveFocusControlAwareFlickable {
            Layout.fillHeight: true
            flickableDirection: Flickable.VerticalFlick

            ColumnLayout {
                anchors.fill: parent
                GroupBox {
                    Layout.fillWidth: true
                    id: configurationGroupBox
                    title: qsTr("Axis configuration")
                    ColumnLayout {
                        anchors.fill: parent
                        CheckBox {
                            Layout.fillWidth: true
                            text: qsTr("Homing to positive hardware limit")
                            enabled: referenceRunButton.enabled
                            checked: yAxisInformation.homingDirectionPositive
                            onClicked: axisController.toggleHomingDirectionPositive(!yAxisInformation.homingDirectionPositive)
                        }
                        Button {
                            Layout.fillWidth: true
                            id: referenceRunButton
                            text: qsTr("Reference run")
                            enabled: axisController.canCalibrate && (HardwareModule.systemStatus.state == SystemStatusServer.Live || HardwareModule.systemStatus.state == SystemStatusServer.NotReady)
                            onClicked: {
                                axisController.referenceRun();
                            }
                        }
                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Trigger first hardware limit")
                            visible: UserManagement.currentUser && UserManagement.hasPermission(App.AxisTriggerHardwareLimits)
                            enabled: referenceRunButton.enabled
                            onClicked: axisController.moveAxis(yAxisInformation.homingDirectionPositive ? -60000 : -4000)
                        }
                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Trigger second hardware limit")
                            enabled: referenceRunButton.enabled
                            visible: UserManagement.currentUser && UserManagement.hasPermission(App.AxisTriggerHardwareLimits)
                            onClicked: axisController.moveAxis(yAxisInformation.homingDirectionPositive ? 4000 : 60000)
                        }
                        GroupBox {
                            Layout.fillWidth: true
                            enabled: HardwareModule.systemStatus.state == SystemStatusServer.Live
                            label: CheckBox {
                                id: softwareLimits
                                checked: yAxisInformation.softwareLimitsEnabled
                                text: qsTr("Software limits")
                                onToggled: {
                                    axisController.enableSoftwareLimits(checked);
                                }
                            }
                            GridLayout {
                                anchors.fill: parent
                                enabled: softwareLimits.checked
                                columns: 2
                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("Lower limit")
                                }
                                MilliFromMicroSpinBox {
                                    id: lowerLimit
                                    editable: true
                                    value: yAxisInformation.lowerLimit
                                    from: yAxisInformation.initiallyPolled ? yAxisInformation.minimumPosition : -50000
                                    to: yAxisInformation.initiallyPolled ? yAxisInformation.maximumPosition : 50000
                                    stepSize: 10
                                    onValueModified: {
                                        axisController.setLowerLimit(value);
                                    }
                                    Layout.preferredWidth: Math.max(lowerLimit.implicitWidth, upperLimit.implicitWidth)
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("Upper limit")
                                }
                                MilliFromMicroSpinBox {
                                    id: upperLimit
                                    editable: true
                                    value: yAxisInformation.upperLimit
                                    from: yAxisInformation.initiallyPolled ? yAxisInformation.minimumPosition : -50000
                                    to: yAxisInformation.initiallyPolled ? yAxisInformation.maximumPosition : 50000
                                    stepSize: 10
                                    onValueModified: {
                                        axisController.setUpperLimit(value);
                                    }
                                    Layout.preferredWidth: Math.max(lowerLimit.implicitWidth, upperLimit.implicitWidth)
                                }
                            }
                        }
                    }
                }
                AxisStatus {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
                LineLaserControl {
                    lineLaserModel: lineLaserModel
                }
                LedIlluminationControl {
                    fieldIlluminationModel: fieldIlluminationModel
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: axisController.calibrating || liveModeEnableTimer.running
    }
}
