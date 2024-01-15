import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

Menu
{
    id: filterGraphMenu
    title: "Main Menu"

    property var graphView: undefined
    property var editor: undefined
    property var pipeController: null
    property var globalPosition: undefined
    property var copyPaste: copyPasteHandler

    property var targetNode: undefined
    property var targetGroup: undefined
    property var targetEdge: undefined

    MenuItem
    {
        id: menuRemoveNode
        property int type: 0
        text: "Remove node"
        enabled: menu.targetNode !== undefined
        onTriggered:
        {
            if (menu.targetNode !== undefined)
            {
                editor.deleteObject(menu.targetNode);
            }
            menu.targetNode = undefined
        }
    }
    MenuItem
    {
        id: menuRemoveGroup
        text: "Remove group"
        enabled: menu.targetGroup !== undefined
        onTriggered:
        {
            editor.deleteGroup(menu.targetGroup);
        }
    }
    MenuItem
    {
        id: menuRemoveEdge
        text: "Remove edge"
        enabled: menu.targetEdge !== undefined
        onTriggered:
        {
            if (pipeController)
            {
                pipeController.deleteEdge(menu.targetEdge);
            }
        }
    }
    MenuItem
    {
        id: menuUngroupContent
        text: "Ungroup"
        enabled: menu.targetGroup !== undefined
        onTriggered:
        {
            editor.unGroupContent(menu.targetGroup);
        }
    }
    MenuItem
    {
        id: copy
        text: qsTr("Copy    (Ctrl + C)")
        enabled: true
        onTriggered:
        {
            copyPasteHandler.copyObjects();
        }
    }
    MenuItem
    {
        id: paste
        text: qsTr("Paste   (Ctrl + V)")
        enabled: true
        onTriggered:
        {
            copyPasteHandler.copyPosition = globalPosition;
            copyPasteHandler.pasteObjects();
        }
    }
    MenuItem
    {
        id: alignSelectedNodesHorizontal
        text: "Align horizontal"
        enabled: true
        onTriggered:
        {
            editor.graphModelVisualizer.alignSelectionHorizontal();
        }
    }
    MenuItem
    {
        id: alignSelectedNodesVertical
        text: "Align vertical"
        enabled: true
        onTriggered:
        {
            editor.graphModelVisualizer.alignSelectionVertical();
        }
    }

    CopyPasteHandler        //FIXME maybe init CopyPasteHandler in Main.qml?
    {
        id: copyPasteHandler
        filterGraph: graphView.visualizedGraph
        graphEditor: editor
    }
}
