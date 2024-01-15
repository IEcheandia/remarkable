import QtQuick 2.5
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQml 2.12
import Precitec.AppGui 1.0
import precitec.gui.components.plotter 1.0
import precitec.gui.components.notifications 1.0
import precitec.gui.components.application 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.userManagement 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0
import precitec.gui.components.image 1.0

SideTabView {
    id: root
    property alias productModel: productSelector.model
    property alias errorConfigModel: resultsDataSetModel.errorConfigModel
    property alias resultsConfigModel: resultsDataSetModel.resultsConfigModel
    property alias sensorConfigModel: resultsDataSetModel.sensorConfigModel
    property var pdfFile: OnlineHelp.HasNoPdf

    /**
     * Component for saving screenshots
     **/
    property var screenshotTool: null

    signal updateSettings()
    signal plotterSettingsUpdated()
    signal configureSeam(var productId, var seamSeriesId, var seamId)
    signal configureSeamSeries(var productId, var seamSeriesId)

    onUpdateSettings: plotter.updateSettings()

    model: resultsFilterModel

    ResultsModel {
        id: resultsModel
    }

    ResultsFilterModel {
        id: resultsFilterModel
        sourceModel: resultsModel
        seamSeriesAvailable: GuiConfiguration.seamSeriesOnProductStructure

        onSeamSeriesAvailableChanged: {
            resultsModel.setActiveLevel(ResultsModel.Product);
            root.currentIndex = resultsFilterModel.mapFromSource(resultsModel.index(resultsModel.level, 0)).row;
        }
    }

    ResultsDataSetModel {
        id: resultsDataSetModel
        resultsLoader: results
        currentProduct: productSelector.selectedProduct
        onSelectedImagePositionChanged: plotter.verticalMarkerPosition = resultsDataSetModel.selectedImagePosition
        onSelectedImageNumberChanged: imageSlider.value = resultsDataSetModel.selectedImageNumber
        onLastPositionChanged: {
            if (plotter.selectFirstErrorPosition() >= 0)
            {
                plotter.showErrorTab();
            }
        }
    }

    SeamSeriesResultsModel {
        id: seamSeariesDataModel
        errorConfigModel: resultsDataSetModel.errorConfigModel
        resultsConfigModel: resultsDataSetModel.resultsConfigModel
        sensorConfigModel: resultsDataSetModel.sensorConfigModel
        resultsSeriesLoader: seriesLoader
        maxSeamsLimit: GuiConfiguration.maximumNumberOfSeamsOnOverview
    }

    ErrorsDataModel {
        id: errorsDataModel
        plotterModel: resultsDataSetModel
    }

    ErrorsDataModel {
        id: seamSeriesErrorsModel
        plotterModel: seamSeariesDataModel
    }

    ResultsExporter {
        id: resultsExporterItem
        resultsConfigModel: root.resultsConfigModel
        errorConfigModel: root.errorConfigModel
        exportDirectory: RemovableDevices.Service.path != "" ? RemovableDevices.Service.path + "/weldmaster/export/" + GuiConfiguration.stationName + "/" : ""
        onExportStarted: NotificationSystem.information(qsTr("Exporting to weldmaster/export/%1/%2 on attached removable device").arg(GuiConfiguration.stationName).arg(fileName))
    }

    ResultsLoader {
        id: results
        onSeamChanged: {
            plotter.clear();
        }
    }

    ResultsSeriesLoader {
        id: seriesLoader
    }

    VideoDataLoader {
        id: videoData
        dataDirectory: WeldmasterPaths.videoBaseDir
    }

    Component {
        id: seamSelectorInPlotterComponent
        SeamSelector {
            id: seamSelectorInPlotter
            selectedProduct: productSelector.selectedProduct
            selectedSerialNumber: instanceTable.selectedSerialNumber
            selectedDate: instanceTable.selectedDate
            selectedProductInstance: seamSelector.selectedProductInstance
            selectedSeamSeries: seamSelector.selectedSeamSeries
            resultsExporter: resultsExporterItem
            labelHeight: 0
            labelVisible: false
            onSelectSeam: {
                seamSelector.selectedSeam = seamSelectorInPlotter.selectedSeam;
                seamSelector.currentIndex = seamSelectorInPlotter.currentIndex;
                seamSelector.selectSeam();
            }
            Component.onCompleted: {
                seamSeriesSelector.objectName = UserManagement.currentUser.name + "-results-product-seam-selector";
            }
        }
    }

    contentItem: StackLayout {
        anchors.fill: parent
        currentIndex: resultsFilterModel.mapToSource(resultsFilterModel.index(root.currentIndex, 0)).row

        ProductSelector {
            id: productSelector
            labelHeight: instanceTable.implicitLabelHeight
            onSelectedProductChanged: {
                if (productSelector.selectedProduct)
                {
                    resultsModel.setActiveLevel(ResultsModel.Instance);
                    seriesLoader.seamSeries = -1;
                    results.seam = -1;
                }
                else
                {
                    resultsModel.setActiveLevel(ResultsModel.Product);
                }

                // if check forces the filter model to evaluate its source model, preventing a "index from wrong model" error message when the page is created
                if (resultsModel == resultsFilterModel.sourceModel)
                {
                    root.currentIndex = resultsFilterModel.mapFromSource(resultsModel.index(resultsModel.level, 0)).row;
                }
            }
            Component.onCompleted: {
                productSelector.objectName = UserManagement.currentUser.name + "-results-product-selector";
            }
        }

        ProductInstanceTable {
            id: instanceTable

            product: productSelector.selectedProduct
            monitoring: root.visible
            resultsExporter: resultsExporterItem
            screenshotTool: root.screenshotTool

            onSelectedProductInstanceChanged: {
                if (instanceTable.selectedProductInstance != undefined)
                {
                    results.seam = -1;
                    seriesLoader.seamSeries = -1;
                    seamSeriesSelector.selectedProductInstance = instanceTable.selectedProductInstance;
                    seamSelector.selectedProductInstance = instanceTable.selectedProductInstance;
                    if (GuiConfiguration.seamSeriesOnProductStructure)
                    {
                        resultsModel.setActiveLevel(ResultsModel.Series);
                    } else
                    {
                        seamSeriesSelector.selectedSeamSeries = productSelector.selectedProduct.findSeamSeries(0);
                        seamSelector.selectedSeamSeries = productSelector.selectedProduct.findSeamSeries(0);
                        resultsModel.setActiveLevel(ResultsModel.Seam);
                    }
                    root.currentIndex = resultsFilterModel.mapFromSource(resultsModel.index(resultsModel.level, 0)).row;
                }
            }
            onSelectedSeamChanged: {
                if (instanceTable.selectedSeamSeries != null && instanceTable.selectedSeam != null)
                {
                    seamSeriesSelector.selectedSeamSeries = instanceTable.selectedSeamSeries;
                    seamSeriesSelector.selectSeries();

                    seamSelector.selectedSeam = instanceTable.selectedSeam;
                    seamSelector.currentIndex = instanceTable.seamInSeriesIndex;
                    seamSelector.selectSeam();
                    plotter.resetPlotterView();
                }
            }
            Component.onCompleted: {
                instanceTable.objectName = UserManagement.currentUser.name + "-results-product-instance-selector";
            }
        }

        SeamSeriesSelector {
            id: seamSeriesSelector
            selectedProduct: productSelector.selectedProduct
            selectedSerialNumber: instanceTable.selectedSerialNumber
            selectedDate: instanceTable.selectedDate
            selectedPartNumber: instanceTable.selectedPartNumber
            resultsExporter: resultsExporterItem
            labelHeight: instanceTable.implicitLabelHeight
            onSelectSeries: {
                if (seamSeriesSelector.selectedSeamSeries != null)
                {
                    seamSeariesDataModel.currentProduct = productSelector.selectedProduct;
                    seriesLoader.productInstance = instanceTable.selectedProductInstance;
                    seriesLoader.seamSeries = seamSeriesSelector.selectedSeamSeries.number;
                    seriesPlotter.resetPlotterView();

                    results.seam = -1;
                    seamSelector.selectedSeamSeries = seamSeriesSelector.selectedSeamSeries;
                    resultsModel.setActiveLevel(ResultsModel.Seam);
                    root.currentIndex = resultsFilterModel.mapFromSource(resultsModel.index(resultsModel.level, 0)).row;
                }
            }
            Component.onCompleted: {
                seamSeriesSelector.objectName = UserManagement.currentUser.name + "-results-product-series-selector";
            }
        }

        Item {
            SeamSelector {
                id: seamSelector
                anchors.fill: parent
                selectedProduct: productSelector.selectedProduct
                selectedSerialNumber: instanceTable.selectedSerialNumber
                selectedDate: instanceTable.selectedDate
                selectedPartNumber: instanceTable.selectedPartNumber
                resultsExporter: resultsExporterItem
                labelHeight: instanceTable.implicitLabelHeight
                visible: !GuiConfiguration.seamSeriesOnProductStructure
                onSelectSeam: {
                    results.seam = -1;
                    results.productInstance = instanceTable.selectedProductInstance;
                    results.seamSeries = seamSeriesSelector.selectedSeamSeries.number;
                    results.seam = seamSelector.selectedSeam.number;
                    videoData.update(productSelector.selectedProduct, instanceTable.selectedDirectoryName, seamSeriesSelector.selectedSeamSeries.number, seamSelector.selectedSeam.number);
                    resultsModel.setActiveLevel(ResultsModel.Results);
                    root.currentIndex = resultsFilterModel.mapFromSource(resultsModel.index(resultsModel.level, 0)).row;
                }
                Component.onCompleted: {
                    seamSeriesSelector.objectName = UserManagement.currentUser.name + "-results-product-seam-selector";
                }
            }

            ColumnLayout {
                visible: GuiConfiguration.seamSeriesOnProductStructure
                anchors.fill: parent

                Label {
                    Layout.fillWidth: true

                    text: qsTr("Select Seam of %1Product \"Series %2\" (SN: %3 from %4)").arg(seamSelector.selectedProduct ? seamSelector.selectedProduct.name : "").arg(seamSelector.selectedSeamSeries ? seamSelector.selectedSeamSeries.visualNumber : "").arg(seamSelector.selectedSerialNumber).arg(Qt.formatDateTime(seamSelector.selectedDate, "yyyy-MM-dd hh:mm:ss.zzz"))
                    font.family: resultsBox.font.family
                    font.pixelSize: resultsBox.font.pixelSize
                    font.bold: true
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: resultsBox.leftPadding
                }
                Label {
                    Layout.fillWidth: true

                    //: a label containing a part number (argument is the part number)
                    text: qsTr("Part Number: %1").arg(instanceTable.selectedPartNumber)
                    visible: instanceTable.selectedPartNumber != ""
                    font.family: resultsBox.font.family
                    font.pixelSize: resultsBox.font.pixelSize
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: resultsBox.leftPadding
                }

                PlotterComponent {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 5

                    id: seriesPlotter

                    resultsModel: seamSeariesDataModel
                    errorsModel: seamSeriesErrorsModel
                    configFilePath: GuiConfiguration.configFilePath
                    enabled: !seamSeariesDataModel.loading
                    toolTipEnabled: false

                    moveButtonsVisible: false
                    labelVisible: false
                    multipleSeamControlsVisible: false

                    onPlotterSettingsUpdated: root.plotterSettingsUpdated()

                    externalConfigureButtonVisible: UserManagement.currentUser && UserManagement.hasPermission(App.RunHardwareAndProductWizard)
                    externalConfigureButtonEnabled: instanceTable.product && !instanceTable.product.defaultProduct

                    additionalTabsModel: ["Seams"]
                    additionalTabComponentsModel: [seamSelectorInPlotterComponent]

                    onConfigure: {
                        if (seamSeriesSelector.selectedSeamSeries != null)
                        {
                            root.configureSeamSeries(seamSeriesSelector.selectedSeamSeries.product.uuid, seamSeriesSelector.selectedSeamSeries.uuid);
                        }
                    }
                }
            }
            BusyIndicator {
                running: seamSeariesDataModel.loading
                anchors.centerIn: parent
            }
        }

        SideTabView {
            id: resultsSideTabView
            model: ListModel {
                ListElement {
                    display: qsTr("Video\nPlotter")
                    icon: "view-video-plot"
                }
                ListElement {
                    display: qsTr("Video")
                    icon: "view-video"
                }
                ListElement {
                    display: qsTr("Plotter")
                    icon: "view-plot"
                }
            }
            alignment: Qt.AlignRight
            contentItem: GroupBox {
                id: resultsBox
                anchors.fill: parent

                label: ColumnLayout {
                    Label {
                        text: seamSelector.selectedSeam ? qsTr("Seam \"%1\" (#%2)").arg(seamSelector.selectedSeam.name).arg(seamSelector.selectedSeam.visualNumber) : ""
                        font.family: resultsBox.font.family
                        font.pixelSize: resultsBox.font.pixelSize
                        font.bold: true
                        verticalAlignment: Text.AlignVCenter
                        height: instanceTable.implicitLabelHeight

                        leftPadding: resultsBox.leftPadding
                    }
                    Label {
                        text: seamSeriesSelector.selectedSeamSeries ?  qsTr("Series \"%1\" (#%2)")
                            .arg(seamSeriesSelector.selectedSeamSeries.name)
                            .arg(seamSeriesSelector.selectedSeamSeries.visualNumber)  : ""
                        visible: GuiConfiguration.seamSeriesOnProductStructure
                        verticalAlignment: Text.AlignVCenter
                        height: instanceTable.implicitLabelHeight

                        leftPadding: resultsBox.leftPadding
                    }
                    Label {
                        text: productSelector.selectedProduct ?  qsTr("Product \"%1\" (SN: %2 from %3)")
                            .arg(productSelector.selectedProduct.name)
                            .arg(instanceTable.selectedSerialNumber)
                            .arg(Qt.formatDateTime(instanceTable.selectedDate, "yyyy-MM-dd hh:mm:ss.zzz")) : ""
                        verticalAlignment: Text.AlignVCenter
                        height: instanceTable.implicitLabelHeight

                        leftPadding: resultsBox.leftPadding
                    }
                    Label {
                        //: a label containing a part number (argument is the part number)
                        text: qsTr("Part Number: %1").arg(instanceTable.selectedPartNumber)
                        visible: instanceTable.selectedPartNumber != ""
                        verticalAlignment: Text.AlignVCenter
                        height: instanceTable.implicitLabelHeight

                        leftPadding: resultsBox.leftPadding
                    }
                    Label {
                        visible: seamSelector.selectedSeam && seamSelector.selectedSeam.linkTo != undefined
                        text: visible ? qsTr("Linked to seam \"%1\" (#%2)").arg(seamSelector.selectedSeam.linkTo.name).arg(seamSelector.selectedSeam.linkTo.visualNumber) : ""
                        verticalAlignment: Text.AlignVCenter
                        height: instanceTable.implicitLabelHeight

                        leftPadding: resultsBox.leftPadding
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    ColumnLayout {
                        visible: resultsSideTabView.currentIndex != 2
                        Layout.fillHeight: true
                        Layout.minimumWidth: resultsBox.width * 0.33
                        SourceImage {
                            id: image
                            source: (resultsSideTabView.currentIndex != 2 && !videoData.loading) ? videoData.getImagePath(resultsDataSetModel.selectedImageNumber) : ""
                            Layout.fillHeight: true
                            Layout.fillWidth: resultsSideTabView.currentIndex == 1
                            Layout.minimumWidth: resultsBox.width * 0.33
                            BusyIndicator {
                                anchors.centerIn: parent
                                running: videoData.loading
                            }
                        }
                        Item {
                            Layout.maximumWidth: image.width
                            Layout.fillWidth: true
                            implicitHeight: previousImageButton.implicitHeight
                            implicitWidth: previousImageButton.implicitWidth + imageSlider.implicitWidth + nextImageButton.implicitWidth
                            ToolButton {
                                id: previousImageButton
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                enabled: imageSlider.value != imageSlider.from
                                icon.name: "go-previous"
                                display: Button.IconOnly
                                onClicked: imageSlider.decrease()
                            }
                            Slider {
                                id: imageSlider
                                from: resultsDataSetModel.firstImageNumber
                                to: resultsDataSetModel.lastImageNumber
                                value: resultsDataSetModel.selectedImageNumber
                                stepSize: 1
                                snapMode: Slider.SnapOnRelease
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: previousImageButton.right
                                anchors.right: nextImageButton.left
                                onValueChanged: resultsDataSetModel.selectImageNumber(imageSlider.value)
                                onFromChanged: value = resultsDataSetModel.selectedImageNumber
                                onToChanged: value = resultsDataSetModel.selectedImageNumber
                            }
                            ToolButton {
                                id: nextImageButton
                                enabled: imageSlider.value != imageSlider.to
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                icon.name: "go-next"
                                display: Button.IconOnly
                                onClicked: imageSlider.increase()
                            }
                        }
                        Label {
                            text: qsTr("Image %1 / Position %2").arg(resultsDataSetModel.selectedImageNumber).arg(resultsDataSetModel.selectedImagePosition)
                        }
                    }
                    PlotterComponent {
                        id: plotter
                        visible: resultsSideTabView.currentIndex != 1
                        configFilePath: GuiConfiguration.configFilePath
                        resultsModel: resultsDataSetModel
                        errorsModel: errorsDataModel
                        moveButtonsVisible: true
                        labelVisible: false
                        onPlotterSettingsUpdated: root.plotterSettingsUpdated()
                        nextButtonEnabled: seamSelector.currentIndex != seamSelector.count - 1
                        previousButtonEnabled: seamSelector.currentIndex > 0
                        verticalMarkerVisible: resultsSideTabView.currentIndex == 0
                        verticalMarkerPosition: resultsDataSetModel.selectedImagePosition

                        externalConfigureButtonVisible: UserManagement.currentUser && UserManagement.hasPermission(App.RunHardwareAndProductWizard)
                        externalConfigureButtonEnabled: instanceTable.product && !instanceTable.product.defaultProduct

                        onGoPrevious : seamSelector.selectPrevious()
                        onGoNext: seamSelector.selectNext()
                        onConfigure: {
                            var seam = instanceTable.product.findSeam(results.seamSeries, results.seam);
                            if (seam != null)
                            {
                                root.configureSeam(seam.seamSeries.product.uuid, seam.seamSeries.uuid, seam.uuid);
                            }
                        }
                        onVerticalMarkerPositionChanged: resultsDataSetModel.selectImagePosition(plotter.verticalMarkerPosition)

                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
