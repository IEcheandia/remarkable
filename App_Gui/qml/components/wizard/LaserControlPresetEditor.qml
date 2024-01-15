import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui 1.0
import precitec.gui.general 1.0

Control {
    property alias preset: presetModel.preset
    property alias model: presetDataFilterModel.sourceModel

    signal triggered()

    id: root

    enabled: root.preset !== null

    LaserControlPresetFilterModel {
        id: presetDataFilterModel
    }

    LaserControlPresetModel {
        id: presetModel
        displayChannel: bar.currentIndex == 0 ? LaserControlPresetModel.One : LaserControlPresetModel.Two
    }

    contentItem: ColumnLayout {

        TabBar {
            Layout.fillWidth: true

            id: bar

            visible: SystemConfiguration.LaserControlTwoChannel

            TabButton {
                text: qsTr("Channel 1")
                implicitWidth: implicitContentWidth + 20
            }

            TabButton {
                text: qsTr("Channel 2")
                implicitWidth: implicitContentWidth + 20
            }

            Component.onCompleted: {
                bar.currentIndex = 0;
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            RowLayout {
                readonly property real samplesInPlot: 7

                anchors {
                    fill: parent
                    leftMargin: root.width / (row.samplesInPlot - 1) - (repeater.count != 0 ? 0.5 * repeater.itemAt(0).implicitWidth : 0)
                }

                id: row

                spacing: root.width / (row.samplesInPlot - 1) - (repeater.count != 0 ? repeater.itemAt(0).implicitWidth : 0)

                Repeater {
                    id: repeater

                    model: presetModel

                    LaserControlSlider {
                        Layout.fillHeight: true

                        value : model.power

                        onSetValue: {
                            if (preset && preset.state === LaserControlPreset.Default)
                            {
                                preset.state = LaserControlPreset.Edit;
                            }
                            model.power = newValue;
                            root.triggered()
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }

    background: LaserControlPlot {
        topBar: bar.visible ? bar.height : 0
        isEnabled: root.enabled
        model: presetDataFilterModel
        dataSetRoleName: bar.currentIndex == 0 ? "channel1Power" : "channel2Power"
    }
}


