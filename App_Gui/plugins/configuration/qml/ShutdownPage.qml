import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0
import precitec.gui.configuration 1.0

import precitec.gui.components.removableDevices 1.0 as RemovableDevices

Item {
    id: root
    property bool usbDevice: RemovableDevices.Service.udi != ""
    property alias shutdownSystemPermission: shutdownService.shutdownSystemPermission
    property alias restartSystemPermission: shutdownService.restartSystemPermission
    property alias stopAllProcessesPermission: shutdownService.stopAllProcessesPermission

    ShutdownService {
        id: shutdownService
    }
    Label {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 5
        text: qsTr("Please remove external USB device.")
        visible: root.usbDevice
    }

    RowLayout {
        anchors.centerIn: parent        
        Column {
            ToolTip.visible: shutdownButtonIcon.hovered || shutdownButton.hovered
            ToolTip.text: qsTr("Shutdown the operating system")
            Button {
                id: shutdownButtonIcon
                visible: shutdownButton.visible
                enabled: shutdownButton.enabled
                icon.name: "system-shutdown"
                icon.width: 64
                icon.height: 64
                icon.color: enabled ? Settings.text : Qt.rgba(Settings.text.r, Settings.text.g, Settings.text.b, 0.5)
                display: Button.IconOnly
                width: shutdownButton.width
                flat: true
                down: pressed || shutdownButton.pressed

                onClicked: {
                    shutdownButton.onClicked();
                }
            }
            Button {
                id: shutdownButton
                enabled: !root.usbDevice
                visible: UserManagement.currentUser && UserManagement.hasPermission(root.shutdownSystemPermission)
                text: qsTr("Shutdown system")
                display: Button.TextOnly
                palette.buttonText: Settings.text
                palette.windowText: enabled ? Settings.text : Qt.rgba(Settings.text.r, Settings.text.g, Settings.text.b, 0.5)
                flat: true
                down: pressed || shutdownButtonIcon.pressed

                onClicked: {
                    shutdownService.shutdownSystem();
                }
            }
        }
        Column {
            ToolTip.visible: rebootButtonIcon.hovered || rebootButton.hovered
            ToolTip.text: qsTr("Reboot the operating system")
            Button {
                id: rebootButtonIcon
                visible: rebootButton.visible
                enabled: rebootButton.enabled
                icon.name: "system-reboot"
                icon.width: 64
                icon.height: 64
                icon.color: enabled ? Settings.text : Qt.rgba(Settings.text.r, Settings.text.g, Settings.text.b, 0.5)
                width: rebootButton.width
                flat: true
                down: pressed || rebootButton.pressed

                onClicked: {
                    rebootButton.onClicked();
                }
            }
            Button {
                id: rebootButton
                enabled: !root.usbDevice
                visible: UserManagement.currentUser && UserManagement.hasPermission(root.restartSystemPermission)
                text: qsTr("Reboot system")
                palette.buttonText: Settings.text
                palette.windowText: enabled ? Settings.text : Qt.rgba(Settings.text.r, Settings.text.g, Settings.text.b, 0.5)
                flat: true
                down: pressed || rebootButtonIcon.pressed

                onClicked: {
                    shutdownService.restartSystem();
                }
            }
        }
        
    }
}
