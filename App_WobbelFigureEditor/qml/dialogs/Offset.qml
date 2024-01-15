import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.components.plotter 1.0

import wobbleFigureEditor.components 1.0

Component
{
    id: offset
    Dialog
    {
        id: showOffset
        property var handler: null
        anchors.centerIn: parent
        width: parent.width * 0.3
        height: parent.height * 0.2
        modal: true
        standardButtons: Dialog.Close | Dialog.Ok
        header: Control
        {
            padding: 5
            background: Rectangle
            {
                color: PrecitecApplication.Settings.alternateBackground
            }
            contentItem: Label
            {
                Layout.fillWidth: true
                text: qsTr("Offset")
                font.bold: true
                color: PrecitecApplication.Settings.alternateText
                horizontalAlignment: Text.AlignHCenter
            }
        }

        onAccepted:
        {
            handler.addOffsetToFigure();
        }
        onRejected:
        {
            destroy();
        }

        GridLayout
        {
            anchors.fill: parent
            columns: 2

            Label
            {
                id: offsetLabel
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: qsTr("Offset (x [mm],     y [mm]):")
                font.pixelSize: 18
                font.bold: true
            }
            TextField
            {
                id: offsetX
                Layout.fillWidth: true
                text: handler ? handler.offset.x.toLocaleString(locale, 'f', 0) : "0"
                validator: IntValidator
                {
                    id: offsetXVal
                    bottom: -2500
                    top: 2500
                }
                onEditingFinished:
                {
                    handler.offset = Qt.point(Number.fromLocaleString(locale, offsetX.text), Number.fromLocaleString(locale, offsetY.text));
                }
            }
            TextField
            {
                id: offsetY
                Layout.fillWidth: true
                text: handler ? handler.offset.y.toLocaleString(locale, 'f', 0) : "0"
                validator: IntValidator
                {
                    id: offsetYVal
                    bottom: -2500
                    top: 2500
                }
                onEditingFinished:
                {
                    handler.offset = Qt.point(Number.fromLocaleString(locale, offsetX.text), Number.fromLocaleString(locale, offsetY.text));
                }
            }

        }
    }
}

