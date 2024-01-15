import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0

Dialog {
    id: deleteSeamSeriesDialog
    property var seamSeries: null
    parent: Overlay.overlay
    anchors.centerIn: parent
    modal: true
    title: qsTr("Delete seam series?")
    standardButtons: Dialog.Yes | Dialog.No
    closePolicy: Popup.CloseOnEscape

    onAccepted: {
        deleteSeamSeriesDialog.seamSeries.product.destroySeamSeries(deleteSeamSeriesDialog.seamSeries);
        destroy();
    }
    onRejected: destroy()

    Label {
        text: qsTr("Do you really want to delete the seam series \"%1\" including all contained seams?\nDeleting a seam series cannot be undone.").arg(deleteSeamSeriesDialog.seamSeries ? deleteSeamSeriesDialog.seamSeries.name : "")
    }
}
