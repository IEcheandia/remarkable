import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import Precitec.AppGui 1.0

ScrollView {
    id: page
    property alias historyModel : listView.model
    ListView {
        id: listView
        anchors.fill: parent
        spacing: 20
        clip: true
        delegate: RowLayout {
            id: delegateItem
            width: ListView.view.width
            spacing: 10
            Rectangle {
                id: indicator
                width: 25
                height: 25
                radius: width * 0.5
                color: model.isNio ? "red" : "green"
                Layout.alignment: Qt.AlignLeft
            }

            GridLayout {
                columns: 2
                Label {
                    text: qsTr("Time:")
                    font.bold: true
                    Layout.alignment: Qt.AlignRight
                }
                Label {
                    text: model.lastResultTime
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("Process serial number:")
                    font.bold: true
                    Layout.alignment: Qt.AlignRight
                }
                Label {
                    text: model.serialNumber
                    Layout.fillWidth: true
                }
                Label {
                    visible: model.partNumber != ""
                    text: qsTr("Part number:")
                    font.bold: true
                    Layout.alignment: Qt.AlignRight
                }
                Label {
                    visible: model.partNumber != ""
                    text: model.partNumber
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("Product name:")
                    font.bold: true
                    Layout.alignment: Qt.AlignRight
                }
                Label {
                    text: model.productName
                    Layout.fillWidth: true
                }
            }
        }
    }
}





