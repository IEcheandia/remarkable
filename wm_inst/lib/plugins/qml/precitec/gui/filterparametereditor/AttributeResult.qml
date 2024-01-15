import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import precitec.gui.components.application 1.0 as PrecitecApplication

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Item {
    id: item
    height: columnLayout.height
    signal parameterValueModified()

    property var attribute: parent.attribute
    property var parameter: parent.parameter
    property var defaultValue: parent.defaultValue
    property var resultsConfigFilterModel: parent.resultsConfigFilterModel

    property int parameterValue: resultsConfigFilterModel && control.currentIndex != -1 ? resultsConfigFilterModel.sourceModel.data(resultsConfigFilterModel.mapToSource(resultsConfigFilterModel.index(control.currentIndex, 0))) : 0
    property int enumValue: parameter ? parameter.value : (attribute ? attribute.defaultValue : -1)
    property int defaultIndex: resultsConfigFilterModel && defaultValue !== undefined ? resultsConfigFilterModel.mapFromSource(resultsConfigFilterModel.sourceModel.indexForResultType(control.defaultValue)).row : -1

    onDefaultValueChanged: checkDefaultValue()
    onParameterChanged: checkParameterValue()
    onResultsConfigFilterModelChanged: {
        checkDefaultValue();
        checkParameterValue();
    }

    function checkDefaultValue()
    {
        if (resultsConfigFilterModel === undefined)
        {
            return;
        }
        if (defaultValue === undefined)
        {
            return;
        }
        resultsConfigFilterModel.sourceModel.ensureItemExists(defaultValue);
    }

    function checkParameterValue()
    {
        if (resultsConfigFilterModel === undefined)
        {
            return;
        }
        if (!parameter)
        {
            return;
        }
        resultsConfigFilterModel.sourceModel.ensureItemExists(parameter.value);
    }

    ColumnLayout {
        id: columnLayout
        width: 350
        height: searchResult.height + control.height

        ComboBox {
            id: control
            Layout.fillWidth: true
            model: filteredModel
            currentIndex: item.resultsConfigFilterModel ? item.resultsConfigFilterModel.mapFromSource(item.resultsConfigFilterModel.sourceModel.indexForResultType(item.enumValue)).row : -1

            onActivated: {
                parameterValueModified();
                // Remap filtered model index to origin model and reset searchBar
                var filteredIndex = filteredModel.index(currentIndex, 0);
                currentIndex = filteredModel.mapToSource(filteredIndex).row;
                searchResult.text = "";
            }

            delegate: ItemDelegate {
                width: control.width
                contentItem: Label {
                    id: label
                    font.weight: control.currentIndex === index ? Font.DemiBold : Font.Normal
                    font.italic: control.defaultIndex == index
                    text: model.name + " (" + model.enumType + ")" + (control.defaultIndex == index ? " (" + qsTr("Default") + ")" : "")
                }
                highlighted: control.highlightedIndex === index
            }

            textRole: "name"
            palette.buttonText: control.currentIndex != control.defaultIndex ? PrecitecApplication.Settings.text : (ApplicationWindow.window ? ApplicationWindow.window.palette.buttenText : "black")

            ToolTip.text: attribute ? LanguageSupport.getStringWithFallback(attribute.description) : ""
            ToolTip.visible: hovered && ToolTip.text != ""
            ToolTip.timeout: 5000

            ResultSettingFilterModel {
                id: filteredModel
                sourceModel: item.resultsConfigFilterModel
                searchText: searchResult.text
            }
        }

        TextField {
            id: searchResult
            placeholderText: qsTr("Search result ...")
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
