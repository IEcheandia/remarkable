import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

///FILTERCOMMENT USERINTERFACE
ColumnLayout {
    id: filterCommentSpecificAttribute
    property var selectedNode: null
    property var graphVisualizer: null
    PositionGroupBox {
        selectedNode: filterCommentSpecificAttribute.selectedNode

        onPositionChanged: {
            if (filterCommentSpecificAttribute.graphVisualizer)
            {
                filterCommentSpecificAttribute.graphVisualizer.updateCommentPosition(filterCommentSpecificAttribute.selectedNode, point)
            }
        }

        Layout.fillWidth: true
    }
    SizeGroupBox {
        selectedNode: filterCommentSpecificAttribute.selectedNode

        onSizeChanged: {
            if (filterCommentSpecificAttribute.graphVisualizer)
            {
                filterCommentSpecificAttribute.graphVisualizer.updateCommentSize(filterCommentSpecificAttribute.selectedNode, size)
            }
        }

        Layout.fillWidth: true
    }
    GroupBox {
        title: qsTr("Text")
        Layout.fillWidth: true
        Layout.fillHeight: true
        ScrollView {
            id: commentTextView
            anchors.fill: parent

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOn
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn

            TextArea {
                id: textFilterComment
                anchors.fill: parent
                selectByMouse: true
                placeholderText: qsTr("Insert your comment!")
                text: filterCommentSpecificAttribute.selectedNode ? filterCommentSpecificAttribute.selectedNode.text : ""
                onEditingFinished: {
                    if (filterCommentSpecificAttribute.graphVisualizer)
                    {
                        filterCommentSpecificAttribute.graphVisualizer.updateCommentAttributes(filterCommentSpecificAttribute.selectedNode, textFilterComment.text);
                    }
                }
            }
        }
    }
}
