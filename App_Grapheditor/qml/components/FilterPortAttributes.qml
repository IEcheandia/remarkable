import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

ColumnLayout {
    id: filterPortAttributeItem
    property var selectedNode: null
    property var graphVisualizer: null

    Label {
        Layout.fillWidth: true
        font.bold: true
        text: filterPortAttributeItem.selectedNode ? filterPortAttributeItem.selectedNode.label : ""
    }
    PositionGroupBox {
        selectedNode: filterPortAttributeItem.selectedNode

        onPositionChanged: {
            if (filterPortAttributeItem.graphVisualizer)
            {
                filterPortAttributeItem.graphVisualizer.updatePortPosition(filterPortAttributeItem.selectedNode, point);
            }
        }

        Layout.fillWidth: true
    }
    ComboBox {
        id: setPortPartner
        Layout.fillWidth: true
        textRole: "name"
        model: filterPortSortModel
        onActivated: filterPortAttributeItem.graphVisualizer.updatePortAttributes(filterPortAttributeItem.selectedNode, filterPortModelAll.getPort(filterPortSortModel.mapToSource(filterPortSortModel.index(index, 0))))
        // TODO: select correct item on load
    }

    Item {
        Layout.fillHeight: true
    }

    FilterPortModel {
        id: filterPortModelAll
        graphModelVisualizer: filterPortAttributeItem.graphVisualizer
    }

    FilterPortSortModel {
        id: filterPortSortModel
        sourceModel: filterPortModelAll
        searchType: 2
        searchUUID: root.selectedNode && root.selectedNode.getFilterPortPartnerQObject() ? root.selectedNode.getFilterPortPartnerQObject().ID : ""
    }
}
