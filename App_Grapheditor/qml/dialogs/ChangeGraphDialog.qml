import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

Dialog {
    id: messageDialogChangeGraph
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
            text: qsTr("Switch graph")
            font.pixelSize: 18
            font.bold: true
            color: PrecitecApplication.Settings.alternateText
            horizontalAlignment: Text.AlignHCenter
        }
    }

    GroupBox {
        implicitWidth: parent.width
        implicitHeight: parent.height

        Label {
            anchors.fill: parent
            text: qsTr("There are unsaved changes. Switch graph and lose current unsaved changes?")
            font.bold: true
            elide: Text.ElideLeft
            wrapMode: Text.Wrap
        }
    }
}
