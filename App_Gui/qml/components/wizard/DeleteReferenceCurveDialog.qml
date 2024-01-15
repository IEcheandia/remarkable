import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0

Dialog {
    property var referenceCurve: null
    property var referenceCurveModel: null
    property bool popOnAccept: true

    id: root

    modal: true
    standardButtons: Dialog.Yes | Dialog.No
    closePolicy: Popup.CloseOnEscape

    onAccepted: {
        if (referenceCurveModel)
        {
            referenceCurveModel.removeReferenceCurve(referenceCurve);
            if (popOnAccept)
            {
                productStackView.pop();
            }
        }
    }

    Label {
        anchors.fill: parent
        font.bold: true
        text: qsTr("Delete %1?").arg(referenceCurve ? referenceCurve.name : "")
        horizontalAlignment: Text.AlignHCenter
    }

    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            root.reject()
        }
    }
}
