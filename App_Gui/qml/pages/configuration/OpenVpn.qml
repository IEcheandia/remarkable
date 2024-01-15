import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import Precitec.AppGui 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0

Control {
    id: pageItem

    signal restartRequired()

    OpenVpnController {
        id: controller
        onRestartRequired: pageItem.restartRequired()
        openVpnConfigDir: WeldmasterPaths.openVpnConfigDir
        exampleConfigDir: WeldmasterPaths.configurationTemplatesDir
    }

    Component {
        id: exampleConfigDialogComponent
        Dialog {
            id: exampleConfigDialog
            // workaround for https://bugreports.qt.io/browse/QTBUG-72372
            footer: DialogButtonBox {
                alignment: Qt.AlignRight
                Button {
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                }
            }
            width: Overlay.overlay.width * 0.8
            height: Overlay.overlay.height * 0.8

            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true
            onRejected: destroy()

            ScrollView {
                anchors.fill: parent
                clip: true
                Label {
                    text: controller.clientConfig()
                    anchors.fill: parent
                }
            }

            Connections {
                target: UserManagement
                function onCurrentUserChanged() {
                    exampleConfigDialog.reject()
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        GroupBox {
            title: qsTr("OpenVPN Status")
            GridLayout {
                columns: 2
                Label {
                    text: qsTr("Active state:")
                }
                Label {
                    text: controller.activeState
                }
                Label {
                    text: qsTr("Sub state:")
                }
                Label {
                    text: controller.subState
                }
            }
        }
        Label {
            text: qsTr("To configure the OpenVPN configuration a CA certificate, Diffie-Hellman and an OpenVPN server certificate and key need to be provided.\nTo generate the tool easy-rsa can be used.")
        }
        Label {
            text: qsTr("Please provide the following files in folder weldmaster/openvpn on the external device:<ul><li>ca.crt</li><li>dh.pem</li><li>weldmaster.crt</li><li>weldmaster.key</li></ul>")
        }
        Button {
            text: qsTr("Import")
            enabled: RemovableDevices.Service.udi != "" && RemovableDevices.Service.path != ""
            onClicked: controller.import(RemovableDevices.Service.path + "/weldmaster/openvpn/")
        }
        Label {
            text: qsTr("An example configuration can be downloaded. Please adjust to your needs. Especially adjust the server IP address to the public IP address one of this station.")
        }
        RowLayout {
            Button {
                text: qsTr("Show example configuration")
                onClicked: exampleConfigDialogComponent.createObject(pageItem).open();
            }
            Button {
                text: qsTr("Download example configuration")
                enabled: RemovableDevices.Service.udi != "" && RemovableDevices.Service.path != ""
                onClicked: controller.downloadClientConfig(RemovableDevices.Service.path + "/weldmaster/")
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
