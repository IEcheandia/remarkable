import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.plotter 1.0
import precitec.gui.components.application 1.0

Item {
    id: root
    property alias currentProduct: qualityMapModel.currentProduct

    signal markAsChanged()

    ColorMapModel {
        id: qualityMapModel
        onMarkAsChanged: root.markAsChanged()
    }

    ColorMapModel {
        id: errorMapModel
        binary: true
        currentProduct: root.currentProduct
        onMarkAsChanged: root.markAsChanged()
    }

    NewQualityLevel {
        id: newLevelDialog
    }

    InfiniteSet {
        id: top
        color: "aqua"
        value: 100
    }

    InfiniteSet {
        id: middle
        color: "magenta"
        value: 0
    }

    InfiniteSet {
        id: bottom
        color: "aqua"
        value: -100
    }

    RowLayout {
        anchors.fill: parent

        GridLayout {
            Layout.fillHeight: true
            columns: 2

            YLegend {
                Layout.fillHeight: true

                id: yLegendLeft
                implicitWidth: minimumWidth
                model: YLegendModel {
                    min: plot.yAxisController.yMinVisual
                    max: plot.yAxisController.yMaxVisual
                    zoom: 1
                    sections: 4
                    panning: 0
                }
                precision: 0
                sectionHeight: (plot.height - 2 * plot.controller.frameHeight) / plot.controller.rows
                frameHeight: plot.controller.frameHeight
                suffix: "%"
            }

            Plotter {
                Layout.fillHeight: true
                Layout.preferredWidth: 0.75 * root.width

                id: plot
                clip: true

                controller {
                    columns: 6
                    rows: 4
                    verticalCrosshairVisible: false
                }

                xAxisController {
                    autoAdjustXAxis: true
                }
                yAxisController {
                    autoAdjustYAxis: true
                }

                Component.onCompleted: {
                    plot.addDataSet(top);
                    plot.addDataSet(middle);
                    plot.addDataSet(bottom);
                    plot.addDataSet(qualityMapModel.example);
                }
            }

            Item {
                Layout.preferredWidth: yLegendLeft.width
                Layout.rowSpan: 2
            }

            Plotter {
                Layout.preferredWidth: 0.75 * root.width
                Layout.preferredHeight: 50
                Layout.topMargin: 30

                id: trackPlot

                controller {
                    rows: 1
                    xOffset: plot.controller.xOffset
                    columns: plot.controller.columns
                    verticalCrosshairVisible: false
                    horizontalCrosshairVisible: false
                }

                xAxisController {
                    autoAdjustXAxis: true
                }
                yAxisController {
                    autoAdjustYAxis: true
                }

                clip: true

                Component.onCompleted: {
                    trackPlot.addDataSet(errorMapModel.example);
                }
            }

            XLegend {
                id: xLegend
                Layout.preferredWidth: 0.75 * root.width
                model: XLegendModel {
                    min: trackPlot.xAxisController.xMinVisual
                    max: trackPlot.xAxisController.xMaxVisual
                    zoom: 1
                    sections: 6
                    panning: 0
                    offset: trackPlot.controller.xOffset / width
                }
                precision: 0
                sectionWidth: (width - trackPlot.controller.xOffset - 2 * trackPlot.controller.frameWidth) / trackPlot.controller.columns
                frameWidth: trackPlot.controller.frameWidth
                suffix: "%"
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 15

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 10

                Label {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    text: qsTr("Quality Levels describe the deviation between the results and a given reference:")
                    wrapMode: Text.WordWrap
                }
                Label {
                    text: qsTr(" - 0% is the ideal middle line")
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: 3
                    color: "magenta"
                }
                Label {
                    text: qsTr(" - 100% are the reference limits")
                }
                Rectangle {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: 3
                    color: "aqua"
                }
                Label {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    text: qsTr("If a value exceeds the limits, an error is triggered.")
                    wrapMode: Text.WordWrap
                }
            }

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true

                id: qualityList
                spacing: 5

                header: RowLayout {
                    width: qualityList.width
                    Label {
                        Layout.fillWidth: true
                        Layout.bottomMargin: 10
                        text: qsTr("Quality Levels")
                    }
                    Label {
                        Layout.bottomMargin: 10
                        Layout.rightMargin: 40
                        text: qsTr("Color")
                    }
                }

                footer: RowLayout {
                    width: qualityList.width
                    Button {
                        Layout.fillWidth: true
                        Layout.topMargin: 10
                        text: qsTr("Add Quality Level")
                        enabled: qualityMapModel.enabled && qualityMapModel.rowCount < 5
                        onClicked: {
                            newLevelDialog.model = qualityMapModel;
                            newLevelDialog.title = qsTr("New Quality Level");
                            newLevelDialog.open();
                        }
                    }
                    Button {
                        Layout.topMargin: 10
                        text: qsTr("Reset")
                        enabled: qualityMapModel.enabled
                        onClicked: {
                            qualityMapModel.reset();
                        }
                    }
                }

                model: qualityMapModel

                delegate:  RowLayout{

                    width: qualityList.width
                    SpinBox {
                        value: model.value
                        from: 0
                        to: 200
                        editable: true
                        enabled: qualityMapModel.enabled && model.value != 100
                        onValueModified: {
                            qualityMapModel.updateValue(index, value / 100);
                        }
                        Component.onCompleted: {
                            contentItem.selectByMouse = true;
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        text: "%"
                    }
                    ColorDialog {
                        id: colorDialog
                        showAlphaChannel: false
                        anchors.centerIn: Overlay.overlay
                        width: Overlay.overlay ? Overlay.overlay.width * 0.9 : 0
                        height: Overlay.overlay ? Overlay.overlay.height * 0.9 : 0
                        modal: true
                        title: qsTr("Select color")
                        onAccepted: {
                            qualityColorSelect.icon.color = color;
                            qualityMapModel.updateColor(index, color);
                        }
                    }
                    ToolButton {
                        id: qualityColorSelect
                        icon.name: "color-picker"
                        icon.color: model.color
                        enabled: qualityMapModel.enabled
                        display: AbstractButton.IconOnly
                        onClicked: {
                            colorDialog.color = model.color;
                            colorDialog.currentColor = model.color;
                            colorDialog.currentHue = model.hue;
                            colorDialog.currentSaturation = model.saturation;
                            colorDialog.currentLightness = model.lightness;
                            colorDialog.crosshairsVisible = true;
                            colorDialog.open();
                        }
                    }
                    ToolButton {
                        id: removeColor
                        icon.name: "edit-delete"
                        enabled: qualityMapModel.enabled && model.value != 100
                        display: AbstractButton.IconOnly
                        onClicked: {
                            qualityMapModel.removeColor(index);
                        }
                    }
                }
            }

            Label {
                Layout.fillWidth: true
                text: qsTr("Error Levels describe how close the result is to being faulty.")
                wrapMode: Text.WordWrap
            }

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true

                id: errorList
                spacing: 5

                header: RowLayout {
                    width: errorList.width
                    Label {
                        Layout.fillWidth: true
                        Layout.bottomMargin: 10
                        text: qsTr("Error Levels")
                    }
                    Label {
                        Layout.bottomMargin: 10
                        Layout.rightMargin: 40
                        text: qsTr("Color")
                    }
                }

                footer: RowLayout {
                    width: errorList.width
                    Button {
                        Layout.fillWidth: true
                        Layout.topMargin: 10
                        text: qsTr("Add Error Level")
                        enabled: errorMapModel.enabled && errorMapModel.rowCount < 5
                        onClicked: {
                            newLevelDialog.model = errorMapModel;
                            newLevelDialog.title = qsTr("New Error Level");
                            newLevelDialog.open();
                        }
                    }
                    Button {
                        Layout.topMargin: 10
                        text: qsTr("Reset")
                        enabled: errorMapModel.enabled
                        onClicked: {
                            errorMapModel.reset();
                        }
                    }
                }

                model: errorMapModel

                delegate:  RowLayout{
                    width: errorList.width
                    SpinBox {
                        value: model.value
                        from: 0
                        to: 200
                        editable: true
                        enabled: errorMapModel.enabled && model.value != 100
                        onValueModified: {
                            errorMapModel.updateValue(index, value / 100);
                        }
                        Component.onCompleted: {
                            contentItem.selectByMouse = true;
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        text: "%"
                    }
                    ColorDialog {
                        id: colorDialog
                        showAlphaChannel: false
                        anchors.centerIn: Overlay.overlay
                        width: Overlay.overlay ? Overlay.overlay.width * 0.9 : 0
                        height: Overlay.overlay ? Overlay.overlay.height * 0.9 : 0
                        modal: true
                        title: qsTr("Select color")
                        onAccepted: {
                            errorColorSelect.icon.color = color;
                            errorMapModel.updateColor(index, color);
                        }
                    }
                    ToolButton {
                        id: errorColorSelect
                        icon.name: "color-picker"
                        icon.color: model.color
                        enabled: errorMapModel.enabled
                        display: AbstractButton.IconOnly
                        onClicked: {
                            colorDialog.color = model.color;
                            colorDialog.currentColor = model.color;
                            colorDialog.currentHue = model.hue;
                            colorDialog.currentSaturation = model.saturation;
                            colorDialog.currentLightness = model.lightness;
                            colorDialog.crosshairsVisible = true;
                            colorDialog.open();
                        }
                    }
                    ToolButton {
                        id: removeColor
                        icon.name: "edit-delete"
                        enabled: errorMapModel.enabled && model.value != 100
                        display: AbstractButton.IconOnly
                        onClicked: {
                            errorMapModel.removeColor(index);
                        }
                    }
                }
            }
        }
    }
}
