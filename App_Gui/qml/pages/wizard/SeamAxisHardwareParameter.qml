import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Item {
    id: hardwareAxisPage
    property alias currentSeam: hardwareParameterModel.seam
    property alias attributeModel: hardwareParameterModel.attributeModel
    enabled: yAxisInformation.axisEnabled && visible
    property alias screenshotTool: image.screenshotTool

    signal markAsChanged()

    function handleLeave()
    {
        liveModeEnableTimer.stop();
        controller.liveMode = false;
    }

    function handleEnabledChanged() {
        if (enabled)
        {
            liveModeEnableTimer.restart();
        } else
        {
            handleLeave();
        }
    }

    onEnabledChanged: handleEnabledChanged()

    Component.onCompleted: {
        handleEnabledChanged();
    }
    Component.onDestruction: hardwareAxisPage.handleLeave()

    AxisController {
        id: axisController
        axisInformation: yAxisInformation
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel
    }

    HardwareParameterController {
        id: controller
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel
        model: hardwareParameterModel
        filterModel: hardwareParametersEditor.filterModel
        grabberDeviceProxy: SystemConfiguration.CameraInterfaceType == 1 ? HardwareModule.grabberDeviceProxy : null
    }

    Timer {
        id: liveModeEnableTimer
        interval: 500
        repeat: false
        onTriggered: {
            controller.liveMode = true;
        }
    }

    Connections {
        target: HardwareModule.systemStatus
        function onReturnedFromCalibration() {
            axisController.endReferenceRun()
        }
    }

    HardwareParameterSeamModel {
        id: hardwareParameterModel
        onMarkAsChanged: hardwareAxisPage.markAsChanged()
    }

    RowLayout {
        anchors.fill: parent
        enabled: !axisController.calibrating && !axisController.updating
        opacity: enabled ? 1.0 : 0.5

        ImageWithAxisControl {
            id: image
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: parent.width * 0.25
            axisController: axisController
            updating: controller.updating
        }

        GridLayout {
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop
            columns: 2
            GroupBox {
                title: qsTr("Axis configuration")
                Layout.fillHeight: true
                HardwareParametersEditor {
                    id: hardwareParametersEditor
                    model: hardwareParameterModel
                    height: parent.height

                    enabled: HardwareModule.systemStatus.state == SystemStatusServer.Live
                    filterKeys: [
                        HardwareParameterSeamModel.YAxisAbsolutePosition,
                        HardwareParameterSeamModel.YAxisSoftwareLimits,
                        HardwareParameterSeamModel.YAxisUpperLimit,
                        HardwareParameterSeamModel.YAxisLowerLimit,
                        HardwareParameterSeamModel.YAxisVelocity,
                        HardwareParameterSeamModel.YAxisAcceleration
                    ]
                }
            }
            AxisStatus {
                Layout.fillHeight: true
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: axisController.calibrating || liveModeEnableTimer.running
    }
}
