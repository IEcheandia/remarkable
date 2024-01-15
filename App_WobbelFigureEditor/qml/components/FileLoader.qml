import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.components.application 1.0 as PrecitecApplication

import wobbleFigureEditor.components 1.0
import "utils.js" as Utils

Item {
    id: fileLoaderModule
    property var fileModel: null
    property var outFileModel: null
    property var fileEditor: null
    property var simulationController: null
    property var screenshotTool: null
    property var fileAndFigure: null
    property var commandManager: null
    property var dxfImportDialog: null

    function createNew() {
        showsFiles.currentIndex = -1
    }
    anchors.fill: parent

    ListView {
        id: showsFiles
        anchors.fill: parent

        flickableDirection: Flickable.VerticalFlick
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {
            active: true
        }
        clip: true
        model: filesSortModel
        currentIndex: -1
        delegate: ItemDelegate {
            visible: !fileModel.loading
            width: ListView.view.width
            id: fileSelectionButton
            checkable: true
            text: model.name
            onClicked:
            {
                if (simulationController.simulationMode)
                {
                    simulationController.simulationMode = false;
                }

                if (dxfImportDialog)
                { // this is an import dialog (and not an ordinary "open file" dialog)
                    if (model.type == WobbleFigureEditor.FileType.Dxf)
                    { // a DXF file was selected, we have to run the UI for its import
                        commandManager.clearStack()
                        fileAndFigure.close()

                        dxfImportDialog.importController.dxfPath = ""
                        dxfImportDialog.importController.dxfPath = model.path
                        dxfImportDialog.uiFilename = model.fileName
                        dxfImportDialog.open()
                        return
                    }
                }

                let outModel = outFileModel || fileModel
                outModel.fileType = model.type;
                FigureEditorSettings.fileType = model.type;
                if (!Utils.checkLoadingFeedback(outModel.loadJsonFromFile(model.fileName)))
                {
                    return
                }

                commandManager.clearStack()
                fileAndFigure.close()
            }
        }

        section.property: "typeName"
        section.criteria: ViewSection.FullString
        section.delegate: sectionHeading
    }

    FileSortModel {
        id: filesSortModel
        sourceModel: fileModel
        scanMasterMode: FigureEditorSettings.scanMasterMode
    }

    Component {
        id: sectionHeading
        Label {
            text: section
            font.bold: true
            font.pixelSize: 16
        }
    }
}
