import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0

BreadCrumpGroupBox {
    id: root
    property var seam: null
    property var productController: null
    property var linkedSeam: seam ? seam.linkTo : null
    property var wizardModel: null

    property Component seamAssemblyImage: Loader {
        signal back()
        signal backToProductStructure()
        signal backToProduct()
        signal backToSeamSeries()
        signal backToSeam()
        signal switchToSeam(int index)
        signal switchToSeamSeries(int index)
        Connections {
            target: item
            function onBack() {
                back()
            }
            function onBackToProductStructure() {
                backToProductStructure()
            }
            function onBackToProduct() {
                backToProduct()
            }
            function onBackToSeamSeries() {
                backToSeamSeries()
            }
            function onBackToSeam() {
                backToSeam()
            }
            function onSwitchToSeam(index) {
                switchToSeam(index)
            }
            function onSwitchToSeamSeries(index) {
                switchToSeamSeries(index)
            }
            function onSwitchToSeamComponent(component) {
                root.switchToSubItem(component)
            }
        }
        sourceComponent: BreadCrumpGroupBox {
            product: root.seam.seamSeries.product
            seamSeries: root.seam.seamSeries
            seam: root.seam
            title: qsTr("Assembly image")
            lastLevelModel: root.wizardModel
            SeamAssemblyImage {
                id: assemblyPage
                width: parent.width
                height: parent.height
                currentSeam: root.seam
                onMarkAsChanged: root.markAsChanged()
                editable: true
            }
        }
    }

    property Component additionalButtonsComponent: ColumnLayout {
        Repeater {
            id: wizardButtons
            model: root.wizardModel
            property real preferredButtonWidth: 0
            Button {
                display: AbstractButton.TextUnderIcon
                text: model.display
                icon.name: model.icon
                icon.color: Settings.iconColor
                onClicked: root.subItemSelected(model.component);
                Component.onCompleted: {
                    wizardButtons.preferredButtonWidth = Math.max(wizardButtons.preferredButtonWidth, implicitWidth)
                }
                Layout.preferredWidth: wizardButtons.preferredButtonWidth
            }
        }
    }

    product: root.seam ? root.seam.seamSeries.product : null
    seamSeries: root.seam ? root.seam.seamSeries: null
    title: qsTr("Details of linked seam %1 (#%2)").arg(root.seam ? root.seam.name : "")
                                                  .arg(root.seam ? root.seam.visualNumber : -1)

    signal switchToLink(var seam)
    signal switchToSubItem(var component)
    signal subItemSelected(var component)
    signal markAsChanged();

    GridLayout {
        anchors.fill: parent
        columns: 2
        Label {
            text: qsTr("Name:")
            Layout.alignment: Qt.AlignRight
        }
        TextField {
            id: nameField
            selectByMouse: true
            text: root.seam ? root.seam.name : ""
            validator: FileSystemNameValidator {
                allowWhiteSpace: true
            }
            palette.text: nameField.acceptableInput ? "black" : "red"
            onEditingFinished: {
                if (root.seam && root.productController && root.seam.name != text)
                {
                    root.seam.name = text;
                    productController.markAsChanged();
                }
            }
            Layout.fillWidth: true
        }
        Label {
            text: qsTr("Linked to:")
            Layout.alignment: Qt.AlignRight
        }
        ItemDelegate {
            icon.name: "link"
            enabled: root.linkedSeam != null
            text: root.linkedSeam ? root.linkedSeam.visualNumber + (root.linkedSeam.name != "" ? (": " + root.linkedSeam.name) : "") : ""
            onClicked: root.switchToLink(linkedSeam)
            Layout.fillWidth: true
        }
        Label {
            text: qsTr("Label:")
            Layout.alignment: Qt.AlignRight
        }
        Label {
            text: root.seam && root.seam.label != undefined ? root.seam.label : ""
            Layout.fillWidth: true
        }
        Label {
            text: qsTr("Number:")
            Layout.alignment: Qt.AlignRight
        }
        Label {
            text: root.seam ? root.seam.visualNumber : ""
            Layout.fillWidth: true
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
