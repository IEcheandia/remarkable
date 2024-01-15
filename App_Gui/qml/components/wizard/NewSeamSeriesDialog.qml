import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0

Dialog {
    id: root
    property var product: null
    property var controller: null

    signal seamSeriesCreated();

    title: qsTr("Add new seam series")
    parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    onAccepted: {
        if (copyFromExisting.checked)
        {
            root.controller.createSeamSeriesCopy(productFilterModel.data(productFilterModel.index(productCombo.currentIndex, 0), (Qt.UserRole)), seamSeriesCombo.model[seamSeriesCombo.currentIndex].uuid, copyNumber.value)
        } else
        {
            root.controller.createSeamSeries();
        }
        root.seamSeriesCreated();
        destroy();
    }
    onRejected: {
        destroy();
    }
    GridLayout {
        columns: 2
        RadioButton {
            text: qsTr("Start with empty seam series")
            checked: true
            Layout.columnSpan: 2
        }
        RadioButton {
            id: copyFromExisting
            enabled: root.product && root.product.seamSeries.length > 0
            text: qsTr("Copy from existing seam series")
            Layout.columnSpan: 2
        }
        ComboBox {
            id: productCombo
            enabled: copyFromExisting.checked
            model: productFilterModel
            textRole: "display"

            Layout.columnSpan: 2
            Layout.fillWidth: true

            Component.onCompleted: {
                currentIndex = productCombo.find(root.product ? root.product.name : "")
            }
        }
        ComboBox {
            id: seamSeriesCombo
            enabled: copyFromExisting.checked
            model: productFilterModel.data(productFilterModel.index(productCombo.currentIndex, 0), (Qt.UserRole+1)) ? productFilterModel.data(productFilterModel.index(productCombo.currentIndex, 0), (Qt.UserRole+1)).seamSeries : []
            displayText: model[currentIndex] ? model[currentIndex].visualNumber + ": " + model[currentIndex].name  : ""

            delegate: ItemDelegate {
                width: seamSeriesCombo.width
                text: modelData.visualNumber + ": " + modelData.name
                highlighted: seamSeriesCombo.highlightedIndex === index
            }
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }
        Text {
            text: qsTr("Number of copies:")
        }
        SpinBox {
            id: copyNumber
            enabled: copyFromExisting.checked
            editable: copyFromExisting.checked
            value: 1
            from: 1
            to: 999
            inputMethodHints: Qt.ImhDigitsOnly
        }
    }
    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            root.reject()
        }
    }
}
