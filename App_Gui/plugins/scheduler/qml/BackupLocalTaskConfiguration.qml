import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.4

import precitec.gui.components.scheduler 1.0

GroupBox {
    //: title of a Group Box
    title: qsTr("Backup configuration")

    property alias settings: manager.settings
    property bool acceptableInput: backupConfiguration.acceptableInput && backupPath.acceptableInput

    function save()
    {
        manager.path = backupPath.text;
        backupConfiguration.save();

        return manager.save();
    }

    BackupLocalDirectoryConfigurationManager {
        id: manager
    }

    ColumnLayout {
        width: parent.width
        RowLayout {
            Layout.fillWidth: true
            Label {
                //: title for a text field
                text: qsTr("Backup directory")
            }
            TextField {
                id: backupPath
                objectName: "scheduler-backupLocalTaskConfiguration-backup-path"
                text: manager.path
                validator: PathValidator {}
                palette.text: backupPath.acceptableInput ? "black" : "red"
                Layout.fillWidth: true
                //: placeholder text for a text field
                placeholderText: qsTr("Path to local directory, e.g. /mnt/backup/")
            }
        }
        BackupConfigurationComponent {
            id: backupConfiguration
            manager: manager
            Layout.fillWidth: true
        }
    }
}
