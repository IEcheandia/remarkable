import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication

Item {
    id: root

    required property FigureCreator figureCreator
    required property int shape
    required property var paintFcn

    property real gap: 5
    property real size: 60

    Layout.fillWidth: true
    implicitHeight: size


    ToolButton {
        id: button

        width: size
        height: size
        anchors.centerIn: parent

        onClicked: {
            attributesCircle.fileEditor.figureCreator.figureShape = shape
        }

        background: Rectangle {
            anchors.fill: button
            border.color: PrecitecApplication.Settings.alternateBackground
            color: root.figureCreator.figureShape === shape ? "gray" : "transparent"

            Canvas {
                anchors.fill: parent
                onPaint: {
                    var ctx = getContext("2d");
                    ctx.strokeStyle = Qt.rgba(0, 0, 0, 1);
                    ctx.lineWidth = 2;
                    paintFcn(ctx);
                }
            }
        }
    }
}
