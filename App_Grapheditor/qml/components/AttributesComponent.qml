import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication

Control {
    id: root
    property var selectedNodesModel: null
    property var selectedGroupsModel: null
    property int selectedNodesCount: 0
    property var graphVisualizer: null
    property var groupController: null
    property var attributeModel: null
    property var resultModel: null
    property var sensorModel: null
    property var errorConfigModel: null
    property var macroController: null

    function updateSelectedNodesCount()
    {
        var selectedNodes = 0;
        if (root.selectedNodesModel !== null)
        {
            selectedNodes = selectedNodes + root.selectedNodesModel.rowCount();
        }
        if (root.selectedGroupsModel !== null)
        {
            selectedNodes = selectedNodes + root.selectedGroupsModel.rowCount();
        }
        root.selectedNodesCount = selectedNodes;
    }

    Connections {
        target: selectedNodesModel
        function onModelReset()
        {
            updateSelectedNodesCount();
        }
        function onRowsInserted()
        {
            updateSelectedNodesCount();
        }
        function onRowsRemoved()
        {
            updateSelectedNodesCount();
        }
    }

    Connections {
        target: selectedGroupsModel
        function onModelReset()
        {
            updateSelectedNodesCount();
        }
        function onRowsInserted()
        {
            updateSelectedNodesCount();
        }
        function onRowsRemoved()
        {
            updateSelectedNodesCount();
        }
    }

    Label {
        visible: root.selectedNodesCount == 0
        anchors.centerIn: parent
        text: qsTr("No nodes selected")
    }
    Label {
        visible: root.selectedNodesCount > 1
        anchors.centerIn: parent
        text: qsTr("Multiple nodes selected")
    }

    Loader {
        id: mainLoader
        anchors.fill: parent
        active: root.selectedNodesCount == 1
        onActiveChanged: {
            if (active)
            {
                if (root.selectedNodesModel.rowCount() != 0)
                {
                    mainLoader.source = root.selectedNodesModel.data(root.selectedNodesModel.index(0, 0), Qt.UserRole + 1).configurationInterface
                }
                if (root.selectedGroupsModel.rowCount() != 0)
                {
                    mainLoader.source = root.selectedGroupsModel.data(root.selectedGroupsModel.index(0, 0), Qt.UserRole + 1).configurationInterface
                }
            }
        }
        onLoaded: {
            if (mainLoader.item.graphVisualizer !== undefined)
            {
                mainLoader.item.graphVisualizer = Qt.binding(function () { return root.graphVisualizer ;});
            }
            if (mainLoader.item.groupController !== undefined)
            {
                mainLoader.item.groupController = Qt.binding(function () { return root.groupController ;});
            }
            if (mainLoader.item.macroController !== undefined)
            {
                mainLoader.item.macroController = Qt.binding(function () { return root.macroController ;});
            }
            if (mainLoader.item.selectedNode !== undefined)
            {
                if (root.selectedNodesModel.rowCount() != 0)
                {
                    mainLoader.item.selectedNode = Qt.binding(function () { return root.selectedNodesModel.data(root.selectedNodesModel.index(0, 0), Qt.UserRole + 1); });
                }
                if (root.selectedGroupsModel.rowCount() != 0)
                {
                    mainLoader.item.selectedNode = Qt.binding(function () { return root.selectedGroupsModel.data(root.selectedGroupsModel.index(0, 0), Qt.UserRole + 1); });
                }
            }
            if (mainLoader.item.resultModel !== undefined)
            {
                mainLoader.item.resultModel = Qt.binding(function () { return root.resultModel ;});
            }
            if (mainLoader.item.sensorModel !== undefined)
            {
                mainLoader.item.sensorModel = Qt.binding(function () { return root.sensorModel ;});
            }
            if (mainLoader.item.errorConfigModel !== undefined)
            {
                mainLoader.item.errorConfigModel = Qt.binding(function () { return root.errorConfigModel ;});
            }
            if (mainLoader.item.attributeModel !== undefined)
            {
                mainLoader.item.attributeModel = Qt.binding(function () { return root.attributeModel ;});
            }
        }
    }
}
