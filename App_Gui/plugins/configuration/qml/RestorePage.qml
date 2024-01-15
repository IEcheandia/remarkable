import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.userManagement 1.0
import precitec.gui.configuration 1.0
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.components.network 1.0 as Network
import precitec.gui.components.application 1.0
import precitec.gui.general 1.0
import Precitec.AppGui 1.0

ColumnLayout {
    id: page
    property int restorePermission: -1
    property var prepareFunction
    property var sideTabView
    property bool blocked: false
    property bool restartRequired: false
    property var screenshotTool: null

    enabled: UserManagement.currentUser && UserManagement.hasPermission(page.restorePermission) && UserManagement.hasPermission(UserManagement.BlockAutomaticLogout)

    HardwareConfigurationBackupModel {
        id: hardwareConfigurationBackupModel
        backupDir: WeldmasterPaths.hardwareConfigurationBackupDir
        configDir: WeldmasterPaths.configurationDir
    }

    Label {
        text: qsTr("Restore overwrites the existing configuration.\nThe system needs to be restarted after restore finished.")
        Layout.alignment: Qt.AlignHCenter
    }
    RemovableDevices.RestoreControl {
        Layout.fillWidth: true
        Layout.fillHeight: true
        productName: Qt.application.name
        model: ListModel {
            ListElement {
                description: qsTr("Configuration")
                name: "config"
                path: ""
            }
        }
        Component.onCompleted: {
            model.setProperty(0, "path", WeldmasterPaths.configurationDir);
        }

        onPrepareRestore: {
            if (page.prepareFunction()) {
                ApplicationWindow.header.blocked = true;
                page.sideTabView.enabled = false;
                UserManagement.blockAutomaticLogout();
                page.blocked = true;
                hardwareConfigurationBackupModel.createBackup()
                restore();
            } else {
                cancel();
            }
        }

        onRestoreFinished: {
            page.restartRequired = true;

            var helper = helperComponent.createObject(page, {"directory": WeldmasterPaths.configurationDir});
            if (helper.load())
            {
                var dialog = hardwareRestoreDialogComponent.createObject(page, {"backupHelper": helper});
                dialog.open();
            } else
            {
                Notifications.NotificationSystem.information(qsTr("Restore of backup finished. Please remove external device and restart system."));
            }
        }

        onRestoreInProgressChanged: {
            HardwareModule.productModel.cleanupEnabled = !restoreInProgress;
            if (!restoreInProgress && page.blocked)
            {
                ApplicationWindow.header.blocked = false;
                page.sideTabView.enabled = true;
                UserManagement.resumeAutomaticLogout();
                page.blocked = false;
            }
        }
    }

    Component {
        id: helperComponent
        SystemHardwareBackupHelper {
            id: backupHelper
        }
    }

    Component {
        id: hardwareRestoreDialogComponent
        Dialog {
            id: hardwareRestoreDialog
            property var backupHelper
            parent: Overlay.overlay
            anchors.centerIn: parent

            header: DialogHeaderWithScreenshot {
                title: hardwareRestoreDialog.title
                screenshotTool: page.screenshotTool
            }
            title: qsTr("Restore Hardware Configuration")
            // workaround for https://bugreports.qt.io/browse/QTBUG-72372
            footer: DialogButtonBox {
                alignment: Qt.AlignRight
                Button {
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                }
            }
            modal: true

            onClosed: {
                Notifications.NotificationSystem.information(qsTr("Restore of backup finished. Please remove external device and restart system."));
                destroy();
            }

            ColumnLayout {
                GroupBox {
                    title: qsTr("Restore Uninterruptible power supply?")
                    Layout.fillWidth: true
                    ColumnLayout {
                        anchors.fill: parent
                        Label {
                            text: qsTr("Restore uninterruptable power supply to \"%1\" setting from backup?").arg(backupHelper.upsModel.data(backupHelper.upsModel.selectedIndex))
                        }
                        Button {
                            text: qsTr("Restore UPS configuration")
                            onClicked: backupHelper.upsModel.save()
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Restore EtherCAT device configuration?")
                    Layout.fillWidth: true
                    EtherCATConfigurationController {
                        id: etherCATController
                        macAddress: backupHelper.etherCATMacAddress
                    }
                    ColumnLayout {
                        anchors.fill: parent
                        Network.WiredDeviceSelector {
                            id: etherCATDeviceSelector
                            enabled: false
                            currentIndex: etherCATDeviceSelector.indexFromMacAddress(etherCATController.macAddress)
                            onModified: {
                                etherCATController.macAddress = etherCATDeviceSelector.macAddress;
                            }
                            Layout.fillWidth: true
                        }
                        Button {
                            id: saveEtherCATButton
                            icon.name: "document-save"
                            text: qsTr("Restore EtherCAT configuration")
                            enabled: etherCATController.modified
                            onClicked: {
                                etherCATController.save();
                            }
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
                GroupBox {
                    title: qsTr("Restore Network settings?")
                    Layout.fillWidth: true
                    ScrollView {
                        anchors.fill: parent
                        ColumnLayout {
                            anchors.fill: parent
                            Repeater {
                                model: backupHelper.connectionUUIDS
                                GroupBox {
                                    title: backupHelper.nameForConnection(modelData)
                                    Layout.fillWidth: true

                                    Network.ConnectionController {
                                        id: connectionController
                                        uuid: modelData
                                    }

                                    ColumnLayout {
                                        anchors.fill: parent
                                        Network.WiredDeviceSelector {
                                            id: connectionDevice
                                            enabled: false
                                            currentIndex: connectionDevice.indexFromMacAddress(backupHelper.macAddressForConnection(connectionController.uuid))
                                            onModified: connectionController.markAsModified()
                                            Layout.fillWidth: true
                                        }
                                        Label {
                                            function methodToString(method) {
                                                if (method == Network.Ipv4SettingModel.Automatic) {
                                                    return qsTr("Automatic (DHCP)");
                                                } else if (method == Network.Ipv4SettingModel.LinkLocal) {
                                                    return qsTr("Link-local");
                                                } else if (method == Network.Ipv4SettingModel.Manual) {
                                                    return qsTr("Manual (static)");
                                                } else if (method == Network.Ipv4SettingModel.Shared) {
                                                    return qsTr("Shared");
                                                } else {
                                                    return qsTr("Disabled")
                                                }
                                            }
                                            text: qsTr("Method: %1").arg(methodToString(backupHelper.ipForConnection(connectionController.uuid).method))
                                        }
                                        Repeater {
                                            id: ipReapeter
                                            model: backupHelper.ipForConnection(connectionController.uuid)
                                            Label {
                                                text: model.address + "/" + model.prefix
                                            }
                                        }
                                        Button {
                                            icon.name: "document-save"
                                            text: qsTr("Restore connection setting")
                                            onClicked: {
                                                ipReapeter.model.save();
                                                connectionController.save();
                                            }
                                            Layout.alignment: Qt.AlignHCenter
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
