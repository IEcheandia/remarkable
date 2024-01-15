import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication

Control {
  id: figurePropertieInformation

  property var figureAnalyzer: null
  property var simulationController: null

  contentItem: GroupBox {
    //:Title for a group box where information of the figure is shown.
    title: qsTr("Figure properties")

    GridLayout {
        anchors.fill: parent
        columns: 2

        Label {
            id: pointCountLabel
            Layout.fillWidth: true
            //:Title for a label where the current number of points is shown.
            text: qsTr("Points:")
        }
        Label {
            id: pointCount
            Layout.fillWidth: true
            text: figurePropertieInformation.figureAnalyzer.pointCount
        }

        Label {
            id: routeLengthLabel
            Layout.fillWidth: true
            //:Title for a label where the current route length is shown.
            text: qsTr("Length:")
        }
        Label {
            id: routeLength
            Layout.fillWidth: true
            text: "%1 mm".arg(Number(figurePropertieInformation.figureAnalyzer.routeLength).toLocaleString(locale, 'f', 3))
        }

        Label {
            id: routeTimeLabel
            Layout.fillWidth: true
            //: Title of route time. Route time is time for welding current route with the current speed.
            text: qsTr("Time:")
        }
        Label {
            id: routeTime
            Layout.fillWidth: true
            text: "%1 s".arg(Number(figurePropertieInformation.figureAnalyzer.figureTime).toLocaleString(locale, 'f', 6))
        }

        Label {
            id: startPointPositionLabel
            Layout.fillWidth: true
            //:Title for a label where the start position/point is shown.
            text: qsTr("Start point:")
        }

        Label {
            id: startPoint
            Layout.fillWidth: true
            text: "(%1, %2)".arg(Number(figurePropertieInformation.figureAnalyzer.startPoint.x).toLocaleString(locale, 'f', 3)).arg(Number(figurePropertieInformation.figureAnalyzer.startPoint.y).toLocaleString(locale, 'f', 3))
        }

        Label {
            id: endPointPositionLabel
            Layout.fillWidth: true
            //:Title for a label where the end position/point is shown.
            text: qsTr("End point:")
        }
        Label {
            id: endPoint
            Layout.fillWidth: true
            text: "(%1, %2)".arg(Number(figurePropertieInformation.figureAnalyzer.endPoint.x).toLocaleString(locale, 'f', 3)).arg(Number(figurePropertieInformation.figureAnalyzer.endPoint.y).toLocaleString(locale, 'f', 3))
        }

        Label {
            id: figureWidthLabel
            Layout.fillWidth: true
            //:Title for a label where the current width is shown.
            text: qsTr("Width:")
        }
        Label {
            id: figureWidth
            Layout.fillWidth: true
            text: "%1 mm".arg(Number(figurePropertieInformation.figureAnalyzer.figureWidth).toLocaleString(locale, 'f', 3))
        }

        Label {
            id: figureHeightLabel
            Layout.fillWidth: true
            //:Title for a label where the current height is shown.
            text: qsTr("Height:")
        }
        Label {
            id: figureHeight
            Layout.fillWidth: true
            text: "%1 mm".arg(Number(figurePropertieInformation.figureAnalyzer.figureHeight).toLocaleString(locale, 'f', 3))
        }
        Label {
            visible: figurePropertieInformation.simulationController.simulationMode
            id: figureWobbleGapLabel
            Layout.fillWidth: true
            //:Title for a label where the current wobble gap is shown.
            text: qsTr("Wobble gap:")
        }
        Label {
            visible: figurePropertieInformation.simulationController.simulationMode
            id: figureWobbleGap
            Layout.fillWidth: true
            text: "%1 mm".arg(Number(figurePropertieInformation.figureAnalyzer.wobblePointDistance).toLocaleString(locale, 'f', 6))
        }
        Label {
            visible: figurePropertieInformation.simulationController.simulationMode
            id: simulationFocusSpeedMinLabel
            Layout.fillWidth: true
            text: qsTr("Min. focus speed:")
        }
        Label {
            visible: figurePropertieInformation.simulationController.simulationMode
            id: simulationFocusSpeedMin
            Layout.fillWidth: true
            text: "%1 mm/s".arg(Number(figurePropertieInformation.figureAnalyzer.minFocusSpeed).toLocaleString(locale, 'f', 2))
        }
        Label {
            visible: figurePropertieInformation.simulationController.simulationMode
            id: simulationFocusSpeedAverageLabel
            Layout.fillWidth: true
            text: qsTr("Average focus speed:")
        }
        Label {
            visible: figurePropertieInformation.simulationController.simulationMode
            id: simulationFocusSpeedAverage
            Layout.fillWidth: true
            text: "%1 mm/s".arg(Number(figurePropertieInformation.figureAnalyzer.averageFocusSpeed).toLocaleString(locale, 'f', 2))
        }
        Label {
            visible: figurePropertieInformation.simulationController.simulationMode
            id: simulationFocusSpeedMaxLabel
            Layout.fillWidth: true
            text: qsTr("Max. focus speed:")
        }
        Label {
            visible: figurePropertieInformation.simulationController.simulationMode
            id: simulationFocusSpeedMax
            Layout.fillWidth: true
            text: "%1 mm/s".arg(Number(figurePropertieInformation.figureAnalyzer.maxFocusSpeed).toLocaleString(locale, 'f', 2))
        }
      }
    }

    background: Rectangle {
        color: "white"
    }
}
