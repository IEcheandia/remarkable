import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import precitec.gui.components.plotter 1.0
import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

Item {
    property bool isSelected: false
    property alias preset: presetModel.preset

    signal selected();
    signal animationFinished();

    id: root

    implicitHeight: root.isSelected ? plot.implicitHeight + grid.implicitHeight : column.implicitHeight

    Behavior on implicitHeight  {
        SequentialAnimation {
            NumberAnimation {
                duration: 150
                easing.type: Easing.InOutQuad
            }
            ScriptAction {
                script: root.animationFinished()
            }
        }
    }

    LaserControlPresetModel {
        id: presetModel
        displayChannel: SystemConfiguration.LaserControlTwoChannel ? LaserControlPresetModel.Both : LaserControlPresetModel.One
    }

    MouseArea {
        anchors.fill: parent

        onClicked: root.selected()
    }

    Control {
        anchors {
            fill: parent
            margins: boundary.border.width
        }

        GridLayout {
            id: labelsGrid

            anchors {
                left: parent.left
                bottom: parent.bottom
                leftMargin: 5 + boundary.border.width
                bottomMargin: 0.25 * (grid.height - (channel1Label.height + channel2Label.height))
            }

            columns: 2

            rowSpacing: 0.5 * (grid.height - (channel1Label.height + channel2Label.height))
            visible: SystemConfiguration.LaserControlTwoChannel

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

        enabled: preset && preset.enabled
        contentItem: ColumnLayout {
            id: column

            Label {
                Layout.topMargin: 5
                Layout.alignment: Qt.AlignHCenter | Qt.AlignTop

                id: nameLabel

                text: preset ? preset.name : ""
            }

            GridLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBottom
                Layout.leftMargin: root.width / 6 - (repeater.count != 0 ? 0.5 * repeater.itemAt(0).implicitWidth : 0)

                id: grid

                columns: 4
                columnSpacing: root.width / 6 - (repeater.count != 0 ? repeater.itemAt(0).implicitWidth : 0)

                Repeater {
                    id: repeater

                    model: presetModel

                    LaserControlParameterItem {
                        Layout.alignment: Qt.AlignBottom | Qt.AlignLeft
                        power: model.power
                        offset: model.offset
                        editable: false
                    }
                }
            }
        }

        background: Rectangle {
            id: boundary

            border.color: "black"
            border.width: 4

            visible: root.isSelected

            LaserControlPlot {
                id: plot

                anchors {
                    fill: parent
                    leftMargin: boundary.border.width
                    rightMargin: boundary.border.width
                    topMargin: boundary.border.width + nameLabel.height
                    bottomMargin: boundary.border.width + grid.height
                }

                Component.onCompleted: {
                    if (preset)
                    {
                        plot.plotter.addDataSet(preset.channel1PowerDataSet);

                        if (SystemConfiguration.LaserControlTwoChannel)
                        {
                            plot.plotter.addDataSet(preset.channel2PowerDataSet);
                        }
                    }
                }
            }
        }
    }
}
