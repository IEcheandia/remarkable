import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0

Item {
    id: root

    property alias graphModel: filterInstanceModel.graphModel
    property alias subGraphModel: filterInstanceModel.subGraphModel
    property alias graphId: filterInstanceModel.graphId
    property alias attributeModel: filterAttributeModel.attributeModel
    property alias filterOnUserLevel: filterInstanceFilterModel.filterOnUserLevel
    property alias filterOnGroup: filterInstanceFilterModel.filterOnGroup
    property var resultsConfigModel
    property var sensorConfigModel
    property var errorConfigModel
    property var view
    property var getFilterParameter
    property var onlineHelp: undefined
    property bool editParameters: true
    property alias filterAttributeModel: filterAttributeModel
    property bool dialogVisible: false

    signal parameterValueChanged(var uuid, var value)

    function pushPreConfiguredGraph()
    {
        if (root.filterOnGroup)
        {
            if (filterGroupsModel.notGrouped)
            {
                filterInstanceFilterModel.group = -1;
                root.view.push(filterInstanceComponent);
            } else
            {
                filterInstanceGroupFilterModel.sourceGraph = "";
                root.view.push(filterGroupsComponent);
            }
        } else
        {
            root.view.push(filterInstanceComponent);
        }
    }

    function pushSubGraph(uuid, name)
    {
        filterInstanceGroupFilterModel.sourceGraph = uuid;
        if (filterInstanceGroupFilterModel.notGrouped)
        {
            filterInstanceFilterModel.group = filterInstanceGroupFilterModel.mapToSource(filterInstanceGroupFilterModel.index(0, 0)).row;
            root.view.push(filterInstanceComponent);
            root.view.currentItem.groupName = name;
        } else
        {
            root.view.push(filterGroupsComponent, {"subGraphName": name});
        }
    }

    FilterInstanceModel {
        id: filterInstanceModel
    }

    FilterGroupsModel {
        id: filterGroupsModel
        graphId: filterInstanceModel.graphId
        graphModel: filterInstanceModel.graphModel
        subGraphModel: filterInstanceModel.subGraphModel
    }

    FilterInstanceGroupFilterModel {
        id: filterInstanceFilterModel
        sourceModel: filterInstanceModel
    }

    FilterAttributeModel {
        id: filterAttributeModel
        graphId: filterInstanceModel.graphId
        graphModel: filterInstanceModel.graphModel
        subGraphModel: filterInstanceModel.subGraphModel
    }

    FilterAttributeSortFilterModel {
        id: filterAttributeSortFilterModel
        sourceModel: filterAttributeModel
        filterOnUserLevel: root.filterOnUserLevel
    }

    FilterInstanceGroupFilterModel {
        id: filterInstanceGroupFilterModel
        sourceModel: filterGroupsModel
        filterOnGroup: false
        filterOnUserLevel: root.filterOnUserLevel
    }

    ResultSettingFilterModel {
        id: resultSettingFilterModel
        sourceModel: resultsConfigModel
    }

    Component {
        id: filterGroupsComponent

        FilterGroupsListBox {
            id: filterGroupGroupBox
            backButton: StackView.index != 0
            onBack: root.view.pop()
            model: filterInstanceGroupFilterModel
            onGroupSelected: {
                filterInstanceFilterModel.group = index;
                root.view.push(filterInstanceComponent);
                root.view.currentItem.groupName = groupName;
            }
        }
    }

    Component {
        id: filterInstanceComponent

        FilterInstanceListBox {
            model: filterInstanceFilterModel
            onBack: root.view.pop()
            onInstanceSelected: {
                filterAttributeModel.filterInstance = uuid;
                root.view.push(filterAttributeComponent);
                root.view.currentItem.filterName = filterName;
                root.view.currentItem.filterId = filterId;
            }
        }
    }

    Component {
        id: filterAttributeComponent

        BackButtonGroupBox {
            id: filterAttributeItem
            property string filterName
            property string filterId
            title: qsTr("Attributes of\n%1").arg(filterName)
            icon: filterId != "" ? WeldmasterPaths.filterPictureDir + filterId + ".png" : ""
            onBack: {
                root.view.pop();
            }

            ListView {
                property int columnIndex: 0

                anchors.fill: parent

                id: filterAttributeListView

                model: filterAttributeSortFilterModel
                spacing: 15
                clip: true

                ScrollBar.vertical: ScrollBar {
                    id: scroll
                }

                delegate: GridLayout {
                    id: delegate

                    width: ListView.view.width - scroll.width
                    columns: 1 + (unitLabel.visible ? 1 : 0) + (helpButton.visible ? 1 : 0)

                    CheckBox {
                        Layout.fillWidth: true

                        visible: !root.editParameters
                        text: LanguageSupport.getString(model.contentName)
                        checked: model.selected
                        onToggled: {
                            model.selected = checked;
                        }
                    }

                    Label {
                        Layout.fillWidth: true

                        visible: root.editParameters
                        elide: Text.ElideRight
                        text: LanguageSupport.getString(model.contentName)
                    }

                    Label {
                        id: unitLabel

                        font.pixelSize: 14
                        text: LanguageSupport.getString(model.unit)
                        visible: root.editParameters
                    }

                    ToolButton {
                        Layout.rowSpan: 2
                        Layout.alignment: Qt.AlignBottom

                        id: helpButton

                        icon.name: "help-hint"
                        visible: root.editParameters && root.onlineHelp != undefined && model.helpFile != ""
                        onClicked: {
                            root.onlineHelp.alignment = Qt.AlignLeft;
                            root.onlineHelp.file = model.helpFile;
                            root.onlineHelp.open();
                        }
                    }

                    RowLayout {
                        property var group: model.group

                        Layout.columnSpan: 1 + (unitLabel.visible ? 1 : 0)
                        Layout.fillWidth: true

                        Repeater {
                            model: group.items

                            id: parameterRepeater

                            delegate: ColumnLayout {
                                readonly property bool multiple: parameterRepeater.count > 1
                                readonly property bool current: index == filterAttributeListView.columnIndex
                                readonly property bool expanded: !delegateLayout.multiple || delegateLayout.current
                                property var parameter: root.getFilterParameter(modelData.instanceId, modelData.attribute, filterAttributeModel.filterInstance, modelData.value)

                                Layout.fillWidth: !delegateLayout.multiple || delegateLayout.expanded

                                id: delegateLayout

                                ParameterEditor {
                                    Layout.fillWidth: true
                                    id: editor
                                    visible: root.editParameters && delegateLayout.expanded
                                    attribute: modelData.attribute
                                    parameter: delegateLayout.parameter
                                    defaultValue: modelData.value
                                    resultsConfigModel: resultSettingFilterModel
                                    sensorConfigModel: root.sensorConfigModel
                                    errorConfigModel: root.errorConfigModel

                                    onParameterValueModified: {
                                        if (editor.parameter && filterAttributeItem.StackView.status == StackView.Active)
                                        {
                                            root.parameterValueChanged(editor.parameter.uuid, editor.item.parameterValue);
                                        }
                                    }
                                    onDialogVisibleChanged: {
                                        root.dialogVisible = editor.dialogVisible;
                                    }
                                }

                                Label {
                                    Layout.preferredHeight: editor.implicitHeight
                                    Layout.preferredWidth: 60

                                    text: editor.item.parameterValue
                                    visible: root.editParameters && !editor.visible
                                    horizontalAlignment: Text.AlignHCenter
                                    verticalAlignment: Text.AlignVCenter

                                    background: Rectangle {
                                        border {
                                            width: 1
                                            color: "lightgrey"
                                        }

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                filterAttributeListView.columnIndex = index;
                                                editor.item.forceActiveFocus();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
