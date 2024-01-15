import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.4

import grapheditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.removableDevices 1.0 as RemovableDevices

Item {
    id: fileHandling
    property var graphView: null
    property var graphLoader: null
    property var graphVisualizer: null
    property var screenshotTool: null
    property var plausibilityController: null
    readonly property var directoryModel: rootDirectoryModel
    property bool creatingNewGraph: false

    function createNew()
    {
        if (graphVisualizer.graphEdited)
        {
            var newGraphDialog = newGraphComponent.createObject(fileHandling);
            newGraphDialog.open();
            return;
        }
        var dialog = createNewGraphDialogComponent.createObject(fileHandling);
        dialog.open();
    }

    function fileOpen()
    {
        var openDialog = graphLoaderDialogComponent.createObject(ApplicationWindow.window);
        openDialog.open();
    }

    function save()
    {
        var newSaveDialog = saveInterface.createObject(ApplicationWindow.window, {"graphLoader": fileHandling.graphLoader, "graphVisualizer": fileHandling.graphVisualizer, "plausibilityController": plausibilityController});
        newSaveDialog.open();
        graphVisualizer.graphEdited = false;
    }

    function saveAs()
    {
        var newSaveAsDialog = saveAsInterface.createObject(ApplicationWindow.window, {"graphLoader": fileHandling.graphLoader, "graphVisualizer": fileHandling.graphVisualizer, "directoryModel": rootDirectoryModel, "plausibilityController": plausibilityController});
        newSaveAsDialog.open();
    }

    function reload()
    {
        if (graphVisualizer.graphEdited)
        {
            var reloadDialog = reloadGraphComponent.createObject(fileHandling);
            reloadDialog.open();
            return;
        }
        graphLoader.reloadGraph()
    }

    function exportGraph()
    {
        var path = RemovableDevices.Service.path + "/weldmaster/";
        if (fileHandling.graphLoader.macro)
        {
            path = path + "macros/";
        }
        else
        {
            path = path + (fileHandling.graphLoader.isGraph ? "graphs/" : "sub_graphs/");
        }
        var newSaveDialog = saveInterface.createObject(ApplicationWindow.window, {
            "graphLoader": fileHandling.graphLoader,
            "graphVisualizer": fileHandling.graphVisualizer,
            "plausibilityController": plausibilityController,
            "exportPath": path,
            /*: title of a dialog*/
            "title": qsTr("Export graph")
        });
        newSaveDialog.open();
    }

    Component {
        id: graphLoaderDialogComponent
        GraphLoaderDialog {
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: parent.width * 0.8
            height: parent.height * 0.8

            graphView: fileHandling.graphView
            graphLoader: fileHandling.graphLoader
            graphVisualizer: fileHandling.graphVisualizer
            directoryModel: rootDirectoryModel
            screenshotTool: fileHandling.screenshotTool
        }
    }

    Component {
        id: saveInterface
        SaveGraphDialog {
            anchors.centerIn: parent
            width: parent.width * 0.4
            height: parent.height * 0.3
        }
    }

    Component {
        id: saveAsInterface
        SaveAsGraphDialog {
            anchors.centerIn: parent
            width: parent.width * 0.4
            height: parent.height * 0.5
        }
    }

    Component {
        id: reloadGraphComponent
        Dialog {
            id: reloadGraphDialog
            parent: Overlay.overlay
            anchors.centerIn: parent
            title: qsTr("Reload graph and discard unsaved changes?")
            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel

            header: PrecitecApplication.DialogHeaderWithScreenshot {
                title: reloadGraphDialog.title
                screenshotTool: fileHandling.screenshotTool
            }

            onAccepted: {
                fileHandling.graphVisualizer.clearFilterGraph();
                fileHandling.graphVisualizer.graphEdited = false;
                fileHandling.reload();
                destroy();
            }
            onRejected: destroy();
        }
    }

    Component {
        id: newGraphComponent
        Dialog {
            id: newGraphDialog
            parent: Overlay.overlay
            anchors.centerIn: parent
            title: qsTr("Create new graph and discard unsaved changes?")
            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel

            header: PrecitecApplication.DialogHeaderWithScreenshot {
                title: newGraphDialog.title
                screenshotTool: fileHandling.screenshotTool
            }

            onAccepted: {
                fileHandling.graphVisualizer.graphEdited = false;
                fileHandling.createNew();
                destroy();
            }
            onRejected: destroy();
        }
    }

    Component {
        id: createNewGraphDialogComponent
        Dialog {
            id: createNewGraphDialog
            parent: Overlay.overlay
            anchors.centerIn: parent
            //: Title of a dialog window
            title: qsTr("Select graph type")
            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel

            header: PrecitecApplication.DialogHeaderWithScreenshot {
                title: createNewGraphDialog.title
                screenshotTool: fileHandling.screenshotTool
            }

            onAccepted: {
                fileHandling.creatingNewGraph = true;
                graphVisualizer.clearFilterGraph();
                graphVisualizer.createNewGraph(graphRadioButton.checked ? GraphModelVisualizer.Graph : GraphModelVisualizer.SubGraph);
                if (graphRadioButton.checked)
                {
                    fileHandling.graphLoader.createNewGraph(graphVisualizer.getFilterGraphID());
                }
                else
                {
                    fileHandling.graphLoader.createNewSubGraph(graphVisualizer.getFilterGraphID());
                }
                fileHandling.creatingNewGraph = false;

                destroy();
            }
            onRejected: destroy();

            ColumnLayout {
                RadioButton {
                    objectName: "grapheditor-filehandling-create-new-graph-dialog-graph"
                    id: graphRadioButton
                    checked: true
                    //: Option in a radio button group
                    text: qsTr("Graph")
                }
                RadioButton {
                    objectName: "grapheditor-filehandling-create-new-graph-dialog-subgraph"
                    //: Option in a radio button group
                    text: qsTr("Sub graph")
                }
            }
        }
    }

    DirectoryLoader {
        id: directoryLoader

        Component.onCompleted: directoryLoader.loadAdditionalDirectories();
    }
    DirectoryModel {
        id: rootDirectoryModel
        loader: directoryLoader
    }

}
