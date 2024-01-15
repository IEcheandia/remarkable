import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0
import precitec.gui.components.network 1.0 as Network
import precitec.gui 1.0
import precitec.gui.configuration 1.0

ColumnLayout {
    id: root
    property alias externalConfiguration: systemConfiguration.visible
    signal restartRequired();
    signal configurationRequested();

    RowLayout {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Network.NetworkConfiguration {
            onRestartRequired: root.restartRequired()
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        Button {
            id: systemConfiguration
            text: qsTr("Open system network configuration tool")
            onClicked: root.configurationRequested()
            Layout.margins: 5
        }
    }
    GroupBox {
        title: qsTr("Select EtherCAT device")
        Layout.fillWidth: true
        ColumnLayout {
            anchors.fill: parent
            EtherCATConfigurationController {
                id: etherCATController
            }
            CheckBox {
                id: useEtherCAT
                text: qsTr("Use EtherCAT")
                Component.onCompleted: {
                    useEtherCAT.checked = etherCATController.enabled;
                    etherCATController.enabled = Qt.binding(function() { return useEtherCAT.checked; });
                }
            }
            Network.WiredDeviceSelector {
                id: etherCATDeviceSelector
                enabled: useEtherCAT.checked
                currentIndex: etherCATDeviceSelector.indexFromMacAddress(etherCATController.macAddress)
                onModified: {
                    etherCATController.macAddress = etherCATDeviceSelector.macAddress;
                }
                Layout.fillWidth: true
            }
            Button {
                id: saveEtherCATButton
                icon.name: "document-save"
                text: qsTr("Save EtherCAT configuration")
                enabled: etherCATController.modified
                onClicked: {
                    etherCATController.save();
                    root.restartRequired();
                }
                Layout.alignment: Qt.AlignHCenter
            }
        }
    }
}
