import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.plotter 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

Item {
    id: hardwareCameraPage
    property alias hardwareParametersModel: controller.model
    property alias title: hardwareParameters.title
    property alias filterKeys: hardwareParametersEditor.filterKeys
    property bool showIdm: false
    property alias screenshotTool: image.screenshotTool

    Connections {
        target: HardwareModule.recorder
        enabled: hardwareCameraPage.visible && showIdm
        function onSampleData(sensorId, data) {
            sampleItem.setSampleData(sensorId, data)
        }
        function onImageDataChanged() {
            HardwareModule.recorder.requestNextImage()
        }
    }

    function handleLeave()
    {
        liveModeEnableTimer.stop();
        controller.liveMode = false;
        idmController.idmDeviceProxy = null;
    }

    function handleVisibleChanged() {
        if (visible)
        {
            if (hardwareCameraPage.showIdm)
            {
                idmController.idmDeviceProxy = HardwareModule.idmDeviceProxy;
            }
            liveModeEnableTimer.restart();
        } else
        {
            handleLeave();
        }
    }

    onVisibleChanged: handleVisibleChanged()
    Component.onCompleted: handleVisibleChanged()
    Component.onDestruction: hardwareCameraPage.handleLeave()

    AxisController {
        id: axisController
        axisInformation: yAxisInformation
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel
    }

    ScanLabController {
        id: scanLabController

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel
        grabberDeviceProxy: SystemConfiguration.CameraInterfaceType == 1 ? HardwareModule.grabberDeviceProxy : null
        weldheadDeviceProxy: HardwareModule.weldHeadDeviceProxy
    }

    IdmController {
        id: idmController
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel
        deviceNotificationServer: HardwareModule.deviceNotificationServer
    }

    HardwareParameterController {
        id: controller
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel
        filterModel: hardwareParametersEditor.filterModel
        grabberDeviceProxy: SystemConfiguration.CameraInterfaceType == 1 ? HardwareModule.grabberDeviceProxy : null
    }

    SampleItem {
        id: sampleItem
        spectrumColor: PrecitecApplication.Settings.alternateBackground
        spectrumScale: idmController.scale
        onSpectrumRendered: {
            if (showIdm)
            {
                HardwareModule.recorder.requestNextImage();
            }
        }
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

    RowLayout {
        anchors.fill: parent
        enabled: !axisController.updating
        opacity: enabled ? 1.0 : 0.5

        ImageWithAxisControl {
            id: image
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: parent.width * 0.25

            visible: !hardwareCameraPage.showIdm
            axisController: axisController
            scanLabController: scanLabController
            updating: controller.updating
        }

        PlotterChart {
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: parent.width * 0.25

            id: plot

            visible: hardwareCameraPage.showIdm

            controller {
                pointSize: 5
                lineSize: 3
                columns: 8
            }
            xAxisController {
                xRange: idmController.scale
            }
            yAxisController {
                autoAdjustYAxis: true
            }
            xUnit: qsTr("μm")
            yUnit: ""
            panningEnabled: false
            zoomEnabled: false
            restoreEnabled: false
            menuAvailable: false
            yLegendLeftVisible: false
            yLegendRightVisible: false

            Label {
                parent: plot.plotter
                //: Label on top of IDM spectrum line to show the peak position
                text: qsTr("Peak at %1 µm").arg(sampleItem.weldingDepth)
                anchors {
                    right: parent.right
                    top: parent.top
                    topMargin: 20
                    rightMargin: 20
                }
            }

            Component.onCompleted: {
                plot.plotter.addDataSet(sampleItem.idmSpectrum);
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop
            GroupBox {
                id: hardwareParameters
                Layout.fillHeight: true
                HardwareParametersEditor {
                    id: hardwareParametersEditor
                    anchors.fill: parent
                    model: controller.model
                    enabled: HardwareModule.systemStatus.state == SystemStatusServer.Live
                }
            }
        }
    }
}

