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
    property var resultsConfigModel: null
    property alias errorConfigModel: configErrorsPage.errorSettingModel
    property alias type: typeLabel.text
    property alias screenshotTool: configErrorsPage.screenshotTool
    property alias loading: referenceListing.loading

    signal markAsChanged()
    signal updateSettings()
    signal plotterSettingsUpdated()

    id: errorConfiguration

    ReferenceErrorConfigController {
        id: visualController
        currentProduct: errorConfiguration.currentProduct
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
            Layout.fillWidth: true
            Layout.columnSpan: 3
            currentIndex: -1
            textRole: "name"
            enabled: !errorConfiguration.loading
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
        Label {
            id: referenceLabel
            text: qsTr("Reference Curve:")
        }
        ComboBox {
            id: referenceComboBox
            Layout.fillWidth: true
            Layout.columnSpan: 3
            model: resultFilterModel
            enabled: !errorConfiguration.loading
            textRole: "name"
            onActivated: {
                if (error)
                {
                    error.envelope = model.data(model.index(currentIndex, 0), Qt.UserRole + 1);
                    errorConfiguration.markAsChanged();
                }
            }
            onCountChanged: {
                if (error && error.envelope)
                {
                    referenceComboBox.currentIndex = referenceComboBox.find(error.envelopeName);
                }
            }
        }
        CheckBox {
            Layout.fillWidth: true
            Layout.columnSpan: 4
            Layout.leftMargin: referenceLabel.width
            text: qsTr("Use Middle Curve As Reference")
            checked: error ? error.useMiddleCurveAsReference : false
            enabled: !errorConfiguration.loading
            onClicked: {
                error.useMiddleCurveAsReference = checked;
                errorConfiguration.markAsChanged();
            }

        }
        Label {
            text: qsTr("Upper Deviation:")
        }
        TextField {
            Layout.fillWidth: true
            selectByMouse: true
            text: error ? error.max.toLocaleString(locale, 'f', 3) : ""
            validator: DoubleValidator {
                bottom: 0
            }
            enabled: !errorConfiguration.loading
            onEditingFinished: {
                if (error)
                {
                    error.max = Number.fromLocaleString(locale, text);
                    errorConfiguration.markAsChanged();
                }
            }
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "list-add"
            enabled: !errorConfiguration.loading
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
            enabled: !errorConfiguration.loading
            onClicked: {
                if (error)
                {
                    error.max -= 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        Label {
            text: qsTr("Lower Deviation:")
        }
        TextField {
            Layout.fillWidth: true
            selectByMouse: true
            text: error ? error.min.toLocaleString(locale, 'f', 3) : ""
            enabled: !errorConfiguration.loading
            validator: DoubleValidator {
                bottom: 0
            }
            onEditingFinished: {
                if (error)
                {
                    error.min = Number.fromLocaleString(locale, text);
                    errorConfiguration.markAsChanged();
                }
            }
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "list-add"
            enabled: !errorConfiguration.loading
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
            enabled: !errorConfiguration.loading
            onClicked: {
                if (error)
                {
                    error.min -= 0.1;
                    errorConfiguration.markAsChanged();
                }
            }
        }
        Label {
            text: qsTr("Threshold%1:").arg(error && error.showSecondThreshold ? " A" : "")
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
            display: AbstractButton.IconOnly
            visible: error && error.showSecondThreshold
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
            display: AbstractButton.IconOnly
            visible: error && error.showSecondThreshold
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
                Layout.fillWidth: true

                id: referenceListing

                monitoring: errorConfiguration.visible
                seam: currentSeam
                resultType: error ? error.resultValue : -1
                triggerType: resultsConfigModel.isLwmType(error.resultValue) && errorConfiguration.currentProduct ? errorConfiguration.currentProduct.lwmTriggerSignalType : -1
                threshold: errorConfiguration.currentProduct ? errorConfiguration.currentProduct.lwmTriggerSignalThreshold : 0.0

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
                yRightLegendUnitVisible: false
                backgroundBorderColor: "white"

                onPlotterSettingsUpdated: errorConfiguration.plotterSettingsUpdated()

                Component.onCompleted: {
                    plotter.plotter.addDataSet(visualController.visualReference);
                    plotter.plotter.addDataSet(visualController.lowerShadow);
                    plotter.plotter.addDataSet(visualController.upperShadow);
                    plotter.plotter.addDataSet(visualController.lowerReference);
                    plotter.plotter.addDataSet(visualController.middleReference);
                    plotter.plotter.addDataSet(visualController.upperReference);
                    plotter.plotter.addDataSet(referenceListing.result);
                    plotter.plotter.addDataSet(referenceListing.trigger);
                }
            }
        }
    }
}

