import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.general 1.0
import precitec.gui.filterparametereditor 1.0

/**
 * Control to configure a Seam.
 **/
BreadCrumpGroupBox {
    id: seamConfiguration
    property var controller: null
    property var keyValueAttributeModel: null
    /**
     * The Seam which should be configured
     **/
    property alias seam: scanfieldController.seam
    /**
     * The SeamInterval which should be configured
     **/
    property var seamInterval: null
    /**
     * Length unit description
     **/
    property string unit

    /**
     * The name of the Seam
     **/
    property alias name: nameField.text
    /**
     * The length of the Seam in µm
     **/
    property int length: Number.fromLocaleString(locale, lengthField.text) * 1000

    property alias graphModel: filterParameterEditor.graphModel
    property alias subGraphModel: filterParameterEditor.subGraphModel
    property alias attributeModel: filterParameterEditor.attributeModel
    property alias resultsConfigModel: filterParameterEditor.resultsConfigModel
    property alias errorConfigModel: filterParameterEditor.errorConfigModel
    property alias sensorConfigModel: filterParameterEditor.sensorConfigModel
    property var qualityNorm: null
    property alias onlineHelp: filterParameterEditor.onlineHelp

    property var wizardModel: null

    property int sideTabIndex

    property var screenshotTool: null

    property Component seamAssemblyImage: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Assembly image")
            lastLevelModel: seamConfiguration.wizardModel
            SeamAssemblyImage {
                id: assemblyPage
                width: parent.width
                height: parent.height
                currentSeam: seamConfiguration.seam
                onMarkAsChanged: seamConfiguration.markAsChanged()
                editable: true
            }
        }
    }
    property Component seamCameraImage: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Sensor")
            lastLevelModel: seamConfiguration.wizardModel
            SeamHardwareParameters {
                id: seamCameraPage
                width: parent.width
                height: parent.height
                title: qsTr("Camera parameter")
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                screenshotTool: seamConfiguration.screenshotTool

                property var basicParameters:[
                    HardwareParameters.LEDFlashDelay,
                    HardwareParameters.LEDPanel1OnOff,
                    HardwareParameters.LEDPanel1Intensity,
                    HardwareParameters.LEDPanel1PulseWidth,
                    HardwareParameters.LEDPanel2OnOff,
                    HardwareParameters.LEDPanel2Intensity,
                    HardwareParameters.LEDPanel2PulseWidth,
                    HardwareParameters.LEDPanel3OnOff,
                    HardwareParameters.LEDPanel3Intensity,
                    HardwareParameters.LEDPanel3PulseWidth,
                    HardwareParameters.LEDPanel4OnOff,
                    HardwareParameters.LEDPanel4Intensity,
                    HardwareParameters.LEDPanel4PulseWidth,
                    HardwareParameters.LEDPanel5OnOff,
                    HardwareParameters.LEDPanel5Intensity,
                    HardwareParameters.LEDPanel5PulseWidth,
                    HardwareParameters.LEDPanel6OnOff,
                    HardwareParameters.LEDPanel6Intensity,
                    HardwareParameters.LEDPanel6PulseWidth,
                    HardwareParameters.LEDPanel7OnOff,
                    HardwareParameters.LEDPanel7Intensity,
                    HardwareParameters.LEDPanel7PulseWidth,
                    HardwareParameters.LEDPanel8OnOff,
                    HardwareParameters.LEDPanel8Intensity,
                    HardwareParameters.LEDPanel8PulseWidth,
                    HardwareParameters.LiquidLensPosition
                ]

                property var cameraParameters:[
                    HardwareParameters.CameraAcquisitionMode,
                    HardwareParameters.CameraReuseLastImage,
                    HardwareParameters.ExposureTime,
                    HardwareParameters.LineLaser1OnOff,
                    HardwareParameters.LineLaser1Intensity,
                    HardwareParameters.LineLaser2OnOff,
                    HardwareParameters.LineLaser2Intensity,
                    HardwareParameters.FieldLight1OnOff,
                    HardwareParameters.FieldLight1Intensity
                ]

                filterKeys: (SystemConfiguration.SOUVIS6000_Application || SystemConfiguration.CameraInterfaceType == 1) ? basicParameters.concat(cameraParameters) : basicParameters
                onMarkAsChanged: seamConfiguration.markAsChanged()
            }
        }
    }

    property Component seamAxis: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Axis")
            lastLevelModel: seamConfiguration.wizardModel
            SeamAxisHardwareParameter {
                id: seamAxisHardwareParameter
                width: parent.width
                height: parent.height
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                onMarkAsChanged: seamConfiguration.markAsChanged()
                screenshotTool: seamConfiguration.screenshotTool
            }
        }
    }


    property Component seamError: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Errors")
            lastLevelModel: seamConfiguration.wizardModel
            SeamErrorList {
                id: errorPage
                width: parent.width
                height: parent.height
                currentProduct: seamConfiguration.seam ? seamConfiguration.seam.seamSeries.product : null
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.attributeModel
                resultsConfigModel: seamConfiguration.resultsConfigModel
                errorConfigModel: seamConfiguration.errorConfigModel
                screenshotTool: seamConfiguration.screenshotTool
                graphModel: seamConfiguration.graphModel
                subGraphModel: seamConfiguration.subGraphModel
                onMarkAsChanged: seamConfiguration.markAsChanged()
                onPlotterSettingsUpdated: seamConfiguration.plotterSettingsUpdated()

                Connections {
                    target: seamConfiguration
                    function onUpdateSettings() {
                        errorPage.updateSettings()
                    }
                }
            }
        }
    }

    property Component referenceCurves: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            id: referenceCurveBox
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Reference Curves")
            lastLevelModel: seamConfiguration.wizardModel
            ReferenceCurveList {
                id: refCurves
                width: parent.width
                height: parent.height
                currentSeam: seamConfiguration.seam
                currentSeamSeries: seamConfiguration.seam ? seamConfiguration.seam.seamSeries : null
                currentProduct: seamConfiguration.seam ? seamConfiguration.seam.seamSeries.product : null
                onMarkAsChanged: seamConfiguration.markAsChanged()
                productController: seamConfiguration.controller
                resultsConfigModel: seamConfiguration.resultsConfigModel
                screenshotTool: seamConfiguration.screenshotTool
                graphModel: seamConfiguration.graphModel
                subGraphModel: seamConfiguration.subGraphModel
                onPlotterSettingsUpdated: seamConfiguration.plotterSettingsUpdated()
                onReferenceCurveSelected: seamConfiguration.referenceCurveSelected(component)
                onReferenceCurveEditorSelected: seamConfiguration.referenceCurveEditorSelected(component)
                onBack: referenceCurveBox.back()
                Connections {
                    target: seamConfiguration
                    function onUpdateSettings() {
                        refCurves.updateSettings()
                    }
                }
            }
        }
    }

    property Component externalHardwareParameters: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                item.currentSeam = null;
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }

        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("External LWM")
            lastLevelModel: seamConfiguration.wizardModel
            SeamHardwareParameters {
                id: externalHardwareParametersPage
                width: parent.width
                height: parent.height
                title: qsTr("External LWM")
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                screenshotTool: seamConfiguration.screenshotTool
                filterKeys: [
                    HardwareParameters.LWMInspectionActive,
                    HardwareParameters.LWMProgramNumber
                ]
                onMarkAsChanged: seamConfiguration.markAsChanged()
            }
        }
    }

    property Component laserControl: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Laser Control")
            lastLevelModel: seamConfiguration.wizardModel
            SeamLaserControl {
                id: laserControlPage
                width: parent.width
                height: parent.height
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                onMarkAsChanged: seamConfiguration.markAsChanged()

                Connections {
                    target: seamConfiguration
                    function onUpdateSettings() {
                        errorPage.updateSettings()
                    }
                }
            }
        }
    }

    property Component seamScanTracker: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("ScanTracker")
            lastLevelModel: seamConfiguration.wizardModel
            SeamHardwareParameters {
                id: seamScanTrackerPage
                width: parent.width
                height: parent.height
                title: qsTr("Scan Tracker")
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                screenshotTool: seamConfiguration.screenshotTool
                filterKeys: [
                    HardwareParameters.TrackerDriverOnOff,
                    HardwareParameters.ScanWidthOutOfGapWidth,
                    HardwareParameters.ScanPosOutOfGapPos,
                    HardwareParameters.ScanTrackerFrequencyContinuously,
                    HardwareParameters.ScanTrackerScanWidthFixed,
                    HardwareParameters.ScanTrackerScanPosFixed
                ]
                onMarkAsChanged: seamConfiguration.markAsChanged()
            }
        }
    }

    property Component laserWeldingMonitor : Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Laser Welding Monitor")
            lastLevelModel: seamConfiguration.wizardModel
            SeamHardwareParameters {
                id: laserWeldingMonitorPage
                width: parent.width
                height: parent.height
                title: qsTr("Laser Welding Monitor")
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                screenshotTool: seamConfiguration.screenshotTool
                filterKeys: [
                    HardwareParameters.LWM40No1AmpPlasma,
                    HardwareParameters.LWM40No1AmpTemperature,
                    HardwareParameters.LWM40No1AmpBackReflection,
                    HardwareParameters.LWM40No1AmpAnalogInput
                ]
                onMarkAsChanged: seamConfiguration.markAsChanged()
            }
        }
    }

    property Component laserPowerScanner : Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Scanner")
            lastLevelModel: seamConfiguration.wizardModel
            SeamHardwareParameters {
                id: laserPowerScannerPage
                width: parent.width
                height: parent.height
                title: qsTr("Scanner")
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                screenshotTool: seamConfiguration.screenshotTool
                property var basicParameters: [
                    HardwareParameters.ScannerLaserPowerStatic,
                    HardwareParameters.LaserOnDelay,
                    HardwareParameters.LaserOffDelay,
                    HardwareParameters.ScannerDriveToZero,
                    HardwareParameters.ScannerJumpSpeed
                    ]
                property var scanLabSpecific: [
                    HardwareParameters.ScannerFileNumber,
                    HardwareParameters.IsLaserPowerDigital,
                    HardwareParameters.LaserDelay,
                    HardwareParameters.ScannerLaserPowerStaticRing,
                    HardwareParameters.WobbleXSize,
                    HardwareParameters.WobbleYSize,
                    HardwareParameters.WobbleFrequency,
                    HardwareParameters.WobbleMode,
                    HardwareParameters.ScannerCompensateHeight,
                    HardwareParameters.IsCompensationHeightFixed,
                    HardwareParameters.CompensationHeight,
                    HardwareParameters.CorrectionFileMode
                ]
                filterKeys: SystemConfiguration.Scanner2DController == SystemConfiguration.Scanlab ? basicParameters.concat(scanLabSpecific) : basicParameters
                onMarkAsChanged: seamConfiguration.markAsChanged()
            }
        }
    }

    property Component seamScanTracker2D : Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("ScanTracker 2D")
            lastLevelModel: seamConfiguration.wizardModel
            SeamScanTracker2DConfiguration {
                id: scanTracker2DPage
                width: parent.width
                height: parent.height
                title: qsTr("ScanTracker 2D")
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                filterKeys: [
                    HardwareParameters.ScannerLaserPowerStatic,
                    HardwareParameters.WobbleFrequency,
                    HardwareParameters.ScanTracker2DAngle,
                    HardwareParameters.Scantracker2DLaserDelay,
                    HardwareParameters.ScanWidthOutOfGapWidth,
                    HardwareParameters.ScanTracker2DScanWidthFixedX,
                    HardwareParameters.ScanTracker2DScanWidthFixedY,
                    HardwareParameters.ScanPosOutOfGapPos,
                    HardwareParameters.ScanTracker2DScanPosFixedX,
                    HardwareParameters.ScanTracker2DScanPosFixedY
                ]
                onMarkAsChanged: seamConfiguration.markAsChanged()
            }
        }
    }

    property Component seamIDM: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("IDM")
            lastLevelModel: seamConfiguration.wizardModel
            SeamHardwareParameters {
                id: seamIdmPage
                width: parent.width
                height: parent.height
                title: qsTr("IDM")
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                screenshotTool: seamConfiguration.screenshotTool
                showIdm: true
                filterKeys: [
                    HardwareParameters.SLDDimmerOnOff,
                    HardwareParameters.IDMLampIntensity,
                    HardwareParameters.IDMAdaptiveExposureBasicValue,
                    HardwareParameters.IDMAdaptiveExposureModeOnOff
                ]
                onMarkAsChanged: seamConfiguration.markAsChanged()
            }
        }
    }

    property Component zCollimator : Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Z Collimator")
            lastLevelModel: seamConfiguration.wizardModel
            SeamHardwareParameters {
                id: zCollimatorPage
                width: parent.width
                height: parent.height
                title: qsTr("Z Collimator")
                currentSeam: seamConfiguration.seam
                attributeModel: seamConfiguration.keyValueAttributeModel
                screenshotTool: seamConfiguration.screenshotTool
                filterKeys: [
                    HardwareParameters.ZCollimatorPositionAbsolute
                ]
                onMarkAsChanged: seamConfiguration.markAsChanged()
            }
        }
    }

    property Component detection: Loader {
        property var additionalButtonsComponent: seamConfiguration.copyButtonsComponent
        property var sideTabModel: item ? item.sideTabModel : undefined
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                item.currentSeam = null;
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                seamConfiguration.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            property alias sideTabModel: detectionPage.sideTabModel
            product: seamConfiguration.seam.seamSeries.product
            seamSeries: seamConfiguration.seam.seamSeries
            seam: seamConfiguration.seam
            title: qsTr("Detection")
            lastLevelModel: seamConfiguration.wizardModel
            property alias currentSeam: detectionPage.currentSeam
            Detection {
                id: detectionPage
                width: parent.width
                height: parent.height
                currentSeam: seamConfiguration.seam
                resultsConfigModel: seamConfiguration.resultsConfigModel
                errorConfigModel: seamConfiguration.errorConfigModel
                sensorConfigModel: seamConfiguration.sensorConfigModel
                sideTabIndex: seamConfiguration.sideTabIndex
                graphModel: seamConfiguration.graphModel
                subGraphModel: seamConfiguration.subGraphModel
                attributeModel: seamConfiguration.attributeModel
                onlineHelp: seamConfiguration.onlineHelp
                screenshotTool: seamConfiguration.screenshotTool
                onMarkAsChanged: seamConfiguration.markAsChanged()
                onPlotterSettingsUpdated: seamConfiguration.plotterSettingsUpdated()

                Connections {
                    target: seamConfiguration
                    function onUpdateSettings() {
                        detectionPage.updateSettings()
                    }
                }
            }
        }
    }

    property Component copyButtonsComponent: ToolButton {
        display: AbstractButton.IconOnly
        icon {
            name: "edit-copy"
            width: 64
            height: 64
        }
        onClicked: {
            var dialog = saveAllDialogComponent.createObject(Overlay.overlay);
            dialog.open();
        }
    }

    property Component additionalButtonsComponent: ColumnLayout {
        ToolButton {
            display: AbstractButton.IconOnly
            icon {
                name: "edit-copy"
                width: 64
                height: 64
            }
            onClicked: {
                var dialog = saveAllDialogComponent.createObject(Overlay.overlay);
                dialog.open();
            }
            Layout.alignment: Qt.AlignRight
        }
        Repeater {
            id: wizardButtons
            model: seamConfiguration.wizardModel
            property real preferredButtonWidth: 0
            Button {
                display: AbstractButton.TextUnderIcon
                text: model.display
                icon.name: model.icon
                icon.color: Settings.iconColor
                onClicked: {
                    if (model.component == WizardModel.SeamIntervalError)
                    {
                        seamConfiguration.seamIntervalErrorSelected(seamIntervalGroupBox.seamIntervalErrorItem);
                    }
                    else
                    {
                        seamConfiguration.subItemSelected(model.component);
                    }
                }
                Component.onCompleted: {
                    wizardButtons.preferredButtonWidth = Math.max(wizardButtons.preferredButtonWidth, implicitWidth)
                }
                Layout.preferredWidth: wizardButtons.preferredButtonWidth
            }
        }
    }

    signal subItemSelected(var component);
    signal switchToSubItem(var component);
    signal seamIntervalSelected(var uuid, var component);
    signal seamIntervalErrorSelected(var component);
    signal createSeamIntervalSelected(var component);
    signal deleteSeamIntervalSelected(var uuid, bool pop);
    signal deleteSeamSelected(var seam, bool popOnAccept);
    signal markAsChanged();
    signal plotterSettingsUpdated();
    signal updateSettings();
    signal switchToLink(var seam);
    signal referenceCurveSelected(var component);
    signal referenceCurveEditorSelected(var component);

    product: seamConfiguration.seam ? seamConfiguration.seam.seamSeries.product : null
    seamSeries: seamConfiguration.seam ? seamConfiguration.seam.seamSeries: null
    title: qsTr("Details of seam %1 (#%2)").arg(seamConfiguration.seam ? seamConfiguration.seam.name : "")
                                           .arg(seamConfiguration.seam ? seamConfiguration.seam.visualNumber : -1)

    ScanfieldSeamController {
        id: scanfieldController
        scanfieldModule {
            grabberDeviceProxy: HardwareModule.grabberDeviceProxy
            calibrationCoordinatesRequestProxy: HardwareModule.calibrationCoordinatesRequestProxy
        }
        hardwareParametersModule {
            attributeModel: seamConfiguration.keyValueAttributeModel
        }
        transformation: image.transformation
        onCameraCenterChanged: image.centerAndFitTo(scanfieldController.cameraCenter, scanfieldController.scanfieldModule.cameraSize)
    }

    ColumnLayout {
        anchors.fill: parent
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            GridLayout {
                Layout.alignment: Qt.AlignTop
                Layout.fillWidth: true
                Layout.fillHeight: true

                columns: 3
                columnSpacing: 10
                Label {
                    text: qsTr("Name:")
                }
                TextField {
                    id: nameField
                    selectByMouse: true
                    text: seamConfiguration.seam ? seamConfiguration.seam.name : ""
                    validator: FileSystemNameValidator {
                        allowWhiteSpace: true
                    }
                    palette.text: nameField.acceptableInput ? "black" : "red"
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    onEditingFinished: {
                        if (seam && controller && seam.name != text)
                        {
                            seam.name = text;
                            controller.markAsChanged();
                        }
                    }
                }
                Label {
                    text: qsTr("Number:")
                }
                TextField {
                    id: numberBox
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    text: seamConfiguration.seam ? seamConfiguration.seam.visualNumber : "0"
                    validator: MeasureTaskNumberValidator {
                        measureTask: seamConfiguration.seam
                    }
                    selectByMouse: true
                    onEditingFinished: {
                        if (seamConfiguration.seam)
                        {
                            seamConfiguration.seam.number = seamConfiguration.seam.numberFromVisualNumber(Number.fromLocaleString(locale, numberBox.text));
                            Notifications.NotificationSystem.warning(qsTr("Changing number of seam results in all historic result and video data becoming inaccessable"));
                            controller.markAsChanged();
                        }
                    }
                    palette.text: numberBox.acceptableInput ? "black" : "red"
                }

                Label {
                    text: qsTr("Length [%1]:").arg(seamConfiguration.unit)
                }
                TextField {
                    Layout.columnSpan: 2

                    id: lengthField

                    enabled: seamConfiguration.seam && seamConfiguration.seam.seamIntervalsCount <= 2
                    selectByMouse: true
                    text: seamConfiguration.seam ? Number(seamConfiguration.seam.length / 1000).toLocaleString(locale, 'f', 3) : 0

                    validator: DoubleValidator {
                        bottom: -1
                    }

                    onEditingFinished: {
                        if (seamConfiguration.seam && seamConfiguration.controller && seamConfiguration.seam.length != seamConfiguration.length)
                        {
                            seamConfiguration.seam.firstSeamInterval.length = seamConfiguration.length;
                            seamConfiguration.controller.markAsChanged();
                        }
                    }

                    palette.text: lengthField.acceptableInput ? "black" : "red"
                }

                Label {
                    visible: !SystemConfiguration.ImageTriggerViaEncoderSignals
                    text: qsTr("Rate of feed [%1/s]:").arg(seamConfiguration.unit)
                }
                TextField {
                    id: velocityField

                    visible: !SystemConfiguration.ImageTriggerViaEncoderSignals
                    enabled: !SystemConfiguration.ImageTriggerViaEncoderSignals
                    selectByMouse: true
                    text: seamConfiguration.seam ? Number(seamConfiguration.seam.velocity / 1000).toLocaleString(locale, 'f', 3) : 0

                    validator: DoubleValidator {
                        bottom: 0
                    }

                    onEditingFinished: {
                        if (seamConfiguration.seam && seamConfiguration.controller)
                        {
                            var currentTimeDelta = seamConfiguration.seam.velocity !== 0 ? 1000 * seamConfiguration.seam.triggerDelta / seamConfiguration.seam.velocity : 0;
                            seamConfiguration.seam.velocity = Number.fromLocaleString(locale, text) * 1000;
                            // attempt to preserve editable time delta value and adjust distance delta according to the new velocity
                            seamConfiguration.seam.triggerDelta = currentTimeDelta * seamConfiguration.seam.velocity / 1000;
                            seamConfiguration.controller.markAsChanged();
                        }
                    }

                    palette.text: velocityField.acceptableInput ? "black" : "red"
                }
                Label {
                    Layout.fillWidth: true

                    visible: !SystemConfiguration.ImageTriggerViaEncoderSignals
                    text: qsTr("%1 [%2/min]").arg(seamConfiguration.seam ? Number(60 * seamConfiguration.seam.velocity / 1000).toLocaleString(locale, 'f', 3) : 0).arg(seamConfiguration.unit)
                }

                Label {
                    text: qsTr("Trigger delta (time) [ms]:")
                    visible: !SystemConfiguration.ImageTriggerViaEncoderSignals
                }
                TextField {
                    id: triggerDeltaTimeField

                    text: seamConfiguration.seam && (seamConfiguration.seam.velocity !== 0) ? Number(1000 * seamConfiguration.seam.triggerDelta / seamConfiguration.seam.velocity).toLocaleString(locale, 'f', 3) : 0
                    selectByMouse: true
                    visible: !SystemConfiguration.ImageTriggerViaEncoderSignals
                    readOnly: SystemConfiguration.ImageTriggerViaEncoderSignals

                    validator: DoubleValidator {
                        notation: DoubleValidator.StandardNotation
                        // the EtherCAT cycle is 1 ms, thus below doesn't make sense
                        bottom: 1
                        // App_GrabberGigE has a watchdog running with 1 s. Thus we need to stay below with a safety margin
                        top: 500
                    }

                    onEditingFinished: {
                        if (seamConfiguration.seam && seamConfiguration.controller && triggerDeltaTimeField.acceptableInput)
                        {
                            seamConfiguration.seam.triggerDelta = Number.fromLocaleString(locale, text) * (seamConfiguration.seam.velocity / 1000);
                            seamConfiguration.controller.markAsChanged();
                        }
                    }

                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 40
                        border.width: triggerDeltaTimeField.readOnly ? 0 : (triggerDeltaTimeField.activeFocus ? 2 : 1)
                        color: triggerDeltaTimeField.palette.base
                        border.color: triggerDeltaTimeField.activeFocus ? triggerDeltaTimeField.palette.highlight : triggerDeltaTimeField.palette.mid
                    }
                    palette.text: triggerDeltaTimeField.acceptableInput ? "black" : "red"
                }
                Label {
                    id: fpsField
                    visible: !SystemConfiguration.ImageTriggerViaEncoderSignals
                    text: qsTr("%1 FPS").arg(seamConfiguration.seam && (seamConfiguration.seam.triggerDelta !== 0) ? Number(seamConfiguration.seam.velocity / seamConfiguration.seam.triggerDelta).toLocaleString(locale, 'f', 1) : 0)
                }

                Label {
                    text: qsTr("Trigger delta (distance) [%1]:").arg(seamConfiguration.unit)
                }
                TextField {
                    Layout.columnSpan: 2
                    readOnly: !SystemConfiguration.SOUVIS6000_Application

                    id: triggerDeltaField

                    text: seamConfiguration.seam ? Number(seamConfiguration.seam.triggerDelta / 1000).toLocaleString(locale, "f", 3) : 0

                    selectByMouse: true
                    validator: DoubleValidator {
                        bottom: 0
                    }

                    onEditingFinished: {
                        if (seamConfiguration.seam && seamConfiguration.controller)
                        {
                            seamConfiguration.seam.triggerDelta = Number.fromLocaleString(locale, text) * 1000;
                            seamConfiguration.controller.markAsChanged();
                        }
                    }

                    background: Rectangle {
                        implicitWidth: 200
                        implicitHeight: 40
                        border.width: triggerDeltaField.readOnly ? 0 : (triggerDeltaField.activeFocus ? 2 : 1)
                        color: triggerDeltaField.palette.base
                        border.color: triggerDeltaField.activeFocus ? triggerDeltaField.palette.highlight : triggerDeltaField.palette.mid
                    }
                }
                Label {
                    text: qsTr("Moving direction:")
                    visible: movingDirectionBox.visible
                }
                ComboBox {
                    id: movingDirectionBox
                    implicitWidth: lengthField.width
                    model: [qsTr("From Upper"), qsTr("From Lower")]
                    visible: GuiConfiguration.configureMovingDirectionOnSeam
                    currentIndex: seamConfiguration.seam ? seamConfiguration.seam.movingDirection : -1
                    onActivated: {
                        if (seam && controller)
                        {
                            seam.movingDirection = currentIndex;
                            controller.markAsChanged();
                        }
                    }
                    Layout.columnSpan: 2
                }
                Label {
                    text: qsTr("Thickness left [mm]:")
                    visible: thicknessLeftField.visible
                }
                TextField {
                    property real currentValue: Number.fromLocaleString(locale, text)
                    id: thicknessLeftField
                    selectByMouse: true
                    visible: GuiConfiguration.configureThicknessOnSeam
                    validator: DoubleValidator {
                        bottom: 0
                    }
                    text: Number(seamConfiguration.seam ? seamConfiguration.seam.thicknessLeft / 1000 : 0).toLocaleString(locale, 'f', 3)
                    onEditingFinished: {
                        if (seam && controller)
                        {
                            seam.thicknessLeft = currentValue * 1000;
                            controller.markAsChanged();
                        }
                    }
                    palette.text: thicknessLeftField.acceptableInput ? "black" : "red"
                    Layout.columnSpan: 2
                }
                Label {
                    text: qsTr("Thickness right [mm]:")
                    visible: thicknessRightField.visible
                }
                TextField {
                    property real currentValue: Number.fromLocaleString(locale, text)
                    id: thicknessRightField
                    selectByMouse: true
                    visible: GuiConfiguration.configureThicknessOnSeam
                    validator: DoubleValidator {
                        bottom: 0
                    }
                    text: Number(seamConfiguration.seam ? seamConfiguration.seam.thicknessRight / 1000 : 0).toLocaleString(locale, 'f', 3)
                    onEditingFinished: {
                        if (seam && controller)
                        {
                            seam.thicknessRight = currentValue * 1000;
                            controller.markAsChanged();
                        }
                    }
                    palette.text: thicknessRightField.acceptableInput ? "black" : "red"
                    Layout.columnSpan: 2
                }
                Label {
                    text: qsTr("Target Difference [mm]:")
                    visible: targetDifferenceField.visible
                }
                TextField {
                    property real currentValue: Number.fromLocaleString(locale, text)
                    id: targetDifferenceField
                    selectByMouse: true
                    visible: GuiConfiguration.configureThicknessOnSeam
                    validator: DoubleValidator {
                        bottom: 0
                    }
                    text: Number(seamConfiguration.seam ? seamConfiguration.seam.targetDifference / 1000 : 0).toLocaleString(locale, 'f', 3)
                    onEditingFinished: {
                        if (seam && controller)
                        {
                            seam.targetDifference = currentValue * 1000;
                            controller.markAsChanged();
                        }
                    }
                    palette.text: targetDifferenceField.acceptableInput ? "black" : "red"
                    Layout.columnSpan: 2
                }
            }
            GroupBox {
                visible: GuiConfiguration.quickEditFilterParametersOnSeam
                title: qsTr("Detection Parameters")
                StackView {
                    id: stackView
                    clip: true
                    anchors.fill: parent

                    Connections {
                        target: seamConfiguration
                        function onSeamChanged() {
                            stackView.pop(null)
                        }
                    }

                    Component.onCompleted: filterParameterEditor.pushPreConfiguredGraph()
                }
                Layout.preferredWidth: parent.width * 0.33
                Layout.fillHeight: true
            }
        }

        GroupBox {
            title: qsTr("Linked seams")
            visible: linkedSeamListing.count > 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            MeasureTaskListing {
                id: linkedSeamListing
                anchors.fill: parent
                showLink: false
                model: seamConfiguration.seam ? seamConfiguration.seam.linkedSeams : []
                onSelected: {
                    for (var i = 0; i < linkedSeamListing.count; i++)
                    {
                        if (linkedSeamListing.model[i].uuid == uuid)
                        {
                            seamConfiguration.switchToLink(linkedSeamListing.model[i]);
                            break;
                        }
                    }
                }
                onDeleted: {
                    for (var i = 0; i < linkedSeamListing.count; i++)
                    {
                        if (linkedSeamListing.model[i].uuid == uuid)
                        {
                            seamConfiguration.deleteSeamSelected(linkedSeamListing.model[i], false);
                            break;
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: (scanfieldController.scanfieldModule.cameraSize.width / scanfieldController.scanfieldModule.cameraSize.height) * scanfieldImageContainer.height
            Layout.minimumHeight: 0.3 * seamConfiguration.height
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignHCenter

            id: scanfieldImageContainer

            visible: SystemConfiguration.Scanner2DEnable && SystemConfiguration.ScannerGeneralMode == SystemConfiguration.ScannerGenMode.ScanMaster
            border.color: PrecitecApplication.Settings.alternateBackground
            border.width: 1
            clip: true

            ColumnLayout {
                anchors.centerIn: parent
                visible: (!image.imageValid || !scanfieldController.scanfieldModule.configurationValid)
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("No Image available. Please acquire Scan Field Image.")
                }
            }

            ColumnLayout {
                anchors.centerIn: parent
                visible: image.imageValid && scanfieldController.scanfieldModule.configurationValid && !scanfieldController.cameraCenterValid
                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("Please set Camera Position in Scan Field Image.")
                }
            }

            PrecitecImage.SourceImageItem {
                anchors.centerIn: parent

                id: image
                thumbnail: scanfieldController.scanfieldModule.cameraSize
                thumbnailOriginalSize: scanfieldController.scanfieldModule.imageSize
                visible: (image.imageValid || image.loading) && scanfieldController.scanfieldModule.configurationValid && scanfieldController.cameraCenterValid
                pauseUpdate: productStackView.busy
                width: parent.width - 10
                height: parent.height - 10

                Binding {
                    target: image
                    property: "source"
                    delayed: true
                    value: scanfieldController.scanfieldModule.sourceImageDir
                    when: scanfieldController.scanfieldModule.configurationValid && !scanfieldController.scanfieldModule.loading
                }

                onImageChanged: image.centerAndFitTo(image.mapToPaintedImage(scanfieldController.cameraCenter), scanfieldController.scanfieldModule.cameraSize)

                Rectangle {
                    visible: image.imageValid && scanfieldController.scanfieldModule.configurationValid && scanfieldController.cameraCenterValid && !productStackView.busy
                    border.color: "red"
                    border.width: 2
                    color: Qt.rgba(1, 0, 0, 0.2)
                    x: scanfieldController.paintedRoi.x
                    y: scanfieldController.paintedRoi.y
                    width: scanfieldController.paintedRoi.width
                    height: scanfieldController.paintedRoi.height
                }

                DragHandler {
                    property point start
                    id: roiDragHandler
                    target: null
                    minimumPointCount: 1
                    maximumPointCount: 1
                    onActiveChanged: {
                        if (active)
                        {
                            start = image.mapFromPaintedImage(centroid.pressPosition);
                        }
                    }
                    onTranslationChanged: {
                        var mappedTranslation = image.mapFromPaintedImage(Qt.point(translation.x, translation.y));
                        scanfieldController.paintedRoi = Qt.rect(start.x, start.y, mappedTranslation.x, mappedTranslation.y);
                        seamConfiguration.markAsChanged();
                    }
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: image.loading
                }
            }
        }

        SeamIntervalsGroupBox {
            id: seamIntervalGroupBox
            visible: GuiConfiguration.seamIntervalsOnProductStructure
            seam: seamConfiguration.seam
            seamInterval: seamConfiguration.seamInterval
            unit: seamConfiguration.unit
            visualizeLeftMargin: seamConfiguration.leftPadding
            visualizeRightMargin: seamConfiguration.rightPadding
            resultsConfigModel: seamConfiguration.resultsConfigModel
            errorConfigModel: seamConfiguration.errorConfigModel
            qualityNorm: seamConfiguration.qualityNorm
            attributeModel: seamConfiguration.attributeModel
            screenshotTool: seamConfiguration.screenshotTool
            graphModel: seamConfiguration.graphModel
            subGraphModel: seamConfiguration.subGraphModel

            onSeamIntervalSelected: seamConfiguration.seamIntervalSelected(uuid, component)
            onDeleteSeamIntervalSelected: seamConfiguration.deleteSeamIntervalSelected(uuid, pop)
            onSeamIntervalErrorSelected: seamConfiguration.seamIntervalErrorSelected(component)
            onMarkAsChanged: seamConfiguration.markAsChanged()
            onPlotterSettingsUpdated: seamConfiguration.plotterSettingsUpdated()

            Connections {
                target: seamConfiguration
                function onUpdateSettings() {
                    seamIntervalGroupBox.updateSettings()
                }
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        Item {
            visible: !seamIntervalGroupBox.visible && !scanfieldImageContainer.visible
            Layout.fillHeight: true
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Button {
                visible: GuiConfiguration.seamIntervalsOnProductStructure
                text: qsTr("Add new seaminterval")
                display: AbstractButton.TextBesideIcon
                icon.name: "list-add"
                onClicked: seamConfiguration.createSeamIntervalSelected(seamIntervalGroupBox.configurationItem)
            }
            Button {
                display: AbstractButton.TextBesideIcon
                text: qsTr("Delete this seam")
                icon.name: "edit-delete"
                onClicked: seamConfiguration.deleteSeamSelected(seamConfiguration.seam, true)
            }
            Button {
                display: AbstractButton.TextBesideIcon
                icon.name: "edit-undo"
                text: qsTr("Reset Roi")
                visible: image.imageValid && scanfieldController.scanfieldModule.configurationValid && scanfieldController.cameraCenterValid
                onClicked: {
                    scanfieldController.resetRoi();
                    seamConfiguration.markAsChanged();
                }
            }
        }
    }

    FilterParameterEditor {
        id: filterParameterEditor

        FilterParameterOnSeamConfigurationController {
            id: filterParameterController
            subGraphModel: filterParameterEditor.subGraphModel
            currentSeam: seamConfiguration.seam
            onMarkAsChanged: seamConfiguration.controller.markAsChanged()
        }

        graphId: filterParameterController.currentGraphId
        view: stackView
        getFilterParameter: filterParameterController.getFilterParameter
        filterOnUserLevel: true
        filterOnGroup: false

        onParameterValueChanged: filterParameterController.updateFilterParameter(uuid, value)
    }


    Component {
        id: saveAllDialogComponent

        SeamSelectionDialog {
            anchors.centerIn: parent

            width: 0.9 * parent.width
            height: 0.9 * parent.height

            productController: seamConfiguration.controller
            graphModel: seamConfiguration.graphModel
            subGraphModel: seamConfiguration.subGraphModel
            screenshotTool: seamConfiguration.screenshotTool
            attributeModel: seamConfiguration.attributeModel
            resultsConfigModel: seamConfiguration.resultsConfigModel
            sensorConfigModel: seamConfiguration.sensorConfigModel
            errorConfigModel: seamConfiguration.errorConfigModel
            onlineHelp: seamConfiguration.onlineHelp

            Component.onCompleted: {
                roiDragHandler.enabled = false;
            }
            Component.onDestruction: {
                roiDragHandler.enabled = true;
            }
        }
    }
}
