import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.4

import precitec.gui.components.scheduler 1.0

ColumnLayout {
    property var manager: null
    property bool acceptableInput: buttonGroup.checkState != Qt.Unchecked

    function save()
    {
        manager.configuration = configuration.checked;
        manager.logFiles = logFiles.checked;
        manager.screenshots = screenshots.checked;
        manager.software = software.checked;
    }

    ButtonGroup {
        id: buttonGroup
        exclusive: false
    }

    CheckBox {
        id: configuration
        objectName: "scheduler-backupLocalTaskConfiguration-configuration"
        checked: manager.configuration
        text: qsTr("Configuration")
        ButtonGroup.group: buttonGroup
    }
    CheckBox {
        id: logFiles
        objectName: "scheduler-backupLocalTaskConfiguration-logfiles"
        checked: manager.logFiles
        text: qsTr("Log files")
        ButtonGroup.group: buttonGroup
    }
    CheckBox {
        id: screenshots
        objectName: "scheduler-backupLocalTaskConfiguration-screenshots"
        checked: manager.screenshots
        text: qsTr("Screenshots")
        ButtonGroup.group: buttonGroup
    }
    CheckBox {
        id: software
        objectName: "scheduler-backupLocalTaskConfiguration-software"
        checked: manager.software
        text: qsTr("Installed software")
        ButtonGroup.group: buttonGroup
    }
}
