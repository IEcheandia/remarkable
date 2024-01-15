import QtQuick 2.5
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import precitec.gui 1.0
import Precitec.AppGui 1.0
/**
 * A GroupBox providing the possibility to select a Product.
 **/
GroupBox {
    id: root
    /**
     * The model of products. Expects to roles:
     * @li display (name of product)
     * @li product (precitec::storage::Product*)
     **/
    property alias model: productFilterModel.sourceModel
    /**
     * The currently selected Product
     **/
    property var selectedProduct: null
    /**
     * PDF file name for online help
     **/
    property var pdfFile: OnlineHelp.HasNoPdf

    property alias labelHeight: titleLabel.height

    label: Label {
        id: titleLabel
        text: qsTr("Select Product")
        font.family: root.font.family
        font.pixelSize: root.font.pixelSize
        font.bold: true
        verticalAlignment: Text.AlignVCenter

        leftPadding: root.leftPadding
    }

    ProductFilterModel {
        id: productFilterModel
        includeDefaultProduct: true
    }

    ScrollView {
        anchors.fill: parent
        ListView {
            id: productSelector
            objectName: root.objectName + "-listview"
            model: productFilterModel
            spacing: 5
            clip: true

            delegate: ItemDelegate {
                objectName: root.objectName + "-item-" + model.uuid
                width: ListView.view.width
                text: model.product.type > 0 ? model.product.type + ": " + model.display : model.display
                onClicked: {
                    root.selectedProduct = model.product;
                }
            }
        }
    }
}
