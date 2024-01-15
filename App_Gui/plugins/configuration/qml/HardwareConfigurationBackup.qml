import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.configuration 1.0
import precitec.gui.general 1.0

Control {
    id: root

    signal restartRequired()

    padding: 5
    ButtonGroup {
        id: selectedBackupGroup
        exclusive: true
    }
    HardwareConfigurationBackupModel {
        id: hardwareConfigurationBackupModel
        onBackupSucceeded: Notifications.NotificationSystem.information(qsTr("Backup of hardware configuration succeeded"))
        onBackupFailed: Notifications.NotificationSystem.error(qsTr("Backup of hardware configuration failed"))
        backupDir: WeldmasterPaths.hardwareConfigurationBackupDir
        configDir: WeldmasterPaths.configurationDir
        onRestoreSucceeded: {
            Notifications.NotificationSystem.information(qsTr("Restore of hardware configuration succeeded, system restart required"));
            root.restartRequired();
        }
        onRestoreFailed: Notifications.NotificationSystem.error(qsTr("Restore of hardware configuration failed, current configuration restored"))
    }
    contentItem: ColumnLayout {
        GroupBox {
            title: qsTr("Available backups")
            Layout.fillHeight: true
            Layout.fillWidth: true

            ScrollView {
                anchors.fill: parent
                ListView {
                    clip: true
                    anchors.fill: parent
                    model: hardwareConfigurationBackupModel
                    delegate: RadioButton {
                        property int row: index
                        text: model.display
                        width: ListView.view.width
                        ButtonGroup.group: selectedBackupGroup
                    }
                }
            }
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Button {
                text: qsTr("Create new hardware configuration backup")
                icon.name: "document-save"
                onClicked: hardwareConfigurationBackupModel.createBackup()
            }
            Button {
                text: qsTr("Restore selected hardware configuration")
                icon.name: "edit-undo"
                enabled: selectedBackupGroup.checkedButton != null
                onClicked: hardwareConfigurationBackupModel.restore(selectedBackupGroup.checkedButton.row)
            }
        }
    }
}
