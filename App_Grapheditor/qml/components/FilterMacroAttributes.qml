import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

ColumnLayout {
    id: root
    property var selectedNode: null
    property var macroController: null

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Label")
        TextField {
            anchors.fill: parent
            text: root.selectedNode ? root.selectedNode.label : ""
            onEditingFinished: {
                if (root.macroController)
                {
                    root.macroController.updateLabel(root.selectedNode, text);
                }
            }
        }
    }
    PositionGroupBox {
        selectedNode: root.selectedNode

        onPositionChanged: {
            if (root.macroController)
            {
                root.macroController.updatePosition(root.selectedNode, point)
            }
        }

        Layout.fillWidth: true
    }
    Item {
        Layout.fillHeight: true
    }
}
