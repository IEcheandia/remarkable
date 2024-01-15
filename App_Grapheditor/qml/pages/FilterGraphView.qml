import QtQuick 2.15
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

import QuickQanava          2.0 as Qan
import "qrc:/QuickQanava"   as Qan

import precitec.gui.components.application 1.0 as PrecitecApplication

Item
{
    property alias visualizedGraph: filterGraph
    property alias qanGraphView: graphView
    property var menu: null
    property var graphEditor: null
    property var nodeItem: null
    property var sourceConnector: null
    property var targetConnector: null
    property bool graphLoadingFinished: false
    property int leftMarginSourceObjectLabel: 0
    property int rightMarginTargetObjectLabel: 0
    property var directoryModel: null
    property var groupController: null
    property var macroController: null

    function getVisibleCenter()
    {
        var visibleRect = Qt.rect(0, 0, graphView.width, graphView.height)
        return graphView.mapToItem(graphView.containerItem, (visibleRect.left + visibleRect.right)/2, (visibleRect.top + visibleRect.bottom)/2)
    }

    id: itemFilterGraphView
    height: parent.height
    width: parent.width
    visible: true
    Qan.GraphView
    {
        id: graphView
        anchors.fill: parent
        clip: true
        zoomIncrement: 0.1
        zoomMax: 10.0
        graph: FilterGraph
        {
            id: filterGraph
            anchors.fill: parent
            objectName: "NormalGraph"
            selectionColor: PrecitecApplication.Settings.alternateBackground
            selectionWeight: 5.0

            verticalDockDelegate: FilterVerticalDock
            {
                id: filterVerticalDock
            }
            onNodeClicked:
            {
                itemFilterGraphView.nodeItem = node;
                itemFilterGraphView.sourceConnector = null;
            }
            onGroupClicked:
            {
                itemFilterGraphView.nodeItem = group;
                graphEditor.insertNewFilterVisualGroup(group, pos);
                itemFilterGraphView.sourceConnector = null;
            }
            onPortClicked:
            {
                itemFilterGraphView.sourceConnector = port;
            }
            onNodeDoubleClicked:
            {
                itemFilterGraphView.sourceConnector = null;
            }
            onNodeRightClicked:
            {
                menu.targetEdge = undefined
                menu.targetGroup = undefined
                var globalPos = node.item.mapToItem(filterGraph, pos.x, pos.y)
                menu.x = globalPos.x
                menu.y = globalPos.y
                menu.targetNode = node
                menu.open()
            }
            onGroupRightClicked:
            {
                menu.targetEdge = undefined
                menu.targetNode = undefined
                if (!group)
                {
                    return
                }
                var globalPos = group.item.mapToItem(filterGraph, pos.x, pos.y)
                menu.x = globalPos.x
                menu.y = globalPos.y
                menu.targetGroup = group
                menu.open()
            }
            onEdgeRightClicked:
            {
                menu.targetNode = undefined
                menu.targetGroup = undefined
                if (!edge)
                {
                    return
                }
                var globalPos = edge.item.mapToItem(filterGraph, pos.x, pos.y)
                menu.x = globalPos.x
                menu.y = globalPos.y
                menu.targetEdge = edge
                menu.open()
            }
            onNodeMoved:
            {
                graphVisualizer.graphEdited = true
                graphVisualizer.updatePosition(node)
            }
            onConnectorHovered:
            {
                itemFilterGraphView.targetConnector = hoveredConnector;
            }
            onGroupLabelChanged:
            {
                groupController.updateGroupLabel(group, group.label)
            }
            onGroupSizeChanged:
            {
                if (itemFilterGraphView.graphLoadingFinished)
                {
                    groupController.updateGroupContent(group)
                }
            }
            function handleGrouping(node)
            {
                graphVisualizer.graphEdited = true;
                groupController.updateGroupProperty(node);
                delayPositionTimer.node = node;
                delayPositionTimer.start();
            }
            onNodeGrouped: handleGrouping(node)
            onNodeUngrouped: handleGrouping(node)

            Timer {
                id: delayPositionTimer
                property var node: null
                repeat: false
                interval: 0
                onTriggered: {
                    graphVisualizer.updatePosition(node);
                    node = null;
                }
            }
        }
        onClicked:
        {
            menu.copyPaste.copyPosition = mapToItem(graphView.containerItem, pos.x, pos.y)
            itemFilterGraphView.sourceConnector = null;
            itemFilterGraphView.nodeItem = null;
            filterGraph.clearSelection();
            var pos3 = mapToItem(graphView.containerItem, pos.x, pos.y);    //Global position of the graph
            graphEditor.insertNewFilterVisual(pos3);
        }
        onRightClicked:
        {
            if (menu !== null)
            {
                menu.targetNode = undefined
                menu.targetGroup = undefined
                menu.targetEdge = undefined
                var viewPos = graphView.mapToItem(filterGraph, pos.x, pos.y)
                menu.x = viewPos.x;
                menu.y = viewPos.y;
                menu.globalPosition = mapToItem(graphView.containerItem, pos.x, pos.y);
                menu.open()
            }
        }
        Keys.onDeletePressed:
        {
            if (nodeItem === null)
            {
                return;
            }
            if (event.modifiers === Qt.ShiftModifier)
            {
                if (nodeItem.isGroup())
                {
                    graphEditor.unGroupContent(nodeItem);
                    nodeItem = null;
                }
                return;
            }
            graphEditor.deleteObject(nodeItem)
            nodeItem = null;
        }
        Keys.onLeftPressed:
        {
            if (nodeItem === null)
            {
                return;
            }
            nodeItem.item.x = nodeItem.item.x-1;
            graphVisualizer.updatePosition(nodeItem)
        }
        Keys.onUpPressed:
        {
            if (nodeItem === null)
            {
                return;
            }
            nodeItem.item.y = nodeItem.item.y-1;
            graphVisualizer.updatePosition(nodeItem)
        }
        Keys.onRightPressed:
        {
            if (nodeItem === null)
            {
                return;
            }
            nodeItem.item.x = nodeItem.item.x+1;
            graphVisualizer.updatePosition(nodeItem)
        }
        Keys.onDownPressed:
        {
            if (nodeItem === null)
            {
                return;
            }
            nodeItem.item.y = nodeItem.item.y+1;
            graphVisualizer.updatePosition(nodeItem)
        }
        Keys.onPressed:
        {
            if (event.modifiers === Qt.ShiftModifier)           //Check shift pressed
            {
                if (nodeItem === null)
                {
                    return;
                }
                if (event.key === 16777219)
                {
                    event.accepted = true;
                    if (nodeItem.isGroup())
                    {
                        graphEditor.unGroupContent(nodeItem);
                        nodeItem = null;
                    }
                }
                return;
            }
            if (event.modifiers === Qt.ControlModifier)
            {
                if (event.key == Qt.Key_C)
                {
                    menu.copyPaste.copyObjects();
                }
                if (event.key == Qt.Key_V)
                {
                    menu.copyPaste.pasteObjects();
                }
            }
            if (event.key == 16777219)  //Backspace key
            {
                event.accepted = true;
                if (nodeItem === null)
                {
                    return;
                }
                graphEditor.deleteObject(nodeItem)
                nodeItem = null;
            }
            if (event.key == Qt.Key_Plus)
            {
                event.accepted = true;
                graphView.zoom += graphView.zoomIncrement;
            }
            if (event.key == Qt.Key_Minus)
            {
                event.accepted = true;
                graphView.zoom -= graphView.zoomIncrement;
            }
        }

        DropArea {
            anchors.fill: parent
            function insertPort(type)
            {
                    // TODO: remove the generateNewId
                    graphEditor.generateNewID();
                    var centerPoint = filterGraphView.getVisibleCenter();
                    var pos = mapToItem(graphView.containerItem, drag.x, drag.y);
                    graphEditor.insertNewPort("New Port", -1, type, pos.x, pos.y);
            }
            onDropped: {
                if (drag.source.objectName == "grapheditor-insert-object-new-inport")
                {
                    insertPort(0);
                }
                else if (drag.source.objectName == "grapheditor-insert-object-new-outport")
                {
                    insertPort(1);
                }
                else if (drag.source.objectName == "grapheditor-insert-object-new-group")
                {
                    graphEditor.insertNewGroup(mapToItem(graphView.containerItem, drag.x, drag.y));
                }
                else if (drag.source.objectName == "grapheditor-insert-object-new-comment")
                {
                    var pos = mapToItem(graphView.containerItem, drag.x, drag.y);
                    graphEditor.insertNewComment("New Comment", -1, "", pos.x, pos.y, 100, 100);
                }
                else if (drag.source.objectName.startsWith("grapheditor-macro-list-index-"))
                {
                    macroController.insertMacro(drag.source.modelIndex, mapToItem(graphView.containerItem, drag.x, drag.y));
                }
                else
                {
                    itemFilterGraphView.sourceConnector = null;
                    itemFilterGraphView.nodeItem = null;
                    filterGraph.clearSelection();
                    var pos3 = mapToItem(graphView.containerItem, drag.x, drag.y);
                    graphEditor.insertNewFilterVisual(pos3);
                }
            }
        }
        Rectangle {
            id: selectionRectangle
            visible: selectionDragHandlerTouch.active || selectionDragHandlerMouse.active
            property point startPoint
            property vector2d translation
            color: Qt.rgba(border.color.r, border.color.g, border.color.b, 0.2)
            width: 100
            height: 100
            border {
                color: PrecitecApplication.Settings.alternateBackground
                width: 1
            }

            function updateSize()
            {
                selectionRectangle.x = (translation.x < 0) ? startPoint.x + translation.x : startPoint.x;
                selectionRectangle.y = (translation.y < 0) ? startPoint.y + translation.y : startPoint.y;
                selectionRectangle.width = Math.abs(translation.x);
                selectionRectangle.height = Math.abs(translation.y);
            }
        }

        PinchHandler {
            target: null
            enabled: !selectionDragHandlerTouch.active
            property real startZoom: 1.0
            minimumScale: 0.1
            onActiveChanged: {
                startZoom = graphView.zoom
            }
            onActiveScaleChanged: {
                graphView.zoom = startZoom * activeScale
            }
        }
        DragHandler {
            id: selectionDragHandlerMouse
            target: filterGraph
            enabled: !graphView.navigable
            acceptedDevices: PointerDevice.GenericPointer
            cursorShape: Qt.IBeamCursor
            onActiveChanged: {
                if (active)
                {
                    selectionRectangle.startPoint = selectionDragHandlerMouse.centroid.position;
                    selectionRectangle.translation = selectionDragHandlerMouse.translation;
                    selectionRectangle.updateSize();
                }
                else
                {
                    var boundary = Qt.rect(selectionRectangle.x, selectionRectangle.y, selectionRectangle.width, selectionRectangle.height);
                    filterGraph.selectNodes(graphView.mapToItem(graphView.containerItem, boundary));
                }
            }
            onTranslationChanged: {
                selectionRectangle.translation = selectionDragHandlerMouse.translation;
                selectionRectangle.updateSize();
            }
        }
        DragHandler {
            id: selectionDragHandlerTouch
            target: filterGraph
            cursorShape: Qt.IBeamCursor
            minimumPointCount: 2
            maximumPointCount: 2
            onActiveChanged: {
                if (active)
                {
                    selectionRectangle.startPoint = selectionDragHandlerTouch.centroid.position;
                    selectionRectangle.translation = selectionDragHandlerTouch.translation;
                    selectionRectangle.updateSize();
                }
                else
                {
                    var boundary = Qt.rect(selectionRectangle.x, selectionRectangle.y, selectionRectangle.width, selectionRectangle.height);
                    filterGraph.selectNodes(graphView.mapToItem(graphView.containerItem, boundary));
                }
            }
            onTranslationChanged: {
                selectionRectangle.translation = selectionDragHandlerTouch.translation;
                selectionRectangle.updateSize();
            }
        }
    }

    Label {
        text: graphView && graphView.graph ? (directoryModel ? directoryModel.nameForDirectory(graphVisualizer.getFilePath()) + " / " : "") + graphView.graph.graphName : qsTr("Graph name")
        font.bold: true

        wrapMode: Text.WrapAnywhere
        horizontalAlignment: Text.AlignHCenter
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
    }

    Pane
    {
        visible: itemFilterGraphView.sourceConnector
        opacity: 0.5
        Label
        {
            text: qsTr("Source: %1").arg(itemFilterGraphView.sourceConnector ? itemFilterGraphView.sourceConnector.label : "")
        }
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.leftMargin: 10
    }
    Pane
    {
        visible: false
        opacity: 0.5
        Label
        {
            text: qsTr("Actual object: %1 Source: %1 Target: %1").arg(graphView.zoom.toLocaleString(locale, "f", 2))
        }
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
    }
    Pane
    {
        visible: itemFilterGraphView.targetConnector
        opacity: 0.5
        Label
        {
            text: qsTr("Target: %1").arg(itemFilterGraphView.targetConnector ? itemFilterGraphView.targetConnector.label : "")
        }
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: 10
    }

}
