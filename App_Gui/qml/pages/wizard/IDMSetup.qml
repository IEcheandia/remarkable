import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.plotter 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0
import Precitec.AppGui 1.0
import precitec.gui.components.image 1.0 as PrecitecImage

Item {
    id: idmSetupPage

    enabled: idmController.ready && !idmController.updating

    Connections {
        target: HardwareModule.recorder
        enabled: visible
        function onSampleData(sensorId, data) {
            sampleItem.setSampleData(sensorId, data)
        }
        function onImageDataChanged() {
            HardwareModule.recorder.requestNextImage()
        }
    }

    Connections {
        target: HardwareModule.systemStatus
        function onReturnedFromCalibration() {
            idmController.endCalibration();
        }
    }

    function pageBecameVisible()
    {
        liveModeEnableTimer.restart();
        idmController.idmDeviceProxy = HardwareModule.idmDeviceProxy;
        scanLabController.weldheadDeviceProxy = HardwareModule.weldHeadDeviceProxy;
    }

    function micrometerTextFromValue(value, locale) {
        return (qsTr("%1 μm")).arg(value);
    }

    function micrometerValueFromText(text, locale) {
        if (text.endsWith(" μm"))
        {
            text = text.slice(0, -3);
        }
        if (text.endsWith("μm"))
        {
            text = text.slice(0, -2);
        }
        return Number.fromLocaleString(locale, text);
    }

    onVisibleChanged: {
        if (visible)
        {
            idmSetupPage.pageBecameVisible();
        }
        else
        {
            liveModeEnableTimer.stop();
            idmController.liveMode = false;
            idmController.idmDeviceProxy = null;
            scanLabController.weldheadDeviceProxy = null;
        }
    }

    Component.onCompleted: {
        if (visible)
        {
            idmSetupPage.pageBecameVisible();
        }
    }

    IdmController {
        id: idmController

        deviceNotificationServer: HardwareModule.deviceNotificationServer
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel

        onCalibratingChanged: {
            if (!calibrating)
            {
                liveModeEnableTimer.restart();
                HardwareModule.recorder.requestNextImage();
            }
        }
    }

    ScanLabController {
        id: scanLabController

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel
        grabberDeviceProxy: SystemConfiguration.CameraInterfaceType == 1 ? HardwareModule.grabberDeviceProxy : null
    }

    SampleItem {
        id: sampleItem
        spectrumColor: PrecitecApplication.Settings.alternateBackground
        spectrumScale: idmController.scale
        onSpectrumRendered: {
            if (idmSetupPage.visible)
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
            idmController.liveMode = true;
        }
    }

    RowLayout {
        anchors {
            fill: parent
            margins: spacing
        }
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            PlotterChart {
                Layout.fillWidth: true
                Layout.minimumHeight: idmSetupPage.height * 0.33
                id: plot
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

                Item {
                    parent: plot.plotter
                    anchors.fill: parent
                    Rectangle {
                        anchors {
                            top: parent.top
                            topMargin: 5
                            left: parent.left
                            right: parent.right
                        }
                        color: "black"
                        height: 8
                    }

                    Label {
                        //: Label on top of IDM spectrum line to show the peak position
                        text: qsTr("Peak at %1 µm").arg(sampleItem.weldingDepth)
                        anchors {
                            right: parent.right
                            top: parent.top
                            topMargin: 20
                            rightMargin: 20
                        }
                    }
                    DetectionCurtain {
                        id: leftCurtain
                        alignment: Qt.AlignLeft
                        limit: idmController.leftLimit
                    }
                    DetectionCurtain {
                        id: rightCurtain
                        alignment: Qt.AlignRight
                        limit: idmController.rightLimit
                    }
                    MouseArea {
                        acceptedButtons: Qt.LeftButton
                        anchors.fill: parent
                        pressAndHoldInterval: 2
                        onPositionChanged: {
                            if ((mouse.x <= parent.width) && (mouse.x > 0))
                            {
                                if (leftCurtain.moving && mouse.x < rightCurtain.anchors.leftMargin)
                                {
                                    idmController.leftLimit = (mouse.x / parent.width);
                                    detectionWindowLeftSpinBox.value = (mouse.x / parent.width) * idmController.scale;
                                }
                                else if (rightCurtain.moving && mouse.x > leftCurtain.anchors.leftMargin)
                                {
                                    idmController.rightLimit = (mouse.x / parent.width);
                                    detectionWindowRightSpinBox.value = (mouse.x / parent.width) * idmController.scale;
                                }
                            }
                        }
                        onPressAndHold: {
                            if (!(leftCurtain.moving || rightCurtain.moving))
                            {
                                if (mouse.x <= (rightCurtain.x + leftCurtain.x) / 2)
                                {
                                    leftCurtain.moving = true;
                                    idmController.leftLimit = (mouse.x / parent.width);
                                    detectionWindowLeftSpinBox.value = (mouse.x / parent.width) * idmController.scale;
                                }
                                else
                                {
                                    rightCurtain.moving = true;
                                    idmController.rightLimit = (mouse.x / parent.width);
                                    detectionWindowRightSpinBox.value = (mouse.x / parent.width) * idmController.scale;
                                }
                            }
                        }
                        onReleased: {
                            if (rightCurtain.moving)
                            {
                                idmController.updateDetectionWindowRight(detectionWindowRightSpinBox.value);
                                rightCurtain.moving = false;
                            }
                            else
                            {
                                idmController.updateDetectionWindowLeft(detectionWindowLeftSpinBox.value);
                                leftCurtain.moving = false;
                            }
                        }
                    }
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: idmController.calibrating
                }

                Component.onCompleted: {
                    plot.plotter.addDataSet(sampleItem.idmSpectrum);
                }
            }
            ImageWithAxisControl {
                scanLabController: scanLabController
                visible: SystemConfiguration.HardwareCameraEnabled
                clip: true

                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }
        PrecitecApplication.ActiveFocusControlAwareFlickable {
            Layout.fillHeight: true
            ColumnLayout {
                GroupBox {
                    title: qsTr("Sample Frequency (SHZ)")
                    RowLayout {
                        SpinBox {
                            id: sampleFrequencySpinBox
                            from: 50
                            to: 70000
                            stepSize: 1
                            value: idmController.sampleFrequency
                            editable: true
                            onValueModified: {
                                idmController.updateSampleFrequency(value);
                            }
                            textFromValue: function(value, locale) {
                                return (qsTr("%1 Hz")).arg(value);
                            }
                            valueFromText: function(text, locale) {
                                if (text.endsWith(" Hz"))
                                {
                                    text = text.slice(0, -3);
                                }
                                if (text.endsWith("Hz"))
                                {
                                    text = text.slice(0, -2);
                                }
                                return Number.fromLocaleString(locale, text);
                            }
                            Component.onCompleted: {
                                contentItem.selectByMouse = true;
                            }
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Lamp Intensity (LAI)")
                    RowLayout {
                        SpinBox {
                            id: lightIntensitySpinBox
                            from: 0
                            to: 100
                            stepSize: 1
                            value: idmController.lampIntensity
                            editable: true
                            onValueModified: {
                                idmController.updateLampIntensity(value);
                            }
                            textFromValue: function(value, locale) {
                                return (qsTr("%1 %")).arg(value);
                            }
                            valueFromText: function(text, locale) {
                                if (text.endsWith(" %"))
                                {
                                    text = text.slice(0, -2);
                                }
                                if (text.endsWith("%"))
                                {
                                    text = text.slice(0, -1);
                                }
                                return Number.fromLocaleString(locale, text);
                            }
                            Component.onCompleted: {
                                contentItem.selectByMouse = true;
                            }
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Detection Window (DWD)")
                    GridLayout {
                        columns: 2
                        Label {
                            text: qsTr("Left:")
                        }
                        SpinBox {
                            id: detectionWindowLeftSpinBox
                            from: 1
                            to: detectionWindowRightSpinBox.value
                            stepSize: 1
                            value: idmController.detectionWindowLeft
                            editable: true
                            onValueModified: {
                                idmController.updateDetectionWindowLeft(value);
                            }
                            textFromValue: micrometerTextFromValue
                            valueFromText: micrometerValueFromText
                            Component.onCompleted: {
                                contentItem.selectByMouse = true;
                            }
                            Layout.preferredWidth: Math.max(detectionWindowLeftSpinBox.implicitWidth, detectionWindowRightSpinBox.implicitWidth)
                        }
                        Label {
                            text: qsTr("Right:")
                        }
                        SpinBox {
                            id: detectionWindowRightSpinBox
                            from: detectionWindowLeftSpinBox.value
                            to: idmController.scale
                            stepSize: 1
                            value: idmController.detectionWindowRight
                            editable: true
                            onValueModified: {
                                idmController.updateDetectionWindowRight(value);
                            }
                            textFromValue: micrometerTextFromValue
                            valueFromText: micrometerValueFromText
                            Component.onCompleted: {
                                contentItem.selectByMouse = true;
                            }
                            Layout.preferredWidth: Math.max(detectionWindowLeftSpinBox.implicitWidth, detectionWindowRightSpinBox.implicitWidth)
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Quality Threshold (QTH)")
                    RowLayout {
                        SpinBox {
                            id: qualityThresholdSpinBox
                            from: 0
                            to: 100
                            stepSize: 1
                            value: idmController.qualityThreshold
                            editable: true
                            onValueModified: {
                                idmController.updateQualityThreshold(value);
                            }
                            Component.onCompleted: {
                                contentItem.selectByMouse = true;
                            }
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Data Averaging (AVD)")
                    RowLayout {
                        SpinBox {
                            id: dataAveragingSpinBox
                            from: 1
                            to: 9999
                            stepSize: 1
                            value: idmController.dataAveraging
                            editable: true
                            onValueModified: {
                                idmController.updateDataAveraging(value);
                            }
                            Component.onCompleted: {
                                contentItem.selectByMouse = true;
                            }
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Spectral Averaging (AVS)")
                    RowLayout {
                        SpinBox {
                            id: spectralAveragingSpinBox
                            from: 1
                            to: 256
                            stepSize: 1
                            value: idmController.spectralAveraging
                            editable: true
                            onValueModified: {
                                idmController.updateSpectralAveraging(value);
                            }
                            Component.onCompleted: {
                                contentItem.selectByMouse = true;
                            }
                        }
                    }
                }
                GroupBox {
                    title: qsTr("WeldingDepth System Offset")
                    RowLayout {
                        SpinBox {
                            from: 0
                            to: 10000
                            stepSize: 100
                            value: idmController.depthSystemOffset
                            editable: true
                            onValueModified: idmController.updateDepthSystemOffset(value)
                            textFromValue: micrometerTextFromValue
                            valueFromText: micrometerValueFromText
                            Component.onCompleted: {
                                contentItem.selectByMouse = true;
                            }
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Dark Reference")
                    Button {
                        anchors.fill: parent
                        text: qsTr("Start")
                        enabled: idmController.ready && !idmController.updating && !idmController.calibrating
                        onClicked: {
                            liveModeEnableTimer.stop();
                            idmController.liveMode = false;
                            idmController.performDarkReference();
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Fiber Switch Position")
                    visible: SystemConfiguration.OCT_with_reference_arms
                    ComboBox {
                        model: [1, 2, 3, 4]
                        onActivated: {
                            scanLabController.setFiberSwitchPosition(currentIndex + 1);
                        }
                    }
                }
            }
        }
    }
}

