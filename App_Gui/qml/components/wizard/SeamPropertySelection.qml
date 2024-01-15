import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

ColumnLayout {
    property alias scanlabScannerAvailable: propertyFilterModel.hasScanlabScanner
    property alias laserControlAvailable: propertyFilterModel.hasLaserControl
    property alias seamPropertyModel: propertyFilterModel.sourceModel

    signal edit();

    id: root

    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        id: list

        model: SeamPropertyFilterModel {
            id: propertyFilterModel
        }

        header: RowLayout {
            Label {
                Layout.preferredWidth: 0.3 * list.width
                text: qsTr("Property:")
                font.bold: true
            }
            Label{
                Layout.fillWidth: true
                text: qsTr("Current Value:")
                font.bold: true
            }
        }

        delegate: RowLayout {
            CheckBox {
                Layout.preferredWidth: 0.3 * list.width
                id: propertyBox
                text: model.parameter
                checked: model.checked
                onToggled: model.checked = propertyBox.checked
            }
            ToolButton {
                display: AbstractButton.IconOnly
                icon.name: "document-edit"
                palette.button: "white"
                visible: model.adjustable
                enabled: model.checked && model.adjustable
                onClicked: root.edit()
            }
            Label {
                Layout.fillWidth: true
                text: model.value
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Button {
            Layout.fillWidth: true

            text: qsTr("Select All")
            onClicked: propertyFilterModel.selectAll()
        }

        Button {
            Layout.fillWidth: true

            text: qsTr("Select None")
            onClicked: propertyFilterModel.selectNone()
        }
    }

}
