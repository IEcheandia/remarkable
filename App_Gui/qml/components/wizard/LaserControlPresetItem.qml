import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

Item {
    property alias separatorVisible: separator.visible
    property alias preset: presetModel.preset

    id: root

    implicitHeight: layout.implicitHeight

    enabled: preset && preset.enabled

    LaserControlPresetModel {
        id: presetModel
        displayChannel: SystemConfiguration.LaserControlTwoChannel ? LaserControlPresetModel.Both : LaserControlPresetModel.One
    }

    MouseArea {
        anchors.fill: parent
        onClicked: root.select()
    }

    signal select();
    signal save();
    signal discard();
    signal deleted();

    RowLayout {
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: 5
            bottomMargin: 10
        }

        ToolButton {
            visible: layout.editable
            display: AbstractButton.IconOnly
            enabled: preset && preset.hasChanges
            icon.name: "document-save"
            onClicked: {
                if (preset && preset.state == LaserControlPreset.New)
                {
                    preset.save();
                } else
                {
                    root.save();
                }
                root.select();
            }
        }
        ToolButton {
            visible: preset && (preset.state == LaserControlPreset.Default || preset.state == LaserControlPreset.New)
            display: AbstractButton.IconOnly
            icon.name: "edit-delete"
            onClicked: {
                root.deleted();
                root.select();
            }

        }
        ToolButton {
            visible: preset && preset.state == LaserControlPreset.Default
            display: AbstractButton.IconOnly
            icon.name: "editor"
            onClicked: {
                if (preset)
                {
                    preset.state = LaserControlPreset.Edit;
                }
                root.select();
            }
        }
        ToolButton {
            visible: preset && preset.state == LaserControlPreset.Edit
            display: AbstractButton.IconOnly
            icon.name: "go-previous"
            onClicked: {
                if (preset)
                {
                    if (preset.hasChanges)
                    {
                        root.discard();
                    } else
                    {
                        preset.state = LaserControlPreset.Default;
                    }
                }
                root.select();
            }
        }
    }

    GridLayout {
        id: labelsGrid

        visible: SystemConfiguration.LaserControlTwoChannel

        anchors {
            left: parent.left
            verticalCenter: parent.verticalCenter
            verticalCenterOffset: 0.5 * presetLabel.implicitHeight
        }

        columns: 2

        rowSpacing: 0.5 * (inputGrid.height - (channel1Label.height + channel2Label.height))

        Label {
            id: channel1Label
            text: qsTr("Channel 1")
        }

        Rectangle {
            width: 50
            height: 5
            color: "magenta"
        }

        Label {
            id: channel2Label
            text: qsTr("Channel 2")
        }

        Rectangle {
            width: 50
            height: 5
            color: "limegreen"
        }
    }

    ColumnLayout {
        property bool editable: preset && (preset.state == LaserControlPreset.Edit || preset.state == LaserControlPreset.New)

        anchors.fill: parent

        id: layout

        TextField {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 5

            id: presetLabel
            readOnly: !layout.editable
            text: preset ? preset.name : ""
            selectByMouse: true

            background: Rectangle {
                border.width: layout.editable ? (presetLabel.activeFocus ? 2 : 1) : 0
                border.color: presetLabel.activeFocus ? presetLabel.palette.highlight : presetLabel.palette.mid
                color: "transparent"
            }

            onTextEdited: {
                if (preset)
                {
                    preset.name = text;
                }
            }

            onPressed: root.select()
        }

        GridLayout {
            Layout.fillWidth: true
            Layout.leftMargin: layout.width / 6 - (repeater.count != 0 ? 0.5 * repeater.itemAt(0).implicitWidth : 0)
            Layout.bottomMargin: 5

            id: inputGrid

            columns: 4

            columnSpacing: layout.width / 6 - (repeater.count != 0 ? repeater.itemAt(0).implicitWidth : 0)

            Repeater {

                id: repeater

                model: presetModel

                LaserControlParameterItem {
                    power: model.power
                    offset: model.offset
                    editable: layout.editable

                    onPowerChanged: model.power = parseInt(power, 10)
                    onOffsetChanged: model.offset = parseInt(offset, 10)
                    onClicked: root.select()
                }
            }
        }

        Rectangle {
            id: separator
            Layout.fillWidth: true
            Layout.preferredHeight: 2
            color: PrecitecApplication.Settings.alternateBackground
        }
    }
}
