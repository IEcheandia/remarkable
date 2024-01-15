import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications

import wobbleFigureEditor.components 1.0

Dialog {
    id: linkProductAndSeamDialog
    property var productModel: null
    property alias screenshotTool: screenshotHeader.screenshotTool
    modal: true
    standardButtons: Dialog.Close | Dialog.Ok

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        id: screenshotHeader
        //: title for a dialog
        title: qsTr("Import seam information")
    }

    onAccepted:
    {
        importSeamDataController.importData();
        destroy();
    }
    onRejected:
    {
        destroy();
    }

    GroupBox {
        id: gridPropertiesView
        anchors.fill: parent
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 10

            Label {
                id: selectProductLabel
                Layout.fillWidth: true
                //: title for a combo box to select a product from a list
                text: qsTr("Select product:")
                font.bold: true
            }

            ComboBox {
                id: selectProduct
                objectName: "figureeditor-select-product"
                Layout.fillWidth: true
                currentIndex: -1
                model: productModel
                textRole: "display"
                onActivated: {
                    importSeamDataController.productModelIndex = productModel.mapToSource(productModel.index(selectProduct.currentIndex, 0));
                    selectSeam.currentIndex = -1;
                }
            }

            Label {
                id: selectSeamLabel
                Layout.fillWidth: true
                //: title for a combo box to select a seam from a list of the product selected before
                text: qsTr("Select seam:")
                font.bold: true
            }

            ComboBox {
                id: selectSeam
                enabled: selectProduct.currentIndex !== -1
                objectName: "figureeditor-select-seam"
                Layout.fillWidth: true
                model: importSeamDataController && importSeamDataController.seams ? importSeamDataController.seams : null
                currentIndex: -1
                textRole: "name"
                onActivated: {
                    importSeamDataController.seamListIndex = selectSeam.currentIndex;
                }
            }

            Label {
                id: importedVelocityLabel
                Layout.fillWidth: true
                //: title for a label which shows the velocity which will be imported
                text: qsTr("Importing velocity [mm/s]:")
                font.bold: true
            }

            Label {
                id: importedVelocity
                Layout.fillWidth: true
                text: importSeamDataController ? importSeamDataController.velocity.toLocaleString(locale, 'f', 3) : "0.0"
                font.bold: true
            }
        }
    }

    ImportSeamDataController {
        id: importSeamDataController
        productModel: linkProductAndSeamDialog.productModel.sourceModel
    }
}
