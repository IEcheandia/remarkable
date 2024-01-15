import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0

Item {
    id: graphLoaderModul
    property var graphView: null
    property var graphLoader: null
    property var graphVisualizer: null
    property string actualGraphName: ""
    property var window: null
    property var directoryModel: null

    property var lastIndex: null
    signal changeGraph()
    signal graphSelected()

    function handleLoadGraph(index)
    {
        const directoryIndex = directoryModel.index(currentFolder.currentIndex, 0);

        graphLoaderModul.graphLoader.loadGraph(directoryIndex, filterGraphFilterModel.data(filterGraphFilterModel.index(index, 0), Qt.UserRole));

        graphLoaderModul.graphSelected();
    }

    ColumnLayout {
        id: layoutGraphLoader
        anchors.fill: parent
        anchors.margins: 5

        ColumnLayout {
            id: layoutForLoadAndExportGraph
            Layout.fillHeight: true
            Layout.fillWidth: true

            ComboBox {
                id: currentFolder
                Layout.fillWidth: true
                Layout.rightMargin: 5
                model: directoryModel
                textRole: "name"
                currentIndex: 0
            }

            TextField {
                id: searchGraph
                placeholderText: qsTr("Search graph ...")
                Layout.fillWidth: true
                Layout.rightMargin: 5
            }
            ListView {
                id: listviewGraph
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.rightMargin: 5
                clip: true
                model: filterGraphFilterModel
                currentIndex: -1
                delegate: ItemDelegate {
                    width: ListView.view.width
                    id: graphSelectionButton
                    enabled: !filterGraphFilterModel.sourceModel.loading
                    checkable: true
                    text: model.name
                    onClicked:
                    {
                        graphView.nodeItem = null;
                        if (!graphVisualizer.graphEdited)
                        {
                            handleLoadGraph(model.index);
                            return;
                        }
                        graphLoaderModul.lastIndex = model.index;
                        var changeGraph = changeGraphDialogComponent.createObject(window);
                        changeGraph.open();
                    }
                }
                highlight: Rectangle {
                    color: "lightsteelblue"
                    radius: 3
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: filterGraphFilterModel.sourceModel.loading
                }
            }
        }
    }

    FilterGraphFilterModel {
        id: filterGraphFilterModel
        sourceModel: directoryModel.data(directoryModel.index(currentFolder.currentIndex, 0), Qt.UserRole + 3)
        searchText: searchGraph.text
    }

    Component {
        id: changeGraphDialogComponent
        ChangeGraphDialog {
            id: changeGraphDialog

            anchors.centerIn: parent
            width: parent.width * 0.2
            height: parent.height * 0.3

            onAccepted: graphLoaderModul.changeGraph()
            onClosed: destroy()
        }
    }

    onChangeGraph:
    {
        //TODO use a own QObject, a GraphLoaderController!
        graphVisualizer.clearFilterGraph();
        graphVisualizer.graphEdited = false;
        handleLoadGraph(graphLoaderModul.lastIndex);
    }
}
