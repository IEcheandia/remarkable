import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import precitec.gui.components.application 1.0
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.userManagement 1.0
import Precitec.AppGui 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0

SideTabView {
    id: overview
    objectName: "page-overview"
    property var recorderServer
    property var pdfFile: OnlineHelp.HasNoPdf
    property alias imageVisible: image.visible
    property alias resultsConfigModel: latestResults.resultsConfigModel
    property alias errorConfigModel: latestResults.errorConfigModel
    property alias sensorConfigModel: latestResults.sensorConfigModel
    property alias inputEnabled: plotter.inputEnabled
    property alias screenshotTool: image.screenshotTool

    signal updateSettings()
    signal plotterSettingsUpdated()
    signal configureSeam(var productId, var seamSeriesId, var seamId)

    onUpdateSettings: plotter.updateSettings()

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
        ListElement {
            display: qsTr("Assembly image")
            icon: "view-assembly-image"
        }
        ListElement {
            display: qsTr("Result history")
            icon: "clock"
        }
    }

    contentItem: Item {
        Connections {
            target: HardwareModule.recorder
            function onSampleData(sensorId, data, context) {
                latestResults.addSample(sensorId, data, context)
            }
        }

        LatestInstanceModel {
            id: latestResults
            resultsServer: HardwareModule.results
            liveUpdate: overview.visible
            maxSeamsLimit: GuiConfiguration.maximumNumberOfSeamsOnOverview
            numberOfSeamsInPlotter: GuiConfiguration.numberOfSeamsInPlotter
            extendedProductInfoHelper {
                serialNumberFromExtendedProductInfo: GuiConfiguration.serialNumberFromExtendedProductInfo != 0
                partNumberFromExtendedProductInfo: GuiConfiguration.partNumberFromExtendedProductInfo != 0
            }
        }
        Binding {
            target: latestResults.extendedProductInfoHelper
            property: "serialNumberFromExtendedProductInfoField"
            value: GuiConfiguration.serialNumberFromExtendedProductInfo - 1
            when: GuiConfiguration.serialNumberFromExtendedProductInfo != 0
        }

        Binding {
            target: latestResults.extendedProductInfoHelper
            property: "partNumberFromExtendedProductInfoField"
            value: GuiConfiguration.partNumberFromExtendedProductInfo - 1
            when: GuiConfiguration.partNumberFromExtendedProductInfo != 0
        }
        LatestProductErrorsModel {
            id: latestProductErrors
            resultsServer: HardwareModule.results
            liveUpdate: overview.visible
            errorConfigModel: overview.errorConfigModel
        }
        HistoryModel {
            id: historyModel
            resultsServer: HardwareModule.results
            max: 10
            extendedProductInfoHelper {
                serialNumberFromExtendedProductInfo: latestResults.extendedProductInfoHelper.serialNumberFromExtendedProductInfo
                serialNumberFromExtendedProductInfoField: latestResults.extendedProductInfoHelper.serialNumberFromExtendedProductInfoField
                partNumberFromExtendedProductInfo: latestResults.extendedProductInfoHelper.partNumberFromExtendedProductInfo
                partNumberFromExtendedProductInfoField: latestResults.extendedProductInfoHelper.partNumberFromExtendedProductInfoField
            }
        }

        anchors.fill: parent
        RowLayout {
            id: mainPageRowLayout
            visible: overview.currentIndex == 0 || overview.currentIndex == 1 || overview.currentIndex == 2
            anchors.fill: parent
            anchors.margins: spacing
            PrecitecImage.Image {
                id: image
                visible: overview.currentIndex == 0 || overview.currentIndex == 1 || overview.currentIndex == 3
                imageOverlayEnabled: false
                gridOverlayEnabled: false
                textOverlayEnabled: false

                debugMeasurement: SystemConfiguration.HardwareCameraEnabled

                Layout.fillHeight: true
                Layout.fillWidth: overview.currentIndex == 1
                Layout.minimumWidth: parent.width * 0.33
            }
            PlotterComponent {
                Layout.preferredHeight: overview.height - 2 * mainPageRowLayout.spacing
                Layout.fillWidth: true

                id: plotter

                visible: overview.currentIndex == 0 || overview.currentIndex == 2 || overview.currentIndex == 3
                resultsModel: latestResults
                errorsModel: latestProductErrors
                configFilePath: GuiConfiguration.configFilePath

                onPlotterSettingsUpdated: overview.plotterSettingsUpdated()
                moveButtonsVisible: true
                multipleSeamControlsVisible: true

                nextButtonEnabled: latestResults.currentIndex < latestResults.maxIndex
                previousButtonEnabled: latestResults.currentIndex > 0

                externalConfigureButtonVisible: UserManagement.currentUser && UserManagement.hasPermission(App.RunHardwareAndProductWizard)
                externalConfigureButtonEnabled: !latestResults.defaultProduct && latestResults.currentIndex >= 0

                onGoNext: {
                    latestResults.next();
                }
                onGoPrevious: {
                    latestResults.previous();
                }
                onConfigure: {
                    var seam = latestResults.currentSeam();
                    if (seam != null)
                    {
                        overview.configureSeam(seam.seamSeries.product.uuid, seam.seamSeries.uuid, seam.uuid);
                    }
                }
                Connections {
                    target: HardwareModule.results
                    function onProductInspectionStarted() {
                        plotter.clear();
                    }
                }
            }
        }
        AssemblyImage {
            id: assemblyImage
            visible: overview.currentIndex == 3
            resultsServer: HardwareModule.results
            assemblyImagesDirectory: WeldmasterPaths.assemblyImagesDir
            anchors.fill: parent
        }
        History {
            id: historyPage
            visible: overview.currentIndex == 4
            historyModel: historyModel
            anchors.fill: parent
            anchors.margins: 20
        }
    }
}
