import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Item {
    property alias currentSeam: laserControlModel.measureTask
    property alias attributeModel: laserControlModel.attributeModel

    id: laserControlPage

    signal markAsChanged()

    LaserControlMeasureModel {
        id: laserControlModel
        laserControlPresetDir: WeldmasterPaths.laserControlPresetDir
        onMarkAsChanged: laserControlPage.markAsChanged()
        channel2Enabled: SystemConfiguration.LaserControlTwoChannel
    }

    ColumnLayout {
        anchors.fill: parent
        CheckBox {
            Layout.fillWidth: true
            text: qsTr("Preset Enabled")
            checked: laserControlModel.presetEnabled
            onToggled: {
                laserControlModel.presetEnabled = checked;
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 20

            id: listView

            spacing: 5
            clip: true
            visible: laserControlModel.presetEnabled

            ScrollBar.vertical: ScrollBar {}

            model: laserControlModel

            currentIndex: laserControlModel.currentPreset
            delegate: LaserControlSelectionItem {
                x: 0.07 * laserControlPage.width
                width: 0.85 * laserControlPage.width

                preset: model.preset

                onSelected: {
                    laserControlModel.currentPreset = index;
                }
                onAnimationFinished: {
                    listView.positionViewAtIndex(index, ListView.Contain);
                }
                Component.onCompleted: {
                    isSelected = Qt.binding(function() { return ListView.isCurrentItem });
                    preset.enabled = Qt.binding(function() { return ListView.isCurrentItem });
                }
            }
        }

        CheckBox {
            Layout.fillWidth: true
            text: qsTr("Delay Enabled")
            checked: laserControlModel.delayEnabled
            onToggled: {
                laserControlModel.delayEnabled = checked;
            }
        }
        RowLayout {
            Layout.leftMargin: 20
            visible: laserControlModel.delayEnabled
            Label {
                text: qsTr("Delay")
            }
            SpinBox {
                value: laserControlModel.delay
                to: 999
                from: -999
                editable: true
                onValueModified: {
                    laserControlModel.delay = value;
                }
                Component.onCompleted: {
                    contentItem.selectByMouse = true;
                }
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: !listView.visible
        }
    }
}
