import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.plotter 1.0
import precitec.gui.general 1.0
import Precitec.AppGui 1.0

Item {
    property alias sensorConfigModel: lwmController.sensorConfigModel
    property alias attributeModel: lwmController.attributeModel
    property var zoomMode: PlotterSettings.OnlyXAxis

    id: root

    function pageBecameVisible()
    {
        liveModeEnableTimer.restart();
        lwmController.weldheadDeviceProxy = HardwareModule.weldHeadDeviceProxy;
    }

    signal clear()
    signal resetPlotterView()
    signal updateSettings()
    signal plotterSettingsUpdated()

    onVisibleChanged: {
        if (visible)
        {
            root.pageBecameVisible();
        } else
        {
            liveModeEnableTimer.stop();
            lwmController.liveMode = false;
            lwmController.weldheadDeviceProxy = null;
        }
    }

    Component.onCompleted: {
        if (visible)
        {
            root.pageBecameVisible();
        }
    }

    Timer {
        id: liveModeEnableTimer
        interval: 500
        repeat: false
        onTriggered: {
            lwmController.liveMode = true;
        }
    }

    Connections {
        target: HardwareModule.recorder
        enabled: visible
        function onSampleData(sensorId, data) {
            lwmController.addSample(sensorId, data)
        }
        function onImageDataChanged() {
            HardwareModule.recorder.requestNextImage()
        }
    }

    LwmController {
        id: lwmController

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel

        onSamplesRendered: HardwareModule.recorder.requestNextImage()
    }

    PlotterMenu {
        id: configureDialog
        screenshotTool: screenshot
        anchors.centerIn: parent
        parent: Overlay.overlay
        zoomEnabled: true
        zoomBothAxisVisible: false
        onPlotterSettingsUpdated: root.plotterSettingsUpdated()
        onAccepted: {
            root.zoomMode = configureDialog.zoomMode;
            plotterControl.zoomFactor = configureDialog.zoomFactor;
            plotterControl.lineSize = configureDialog.lineWidth;
            plotterControl.xLegendPrecision = configureDialog.xLegendPrecision;
            plotterControl.yLegendPrecision = configureDialog.yLegendPrecision;
            plotterControl.pointsVisible = configureDialog.pointsVisible;
            plotterControl.linesVisible = configureDialog.linesVisible;
            plotterControl.blocksVisible = configureDialog.blocksVisible;
            plotterControl.horizontalCrosshairVisible = configureDialog.horizontalCrosshairVisible;
            plotterControl.verticalCrosshairVisible = configureDialog.verticalCrosshairVisible;
        }
        onAboutToShow: {
            configureDialog.zoomMode = root.zoomMode
            configureDialog.xLegendPrecision = plotterControl.xLegendPrecision
            configureDialog.yLegendPrecision = plotterControl.yLegendPrecision
            configureDialog.configFilePath = plotterControl.configFilePath
            configureDialog.zoomFactor = plotterControl.zoomFactor
            configureDialog.lineWidth = plotterControl.lineSize
            configureDialog.pointsVisible = plotterControl.pointsVisible
            configureDialog.linesVisible = plotterControl.linesVisible
            configureDialog.blocksVisible = plotterControl.blocksVisible
            configureDialog.horizontalCrosshairVisible = plotterControl.horizontalCrosshairVisible
            configureDialog.verticalCrosshairVisible = plotterControl.verticalCrosshairVisible
        }
    }

    XAxisController {
        id: xControl
        autoAdjustXAxis: true
        zoomMode: root.zoomMode
    }

    PlotterController {
        id: plotterControl
        colorGradientEnabled: GuiConfiguration.colorSignalsByQuality
    }

    RowLayout {
        anchors {
            fill: parent
            margins: 5
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            property var model: [lwmController.backReflection, lwmController.laserPower, lwmController.plasma, lwmController.temperature]

            id: plotterComponents

            RowLayout {
                anchors {
                    right: parent.right
                    top: parent.top
                    margins: 5
                }

                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "application-menu"
                    onClicked: configureDialog.open()
                }
                ToolButton {
                    id: resetPlotterButton
                    display: AbstractButton.IconOnly
                    icon.name: "restore"
                    onClicked: root.resetPlotterView()
                }
                ToolButton {
                    Layout.alignment: Qt.AlignRight
                    icon.name: "edit-clear"
                    onClicked: {
                        root.clear();
                    }
                }
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: lwmController.updating
            }

            GridLayout {
                anchors.fill: parent

                columns: 2
                enabled: lwmController.ready && !lwmController.updating

                Repeater {
                    model: plotterComponents.model

                    delegate: Label {
                        Layout.row: 2 * index
                        Layout.column: 0
                        Layout.fillWidth: true
                        Layout.columnSpan: 2

                        font.bold: true
                        text: modelData.name
                    }
                }

                Repeater {
                    model: plotterComponents.model

                    delegate: YLegend {
                        Layout.row: 2 * index + 1
                        Layout.column: 0
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignRight

                        precision: plotterControl.yLegendPrecision
                        implicitWidth: minimumWidth
                        sectionHeight: (height - 2 * plotterControl.frameHeight) / plotterControl.rows
                        frameHeight: plotterControl.frameHeight

                        model: YLegendModel {
                            sections: plotterControl.rows
                        }

                        Component.onCompleted: {
                            var plotter = repeater.itemAt(index);
                            model.min = Qt.binding(function() { return plotter.yAxisController.yMinVisual; });
                            model.max = Qt.binding(function() { return plotter.yAxisController.yMaxVisual; });
                            model.zoom = Qt.binding(function() { return plotter.yAxisController.zoomY; });
                            model.panning = Qt.binding(function() { return plotter.yAxisController.panning + plotter.yAxisController.zoomOffset; });
                        }
                    }
                }

                Repeater {
                    id: repeater

                    model: plotterComponents.model

                    delegate: PlotterItem {
                        Layout.row: 2 * index + 1
                        Layout.column: 1
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        id: plotter

                        xAxisController: xControl
                        controller: plotterControl
                        menuVisible: configureDialog.visible

                        yAxisController {
                            autoAdjustYAxis: true
                            constantYRange: GuiConfiguration.scalePlotterFromSettings
                            zoomMode: root.zoomMode
                        }

                        Connections {
                            target: root
                            function onClear() {
                                plotter.clear()
                            }
                            function onResetPlotterView() {
                                plotter.resetPlotterView()
                            }
                            function onUpdateSettings() {
                                plotter.controller.updateSettings()
                            }
                        }

                        Component.onCompleted: {
                            plotter.addDataSet(modelData)
                        }
                    }
                }

                Item {
                    height: xLegend.height
                }

                XLegend {
                    id: xLegend
                    Layout.fillWidth: true
                    precision: plotterControl.xLegendPrecision
                    model: XLegendModel {
                        min: xControl.xMinVisual
                        max: xControl.xMaxVisual
                        zoom: xControl.zoomX
                        sections: plotterControl.xOffset == 0 ? plotterControl.columns : plotterControl.columns + 1
                        panning: xControl.panning + xControl.zoomOffset
                        offset: plotterControl.xOffset / width
                    }
                    sectionWidth: (width - plotterControl.xOffset - 2 * plotterControl.frameWidth) / plotterControl.columns
                    frameWidth: plotterControl.frameWidth
                }
            }
        }

        ColumnLayout {
            Layout.maximumWidth: 0.2 * root.width
            Layout.fillWidth: true
            Layout.fillHeight: true

            enabled: lwmController.ready && !lwmController.updating

            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Back Reflection Amplification Factor")
                ComboBox {
                    anchors.fill: parent
                    model: lwmController.backReflectionAmplificationModel
                    currentIndex: lwmController.backReflectionAmplification
                    onActivated: {
                        lwmController.backReflectionAmplification = currentIndex;
                    }
                }
            }

            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Laser Power Amplification Factor")
                ComboBox {
                    anchors.fill: parent
                    model: lwmController.laserPowerAmplificationModel
                    currentIndex: lwmController.laserPowerAmplification
                    onActivated: {
                        lwmController.laserPowerAmplification = currentIndex;
                    }
                }
            }

            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Plasma Amplification Factor")
                ComboBox {
                    anchors.fill: parent
                    model: lwmController.plasmaAmplificationModel
                    currentIndex: lwmController.plasmaAmplification
                    onActivated: {
                        lwmController.plasmaAmplification = currentIndex;
                    }
                }
            }

            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Temperature Amplification Factor")
                ComboBox {
                    anchors.fill: parent
                    model: lwmController.temperatureAmplificationModel
                    currentIndex: lwmController.temperatureAmplification
                    onActivated: {
                        lwmController.temperatureAmplification = currentIndex;
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
