import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0
import precitec.gui.components.ethercat 1.0 as EtherCAT

Item {
    id: root
    /**
     * Signal that the user toggled the bit at @p byteIndex / @p bitIndex.
     * The new value of the bit shall be @p state.
     **/
    signal bitToggled(int byteIndex, int bitIndex, bool state)
    property alias viConfig: ethercatGatewayModel.viConfig
    property var model: ethercatGatewayModel
    EtherCAT.GatewayModel {
        id: ethercatGatewayModel
    }
    EtherCAT.GatewayFilterModel {
        id: inputFilterModel
        signalType: EtherCAT.ViConfigService.Input
        sourceModel: ethercatGatewayModel
    }
    EtherCAT.GatewayFilterModel {
        id: outputFilterModel
        signalType: EtherCAT.ViConfigService.Output
        sourceModel: ethercatGatewayModel
    }
    ScrollView {
        anchors.fill: parent
        Item {
            implicitHeight: Math.max(inputGroup.implicitHeight, outputGroup.implicitHeight)
            implicitWidth: root.width
            clip: true

            GatewayGroup {
                id: inputGroup
                title: qsTr("Input")
                model: inputFilterModel
                width: parent.width/2
                anchors {
                    left: parent.left
                    top: parent.top
                }
            }
            GatewayGroup {
                id: outputGroup
                title: qsTr("Output")
                model: outputFilterModel
                width: parent.width/2
                anchors {
                    right: parent.right
                    top: parent.top
                }

                onBitToggled: {
                    root.bitToggled(byteIndex, bitIndex, state);
                }
            }
        }
    }
}
