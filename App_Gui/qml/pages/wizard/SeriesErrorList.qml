import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0

Item {
    id: errorPage
    property var currentProduct: null
    property alias currentSeamSeries: errorModel.currentSeamSeries
    property alias attributeModel: errorModel.attributeModel
    property alias errorConfigModel: errorSettingFilterModel.sourceModel
    property var screenshotTool: null

    signal markAsChanged()

    SeamSeriesErrorModel {
        id: errorModel
        onMarkAsChanged: errorPage.markAsChanged()
    }

    ResultSettingFilterModel {
        id: errorSettingFilterModel
    }

    Component {
        id: newErrorDialog
        NewOverlyingErrorDialog {
            anchors.centerIn: parent
            parent: Overlay.overlay
            height: 0.9 * parent.height
            width: 0.95 * height
            controller: errorModel
            configuration: errorConfigurationComponent
            screenshotTool: errorPage.screenshotTool
        }
    }

    Component {
        id: deleteDialog
        DeleteErrorDialog {
            controller: errorModel
        }
    }

    Component {
        id: errorConfigurationComponent
        BackButtonGroupBox {
            id: errorConfigurationGroup
            property alias error: configuration.error
            property alias type: configuration.type
            property alias isTypeError: configuration.isTypeError

            title: qsTr("Configure Error \"%1\"").arg(error.name)

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
                OverlyingErrorConfiguration {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: configuration
                    valueList: errorSettingFilterModel
                    errorList: errorSettingFilterModel
                    errorConfigModel: errorPage.errorConfigModel
                    screenshotTool: errorPage.screenshotTool
                    onMarkAsChanged: errorPage.markAsChanged()
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
                Layout.preferredWidth: 130
                Layout.preferredHeight: 40
                Label {
                    anchors {
                        fill: parent
                        rightMargin: 20
                    }
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: qsTr("Threshold")
                }
            }
        }
        ListView {
            property int selectedRow: -1
            id: errorsList
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            model: errorModel
            contentWidth: width
            spacing: 5

            onHeightChanged: {
                if (selectedRow != -1)
                {
                    errorsList.positionViewAtIndex(selectedRow, ListView.Contain);
                }
            }

            delegate: RowLayout {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                ItemDelegate {
                    Layout.fillWidth: true
                    text: model.error.name + " (" + model.type + ")"
                    icon.name: "application-menu"
                    onClicked: {
                        productStackView.push(errorConfigurationComponent, {"error": model.error, "type": model.type, "isTypeError": model.isTypeError});
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
                    Layout.preferredWidth: 130
                    border.width: 0
                    color: "lightgray"

                    TextField {
                        anchors {
                            fill: parent
                            rightMargin: 20
                        }
                        id: thresholdField
                        selectByMouse: true
                        validator: IntValidator {
                            bottom: 0
                        }
                        text: model.error.threshold
                        onEditingFinished: {
                            model.error.threshold = parseInt(text);
                            errorPage.markAsChanged();
                        }
                        onActiveFocusChanged: {
                            if (activeFocus) {
                                errorsList.selectedRow = index;
                            }
                        }
                        palette.text: thresholdField.acceptableInput ? "black" : "red"
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
        visible: errorsList.count == 0
        anchors.centerIn: parent
        text: qsTr("No Errors defined for this Series")
    }
}
