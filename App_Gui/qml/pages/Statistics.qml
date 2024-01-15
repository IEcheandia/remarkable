import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0
import precitec.gui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.general 1.0

SideTabView {
    id: root
    property alias productModel: productSelector.model

    property alias errorConfigModel: resultsStatisticsProductInstanceVisualization.errorConfigModel

    property var pdfFile: OnlineHelp.HasNoPdf

    model: resultsFilterModel

    enabled: !resultsStatisticsController.calculating

    ResultsStatisticsModel {
        id: resultsStatisticsModel
    }

    ResultsFilterModel {
        id: resultsFilterModel
        sourceModel: resultsStatisticsModel
        seamSeriesAvailable: GuiConfiguration.seamSeriesOnProductStructure

        onSeamSeriesAvailableChanged: {
            resultsStatisticsModel.setActiveLevel(ResultsStatisticsModel.Product);
            root.currentIndex = resultsFilterModel.mapFromSource(resultsStatisticsModel.index(resultsStatisticsModel.level, 0)).row;
        }
    }

    ResultsStatisticsController {
        id: resultsStatisticsController
        resultsStoragePath: WeldmasterPaths.resultsBaseDir
    }

    contentItem: StackLayout {
        anchors.fill: parent
        currentIndex: resultsFilterModel.mapToSource(resultsFilterModel.index(root.currentIndex, 0)).row

        ProductSelector {
            id: productSelector
            labelHeight: resultsStatisticsProductInstanceVisualization.implicitLabelHeight
            onSelectedProductChanged: {
                if (productSelector.selectedProduct != undefined)
                {
                    resultsStatisticsModel.setActiveLevel(ResultsStatisticsModel.Instance);
                    resultsStatisticsController.currentProduct = productSelector.selectedProduct
                    root.currentIndex = resultsFilterModel.mapFromSource(resultsStatisticsModel.index(resultsStatisticsModel.level, 0)).row;
                }
            }
        }

        ResultsStatisticsProductInstanceVisualization {
            id: resultsStatisticsProductInstanceVisualization

            controller: resultsStatisticsController

            onClicked: {
                if (GuiConfiguration.seamSeriesOnProductStructure)
                {
                    resultsStatisticsModel.setActiveLevel(ResultsStatisticsModel.Series);
                } else
                {
                    resultsStatisticsModel.setActiveLevel(ResultsStatisticsModel.Seams);
                    resultsStatisticsSeamsVisualization.seamSeries = productSelector.selectedProduct.findSeamSeries(0);
                }
                root.currentIndex = resultsFilterModel.mapFromSource(resultsStatisticsModel.index(resultsStatisticsModel.level, 0)).row;
            }

            BusyIndicator {
                anchors.centerIn: parent
                width: 150
                height: 150
                running: resultsStatisticsController.calculating
            }
        }

        ResultsStatisticsSeamSeriesVisualization {
            id: resultsStatisticsSeamSeriesVisualization
            controller: resultsStatisticsController
            errorConfigModel: root.errorConfigModel

            onClicked: {
                resultsStatisticsModel.setActiveLevel(ResultsStatisticsModel.Seams);
                resultsStatisticsSeamsVisualization.seamSeries = productSelector.selectedProduct.findSeamSeries(seamSeriesId);
                root.currentIndex = resultsFilterModel.mapFromSource(resultsStatisticsModel.index(resultsStatisticsModel.level, 0)).row;
            }
        }

        ResultsStatisticsSeamsVisualization {
            id: resultsStatisticsSeamsVisualization
            controller: resultsStatisticsController
            errorConfigModel: root.errorConfigModel

            onClicked: {
                resultsStatisticsModel.setActiveLevel(ResultsStatisticsModel.Seam);
                resultsStatisticsSeamVisualization.seam = productSelector.selectedProduct.findSeam(seamId);
                root.currentIndex = resultsFilterModel.mapFromSource(resultsStatisticsModel.index(resultsStatisticsModel.level, 0)).row;
            }
        }

        ResultsStatisticsSeamVisualization {
            id: resultsStatisticsSeamVisualization
            controller: resultsStatisticsController
            errorConfigModel: root.errorConfigModel

            onClicked: {
                resultsStatisticsModel.setActiveLevel(ResultsStatisticsModel.LinkedSeam);
                resultsStatisticsLinkedSeamVisualization.linkedSeam = productSelector.selectedProduct.findSeam(seamId);
                root.currentIndex = resultsFilterModel.mapFromSource(resultsStatisticsModel.index(resultsStatisticsModel.level, 0)).row;
            }
        }

        ResultsStatisticsLinkedSeamVisualization {
            id: resultsStatisticsLinkedSeamVisualization
            controller: resultsStatisticsController
            errorConfigModel: root.errorConfigModel
        }
    }
}
