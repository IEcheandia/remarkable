import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import QtQuick.Shapes 1.15

import wobbleFigureEditor.components 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication

GridLayout {
    id: root
    required property WobbleFigureEditor fileEditor
    property FigureCreator figureCreator: root.fileEditor.figureCreator
    columns: 3

    Label {
        Layout.fillWidth: true
        Layout.columnSpan: 3
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("Type:")
        font.bold: true

        Rectangle {
            border.color: PrecitecApplication.Settings.alternateBackground
            color: "transparent"
            anchors.fill: parent
        }
    }

    RowLayout {
        Layout.columnSpan: 3

        ToolButton {
            Layout.fillWidth: true
            text: "Seam"
            icon.name: "document-new"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            enabled: root.fileEditor.fileType === FileType.None
            onClicked: {
                figureCreator.fileType = FileType.Seam
                FigureEditorSettings.fileType = FileType.Seam
            }
            background: Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: figureCreator.fileType === FileType.Seam ? "gray" : "transparent"
            }
        }

        ToolButton {
            Layout.fillWidth: true
            text: "Wobble"
            icon.name: "document-new"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            enabled: root.fileEditor.fileType === FileType.None
            onClicked: {
                figureCreator.fileType = FileType.Wobble
                FigureEditorSettings.fileType = FileType.Wobble
            }
            background: Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                color: figureCreator.fileType === FileType.Wobble ? "gray" : "transparent"
            }
        }
    }

    Label {
        Layout.fillWidth: true
        Layout.columnSpan: 3
        horizontalAlignment: Text.AlignHCenter
        text: qsTr("Figure:")
        font.bold: true

        Rectangle {
            border.color: PrecitecApplication.Settings.alternateBackground
            color: "transparent"
            anchors.fill: parent
        }
    }

    RowLayout {
        Layout.columnSpan: 3
        Layout.fillWidth: true

        FigureButton {
            figureCreator: root.figureCreator
            shape: FigureCreator.Line

            paintFcn: function (ctx){
                ctx.beginPath();
                ctx.moveTo(gap, size * .5)
                ctx.lineTo(size - gap, size * .5);
                ctx.stroke()
            }
        }
        FigureButton {
            figureCreator: root.figureCreator
            shape: FigureCreator.Circle

            paintFcn: function (ctx){
                ctx.beginPath();
                ctx.arc(size * .5, size * .5, size * .5 - gap, 0, 2 * Math.PI)
                ctx.stroke()
            }
        }
        FigureButton {
            figureCreator: root.figureCreator
            shape: FigureCreator.Eight

            paintFcn: function (ctx){
                let r = (size - 2 * gap) / 4
                ctx.beginPath();
                ctx.arc(size * .5, gap + r, r, 0, 2 * Math.PI)
                ctx.stroke()
                ctx.beginPath();
                ctx.arc(size * .5, size - gap - r, r, 0, 2 * Math.PI)
                ctx.stroke()
            }
        }
        FigureButton {
            figureCreator: root.figureCreator
            shape: FigureCreator.Ellipse

            paintFcn: function (ctx){
                ctx.beginPath();
                let h = (size - gap * 2) * 0.5
                ctx.ellipse(gap, (size - h) / 2, size - gap * 2, h)
                ctx.stroke()
            }
        }
        FigureButton {
            figureCreator: root.figureCreator
            shape: FigureCreator.ArchSpiral

            paintFcn: function (ctx){
                let radius = (size - gap * 2) / 2. / 2.
                let windings = 2
                let n = 100
                let step = 1. / n

                ctx.beginPath();
                for (let i = 0; i <= n; ++i)
                {
                    let t = i * step
                    let r = t * radius * windings;
                    let u = t * windings * 2 * Math.PI;
                    ctx.lineTo(Math.cos(u) * r + size * .5,
                               Math.sin(u) * -r + size * .5)
                }
                ctx.stroke()
            }
        }
    }

    Label {
        id: numberOfPointsLabel
        Layout.fillWidth: true
        text: qsTr("Number of points:")
        font.bold: true
    }

    Label {
        id: rotationAngleLabel
        Layout.fillWidth: true
        text: figureCreator.figureShape == FigureCreator.Circle ? qsTr("Start Angle [°]:") :  qsTr("Rotation [°]:")
        font.bold: true
    }

    Canvas {
        id: alphaCanvas
        Layout.rowSpan: 2
        width: 60
        height: 60
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            ctx.strokeStyle = Qt.rgba(0, 0, 0, 1)
            ctx.lineWidth = 1;

            let r = width * .45

            ctx.save()
            ctx.setLineDash([3, 3])
            ctx.beginPath()
            ctx.arc(width * .5, height * .5, r, 0, 2 * Math.PI)
            ctx.stroke()
            ctx.restore()

            ctx.save()
            ctx.setLineDash([100, 0]) // workaround because this porperty is not covered by save/restore
            ctx.translate(width * .5, height * .5)
            ctx.rotate(-figureCreator.alpha / 180 * Math.PI)
            ctx.beginPath()
            ctx.moveTo(0, 0)
            ctx.lineTo(r, 0)
            ctx.stroke()
            ctx.restore()
        }

        Connections {
            target: figureCreator

            function onAlphaChanged() {
                alphaCanvas.requestPaint()
            }
        }
    }

    TextField {
        id: numberOfPoints
        Layout.fillWidth: true
        selectByMouse: true
        text: Number(figureCreator.pointNumber).toLocaleString(locale, 'f', 0)
        palette.text: numberOfPoints.acceptableInput ? "black" : "red"
        validator: IntValidator {
            bottom: 0
            top: 50000 //2^20 = 1048576 -> max number; RTC6-Karte max number 2^23
        }
        onEditingFinished: {
            figureCreator.pointNumber = Number.fromLocaleString(locale, numberOfPoints.text)
        }
    }

    TextField {
        id: rotationAngle
        Layout.fillWidth: true
        selectByMouse: true
        text: Number(figureCreator.alpha).toLocaleString(locale, 'f', 2)
        palette.text: rotationAngle.acceptableInput ? "black" : "red"
        validator: DoubleValidator {
            bottom: 0
            top: 360
            decimals: 2
        }
        onEditingFinished: {
            figureCreator.alpha = Number.fromLocaleString(locale, rotationAngle.text)
        }
    }
}
