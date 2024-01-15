import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.general 1.0

Item {
    id:configErrorPage
    property var errorSettingModel: null
    property var screenshotTool: null

    ResultSettingFilterModel {
        id: errorSettingFilterModel
        sourceModel: errorSettingModel
        searchText: searchField.text
        Component.onCompleted: {
            errorSettingFilterModel.updateUsedFlags();
        }
    }

    ErrorTemplateModel {
        id: errorTemplateModel
    }

    ErrorTemplateFilterModel {
        id: errorTemplateFilterModel
        qualityFaultCategory2: GuiConfiguration.qualityFaultCategory2
        errorSettingModel: errorSettingModel
        sourceModel: errorTemplateModel
    }

    Component {
        id: addErrorDialogComponent
        Dialog {
            id: addErrorDialog
            anchors.centerIn: parent
            modal: true
            title: qsTr("New Error Type:")
            standardButtons: Dialog.Ok | Dialog.Cancel
            closePolicy: Popup.CloseOnEscape
            onAccepted: {
                errorSettingModel.createNewError(nameField.text, enumField.text);
                destroy();
            }
            onRejected: {
                destroy();
            }

            header: DialogHeaderWithScreenshot {
                title: addErrorDialog.title
                screenshotTool: configErrorPage.screenshotTool
            }

            ButtonGroup {
                id: typeGroup
            }

            GridLayout {
                anchors.fill: parent
                columns: 4
                columnSpacing: 20
                Repeater {
                    model: errorTemplateFilterModel
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.columnSpan: index == 0 ? 4 : 1
                        Layout.margins: 0
                        RadioButton {
                            Layout.fillWidth: true
                            text: model.name
                            ButtonGroup.group: typeGroup
                            onClicked: {
                                nameField.text = model.name;
                                enumField.text = index == 0 ? Math.max(model.enumType, errorSettingModel.highestEnumValue() + 1) : model.enumType
                            }
                        }
                        Label {
                            text: index == 0 ? Math.max(model.enumType, errorSettingModel.highestEnumValue() + 1) : model.enumType
                            font.pixelSize: 10
                            Layout.alignment: Qt.AlignRight
                        }
                    }
                }
                RowLayout {
                    visible: typeGroup.checkState != Qt.Unchecked
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
                    }
                }
                TextField {
                    id: nameField
                    Layout.columnSpan: 4
                    Layout.fillWidth: true
                    visible: typeGroup.checkState != Qt.Unchecked
                    selectByMouse: true
                }
            }
            Component.onCompleted: {
                addErrorDialog.standardButton(Dialog.Ok).enabled = Qt.binding(function () { return typeGroup.checkState != Qt.Unchecked; });
            }
            Connections {
                target: UserManagement
                function onCurrentUserChanged() {
                    addErrorDialog.reject()
                }
            }
        }
    }

    Component {
        id: deleteErrorDialogComponent
        Dialog {
            id: deleteErrorDialog
            property var modelIndex: null
            property var enumType: null
            property string name: ""
            anchors.centerIn: parent
            modal: true
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape
            onAccepted: {
                errorSettingModel.deleteError(modelIndex, enumType);
                destroy();
            }
            onRejected: {
                destroy();
            }
            Label {
                font.bold: true
                text: qsTr("Delete error \"%1\" ? ").arg(name)
            }
            Connections {
                target: UserManagement
                onCurrentUserChanged: deleteErrorDialog.reject()
            }
        }
    }

    Label {
        visible: errorConfigList.count == 0
        text: qsTr("No errors avaiable")
        font.bold: true
        anchors.centerIn: parent
    }

    ColumnLayout {
        anchors{
            fill: parent
            margins: spacing
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 10
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
                            errorSettingFilterModel.sortOnType = ResultSetting.Name;
                            break;
                        case 1:
                            errorSettingFilterModel.sortOnType = ResultSetting.EnumType;
                            break;
                        default:
                            errorSettingFilterModel.sortOnType = ResultSetting.Name;
                            break;
                    }
                }
            }
            ToolButton {
                display: AbstractButton.IconOnly
                icon.name: errorSettingFilterModel.sortAsc ? "view-sort-ascending" : "view-sort-descending"
                onClicked: {
                    errorSettingFilterModel.sortAsc = !errorSettingFilterModel.sortAsc;
                }
            }
            TextField {
                id: searchField
                selectByMouse: true
                placeholderText: qsTr("Search by name")
                Layout.fillWidth: true
            }
        }

        ScrollView
        {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            ListView {
                id: errorConfigList
                width: configErrorPage.width
                implicitHeight: contentHeight
                model: errorSettingFilterModel
                spacing: 10
                delegate: errorDelegate
            }
            Component {
                id: errorDelegate
                GroupBox {
                    title: qsTr("Enum %1 %2").arg(enumType).arg(qualityName)
                    width: ListView.view.width
                    id: delegate
                    ColumnLayout {
                        anchors.fill: parent
                        RowLayout {
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
                                    var modelIndex = errorSettingFilterModel.index(index, 0);
                                    errorSettingModel.updateValue(errorSettingFilterModel.mapToSource(modelIndex), text, ResultSetting.Name);
                                }
                            }
                            Button {
                                text: qsTr("Delete Error")
                                enabled: disabled > 0 ? false : true
                                display: AbstractButton.TextBesideIcon
                                icon.name: "edit-delete"
                                onClicked: {
                                    var modelIndex = errorSettingFilterModel.index(index, 0);
                                    var dialog = deleteErrorDialogComponent.createObject(configErrorPage);
                                    dialog.modelIndex = errorSettingFilterModel.mapToSource(modelIndex);
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
                                title: qsTr("Plotter number %1").arg(Number(plotterNumber))
                                Layout.alignment: Qt.AlignTop
                                SpinBox {
                                    Layout.fillWidth: true
                                    from: 1
                                    to: 3
                                    stepSize: 1
                                    value: plotterNumber
                                    editable: true
                                    onValueModified: {
                                        var modelIndex = errorSettingFilterModel.index(index, 0);
                                        errorSettingModel.updateValue(errorSettingFilterModel.mapToSource(modelIndex), value, ResultSetting.PlotterNumber);
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
                                            var modelIndex = errorSettingFilterModel.index(index, 0);
                                            errorSettingModel.updateValue(errorSettingFilterModel.mapToSource(modelIndex), checked, ResultSetting.Plottable);
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
                                            var modelIndex = errorSettingFilterModel.index(index, 0);
                                            errorSettingModel.updateValue(errorSettingFilterModel.mapToSource(modelIndex), color, ResultSetting.LineColor);
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
                var dialog = addErrorDialogComponent.createObject(configErrorPage);
                dialog.open();
           }
        }
    }

}
