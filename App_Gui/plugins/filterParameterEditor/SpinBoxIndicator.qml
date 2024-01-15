import QtQuick 2.5
import QtQuick.Controls 2.3

Item {
    property string iconName: ""
    implicitWidth: button.implicitWidth
    implicitHeight: button.implicitHeight
    signal clicked()
    onEnabledChanged: {
        if (!enabled)
        {
            activationTimer.stop()
            repeatTimer.stop()
        }
    }
    ToolButton {
        id: button
        icon.name: parent.iconName
        onPressAndHold: {
            activationTimer.restart();
            parent.clicked();
        }
        onReleased: {
            activationTimer.stop()
            repeatTimer.stop()
        }
        onClicked: parent.clicked()
    }

    Timer {
        id: activationTimer
        interval: 100
        repeat: false
        onTriggered: {
            repeatTimer.restart()
        }
    }

    Timer {
        id: repeatTimer
        interval: 100
        triggeredOnStart: true
        repeat: true
        onTriggered: {
            parent.clicked()
        }
    }
}
