import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

/**
 * A simple Pie Chart item.
 *
 * Displays section labels in the middle of the slice, surrounding the chart
 *
 * Requires the data input as an array.
 *
 * Each item needs to provide the following values:
 *  @li name (string value to display)
 *  @li value (int count of the instance occurances)
 *  @li percentValue (double value in [0.0, 1.0], representing the relative portion the slice should occupy
 *  @li color
 **/

Item {
    property alias title: label.text

    property var slices: []

    property color colorBackground: "lightgrey"

    property int radius: 0

    id: root

    onRadiusChanged: canvas.requestPaint()
    onSlicesChanged: canvas.requestPaint()

    implicitHeight: 250

    ColumnLayout {
        anchors.fill: parent

        Canvas {
            Layout.fillHeight: true
            Layout.fillWidth: true

            id: canvas

            onPaint: {
                var context = getContext("2d")
                var radius = root.radius == 0 ? 0.4 * height : root.radius
                var center = Qt.vector2d(0.5 * width, 0.5 * height)

                context.reset()

                // background
                context.fillStyle = root.colorBackground
                context.beginPath()
                context.moveTo(center.x, center.y)
                context.arc(center.x, center.y, radius, 0, Math.PI * 2, false)
                context.lineTo(center.x, center.y)
                context.fill()

                var sliceStart = 0;

                for (var i = 0; i < root.slices.length; i++)
                {
                    var name = slices[i].name;
                    var value = slices[i].value;
                    var percentValue = slices[i].percentValue;
                    var color = slices[i].color;
                    var label = ("%2 (%3\%)").arg(value).arg((percentValue * 100).toLocaleString(locale, 'f', 0))

                    if (percentValue === 0.0)
                    {
                        // skip empty slice
                        continue;
                    }

                    var sliceEnd = sliceStart + 360 * percentValue;

                    var start = Math.PI * (sliceStart / 180)
                    var end = Math.PI * (sliceEnd / 180)

                    var middlePoint = Qt.vector2d(Math.cos(0.5 * (end + start)), Math.sin(0.5 * (end + start))).times(radius * 1.2).plus(center)

                    // slice
                    context.beginPath()
                    context.fillStyle = color
                    context.moveTo(center.x, center.y)
                    context.arc(center.x, center.y, radius, start, end, false)
                    context.lineTo(center.x, center.y)
                    context.fill()

                    // outline
                    context.beginPath()
                    if (percentValue !== 1.0)
                    {
                        context.moveTo(center.x, center.y)
                    }

                    context.arc(center.x, center.y, radius, start, end, false)

                    if (percentValue !== 1.0)
                    {
                        context.lineTo(center.x, center.y)
                    }
                    context.lineWidth = 1
                    context.strokeStyle = 'black'
                    context.stroke()

                    // text
                    context.fillStyle = 'black'

                    context.font = 'normal normal 12px sans-serif'
                    var normalLabelWidth = context.measureText(label).width;
                    var spaceWidth = context.measureText(" ").width;

                    context.font = 'normal bold 12px sans-serif'
                    var boldNameWidth = context.measureText(name).width;

                    if (middlePoint.x < center.x)
                    {
                        context.fillText(name, middlePoint.x - boldNameWidth - spaceWidth - normalLabelWidth, middlePoint.y)
                        context.font = 'normal normal 12px sans-serif'
                        context.fillText(label, middlePoint.x - normalLabelWidth, middlePoint.y)
                    } else
                    {
                        context.fillText(name, middlePoint.x, middlePoint.y)
                        context.font = 'normal normal 12px sans-serif'
                        context.fillText(label, middlePoint.x + boldNameWidth + spaceWidth, middlePoint.y)
                    }

                    sliceStart  = sliceEnd;
                }
            }
        }

        Label {
            Layout.alignment: Qt.AlignHCenter

            id: label

            font.bold: true
        }
    }
}
