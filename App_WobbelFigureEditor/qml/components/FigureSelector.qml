import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0 as WobbleFigureEditor

/**
 * Control to select a seam or wobble figure.
 **/
Control {
    id: root
    /**
     * The FileModel containing the seam and wobble figures.
     **/
    property alias fileModel: figureSortModel.sourceModel
    /**
     * The FileType to filter the FileModel.
     **/
    property alias fileType: figureSortModel.fileType
    /**
     * Optional PreviewDialog. If set a toolbutton for preview is added
     **/
    property var previewDialog: null

    /**
     * Signal gets emitted when a figure is selected
     * @param modelIndex QModelIndex of the selected figure in @link{fileModel}.
     **/
    signal fileModelIndexSelected(var modelIndex)

    /**
     * Sets the currentIndex of the internal ComboBox to the item identified by @p modelIndex
     **/
    function selectIndex(modelIndex)
    {
        var mappedIndex = figureSortModel.mapFromSource(modelIndex);
        figureCombo.currentIndex = mappedIndex.row;
    }

    WobbleFigureEditor.FileSortModel {
        id: figureSortModel
    }

    contentItem: RowLayout {
        ComboBox {
            id: figureCombo
            objectName: root.objectName + "figure-selector-combo"
            model: figureSortModel
            textRole: "name"
            onActivated: root.fileModelIndexSelected(figureSortModel.mapToSource(figureSortModel.index(index, 0)))

            Layout.fillWidth: true
        }
        ToolButton {
            objectName: root.objectName + "figure-selector-preview"
            icon.name: "quickview"
            display: ToolButton.IconOnly
            enabled: figureCombo.currentIndex != -1
            visible: root.previewDialog != null
            //: Title for a toolbutton, used as tool tip
            text: qsTr("Preview selected figure")
            onClicked: previewDialog.openFigure(figureSortModel.mapToSource(figureSortModel.index(figureCombo.currentIndex, 0)))
        }
    }
}
