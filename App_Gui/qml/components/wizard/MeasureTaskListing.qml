import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

GroupBox {
    id: root
    property alias model: measureTaskListView.model
    property bool showLink: true
    property alias count: measureTaskListView.count
    property int collapsedWidth: titleLabel.implicitWidth + collapseButton.width + 3 * titleRow.spacing + root.leftPadding
    property bool expanded: true
    property bool collapsable: false

    signal selected(var uuid)
    signal deleted(var uuid)

    label: RowLayout {
        width: root.availableWidth
        id: titleRow

        Label {
            Layout.leftMargin: root.leftPadding
            Layout.fillWidth: true

            id: titleLabel

            text: root.title
            font.family: root.font.family
            font.pixelSize: root.font.pixelSize
            font.bold: true
            verticalAlignment: Text.AlignVCenter
        }
        ToolButton {
            display: AbstractButton.IconOnly
            id: collapseButton
            icon.name: root.expanded ? "go-previous" : "go-next"
            background: Item {}
            visible: root.collapsable
            onClicked: {
                root.expanded = !root.expanded;
            }
        }
    }

    ListView {
        anchors.fill: parent
        id: measureTaskListView
        clip: true
        implicitHeight: childrenRect.height

        ScrollBar.vertical: ScrollBar {
            id: verticalScroll
        }

        delegate: RowLayout {
            width: ListView.view.width - verticalScroll.width
            spacing: 0
            ItemDelegate {
                id: mainLabel
                Layout.fillWidth: true
                text: modelData.visualNumber + (modelData.name != "" && root.expanded ? (": " + modelData.name) : "")
                icon.name: root.showLink && modelData.linkTo != undefined && !root.expanded ? "link" : "application-menu"
                down: pressed || linkLabel.pressed
                onClicked: root.selected(modelData.uuid)
            }
            ItemDelegate {
                id: linkLabel
                visible: root.showLink && modelData.linkTo != undefined && root.expanded
                icon.name: "link"
                text: visible ? "%1: %2".arg(modelData.linkTo.visualNumber).arg(modelData.linkTo.name) : ""
                down: pressed || mainLabel.pressed
                onClicked: root.selected(modelData.uuid)
            }
            ToolButton {
                display: AbstractButton.IconOnly
                icon.name: "edit-delete"
                palette.button: "white"
                visible: root.expanded
                onClicked: root.deleted(modelData.uuid)
            }
        }
    }
}
