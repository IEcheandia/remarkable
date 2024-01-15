import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui 1.0
import precitec.gui.components.ethercat 1.0 as EtherCAT

/**
 * View to set the analog out value of an EtherCAT slave.
 **/
Item {
    id: root
    /**
     * The SlaveInfoModel.Type of the EetherCAT slave this out view is for.
     **/
    property var type
    /**
     * Reference to a SlaveInfoModel.
     **/
    property alias model: controller.model
    /**
     * The index of the EtherCAT slave in the model.
     **/
    property alias slaveIndex: controller.index

    enabled: UserManagement.currentUser && UserManagement.hasPermission(App.ModifyEthercat)

    EtherCAT.AnalogOutController {
        id: controller
        monitoring: root.visible
    }

    ColumnLayout {
        anchors.fill: parent

        AnalogOutChannelControl {
            title: qsTr("Channel 1")
            from: type == EtherCAT.SlaveInfoModel.AnalogOut0To10 ? 0 : -10
            value: controller.channel1
            onValueChanged: {
                controller.setChannel1(value);
            }

            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        AnalogOutChannelControl {
            title: qsTr("Channel 2")
            from: type == EtherCAT.SlaveInfoModel.AnalogOut0To10 ? 0 : -10
            value: controller.channel2
            onValueChanged: {
                controller.setChannel2(value);
            }

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
