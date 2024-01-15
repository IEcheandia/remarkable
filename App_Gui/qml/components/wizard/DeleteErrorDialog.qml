import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Dialog {
    property var error: null
    property var controller: null
    property bool popOnAccept: true

    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    title: qsTr("Delete Error?")
    standardButtons: Dialog.Yes | Dialog.No
    closePolicy: Popup.CloseOnEscape

    onAccepted: {
        if (error && controller)
        {
            controller.removeError(error);
        }
        if (popOnAccept)
        {
            productStackView.pop();
        }
        destroy();
    }
    onRejected: {
        destroy();
    }

    Label {
        text: qsTr("Do you really want to delete this Error?")
    }
}
