import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import grapheditor.components 1.0

Component
{
    id: searchObjectInterface
    Dialog
    {
        id: searchObjectDialog

        property alias nodeModel: nodeSortModel.sourceModel
        property var navigable: undefined
        property var actualNode: null

        anchors.centerIn: parent
        width: parent.width * 0.5
        height: parent.height * 0.75
        modal: true
        standardButtons: Dialog.Close | Dialog.Ok
        header: Control
        {
            padding: 5
            background: Rectangle
            {
                color: PrecitecApplication.Settings.alternateBackground
                height: 40
            }
            contentItem: Label
            {
                Layout.fillWidth: true
                text: qsTr("Search object by ...!")
                font.pixelSize: 18
                font.bold: true
                color: PrecitecApplication.Settings.alternateText
                horizontalAlignment: Text.AlignHCenter
            }
        }

        onAccepted:
        {
            navigable.graph.clearSelection();
            if (actualNode === null)
            {
                return;
            }
            if (actualNode.group)   //alternative isGroup()
            {
                navigable.centerOn(actualNode.group.item); // Focus the group is working
                navigable.graph.selectNode(actualNode);
            }
            else
            {
                navigable.centerOn(actualNode.item);
                navigable.graph.selectNode(actualNode);
            }
        }
        onRejected:
        {
            destroy();
        }

        GroupBox
        {
            id: searchObjectContent
            implicitWidth: parent.width
            implicitHeight: parent.height
            ColumnLayout
            {
                id: contentLayout
                anchors.fill: parent

                Label
                {
                    id: searchFieldLabel
                    text: "Search any node with its name, object/node type, type, group or ID."
                    Layout.fillWidth: true
                    Layout.preferredHeight: 25
                    Layout.leftMargin: 9
                    verticalAlignment: Text.AlignVCenter
                    font.bold: true
                }
                TextField
                {
                    id: labelSearchField
                    placeholderText: qsTr("Search any node...")
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    selectByMouse: true
                }
                Rectangle
                {
                    id: searchProperties
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    color: "lightgrey"
                    border.width: 1
                    border.color: "grey"
                    radius: 5

                    RowLayout
                    {
                        id: searchPropertiesInterface
                        anchors.fill: parent

                        CheckBox
                        {
                            id: searchFilter
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            text: "Search in filters"
                            checked: true
                            onClicked:
                            {
                                nodeSortModel.searchFilter = searchFilter.checked
                            }
                        }
                        CheckBox
                        {
                            id: searchPort
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                            text: "Search in ports"
                            checked: true
                            onClicked:
                            {
                                nodeSortModel.searchPort = searchPort.checked
                            }
                        }
                        CheckBox
                        {
                            id: searchComment
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            text: "Search in comments"
                            checked: true
                            onClicked:
                            {
                                nodeSortModel.searchComment = searchComment.checked
                            }
                        }
                        CheckBox
                        {
                            id: searchGroup
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                            text: "Search in groups"
                            checked: true
                            onClicked:
                            {
                                nodeSortModel.searchGroup = searchGroup.checked
                            }
                        }
                    }
                }
                ScrollView
                {
                    id: allNodeScrollView
                    implicitWidth: parent.width
                    Layout.fillHeight: true
                    ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    clip: true

                    ListView
                    {
                        id: nodeShowSearching
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        model: nodeSortModel
                        spacing: 5
                        currentIndex: -1
                        delegate: ItemDelegate
                        {
                            id: nodeSelectionButton
                            width: ListView.view.width - 15
                            height: 50
                            checkable: true
                            padding: 1
                            contentItem: RowLayout
                            {
                                id: rowLayout
                                Label
                                {
                                    id: nodeLabels
                                    Layout.preferredWidth: rowLayout.width/2
                                    Layout.leftMargin: 9
                                    text: model.name
                                    verticalAlignment: Text.AlignVCenter
                                    wrapMode: Text.WordWrap
                                    font.bold: true
                                }
                                Rectangle
                                {
                                    id: leftBorder
                                    width: 1
                                    Layout.fillHeight: true
                                    color: "darkgrey"
                                }
                                Label
                                {
                                    id: nodeType
                                    Layout.preferredWidth: rowLayout.width/7
                                    text: model.nodeType
                                    verticalAlignment: Text.AlignVCenter
                                }
                                Rectangle
                                {
                                    id: middleBorder
                                    width: 1
                                    Layout.fillHeight: true
                                    color: "darkgrey"
                                }
                                Label
                                {
                                    id: type
                                    Layout.preferredWidth: rowLayout.width/4
                                    text: model.type
                                    verticalAlignment: Text.AlignVCenter
                                }
                                Rectangle
                                {
                                    id: rightBorder
                                    width: 1
                                    Layout.fillHeight: true
                                    color: "darkgrey"
                                }
                                Label
                                {
                                    id: group
                                    Layout.preferredWidth: rowLayout.width/8
                                    Layout.rightMargin: 1
                                    text: model.group
                                    verticalAlignment: Text.AlignVCenter
                                }
                            }
                            background: Rectangle
                            {
                                id: itemDelegateBackground
                                color: "transparent"
                                border.width: 1
                                border.color: "black"
                                radius: 3
                            }
                            onClicked:
                            {
                                actualNode = nodeModel.getClickedNode(nodeSortModel.mapToSource(nodeSortModel.index(model.index,0)));
                                nodeShowSearching.currentIndex = index
                            }
                        }
                        highlight: Rectangle { color: "lightsteelblue"; radius: 3 }
                    }
                }
            }
        }
        NodeSortModel
        {
            id: nodeSortModel
            searchText: labelSearchField.text
        }
    }
}

