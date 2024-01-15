import QtQuick 2.5
import QtQuick.Controls 2.3
import precitec.gui.components.application 1.0 as PrecitecApplication

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

/**
 * ComboBox taking values from an attribute representing an enumeration value
 * and the value from a parameter.
 **/
ComboBox {
    id: control
    /**
     * Emitted when a combobox element gets activated.
     **/
    signal parameterValueModified()

    implicitWidth: maxContentWidth + leftPadding + rightPadding

    property var attribute: null
    property var parameter: null
    property int parameterValue: currentIndex
    property var defaultValue: undefined
    property int maxContentWidth: 0
    function attributeFields(attribute)
    {
        if (attribute == null)
        {
            return [];
        }
        var fields = attribute.fields();
        for (var i = 0; i < fields.length; i++)
        {
            fields[i] = LanguageSupport.getString(fields[i]);            
        }
        
        return fields;
    }
      

    model: attributeFields(attribute)
    currentIndex: parameter ? attribute.convertFromValueToIndex(parameter.value) : (attribute ? attribute.defaultValue : -1)
    onActivated: parameterValueModified()

    delegate: ItemDelegate {
        width: control.width
        contentItem: Label {
            id: label
            font.weight: control.currentIndex === index ? Font.DemiBold : Font.Normal
            font.italic: control.defaultValue == index
            text: modelData + (attribute.convertFromValueToIndex(control.defaultValue) == index ? " (" + qsTr("Default") + ")" : "")
            
        }
        highlighted: control.highlightedIndex === index
        Component.onCompleted: {
            console.log( "AttributeComboBox: Component on complete. control.currentIndex: " + control.currentIndex);           
            control.maxContentWidth = Math.max(control.maxContentWidth, implicitWidth);
        }
    }
    palette.buttonText: control.currentIndex != control.defaultValue ? PrecitecApplication.Settings.text : (ApplicationWindow.window ? ApplicationWindow.window.palette.buttenText : "black")

    ToolTip.text: attribute ? LanguageSupport.getStringWithFallback(attribute.description) : ""
    ToolTip.visible: hovered && ToolTip.text != ""
    ToolTip.timeout: 5000
}
