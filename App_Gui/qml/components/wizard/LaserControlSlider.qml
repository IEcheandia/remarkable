import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication

Item {
    property int value: 0

    id: slider

    implicitWidth: 30

    signal setValue(int newValue)

    MouseArea {
        anchors.fill: parent
        onPositionChanged: {
            slider.setValue(Math.max(0, Math.min(100 * (height - mouse.y) / height, 100)));
        }
    }

    Rectangle {
        anchors {
            top: parent.top
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
        }

        id: bar

        width: 4
        color: slider.enabled ? PrecitecApplication.Settings.alternateBackground : "darkgray"

        Rectangle {
            anchors {
                horizontalCenter: parent.horizontalCenter
                verticalCenter: parent.verticalCenter
                verticalCenterOffset: (0.5 - 0.01 * slider.value) * parent.height
            }

            border {
                width: 2
                color: slider.enabled ? PrecitecApplication.Settings.alternateBackground : "darkgray"
            }

            width: 30
            height: 15
            radius: 2
            color: "white"
        }
    }
}
