import QtQuick 2.5
import QtQuick.Controls 2.3

/**
 * SpinBox taking values from an attribute representing an int value
 * and the value from a parameter.
 **/
MilliFromMicroSpinBox {
    /**
     * Emitted when the SpinBox value gets modified.
     **/
    signal parameterValueModified()
    property var attribute: null
    property var parameter: null
    property int parameterValue: value
    property var defaultValue: undefined
    editable: true
    stepSize: attribute ? attribute.step : 1
    from: attribute ? attribute.minValue : 0
    to: attribute ? attribute.maxValue : 99
    value: parameter ? parameter.value : (attribute ? attribute.defaultValue : 0)

    onValueModified: parameterValueModified()
}

