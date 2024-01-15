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

    signal seamCreated(bool newSeam);

    ProductFilterModel {
        id: productFilterModel
        sourceModel: controller
    }

    title: qsTr("Add new seam")
    parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    Component.onCompleted: {
        root.standardButton(Dialog.Ok).enabled = Qt.binding(function () { return !copyFromExistingSeamButton.checked || (seamCombo.currentIndex != -1); });
    }

    onAccepted: {
        if (copyFromExistingSeamButton.checked)
        {
            root.controller.createSeamCopy(productFilterModel.data(productFilterModel.index(productCombo.currentIndex, 0), (Qt.UserRole)), seamCombo.model[seamCombo.currentIndex].uuid, copyNumber.value)
        } else
        {
            root.controller.createSeam();
        }
        root.seamCreated(!copyFromExistingSeamButton.checked);
        destroy();
    }
    onRejected: {
        destroy();
    }
    GridLayout {
        columns: 2
        RadioButton {
            text: qsTr("Start with empty seam")
            checked: true
            Layout.columnSpan: 2
        }
        RadioButton {
            id: copyFromExistingSeamButton
            text: qsTr("Copy from existing seam")
            Layout.columnSpan: 2
        }
        ComboBox {
            id: productCombo
            enabled: copyFromExistingSeamButton.checked
            model: productFilterModel
            textRole: "display"

            Layout.columnSpan: 2
            Layout.fillWidth: true

            Component.onCompleted: {
                currentIndex = productCombo.find(root.product ? root.product.name : "")
            }
        }
        ComboBox {
            id: seamCombo
            enabled: copyFromExistingSeamButton.checked
            model: productFilterModel.data(productFilterModel.index(productCombo.currentIndex, 0), (Qt.UserRole+1)) ? productFilterModel.data(productFilterModel.index(productCombo.currentIndex, 0), (Qt.UserRole+1)).allRealSeams : []
            displayText: model[currentIndex] ? model[currentIndex].visualNumber + ": " + model[currentIndex].name  : ""

            delegate: ItemDelegate {
                width: seamCombo.width
                text: modelData.visualNumber + ": " + modelData.name
                highlighted: seamCombo.highlightedIndex === index
            }

            Layout.columnSpan: 2
            Layout.fillWidth: true
        }
        Text {
            text: qsTr("Number of copies:")
        }
        SpinBox {
            id: copyNumber
            enabled: copyFromExistingSeamButton.checked
            editable: copyFromExistingSeamButton.checked
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
