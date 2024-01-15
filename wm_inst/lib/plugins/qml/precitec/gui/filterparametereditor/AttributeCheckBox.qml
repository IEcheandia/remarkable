import QtQuick 2.5
import QtQuick.Controls 2.3
import precitec.gui.general 1.0

/**
 * CheckBox taking values from an attribute representing a boolean value
 * and the value from a parameter.
 **/
CheckBox {
    /**
     * Emitted when the CheckBox gets toggled
     **/
    signal parameterValueModified()
    property var attribute: null
    property var parameter: null
    property bool parameterValue: checked
    property var defaultValue: undefined
    checked: parameter ? parameter.value : (attribute ? attribute.defaultValue : false)
    text: qsTr("On/Off")
    onToggled: {
        parameterValueModified();
        checked = Qt.binding(function () { return parameter ? parameter.value : (attribute ? attribute.defaultValue : false); });
    }

    ToolTip.text: attribute ? LanguageSupport.getStringWithFallback(attribute.description) : ""
    ToolTip.visible: hovered && ToolTip.text != ""
    ToolTip.timeout: 5000
}
