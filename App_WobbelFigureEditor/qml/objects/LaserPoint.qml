import QtQuick 2.7
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import QuickQanava 2.0 as Qan
import "qrc:/QuickQanava" as Qan

Qan.NodeItem {
    id: laserPointItem
    width: 15
    height: 15

    property bool hovering: false

    Rectangle {
        z: 0
        anchors.fill: parent

        color: node.color
        border.color: node.laserPower != -1.0 ? node.color : "black"
        border.width: 3
        radius: laserPointItem.width * 0.5

        // Responsible for notifying the graph when the mouse hovers above the point. Used to trigger tooltips that describe the points.
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            acceptedButtons: Qt.NoButton
            onEntered: {
                if (!pressed) {
                    node.graph.mouseEnteredPoint(node.ID)
                    hovering = true
                }
                cursorShape = Qt.PointingHandCursor
            }
            onExited: {
                if (hovering) {
                    node.graph.mouseExitedPoint(node.ID)
                    hovering = false
                }
            }
        }
    }
}
