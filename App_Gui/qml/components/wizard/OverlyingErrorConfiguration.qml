import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.plotter 1.0
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Control {
    id: errorConfiguration
    property var error: null
    property alias type: typeLabel.text
    property alias valueList: valueMenu.model
    property alias errorList: errorMenu.model
    property alias errorConfigModel: configErrorsPage.errorSettingModel
    property alias isTypeError: valueMenu.visible
    property alias screenshotTool: configErrorsPage.screenshotTool

    signal markAsChanged()

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
        columns: 3

        Label {
            text: qsTr("Name:")
        }
        TextField {
            id: nameField
            Layout.fillWidth: true
            Layout.columnSpan: 2
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
            Layout.columnSpan: 2
            Layout.preferredHeight: nameField.height
            verticalAlignment: Text.AlignVCenter
        }
        Label {
            text: qsTr("Value:")
            visible: valueMenu.visible
        }
        ComboBox {
            id: valueMenu
            Layout.fillWidth: true
            Layout.columnSpan: 2
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
            text: qsTr("Threshold:")
        }
        TextField {
            id: thresholdField
            Layout.fillWidth: true
            Layout.columnSpan: 2
            selectByMouse: true
            text: error ? error.threshold : 0
            validator: IntValidator {
                bottom: 0
            }
            onEditingFinished: {
                if (error)
                {
                    error.threshold = parseInt(text);
                    errorConfiguration.markAsChanged();
                }
            }
            palette.text: thresholdField.acceptableInput ? "black" : "red"
        }
        Item {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
