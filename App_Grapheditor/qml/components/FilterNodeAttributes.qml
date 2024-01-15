import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0
import precitec.gui.filterparametereditor 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

///FILTERNODE USERINTERFACE
Item {
    id: objectSpecificAttribute
    property var selectedNode: null
    property alias attributeModel: groupTableModel.attributeModel
    property alias resultModel: rootResultsSettingFilterModel.sourceModel
    property var sensorModel: null
    property var errorConfigModel: null
    property alias graphVisualizer: groupTableModel.graphModelVisualizer

    ColumnLayout {
        anchors.fill: parent

        id: filterNodeAttributes

        GroupBox {
            Layout.fillWidth: true
            title: qsTr("Label")
            TextField {
                anchors.fill: parent
                text: objectSpecificAttribute.selectedNode ? objectSpecificAttribute.selectedNode.label : ""
                onEditingFinished: {
                    if (objectSpecificAttribute.graphVisualizer)
                    {
                        objectSpecificAttribute.graphVisualizer.updateFilterLabel(objectSpecificAttribute.selectedNode, text);
                    }
                }
            }
        }

        PositionGroupBox {
            selectedNode: objectSpecificAttribute.selectedNode

            onPositionChanged: {
                if (objectSpecificAttribute.graphVisualizer)
                {
                    objectSpecificAttribute.graphVisualizer.updateFilterPosition(objectSpecificAttribute.selectedNode, point);
                }
            }

            Layout.fillWidth: true
        }

        GroupBox {
            readonly property int menuMargin: 10

            id: filterNodeLabel

            Layout.fillWidth: true
            Layout.fillHeight: true
            //: This is a Label
            title: qsTr("Attributes")

            ListView {
                property int columnIndex: 0

                anchors.fill: parent

                id: filterParameterList

                clip: true
                spacing: 5

                model: groupTableModel

                delegate: GridLayout {
                    property int userLevel: model.userLevel

                    function updateUserLevel(index)
                    {
                        model.userLevel = index;
                    }

                    id: delegateItem

                    width: ListView.view.width
                    columns: 2

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2

                        Label {
                            Layout.fillWidth: true

                            elide: Text.ElideRight
                            text: LanguageSupport.getString(model.contentName)
                        }

                        Label {
                            font.pixelSize: 14
                            text: LanguageSupport.getString(model.unit)
                        }

                        ToolButton {
                            id: menuButton
                            display: AbstractButton.IconOnly
                            icon.name: "application-menu"
                            icon.color: PrecitecApplication.Settings.alternateText
                            checkable: true
                        }
                    }

                    RowLayout {
                        property var group: model.group

                        Layout.fillWidth: true
                        Layout.columnSpan: 2

                        Repeater {
                            model: group.items

                            id: parameterRepeater

                            delegate: ColumnLayout {
                                readonly property bool multiple: parameterRepeater.count > 1
                                readonly property bool current: index == filterParameterList.columnIndex
                                readonly property bool expanded: !delegateLayout.multiple || delegateLayout.current
                                property var parameter: groupTableModel.getFilterParameter(modelData.instanceId, modelData.attribute, groupTableModel.filterInstance, modelData.value)

                                Layout.fillWidth: !delegateLayout.multiple || delegateLayout.expanded

                                id: delegateLayout

                                ParameterEditor {
                                    Layout.fillWidth: true
                                    id: editor
                                    visible: delegateLayout.expanded
                                    attribute: modelData.attribute
                                    parameter: delegateLayout.parameter
                                    defaultValue: modelData.defaultValue
                                    resultsConfigModel: rootResultsSettingFilterModel
                                    sensorConfigModel: objectSpecificAttribute.sensorModel
                                    errorConfigModel: objectSpecificAttribute.errorConfigModel

                                    onParameterValueModified: {
                                        delegateLayout.parameter.value = editor.item.parameterValue;
                                        modelData.value = editor.item.parameterValue;
                                        objectSpecificAttribute.graphVisualizer.graphEdited = true;
                                    }
                                }

                                Label {
                                    Layout.preferredHeight: editor.implicitHeight
                                    Layout.preferredWidth: 60

                                    text: modelData.value
                                    visible: !editor.visible
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
                                                filterParameterList.columnIndex = index;
                                                editor.item.forceActiveFocus();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    CheckBox {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        Layout.leftMargin: filterNodeLabel.menuMargin - publicityCheckBox.padding

                        id: publicityCheckBox

                        visible: menuButton.checked
                        //: This is a Label
                        text: qsTr("Publicity")
                        checked: model.publicity
                        onToggled: {
                            model.publicity = publicityCheckBox.checked;
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        Layout.leftMargin: filterNodeLabel.menuMargin
                        visible: menuButton.checked
                        //: This is a Label
                        text: qsTr("User Level")
                    }

                    ComboBox {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        Layout.leftMargin: filterNodeLabel.menuMargin

                        id: userLevelComboBox

                        visible: menuButton.checked
                        currentIndex: delegateItem.userLevel
                        //: These are values for a drop-down menu
                        model: [qsTr("SuperUser"), qsTr("Admin"), qsTr("GroupLeader"), qsTr("Operator")]
                        onActivated: {
                            delegateItem.updateUserLevel(userLevelComboBox.currentIndex);
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        Layout.leftMargin: filterNodeLabel.menuMargin
                        visible: menuButton.checked
                        //: This is a Label
                        text: qsTr("Description:")
                    }

                    TextArea {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        Layout.leftMargin: filterNodeLabel.menuMargin
                        visible: menuButton.checked
                        readOnly: true
                        wrapMode: Text.Wrap
                        background: Rectangle {
                            border {
                                width: 1
                                color: "lightgrey"
                            }
                        }
                        text: LanguageSupport.getString(model.description)
                    }

                    Label {
                        Layout.leftMargin: filterNodeLabel.menuMargin
                        visible: menuButton.checked
                        //: This is a Label
                        text: qsTr("Min:")
                    }

                    TextField {
                        Layout.fillWidth: true
                        visible: menuButton.checked
                        readOnly: true
                        text: model.minValue
                    }

                    Label {
                        Layout.leftMargin: filterNodeLabel.menuMargin
                        visible: menuButton.checked
                        //: This is a Label
                        text: qsTr("Default:")
                    }

                    TextField {
                        Layout.fillWidth: true
                        visible: menuButton.checked
                        readOnly: true
                        text: model.defaultValue
                    }

                    Label {
                        Layout.leftMargin: filterNodeLabel.menuMargin
                        visible: menuButton.checked
                        //: This is a Label
                        text: qsTr("Max:")
                    }

                    TextField {
                        Layout.fillWidth: true
                        visible: menuButton.checked
                        readOnly: true
                        text: model.maxValue
                    }

                    Rectangle {
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        height: 1
                        color: "lightgray"
                        visible: index != filterParameterList.count - 1
                    }
                }
            }
        }
    }

    FilterAttributeGroupModel {
        id: groupTableModel
        filterInstance: objectSpecificAttribute.selectedNode ? objectSpecificAttribute.selectedNode.ID : ""
    }

    ResultSettingFilterModel {
        id: rootResultsSettingFilterModel
    }
}
