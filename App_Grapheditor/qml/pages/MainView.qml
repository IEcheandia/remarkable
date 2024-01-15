import QtQuick 2.15
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0 as PrecitecApplication

import Precitec.AppGui 1.0
import precitec.gui.general 1.0
import grapheditor.components 1.0

Item {
    id: graphEditorPage
    property alias attributeModel: graphAndAttributeComponent.attributeModel
    property alias resultModel: graphAndAttributeComponent.resultSettingModel
    property alias sensorModel: graphAndAttributeComponent.sensorSettingModel
    property alias errorConfigModel: graphAndAttributeComponent.errorConfigModel
    property alias screenshotTool: fileHandling.screenshotTool
    property alias onlineHelp: insertObjects.onlineHelp

    GraphFilterModel {
        id: graphFilterModel
        filterImagePath: WeldmasterPaths.filterPictureDir
        pdfFilesDir: WeldmasterPaths.pdfFilesDir
        Component.onCompleted: {
            graphFilterModel.init(WeldmasterPaths.filterLibDir)
        }
    }

    FilterSortModel {
        id: filterSortModel
        sourceModel: graphFilterModel
        searchText: insertObjects.searchFieldText
        excludeBridges: rootGraphLoader.isGraph
    }

    GraphLoader {
        id: rootGraphLoader
        directoryModel: fileHandling.directoryModel
    }

    GraphModel {
        id: rootMacroGraphModel
        pdfFilesDir: WeldmasterPaths.pdfFilesDir
        Component.onCompleted: {
            rootMacroGraphModel.loadGraphs(WeldmasterPaths.systemMacroDir, WeldmasterPaths.userMacroDir)
        }
    }

    GraphModelVisualizer {
        id: graphVisualizer
        graphLoader: rootGraphLoader
        filterGraph: filterGraphView.visualizedGraph
        graphFilterModel: graphFilterModel
        attributeModel: graphEditorPage.attributeModel
        graphView: filterGraphView.qanGraphView
        macroController: macroController
        pipeController: pipeController
    }

    Connections {
        target: graphFilterModel
        function onReady()
        {
            graphVisualizer.startUp();
            rootGraphLoader.createNewGraph(graphVisualizer.getFilterGraphID());
            insertObjects.enabled = true;
            graphAndAttributeComponent.enabled = true;
            graphChangedConnection.enabled = Qt.binding(function() { return !fileHandling.creatingNewGraph; });
        }
    }

    Connections {
        target: graphVisualizer
        function onNewObjectInserted(object) {
            filterGraphView.visualizedGraph.clearSelection();
            filterGraphView.visualizedGraph.selectNode(object);
        }
    }

    FilterGraphImageSaver {
        id: graphImageSaver
        graphView: filterGraphView.qanGraphView
    }

    GraphEditor {
        id: graphEditor
        graphModelVisualizer: graphVisualizer
        pipeController: pipeController
    }

    PipeController {
        id: pipeController
        graph: graphVisualizer.graph
        filterGraph: filterGraphView.visualizedGraph
        zoom: filterGraphView.qanGraphView.zoom
        onChanged: {
            graphVisualizer.graphEdited = true;
        }
    }

    MacroController {
        id: macroController
        filterGraph: filterGraphView.visualizedGraph
        gridSize: graphVisualizer.gridSize
        useGridSizeAutomatically: graphVisualizer.useGridSizeAutomatically
        graph: graphVisualizer.graph
        groupController: graphVisualizer.groupController
        macroModel: rootMacroGraphModel

        onChanged: {
            graphVisualizer.graphEdited = true;
        }
    }

    PlausibilityController {
        id: plausibilityController
        filterGraph: filterGraphView.visualizedGraph
        graph: graphVisualizer.graph
    }

    Connections {
        id: graphChangedConnection
        target: rootGraphLoader
        enabled: false
        function onGraphChanged()
        {
            filterGraphView.graphLoadingFinished = false;
            graphVisualizer.initGraph();
            filterGraphView.graphLoadingFinished = true;
            filterGraphView.qanGraphView.zoom = 2.0;
            filterGraphView.qanGraphView.fitInView();
        }
    }

    FilterGroupModel {
        id: filterGroups
        graphModelVisualizer: graphVisualizer
    }

    NodeModel {
        id: allNodesModel
        graphModelVisualizer: graphVisualizer
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ButtonLayout {
            id: buttons

            graphView: filterGraphView.qanGraphView
            graphEditor: graphEditor
            nodesModel: allNodesModel
            graphImageSaver: graphImageSaver
            documentModified: graphVisualizer.graphEdited && !graphVisualizer.graphNameEdited
            graphIsMacro: rootGraphLoader.macro
            groupController: graphVisualizer.groupController
            plausibilityController: plausibilityController

            onNewGraph: fileHandling.createNew()
            onOpen: fileHandling.fileOpen()
            onSave: fileHandling.save()
            onSaveAs: fileHandling.saveAs()
            onReload: fileHandling.reload()
            onExportGraph: fileHandling.exportGraph()

            Layout.fillWidth: true
        }
        RowLayout {
            spacing: 0
            InsertObjects {
                id: insertObjects

                graphEditorObject: graphEditor
                filterGraphView: filterGraphView
                macroGraphModel: rootMacroGraphModel
                macroController: macroController
                graphIsMacro: rootGraphLoader.macro
                Layout.fillHeight: true
                Layout.preferredWidth: hidden ? minimumWidth + 10 : 0.2 * graphEditorPage.width

                Behavior on Layout.preferredWidth {
                    NumberAnimation {}
                }
            }

            Rectangle {
                color: PrecitecApplication.Settings.alternateBackground
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                Layout.maximumWidth: 1
            }

            FilterGraphView {
                id: filterGraphView
                menu: filterGraphMenu
                graphEditor: graphEditor
                leftMarginSourceObjectLabel: insertObjects.width
                rightMarginTargetObjectLabel: graphAndAttributeComponent.width
                directoryModel: fileHandling.directoryModel
                groupController: graphVisualizer.groupController
                macroController: macroController
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            Rectangle {
                color: PrecitecApplication.Settings.alternateBackground
                Layout.fillHeight: true
                Layout.preferredWidth: 1
                Layout.maximumWidth: 1
            }

            GraphAndAttribute {
                id: graphAndAttributeComponent
                enabled: false
                rootGraphVisualizer: graphVisualizer
                groupController: graphVisualizer.groupController
                filterGraphViewer: filterGraphView
                macroController: macroController

                Layout.fillHeight: true
                Layout.preferredWidth: hidden ? minimumWidth + 10 : 0.2 * graphEditorPage.width

                Behavior on Layout.preferredWidth {
                    NumberAnimation {}
                }
            }
        }
    }

    FileHandling {
        id: fileHandling

        graphView: filterGraphView
        graphLoader: rootGraphLoader
        graphVisualizer: graphVisualizer
        plausibilityController: plausibilityController
    }

    GraphEditorMenu {
        id: filterGraphMenu
        graphView: filterGraphView
        editor: graphEditor
        pipeController: pipeController
    }

    Component.onCompleted: {
        graphVisualizer.groupController.plausibilityController = Qt.binding(function() { return plausibilityController; });
    }
}

