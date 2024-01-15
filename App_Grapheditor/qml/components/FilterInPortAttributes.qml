import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

ColumnLayout {
    id: root
    property var selectedNode: null
    property var graphVisualizer: null

    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Label")
        TextField {
            anchors.fill: parent
            text: root.selectedNode ? root.selectedNode.label : ""
            onEditingFinished: {
                if (root.graphVisualizer)
                {
                    root.graphVisualizer.updatePortLabel(root.selectedNode, text);
                }
            }
        }
    }
    PositionGroupBox {
        selectedNode: root.selectedNode

        onPositionChanged: {
            if (root.graphVisualizer)
            {
                root.graphVisualizer.updatePortPosition(root.selectedNode, point);
            }
        }

        Layout.fillWidth: true
    }
    Item {
        Layout.fillHeight: true
    }
}
