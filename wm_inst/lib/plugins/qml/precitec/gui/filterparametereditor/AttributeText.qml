import QtQuick 2.5
import QtQuick.Controls 2.3
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

/**
* TextArea taking values from an attribute representing a string value
* and the value from a parameter.
**/
TextArea {
	/**
	* Emitted when the TextArea value gets modified.
	**/
	signal parameterValueModified()
	property var attribute: null
	property var parameter: null
	property string parameterValue: ""
	property var defaultValue: undefined
	id: textArea
	property bool isDefault: text == defaultValue || defaultValue === undefined
	text: parameter ? parameter.value : (attribute ? attribute.defaultValue : "")
	onEditingFinished: {
		parameterValue = textArea.text
		parameterValueModified();
	}
	selectByMouse: true

	ToolTip.text: attribute ? LanguageSupport.getStringWithFallback(attribute.description) : ""
	ToolTip.visible: hovered && ToolTip.text != ""
	ToolTip.timeout: 5000

	background: Rectangle {
        border.width: textArea.activeFocus ? 2 : 1
		border.color: textArea.activeFocus ? textArea.palette.highlight : textArea.palette.mid
        color: "transparent"
	}
}