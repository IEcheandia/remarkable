import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

GroupBox {
    id: root
    property alias from: slider.from
    property alias value: slider.value

    ColumnLayout {
        anchors.fill: parent
        Slider {
            id: slider
            from: -10
            to: 10
            value: 0
            stepSize: 0.1
            live: false

            Layout.fillWidth: true
        }
        RowLayout {
            Label {
                text: slider.from + " V"
            }
            Label {
                text: Number(slider.value).toLocaleString(locale) + " V"
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }
            Label {
                text: slider.to + " V"
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
