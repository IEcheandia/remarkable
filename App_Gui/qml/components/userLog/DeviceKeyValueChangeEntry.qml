import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

GridLayout {
    columns: 2
    Label {
        text: qsTr("Device:")
        Layout.alignment: Qt.AlignRight
    }
    Label {
        text: change ? change.device : ""
    }
    Label {
        text: qsTr("Key:")
        Layout.alignment: Qt.AlignRight
    }
    Label {
        text: change ? change.key : ""
    }
    Label {
        text: qsTr("Value:")
        Layout.alignment: Qt.AlignRight
    }
    Label {
        text: change ? change.value : ""
    }
}
