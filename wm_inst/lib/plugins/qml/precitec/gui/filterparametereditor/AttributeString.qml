import QtQuick 2.5
import QtQuick.Controls 2.3
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

/**
 * TextField taking values from an attribute representing a string value
 * and the value from a parameter.
 **/
TextField {
    /**
     * Emitted when the TextField value gets modified.
     **/
    signal parameterValueModified()
    property var attribute: null
    property var parameter: null
    property string parameterValue: ""
    property var defaultValue: undefined
    id: textField
    property bool isDefault: text == defaultValue || defaultValue === undefined
    text: parameter ? parameter.value : (attribute ? attribute.defaultValue : "")
    onEditingFinished: {
        parameterValue = textField.text
        parameterValueModified();
    }
    selectByMouse: true

    ToolTip.text: attribute ? LanguageSupport.getStringWithFallback(attribute.description) : ""
    ToolTip.visible: hovered && ToolTip.text != ""
    ToolTip.timeout: 5000
}
