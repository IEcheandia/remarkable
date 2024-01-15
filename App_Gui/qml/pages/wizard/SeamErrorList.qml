import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0

Item {
    id: errorPage
    property alias currentSeam: errorController.currentSeam
    property var currentProduct: null
    property alias resultsConfigModel: seamErrorValueFilterModel.sourceModel
    property alias errorConfigModel: errorSettingFilterModel.sourceModel
    property alias attributeModel: errorController.attributeModel
    property var screenshotTool: null
    property alias graphModel: seamErrorValueFilterModel.graphModel
    property alias subGraphModel: seamErrorValueFilterModel.subGraphModel

    signal markAsChanged()
    signal plotterSettingsUpdated();
    signal updateSettings();

    SeamErrorValueFilterModel {
        id: seamErrorValueFilterModel
        currentSeam: errorPage.currentSeam
    }

    SeamErrorModel {
        id: errorController
        onMarkAsChanged: errorPage.markAsChanged()
    }

    ResultSettingFilterModel {
        id: errorSettingFilterModel
    }

    Component {
        id: newErrorDialog
        NewSimpleErrorDialog {
            anchors.centerIn: parent
            parent: Overlay.overlay
            height: 0.9 * parent.height
            controller: errorController
            staticConfiguration: errorDetailComponent
            referenceConfiguration: referenceErrorDetailComponent
            screenshotTool: errorPage.screenshotTool
        }
    }

    Component {
        id: deleteDialog
        DeleteErrorDialog {
            controller: errorController
        }
    }

    Component {
        id: errorDetailComponent
        BackButtonGroupBox {
            id: errorConfigurationGroup
            property alias error: configuration.error
            property alias type: configuration.type
            property bool saveButtonsEnabled: !configuration.loading

            title: qsTr("Configure Error \"%1\"").arg(error ? error.name : "")
            navigationEnabled: !configuration.loading

            Connections {
                target: errorPage
                function onCurrentProductChanged() {
                    if (errorConfigurationGroup.visible)
                    {
                        errorConfigurationGroup.back();
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                StaticErrorConfiguration {
                    id: configuration
                    valueList: seamErrorValueFilterModel
                    errorList: errorSettingFilterModel
                    currentProduct: errorPage.currentProduct
                    errorConfigModel: errorPage.errorConfigModel
                    currentSeam: errorPage.currentSeam
                    screenshotTool: errorPage.screenshotTool
                    onMarkAsChanged: errorPage.markAsChanged()
                    onPlotterSettingsUpdated: errorPage.plotterSettingsUpdated()
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Connections {
                        target: errorPage
                        function onUpdateSettings() {
                            configuration.updateSettings()
                        }
                    }
                }
                Button {
                    Layout.alignment: Qt.AlignHCenter
                    display: AbstractButton.TextBesideIcon
                    text: qsTr("Delete this Error")
                    icon.name: "edit-delete"
                    onClicked: {
                        var dialog = deleteDialog.createObject(errorPage, {"error": configuration.error});
                        dialog.open();
                    }
                }
            }
        }
    }

    Component {
        id: referenceErrorDetailComponent
        BackButtonGroupBox {
            id: referenceErrorConfigurationGroup
            property alias error: referenceConfiguration.error
            property alias type: referenceConfiguration.type
            property bool saveButtonsEnabled: !referenceConfiguration.loading

            title: qsTr("Configure Error \"%1\"").arg(error ? error.name : "")
            navigationEnabled: !referenceConfiguration.loading

            Connections {
                target: errorPage
                function onCurrentProductChanged() {
                    if (referenceErrorConfigurationGroup.visible)
                    {
                        referenceErrorConfigurationGroup.back();
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                ReferenceErrorConfiguration {
                    id: referenceConfiguration
                    valueList: seamErrorValueFilterModel
                    errorList: errorSettingFilterModel
                    currentProduct: errorPage.currentProduct
                    resultsConfigModel: errorPage.resultsConfigModel
                    errorConfigModel: errorPage.errorConfigModel
                    currentSeam: errorPage.currentSeam
                    screenshotTool: errorPage.screenshotTool
                    onMarkAsChanged: errorPage.markAsChanged()
                    onPlotterSettingsUpdated: errorPage.plotterSettingsUpdated()
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Connections {
                        target: errorPage
                        function onUpdateSettings() {
                            referenceConfiguration.updateSettings()
                        }
                    }
                }
                Button {
                    Layout.alignment: Qt.AlignHCenter
                    display: AbstractButton.TextBesideIcon
                    text: qsTr("Delete this Error")
                    icon.name: "edit-delete"
                    onClicked: {
                        var dialog = deleteDialog.createObject(errorPage, {"error": referenceConfiguration.error});
                        dialog.open();
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors {
            fill: parent
            topMargin: 0
        }
        RowLayout {
            Layout.fillWidth: true
            Item {
                Layout.fillWidth: true
            }
            Rectangle {
                color: "lightgray"
                Layout.preferredWidth: 450
                Layout.preferredHeight: 40
                RowLayout {
                    id: tableHeader
                    anchors.fill: parent
                    spacing: 0
                    Label {
                        Layout.preferredWidth: 140
                        Layout.rightMargin: 15
                        Layout.fillHeight: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Max")
                    }
                    Label {
                        Layout.preferredWidth: 140
                        Layout.fillHeight: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Min")
                    }
                    Label {
                        Layout.preferredWidth: 140
                        Layout.leftMargin: 15
                        Layout.fillHeight: true
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: qsTr("Threshold")
                    }
                }
            }
        }
        ListView {
            property int selectedRow: -1
            id: errorsListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: errorController
            contentWidth: width
            spacing: 5

            onHeightChanged: {
                if (selectedRow != -1)
                {
                    errorsListView.positionViewAtIndex(selectedRow, ListView.Contain);
                }
            }

            delegate: RowLayout {
                width: ListView.view.width
                ItemDelegate {
                    Layout.fillWidth: true
                    text: model.error.name + " (" + model.type + ")"
                    icon.name: "application-menu"
                    onClicked: {
                        if (model.error.boundaryType == SeamError.Static)
                        {
                            productStackView.push(errorDetailComponent, {"error": model.error, "type": model.type});
                        } else if (error.boundaryType == SeamError.Reference)
                        {
                            productStackView.push(referenceErrorDetailComponent, {"error": model.error, "type": model.name});
                        }
                    }
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "edit-delete"
                    palette.button: "white"
                    onClicked: {
                        var dialog = deleteDialog.createObject(errorPage, {"error": model.error});
                        dialog.popOnAccept = false;
                        dialog.open();
                    }
                }
                Rectangle {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 450
                    border.width: 0
                    color: "lightgray"
                    RowLayout {
                        anchors.fill: parent
                        spacing: 0
                        TextField {
                            id: referenceLabel
                            Layout.preferredWidth: 295
                            Layout.fillHeight: true
                            text: model.error.envelopeName
                            visible: model.error.boundaryType == SeamError.Reference
                            readOnly: true
                            palette.highlight: "gray"
                        }
                        TextField {
                            id: maxField
                            Layout.preferredWidth: 140
                            Layout.rightMargin: 15
                            Layout.fillHeight: true
                            selectByMouse: true
                            visible: model.error.boundaryType == SeamError.Static
                            validator: DoubleValidator {
                                bottom: model.error.min
                                top: 100000
                            }
                            text: model.error.max.toLocaleString(locale, 'f', 3)
                            onEditingFinished: {
                                model.error.max = Number.fromLocaleString(locale, text);
                                errorPage.markAsChanged();
                            }
                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    errorsListView.selectedRow = index;
                                }
                            }
                            palette.text: maxField.acceptableInput ? "black" : "red"
                        }
                        TextField {
                            id: minField
                            Layout.preferredWidth: 140
                            Layout.fillHeight: true
                            selectByMouse: true
                            visible: model.error.boundaryType == SeamError.Static
                            validator: DoubleValidator {
                                bottom: -100000
                                top: model.error.max
                            }
                            text: model.error.min.toLocaleString(locale, 'f', 3)
                            onEditingFinished: {
                                model.error.min = Number.fromLocaleString(locale, text);
                                errorPage.markAsChanged();
                            }
                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    errorsListView.selectedRow = index;
                                }
                            }
                            palette.text: minField.acceptableInput ? "black" : "red"
                        }
                        TextField {
                            id: thresholdField
                            Layout.preferredWidth: error.showSecondThreshold ? 67 : 140
                            Layout.leftMargin: 15
                            Layout.fillHeight: true
                            selectByMouse: true
                            validator: DoubleValidator {
                                bottom: 0
                            }
                            text: model.error.threshold.toLocaleString(locale, 'f', 3)
                            onEditingFinished: {
                                model.error.threshold = Number.fromLocaleString(locale, text);
                                errorPage.markAsChanged();
                            }
                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    errorsListView.selectedRow = index;
                                }
                            }
                            palette.text: thresholdField.acceptableInput ? "black" : "red"
                        }
                        TextField {
                            id: secondThresholdField
                            visible: error.showSecondThreshold
                            Layout.preferredWidth: 67
                            Layout.leftMargin: 6
                            Layout.fillHeight: true
                            selectByMouse: true
                            validator: DoubleValidator {
                                bottom: 0
                            }
                            text: model.error.secondThreshold.toLocaleString(locale, 'f', 3)
                            onEditingFinished: {
                                model.error.secondThreshold = Number.fromLocaleString(locale, text);
                                errorPage.markAsChanged();
                            }
                            onActiveFocusChanged: {
                                if (activeFocus) {
                                    errorsListView.selectedRow = index;
                                }
                            }
                            palette.text: secondThresholdField.acceptableInput ? "black" : "red"
                        }
                        
                    }
                }
            }
        }
        Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Add new Error")
            display: AbstractButton.TextBesideIcon
            icon.name: "list-add"
            onClicked: {
                var dialog = newErrorDialog.createObject(errorPage);
                dialog.open();
            }
        }
    }
    Label {
        visible: errorsListView.count == 0
        anchors.centerIn: parent
        text: qsTr("No Errors defined for this Seam")
    }
}
