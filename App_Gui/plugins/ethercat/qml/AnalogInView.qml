import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0
import precitec.gui.components.ethercat 1.0 as EtherCAT

/**
 * View of analog EtherCAT slave.
 **/
Item {
    id: root
    /**
     * Reference to a SlaveInfoModel.
     **/
    property alias model: controller.model
    /**
     * The index of the EtherCAT slave in the model.
     **/
    property alias slaveIndex: controller.index

    onVisibleChanged: controller.clear()

    Component.onCompleted: controller.clear()

    EtherCAT.AnalogInController {
        id: controller
        monitoring: root.visible
    }

    ColumnLayout {
        anchors.fill: parent

        AnalogPlotterBox {
            title: qsTr("Channel 1 (%1 V)").arg(Number(controller.channel1CurrentValue).toLocaleString(locale))
            dataSet: controller.channel1
            range: controller.oversampling ? 1 : 10
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        AnalogPlotterBox {
            title: qsTr("Channel 2 (%1 V)").arg(Number(controller.channel2CurrentValue).toLocaleString(locale))
            dataSet: controller.channel2
            range: controller.oversampling ? 1 : 10
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
