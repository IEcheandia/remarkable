import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui 1.0

import precitec.gui.components.application 1.0
import precitec.gui.components.ethercat 1.0 as EtherCAT

Item {
    id: root
    /**
     * Signal that the user toggled the bit at @p bitIndex.
     * The new value of the bit shall be @p state.
     **/
    signal bitToggled(int byteIndex, int bitIndex, bool state)
    property alias model: ethercatDigitalSlaveModel
    /**
     * Whether this is an output (@c true) or input (@c false) slave
     **/
    property bool output: false
    EtherCAT.DigitalSlaveModel {
        id: ethercatDigitalSlaveModel
    }

    ListView {
        anchors.fill: parent

        id: list

        ScrollBar.vertical: ScrollBar {}

        clip: true
        model: ethercatDigitalSlaveModel
        spacing: 5
        delegate: Control {
            id: delegate
            width: list.width
            padding: 5
            contentItem: RowLayout {
                Label {
                    text: qsTr("Bit %1").arg(index)
                }
                Rectangle {
                    implicitWidth: 20
                    implicitHeight: implicitWidth
                    width: implicitWidth
                    height: width
                    radius: width / 2
                    border.width: 1
                    border.color: Settings.iconColor
                    color: model.bit ? Settings.iconColor : "transparent"
                }
                Button {
                    text: qsTr("Toggle")
                    visible: root.output && UserManagement.currentUser && UserManagement.hasPermission(App.ModifyEthercat)
                    onClicked: {
                        root.bitToggled(0, index, !model.bit)
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
            }
        }
    }
}

