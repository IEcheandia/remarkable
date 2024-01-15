import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.4

import precitec.gui.components.scheduler 1.0

GroupBox {
    //: title of a Group Box
    title: qsTr("Delete old backups from local directory")

    property alias settings: manager.settings
    property alias acceptableInput: backupPath.acceptableInput

    function save()
    {
        manager.path = backupPath.text;
        manager.timeToLive = Number.fromLocaleString(locale, timeToLive.text);

        return manager.save();
    }

    DeleteBackupsConfigurationManager {
        id: manager
    }

    ColumnLayout {
        width: parent.width
        GridLayout {
            columns: 2
            Layout.fillWidth: true
            Label {
                //: title for a text field
                text: qsTr("Backup directory")
                Layout.alignment: Qt.AlignRight
            }
            TextField {
                id: backupPath
                objectName: "scheduler-deleteBackupsTaskConfiguration-backup-path"
                text: manager.path
                validator: PathValidator {}
                palette.text: backupPath.acceptableInput ? "black" : "red"
                Layout.fillWidth: true
                //: placeholder text for a text field
                placeholderText: qsTr("Path to local directory, e.g. /mnt/backup/")
            }
            Label {
                //: title for a text field
                text: qsTr("Number of days to keep backup")
                Layout.alignment: Qt.AlignRight
            }
            TextField {
                id: timeToLive
                objectName: "scheduler-deleteBackupsTaskConfiguration-ttl"
                text: manager.timeToLive
                validator: IntValidator {
                    bottom: 0
                    top: 3650
                }
                Layout.fillWidth: true
                //: placeholder text for a text field
                placeholderText: qsTr("Keep backups younger than entered days")
            }
        }
    }
}
