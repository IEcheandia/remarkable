import QtQuick              2.8
import QtQuick.Controls     2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts      1.3
import QtGraphicalEffects   1.0

import QuickQanava 2.0 as Qan

Qan.GroupItem {
    id: filterGroup

    default property alias children : template
    container: template.content   // See qan::GroupItem::container property documentation
    onContainerChanged: {
        if (container) {
            filterGroup.width = Qt.binding(function() {
                return Math.max(filterGroup.minimumGroupWidth, template.content.width)
            })
            filterGroup.height = Qt.binding(function() {
                return Math.max(filterGroup.minimumGroupHeight, template.content.height)
            })
        }
    }

    //! Show or hide group top left label editor (default to visible).
    property alias labelEditorVisible : template.labelEditorVisible

    //! Show or hide group top left expand button (default to visible).
    property alias expandButtonVisible : template.expandButtonVisible

    Qan.RectGroupTemplate {
        id: template
        anchors.fill: parent
        groupItem: parent
        z: 1

        preferredGroupWidth: parent.preferredGroupWidth
        preferredGroupHeight: parent.preferredGroupHeight
    }

    // Emitted by qan::GroupItem when node dragging start
    onNodeDragEnter: { template.onNodeDragEnter() }
    // Emitted by qan::GroupItem when node dragging ends
    onNodeDragLeave: { template.onNodeDragLeave() }

    Component.onCompleted:
    {
        filterGroup.expandButtonVisible = false;
    }
}
