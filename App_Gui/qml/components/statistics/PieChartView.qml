import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

/**
 * A simple Pie Chart item.
 *
 * Displays section labels in a vertical Legend, next to the chart
 *
 * Requires the data input as a model.
 *
 * Model must provide the following roles:
 *  @li DisplayRole: name (string value to display)
 *  @li UserRole: count (int count of the instance occurances)
 *  @li UserRole + 1: countInPercent (double value in [0.0, 1.0], representing the relative portion the slice should occupy
 *  @li UserRole + 2: color
 **/

Item {
    property alias title: label.text

    property var model: null

    property color colorBackground: "lightgrey"

    property int radius: 0

    id: root

    Connections {
        target: model
        function onDataChanged() {
            canvas.requestPaint()
        }
        function onModelReset() {
            canvas.requestPaint()
        }
    }

    onRadiusChanged: canvas.requestPaint()

    implicitHeight: 250

    GridLayout {
        anchors.fill: parent

        columns: 2

        Canvas {
            Layout.minimumWidth: height
            Layout.fillHeight: true

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

                if (!root.model)
                {
                    return;
                }

                var sliceStart = 0;

                for (var i = 0; i < root.model.rowCount(); i++)
                {
                    var value = model.data(model.index(i, 0), Qt.UserRole);
                    var percentValue = model.data(model.index(i, 0), Qt.UserRole + 1);
                    var color = model.data(model.index(i, 0), Qt.UserRole + 2);

                    if (percentValue === 0.0)
                    {
                        // skip empty slice
                        continue;
                    }

                    var sliceEnd = sliceStart + 360 * percentValue;

                    var start = Math.PI * (sliceStart / 180)
                    var end = Math.PI * (sliceEnd / 180)

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

                    context.fillStyle = 'black'

                    sliceStart  = sliceEnd;
                }
            }
        }

        ListView {
            Layout.fillHeight: true
            Layout.fillWidth: true

            model: root.model

            delegate: RowLayout {

                width: ListView.view.width

                Rectangle {
                    Layout.preferredWidth: 20
                    Layout.preferredHeight: 20
                    Layout.alignment: Qt.AlignVCenter

                    id: legend

                    border.width: 1
                    color: model.color
                }

                Label {
                    Layout.fillWidth: true

                    id: label

                    text: ("<b>%1</b> %2 (%3\%)").arg(model.name).arg(model.count).arg((model.countInPercent * 100).toLocaleString(locale, 'f', 0))
                    wrapMode: Text.Wrap
                }
            }
        }

        Label {
            Layout.columnSpan: 2
            Layout.leftMargin: root.height > label.implicitWidth ? 0.5 * (root.height - label.implicitWidth) : 0

            id: label

            font.bold: true
        }
    }
}
