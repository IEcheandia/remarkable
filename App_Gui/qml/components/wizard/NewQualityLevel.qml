import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0

Dialog {
    id: root
    property var model: null
    property color selectedColor: "gray"

    title: qsTr("New Level")
    parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: {
        if (model)
        {
            model.addColor(valueBox.value / 100, selectedColor);
            root.model = null;
        }
    }

    onRejected: {
        root.model = null;
    }

    RowLayout {
        anchors.fill: parent
        SpinBox {
            id: valueBox
            value: 0
            from: 0
            to: 200
            editable: true
            enabled: model ? model.enabled : false
            Component.onCompleted: {
                contentItem.selectByMouse = true;
            }
        }
        Label {
            Layout.fillWidth: true
            text: "%"
        }
        ColorDialog {
            id: colorDialog
            showAlphaChannel: false
            anchors.centerIn: Overlay.overlay
            width: Overlay.overlay ? Overlay.overlay.width * 0.9 : 0
            height: Overlay.overlay ? Overlay.overlay.height * 0.9 : 0
            modal: true
            title: qsTr("Select color")
            onAccepted: {
                root.selectedColor = color;
            }
        }
        ToolButton {
            id: buttonColorSelect
            icon.name: "color-picker"
            icon.color: root.selectedColor
            display: AbstractButton.IconOnly
            ToolTip.text: qsTr("Select color for this data plot")
            ToolTip.visible: hovered
            onClicked: {
                colorDialog.color = root.selectedColor;
                colorDialog.currentColor = root.selectedColor;
                colorDialog.currentHue = root.selectedColor.hslHue;
                colorDialog.currentSaturation = root.selectedColor.hslSaturation;
                colorDialog.currentLightness = root.selectedColor.hslLightness;
                colorDialog.crosshairsVisible = true;
                colorDialog.open();
            }
        }
    }
}
