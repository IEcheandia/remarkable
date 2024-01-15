import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0
import precitec.gui.components.ethercat 1.0 as EtherCAT

GroupBox {
    id: root
    property alias model: filterModel.sourceModel
    property alias signalType: filterModel.signalType

    EtherCAT.GatewayFilterModel {
        id: filterModel
    }
    ColorDialog {
        id: colorDialog
        property int index: -1
        showAlphaChannel: false
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: 0.9 * parent.width
        height: 0.9 * parent.height
        title: qsTr("Select color")
        onAccepted: {
            if (index != -1)
            {
                root.model.setColor(filterModel.mapToSource(filterModel.index(index, 0)), color);
            }
        }
    }

    ScrollView {
        anchors.fill: parent
        ListView {
            id: listView
            anchors.fill: parent
            clip: true
            model: filterModel

            delegate: RowLayout {
                width: ListView.view.width
                CheckBox {
                    text: model.name
                    checked: model.enabled
                    onToggled: {
                        root.model.toggleEnabled(filterModel.mapToSource(filterModel.index(index, 0)));
                    }

                    Layout.fillWidth: true
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "color-picker"
                    icon.color: enabled ? model.color : Qt.rgba(model.color.r, model.color.g, model.color.b, 0.5)
                    enabled: model.enabled
                    onClicked: {
                        colorDialog.color = model.color;
                        colorDialog.index = index;
                        colorDialog.open();
                    }
                }
            }
        }
    }
}
