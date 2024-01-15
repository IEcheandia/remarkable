import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.general 1.0

Item {
    id:root
    width: 1500
    height: 1600
    property var resultSettingModel: null
    property var screenshotTool: null
    property var highestEnumValue

    ResultSettingFilterModel {
        id: resultSettingsFilterModel
        sourceModel: resultSettingModel
        searchText: searchField.text
    }

    ResultTemplateModel{
        id: resultTemplateModel
    }

    ResultTemplateFilterModel{
        id: resultTemplateFilterModel
        qualityFaultCategory2: GuiConfiguration.qualityFaultCategory2
        resultSettingModel: resultSettingModel
        sourceModel: resultTemplateModel
    }

    Component {
        id: addResultsDialogComponent
        Dialog {
            id: addResultsDialog
            anchors.centerIn: parent
            modal: true
            title: qsTr("New Result")
            standardButtons: Dialog.Ok | Dialog.Cancel
            closePolicy: Popup.CloseOnEscape
            onAccepted: {
                resultSettingModel.createNewResult(nameField.text, enumField.text);
                destroy();
            }
            onRejected: {
                destroy();
            }

            header: DialogHeaderWithScreenshot {
                title: addResultsDialog.title
                screenshotTool: root.screenshotTool
            }

            ButtonGroup {
                id: typeGroup
            }

            GridLayout {
                anchors.fill: parent
                columns: 4
                columnSpacing: 40

                RowLayout {
                    Layout.fillWidth: true
                    Layout.columnSpan: 4
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Name")
                        font.bold: true
                    }
                    Label {
                        id: enumField
                        Layout.alignment: Qt.AlignRight
                        font.bold: true
                        text: highestEnumValue
                    }
                }
                TextField {
                    id: nameField
                    Layout.columnSpan: 4
                    Layout.fillWidth: true
                    selectByMouse: true
                    text: qsTr("User Defined")
                }
            }
            Component.onCompleted: {
                addResultsDialog.standardButton(Dialog.Ok);
                getHighestResultEnum();
            }

            function getHighestResultEnum(){

                highestEnumValue = resultSettingModel.highestEnumValue()
                if( highestEnumValue >= 50000 && highestEnumValue < 60000){
                    highestEnumValue  = resultSettingModel.highestEnumValue() + 1
                } else {
                    highestEnumValue = 50000;
                }
            }
            Connections {
                target: UserManagement
                function onCurrentUserChanged() {
                    addResultsDialog.reject()
                }
            }
        }
    }

    Component {
        id: deleteResultDialogComponent
        Dialog {
            id: deleteResultDialog
            property var modelIndex: null
            property var enumType: null
            property string name: ""
            anchors.centerIn: parent
            modal: true
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape
            onAccepted: {
                resultSettingModel.deleteResult(modelIndex, enumType);
                destroy();
            }
            onRejected: {
                destroy();
            }
            Label {
                font.bold: true
                text: qsTr("Delete result \"%1\" ? ").arg(name)
            }
            Connections {
                target: UserManagement
                function onCurrentUserChanged() {
                    deleteResultDialog.reject()
                }
            }
        }
    }

    Label {
        visible: resultConfigList.count == 0
        text: qsTr("No results available")
        font.bold: true
        anchors.centerIn: parent
    }

    ColumnLayout {
        anchors.fill: parent
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: spacing
            Layout.rightMargin: spacing
            Layout.topMargin: 5
            Label {
                text: qsTr("Sort on")
            }

            ComboBox {
                model: [qsTr("Name"), qsTr("Enum")]
                enabled: true
                onActivated: {
                    switch (index)
                    {
                        case 0:
                            resultSettingsFilterModel.sortOnType = ResultSetting.Name;
                            break;
                        case 1:
                            resultSettingsFilterModel.sortOnType = ResultSetting.EnumType;
                            break;
                        case 2:
                            resultSettingsFilterModel.sortOnType = ResultSetting.Visible;
                            break;
                        default:
                            resultSettingsFilterModel.sortOnType = ResultSetting.EnumType;
                            break;
                    }
                }
            }
            ToolButton {
                display: AbstractButton.IconOnly
                icon.name: resultSettingsFilterModel.sortAsc ? "view-sort-ascending" : "view-sort-descending"
                onClicked: {
                    resultSettingsFilterModel.sortAsc = !resultSettingsFilterModel.sortAsc;
                }
            }
            TextField {
                id: searchField
                selectByMouse: true
                placeholderText: qsTr("Search by name or enum")
                Layout.fillWidth: true
            }
        }

        ScrollView
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ListView {
                id: resultConfigList
                width: root.width
                implicitHeight: contentHeight
                model: resultSettingsFilterModel
                spacing: 10
                delegate: resultDelegate
            }

            Component {
                id: resultDelegate
                GroupBox {
                    title: qsTr("Enum %1").arg(enumType)
                    width: ListView.view.width
                    id: delegate
                    property var delegateIndex: index
                    ColumnLayout {
                        id: rootLayout
                        anchors.fill: parent
                        RowLayout {
                            id: nameLayout
                            implicitHeight: nameField.implicitHeight
                            Layout.fillWidth: true
                            Label {
                                text: qsTr("Name:")
                            }
                            TextField {
                                id: nameField
                                selectByMouse: true
                                text: name
                                Layout.fillWidth: true
                                onEditingFinished : {
                                    var modelIndex = resultSettingsFilterModel.index(index, 0);
                                    resultSettingModel.updateValue(resultSettingsFilterModel.mapToSource(modelIndex), text, ResultSetting.Name);
                                }
                            }
                            Button {
                                text: qsTr("Delete Result")
                                enabled: disabled > 0 ? false : true
                                display: AbstractButton.TextBesideIcon
                                icon.name: "edit-delete"
                                onClicked: {
                                    var modelIndex = resultSettingsFilterModel.index(index, 0);
                                    var dialog = deleteResultDialogComponent.createObject(root);
                                    dialog.modelIndex = resultSettingsFilterModel.mapToSource(modelIndex);
                                    dialog.enumType = enumType;
                                    dialog.name = name;
                                    dialog.open();
                                }
                            }
                            Label {
                                Layout.alignment: Qt.AlignRight
                                width: 5
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            GroupBox {
                                title: qsTr("Plotter minimum")
                                Layout.alignment: Qt.AlignTop
                                TextField {
                                    id: minField
                                    Layout.fillWidth: true
                                    text: min.toLocaleString(locale, 'f', 3)
                                    selectByMouse: true
                                    validator: DoubleValidator {
                                        bottom: -1000000.0
                                        top: max
                                    }
                                    onEditingFinished: {
                                        var modelIndex = resultSettingsFilterModel.index(index, 0);
                                        resultSettingModel.updateValue(resultSettingsFilterModel.mapToSource(modelIndex), Number.fromLocaleString(locale, text), ResultSetting.Min);
                                    }
                                    palette.text: minField.acceptableInput ? "black" : "red"
                                }
                            }

                            GroupBox {
                                title: qsTr("Plotter maximum")
                                Layout.alignment: Qt.AlignTop
                                TextField {
                                    id: maxField
                                    Layout.fillWidth: true
                                    text: max.toLocaleString(locale, 'f', 3)
                                    selectByMouse: true
                                    validator: DoubleValidator {
                                        bottom: min
                                        top: 1000000
                                    }
                                    onEditingFinished: {
                                        var modelIndex = resultSettingsFilterModel.index(index, 0);
                                        resultSettingModel.updateValue(resultSettingsFilterModel.mapToSource(modelIndex), Number.fromLocaleString(locale, text), ResultSetting.Max);
                                    }
                                    palette.text: maxField.acceptableInput ? "black" : "red"
                                }
                            }

                            GroupBox {
                                title: qsTr("Plotter number")
                                Layout.alignment: Qt.AlignTop
                                SpinBox {
                                    Layout.fillWidth: true
                                    from: 1
                                    to: 3
                                    stepSize: 1
                                    value: plotterNumber
                                    editable: true
                                    onValueModified: {
                                        var modelIndex = resultSettingsFilterModel.index(index, 0);
                                        resultSettingModel.updateValue(resultSettingsFilterModel.mapToSource(modelIndex), value, ResultSetting.PlotterNumber);
                                    }
                                    Component.onCompleted: {
                                        contentItem.selectByMouse = true;
                                    }
                                }
                            }

                            GroupBox {
                                title: qsTr("Plottable")
                                Layout.alignment: Qt.AlignTop
                                CheckBox {
                                    id: checkbox
                                    property alias value: checkbox.checked
                                    property bool updatesEnabled: false
                                    text: qsTr("On/Off")
                                    checked: plottable
                                    onToggled: {
                                        if (updatesEnabled) {
                                            var modelIndex = resultSettingsFilterModel.index(index, 0);
                                            resultSettingModel.updateValue(resultSettingsFilterModel.mapToSource(modelIndex), checked, ResultSetting.Plottable);
                                        }
                                    }
                                }
                                Component.onCompleted: {
                                    checkbox.updatesEnabled = true;
                                }
                            }

                            GroupBox {
                                title: qsTr("Line color")
                                Layout.alignment: Qt.AlignTop
                                ColorDialog {
                                    id: colorDialog
                                    showAlphaChannel: false
                                    anchors.centerIn: Overlay.overlay
                                    width: Overlay.overlay ? Overlay.overlay.width * 0.9 : 0
                                    height: Overlay.overlay ? Overlay.overlay.height * 0.9 : 0
                                    modal: true
                                    title: qsTr("Select color")
                                    property bool updatesEnabled: false
                                    onAccepted: {
                                        if (updatesEnabled) {
                                            var modelIndex = resultSettingsFilterModel.index(index, 0);
                                            resultSettingModel.updateValue(resultSettingsFilterModel.mapToSource(modelIndex), color, ResultSetting.LineColor);
                                        }
                                    }
                                }
                                ToolButton {
                                    id: buttonColorSelect
                                    icon.name: "color-picker"
                                    icon.color: lineColor
                                    display: AbstractButton.IconOnly
                                    ToolTip.text: qsTr("Select color for this data plot")
                                    ToolTip.visible: hovered
                                    onClicked: {
                                        colorDialog.color = lineColor;
                                        colorDialog.currentColor = lineColor;
                                        colorDialog.currentHue = hue;
                                        colorDialog.currentSaturation = saturation;
                                        colorDialog.currentLightness = lightness;
                                        colorDialog.crosshairsVisible = true;
                                        colorDialog.open();
                                    }
                                }
                                Component.onCompleted: {
                                    colorDialog.updatesEnabled = true;
                                }
                            }

                            GroupBox {
                                title: qsTr("Visible")
                                Layout.alignment: Qt.AlignTop
                                CheckBox {
                                    id: checkbox2
                                    property bool updatesEnabled: false
                                    text: qsTr("On/Off")
                                    checked: visibleItem
                                    onToggled: {
                                        if (updatesEnabled) {
                                            var modelIndex = resultSettingsFilterModel.index(index, 0);
                                            resultSettingModel.updateValue(resultSettingsFilterModel.mapToSource(modelIndex), checked, ResultSetting.Visible);
                                        }
                                    }
                                }
                                Component.onCompleted: {
                                    checkbox2.updatesEnabled = true;
                                }
                            }
                            GroupBox {
                                title: qsTr("Visualization")
                                Layout.alignment: Qt.AlignTop
                                ComboBox {
                                    model: [qsTr("Binary"), qsTr("2D (x/y)"), qsTr("3D (x/y/z)")]
                                    property bool updatesEnabled: false
                                    onActivated: {
                                        if (updatesEnabled) {
                                            var modelIndex = resultSettingsFilterModel.index(delegate.delegateIndex, 0);
                                            resultSettingModel.updateValue(resultSettingsFilterModel.mapToSource(modelIndex), index, ResultSetting.Visualization);
                                        }
                                    }

                                    Component.onCompleted: {
                                        currentIndex = visualization;
                                        updatesEnabled = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Add new Result")
            display: AbstractButton.TextBesideIcon
            icon.name: "list-add"
            onClicked: {
                var dialog = addResultsDialogComponent.createObject(root);
                dialog.open();
            }
        }
    }
}

