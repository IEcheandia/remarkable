import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

GridLayout {
    columns: 2
    Label {
        text: qsTr("Station:")
        Layout.alignment: Qt.AlignRight
    }
    Label {
        text: change ? change.station : ""
    }
}
