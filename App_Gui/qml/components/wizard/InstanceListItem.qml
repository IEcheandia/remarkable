import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

ListView {
    property var productInstanceModel: null
    property bool loading: false
    property bool checkBoxVisible: false
    property color firstColor: "purple"
    property color secondColor: "orange"

    signal itemClicked(var index, var color)

    id: instanceList

    clip: true
    implicitWidth: 300

    ScrollBar.vertical: ScrollBar {
        id: bar
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: instanceList.loading
    }

    delegate: ItemDelegate {
        id: delegateItem

        width: ListView.view.width - bar.width
        highlighted: ListView.isCurrentItem
        enabled: !instanceList.loading

        contentItem: RowLayout {
            CheckBox {
                Layout.alignment: Qt.AlignVCenter
                visible: instanceList.checkBoxVisible
                checked: model.selected
                onToggled: {
                    model.selected = checked;
                    if (model.result && checked)
                    {
                        model.result.color = serialLabel.color;
                    }
                }
            }

            Rectangle {
                Layout.preferredWidth: 25
                Layout.preferredHeight: 25
                Layout.alignment: Qt.AlignVCenter

                radius: width * 0.5
                color: model.nioColor
            }

            ColumnLayout {
                spacing: 0

                Label {
                    property real gradient: index / instanceList.count

                    Layout.fillWidth: true

                    id: serialLabel

                    font.bold: true
                    text: model.serialNumber
                    color: Qt.rgba((secondColor.r - firstColor.r) * gradient + firstColor.r, (secondColor.g - firstColor.g) * gradient + firstColor.g, (secondColor.b - firstColor.b) * gradient + firstColor.b, 1.0)
                }

                Label {
                    Layout.fillWidth: true
                    font.bold: false
                    text: qsTr("From linked seam #%1").arg(model.visualSeamNumber)
                    visible: model.linkedSeam
                }

                Label {
                    Layout.fillWidth: true
                    text: Qt.formatDateTime(model.date, "yyyy-MM-dd hh:mm:ss.zzz")
                }
            }
        }

        background: Rectangle {
            color: delegateItem.highlighted ? "lightblue" : "transparent"
        }

        Component.onCompleted: {
            instanceList.implicitWidth = Math.max(instanceList.implicitWidth, implicitWidth + bar.width);
        }

        onClicked: instanceList.itemClicked(instanceList.model.mapToSource(instanceList.model.index(index, 0)), serialLabel.color)
    }
}
