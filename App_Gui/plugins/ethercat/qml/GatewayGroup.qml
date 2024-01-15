import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui 1.0

import precitec.gui.components.application 1.0
import precitec.gui.components.ethercat 1.0 as EtherCAT

GroupBox {
    id: root
    /**
     * Signal that the user toggled the bit at @p byteIndex / @p bitIndex.
     * The new value of the bit shall be @p state.
     **/
    signal bitToggled(int byteIndex, int bitIndex, bool state)
    property alias model: listview.model
    ListView {
        id: listview
        clip: true
        width: root.width
        implicitHeight: contentHeight
        interactive: false
        section.property: "byte"
        section.delegate: Label {
            width: listview.width
            height: implicitHeight + 5
            text: qsTr("Byte %1").arg(section)
            color: Settings.alternateText
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            background: Rectangle {
                color: Settings.alternateBackground
            }
        }

        delegate: Control {
            id: delegate
            width: ListView.view.width
            padding: 5
            contentItem: RowLayout {
                Label {
                    text: model.bit
                }
                Rectangle {
                    implicitWidth: 20
                    implicitHeight: implicitWidth
                    width: implicitWidth
                    height: width
                    radius: width / 2
                    border.width: 1
                    border.color: Settings.iconColor
                    color: model.state ? Settings.iconColor : "transparent"
                }
                Button {
                    text: qsTr("Toggle")
                    visible: delegate.ListView.view.model.signalType == EtherCAT.ViConfigService.Output && UserManagement.currentUser && UserManagement.hasPermission(App.ModifyEthercat)
                    onClicked: {
                        root.bitToggled(model.byte, model.bit, !model.state)
                    }
                }
                ItemDelegate {
                    text: model.display
                    background.visible: false
                    Layout.fillWidth: true
                }
            }
        }
    }
}
