import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication

Rectangle {
    id: curtain
    property bool moving: false
    property real limit: 0
    property var alignment: Qt.AlignLeft
    anchors {
        top: parent.top
        topMargin: 0
        left: parent.left
        leftMargin: limit * parent.width
    }

    width: moving ? 4 : 2
    height: parent.height
    color: PrecitecApplication.Settings.alternateBackground

    Rectangle {
        anchors {
            top: parent.top
            right: alignment == Qt.AlignLeft ? parent.left : undefined
            left: alignment == Qt.AlignLeft? undefined : parent.right
            bottom: parent.bottom
        }
        width: alignment == Qt.AlignLeft ? parent.anchors.leftMargin : curtain.parent.width - parent.anchors.leftMargin - parent.width
        color: "#44aaaaaa"
    }

    Rectangle {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: 30
        height: 20
        radius: 4
        border {
            color: PrecitecApplication.Settings.alternateBackground
            width: moving ? 1 : 2
        }
    }
}
