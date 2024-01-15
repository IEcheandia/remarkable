import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

Component
{
    id: messageDialogQuitInterface

    Dialog
    {
        id: messageDialogQuit
        anchors.centerIn: parent
        width: parent.width * 0.15
        height: parent.height * 0.2
        modal: true
        standardButtons: Dialog.Cancel | Dialog.Ok
        header: Control
        {
            padding: 5
            background: Rectangle
            {
                color: PrecitecApplication.Settings.alternateBackground
                height: 40
            }
            contentItem: Label
            {
                Layout.fillWidth: true
                text: qsTr("Close program?")
                font.pixelSize: 18
                font.bold: true
                color: PrecitecApplication.Settings.alternateText
                horizontalAlignment: Text.AlignHCenter
            }
        }

        onAccepted:
        {
            Qt.quit();
        }
        onRejected:
        {
            destroy();
        }

        GroupBox
        {
            implicitWidth: parent.width
            implicitHeight: parent.height

            Label
            {
                anchors.fill: parent
                text: "There are unsaved changes.\nDo you still want to quit?"
                font.bold: true
                font.pixelSize: 16
            }
        }
    }
}


