import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import precitec.gui.components.application 1.0 as PrecitecApplication

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Item {
    id: item
    height: columnLayout.height
    /**
     * Emitted when a combobox element gets activated.
     **/
    signal parameterValueModified()

    property var attribute: parent.attribute
    property var parameter: parent.parameter
    property var defaultValue: parent.defaultValue
    property var sensorConfigModel: parent.sensorConfigModel
    property int parameterValue: sensorConfigModel && control.currentIndex != -1 ? sensorConfigModel.data(sensorConfigModel.index(control.currentIndex, 0)) : 0
    property int enumValue: parameter ? parameter.value : (attribute ? attribute.defaultValue : -1)
    property int defaultIndex: sensorConfigModel && defaultValue !== undefined ? sensorConfigModel.indexForResultType(control.defaultValue).row : -1

    function checkDefaultValue()
    {
        if (sensorConfigModel === undefined)
        {
            return;
        }
        if (defaultValue === undefined)
        {
            return;
        }
        sensorConfigModel.ensureItemExists(defaultValue);
    }

    function checkParameterValue()
    {
        if (sensorConfigModel === undefined)
        {
            return;
        }
        if (!parameter)
        {
            return;
        }
        sensorConfigModel.ensureItemExists(parameter.value);
    }

    onDefaultValueChanged: checkDefaultValue()
    onParameterChanged: checkParameterValue()
    onSensorConfigModelChanged: {
        checkDefaultValue();
        checkParameterValue();
    }

    ColumnLayout {
        id: columnLayout
        width: 350
        height: searchResult.height + control.height

        /**
        * ComboBox taking values from an attribute representing a sensor
        * and the value from a parameter.
        **/
        ComboBox {
            id: control
            Layout.fillWidth: true
            model: filteredModel
            /**
            * Index is mapped from enum value.
            **/
            currentIndex: sensorConfigModel ? sensorConfigModel.indexForResultType(control.enumValue).row : -1
            onActivated: {
                parameterValueModified();
                // Remap filtered model index to origin model and reset searchBar
                var filteredIndex = filteredModel.index(currentIndex, 0);
                currentIndex = filteredModel.mapToSource(filteredIndex).row;
                searchResult.text = "";
            }

            delegate: ItemDelegate {
                id: delegate
                function updateImplicitWidth()
                {
                    if (delegate.implicitWidth > control.popup.width)
                    {
                        control.popup.width = delegate.implicitWidth
                    }
                }
                width: control.popup.width
                contentItem: Label {
                    id: label
                    font.weight: control.currentIndex === index ? Font.DemiBold : Font.Normal
                    font.italic: control.defaultIndex == index
                    text: model.name + " (" + model.enumType + ")" + (control.defaultIndex == index ? " (" + qsTr("Default") + ")" : "")
                }
                highlighted: control.highlightedIndex === index
                onImplicitWidthChanged: updateImplicitWidth()
                Component.onCompleted: updateImplicitWidth()
            }

            textRole: "name"
            palette.buttonText: control.currentIndex != control.defaultIndex ? PrecitecApplication.Settings.text : (ApplicationWindow.window ? ApplicationWindow.window.palette.buttenText : "black")

            ToolTip.text: attribute ?LanguageSupport.getStringWithFallback(attribute.description) : ""
            ToolTip.visible: hovered && ToolTip.text != ""
            ToolTip.timeout: 5000

            SettingNameFilterModel {
                id: filteredModel
                sourceModel: item.sensorConfigModel
                searchText: searchResult.text
            }
        }

        TextField {
            id: searchResult
            placeholderText: qsTr("Search sensor ...")
            Layout.fillWidth: true
            onTextChanged: {
                if (text.length > 0) {
                    control.popup.open();
                } else {
                    control.popup.close();
                }
            }
        }
    }
}
