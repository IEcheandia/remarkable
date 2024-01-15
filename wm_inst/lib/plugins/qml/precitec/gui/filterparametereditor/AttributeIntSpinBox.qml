import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

/**
 * TextField taking values from an attribute representing an int value
 * and the value from a parameter.
 **/
Control {
    id: control
    /**
     * Emitted when the TextField value gets modified.
     **/
    signal parameterValueModified()
    property var attribute: null
    property var parameter: null
    property var defaultValue: undefined
    property int precision: attribute ? attribute.floatingPointPrecision : 3
    property int parameterValue: value
    property int from: attribute ? attribute.minValue : 0;
    property int to: attribute ? attribute.maxValue : 99;
    property int value: parameter ? parameter.value : (attribute ? attribute.defaultValue : 0);
    property bool isDefault: value == defaultValue || defaultValue === undefined
    readonly property string unit: attribute ? LanguageSupport.getStringWithFallback(attribute.unit, attribute.unit) : ""

    contentItem: ColumnLayout {
        TextField {
            id: textField
            validator: IntValidator {
                bottom: Math.min(control.from, control.to)
                top:  Math.max(control.from, control.to)
            }
            text: control.value + (!activeFocus && !isDefault ? " (" + defaultValue + ")" : "")
            onEditingFinished: {
                control.parameterValue = Number.fromLocaleString(control.locale, textField.text);
                control.parameterValueModified();
            }
            color: textField.acceptableInput || !activeFocus ? (control.isDefault ? (ApplicationWindow.window ? ApplicationWindow.window.palette.text : "black") : PrecitecApplication.Settings.text) : "red"
            selectByMouse: true

            Layout.fillWidth: true
        }
        RowLayout {
            visible: control.attribute && !control.attribute.readOnly
            Label {
                text: control.unit != "" ? control.from + " " + control.unit : control.from
            }
            Slider {
                id: slider
                from: Math.min(control.from, control.to)
                to: Math.max(control.from, control.to)
                value: from, to, control.value
                stepSize: 1.0
                snapMode: Slider.SnapOnRelease
                live: false
                Connections {
                    target: slider
                    enabled: control.parameter && control.attribute
                    function onValueChanged()
                    {
                        if (control.parameterValue != slider.value)
                        {
                            control.parameterValue = slider.value;
                            control.parameterValueModified();
                        }
                    }
                }
                Layout.fillWidth: true
            }
            Label {
                text: control.unit != "" ? control.to + " " + control.unit : control.to
            }
            Layout.fillWidth: true
        }
    }

    ToolTip.text: attribute ? LanguageSupport.getStringWithFallback(attribute.description) : ""
    ToolTip.visible: hovered && ToolTip.text != ""
    ToolTip.timeout: 5000
}
