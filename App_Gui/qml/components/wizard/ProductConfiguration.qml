import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0
import precitec.gui.filterparametereditor 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.notifications 1.0 as Notifications

StackView {
    id: productConfiguration
    property var product: null
    property alias name: nameField.text
    property alias lenghtUnitIndex: unitCombo.currentIndex
    property string assemblyImage: product ? product.assemblyImage : ""
    property alias title: groupBoxItem.title
    property alias assemblyImagesModel: imagesView.model
    property alias leftPad: groupBoxItem.leftPadding
    property var controller
    property bool seamSeriesSupport: GuiConfiguration.seamSeriesOnProductStructure
    property alias yAxisAvailable: yAxisStartPositionSpinBox.visible
    property var qualityNormModel: null
    property var graphModel: null
    property var subGraphModel: null
    property var attributeModel: null
    property var wizardModel: null
    property var resultsConfigModel: null
    property alias unitVisible: unitCombo.visible

    property Component additionalButtonsComponent: ColumnLayout {
        Repeater {
            id: wizardButtons
            model: productConfiguration.wizardModel
            property real preferredButtonWidth: 0
            Button {
                objectName: "product-configuration-wizard-buttons-" + model.component
                display: AbstractButton.TextUnderIcon
                text: model.display
                icon.name: model.icon
                icon.color: PrecitecApplication.Settings.iconColor
                onClicked: {
                    if (model.component == WizardModel.ProductError)
                    {
                        productConfiguration.errorsSelected();
                    } else if (model.component == WizardModel.ProductCamera)
                    {
                        productConfiguration.sensorSelected();
                    } else if (model.component == WizardModel.ProductLaserControl)
                    {
                        productConfiguration.laserControlSelected();
                    } else if (model.component == WizardModel.ProductLaserWeldingMonitor)
                    {
                        productConfiguration.lwmSelected();
                    } else if (model.component == WizardModel.ProductScanTracker)
                    {
                        productConfiguration.scanTrackerSelected();
                    } else if (model.component == WizardModel.ProductColorMaps)
                    {
                        productConfiguration.colorMapsSelected();
                    } else if (model.component == WizardModel.ProductIDM)
                    {
                        productConfiguration.idmSelected();
                    } else if (model.component == WizardModel.ProductHardwareParametersOverview)
                    {
                        productConfiguration.hardwareParametersOverviewSelected();
                    } else if (model.component == WizardModel.ProductDetectionOverview)
                    {
                        productConfiguration.detecionOverviewSelected();
                    } else if (model.component == WizardModel.ProductZCollimator)
                    {
                        productConfiguration.zCollimatorSelected();
                    } else if (model.component == WizardModel.ProductScanLabScanner)
                    {
                        productConfiguration.scanlabScannerSelected();
                    }
                    else if (model.component == WizardModel.ProductScanTracker2D)
                    {
                        productConfiguration.scanTracker2DSelected();
                    }
                }
                Component.onCompleted: {
                    wizardButtons.preferredButtonWidth = Math.max(wizardButtons.preferredButtonWidth, implicitWidth)
                }
                Layout.preferredWidth: wizardButtons.preferredButtonWidth
            }
        }
    }

    signal back()
    signal backToProductStructure()
    signal sensorSelected()
    signal scanTrackerSelected()
    signal seamSelected(var uuid)
    signal seamDeleted(var uuid)
    signal seamSeriesSelected(var uuid)
    signal seamSeriesCreated()
    signal seamCreated()
    signal colorMapsSelected()
    signal errorsSelected()
    signal lwmSelected()
    signal scanlabScannerSelected()
    signal laserControlSelected()
    signal idmSelected()
    signal hardwareParametersOverviewSelected()
    signal detecionOverviewSelected()
    signal zCollimatorSelected()
    signal scanTracker2DSelected()
    signal removableDeviceCommunicationStatusChanged(bool isFinished)

    implicitWidth: Math.max(groupBoxItem.implicitWidth, assemblyImageSelector.implicitWidth)
    implicitHeight: Math.max(groupBoxItem.implicitHeight, assemblyImageSelector.implicitHeight)
    Connections {
        target: productConfiguration.controller
        function onCopyInProgressChanged() {
            if (productConfiguration.controller.copyInProgress)
            {
                productConfiguration.enabled = false;
                productConfiguration.removableDeviceCommunicationStatusChanged(false);
            } else
            {
                productConfiguration.enabled = true;
                productConfiguration.removableDeviceCommunicationStatusChanged(true);
            }
        }
    }
    ParametersExporter {
        id: productParametersExporter
        product: productConfiguration.product
        graphModel: productConfiguration.graphModel
        subGraphModel: productConfiguration.subGraphModel
        attributeModel: productConfiguration.attributeModel
        onExportingChanged:
        {
            if (productParametersExporter.exporting)
            {
                productConfiguration.enabled = false;
                productConfiguration.removableDeviceCommunicationStatusChanged(false);
            } else
            {
                productConfiguration.enabled = true;
                productConfiguration.removableDeviceCommunicationStatusChanged(true);
            }
        }
    }

    Component {
        id: discardChangesDialogComponent
        Dialog {
            id: discardChangesDialog
            property var product: null
            modal: true
            parent: Overlay.overlay
            anchors.centerIn: parent
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape

            onAccepted: {
                productConfiguration.controller.discardChanges(discardChangesDialog.product.uuid);
                productConfiguration.back();
            }

            onRejected: destroy()

            Label {
                text: qsTr("Do you really want to discard all changes for Product \"%1\"?").arg(discardChangesDialog.product ? discardChangesDialog.product.name : "")
            }
        }
    }

    Component {
        id: deleteProductDialogComponent
        Dialog {
            id: deleteProductDialog
            property var product: null
            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true
            title: qsTr("Delete product?")
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape

            onAccepted: {
                productConfiguration.controller.deleteProduct(product.uuid);
                productConfiguration.back();
                destroy();
            }
            onRejected: {
                destroy();
            }

            Label {
                text: qsTr("Do you really want to delete the product \"%1\"?\nDeleting a product cannot be undone.").arg(deleteProductDialog.product ? deleteProductDialog.product.name : "")
            }
        }
    }

    Component {
        id: overwriteSeparatedProductFolderDialogComponent
        Dialog {
            id: overwriteSeparatedProductFolderDialog
            property string targetPath: ""
            property string productName: ""
            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true
            title: qsTr("Overwrite product?")
            standardButtons: Dialog.Yes | Dialog.No

            onAccepted: {
                productConfiguration.controller.exportCurrentProductSeparately(targetPath);
                destroy();
            }
            onRejected: {
                destroy();
            }

            Label {
                text: qsTr("Warning! Product \"%1\" will be overwritten on the removable device.\n ").arg(overwriteSeparatedProductFolderDialog.productName)
            }
        }
    }

    Component {
        id: newSeamDialogComponent
        NewSeamDialog {
            onSeamCreated: productConfiguration.seamCreated()
        }
    }

    Component {
        id: newSeamSeriesDialogComponent
        NewSeamSeriesDialog {
            onSeamSeriesCreated: productConfiguration.seamSeriesCreated()
        }
    }

    Component {
        id: seamSeriesListingComponent
        MeasureTaskListing {
            title: qsTr("Seam Series")
            model: productConfiguration.product ? productConfiguration.product.seamSeries : undefined
            onSelected: productConfiguration.seamSeriesSelected(uuid)
            onDeleted: {
                controller.selectSeamSeries(uuid)
                var dialog = deleteSeamSeriesDialogComponent.createObject(productConfiguration, {"seamSeries": controller.currentSeamSeries});
                dialog.open();
            }
        }
    }

    Component {
        id: deleteSeamSeriesDialogComponent
        DeleteSeamSeriesDialog {
        }
    }

    Component {
        id: seamListingComponent
        SeamListing {
            model: productConfiguration.product ? productConfiguration.product.allSeams : undefined
            onSelected: productConfiguration.seamSelected(uuid)
            onDeleted: productConfiguration.seamDeleted(uuid)
        }
    }

    property var assemblyImageSelector: BreadCrumpGroupBox {
        id: assemblyImageSelector
        product: productConfiguration.product
        productToNextLevelEnabled: false
        title: qsTr("Select assembly image")
        onBack: productConfiguration.pop()
        onBackToProduct: productConfiguration.pop()
        onBackToProductStructure: productConfiguration.backToProductStructure()
        ScrollView {
            anchors.fill: parent
            implicitHeight: imagesView.implicitHeight
            ListView {
                id: imagesView
                anchors.fill: parent
                clip: true
                spacing: 5
                implicitHeight: count * 100 + count * spacing

                delegate: Item {
                    width: ListView.view.width
                    height: childrenRect.height

                    RowLayout {
                        width: parent.width
                        Image {
                            fillMode: Image.PreserveAspectFit
                            source: "file://" + model.path
                            Layout.preferredHeight: 100
                            Layout.preferredWidth: 100
                        }
                        Label {
                            text: model.display
                            Layout.fillWidth: true
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            if (productConfiguration.product)
                            {
                                productConfiguration.product.assemblyImage = model.fileName;
                            } else
                            {
                                productConfiguration.assemblyImage = model.fileName;
                            }
                            productConfiguration.pop();
                        }
                    }
                }
            }
        }
        Label {
            anchors.fill: parent
            visible: imagesView.count == 0 && !assemblyImagesModel.loading
            text: qsTr("No assembly images available")
        }
        BusyIndicator {
            anchors.centerIn: parent
            running: assemblyImagesModel.loading
        }
    }

    initialItem: ColumnLayout {
            BreadCrumpGroupBox {
                Layout.fillWidth: true
            id: groupBoxItem
            onBack: productConfiguration.back()
            onBackToProductStructure: productConfiguration.backToProductStructure()
            GridLayout {
                anchors.fill: parent
                columns: 3
                columnSpacing: 10
                Label {
                    text: qsTr("Name:")
                }
                TextField {
                    id: nameField
                    objectName: "product-configuration-name"
                    selectByMouse: true
                    text: productConfiguration.product ? productConfiguration.product.name : ""
                    validator: FileSystemNameValidator {
                        allowWhiteSpace: true
                    }
                    palette.text: nameField.acceptableInput ? "black" : "red"
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    onEditingFinished: {
                        if (productConfiguration.product)
                        {
                            productConfiguration.product.name = text;
                        }
                    }
                }
                Label {
                    text: qsTr("Type:")
                }
                TextField {
                    Layout.preferredWidth: 0.25 * parent.width
                    id: typeBox
                    objectName: "product-configuration-type"
                    text: productConfiguration.product ? productConfiguration.product.type : 0
                    validator: ProductTypeValidator {
                        product: productConfiguration.product
                        controller: productConfiguration.controller
                    }
                    selectByMouse: true
                    onEditingFinished: {
                        if (productConfiguration.product)
                        {
                            productConfiguration.product.type = Number.fromLocaleString(locale, typeBox.text);
                        }
                    }
                    palette.text: typeBox.acceptableInput ? "black" : "red"
                }
                Item {
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("Unit type:")
                    visible: unitCombo.visible
                }
                ComboBox {
                    Layout.preferredWidth: 0.25 * parent.width
                    id: unitCombo
                    objectName: "product-configuration-unit"
                    currentIndex: productConfiguration.product ? productConfiguration.product.lengthUnit : -1
                    model: [qsTr("Millimeter"), qsTr("Degree")]
                    onActivated: {
                        if (productConfiguration.product)
                        {
                            if (index == 0)
                            {
                                productConfiguration.product.lengthUnit = Product.Millimeter;
                            } else if (index == 1)
                            {
                                productConfiguration.product.lengthUnit = Product.Degree;
                            }
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                    visible: unitCombo.visible
                }
                Label {
                    text: qsTr("Start position of Y axis:")
                    visible: yAxisStartPositionSpinBox.visible
                }
                MilliFromMicroSpinBox {
                    Layout.preferredWidth: 0.25 * parent.width
                    id: yAxisStartPositionSpinBox
                    objectName: "product-configuration-y-axis"
                    editable: true
                    value: productConfiguration.product ? productConfiguration.product.startPositionYAxis : 0
                    from: yAxisInformation.minimumPosition
                    to: yAxisInformation.maximumPosition
                    onValueModified: {
                        if (productConfiguration.product)
                        {
                            productConfiguration.product.startPositionYAxis = value;
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                    visible: yAxisStartPositionSpinBox.visible
                }
                Label {
                    text: qsTr("Video recorder:")
                }
                ComboBox {
                    Layout.preferredWidth: 0.25 * parent.width
                    id: videoRecorderComboBox
                    objectName: "product-configuration-video-recorder"
                    model: [qsTr("Do not change"), qsTr("Enable"), qsTr("Disable")]
                    onActivated: productConfiguration.controller.setVideoRecorderEnabled(product.uuid, videoRecorderComboBox.currentIndex == 0 ? undefined : videoRecorderComboBox.currentIndex == 1)
                    Component.onCompleted: {
                        var enabled = productConfiguration.controller.isVideoRecorderEnabled(product.uuid);
                        if (enabled === undefined)
                        {
                            videoRecorderComboBox.currentIndex = 0;
                        } else if (enabled === true)
                        {
                            videoRecorderComboBox.currentIndex = 1;
                        } else
                        {
                            videoRecorderComboBox.currentIndex = 2;
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("Assembly image:")
                }
                RowLayout {
                    Layout.columnSpan: 2
                    Image {
                        asynchronous: true
                        fillMode: Image.PreserveAspectFit
                        source: productConfiguration.assemblyImage != "" ? "file://" + WeldmasterPaths.assemblyImagesDir + "/" + productConfiguration.assemblyImage : ""
                        visible: productConfiguration.assemblyImage != ""
                        Layout.preferredHeight: 100
                        Layout.preferredWidth: 100
                    }
                    Label {
                        Layout.fillWidth: true
                        text: productConfiguration.assemblyImage == "" ? qsTr("None") : productConfiguration.assemblyImage
                    }
                    ToolButton {
                        objectName: "product-configuration-assembly-image-open"
                        display: AbstractButton.IconOnly
                        icon.name: "folder-open"
                        onClicked: {
                            productConfiguration.push(assemblyImageSelector)
                        }
                    }
                    ToolButton {
                        objectName: "product-configuration-assembly-image-remove"
                        enabled: productConfiguration.assemblyImage != ""
                        display: AbstractButton.IconOnly
                        icon.name: "remove"
                        onClicked: {
                            if (productConfiguration.product)
                            {
                                productConfiguration.product.assemblyImage = "";
                            } else
                            {
                                productConfiguration.assemblyImage = "";
                            }
                        }
                    }
                }
                Label {
                    text: qsTr("Quality norm:")
                    visible: GuiConfiguration.configureThicknessOnSeam && GuiConfiguration.seamIntervalsOnProductStructure && !SystemConfiguration.SOUVIS6000_Is_PreInspection
                }
                ComboBox {
                    Layout.preferredWidth: 0.25 * parent.width
                    id: qualityNormComboBox
                    objectName: "product-configuration-quality-norm"
                    model: qualityNormModel
                    textRole: "name"
                    visible: GuiConfiguration.configureThicknessOnSeam && GuiConfiguration.seamIntervalsOnProductStructure && !SystemConfiguration.SOUVIS6000_Is_PreInspection
                    currentIndex: qualityNormModel && product ? qualityNormModel.indexForQualityNorm(product.qualityNorm).row : -1
                    onActivated: {
                        if (product)
                        {
                            product.qualityNorm = qualityNormModel.idAtIndex(qualityNormComboBox.currentIndex);
                            productConfiguration.controller.markAsChanged();
                        }
                    }
                }
                Label {
                    text: qsTr("LWM Trigger Signal:")
                    visible: SystemConfiguration.LWM40_No1_Enable
                }
                RowLayout {
                    Layout.maximumWidth: 0.25 * parent.width
                    visible: SystemConfiguration.LWM40_No1_Enable
                    ComboBox {
                        Layout.fillWidth: true
                        id: lwmSingalTypeComboBox
                    objectName: "product-configuration-lwm-trigger-signal"
                        textRole: "name"
                        model: LwmResultFilterModel {
                            id: lwmFilter
                            sourceModel: resultsConfigModel
                        }
                        currentIndex: product && product.lwmTriggerSignalType != -1 ? lwmFilter.mapFromSource(resultsConfigModel.indexForResultType(product.lwmTriggerSignalType)).row : -1

                        onActivated: {
                            if (product)
                            {
                                product.lwmTriggerSignalType = resultsConfigModel.data(lwmFilter.mapToSource(lwmFilter.index(index, 0)), Qt.DisplayRole);
                                productConfiguration.controller.markAsChanged();
                            }
                        }
                    }
                    ToolButton {
                        icon.name: "edit-undo"
                        onClicked: {
                            if (product)
                            {
                                product.lwmTriggerSignalType = -1;
                                productConfiguration.controller.markAsChanged();
                            }
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                    visible: SystemConfiguration.LWM40_No1_Enable
                }
                Label {
                    text: qsTr("LWM Trigger Threshold:")
                    visible: SystemConfiguration.LWM40_No1_Enable
                }
                TextField {
                    Layout.preferredWidth: 0.25 * parent.width
                    id: lwmSingalThresholdField
                    objectName: "product-configuration-lwm-trigger-threshold"
                    visible: SystemConfiguration.LWM40_No1_Enable
                    text: productConfiguration.product ? productConfiguration.product.lwmTriggerSignalThreshold : 0.0
                    selectByMouse: true
                    validator: DoubleValidator {}
                    onEditingFinished: {
                        if (productConfiguration.product)
                        {
                            productConfiguration.product.lwmTriggerSignalThreshold = Number.fromLocaleString(locale, lwmSingalThresholdField.text);
                            productConfiguration.controller.markAsChanged();
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                    visible: SystemConfiguration.LWM40_No1_Enable
                }
            }
        }

        Loader {
            sourceComponent: productConfiguration.seamSeriesSupport ? seamSeriesListingComponent : seamListingComponent

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Button {
                objectName: "product-configuration-add-new-seam-series"
                text: qsTr("Add new seam series")
                display: AbstractButton.TextBesideIcon
                visible: productConfiguration.seamSeriesSupport
                icon.name: "list-add"
                onClicked: {
                    var dialog = newSeamSeriesDialogComponent.createObject(productConfiguration, {"product": productConfiguration.product, "controller": productConfiguration.controller});
                    dialog.open();
                }
            }
            Button {
                objectName: "product-configuration-add-new-seam"
                text: qsTr("Add new seam")
                display: AbstractButton.TextBesideIcon
                visible: !productConfiguration.seamSeriesSupport
                icon.name: "list-add"
                onClicked: {
                    var dialog = newSeamDialogComponent.createObject(productConfiguration, {"product": productConfiguration.product, "controller": productConfiguration.controller});
                    dialog.open();
                }
            }
            Button {
                objectName: "product-configuration-discard-changes"
                text: qsTr("Discard changes")
                display: AbstractButton.TextBesideIcon
                enabled: productConfiguration.controller.changes
                icon.name: "edit-undo"
                onClicked: {
                    var dialog = discardChangesDialogComponent.createObject(productConfiguration, {"product": productConfiguration.product});
                    dialog.product = productConfiguration.product;
                    dialog.open();
                }
            }
            Button {
                display: AbstractButton.TextBesideIcon
                text: qsTr("Export parameters (Excel)")
                icon.name: "cloud-upload"
                enabled: RemovableDevices.Service.udi != "" && RemovableDevices.Service.path != "" && !productParametersExporter.exporting
                onClicked: {
                     productParametersExporter.performExport(RemovableDevices.Service.path + "/" + Qt.application.name + "/product_parameters/");
                }
                BusyIndicator {
                    visible: productParametersExporter.exporting
                    anchors.fill: parent
                    running: productParametersExporter.exporting
                }
            }
            Button {
                id: exportButton
                text: qsTr("Export product")
                icon.name: "document-export"
                enabled: RemovableDevices.Service.udi != "" && RemovableDevices.Service.path != "" && !productConfiguration.controller.copyInProgress
                onClicked: {
                    if (productConfiguration.product)
                    {
                       var targetPath = RemovableDevices.Service.path + "/" + Qt.application.name + RemovableDevicePaths.separatedProductsDir;
                       if (productConfiguration.controller.separatedProductFolderWithNameExists(targetPath + "/" + productConfiguration.product.name))
                       {
                           var dialog = overwriteSeparatedProductFolderDialogComponent.createObject(productConfiguration,
                                                                                                 {
                                                                                                  "targetPath": targetPath,
                                                                                                  "productName": productConfiguration.product.name
                                                                                                });
                            dialog.open();
                       } else
                       {
                           productConfiguration.controller.exportCurrentProductSeparately(targetPath);
                       }
                    }
                }

                BusyIndicator {
                    visible: productConfiguration.controller.copyInProgress
                    anchors.fill: parent
                    running: productConfiguration.controller.copyInProgress
                }
            }
            Button {
                objectName: "product-configuration-delete-product"
                display: AbstractButton.TextBesideIcon
                text: qsTr("Delete this product")
                icon.name: "edit-delete"
                onClicked: {
                    var dialog = deleteProductDialogComponent.createObject(productConfiguration, {"product": productConfiguration.product});
                    dialog.open();
                }
            }

            Button {
                objectName: "product-configuration-trigger-scheduler"
                display: AbstractButton.TextBesideIcon
                text: qsTr("Trigger scheduler")
                icon.name: "trigger-scheduler"
                onClicked: {
                    productConfiguration.controller.manualTrigger(product.uuid, product.type, product.name, product.filePath);
                }
            }
        }
    }
}