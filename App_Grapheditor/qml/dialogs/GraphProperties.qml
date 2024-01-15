import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

Component
{
    id: graphInterface
    Dialog
    {
        id: graphProperties
        property var filterGraph: null
        property var graphVisualizer: null
        anchors.centerIn: parent
        width: parent.width * 0.5
        height: parent.height * 0.5
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
                text: qsTr("Change graph properties")
                font.pixelSize: 18
                font.bold: true
                color: PrecitecApplication.Settings.alternateText
                horizontalAlignment: Text.AlignHCenter
            }
        }

        onAccepted:
        {
            graphProperties.filterGraph.graphName = graphNameText.text;
            graphProperties.filterGraph.graphComment = graphCommentText.text;
            graphProperties.graphVisualizer.setFilterProperties(graphNameText.text, graphCommentText.text, graphGroupText.text);
        }
        onRejected:
        {
            destroy();
        }

        GroupBox
        {
            id: graphPropertiesView
            implicitWidth: parent.width
            implicitHeight: parent.height
            ColumnLayout
            {
                id: graphPropertiesInterface
                anchors.fill: parent
                Label
                {
                    id: graphNameLabel
                    Layout.fillWidth: true
                    text: qsTr("Graph name")
                    font.pixelSize: 18
                    font.bold: true
                }
                TextField
                {
                    id: graphNameText
                    Layout.fillWidth: true
                    text: filterGraph.graphName
                    selectByMouse: true
                }
                Label {
                    Layout.fillWidth: true
                    //: Title for a text field
                    text: qsTr("Group")
                    font.pixelSize: 18
                    font.bold: true
                }
                TextField {
                    id: graphGroupText
                    objectName: "grapheditor-graphproperties-group"
                    Layout.fillWidth: true
                    text: filterGraph.groupName
                    //: Placeholder text for the Group Text Field
                    placeholderText: qsTr("Enter a group name")
                    selectByMouse: true
                }
                Label
                {
                    id: graphIDLabel
                    Layout.fillWidth: true
                    text: qsTr("Graph ID")
                    font.pixelSize: 18
                    font.bold: true
                }
                TextField
                {
                    id: graphIDText
                    Layout.fillWidth: true
                    readOnly: true
                    text: filterGraph.graphID
                }
                Label
                {
                    id: graphFullPathLabel
                    Layout.fillWidth: true
                    text: qsTr("Graph path")
                    font.pixelSize: 18
                    font.bold: true
                }
                TextField
                {
                    id: graphFullPath
                    Layout.fillWidth: true
                    readOnly: true
                    text: filterGraph.graphPath
                    selectByMouse: true
                }
                Label
                {
                    id: graphCommentLabel
                    Layout.fillWidth: true
                    text: qsTr("Graph comment")
                    font.pixelSize: 18
                    font.bold: true
                }
                ScrollView
                {
                    id: graphCommentTextView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    background: Rectangle
                    {
                        border.color: "lightgrey"
                        border.width: 1
                    }

                    TextArea
                    {
                        id: graphCommentText
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: filterGraph.graphComment
                        selectByMouse: true
                        wrapMode: TextEdit.WordWrap
                        clip: true
                    }
                }
            }
        }
    }
}

