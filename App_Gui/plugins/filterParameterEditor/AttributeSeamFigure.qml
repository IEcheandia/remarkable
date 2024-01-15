import QtQuick 2.5
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

import wobbleFigureEditor.components 1.0 as WobbleFigureEditor

WobbleFigureEditor.FigureSelector {
    id: control
    /**
     * Emitted when the selected figure gets modified.
     **/
    signal parameterValueModified()
    property var attribute: null
    property var parameter: null
    property var defaultValue: undefined
    property int precision: attribute ? attribute.floatingPointPrecision : 3
    property int parameterValue: value
    property int from: attribute ? attribute.minValue : 0;
    property int to: attribute ? attribute.maxValue : 99;
    property int value: parameter ? parameter.value : (attribute ? attribute.defaultValue : 0);
    property bool isDefault: value == defaultValue || defaultValue === undefined
    property alias dialogVisible: previewDialog.visible

    WobbleFigureEditor.FileModel {
        id: fileModel

        Component.onCompleted: fileModel.loadFiles()
        onLoadingChanged: {
            if (fileModel.loading)
            {
                return;
            }
            var modelIndex = control.fileType == WobbleFigureEditor.FileType.Seam ? fileModel.indexForSeamFigure(control.value) : fileModel.indexForWobbleFigure(control.value);
            control.selectIndex(modelIndex);
        }
    }

    WobbleFigureEditor.PreviewDialog {
        id: previewDialog
        fileModel: fileModel
    }

    objectName: "attribute-seam-figure"
    fileModel: fileModel
    fileType: WobbleFigureEditor.FileType.Seam
    previewDialog: previewDialog

    onFileModelIndexSelected: {
        control.parameterValue = Number.parseInt(fileModel.data(modelIndex, Qt.UserRole + 3));
        control.parameterValueModified();
    }
}
