import QtQuick 2.7
import QtQuick 2.15
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import QuickQanava 2.0 as Qan
import "qrc:/QuickQanava" as Qan

import wobbleFigureEditor.components 1.0

Qan.EdgeItem {
    id: qanEdgeItem
    // Private hack for visual connector edge color dynamic modification
    property color color: style ? style.lineColor : Qt.rgba(0., 0., 0., 1.)

    PrecitecEdge {
        id: edge
        anchors.fill: parent
        edgeItem: qanEdgeItem
        color: parent.color
        isRampEdge: qanEdgeItem.edge.isRampEdge
        hasGradient: qanEdgeItem.edge.hasGradient
        startColor: qanEdgeItem.edge.startColor
        endColor: qanEdgeItem.edge.endColor
        startPosition: qanEdgeItem.edge.startPowerPosition
        endPosition: qanEdgeItem.edge.endPowerPosition
        recStrength: qanEdgeItem.edge.recStrength
    }

    HoverHandler {
        id: handler
        cursorShape: edge.isRampEdge ? Qt.PointingHandCursor : undefined
    }
}
