import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0 as WobbleFigureEditor

/**
 * Dialog to preview a figure.
 * Requires the @link{fileModel} to be set and opens with the @link{openFigure} function
 * which takes a QModelIndex into the @link{fileModel}.Array()
 *
 * By default parented to Overlay taking 90 % of the Overlay size.
 **/
Dialog {
    id: previewDialog
    /**
     * FileModel
     **/
    property alias fileModel: previewController.fileModel

    parent: Overlay.overlay
    width: parent.width * 0.9
    height: parent.height * 0.9
    anchors.centerIn: parent
    modal: true

    /**
     * Opens the dialog and previews the figure identified by @p modelIndex.
     * @param modelIndex (QModelIndex) from @link{fileModel}
     **/
    function openFigure(modelIndex) {
        previewController.previewSeamFigure(modelIndex);
        previewDialog.open();
        figureEditor.view.fitInView();
    }

    WobbleFigureEditor.LaserPointController {
        id: laserPointController
        figure: figureEditor.actualFigure
    }

    WobbleFigureEditor.PreviewController {
        id: previewController
        laserPointController: laserPointController
    }

    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        Button {
            //: Close button of a dialog
            text: qsTr("Close")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }
    WobbleFigureEditor.WobbleFigureView {
        id: figureEditor
        anchors.fill: parent

        WobbleFigureEditor.HeatMapLegend {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.bottomMargin: 10

            height: parent.height * 0.25
            width: parent.width * 0.04
            color: "transparent"
        }
    }
}
