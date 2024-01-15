import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3

import QuickQanava          2.0 as Qan
import "qrc:/QuickQanava"   as Qan

import wobbleFigureEditor.components 1.0

GridController {
    id: gridWithOrigin

    anchors.fill: parent

    gridScale: 25
    scale: FigureEditorSettings.scale
    property int minorLineToDrawCount: 0
    property int majorLineToDrawCount: 0
    /**
     * Whether the coordinate system is for a wobble figure or a seam figure.
     * Has influence on the text of the axis
     **/
    property bool wobbleFigure: false
    /**
     * Whether the coordinate system follows the origin. If @c false, the coordinate system is rendered in bottom left
     **/
    property bool followsOrigin: true
    /**
     * Additional scale factor for the coordinate system
     **/
    property real coordinateSystemScale: 1.0
    /**
     * The container item of the graph
     **/
    property var containerItem: null

    property real tickLabelSize: 8
    property color tickLabelColor: "#505050"

    Component {
        id: vTick

        Label {
            id: label
            text: "x mm"
            y: gridCanvas.canvasSize.height - height
            font {
                pointSize: tickLabelSize
            }
            color: gridWithOrigin.tickLabelColor
            transformOrigin: Item.BottomLeft
            rotation: -45
        }
    }

    Component {
        id: hTick

        Label {
            id: label
            text: "y mm"
            font {
                pointSize: gridWithOrigin.tickLabelSize
            }
            color: gridWithOrigin.tickLabelColor
        }
    }

    property var tickLabels: []
    onRedrawLines: {
        gridWithOrigin.minorLineToDrawCount = minorLineToDrawCount
        gridWithOrigin.majorLineToDrawCount = majorLineToDrawCount
        gridCanvas.requestPaint()

        for (const lbl of gridWithOrigin.tickLabels)
        {
            lbl.destroy()
        }
        gridWithOrigin.tickLabels = []

        let bottomY = gridWithOrigin.height // NOTE: Taking width/height from the GridController is essential during window-resizing events. So far this is only known way to get the updated window geomentry during grid-redraw.
        let offset = 3
        let left = mmLabel.x + mmLabel.width // used to remember maximum x that is occupied by vertical labels (to make sure horizontal labels don't overlap with them)
        for (let i = 0; i < gridWithOrigin.minorTickCount(); ++i)
        {
            let val = gridWithOrigin.minorTickValue(i)
            let txt = val.toLocaleString(locale, "f", 2)
            if (gridWithOrigin.minorTickIsVertical(i))
            {
                let x = gridWithOrigin.minorTickPos(i) + gridWithOrigin.tickLabelSize / 2
                if (x >= left + gridWithOrigin.tickLabelSize)
                {
                    let label = vTick.createObject(gridWithOrigin)
                    gridWithOrigin.tickLabels.push(label)

                    let y = bottomY - label.height

                    label.x = x
                    label.text = txt
                }
            }
            else
            {
                let y = gridWithOrigin.minorTickPos(i) - offset
                if (y > bottomY - 20)
                {
                    continue;
                }

                let label = hTick.createObject(gridWithOrigin)
                gridWithOrigin.tickLabels.push(label)

                label.x = offset
                label.y = y - gridWithOrigin.tickLabelSize - offset
                label.text = txt

                let right = label.x + label.width
                if (right > left)
                {
                    left = right
                }
            }
        }
    }

    Canvas {
        id: gridCanvas
        opacity: 0.9
        anchors.fill: parent
        visible: gridWithOrigin.visible
        enabled: false  // Disable mouse event catching
        onPaint: {
            var ctx = gridCanvas.getContext('2d')
            ctx.reset();

            if (gridWithOrigin.showScanField) {
                // draw scanfield
                context.beginPath();
                context.fillStyle = context.createPattern("lightgreen", Qt.Dense3Pattern);
                context.strokeStyle = "darkgreen";
                context.ellipse(gridWithOrigin.scanField.x, gridWithOrigin.scanField.y, gridWithOrigin.scanField.width, gridWithOrigin.scanField.height);
                context.fill();
                context.beginPath();
                context.ellipse(gridWithOrigin.scanField.x, gridWithOrigin.scanField.y, gridWithOrigin.scanField.width, gridWithOrigin.scanField.height);
                context.stroke();
            }

            ctx.strokeStyle = gridWithOrigin.thickColor

            // iterate over minor lines...
            if (gridWithOrigin.minorLineToDrawCount <= gridWithOrigin.minorLines.length) {
                ctx.lineWidth = 1.
                context.beginPath();

                for (let l = 0; l < gridWithOrigin.minorLineToDrawCount; l++) {
                    let line = gridWithOrigin.minorLines[l];
                    if (!line)
                        break;
                    ctx.moveTo(line.p1.x, line.p1.y)
                    ctx.lineTo(line.p2.x, line.p2.y)

                }
                context.stroke();
            }
        }

    } // Canvas gridCanvas

    Label {
        id: mmLabel
        y: gridCanvas.canvasSize.height - height - 3
        x: 3
        text: "[mm]"
        font {
            pointSize: gridWithOrigin.tickLabelSize
        }
        color: gridWithOrigin.tickLabelColor
    }

    Item {
        id: coordinateSystem
        visible: gridWithOrigin.showCoordinateSystem
        anchors.fill: parent
        property real axisSize: gridWithOrigin.origin.width * gridWithOrigin.coordinateSystemScale
        Rectangle {
            id: xAxis
            anchors {
                left: center.left
                bottom: center.bottom
                leftMargin: coordinateSystem.axisSize
                bottomMargin: coordinateSystem.axisSize * 0.5
            }
            width: gridWithOrigin.origin.height * gridWithOrigin.coordinateSystemScale
            height: coordinateSystem.axisSize
            color: "black"
        }
        Rectangle {
            id: yAxis
            anchors {
                left: center.left
                bottom: center.bottom
                leftMargin: coordinateSystem.axisSize * 0.5
                bottomMargin: coordinateSystem.axisSize
            }
            width: coordinateSystem.axisSize
            height: gridWithOrigin.origin.height * gridWithOrigin.coordinateSystemScale
            color: "black"
        }
        Rectangle {
            id: center
            state: gridWithOrigin.followsOrigin ? "followsOrigin" : "bottomLeft"
            states: [
                State {
                    name: "followsOrigin"
                    PropertyChanges {
                        target: center
                        x: gridWithOrigin.origin.x - coordinateSystem.axisSize
                        y: gridWithOrigin.origin.y - coordinateSystem.axisSize
                    }
                },
                State {
                    name: "bottomLeft"
                    AnchorChanges {
                        target: center
                        anchors.left: parent.left
                        anchors.bottom: parent.bottom
                    }
                }
            ]
            anchors.leftMargin: gridWithOrigin.followsOrigin ? 0 : coordinateSystem.axisSize * 2
            anchors.bottomMargin: gridWithOrigin.followsOrigin ? 0 : xAxisLabel.height + coordinateSystem.axisSize
            width: coordinateSystem.axisSize * 2
            height: coordinateSystem.axisSize * 2
            color: "black"
            radius: coordinateSystem.axisSize
        }
        Label {
            id: xAxisLabel
            anchors {
                left: xAxis.right
                top: xAxis.bottom
            }
            text: gridWithOrigin.wobbleFigure ? qsTr('Parallel to welding direction') : "X"
            color: "black"
            font {
                bold: true
                pointSize: 20 * gridWithOrigin.containerItem.scale * gridWithOrigin.coordinateSystemScale
            }
        }
        Label {
            anchors {
                left: yAxis.right
                bottom: yAxis.top
            }
            text: gridWithOrigin.wobbleFigure ? qsTr('Vertical to welding direction') : "Y"
            rotation: gridWithOrigin.wobbleFigure && !gridWithOrigin.followsOrigin ? 270 : 0
            transformOrigin: Item.BottomLeft
            color: "black"
            font {
                bold: true
                pointSize: 20 * gridWithOrigin.containerItem.scale * gridWithOrigin.coordinateSystemScale
            }
        }
    }
} // Qan.AbstractLineGrid2
