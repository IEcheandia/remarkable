import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.general 1.0

import grapheditor.components 1.0

Item
{
    //FIXME PureResult is wrong in the languageFile -->
    id: filterDescriptionItem
    property alias filterGraph: nodesFilterModel.filterGraph

    SelectedFilterNodesFilterModel {
        id: nodesFilterModel
    }

    ListView {
        id: listView
        anchors.fill: parent
        clip: true
        model: nodesFilterModel

        ScrollBar.vertical: ScrollBar {}

        delegate: GroupBox {
            width: ListView.view.width
            title: qsTr("Description of %1").arg(model.itemLabel)

            GridLayout {
                id: delegate
                property var implicitMaxLeftColumn: Math.max(filterIDLabel.implicitWidth, filterTypeLabel.implicitWidth, filterTypeIDLabel.implicitWidth, filterDescriptionLabel.implicitWidth, filterGroupLabel.implicitWidth, filterNameLabel.implicitWidth)
                property var maxRightColumn: width - implicitMaxLeftColumn - spacing
                width: parent.width
                columns: 2
                Label {
                    id: filterIDLabel
                    text: qsTr("ID:")
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
                Label {
                    text: String(model.itemData.ID).slice(1, -1)
                    wrapMode: Text.WrapAnywhere
                    Layout.maximumWidth: delegate.maxRightColumn
                }
                Label {
                    id: filterTypeLabel
                    visible: model.itemData.type != ""
                    text: qsTr("Type:")
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
                Label {
                    visible: model.itemData.type != ""
                    text: model.itemData.type
                    wrapMode: Text.WrapAnywhere
                    Layout.maximumWidth: delegate.maxRightColumn
                }
                Label {
                    id: filterTypeIDLabel
                    text: qsTr("Type ID:")
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
                Label {
                    text: String(model.itemData.typeID).slice(1, -1)
                    wrapMode: Text.WrapAnywhere
                    Layout.maximumWidth: delegate.maxRightColumn
                }
                Label {
                    id: filterDescriptionLabel
                    text: qsTr("Description:")
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
                Label {
                    wrapMode: Text.Wrap
                    text: model.itemData.contentName != "" ? LanguageSupport.getString(model.itemData.contentName + ".Beschreibung") : model.itemData.description
                    Layout.maximumWidth: delegate.maxRightColumn
                }
                Label {
                    id: filterGroupLabel
                    visible: model.itemData.contentName != ""
                    text: qsTr("Group:")
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
                Label {
                    wrapMode: Text.Wrap
                    visible: model.itemData.contentName != ""
                    text: LanguageSupport.getString(model.itemData.contentName + ".Gruppe")
                    Layout.maximumWidth: delegate.maxRightColumn
                }
                Label {
                    id: filterNameLabel
                    visible: model.itemData.contentName != ""
                    text: qsTr("Name:")
                    Layout.alignment: Qt.AlignRight | Qt.AlignTop
                }
                Label {
                    wrapMode: Text.Wrap
                    visible: model.itemData.contentName != ""
                    text: LanguageSupport.getString(model.itemData.contentName + ".Name")
                    Layout.maximumWidth: delegate.maxRightColumn
                }
            }
        }
    }
}
