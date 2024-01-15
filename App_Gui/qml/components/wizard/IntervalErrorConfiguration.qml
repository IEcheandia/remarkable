import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.plotter 1.0
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Control {
    id: errorConfiguration
    property alias error: intervalErrorConfigModel.intervalError
    property alias valueList: valueMenu.model
    property alias errorList: errorMenu.model
    property alias currentSeam: intervalErrorConfigModel.currentSeam
    property alias currentProduct: referenceListing.currentProduct
    property alias errorConfigModel: configErrorsPage.errorSettingModel
    property alias type: typeLabel.text
    property alias qualityNorm: intervalErrorConfigModel.qualityNorm
    property bool souvisPreInspectionEnabled: false

    signal markAsChanged()
    signal plotterSettingsUpdated()
    signal updateSettings()

    onUpdateSettings: plotter.updateSettings()

    onErrorChanged: plotter.plotter.clear()

    IntervalErrorConfigModel {
        id: intervalErrorConfigModel
        onDataChanged: errorConfiguration.markAsChanged()
    }

    BackButtonGroupBox {
        id: errorSettings
        visible: productStackView.currentItem == errorSettings
        title: qsTr("Error Settings")
        ConfigErrorsPage {
            id: configErrorsPage
            width: parent.width
            height: parent.height
        }
    }

    contentItem: GridLayout {
        anchors.fill: parent
        columns: 4

        Label {
            text: qsTr("Name:")
        }
        TextField {
            id: nameField
            Layout.columnSpan: 3
            Layout.fillWidth: true
            selectByMouse: true
            text: error ? error.name : ""
            onEditingFinished: {
                if (error)
                {
                    error.name = text;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        Label {
            text: qsTr("Type:")
        }
        Label {
            id: typeLabel
            Layout.fillWidth: true
            Layout.columnSpan: 3
            Layout.preferredHeight: nameField.height
            verticalAlignment: Text.AlignVCenter
        }
        Label {
            text: qsTr("Value:")
        }
        ComboBox {
            id: valueMenu
            Layout.columnSpan: 3
            Layout.fillWidth: true
            currentIndex: -1
            textRole: "name"
            onActivated: {
                if (error)
                {
                    error.resultValue = model.data(model.index(valueMenu.currentIndex, 0));
                    errorConfiguration.markAsChanged();
                }
            }
            Component.onCompleted: {
                if (error && model)
                {
                    valueMenu.currentIndex = model.findIndex(error.resultValue);
                }
            }
        }
        Label {
            text: qsTr("Error:")
        }
        ComboBox {
            id: errorMenu
            Layout.fillWidth: true
            Layout.columnSpan: 2
            currentIndex: -1
            textRole: "name"
            onActivated: {
                if (error)
                {
                    error.errorType = model.data(model.index(currentIndex, 0));
                    errorConfiguration.markAsChanged();
                }
            }
            Component.onCompleted: {
                if (error && model)
                {
                    errorMenu.currentIndex = model.findIndex(error.errorType);
                }
            }
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "configure-results"
            onClicked: {
                productStackView.push(errorSettings);
            }
        }
        Label {
            text: qsTr("Maximum Limit:")
        }
        TextField {
            id: maxLimitField
            Layout.fillWidth: true
            selectByMouse: true
            text: error ? error.maxLimit.toLocaleString(locale, 'f', 3) : ""
            validator: DoubleValidator {
                bottom: error ? error.minLimit : -100000
            }
            onEditingFinished: {
                if (error)
                {
                    error.maxLimit = Number.fromLocaleString(locale, text);
                    errorConfiguration.markAsChanged();
                }
            }
            palette.text: maxLimitField.acceptableInput ? "black" : "red"
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "list-add"
            onClicked: {
                if (error)
                {
                    error.maxLimit += 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "menu_new_sep"
            enabled: error.maxLimit - error.minLimit >= 0.1
            onClicked: {
                if (error)
                {
                    error.maxLimit -= 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        Label {
            text: qsTr("Minimum Limit:")
        }
        TextField {
            id: minLimitField
            Layout.fillWidth: true
            selectByMouse: true
            text: error ? error.minLimit.toLocaleString(locale, 'f', 3) : ""
            validator: DoubleValidator {
                top: error ? error.maxLimit : 100000
            }
            onEditingFinished: {
                if (error)
                {
                    error.minLimit = Number.fromLocaleString(locale, text);
                    errorConfiguration.markAsChanged();
                }
            }
            palette.text: minLimitField.acceptableInput ? "black" : "red"
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "list-add"
            enabled: error.maxLimit - error.minLimit >= 0.1
            onClicked: {
                if (error)
                {
                    error.minLimit += 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "menu_new_sep"
            onClicked: {
                if (error)
                {
                    error.minLimit -= 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        Label {
            text: qsTr("Shift:")
        }
        TextField {
            id: shiftField
            Layout.fillWidth: true
            selectByMouse: true
            text: error ? error.shift.toLocaleString(locale, 'f', 3) : ""
            validator: DoubleValidator {
                            top: error.maxLimit - error.highestMax
                            bottom: error.minLimit - error.lowestMin
            }
            onEditingFinished: {
                if (error)
                {
                    error.shift = Number.fromLocaleString(locale, text);
                    errorConfiguration.markAsChanged();
                }
            }
            palette.text: shiftField.acceptableInput ? "black" : "red"
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "list-add"
                        enabled: Math.abs(error.maxLimit - error.highestMax - error.shift) >= 0.1
            onClicked: {
                if (error)
                {
                    error.shift += 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "menu_new_sep"
                        enabled:  Math.abs(error.minLimit - error.lowestMin - error.shift) >= 0.1
            onClicked: {
                if (error)
                {
                    error.shift -= 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        RowLayout {
            Layout.columnSpan: 4

            Repeater {
                model: intervalErrorConfigModel

                GridLayout {
                    Layout.fillWidth: true
                    columns: 5 + (intervalErrorConfigModel.qualityNormAvailable && intervalErrorConfigModel.qualityNormResultAvailable && !errorConfiguration.souvisPreInspectionEnabled ? 1 : 0)

                    Label {
                        Layout.columnSpan: 5
                        Layout.fillWidth: true
                        padding: 5
                        text: qsTr("Level %1").arg(index + 1)
                        font.bold : true
                        background: Rectangle {
                            color: model.color
                        }
                    }
                    Label {
                        Layout.preferredWidth: 100
                        padding: 5
                        text: qsTr("QN")
                        font.bold : true
                        visible: intervalErrorConfigModel.qualityNormAvailable && intervalErrorConfigModel.qualityNormResultAvailable && !errorConfiguration.souvisPreInspectionEnabled
                        background: Rectangle {
                            color: model.color
                        }
                    }
                    Label {
                        text: qsTr("Maximum:")
                        leftPadding: 10
                    }
                    TextField {
                        id: maxField
                        Layout.fillWidth: true
                        selectByMouse: true
                        text: model.max.toLocaleString(locale, 'f', 3)
                        validator: DoubleValidator {
                            top: error ? error.maxLimit - error.shift : 100000
                            bottom: model.min
                        }
                        onEditingFinished: model.max = Number.fromLocaleString(locale, text);
                        palette.text: maxField.acceptableInput ? "black" : "red"
                    }
                    ToolButton {
                        display: AbstractButton.IconOnly
                        icon.name: "list-add"
                        enabled: Math.abs(error.maxLimit - model.max - error.shift) >= 0.1
                        onClicked: {
                            if (error)
                            {
                                model.max += 0.1;
                            }
                        }
                    }
                    ToolButton {
                        display: AbstractButton.IconOnly
                        icon.name: "menu_new_sep"
                        enabled: model.max - model.min >= 0.1
                        onClicked: {
                            if (error)
                            {
                                model.max -= 0.1;
                            }
                        }
                    }
                    ToolButton {
                        display: AbstractButton.IconOnly
                        icon.name: "go-top"
                        enabled: !intervalErrorConfigModel.visualReference.isEmpty()
                        onClicked: intervalErrorConfigModel.setMaxFromReference(index);
                    }
                    TextField {
                        Layout.preferredWidth: 100
                        visible: intervalErrorConfigModel.qualityNormAvailable && intervalErrorConfigModel.qualityNormResultAvailable && !errorConfiguration.souvisPreInspectionEnabled
                        text: model.qnMax.toLocaleString(locale, 'f', 3)
                        readOnly: true
                        background: Rectangle {
                            border.width: 1
                            color: "gainsboro"
                            border.color: palette.mid
                        }
                    }
                    Label {
                        text: qsTr("Minimum:")
                        leftPadding: 10
                    }
                    TextField {
                        id: minField
                        Layout.fillWidth: true
                        selectByMouse: true
                        text: model.min.toLocaleString(locale, 'f', 3)
                        validator: DoubleValidator {
                            bottom: error ? error.minLimit - error.shift : -100000
                            top: model.max
                        }
                        onEditingFinished: model.min = Number.fromLocaleString(locale, text);
                        palette.text: minField.acceptableInput ? "black" : "red"
                    }
                    ToolButton {
                        display: AbstractButton.IconOnly
                        icon.name: "list-add"
                        enabled: model.max - model.min >= 0.1
                        onClicked: {
                            if (error)
                            {
                                model.min += 0.1;
                            }
                        }
                    }
                    ToolButton {
                        display: AbstractButton.IconOnly
                        icon.name: "menu_new_sep"
                        enabled: Math.abs(error.minLimit - model.min - error.shift) >= 0.1
                        onClicked: {
                            if (error)
                            {
                                model.min -= 0.1;
                            }
                        }
                    }
                    ToolButton {
                        display: AbstractButton.IconOnly
                        icon.name: "go-bottom"
                        enabled: !intervalErrorConfigModel.visualReference.isEmpty()
                        onClicked: intervalErrorConfigModel.setMinFromReference(index);
                    }
                    TextField {
                        Layout.preferredWidth: 100
                        visible: intervalErrorConfigModel.qualityNormAvailable && intervalErrorConfigModel.qualityNormResultAvailable && !errorConfiguration.souvisPreInspectionEnabled
                        text: model.qnMin.toLocaleString(locale, 'f', 3)
                        readOnly: true
                        background: Rectangle {
                            border.width: 1
                            color: "gainsboro"
                            border.color: palette.mid
                        }
                    }
                    Label {
                        text: qsTr("Threshold%1").arg(error.showSecondThreshold ? " A:" : ":")
                        leftPadding: 10
                    }
                    TextField {
                        id: thresholdField
                        Layout.columnSpan: 4
                        Layout.fillWidth: true
                        selectByMouse: true
                        text: model.threshold.toLocaleString(locale, 'f', 3)
                        validator: DoubleValidator {
                            bottom: 0
                        }
                        onEditingFinished: model.threshold = Number.fromLocaleString(locale, text);
                        palette.text: thresholdField.acceptableInput ? "black" : "red"
                    }
                    TextField {
                        Layout.preferredWidth: 100
                        visible: intervalErrorConfigModel.qualityNormAvailable && intervalErrorConfigModel.qualityNormResultAvailable && !errorConfiguration.souvisPreInspectionEnabled
                        text: model.qnThreshold.toLocaleString(locale, 'f', 3)
                        readOnly: true
                        background: Rectangle {
                            border.width: 1
                            color: "gainsboro"
                            border.color: palette.mid
                        }
                    }
                    Label {
                        text: qsTr("Threshold B:")
                        leftPadding: 10
                        visible: error.showSecondThreshold
                    }
                    TextField {
                        id: secondThresholdField
                        Layout.columnSpan: 4
                        Layout.fillWidth: true
                        selectByMouse: true
                        text: model.secondThreshold.toLocaleString(locale, 'f', 3)
                        visible: error.showSecondThreshold
                        validator: DoubleValidator {
                            bottom: 0
                        }
                        onEditingFinished: model.secondThreshold = Number.fromLocaleString(locale, text);
                        palette.text: secondThresholdField.acceptableInput ? "black" : "red"
                    }
                    TextField {
                        Layout.preferredWidth: 100
                        visible: error.showSecondThreshold && intervalErrorConfigModel.qualityNormAvailable && intervalErrorConfigModel.qualityNormResultAvailable && !errorConfiguration.souvisPreInspectionEnabled
                        text: model.qnSecondThreshold.toLocaleString(locale, 'f', 3)
                        readOnly: true
                        background: Rectangle {
                            border.width: 1
                            color: "gainsboro"
                            border.color: palette.mid
                        }
                    }


                    Component.onCompleted: {
                        plotter.plotter.addDataSet(model.lower);
                        plotter.plotter.addDataSet(model.upper);
                        plotter.plotter.addDataSet(model.shiftedLower);
                        plotter.plotter.addDataSet(model.shiftedUpper);
                    }
                }
            }
        }

        RowLayout {
            Layout.columnSpan: 4

            InstanceResultListing {
                Layout.minimumWidth: 200
                Layout.maximumWidth: referenceListing.implicitListWidth
                Layout.fillHeight: true

                id: referenceListing

                monitoring: errorConfiguration.visible
                seam: currentSeam
                resultType: error ? error.resultValue : -1

                onLoadingChanged: plotter.plotter.resetPlotterView()
            }

            PlotterChart {
                id: plotter

                Layout.fillWidth: true
                Layout.fillHeight: true

                xAxisController {
                    autoAdjustXAxis: true
                }
                yAxisController {
                    autoAdjustYAxis: true
                }
                controller {
                    configFilePath: GuiConfiguration.configFilePath
                }
                panningEnabled: !errorMenu.popup.visible && !valueMenu.popup.visible
                yLegendRightVisible: false
                xLegendUnitVisible: false
                yLeftLegendUnitVisible: false
                backgroundBorderColor: "white"

                onPlotterSettingsUpdated: errorConfiguration.plotterSettingsUpdated()

                Component.onCompleted: {
                    plotter.plotter.addDataSet(referenceListing.result);
                }
            }
        }
    }
}
