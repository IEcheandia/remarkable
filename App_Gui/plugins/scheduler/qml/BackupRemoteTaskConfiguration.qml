import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.4

import precitec.gui.components.scheduler 1.0

GroupBox {
    id: root
    //: title of a Group Box
    title: qsTr("Backup configuration")

    property alias settings: manager.settings
    property bool acceptableInput: backupConfiguration.acceptableInput && transferDirectory.acceptableInput

    function save()
    {
        backupConfiguration.save();
        var transferSettings = transferDirectory.save();

        return manager.save(transferSettings);
    }

    BackupLocalDirectoryConfigurationManager {
        id: manager
    }

    ColumnLayout {
        width: parent.width
        BackupConfigurationComponent {
            id: backupConfiguration
            manager: manager
            Layout.fillWidth: true
        }
        TransferDirectoryTaskConfiguration {
            id: transferDirectory
            settings: root.settings
            title: qsTr("Remote Host")
            placeholderInfo: false
            Layout.fillWidth: true
        }
    }
}
