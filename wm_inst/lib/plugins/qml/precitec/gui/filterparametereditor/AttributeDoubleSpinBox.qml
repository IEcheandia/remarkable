import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

/**
 * TextField taking values from an attribute representing a double value
 * and the value from a parameter.
 **/
Control {
    /**
     * Emitted when the TextField value gets modified.
     **/
    signal parameterValueModified()
    property var attribute: null
    property var parameter: null
    property var defaultValue: undefined
    property int precision: attribute ? attribute.floatingPointPrecision : 3
    property real parameterValue: value
    id: spinbox
    property real from: attribute ? attribute.minValue : 0;
    property real to: attribute ? attribute.maxValue : 99;
    property real value: parameter ? parameter.value : (attribute ? attribute.defaultValue : 0);
    property bool isDefault: value == defaultValue || defaultValue === undefined
    readonly property string unit: attribute ? LanguageSupport.getStringWithFallback(attribute.unit, attribute.unit) : ""

    contentItem: ColumnLayout {
        TextField {
            id: textField
            validator: DoubleValidator {
                bottom: Math.min(spinbox.from, spinbox.to)
                top:  Math.max(spinbox.from, spinbox.to)
            }
            text: Number(value).toLocaleString(locale, "f", spinbox.precision) + (!activeFocus && !isDefault ? " (" + Number(defaultValue).toLocaleString(locale, "f", spinbox.precision) + ")" : "")
            onEditingFinished: {
                spinbox.parameterValue = Number.fromLocaleString(locale, textField.text);
                spinbox.parameterValueModified();
            }
            color: textField.acceptableInput || !activeFocus ? (isDefault ? (ApplicationWindow.window ? ApplicationWindow.window.palette.text : "black") : PrecitecApplication.Settings.text) : "red"
            selectByMouse: true

            Layout.fillWidth: true
        }
        RowLayout {
            visible: spinbox.attribute && !spinbox.attribute.readOnly
            Label {
                readonly property string fromString: Number(spinbox.from).toLocaleString(locale, 'f', spinbox.precision)
                text: spinbox.unit != "" ? fromString + " " + spinbox.unit : fromString
            }
            Slider {
                id: slider
                from: Math.min(spinbox.from, spinbox.to)
                to: Math.max(spinbox.from, spinbox.to)
                value: from, to, spinbox.value
                stepSize: 0.1
                snapMode: Slider.SnapOnRelease
                live: false
                Connections {
                    target: slider
                    enabled: spinbox.parameter && spinbox.attribute
                    function onValueChanged()
                    {
                        if (spinbox.parameterValue != slider.value)
                        {
                            spinbox.parameterValue = slider.value;
                            spinbox.parameterValueModified();
                        }
                    }
                }
                Layout.fillWidth: true
            }
            Label {
                readonly property string to: Number(spinbox.to).toLocaleString(locale, 'f', spinbox.precision)
                text:spinbox.unit != "" ? to + " " + spinbox.unit : to
            }
            Layout.fillWidth: true
        }
    }

    ToolTip.text: attribute ? LanguageSupport.getStringWithFallback(attribute.description) : ""
    ToolTip.visible: hovered && ToolTip.text != ""
    ToolTip.timeout: 5000
}
