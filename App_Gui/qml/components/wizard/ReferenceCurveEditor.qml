import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0
import precitec.gui.components.plotter 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication

Control {
    id: root

    property alias referenceCurve: curveConstructor.referenceCurve
    property alias currentProduct: productInstanceModel.product
    property alias currentSeam: curveConstructor.seam
    property var resultsConfigModel: null
    property var zoomMode: PlotterSettings.BothAxis
    property alias hasChanges: curveConstructor.hasChanges
    property bool loading: productInstanceModel.loading || curveConstructor.loading
    property bool updating: curveConstructor.updating

    signal plotterSettingsUpdated();
    signal updateSettings();
    signal back();

    onUpdateSettings: {
        inputPlotter.updateSettings();
        selectPlotter.updateSettings();
        referencePlotter.updateSettings();
    }

    ProductInstanceModel {
        id: productInstanceModel

        monitoring: root.visible
        directory: WeldmasterPaths.resultsBaseDir
        extendedProductInfoHelper {
            serialNumberFromExtendedProductInfo: GuiConfiguration.serialNumberFromExtendedProductInfo != 0
            partNumberFromExtendedProductInfo: GuiConfiguration.partNumberFromExtendedProductInfo != 0
        }

        onLoadingChanged: {
            if (!productInstanceModel.loading)
            {
                productInstanceModel.ensureAllMetaDataLoaded();
            }
        }
    }

    ReferenceCurveConstructor {
        id: curveConstructor
        productInstanceModel: productInstanceModel

        resultType: root.referenceCurve ? root.referenceCurve.resultType : -1
        triggerType: root.resultsConfigModel && root.referenceCurve && root.currentProduct && resultsConfigModel.isLwmType(root.referenceCurve.resultType) ? root.currentProduct.lwmTriggerSignalType : -1
        threshold: root.currentProduct ? root.currentProduct.lwmTriggerSignalThreshold : 0.0
        currentProduct: root.currentProduct

        onProgressChanged: {
            progressBar.value = progress;
        }
    }

    InstanceResultSortModel {
        id: instanceDataSortModel
        sourceModel: curveConstructor
    }

    PlotterController {
        id: configControl

        configFilePath: GuiConfiguration.configFilePath
    }

    XAxisController {
        id: xAxisControl

        autoAdjustXAxis: true
        zoomMode: root.zoomMode
    }

    YAxisController {
        id: yAxisControl

        autoAdjustYAxis: true
        zoomMode: root.zoomMode
    }

    YLegendModel {
        id: yLegendModel

        min: yAxisControl.yMinVisual
        max: yAxisControl.yMaxVisual
        zoom: yAxisControl.zoom
        sections: configControl.rows
        panning: yAxisControl.panning + yAxisControl.zoomOffset
    }

    PlotterMenu {
        anchors.centerIn: parent

        id: configureDialog

        screenshotTool: screenshot
        parent: Overlay.overlay
        zoomEnabled: true

        onPlotterSettingsUpdated: root.plotterSettingsUpdated()

        onAccepted: {
            root.zoomMode = zoomMode;
            configControl.zoomFactor = zoomFactor;
            configControl.lineSize = lineWidth;
            configControl.xLegendPrecision = xLegendPrecision;
            configControl.yLegendPrecision = yLegendPrecision;
            configControl.pointsVisible = pointsVisible;
            configControl.linesVisible = linesVisible;
            configControl.blocksVisible = blocksVisible;
            configControl.horizontalCrosshairVisible = horizontalCrosshairVisible;
            configControl.verticalCrosshairVisible = verticalCrosshairVisible;
        }

        onAboutToShow: {
            zoomMode = root.zoomMode;
            xLegendPrecision = configControl.xLegendPrecision;
            yLegendPrecision = configControl.yLegendPrecision;
            configFilePath = configControl.configFilePath
            zoomFactor = configControl.zoomFactor;
            lineWidth = configControl.lineSize;
            pointsVisible = configControl.pointsVisible;
            linesVisible = configControl.linesVisible;
            blocksVisible = configControl.blocksVisible;
            horizontalCrosshairVisible = configControl.horizontalCrosshairVisible;
            verticalCrosshairVisible = configControl.verticalCrosshairVisible;
        }
    }

    contentItem: GridLayout {

        columns: 2

        ColumnLayout {
            Layout.maximumWidth: list.implicitWidth
            Layout.fillWidth: true
            Layout.fillHeight: true

            ComboBox {
                Layout.fillWidth: true

                model: [qsTr("Average Value"), qsTr("Median Value"), qsTr("Min and Max Value")]
                currentIndex: curveConstructor.referenceType
                onCurrentIndexChanged: curveConstructor.referenceType = currentIndex
                enabled: !root.loading
            }

            RowLayout {

                visible: curveConstructor.referenceType != ReferenceCurve.MinMax

                Label {
                    text: qsTr("Jitter:")
                }

                TextField {
                    Layout.fillWidth: true

                    id: jitterInput

                    selectByMouse: true
                    text: Number(curveConstructor.jitter).toLocaleString(locale, 'f', 2)
                    enabled: !root.loading

                    validator: DoubleValidator {
                        bottom: 0.0
                    }

                    onEditingFinished: {
                        curveConstructor.jitter = Number.fromLocaleString(locale, text);
                    }

                    palette.text: jitterInput.acceptableInput ? "black" : "red"
                }
            }

            InstanceListItem {
                Layout.fillHeight: true
                Layout.fillWidth: true

                id: list

                model: instanceDataSortModel
                productInstanceModel: productInstanceModel
                currentIndex: instanceDataSortModel.mapFromSource(curveConstructor.currentIndex).row
                loading: root.loading
                checkBoxVisible: true

                onItemClicked: {
                    curveConstructor.currentIndex = index;
                    curveConstructor.result.color = color;
                }
            }

            ComboBox {
                Layout.fillWidth: true

                enabled: !root.loading
                model: [qsTr("All results"), qsTr("Faults Only"), qsTr("Remove Faults")]
                onCurrentIndexChanged: {
                    switch (currentIndex)
                    {
                        case 1:
                            instanceDataSortModel.filterType = InstanceResultSortModel.OnlyNIO;
                            break;
                        case 2:
                            instanceDataSortModel.filterType = InstanceResultSortModel.RemoveNIO;
                            break;
                        default:
                            instanceDataSortModel.filterType = InstanceResultSortModel.All;
                            break;
                    }
                }
            }

            CheckBox {
                Layout.fillWidth: true
                text: qsTr("Include linked seams")
                enabled: !root.loading
                checked: instanceDataSortModel.includeLinkedSeams
                onToggled: {
                    instanceDataSortModel.includeLinkedSeams = !instanceDataSortModel.includeLinkedSeams;
                }
            }

            RowLayout {
                Layout.fillWidth: true

                enabled: !root.loading

                ComboBox {
                    Layout.fillWidth: true
                    model: [qsTr("Name"), qsTr("Date")]
                    onCurrentIndexChanged: {
                        instanceDataSortModel.sortOnDate = currentIndex !== 0;
                    }
                }

                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: instanceDataSortModel.sortOrder === Qt.DescendingOrder ? "view-sort-descending" : "view-sort-ascending"
                    onClicked: {
                        if (instanceDataSortModel.sortOrder === Qt.DescendingOrder)
                        {
                            instanceDataSortModel.sortOrder = Qt.AscendingOrder;
                        } else
                        {
                            instanceDataSortModel.sortOrder = Qt.DescendingOrder;
                        }

                        // reselect index after sorting
                        var row = curveConstructor.currentIndex.row;
                        curveConstructor.resetCurrentIndex();
                        curveConstructor.currentIndex = curveConstructor.index(row, 0);
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Select All")
                    enabled: !root.loading
                    onClicked: curveConstructor.selectAll()
                }

                Button {
                    Layout.fillWidth: true
                    text: qsTr("Select None")
                    enabled: !root.loading
                    onClicked: curveConstructor.selectNone()
                }
            }
        }

        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true

            Label {
                anchors.centerIn: parent

                visible: list.count === 0
                text: productInstanceModel.loading ? "" : qsTr("There aren't any \"%1\" results for this seam").arg(referenceCurve && resultsConfigModel ? qsTr("<b>%1</b>").arg(resultsConfigModel.nameForResultType(referenceCurve.resultType)) : "")
            }

            BusyIndicator {
                anchors.centerIn: parent
                running: productInstanceModel.loading
            }

            GridLayout {
                anchors.fill: parent

                columns: 2
                visible: list.count !== 0

                YLegend {
                    id: inputYLegend
                    Layout.fillHeight: true

                    precision: configControl.yLegendPrecision

                    model: yLegendModel
                    sectionHeight: (height - 2 * configControl.frameHeight) / configControl.rows
                    frameHeight: configControl.frameHeight
                }

                PlotterItem {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: inputPlotter

                    clip: true
                    controller: configControl
                    xAxisController: xAxisControl
                    yAxisController: yAxisControl

                    RowLayout {
                        anchors {
                            right: parent.right
                            top: parent.top
                            topMargin: spacing
                            rightMargin: spacing
                        }

                        ToolButton {
                            display: AbstractButton.IconOnly
                            icon.name: "restore"
                            onClicked: {
                                inputPlotter.resetPlotterView()
                                selectPlotter.resetPlotterView()
                                referencePlotter.resetPlotterView()
                            }
                        }

                        ToolButton {
                            display: AbstractButton.IconOnly
                            icon.name: "application-menu"
                            onClicked: configureDialog.open()
                        }
                    }

                    Component.onCompleted: {
                        inputPlotter.addDataSet(curveConstructor.result);
                        inputPlotter.addDataSet(curveConstructor.trigger);
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    Layout.preferredHeight: 2

                    color: PrecitecApplication.Settings.alternateBackground
                }

                YLegend {
                    Layout.fillHeight: true

                    precision: configControl.yLegendPrecision

                    model: yLegendModel
                    sectionHeight: (height - 2 * configControl.frameHeight) / configControl.rows
                    frameHeight: configControl.frameHeight
                }

                PlotterItem {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: selectPlotter

                    clip: true
                    model: curveConstructor
                    roleNames: "result"
                    controller: configControl
                    xAxisController: xAxisControl
                    yAxisController: yAxisControl

                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    Layout.preferredHeight: 2

                    color: PrecitecApplication.Settings.alternateBackground
                }

                YLegend {
                    Layout.fillHeight: true

                    precision: configControl.yLegendPrecision

                    model: yLegendModel
                    sectionHeight: (height - 2 * configControl.frameHeight) / configControl.rows
                    frameHeight: configControl.frameHeight
                }

                PlotterItem {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: referencePlotter

                    controller: configControl
                    xAxisController: xAxisControl
                    yAxisController: yAxisControl

                    ColumnLayout {
                        anchors.centerIn: parent

                        BusyIndicator {
                            Layout.alignment: Qt.AlignHCenter
                            running: curveConstructor.updating
                        }

                        ProgressBar {
                            id: progressBar
                            visible: curveConstructor.updating
                        }
                    }

                    Component.onCompleted: {
                        referencePlotter.addDataSet(curveConstructor.lower);
                        referencePlotter.addDataSet(curveConstructor.middle);
                        referencePlotter.addDataSet(curveConstructor.upper);
                    }
                }

                Item {
                    implicitWidth: inputYLegend.width
                }

                XLegend {
                    Layout.fillWidth: true

                    precision: configControl.xLegendPrecision

                    model: XLegendModel {
                        min: xAxisControl.xMinVisual
                        max: xAxisControl.xMaxVisual
                        zoom: xAxisControl.zoom
                        sections: configControl.xOffset == 0 ? configControl.columns : configControl.columns + 1
                        panning: xAxisControl.panning + xAxisControl.zoomOffset
                        offset: configControl.xOffset / width
                    }
                    sectionWidth: (width - configControl.xOffset - 2 * configControl.frameWidth) / configControl.columns
                    frameWidth: configControl.frameWidth
                }
            }
        }

        Button {
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignHCenter

            display: AbstractButton.TextBesideIcon
            text: qsTr("Update")
            icon.name: "document-save"
            enabled: curveConstructor.hasChanges && !root.loading && !root.updating

            onClicked: {
                curveConstructor.save();
                root.back();
            }
        }
    }
}
