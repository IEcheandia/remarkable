import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0
import precitec.gui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.general 1.0

SideTabView {
    id: simulationItem

    signal updateSettings()
    signal plotterSettingsUpdated()
    signal simulationItemEnabledChanged(bool enabled)

    property alias logModel: viewer.logModel
    property alias moduleModel: viewer.moduleModel
    property alias errorConfigModel: viewer.errorConfigModel
    property alias sensorConfigModel: viewer.sensorConfigModel
    property alias attributeModel: viewer.attributeModel
    property alias keyValueAttributeModel: viewer.keyValueAttributeModel
    property alias graphModel: viewer.graphModel
    property alias subGraphModel: viewer.subGraphModel
    property var pdfFile: stack.children[stack.currentIndex].pdfFile
    property alias resultsConfigModel: viewer.resultsConfigModel
    property alias onlineHelp: viewer.onlineHelp
    property var screenshotTool: null

    onUpdateSettings: viewer.updateSettings()

    model: ListModel {
        ListElement {
            display: qsTr("Product")
            icon: "select-product"
            enabled: true
        }
        ListElement {
            display: qsTr("Assembly")
            icon: "select-assembly"
            enabled: false
        }
        ListElement {
            display: qsTr("Simulation")
            icon: "folder-video-symbolic"
            enabled: false
        }
    }

    SimulationController {
        id: simulationController
        productModel: SimulationModule.productModel
        currentMeasureTask: SimulationModule.systemStatus.measureTaskID
        subGraphModel: simulationItem.subGraphModel
        graphModel: simulationItem.graphModel
        inspectionCmdProxy: SimulationModule.inspectionCmdProxy
        simulationCmdProxy: SimulationModule.simulationCmdProxy
        storageUpdateProxy: SimulationModule.storageUpdateProxy
    }
    Connections {
        target: SimulationModule.systemStatus
        function onParameterUpdated(measureTaskId) {
            if (visible && measureTaskId == simulationController.currentMeasureTask)
            {
                simulationController.sameFrame();
            }
        }
    }

    contentItem: StackLayout {
        id: stack
        anchors.fill: parent
        currentIndex: simulationItem.currentIndex

        ProductSelector {
            id: productSelector
            property bool recursion: false
            pdfFile: OnlineHelp.HasNoPdf
            model: SimulationModule.productModel
            onSelectedProductChanged: {
                if (recursion)
                {
                    return;
                }
                recursion = true;
                if (productSelector.selectedProduct != null)
                {
                    simulationItem.currentIndex++;
                } else if (simulationItem.currentIndex != 0)
                {
                    var reloadedProduct = SimulationModule.productModel.findProduct(simulationController.product);
                    if (reloadedProduct)
                    {
                        productSelector.selectedProduct = reloadedProduct;
                        simulationController.setProductInstance(productSelector.selectedProduct.uuid, simulationController.productInstance);
                        simulationController.jumpToCurrentFrame();
                    }
                }
                simulationItem.model.setProperty(1, "enabled", productSelector.selectedProduct != null);
                recursion = false;
            }
            Component.onCompleted: {
                productSelector.objectName = UserManagement.currentUser.name + "-simulation-product-selector";
            }
        }
        ProductInstanceSelector {
            id: instanceSelector
            pdfFile: OnlineHelp.HasNoPdf
            nioIndicator: false
            filterOnNio: false
            directory: WeldmasterPaths.videoBaseDir
            videoRecorderProxy: HardwareModule.videoRecorderProxy
            productDirectoryName: productSelector.selectedProduct ? (productSelector.selectedProduct.defaultProduct ? ProductInstanceModel.ProductName : ProductInstanceModel.Uuid) : ProductInstanceModel.Uuid
            product: productSelector.selectedProduct
            supportsDownload: true
            supportsDelete: true
            supportsUpload: UserManagement.currentUser && UserManagement.hasPermission(App.UploadVideo)
            downloadDirectoryName: "video"
            downloadPermission: App.DownloadVideo
            deletePermission: App.DeleteVideo
            monitoring: simulationItem.visible
            screenshotTool: simulationItem.screenshotTool
            onTransferInProgressChanged: {
               simulationItem.enabled = isFinished;
               simulationItem.simulationItemEnabledChanged(isFinished);
            }
            onSelectedProductInstanceChanged: {
                simulationItem.model.setProperty(2, "enabled", instanceSelector.selectedProductInstance != undefined);
                if (instanceSelector.selectedProductInstance != undefined)
                {
                    simulationController.product = productSelector.selectedProduct.uuid;
                    simulationController.setProductInstance(productSelector.selectedProduct.uuid, instanceSelector.selectedProductInstance);
                    simulationController.jumpToFrame(0);
                    if (simulationItem.currentIndex == 1)
                    {
                        simulationItem.currentIndex++;
                    }
                }
            }
            Component.onCompleted: {
                instanceSelector.objectName = UserManagement.currentUser.name + "-simulation-product-instance-selector";
            }
        }
        SimulationViewer {
            id: viewer
            pdfFile: OnlineHelp.HasNoPdf
            currentProduct: productSelector.selectedProduct
            simulationController: simulationController
            serialNumber: instanceSelector.selectedSerialNumber != undefined ? instanceSelector.selectedSerialNumber : ""
            partNumber: instanceSelector.selectedPartNumber != undefined ? instanceSelector.selectedPartNumber : ""
            screenshotTool: simulationItem.screenshotTool
            hasNextProductInstance: instanceSelector.hasNext
            hasPreviousProductInstance: instanceSelector.hasPrevious
            onPlotterSettingsUpdated: simulationItem.plotterSettingsUpdated()
            onPreviousProductInstanceSelected: instanceSelector.selectPrevious()
            onNextProductInstanceSelected: instanceSelector.selectNext()

            Component.onCompleted: {
                viewer.objectName = UserManagement.currentUser.name + "-simulation-viewer";
            }
        }
    }

}
