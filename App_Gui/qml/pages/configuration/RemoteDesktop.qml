import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0
import precitec.gui 1.0

Control {
    id: pageItem
    property var remoteDesktop
    property string openVpnConfigDir
    property string configurationTemplatesDir

    signal restartRequired()

    ColumnLayout {
        anchors.fill: parent
        TabBar {
            id: tabBar
            Layout.fillWidth: true
            TabButton {
                text: qsTr("VNC")
            }
            TabButton {
                text: qsTr("OpenVPN")
            }
        }
        StackLayout {
            id: stack
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabBar.currentIndex

            ColumnLayout {
                spacing: 50
                Label {
                    visible: pageItem.remoteDesktop.enabled
                    text: qsTr("Remote desktop access is currently enabled")
                    Layout.margins: 5
                    Layout.alignment: Qt.AlignHCenter
                }
                Label {
                    visible: !pageItem.remoteDesktop.enabled
                    text: qsTr("Remote desktop access is currently disabled")
                    Layout.margins: 5
                    Layout.alignment: Qt.AlignHCenter
                }
                Label {
                    text: qsTr("To connect use a VNC client (e.g. TigerVNC) and open a connection to the default VNC port via the OpenVPN connection.\nPassword is precitec.")
                    Layout.margins: 5
                    Layout.alignment: Qt.AlignHCenter
                }
                Button {
                    visible: !pageItem.remoteDesktop.enabled
                    text: qsTr("Start")
                    onClicked: pageItem.remoteDesktop.start()

                    Layout.margins: 5
                    Layout.alignment: Qt.AlignHCenter
                }
                Button {
                    visible: pageItem.remoteDesktop.enabled
                    text: qsTr("Stop")
                    onClicked: pageItem.remoteDesktop.stop()
                    Layout.margins: 5
                    Layout.alignment: Qt.AlignHCenter
                }
            }

            Loader {
                id: loader
                active: false
                source: "OpenVpn.qml"
                Component.onCompleted: {
                    loader.setSource("OpenVpn.qml");
                }
                Connections {
                    target: stack
                    function onCurrentIndexChanged() {
                        if (stack.currentIndex == 1 && loader.active == false)
                        {
                            loader.active = true;
                        }
                    }
                }
                Connections {
                    target: loader.item
                    function onRestartRequired() {
                        pageItem.restartRequired()
                    }
                }
            }
        }
    }
}
