import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Qt.labs.platform 1.0

import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.configuration 1.0
import precitec.gui.general 1.0

RemovableDevices.BackupControl {
    id: root

    Component {
        id: helperComponent
        SystemHardwareBackupHelper {
            id: backupHelper
        }
    }

    RemovableDevices.DateNamedDirectoryModel {
        id: archiveModel
        path: WeldmasterPaths.updateArchiveDir
    }

    prepareBackupFunction: function() {
        var helper = helperComponent.createObject(root, {"directory": WeldmasterPaths.configurationDir});
        helper.backup();
    }

    productName: Qt.application.name
    stationId: GuiConfiguration.stationId
    stationName: GuiConfiguration.stationName
    model: ListModel {
        ListElement {
            description: qsTr("Configuration")
            checked: true
            name: "config"
            path: ""
            size: ""
        }
        ListElement {
            description: qsTr("Log files")
            checked: false
            name: "logs"
            path: ""
            size: ""
        }
        ListElement {
            description: qsTr("Screenshots")
            checked: false
            name: "screenshots"
            path: ""
            size: ""
        }
        ListElement {
            description: qsTr("Service information (Core dumps)")
            checked: false
            name: "cores"
            path: "/var/lib/systemd/coredump/"
            size: ""
        }
    }
    Component.onCompleted: {
        model.setProperty(0, "path", WeldmasterPaths.configurationDir);
        model.setProperty(1, "path", WeldmasterPaths.logfilesDir);
        // drop the "file://" by converting to url and slice 7 from left
        model.setProperty(2, "path", StandardPaths.writableLocation(StandardPaths.PicturesLocation).toString().slice(7));

        if (archiveModel.rowCount() > 0)
        {
            model.insert(1, {
                "description": qsTr("Installed software"),
                "checked": true,
                "name": "software",
                "path": archiveModel.data(archiveModel.index(0, 0), Qt.UserRole + 1),
                "size": ""
            })
        }
        root.calculateDiskUsage();
    }
}
