import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui 1.0
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

/**
 * A wizard page showing all products and their structure.
 * Starting point for the product configuration.
 **/
PrecitecApplication.SideTabView {
    id: productStructurePage
    property alias currentProduct: controller.currentProduct
    property alias currentSeamSeries: controller.currentSeamSeries
    property alias currentSeam: controller.currentSeam
    property alias currentSeamInterval: controller.currentSeamInterval
    property string unit: currentProduct ? (currentProduct.lengthUnit == Product.Millimeter ? "mm" : "°" ) : "mm"
    property var productController: controller
    property var keyValueAttributeModel: null
    property var graphModel: null
    property var subGraphModel: null
    property var attributeModel: null
    property alias wizardModel: wizardFilterModel.sourceModel
    property var resultsConfigModel: null
    property var errorConfigModel: null
    property var sensorConfigModel: null
    property var qualityNormModel: null
    property var onlineHelp: null
    property var screenshotTool: null

    signal updateSettings()
    signal plotterSettingsUpdated()
    signal productStructureEnabledChanged(bool enabled)
    Connections {
        target: productStructurePage.productController
        function onCopyInProgressChanged() {
            productStructurePage.productController.copyInProgress
            if (productStructurePage.productController.copyInProgress)
            {
                productStructurePage.enabled = false;
                productStructurePage.productStructureEnabledChanged(false);
            } else
            {
                productStructurePage.enabled = true;
                productStructurePage.productStructureEnabledChanged(true);
            }
        }
    }
    function configureSeam(productId, seamSeriesId, seamId)
    {
        productStackView.pop(null, StackView.Immediate);
        controller.selectProduct(productId, seamSeriesId, seamId);
        controller.selectSeamSeries(seamSeriesId, seamId);
        controller.selectSeam(seamId);
        productStackView.productItem = productStackView.push(productConfiguration, StackView.Immediate);
        if (GuiConfiguration.seamSeriesOnProductStructure)
        {
            productStackView.seamSeriesItem = productStackView.push(seamSeriesConfiguration, StackView.Immediate);
        }
        productStackView.seamItem = productStackView.push(componentForSeamConfiguration(), StackView.Immediate);
    }

    function configureSeamSeries(productId, seamSeriesId)
    {
        productStackView.pop(null, StackView.Immediate);
        controller.selectProduct(productId, seamSeriesId);
        controller.selectSeamSeries(seamSeriesId);
        productStackView.productItem = productStackView.push(productConfiguration, StackView.Immediate);
        productStackView.seamSeriesItem = productStackView.push(seamSeriesConfiguration, StackView.Immediate);
    }

    function componentForSeamConfiguration()
    {
        if (!currentSeam || currentSeam.linkTo == undefined)
        {
            return seamConfigurationComponent;
        }
        return linkedSeamConfigurationComponent;
    }

    alignment: Qt.AlignRight
    model: productStackView.currentItem.sideTabModel
    onModelChanged: {
        currentIndex = 0;
    }

    ProductFilterModel {
        id: productFilterModel
        sourceModel: controller
    }

    ProductController {
        id: controller

        scanfieldPath: WeldmasterPaths.scanFieldDir
        productModel: HardwareModule.productModel
        schedulerEventsProxy: HardwareModule.schedulerEventsProxy
    }

    SeamWizardFilterModel {
        id: wizardFilterModel
        yAxisAvailable: yAxisInformation.axisEnabled
        scanTracker: SystemConfiguration.ScanTrackerEnable
        sensorGrabberAvailable: SystemConfiguration.HardwareCameraEnabled
        laserControlAvailable: SystemConfiguration.LaserControlEnable
        ledAvailable: SystemConfiguration.LED_IlluminationEnable
        lwmAvailable: SystemConfiguration.LWM40_No1_Enable
        scanlabScannerAvailable: SystemConfiguration.Scanner2DEnable
        idmAvailable: SystemConfiguration.IDM_Device1Enable
        zCollimator: SystemConfiguration.ZCollimatorEnable
        scanTracker2DAvailable: SystemConfiguration.ScannerGeneralMode === SystemConfiguration.ScannerGenMode.ScanTracker2D

    }

    SeamSeriesWizardFilterModel {
        id: seamSeriesWizardFilterModel
        yAxisAvailable: yAxisInformation.axisEnabled
        scanTracker: SystemConfiguration.ScanTrackerEnable
        sourceModel: wizardFilterModel.sourceModel
        sensorGrabberAvailable: SystemConfiguration.HardwareCameraEnabled
        laserControlAvailable: SystemConfiguration.LaserControlEnable
        ledAvailable: SystemConfiguration.LED_IlluminationEnable
        lwmAvailable: SystemConfiguration.LWM40_No1_Enable
        scanlabScannerAvailable: SystemConfiguration.Scanner2DEnable
        idmAvailable: SystemConfiguration.IDM_Device1Enable
        zCollimator: SystemConfiguration.ZCollimatorEnable
        scanTracker2DAvailable: SystemConfiguration.ScannerGeneralMode === SystemConfiguration.ScannerGenMode.ScanTracker2D
    }

    ProductWizardFilterModel {
        id: productWizardFilterModel
        yAxisAvailable: yAxisInformation.axisEnabled
        scanTracker: SystemConfiguration.ScanTrackerEnable
        sourceModel: wizardFilterModel.sourceModel
        sensorGrabberAvailable: SystemConfiguration.HardwareCameraEnabled
        laserControlAvailable: SystemConfiguration.LaserControlEnable
        ledAvailable: SystemConfiguration.LED_IlluminationEnable
        lwmAvailable: SystemConfiguration.LWM40_No1_Enable
        scanlabScannerAvailable: SystemConfiguration.Scanner2DEnable
        idmAvailable: SystemConfiguration.IDM_Device1Enable
        zCollimator: SystemConfiguration.ZCollimatorEnable
        scanTracker2DAvailable: SystemConfiguration.ScannerGeneralMode === SystemConfiguration.ScannerGenMode.ScanTracker2D
    }

    AssemblyImagesModel {
        id: imagesModel
        Component.onCompleted: {
            imagesModel.loadImages(WeldmasterPaths.assemblyImagesDir);
        }
    }

    Component {
        id: deleteSeamDialogComponent
        Dialog {
            id: deleteSeamDialog
            property var seam: null
            property bool popOnAccept: true
            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true
            title: qsTr("Delete seam?")
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape

            onAccepted: {
                seam.seamSeries.destroySeam(seam);
                if (popOnAccept)
                {
                    productStackView.pop();
                }
                destroy();
            }
            onRejected: {
                destroy();
            }

            Label {
                text: qsTr("Do you really want to delete the seam \"%1\" from Product \"%2\"?\nDeleting a seam cannot be undone.").arg(deleteSeamDialog.seam ? deleteSeamDialog.seam.name : "").arg(deleteSeamDialog.seam ? deleteSeamDialog.seam.seamSeries.product.name : "")
            }
        }
    }

    Component {
        id: newProductDialogComponent
        Dialog {
            id: newProductDialog
            title: qsTr("Add new product")
            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel
            onAccepted: {
                var newProduct = null;
                if (copyFromExistingProductButton.checked)
                {
                    newProduct = controller.createProductAsCopy(productFilterModel.data(productFilterModel.index(productCombo.currentIndex, 0), Qt.UserRole));
                } else
                {
                    newProduct = controller.createProduct();
                }
                if (newProduct)
                {
                    controller.selectProduct(newProduct.uuid);
                    productStackView.push(productConfiguration);
                }
                destroy();
            }
            onRejected: {
                destroy();
            }
            ColumnLayout {
                RadioButton {
                    text: qsTr("Start with empty product")
                    checked: true
                }
                RadioButton {
                    id: copyFromExistingProductButton
                    text: qsTr("Copy from existing product")
                    enabled: productCombo.count > 0
                }
                ComboBox {
                    id: productCombo
                    enabled: copyFromExistingProductButton.checked
                    model: productFilterModel
                    textRole: "display"

                    Layout.fillWidth: true
                }
            }
            Connections {
                target: UserManagement
                function onCurrentUserChanged() {
                    newProductDialog.reject()
                }
            }
        }
    }

    Component {
        id: deleteSeamIntervalDialogComponent
        Dialog {
            id: deleteSeamIntervalDialog
            property var seam: null
            property var seamInterval: null
            property bool popOnAccept: true
            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true
            title: qsTr("Delete seaminteval?")
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape

            onAccepted: {
                if (seam && seamInterval)
                {
                    seam.destroySeamInterval(seamInterval);
                    productStructurePage.productController.markAsChanged();
                    if (popOnAccept)
                    {
                        productStackView.pop();
                    }
                }
                destroy();
            }
            onRejected: {
                destroy();
            }

            Label {
                text: qsTr("Do you really want to delete the seaminterval \"%1\" from Seam \"%2\"?\nDeleting a seaminterval cannot be undone.")
                .arg(deleteSeamIntervalDialog.seamInterval ? deleteSeamIntervalDialog.seamInterval.name : "")
                .arg(deleteSeamIntervalDialog.seamInterval ? deleteSeamIntervalDialog.seamInterval.seam.name : "")
            }
        }
    }

    Component {
        id: productConfiguration
        ProductConfiguration {
            id: productConfigTitleBox
            title: qsTr("Details of product %1").arg(productStructurePage.currentProduct ? productStructurePage.currentProduct.name : "")
            product: productStructurePage.currentProduct
            assemblyImagesModel: imagesModel
            yAxisAvailable: yAxisInformation.axisEnabled
            controller: productController
            qualityNormModel: productStructurePage.qualityNormModel
            graphModel: productStructurePage.graphModel
            subGraphModel: productStructurePage.subGraphModel
            attributeModel: productStructurePage.attributeModel
            wizardModel: productWizardFilterModel
            resultsConfigModel: productStructurePage.resultsConfigModel
            unitVisible: !wizardFilterModel.scanlabScannerAvailable || wizardFilterModel.scanTracker2DAvailable
            onRemovableDeviceCommunicationStatusChanged: {
               productStructure.enabled = isFinished;
               productStructurePage.productStructureEnabledChanged(isFinished);
            }

            onSeamSelected:  {
                controller.selectSeam(uuid);
                productStackView.seamItem = productStackView.push(componentForSeamConfiguration())
            }
            onSeamDeleted: {
                controller.selectSeam(uuid);
                var dialog = deleteSeamDialogComponent.createObject(productStructurePage);
                dialog.seam = productStructurePage.currentSeam
                dialog.popOnAccept = false;
                dialog.open();
            }
            onSeamSeriesSelected: {
                controller.selectSeamSeries(uuid)
                productStackView.seamSeriesItem = productStackView.push(seamSeriesConfiguration);
            }
            onSeamCreated: {
                if (productStructurePage.currentSeam)
                {
                    productStackView.seamItem = productStackView.push(seamConfigurationComponent)
                }
            }
            onSeamSeriesCreated: {
                if (productStructurePage.currentSeamSeries)
                {
                    productStackView.seamSeriesItem = productStackView.push(seamSeriesConfiguration)
                }
            }
            onSensorSelected: productStackView.push(productCameraImage)
            onScanTrackerSelected: productStackView.push(productScanTracker)
            onColorMapsSelected: productStackView.push(colorMapEditor)
            onErrorsSelected: productStackView.push(productErrors)
            onLwmSelected: productStackView.push(productLaserWeldingMonitor)
            onLaserControlSelected: productStackView.push(productLaserControl)
            onIdmSelected: productStackView.push(productIDM)
            onHardwareParametersOverviewSelected: productStackView.push(productHardwareParametersOverview)
            onDetecionOverviewSelected: productStackView.push(productDetectionOverview)
            onZCollimatorSelected: productStackView.push(productZCollimator)
            onScanlabScannerSelected: productStackView.push(productScanlabScanner)
            onScanTracker2DSelected: productStackView.push(productScanTracker2D)
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
    Component {
        id: productCameraImage
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("Sensor")
                ProductHardwareParameters {
                    id: productCameraPage
                    width: parent.width
                    height: parent.height
                    currentProduct: productStructurePage.currentProduct
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    title: qsTr("Camera parameter")

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
                        HardwareParameters.LEDPanel4PulseWidth,,
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
                        HardwareParameters.LineLaser1OnOff,
                        HardwareParameters.LineLaser1Intensity,
                        HardwareParameters.LineLaser2OnOff,
                        HardwareParameters.LineLaser2Intensity,
                        HardwareParameters.FieldLight1OnOff,
                        HardwareParameters.FieldLight1Intensity,
                        HardwareParameters.ExposureTime,
                        HardwareParameters.ClearEncoderCounter1,
                        HardwareParameters.ClearEncoderCounter2,
                        HardwareParameters.LiquidLensPosition
                    ]

                    property var cameraParameters:[
                        HardwareParameters.LinLogMode,
                        HardwareParameters.LinLogValue1,
                        HardwareParameters.LinLogValue2,
                        HardwareParameters.LinLogTime1,
                        HardwareParameters.LinLogTime2
                    ]

                    filterKeys: HardwareModule.CameraInterfaceType == 0 ? basicParameters.concat(cameraParameters) : basicParameters

                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: productScanTracker
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("Scan Tracker")
                ProductHardwareParameters {
                    id: productScanTrackerPage
                    width: parent.width
                    height: parent.height
                    currentProduct: productStructurePage.currentProduct
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    title: qsTr("Scan Tracker")
                    filterKeys: [
                        HardwareParameters.TrackerDriverOnOff,
                        HardwareParameters.ScanWidthOutOfGapWidth,
                        HardwareParameters.ScanPosOutOfGapPos,
                        HardwareParameters.ScanTrackerFrequencyContinuously,
                        HardwareParameters.ScanTrackerScanWidthFixed,
                        HardwareParameters.ScanTrackerScanPosFixed
                    ]
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: productZCollimator
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("Z Collimator")
                ProductHardwareParameters {
                    id: productZCollimatorPage
                    width: parent.width
                    height: parent.height
                    currentProduct: productStructurePage.currentProduct
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    title: qsTr("Z Collimator")
                    filterKeys: [
                        HardwareParameters.ZCollimatorPositionAbsolute
                    ]
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: productLaserWeldingMonitor
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("Laser Welding Monitor")
                ProductHardwareParameters {
                    id: productLaserWeldingMonitorPage
                    width: parent.width
                    height: parent.height
                    currentProduct: productStructurePage.currentProduct
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    title: qsTr("Laser Welding Monitor")
                    filterKeys: [
                        HardwareParameters.LWM40No1AmpPlasma,
                        HardwareParameters.LWM40No1AmpTemperature,
                        HardwareParameters.LWM40No1AmpBackReflection,
                        HardwareParameters.LWM40No1AmpAnalogInput
                    ]
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: productScanlabScanner
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("Scanner")
                ProductHardwareParameters {
                    id: productScanlabScannerPage
                    width: parent.width
                    height: parent.height
                    currentProduct: productStructurePage.currentProduct
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    title: qsTr("Scanner")
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
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: productScanTracker2D
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("ScanTracker 2D")
                ProductScanTracker2DConfiguration {
                    id: productScanTracker2DPage
                    width: parent.width
                    height: parent.height
                    currentProduct: productStructurePage.currentProduct
                    attributeModel: productStructurePage.keyValueAttributeModel
                    title: qsTr("ScanTracker 2D")
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
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: productIDM
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("IDM")
                ProductHardwareParameters {
                    id: productIDMPage
                    width: parent.width
                    height: parent.height
                    currentProduct: productStructurePage.currentProduct
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    showIdm: true
                    title: qsTr("IDM")
                    filterKeys: [
                        HardwareParameters.SLDDimmerOnOff,
                        HardwareParameters.IDMLampIntensity,
                        HardwareParameters.IDMAdaptiveExposureBasicValue,
                        HardwareParameters.IDMAdaptiveExposureModeOnOff
                    ]
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: productLaserControl
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("Laser Control")
                ProductLaserControl {
                    id: laserControlPage
                    width: parent.width
                    height: parent.height
                    currentProduct: productStructurePage.currentProduct
                    attributeModel: productStructurePage.keyValueAttributeModel
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: colorMapEditor
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("Color Maps")
                ColorMapEditor {
                    anchors.fill: parent
                    currentProduct: productStructurePage.currentProduct
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: productErrors
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                productToNextLevelEnabled: false
                title: qsTr("Errors")
                ProductErrorList {
                    anchors.fill: parent
                    currentProduct: productStructurePage.currentProduct
                    errorConfigModel: productStructurePage.errorConfigModel
                    attributeModel: productStructurePage.attributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: productHardwareParametersOverview
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: HardwareParametersOverview {
                keyValueAttributeModel: productStructurePage.keyValueAttributeModel
                product: productStructurePage.currentProduct
            }
        }
    }

    Component {
        id: productDetectionOverview
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
            }
            sourceComponent: DetectionOverview {
                product: productStructurePage.currentProduct
                graphModel: productStructurePage.graphModel
                subGraphModel: productStructurePage.subGraphModel

                onGraphDeltaSelected: productStackView.push(detectionParametersToGraphDelta, {"seam": seam})
                onDeltaSelected: productStackView.push(detectionParameterSetsDelta, {"seam": seam})
            }
        }
    }

    Component {
        id: detectionParametersToGraphDelta
        Loader {
            property var seam: null
            signal back()
            Connections {
                target: item
                function onBack() {
                    back()
                }
            }
            sourceComponent: DetectionParametersToGraphDelta {
                seam: parent.seam
                graphModel: productStructurePage.graphModel
                subGraphModel: productStructurePage.subGraphModel
                attributeModel: productStructurePage.attributeModel
                resultsConfigModel: productStructurePage.resultsConfigModel
                sensorConfigModel: productStructurePage.sensorConfigModel
                screenshotTool: productStructurePage.screenshotTool

                onMarkAsChanged: productStructurePage.productController.markAsChanged()
            }
        }
    }

    Component {
        id: detectionParameterSetsDelta
        Loader {
            property var seam: null
            signal back()
            Connections {
                target: item
                function onBack() {
                    back()
                }
            }
            sourceComponent: DetectionParameterSetsDelta {
                seam: parent.seam
                graphModel: productStructurePage.graphModel
                subGraphModel: productStructurePage.subGraphModel
                attributeModel: productStructurePage.attributeModel
                resultsConfigModel: productStructurePage.resultsConfigModel
                sensorConfigModel: productStructurePage.sensorConfigModel
                screenshotTool: productStructurePage.screenshotTool

                onMarkAsChanged: productStructurePage.productController.markAsChanged()
            }
        }
    }

    Component {
        id: seamSeriesConfiguration
        Loader {
            property var additionalButtonsComponent: item ? item.additionalButtonsComponent : undefined
            signal back()
            signal backToProductStructure()
            signal backToProduct()
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
                function onSwitchToSeam(index) {
                    switchToSeam(index)
                }
                function onSwitchToSeamSeries(index) {
                    switchToSeamSeries(index)
                }
            }
            sourceComponent: SeamSeriesConfiguration {
                Layout.fillWidth: true
                controller: productStructurePage.productController
                seamSeries: productStructurePage.currentSeamSeries ? productStructurePage.currentSeamSeries : null
                attributeModel: productStructurePage.keyValueAttributeModel
                wizardFilterModel: seamSeriesWizardFilterModel
                screenshotTool: productStructurePage.screenshotTool

                onSeamSelected:  {
                    controller.selectSeam(uuid);
                    productStackView.seamItem = productStackView.push(componentForSeamConfiguration())
                }
                onSeamDeleted: {
                    controller.selectSeam(uuid);
                    var dialog = deleteSeamDialogComponent.createObject(productStructurePage);
                    dialog.seam = productStructurePage.currentSeam
                    dialog.popOnAccept = false;
                    dialog.open();
                }
                onSeamCreated: {
                    if (productStructurePage.currentSeam)
                    {
                        productStackView.seamItem = productStackView.push(seamConfigurationComponent)
                    }
                }
                onLaserControlSelected: productStackView.push(seriesLaserControlComponent)
                onScanTrackerSelected: productStackView.push(seriesScanTrackerComponent)
                onLwmSelected: productStackView.push(seriesLWMComponent)
                onErrorsSelected: productStackView.push(seriesErrorsComponent)
                onScanlabSelected: productStackView.push(seriesScanlabComponent)
                onIdmSelected: productStackView.push(seriesIDMComponent)
                onZCollimatorSelected: productStackView.push(seriesZCollimatorComponent)
                onScanTracker2DSelected: productStackView.push(seriesScanTracker2DComponent)
            }
        }
    }

    Component {
        id: seriesLaserControlComponent
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                seamSeries: productStructurePage.currentSeamSeries ? productStructurePage.currentSeamSeries : null
                productToNextLevelEnabled: false
                seamSeriesToNextLevelEnabled: false
                title: qsTr("Laser Control")

                SeamLaserControl {
                    id: laserControlPage
                    width: parent.width
                    height: parent.height
                    currentSeam: productStructurePage.currentSeamSeries
                    attributeModel: productStructurePage.keyValueAttributeModel
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: seriesScanTrackerComponent
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                seamSeries: productStructurePage.currentSeamSeries ? productStructurePage.currentSeamSeries : null
                productToNextLevelEnabled: false
                seamSeriesToNextLevelEnabled: false
                title: qsTr("ScanTracker")

                SeriesHardwareParameters {
                    id: seriesScanTrackerPage
                    width: parent.width
                    height: parent.height
                    title: qsTr("Scan Tracker")
                    currentSeamSeries: productStructurePage.currentSeamSeries
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    filterKeys: [
                        HardwareParameters.TrackerDriverOnOff,
                        HardwareParameters.ScanWidthOutOfGapWidth,
                        HardwareParameters.ScanPosOutOfGapPos,
                        HardwareParameters.ScanTrackerFrequencyContinuously,
                        HardwareParameters.ScanTrackerScanWidthFixed,
                        HardwareParameters.ScanTrackerScanPosFixed
                    ]
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: seriesLWMComponent
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                seamSeries: productStructurePage.currentSeamSeries ? productStructurePage.currentSeamSeries : null
                productToNextLevelEnabled: false
                seamSeriesToNextLevelEnabled: false
                title: qsTr("Laser Welding Monitor")
                SeriesHardwareParameters {
                    id: seriesLWMPage
                    width: parent.width
                    height: parent.height
                    currentSeamSeries: productStructurePage.currentSeamSeries
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    title: qsTr("Laser Welding Monitor")
                    filterKeys: [
                        HardwareParameters.LWM40No1AmpPlasma,
                        HardwareParameters.LWM40No1AmpTemperature,
                        HardwareParameters.LWM40No1AmpBackReflection,
                        HardwareParameters.LWM40No1AmpAnalogInput
                    ]
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: seriesScanlabComponent
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                seamSeries: productStructurePage.currentSeamSeries ? productStructurePage.currentSeamSeries : null
                productToNextLevelEnabled: false
                seamSeriesToNextLevelEnabled: false
                title: qsTr("Scanner")
                SeriesHardwareParameters {
                    id: seriesScanlabPage
                    width: parent.width
                    height: parent.height
                    currentSeamSeries: productStructurePage.currentSeamSeries
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    title: qsTr("Scanner")
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
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: seriesScanTracker2DComponent
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                seamSeries: productStructurePage.currentSeamSeries ? productStructurePage.currentSeamSeries : null
                productToNextLevelEnabled: false
                seamSeriesToNextLevelEnabled: false
                title: qsTr("ScanTracker 2D")
                SeamSeriesScanTracker2DConfiguration {
                    id: seriesScanTracker2DPage
                    width: parent.width
                    height: parent.height
                    currentSeamSeries: productStructurePage.currentSeamSeries
                    attributeModel: productStructurePage.keyValueAttributeModel
                    title: qsTr("ScanTracker 2D")
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
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: seriesIDMComponent
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                seamSeries: productStructurePage.currentSeamSeries ? productStructurePage.currentSeamSeries : null
                productToNextLevelEnabled: false
                seamSeriesToNextLevelEnabled: false
                title: qsTr("IDM")
                SeriesHardwareParameters {
                    id: seriesIDMPage
                    width: parent.width
                    height: parent.height
                    currentSeamSeries: productStructurePage.currentSeamSeries
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    showIdm: true
                    title: qsTr("IDM")
                    filterKeys: [
                        HardwareParameters.SLDDimmerOnOff,
                        HardwareParameters.IDMLampIntensity,
                        HardwareParameters.IDMAdaptiveExposureBasicValue,
                        HardwareParameters.IDMAdaptiveExposureModeOnOff
                    ]
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: seriesZCollimatorComponent
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                seamSeries: productStructurePage.currentSeamSeries ? productStructurePage.currentSeamSeries : null
                productToNextLevelEnabled: false
                seamSeriesToNextLevelEnabled: false
                title: qsTr("Z Collimator")
                SeriesHardwareParameters {
                    id: seriesZCollimatorPage
                    width: parent.width
                    height: parent.height
                    currentSeamSeries: productStructurePage.currentSeamSeries
                    attributeModel: productStructurePage.keyValueAttributeModel
                    screenshotTool: productStructurePage.screenshotTool
                    title: qsTr("Z Collimator")
                    filterKeys: [
                        HardwareParameters.ZCollimatorPositionAbsolute
                    ]
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                }
            }
        }
    }

    Component {
        id: seriesErrorsComponent
        Loader {
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
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
            }
            sourceComponent: BreadCrumpGroupBox {
                product: productStructurePage.currentProduct
                seamSeries: productStructurePage.currentSeamSeries ? productStructurePage.currentSeamSeries : null
                productToNextLevelEnabled: false
                seamSeriesToNextLevelEnabled: false
                title: qsTr("Errors")
                SeriesErrorList {
                    anchors.fill: parent
                    currentProduct: productStructurePage.currentProduct
                    errorConfigModel: productStructurePage.errorConfigModel
                    currentSeamSeries: productStructurePage.currentSeamSeries
                    attributeModel: productStructurePage.attributeModel
                    onMarkAsChanged: productStructurePage.productController.markAsChanged()
                    screenshotTool: productStructurePage.screenshotTool
                }
            }
        }
    }

    Component {
        id: seamConfigurationComponent
        Loader {
            property var additionalButtonsComponent: item ? item.additionalButtonsComponent : undefined
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
            signal switchToSeam(int index)
            signal switchToSeamSeries(int index)
            signal switchToLink(var seam)
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
                function onSwitchToSeam(index) {
                    switchToSeam(index)
                }
                function onSwitchToSeamSeries(index) {
                    switchToSeamSeries(index)
                }
                function onSwitchToLink(seam) {
                    switchToLink(seam)
                }
            }
            sourceComponent: SeamConfiguration {
                id: seamConfiguration
                seam: productStructurePage.currentSeam
                seamInterval: productStructurePage.currentSeamInterval
                unit: productStructurePage.unit
                graphModel: productStructurePage.graphModel
                subGraphModel: productStructurePage.subGraphModel
                attributeModel: productStructurePage.attributeModel
                resultsConfigModel: productStructurePage.resultsConfigModel
                errorConfigModel: productStructurePage.errorConfigModel
                sensorConfigModel: productStructurePage.sensorConfigModel
                qualityNorm: productStructurePage.qualityNormModel.qualityNorm(currentProduct.qualityNorm)
                controller: productController
                onlineHelp: productStructurePage.onlineHelp
                wizardModel: wizardFilterModel
                keyValueAttributeModel: productStructurePage.keyValueAttributeModel
                sideTabIndex: productStructurePage.currentIndex
                screenshotTool: productStructurePage.screenshotTool

                function switchToItem(component, transition)
                {
                    if (component == WizardModel.SeamAssemblyImage)
                    {
                        productStackView.push(seamConfiguration.seamAssemblyImage, {}, transition);
                    } else if (component == WizardModel.SeamCamera)
                    {
                        productStackView.push(seamConfiguration.seamCameraImage, {}, transition);
                    } else if (component == WizardModel.SeamAxis)
                    {
                        productStackView.push(seamConfiguration.seamAxis, {}, transition);
                    } else if (component == WizardModel.SeamDetection)
                    {
                        productStackView.push(seamConfiguration.detection, {}, transition);
                    } else if (component == WizardModel.SeamError)
                    {
                        productStackView.push(seamConfiguration.seamError, {}, transition);
                    } else if (component == WizardModel.SeamReferenceCurves)
                    {
                        productStackView.push(seamConfiguration.referenceCurves, {}, transition);
                    } else if (component == WizardModel.SeamLaserControl)
                    {
                        productStackView.push(seamConfiguration.laserControl, {}, transition);
                    } else if (component == WizardModel.SeamLaserWeldingMonitor)
                    {
                        productStackView.push(seamConfiguration.laserWeldingMonitor, {}, transition);
                    } else if (component == WizardModel.SeamScanTracker)
                    {
                        productStackView.push(seamConfiguration.seamScanTracker, {}, transition);
                    } else if (component == WizardModel.SeamScanLabScanner)
                    {
                        productStackView.push(seamConfiguration.laserPowerScanner, {}, transition);
                    } else if (component == WizardModel.SeamReferenceImage)
                    {
                        productStackView.push(seamConfiguration.referenceImage, {}, transition);
                    } else if (component == WizardModel.SeamIDM)
                    {
                        productStackView.push(seamConfiguration.seamIDM, {}, transition);
                    } else if (component == WizardModel.SeamZCollimator)
                    {
                        productStackView.push(seamConfiguration.zCollimator, {}, transition);
                    } else if (component == WizardModel.SeamScanTracker2D)
                    {
                        productStackView.push(seamConfiguration.seamScanTracker2D, {}, transition);
                    } else if (component == WizardModel.SeamExternalLWM)
                    {
                        productStackView.push(seamConfiguration.externalHardwareParameters, {}, transition);
                    }
                }

                onSwitchToSubItem: {
                    productStackView.pop(productStackView.seamItem, StackView.Immediate);
                    switchToItem(component, StackView.Immediate);
                }
                onSubItemSelected: switchToItem(component, StackView.Transition)
                onSeamIntervalSelected: {
                    productStructurePage.productController.selectSeamInterval(uuid);
                    productStackView.push(component);
                }
                onSeamIntervalErrorSelected: productStackView.push(component)
                onCreateSeamIntervalSelected: {
                    var seamInterval = productStructurePage.currentSeam.createSeamInterval();
                    seamConfiguration.seamIntervalSelected(seamInterval.uuid, component);
                    productStructurePage.productController.markAsChanged();
                }
                onDeleteSeamIntervalSelected: {
                    productStructurePage.productController.selectSeamInterval(uuid);
                    var dialog = deleteSeamIntervalDialogComponent.createObject(productStructurePage);
                    dialog.popOnAccept = pop;
                    dialog.seam = productStructurePage.currentSeam
                    dialog.seamInterval = productStructurePage.currentSeamInterval
                    dialog.open();
                }
                onDeleteSeamSelected: {
                    var dialog = deleteSeamDialogComponent.createObject(productStructurePage);
                    dialog.seam = seam;
                    dialog.popOnAccept = popOnAccept;
                    dialog.open();
                }
                onReferenceCurveSelected: productStackView.push(component)
                onReferenceCurveEditorSelected: productStackView.push(component)
                onMarkAsChanged: productStructurePage.productController.markAsChanged()
                onPlotterSettingsUpdated: productStructurePage.plotterSettingsUpdated()

                Connections {
                    target: productStructurePage
                    function onUpdateSettings() {
                        seamConfiguration.updateSettings()
                    }
                }
            }
        }
    }

    Component {
        id: linkedSeamConfigurationComponent

        Loader {
            property var additionalButtonsComponent: item ? item.additionalButtonsComponent : undefined
            signal back()
            signal backToProductStructure()
            signal backToProduct()
            signal backToSeamSeries()
            signal switchToSeam(int index)
            signal switchToSeamSeries(int index)
            signal switchToLink(var seam)
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
                function onSwitchToSeam(index) {
                    switchToSeam(index)
                }
                function onSwitchToSeamSeries(index) {
                    switchToSeamSeries(index)
                }
                function onSwitchToLink(seam) {
                    switchToLink(seam)
                }
            }
            sourceComponent: LinkedSeamConfiguration {
                id: linkedSeamConfiguration
                seam: productStructurePage.currentSeam
                productController: controller
                wizardModel: LinkedSeamWizardFilterModel {
                    sourceModel: productStructurePage.wizardModel
                }

                function switchToItem(component, transition)
                {
                    if (component == WizardModel.SeamAssemblyImage)
                    {
                        productStackView.push(linkedSeamConfiguration.seamAssemblyImage, {}, transition);
                    }
                }

                onSwitchToSubItem: {
                    productStackView.pop(productStackView.seamItem, StackView.Immediate);
                    switchToItem(component, StackView.Immediate);
                }
                onSubItemSelected: switchToItem(component, StackView.Transition)
                onMarkAsChanged: productStructurePage.productController.markAsChanged()
            }
        }
    }

    Component {
        id: importProductXmlDialogComponent
        Dialog {
            id: importProductXmlDialog
            parent: Overlay.overlay
            modal: true
            anchors.centerIn: parent
            width: parent.width * 0.8
            height: parent.height * 0.8
            onClosed: importProductXmlDialog.destroy()

            header: PrecitecApplication.DialogHeaderWithScreenshot {
                title: qsTr("Import product from weldmaster/import on attached removable device")
                screenshotTool: productStructurePage.screenshotTool
            }
            // workaround for https://bugreports.qt.io/browse/QTBUG-72372
            footer: DialogButtonBox {
                alignment: Qt.AlignRight
                Button {
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                }
            }

            ColumnLayout {
                anchors.fill: parent
                Label {
                    text: qsTr("Please note that only the product structure is imported.\nNeither hardware parameters, nor filter parameters nor sum errors are imported.")
                }
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ListView {
                        id: availableProductsToImportListView
                        anchors.fill: parent
                        model: productStructurePage.productController.productImportList(RemovableDevices.Service.path + "/weldmaster/import/")
                        delegate: ItemDelegate {
                            text: modelData.substring(modelData.lastIndexOf("/") + 1)
                            icon.name: "cloud-upload"
                            onClicked: {
                                var product = productStructurePage.productController.importFromXml(modelData);
                                if (product)
                                {
                                    controller.selectProduct(product.uuid);
                                    productStackView.push(productConfiguration);
                                    importProductXmlDialog.close();
                                }
                            }
                        }
                    }
                }
                Label {
                    visible: availableProductsToImportListView.count == 0
                    text: qsTr("No products found")
                }
            }
        }
    }

    Component {
        id: overwriteProductOnDiskWithSeparatedProductDialogComponent
        Dialog {
            id: overwriteProductOnDiskDialog
            property string sourcePath: ""
            property string productName: ""
            property string targetPath: ""
            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true
            title: qsTr("Overwrite product")
            standardButtons: Dialog.Yes | Dialog.No

            onAccepted: {
                productStructurePage.productController.importSeparatedProduct(sourcePath);
                destroy();
            }
            onRejected: {
                destroy();
            }

            Label {
                text: qsTr("Warning! Product \"%1\" will be overwritten.")
                            .arg(overwriteProductOnDiskDialog.productName)
            }
        }
    }

    Component {
        id: importSeparatedProductsDialogComponent
        Dialog {
            id: importSeparatedProductDialog
            parent: Overlay.overlay
            modal: true
            anchors.centerIn: parent
            width: parent.width * 0.8
            height: parent.height * 0.8
            onClosed: importSeparatedProductDialog.destroy()

            header: PrecitecApplication.DialogHeaderWithScreenshot {
                title: qsTr("Import product from removable device")
                screenshotTool: productStructurePage.screenshotTool
            }
            // workaround for https://bugreports.qt.io/browse/QTBUG-72372
            footer: DialogButtonBox {
                alignment: Qt.AlignRight
                Button {
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                }
            }

            ColumnLayout {
                anchors.fill: parent
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ListView {
                        id: availableSeparatedProductsToImportListView
                        anchors.fill: parent
                        model: productStructurePage.productController.separatedProductImportList(RemovableDevices.Service.path + "/" +
                                                                                                 Qt.application.name +
                                                                                                 RemovableDevicePaths.separatedProductsDir);
                        delegate: ItemDelegate {
                            text: modelData
                            icon.name: "cloud-upload"
                            onClicked: {
                                var sourcePath = RemovableDevices.Service.path + "/" +
                                                 Qt.application.name +
                                                 RemovableDevicePaths.separatedProductsDir +
                                                 modelData;
                                var targetPath = productStructurePage.productController.absolutePathToProduct(sourcePath + "/" +
                                                                                                                       RemovableDevicePaths.separatedProductJsonDir);
                                if (targetPath == "")
                                {
                                    productStructurePage.productController.importSeparatedProduct(sourcePath)
                                } else
                                {
                                    var dialog = overwriteProductOnDiskWithSeparatedProductDialogComponent.createObject(productStructurePage,
                                                                                                 {"sourcePath": sourcePath,
                                                                                                  "productName": modelData,
                                                                                                  "targetPath": targetPath});
                                    dialog.open();
                                }
                                importSeparatedProductDialog.close();
                            }
                         }
                     }
                }
                Label {
                    visible: availableSeparatedProductsToImportListView.count == 0
                    text: qsTr("No products found")
                }
            }
        }
    }

    contentItem: RowLayout {
        StackView {
            id: productStackView
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.topMargin: 5
            Layout.bottomMargin: 5
            clip: true
            property var productItem: null
            property var seamSeriesItem: null
            property var seamItem: null
            Connections {
                target: productStackView.currentItem
                ignoreUnknownSignals: true
                enabled: productStackView.currentItem ? (productStackView.currentItem.automaticPop != undefined ? productStackView.currentItem.automaticPop : true) : true
                function onBack() {
                    productStackView.pop()
                }
                function onBackToProductStructure() {
                    productStackView.pop(null)
                }
                function onBackToProduct() {
                    productStackView.pop(productStackView.productItem)
                }
                function onBackToSeamSeries() {
                    productStackView.pop(productStackView.seamSeriesItem)
                }
                function onBackToSeam() {
                    productStackView.pop(productStackView.seamItem)
                }
                function onSwitchToSeam(index) {
                    var newSeam = productStructurePage.currentSeamSeries.seams[index];
                    if (GuiConfiguration.seamSeriesOnProductStructure)
                    {
                        productStackView.pop(productStackView.seamSeriesItem, StackView.Immediate);
                    }
                    else
                    {
                        productStackView.pop(productStackView.productItem, StackView.Immediate);
                    }
                    productStructurePage.productController.selectSeam(newSeam.uuid);
                    productStackView.seamItem = productStackView.push(componentForSeamConfiguration(), {}, StackView.ReplaceTransition);
                }
                function onSwitchToSeamSeries(index) {
                    var newSeamSereies = productStructurePage.currentProduct.seamSeries[index];
                    productStackView.pop(productStackView.productItem, StackView.Immediate);
                    productStructurePage.productController.selectSeamSeries(newSeamSereies.uuid);
                    productStackView.seamSeriesItem = productStackView.push(seamSeriesConfiguration, {}, StackView.ReplaceTransition);
                }
                function onSwitchToLink(seam) {
                    controller.selectSeam(seam.uuid);
                    if (GuiConfiguration.seamSeriesOnProductStructure)
                    {
                        productStackView.pop(productStackView.seamSeriesItem, StackView.Immediate);
                    }
                    else
                    {
                        productStackView.pop(productStackView.productItem, StackView.Immediate);
                    }
                    productStackView.seamItem = productStackView.push(componentForSeamConfiguration(), StackView.Immediate);
                }
            }
            onCurrentItemChanged: {
                if (currentItem == initialItem)
                {
                    controller.discardCopyWhenUnmodified();
                }
            }
            initialItem: GroupBox {
                label: Label {
                    text: qsTr("Products")
                    font.family: parent.font.family
                    font.pixelSize: parent.font.pixelSize
                    font.bold: true
                    verticalAlignment: Text.AlignVCenter
                    topPadding: 10
                    leftPadding: parent.leftPadding
                }
                bottomPadding: 0

                ColumnLayout {
                    id: productsScrollView
                    anchors.fill: parent
                    ScrollView {
                        clip: true
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        ListView {
                            anchors.fill: parent
                            id: productListView
                            model: productFilterModel
                            clip: true
                            implicitHeight: childrenRect.height

                            delegate: ItemDelegate {
                                width: productsScrollView.width
                                text: model.product.type + ": " + model.display + (model.changed ? " *" : "")
                                icon.name: "application-menu"
                                onClicked: {
                                    controller.selectProduct(model.uuid);
                                    productStackView.productItem = productStackView.push(productConfiguration);
                                }
                            }
                        }
                    }
                    RowLayout {
                            Layout.alignment: Qt.AlignHCenter
                        Button {
                            objectName: "product-structure-add-new-product"
                            text: qsTr("Add new product")
                            display: AbstractButton.TextBesideIcon
                            icon.name: "list-add"
                            onClicked: {
                                var dialog = newProductDialogComponent.createObject(productStructurePage);
                                dialog.open();
                            }
                        }
                        Button {
                            objectName: "product-structure-import-product-xml"
                            text: qsTr("Import product (XML)")
                            display: AbstractButton.TextBesideIcon
                            icon.name: "cloud-upload"
                            visible: UserManagement.currentUser && UserManagement.hasPermission(App.ImportProducts)
                            enabled: RemovableDevices.Service.udi != "" && RemovableDevices.Service.path != ""
                            onClicked: {
                                var dialog = importProductXmlDialogComponent.createObject(productStructurePage);
                                dialog.open();
                            }
                        }
                        Button {
                            objectName: "product-structure-import-product-json"
                            text: qsTr("Import product (Json)")
                            display: AbstractButton.TextBesideIcon
                            icon.name: "cloud-upload"
                            visible: UserManagement.currentUser && UserManagement.hasPermission(App.ImportProducts)
                            enabled: RemovableDevices.Service.udi != "" && RemovableDevices.Service.path != "" && !productStructurePage.productController.copyInProgress
                            onClicked: {
                                var dialog = importSeparatedProductsDialogComponent.createObject(productStructurePage);
                                dialog.open();
                            }
                            BusyIndicator {
                                visible: productStructurePage.productController.copyInProgress
                                id: busyIndicator
                                anchors.fill: parent
                                running: productStructurePage.productController.copyInProgress
                            }
                        }
                    }
                }
            }
        }
        ColumnLayout {
            id: saveButtonsColumn
            visible: productStackView.currentItem ? (productStackView.currentItem.saveButtonsVisible != undefined ? productStackView.currentItem.saveButtonsVisible : true) : true
            enabled: productStackView.currentItem ? (productStackView.currentItem.saveButtonsEnabled != undefined ? productStackView.currentItem.saveButtonsEnabled : true) : true
            Layout.topMargin: spacing
            Layout.rightMargin: spacing
            ToolButton {
                id: saveButton
                objectName: "product-structure-save"
                display: AbstractButton.IconOnly
                enabled: controller.changes
                icon {
                    name: "document-save"
                    width: 64
                    height: 64
                }
                onClicked: {
                    controller.saveChanges();
                }

                ToolTip.text: qsTr("Save product changes.")
                ToolTip.visible: hovered
                ToolTip.delay: 200
                ToolTip.timeout: 5000

                Layout.alignment: Qt.AlignTop | Qt.AlignRight
            }
            Loader {
                id: additionalButtonsLoader
                sourceComponent: productStackView.currentItem.additionalButtonsComponent
                Layout.preferredWidth: item ? implicitWidth : 0
            }
            Item {
                Layout.fillHeight: true
            }
        }
    }
}
