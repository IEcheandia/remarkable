import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication

Control {
    id: seamIntervalLevels

    property alias currentSeam: errorModel.currentSeam
    property var currentProduct: null
    property alias errorConfigModel: errorSettingFilterModel.sourceModel
    property alias resultsConfigModel: seamErrorValueFilterModel.sourceModel
    property alias attributeModel: errorModel.attributeModel
    property alias qualityNorm: errorModel.qualityNorm
    property bool souvisPreInspectionEnabled: false
    property var screenshotTool: null
    property alias graphModel: seamErrorValueFilterModel.graphModel
    property alias subGraphModel: seamErrorValueFilterModel.subGraphModel

    signal markAsChanged()
    signal plotterSettingsUpdated();
    signal updateSettings();

    SeamErrorValueFilterModel {
        id: seamErrorValueFilterModel
        currentSeam: seamIntervalLevels.currentSeam
    }

    IntervalErrorModel {
        id: errorModel
        onMarkAsChanged: seamIntervalLevels.markAsChanged()
    }

    IntervalErrorFilterModel {
        id: errorFilterModel
        sourceModel: errorModel
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
            controller: errorModel
            staticConfiguration: errorDetailComponent
            interval: true
            screenshotTool: seamIntervalLevels.screenshotTool
        }
    }

    Component {
        id: errorDetailComponent
        BackButtonGroupBox {
            id: errorDetailGroup
            property alias error: configuration.error
            property alias type: configuration.type
            property alias qualityNorm: configuration.qualityNorm

            title: qsTr("Configure Interval Error \"%1\"").arg(error.name)

            Connections {
                target: seamIntervalLevels
                function onCurrentProductChanged() {
                    if (errorDetailGroup.visible)
                    {
                        errorDetailGroup.back();
                    }
                }
            }

            ColumnLayout {
                anchors.fill: parent
                IntervalErrorConfiguration {
                    id: configuration

                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    valueList: seamErrorValueFilterModel
                    errorList: errorSettingFilterModel
                    currentProduct: seamIntervalLevels.currentProduct
                    currentSeam: seamIntervalLevels.currentSeam
                    errorConfigModel: seamIntervalLevels.errorConfigModel
                    souvisPreInspectionEnabled: seamIntervalLevels.souvisPreInspectionEnabled
                    onMarkAsChanged: seamIntervalLevels.markAsChanged()
                    onPlotterSettingsUpdated: seamIntervalLevels.plotterSettingsUpdated()

                    Connections {
                        target: seamIntervalLevels
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
                        var dialog = deleteDialog.createObject(seamIntervalLevels, {"error": configuration.error});
                        dialog.open();
                    }
                }
            }
        }
    }

    Component {
        id: deleteDialog
        DeleteErrorDialog {
            controller: errorModel
        }
    }

    ColumnLayout {
        anchors.fill: parent
        RowLayout {
            Layout.fillWidth: true
            Item {
                Layout.fillWidth: true
            }
            Repeater {
                model: IntervalErrorConfigModel {}
                Rectangle {
                    color: model.color
                    Layout.preferredWidth: 300
                    Layout.preferredHeight: grid.implicitHeight
                    GridLayout {
                        id: grid
                        columns: 3
                        anchors.fill: parent
                        columnSpacing: 0
                        Label {
                            Layout.fillWidth: true
                            Layout.columnSpan: 3
                            Layout.topMargin: 5
                            horizontalAlignment: Text.AlignHCenter
                            font.bold: true
                            text: qsTr("Level %1").arg(index + 1)
                        }
                        Label {
                            Layout.preferredWidth: 80
                            Layout.rightMargin: 10
                            Layout.bottomMargin: 5
                            horizontalAlignment: Text.AlignHCenter
                            text: qsTr("Max")
                            font.pixelSize: 14
                        }
                        Label {
                            Layout.preferredWidth: 80
                            Layout.bottomMargin: 5
                            horizontalAlignment: Text.AlignHCenter
                            text: qsTr("Min")
                            font.pixelSize: 14
                        }
                        Label {
                            Layout.preferredWidth: 130
                            Layout.leftMargin: 10
                            Layout.bottomMargin: 5
                            horizontalAlignment: Text.AlignHCenter
                            text: qsTr("Threshold")
                            font.pixelSize: 14
                        }
                        Label {
                            Layout.preferredWidth: 80
                            Layout.rightMargin: 10
                            Layout.bottomMargin: 5
                            horizontalAlignment: Text.AlignHCenter
                            visible: errorModel.qualityNormAvailable && !seamIntervalLevels.souvisPreInspectionEnabled
                            text: qsTr("QN Max")
                            font.pixelSize: 14
                        }
                        Label {
                            Layout.preferredWidth: 80
                            Layout.bottomMargin: 5
                            horizontalAlignment: Text.AlignHCenter
                            visible: errorModel.qualityNormAvailable && !seamIntervalLevels.souvisPreInspectionEnabled
                            text: qsTr("QN Min")
                            font.pixelSize: 14
                        }
                        Label {
                            Layout.preferredWidth: 130
                            Layout.leftMargin: 10
                            Layout.bottomMargin: 5
                            horizontalAlignment: Text.AlignHCenter
                            visible: errorModel.qualityNormAvailable && !seamIntervalLevels.souvisPreInspectionEnabled
                            text: qsTr("QN Threshold")
                            font.pixelSize: 14
                        }
                    }
                }
            }
        }
        ListView {
            property int selectedRow: -1
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: intervalErrorsList
            contentWidth: width
            clip: true
            model: errorFilterModel
            spacing: 5

            onHeightChanged: {
                if (selectedRow != -1)
                {
                    intervalErrorsList.positionViewAtIndex(selectedRow, ListView.Contain);
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
                        productStackView.push(errorDetailComponent, {"qualityNorm": errorModel.qualityNorm, "error": model.error, "type": model.type});
                    }
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "edit-delete"
                    palette.button: "white"
                    onClicked: {
                        var dialog = deleteDialog.createObject(seamIntervalLevels, {"error": model.error});
                        dialog.popOnAccept = false;
                        dialog.open();
                    }
                }

                Repeater {
                    id: repeater
                    property int rowIndex: index
                    model: IntervalErrorSimpleConfigModel {
                        id: intervalConfigModel
                        onDataChanged: seamIntervalLevels.markAsChanged()
                        intervalError: model.error
                        qualityNormResult: model.qualityNormResult
                        currentSeam: errorModel.currentSeam

                        property Connections qnButtonConnection: Connections {
                            target: buttonRepeater
                            function onSetQualityNorm() {
                                intervalConfigModel.setValueFromQualityNorms(level)
                            }
                        }
                    }

                    Rectangle {
                        Layout.fillHeight: true
                        Layout.preferredWidth: 300
                        Layout.minimumHeight: rectangleGrid.implicitHeight
                        border.width: 0
                        color: model.color
                        GridLayout {
                            id: rectangleGrid
                            anchors.fill: parent

                            columns: error.showSecondThreshold ? 4 : 3
                            columnSpacing: 0
                            rowSpacing: 1

                            TextField {
                                id: maxField
                                Layout.preferredWidth: 80
                                Layout.fillHeight: true
                                Layout.minimumHeight: implicitHeight
                                selectByMouse: true
                                horizontalAlignment: Text.AlignHCenter
                                validator: DoubleValidator {
                                    bottom: model.min
                                    top: 100000
                                }
                                text: model.max.toLocaleString(locale, 'f', 3)
                                onEditingFinished: model.max = Number.fromLocaleString(locale, text)
                                onActiveFocusChanged: {
                                    if (activeFocus) {
                                        intervalErrorsList.selectedRow = repeater.rowIndex;
                                        intervalErrorsList.positionViewAtIndex(intervalErrorsList.selectedRow, ListView.Contain);
                                    }
                                }
                                palette.text: maxField.acceptableInput ? "black" : "red"
                            }
                            TextField {
                                id: minField
                                Layout.preferredWidth: 80
                                Layout.fillHeight: true
                                Layout.minimumHeight: implicitHeight
                                selectByMouse: true
                                horizontalAlignment: Text.AlignHCenter
                                validator: DoubleValidator {
                                    bottom: -100000
                                    top: model.max
                                }
                                text: model.min.toLocaleString(locale, 'f', 3)
                                onEditingFinished: model.min = Number.fromLocaleString(locale, text)
                                onActiveFocusChanged: {
                                    if (activeFocus) {
                                        intervalErrorsList.selectedRow = repeater.rowIndex;
                                        intervalErrorsList.positionViewAtIndex(intervalErrorsList.selectedRow, ListView.Contain);
                                    }
                                }
                                palette.text: minField.acceptableInput ? "black" : "red"
                            }
                            TextField {
                                id: thresholdField
                                Layout.preferredWidth: error.showSecondThreshold ? 65 : 130
                                Layout.fillHeight: true
                                Layout.minimumHeight: implicitHeight
                                selectByMouse: true
                                horizontalAlignment: Text.AlignHCenter
                                validator: DoubleValidator {
                                    bottom: 0
                                }
                                text: model.threshold.toLocaleString(locale, 'f', 3)
                                onEditingFinished: model.threshold = Number.fromLocaleString(locale, text)
                                onActiveFocusChanged: {
                                    if (activeFocus) {
                                        intervalErrorsList.selectedRow = repeater.rowIndex;
                                        intervalErrorsList.positionViewAtIndex(intervalErrorsList.selectedRow, ListView.Contain);
                                    }
                                }
                                palette.text: thresholdField.acceptableInput ? "black" : "red"
                            }
                            TextField {
                                id: secondThresholdField
                                visible: error.showSecondThreshold
                                Layout.preferredWidth: 65
                                Layout.fillHeight: true
                                Layout.minimumHeight: implicitHeight
                                selectByMouse: true
                                horizontalAlignment: Text.AlignHCenter
                                validator: DoubleValidator {
                                    bottom: 0
                                }
                                text: model.secondThreshold.toLocaleString(locale, 'f', 3)
                                onEditingFinished: model.secondThreshold = Number.fromLocaleString(locale, text)
                                onActiveFocusChanged: {
                                    if (activeFocus) {
                                        intervalErrorsList.selectedRow = repeater.rowIndex;
                                        intervalErrorsList.positionViewAtIndex(intervalErrorsList.selectedRow, ListView.Contain);
                                    }
                                }
                                palette.text: secondThresholdField.acceptableInput ? "black" : "red"
                            }

                            Label {
                                Layout.preferredWidth: 80
                                text: model.qnMax.toLocaleString(locale, 'f', 3)
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                visible: errorModel.qualityNormAvailable && intervalConfigModel.qualityNormResultAvailable && !seamIntervalLevels.souvisPreInspectionEnabled
                                background: Rectangle {
                                    border.width: 1
                                    color: "gainsboro"
                                    border.color: palette.mid
                                }
                            }
                            Label {
                                Layout.preferredWidth: 80
                                text: model.qnMin.toLocaleString(locale, 'f', 3)
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                visible: errorModel.qualityNormAvailable && intervalConfigModel.qualityNormResultAvailable && !seamIntervalLevels.souvisPreInspectionEnabled
                                background: Rectangle {
                                    border.width: 1
                                    color: "gainsboro"
                                    border.color: palette.mid
                                }
                            }
                            Label {
                                Layout.preferredWidth: error.showSecondThreshold ? 65 : 130
                                text: model.qnThreshold.toLocaleString(locale, 'f', 3)
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                visible: errorModel.qualityNormAvailable && intervalConfigModel.qualityNormResultAvailable && !seamIntervalLevels.souvisPreInspectionEnabled
                                background: Rectangle {
                                    border.width: 1
                                    color: "gainsboro"
                                    border.color: palette.mid
                                }
                            }
                            Label {
                                Layout.preferredWidth: 65
                                text: model.qnSecondThreshold.toLocaleString(locale, 'f', 3)
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                                visible: error.showSecondThreshold && errorModel.qualityNormAvailable && intervalConfigModel.qualityNormResultAvailable && !seamIntervalLevels.souvisPreInspectionEnabled
                                background: Rectangle {
                                    border.width: 1
                                    color: "gainsboro"
                                    border.color: palette.mid
                                }
                            }
                        }
                    }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            Item {
                Layout.fillWidth: true
            }
            Repeater {
                id: buttonRepeater

                signal setQualityNorm(int level);

                model: IntervalErrorConfigModel {}
                RowLayout {
                    spacing: 0
                    visible: errorModel.qualityNormAvailable && !seamIntervalLevels.souvisPreInspectionEnabled
                    Button {
                        id: qnButton
                        Layout.preferredWidth: 300
                        text: qsTr("Set Quality Norm")
                        onClicked: buttonRepeater.setQualityNorm(index)
                    }
                    Rectangle {
                        Layout.preferredWidth: 10
                        Layout.preferredHeight: qnButton.implicitHeight
                        color: model.color
                    }
                }
            }
        }
        Button {
            id: newErrorButton
            Layout.alignment: Qt.AlignHCenter | Qt.AlignBottom
            text: qsTr("Add new Error")
            display: AbstractButton.TextBesideIcon
            icon.name: "list-add"
            onClicked: {
                var dialog = newErrorDialog.createObject(seamIntervalLevels);
                dialog.open();
            }
        }
    }
}
