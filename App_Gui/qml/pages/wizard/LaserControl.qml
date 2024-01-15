import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Item {
    property alias productController: laserControlModel.productController
    property alias attributeModel: laserControlModel.attributeModel

    id: laserControlConfiguration

    LaserControlModel {
        id: laserControlModel
        laserControlPresetDir: WeldmasterPaths.laserControlPresetDir
        onCurrentPresetChanged: editor.preset = laserControlModel.currentPresetItem
    }

    Component {
        id: saveDialog
        Dialog {
            property var preset: null

            anchors.centerIn: parent

            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel
            contentItem: Label {
                text: qsTr("Save Changes to Product?")
                font.bold: true
            }
            onAccepted: {
                if (preset)
                {
                    preset.save();
                    laserControlModel.updateHardwareParameters(preset.uuid);
                }
            }
        }
    }
    Component {
        id: discardDialog
        Dialog {
            property var preset: null

            anchors.centerIn: parent

            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel
            contentItem: Label {
                text: qsTr("Discard Changes?")
                font.bold: true
            }
            onAccepted: {
                if (preset)
                {
                    preset.restore();
                }
            }
        }
    }
    Component {
        id: deleteDialog
        Dialog {
            property var preset: null

            anchors.centerIn: parent

            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel
            contentItem: Label {
                text: qsTr("Delete Preset?")
                font.bold: true
            }
            onAccepted: laserControlModel.deletePreset(preset)
        }
    }

    ColumnLayout {
        anchors {
            fill: parent
            topMargin: spacing
            bottomMargin: spacing
            leftMargin: 0.07 * parent.width
            rightMargin: 0.07 * parent.width
        }

        spacing: 10

        LaserControlPresetEditor {
            Layout.fillWidth: true

            id: editor

            model: laserControlModel

            onTriggered: listView.positionViewAtIndex(listView.currentIndex, ListView.Contain)
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: listView

            spacing: 5
            clip: true

            ScrollBar.vertical: ScrollBar {
                id: verticalBar
            }

            model: laserControlModel
            currentIndex: laserControlModel.currentPreset

            interactive: !laserControlModel.isEditing

            onCurrentIndexChanged: positionViewAtIndex(currentIndex, ListView.Contain)

            highlight: Item {
                Rectangle {
                    anchors {
                        fill: parent
                        bottomMargin: 2 + (currentIndex != (listView.count - 1) ? 5 : 0)
                    }
                    color: "aliceblue"
                    radius: 5
                }
            }

            delegate: LaserControlPresetItem {
                width: listView.width - verticalBar.width
                preset: model.preset
                separatorVisible: index != (listView.count - 1)

                onDeleted: {
                    var dialog = deleteDialog.createObject(laserControlConfiguration, {"preset": model.preset});
                    dialog.open();
                }
                onSelect: {
                    laserControlModel.currentPreset = index;
                }
                onSave: {
                    var dialog = saveDialog.createObject(laserControlConfiguration, {"preset": model.preset});
                    dialog.open();
                }
                onDiscard: {
                    var dialog = discardDialog.createObject(laserControlConfiguration, {"preset": model.preset});
                    dialog.open();
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Add new preset")
            display: AbstractButton.TextBesideIcon
            enabled: !laserControlModel.isEditing
            icon.name: "list-add"
            onClicked: {
                editor.preset = laserControlModel.createNewPreset();
            }
        }
    }

    onHeightChanged: listView.positionViewAtIndex(listView.currentIndex, ListView.Contain)
}
