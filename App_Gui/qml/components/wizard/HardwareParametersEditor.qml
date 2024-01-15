import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.filterparametereditor 1.0

/**
 * Gui for Hardware parameter editor.
 **/

PrecitecApplication.ActiveFocusControlAwareFlickable  {
    id: hardwareParametersEditor
    /**
    * The HardwareParameterModel
    **/
    property alias model: hardwareParameterFilterModel.sourceModel
    /**
    * The keys to include from the HardwareParameterModel
    **/
    property alias filterKeys: hardwareParameterFilterModel.filterKeys
    /**
     * The filter model for hardware parameters.
     **/
    property var filterModel: hardwareParameterFilterModel

    Column {
        HardwareParameterFilterModel {
            id: hardwareParameterFilterModel
            deviceProxy: HardwareModule.serviceDeviceProxy
            weldHeadDeviceProxy: HardwareModule.weldHeadDeviceProxy
        }

        Repeater {
            id: repeater
            model: hardwareParameterFilterModel
            GroupBox {
                id: group
                property var modelIndex: hardwareParameterFilterModel.mapToSource(hardwareParameterFilterModel.index(index, 0))
                height: groupCheckBox.checked ? implicitHeight : groupCheckBox.height
                width: Math.max(implicitWidth, parent.width)
                Behavior on height {
                    NumberAnimation {}
                }
                label: CheckBox {
                    id: groupCheckBox
                    width: parent.width
                    checked: model.enabled
                    text: model.display
                    onToggled: {
                        hardwareParametersEditor.model.setEnable(model.key, !model.enabled);
                    }
                }
                ColumnLayout {
                    width: parent.width

                    opacity: groupCheckBox.checked ? 1 : 0
                    Behavior on opacity {
                        NumberAnimation {}
                    }
                    Repeater {
                        model: HardwareParameterOverriddenModel {
                            hardwareParameterModel: hardwareParametersEditor.model
                            hardwareParameterIndex: group.modelIndex
                        }
                        ItemDelegate {
                            icon.name: "emblem-warning"
                            icon.color: "transparent"
                            property bool isFloat: model.parameter.type == Parameter.Float || model.parameter.type == Parameter.Double
                            text: qsTr("Overridden on \"%1\": %2").arg(model.measureTask.name).arg(isFloat ? Number(model.parameter.value).toLocaleString(locale, 'f', 2) : model.parameter.value)
                            background: null
                        }
                    }
                    Repeater {
                        model: HardwareParameterOverridesModel {
                            hardwareParameterModel: hardwareParametersEditor.model
                            hardwareParameterIndex: group.modelIndex
                        }
                        ItemDelegate {
                            property bool isFloat: model.parameter.type == Parameter.Float || model.parameter.type == Parameter.Double
                            property string productText: qsTr("Overrides value on product: %1").arg(isFloat ? Number(model.parameter.value).toLocaleString(locale, 'f', 2) : model.parameter.value)
                            property string seamSeriesText: qsTr("Overrides value on seam series: %1").arg(isFloat ? Number(model.parameter.value).toLocaleString(locale, 'f', 2) : model.parameter.value)
                            icon.name: "emblem-warning"
                            icon.color: "transparent"
                            text: model.seamSeries ? seamSeriesText : (model.product ? productText : "")
                            background: null
                        }
                    }
                    ParameterEditor {
                        Layout.fillWidth: true
                        attribute: model.attribute
                        parameter: model.parameter
                        milliFromMicro: model.milliFromMicro
                        enabled: groupCheckBox.checked
                        onParameterValueModified: {
                            hardwareParametersEditor.model.updateHardwareParameter(model.key, parameterValue);
                        }
                    }
                }
            }
        }
    }
}
