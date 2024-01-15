import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

GroupBox {
    id: box
    property int number
    property alias channelEnabled: enabledBox.checked
    property alias maxCurrent: currentSlider.to
    property alias minCurrent: currentSlider.from
    property alias referenceBrightness: brightnessSlider.value
    property int measuringResult
    property alias currentValue: currentSlider.value
    property int originalValue

    signal updateEnabled(bool isEnabled)
    signal updateCurrentValue(int newValue)
    signal updateReferenceBrightness(int newValue)

    label: CheckBox {
        Layout.alignment : Qt.AlignRight
        id: enabledBox
        text: (qsTr("Channel %1")).arg(number)
        padding: 0
        onCheckedChanged: {
            box.updateEnabled(checked);
        }
    }
    GridLayout {
        anchors.fill: parent
        columns: 4
        rowSpacing: 10

        Label {
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: qsTr("Reference \nBrightness")
            enabled: channelEnabled
        }
        SpinBox {
            Layout.columnSpan: 2
            Layout.alignment : Qt.AlignRight
            enabled: channelEnabled
            from: brightnessSlider.from
            to: brightnessSlider.to
            stepSize: 1
            editable: true
            value: referenceBrightness
            onValueModified: {
                box.updateReferenceBrightness(value);
            }
            Component.onCompleted: {
                contentItem.selectByMouse = true;
            }
        }
        Label {
            text: brightnessSlider.from
            enabled: channelEnabled
        }
        Slider {
            Layout.fillWidth: true
            Layout.columnSpan: 2
            id: brightnessSlider
            enabled: channelEnabled
            wheelEnabled: true
            live: false
            from: 0
            to: 255
            stepSize: 1
            onValueChanged: {
                box.updateReferenceBrightness(value);
            }
        }
        Label {
            Layout.alignment : Qt.AlignRight
            text: brightnessSlider.to
            enabled: channelEnabled
        }
        Label {
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: qsTr("Measured \nBrightness")
            enabled: channelEnabled
        }
        SpinBox {
            Layout.columnSpan: 2
            Layout.alignment : Qt.AlignRight
            enabled: channelEnabled
            value: measuringResult
            from: brightnessSlider.from
            to: brightnessSlider.to
            editable: false
            down.indicator: Item{}
            up.indicator: Item{}
        }
        Label {
            Layout.fillWidth: true
            Layout.columnSpan: 4
            text: qsTr("LED current")
            enabled: channelEnabled
            font.bold: true
        }
        Label {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            text: qsTr("Value")
            enabled: channelEnabled
        }
        SpinBox {
            Layout.columnSpan: 2
            Layout.alignment : Qt.AlignRight
            enabled: channelEnabled
            from: currentSlider.from
            to: currentSlider.to
            stepSize: maxCurrent/100.0
            editable: true
            value: currentValue
            onValueModified: {
                box.updateCurrentValue(value);
            }
            Component.onCompleted: {
                contentItem.selectByMouse = true;
            }
        }
        Label {
            text: qsTr("%1 mA").arg(minCurrent)
            enabled: channelEnabled
        }
        Slider {
            Layout.fillWidth: true
            Layout.columnSpan: 2
            id: currentSlider
            enabled: channelEnabled
            wheelEnabled: true
            live: false
            from: 500
            stepSize: maxCurrent/100.0
            onValueChanged: {
                box.updateCurrentValue(value);
            }
        }
        Label {
            Layout.alignment : Qt.AlignRight
            text: qsTr("%1 mA").arg(maxCurrent)
            enabled: channelEnabled
        }
        Label {
            Layout.columnSpan: 2
            text: qsTr("Original Value")
            enabled: channelEnabled
        }
        SpinBox {
            Layout.columnSpan: 2
            Layout.alignment : Qt.AlignRight
            enabled: channelEnabled
            value: originalValue
            from: currentSlider.from
            to: currentSlider.to
            editable: false
            down.indicator: Item{}
            up.indicator: Item{}
        }
    }
}
