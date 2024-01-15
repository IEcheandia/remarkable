import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

ColumnLayout {
    id: root
    property var selectedNode: null
    property var groupController: null

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Label")
        TextField {
            anchors.fill: parent
            text: root.selectedNode ? root.selectedNode.label : ""
            onEditingFinished: {
                if (root.groupController)
                {
                    root.groupController.updateGroupLabel(root.selectedNode, text);
                }
            }
        }
    }
    PositionGroupBox {
        selectedNode: root.selectedNode

        onPositionChanged: {
            if (root.groupController)
            {
                root.groupController.updateGroupPosition(root.selectedNode, point)
            }
        }

        Layout.fillWidth: true
    }
    SizeGroupBox {
        selectedNode: root.selectedNode

        onSizeChanged: {
            if (root.groupController)
            {
                root.groupController.updateGroupSize(root.selectedNode, size)
            }
        }

        Layout.fillWidth: true
    }
    Item {
        Layout.fillHeight: true
    }
}
