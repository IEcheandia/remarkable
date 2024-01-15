import QtQuick 2.12

import precitec.gui.components.plotter 1.0

Item {
    property alias model: plot.model
    property alias dataSetRoleName: plot.roleNames
    property alias plotter: plot
    property bool isEnabled: true
    property int topBar: 0

    implicitHeight: 325 + root.topBar

    id: root

    DataSet {
        property vector2d p
        id: sinus
        color: root.isEnabled ? "blue" : "gray"
        drawingMode: DataSet.Line
        maxElements: 151
        Component.onCompleted: {
            var angle = 0.0;
            for (var i = 0; i < 151; i++)
            {
                p.x = i;
                p.y = Math.sin(angle);
                angle += (2 * Math.PI) / 100;
                sinus.addSample(sinus.p);
            }
        }
    }

    DataSet {
        property vector2d p
        id: focusPoints
        color: root.isEnabled ? "red" : "gray"
        drawingMode: DataSet.Points
        drawingOrder: DataSet.OnTop
        maxElements: 7
        Component.onCompleted: {
            var angle = 0.0;
            for (var i = 0; i < 7; i++)
            {
                p.x = 25 * i;
                p.y = Math.sin(angle);
                angle += (2 * Math.PI) / 4;

                focusPoints.addSample(focusPoints.p);
            }
        }
    }

    Plotter {
        anchors {
            fill: parent
            topMargin: root.topBar
        }

        id: plot

        clip: true

        xAxisController {
            autoAdjustXAxis: true
        }
        yAxisController {
            autoAdjustYAxis: true
        }

        controller {
            columns: 6
            rows: 2
            verticalCrosshairVisible: false
            horizontalCrosshairVisible: false
        }

        Component.onCompleted: {
            plot.addDataSet(sinus);
            plot.addDataSet(focusPoints);
        }
    }
}
