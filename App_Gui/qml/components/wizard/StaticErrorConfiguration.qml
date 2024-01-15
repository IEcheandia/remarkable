import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.plotter 1.0
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Control {
    property alias error: visualController.seamError
    property alias valueList: valueMenu.model
    property alias errorList: errorMenu.model
    property alias currentSeam: controller.currentSeam
    property alias currentProduct: referenceListing.currentProduct
    property alias errorConfigModel: configErrorsPage.errorSettingModel
    property alias type: typeLabel.text
    property alias screenshotTool: configErrorsPage.screenshotTool
    property alias loading: referenceListing.loading

    signal markAsChanged()
    signal updateSettings()
    signal plotterSettingsUpdated()

    id: errorConfiguration

    StaticErrorConfigController {
        id: visualController
    }

    onUpdateSettings: plotter.updateSettings()

    onErrorChanged: plotter.plotter.clear()

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

    ReferenceCurvesModel {
        id: controller
    }

    ReferenceResultTypeFilterModel {
        id: resultFilterModel
        sourceModel: controller
        resultType: error ? error.resultValue : -1
    }

    contentItem: GridLayout {
        anchors.fill: parent
        columns: 4

        Label {
            text: qsTr("Name:")
            Layout.preferredWidth: 100
        }
        TextField {
            id: nameField
            Layout.fillWidth: true
            Layout.columnSpan: 3
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
            Layout.preferredWidth: 100
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
            Layout.preferredWidth: 100
        }
        ComboBox {
            id: valueMenu
            Layout.fillWidth: true
            Layout.columnSpan: 3
            enabled: !errorConfiguration.loading
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
            Layout.preferredWidth: 100
        }
        ComboBox {
            id: errorMenu
            Layout.fillWidth: true
            Layout.columnSpan: 2
            currentIndex: -1
            textRole: "name"
            enabled: !errorConfiguration.loading
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
            enabled: !errorConfiguration.loading
            onClicked: {
                productStackView.push(errorSettings);
            }
        }
        RowLayout {
            id: minMaxRow
            Layout.columnSpan: 4
            spacing: 10
            GridLayout {
                Layout.preferredWidth: (minMaxRow.width - 10) / 2
                columns: 5
                Label {
                    text: qsTr("Maximum:")
                    Layout.preferredWidth: 100
                }
                TextField {
                    id: maxField
                    Layout.fillWidth: true
                    selectByMouse: true
                    text: error ? error.max.toLocaleString(locale, 'f', 3) : ""
                    enabled: !errorConfiguration.loading
                    validator: DoubleValidator {
                        top: error ? error.maxLimit - error.shift : 100000
                        bottom: error ? error.min : 0
                    }
                    onEditingFinished: {
                        if (error)
                        {
                            error.max = Number.fromLocaleString(locale, text);
                            errorConfiguration.markAsChanged();
                        }
                    }
                    palette.text: maxField.acceptableInput ? "black" : "red"
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "list-add"
                    enabled: error && (Math.abs(error.maxLimit - error.max - error.shift) >= 0.1) && !errorConfiguration.loading
                    onClicked: {
                        if (error)
                        {
                            error.max += 0.1;
                            errorConfiguration.markAsChanged();
                        }
                    }
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "menu_new_sep"
                    enabled: error && (error.max - error.min >= 0.1) && !errorConfiguration.loading
                    onClicked: {
                        if (error)
                        {
                            error.max -= 0.1;
                            errorConfiguration.markAsChanged();
                        }
                    }
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "go-top"
                    enabled: !visualController.visualReference.isEmpty() && !errorConfiguration.loading
                    onClicked: {
                        if (error)
                        {
                            visualController.setMaxFromReference();
                            errorConfiguration.markAsChanged();
                        }
                    }
                }
                Label {
                    text: qsTr("Minimum:")
                    Layout.preferredWidth: 100
                }
                TextField {
                    id: minField
                    Layout.fillWidth: true
                    selectByMouse: true
                    text: error ? error.min.toLocaleString(locale, 'f', 3) : ""
                    enabled: !errorConfiguration.loading
                    validator: DoubleValidator {
                        top: error ? error.max : 0
                        bottom: error ? error.minLimit - error.shift : -100000
                    }
                    onEditingFinished: {
                        if (error)
                        {
                            error.min = Number.fromLocaleString(locale, text);
                            errorConfiguration.markAsChanged();
                        }
                    }
                    palette.text: minField.acceptableInput ? "black" : "red"
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "list-add"
                    enabled: error && (error.max - error.min >= 0.1) && !errorConfiguration.loading
                    onClicked: {
                        if (error)
                        {
                            error.min += 0.1;
                            errorConfiguration.markAsChanged();
                        }
                    }
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "menu_new_sep"
                    enabled: error && (Math.abs(error.minLimit - error.min - error.shift) >= 0.1) && !errorConfiguration.loading
                    onClicked: {
                        if (error)
                        {
                            error.min -= 0.1;
                            errorConfiguration.markAsChanged();
                        }
                    }
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "go-bottom"
                    enabled: !visualController.visualReference.isEmpty() && !errorConfiguration.loading
                    onClicked: {
                        if (error)
                        {
                            visualController.setMinFromReference();
                            errorConfiguration.markAsChanged();
                        }
                    }
                }
            }
            GridLayout {
                Layout.preferredWidth: (minMaxRow.width - 10) / 2
                columns: 4
                Label {
                    text: qsTr("Limit:")
                }
                TextField {
                    id: maxLimitField
                    Layout.fillWidth: true
                    selectByMouse: true
                    text: error ? error.maxLimit.toLocaleString(locale, 'f', 3) : ""
                    enabled: !errorConfiguration.loading
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
                    enabled: !errorConfiguration.loading
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
                    enabled: error && (error.maxLimit - error.minLimit >= 0.1) && !errorConfiguration.loading
                    onClicked: {
                        if (error)
                        {
                            error.maxLimit -= 0.1;
                            errorConfiguration.markAsChanged();
                        }
                    }
                }
                Label {
                    text: qsTr("Limit:")
                }
                TextField {
                    id: minLimitField
                    Layout.fillWidth: true
                    selectByMouse: true
                    text: error ? error.minLimit.toLocaleString(locale, 'f', 3) : ""
                    enabled: !errorConfiguration.loading
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
                    enabled: error && (error.maxLimit - error.minLimit >= 0.1) && !errorConfiguration.loading
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
                    enabled: !errorConfiguration.loading
                    onClicked: {
                        if (error)
                        {
                            error.minLimit -= 0.1;
                            errorConfiguration.markAsChanged();
                        }
                    }
                }
            }
        }
        Label {
            text: qsTr("Shift:")
            Layout.preferredWidth: 100
        }
        TextField {
            id: shiftField
            Layout.fillWidth: true
            selectByMouse: true
            text: error ? error.shift.toLocaleString(locale, 'f', 3) : ""
            enabled: !errorConfiguration.loading
            validator: DoubleValidator {
                top: error ? error.maxLimit - error.max : Number.MAX_VALUE
                bottom: error ? error.minLimit - error.min : Number.MIN_VALUE
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
            enabled: error && (Math.abs(error.maxLimit - error.max - error.shift) >= 0.1) && !errorConfiguration.loading
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
            enabled: error && (Math.abs(error.minLimit - error.min - error.shift) >= 0.1) && !errorConfiguration.loading
            onClicked: {
                if (error)
                {
                    error.shift -= 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        Label {
            text: qsTr("Threshold%1:").arg(error && error.showSecondThreshold ? " A" : "")
            Layout.preferredWidth: 100
        }
        TextField {
            id: thresholdField
            Layout.fillWidth: true
            selectByMouse: true
            text: error ? error.threshold.toLocaleString(locale, 'f', 3) : ""
            enabled: !errorConfiguration.loading
            validator: DoubleValidator {
                bottom: 0
            }
            onEditingFinished: {
                if (error)
                {
                    error.threshold = Number.fromLocaleString(locale, text);
                    errorConfiguration.markAsChanged();
                }
            }
            palette.text: thresholdField.acceptableInput ? "black" : "red"
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "list-add"
            enabled: !errorConfiguration.loading
            onClicked: {
                if (error)
                {
                    error.threshold += 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "menu_new_sep"
            enabled: error && (error.threshold >= 0.1) && !errorConfiguration.loading
            onClicked: {
                if (error)
                {
                    error.threshold -= 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        Label {
            text: qsTr("Threshold B:")
            Layout.preferredWidth: 100
            visible: error && error.showSecondThreshold
        }
        TextField {
            id: secondThresholdField
            visible: error && error.showSecondThreshold
            Layout.fillWidth: true
            selectByMouse: true
            text: error ? error.secondThreshold.toLocaleString(locale, 'f', 3) : ""
            enabled: !errorConfiguration.loading
            validator: DoubleValidator {
                bottom: 0
            }
            onEditingFinished: {
                if (error)
                {
                    error.secondThreshold = Number.fromLocaleString(locale, text);
                    errorConfiguration.markAsChanged();
                }
            }
            palette.text: secondThresholdField.acceptableInput ? "black" : "red"
        }
        ToolButton {
            visible: error && error.showSecondThreshold
            display: AbstractButton.IconOnly
            icon.name: "list-add"
            enabled: !errorConfiguration.loading
            onClicked: {
                if (error)
                {
                    error.secondThreshold += 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        ToolButton {
            visible: error && error.showSecondThreshold
            display: AbstractButton.IconOnly
            icon.name: "menu_new_sep"
            enabled: error && (error.secondThreshold >= 0.1) && !errorConfiguration.loading
            onClicked: {
                if (error)
                {
                    error.secondThreshold -= 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }

        RowLayout {
            Layout.columnSpan: 4

            InstanceResultListing {
                Layout.maximumWidth: referenceListing.implicitListWidth
                Layout.fillHeight: true

                id: referenceListing

                monitoring: errorConfiguration.visible
                seam: currentSeam
                resultType: error ? error.resultValue : -1

                onLoadingChanged: plotter.plotter.resetPlotterView()
            }

            PlotterChart {
                Layout.fillWidth: true
                Layout.fillHeight: true

                id: plotter

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
                yRightLegendUnitVisible: false
                backgroundBorderColor: "white"

                onPlotterSettingsUpdated: errorConfiguration.plotterSettingsUpdated()

                Component.onCompleted: {
                    plotter.plotter.addDataSet(visualController.visualReference);
                    plotter.plotter.addDataSet(visualController.lowerBoundary);
                    plotter.plotter.addDataSet(visualController.upperBoundary);
                    plotter.plotter.addDataSet(visualController.shiftedLowerBoundary);
                    plotter.plotter.addDataSet(visualController.shiftedUpperBoundary);
                    plotter.plotter.addDataSet(referenceListing.result);
                }
            }
        }
    }
}
