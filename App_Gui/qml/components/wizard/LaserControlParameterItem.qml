import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

Control {
    property alias power: powerEdit.text
    property alias offset: offsetEdit.text
    property bool editable: true

    signal clicked()

    id: laserControlItem

    implicitWidth: grid.implicitWidth
    implicitHeight: grid.implicitHeight

    FontMetrics {
        id: fontMetrics
        font: powerLabel.font
    }

    contentItem: GridLayout {
        id: grid

        property int implicitTextFieldWidth: fontMetrics.advanceWidth("100") + powerEdit.leftPadding + powerEdit.rightPadding

        anchors.fill: parent

        columns: 3
        rowSpacing: 1
        columnSpacing: 0

        Label {
            Layout.fillHeight: true
            Layout.fillWidth: true

            id: powerLabel

            text: qsTr("Power ")

            horizontalAlignment: Text.AlignRight
            verticalAlignment: TextInput.AlignVCenter
        }
        TextField {
            Layout.preferredWidth: grid.implicitTextFieldWidth

            id: powerEdit

            readOnly: !editable
            color: powerLabel.color

            background: Rectangle {
                border.width: editable ? (powerEdit.activeFocus ? 2 : 1) : 0
                border.color: powerEdit.activeFocus ? powerEdit.palette.highlight : powerEdit.palette.mid
                color: "transparent"
            }
            validator: IntValidator {
                bottom: 0
                top: 100
            }
            onPressed: laserControlItem.clicked()
            palette.text: powerEdit.acceptableInput ? "black" : "red"
        }
        Label {
            Layout.fillHeight: true

            text: " %"
            verticalAlignment: TextInput.AlignVCenter
        }
        Label {
            Layout.fillHeight: true
            Layout.fillWidth: true

            text: qsTr("Offset ")
            horizontalAlignment: Text.AlignRight
            verticalAlignment: TextInput.AlignVCenter
        }
        TextField {
            Layout.preferredWidth: grid.implicitTextFieldWidth

            id: offsetEdit

            readOnly: !editable
            color: powerLabel.color

            background: Rectangle {
                border.width: editable ? (offsetEdit.activeFocus ? 2 : 1) : 0
                border.color: offsetEdit.activeFocus ? offsetEdit.palette.highlight : offsetEdit.palette.mid
                color: "transparent"
            }
            validator: IntValidator {
                bottom: 0
                top: 100
            }
            onPressed: laserControlItem.clicked()
            palette.text: offsetEdit.acceptableInput ? "black" : "red"
        }
        Label {
            Layout.fillHeight: true

            text: " %"
            verticalAlignment: TextInput.AlignVCenter
        }
    }
}
