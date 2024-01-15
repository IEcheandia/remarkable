import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import precitec.gui.components.plotter 1.0
import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

Item {
    id: root
    property var resultsModel: null
    property alias errorsModel: errorList.model
    property var roleNames: ["dataSet", "separators", "boundaries"]
    property alias configFilePath: plotterControl.configFilePath
    property bool moveButtonsVisible: false
    property alias nextButtonEnabled: nextButton.enabled
    property alias previousButtonEnabled: previousButton.enabled
    property bool labelVisible: true
    property bool inputEnabled: true
    property alias horizontalMarkerVisible: plotterControl.horizontalMarkerVisible
    property alias verticalMarkerVisible: plotterControl.verticalMarkerVisible
    property alias horizontalMarkerPosition: plotterControl.horizontalMarkerPosition
    property alias verticalMarkerPosition: plotterControl.verticalMarkerPosition
    property alias multipleSeamControlsVisible: numberOfSeamBox.visible
    property bool toolTipEnabled: true

    /**
     * Whether an external configure button should be visible in the button row.
     * If clicked the signal @link{configure} is emitted.
     **/
    property alias externalConfigureButtonVisible: externalConfigureButton.visible
    /**
     * Whether the external configure button should be enabled.
     * If clicked the signal @link{configure} is emitted.
     **/
    property alias externalConfigureButtonEnabled: externalConfigureButton.enabled

    property var zoomMode: PlotterSettings.BothAxis

    property bool xLegendVisible: true

    property alias additionalTabsModel: additionalTabsRepeater.model
    property alias additionalTabComponentsModel: additionalTabComponentsRepeater.model

    signal clear()
    signal resetPlotterView()
    signal updateSettings()
    signal plotterSettingsUpdated()
    signal goPrevious()
    signal goNext()
    /**
     * Emitted when the external configure button is clicked.
     * @see externalConfigureButtonVisible
     * @see externalConfigureButtonEnabled
     **/
    signal configure()

    function showErrorTab()
    {
        if (errorsTabButton.visible)
        {
            bar.currentIndex = bar.count - 1
        }
    }

    function selectFirstErrorPosition()
    {
        if (errorList.count > 0)
        {
            return errorList.itemAtIndex(0).selectErrorPosition()
        }
        return -1
    }

    PlotterFilterModel {
        id: plotter1FilterModel
        plotterNumber: 1
        sourceModel: resultsModel
    }
    PlotterFilterModel {
        id: plotter2FilterModel
        plotterNumber: 2
        sourceModel: resultsModel
    }
    PlotterFilterModel {
        id: plotter3FilterModel
        plotterNumber: 3
        sourceModel: resultsModel
    }
    PlotterFilterModel {
        id: plotter1ListFilterModel
        plotterNumber: 1
        nioFilter: true
        sourceModel: resultsModel
    }
    PlotterFilterModel {
        id: plotter2ListFilterModel
        plotterNumber: 2
        nioFilter: true
        sourceModel: resultsModel
    }
    PlotterFilterModel {
        id: plotter3ListFilterModel
        plotterNumber: 3
        nioFilter: true
        sourceModel: resultsModel
    }

    PlotterMenu {
        id: configureDialog
        screenshotTool: screenshot
        anchors.centerIn: parent
        parent: Overlay.overlay
        zoomEnabled: true
        zoomBothAxisVisible: true
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
        anchors.fill: parent
        spacing: 0

        ColumnLayout {
            Layout.preferredWidth: Math.max(0.2 * root.width, bar.contentWidth)
            Layout.minimumWidth: bar.contentWidth
            Layout.maximumWidth: Math.max(0.2 * root.width, bar.contentWidth)
            Layout.fillHeight: true
            Layout.margins: 0

            TabBar {
                id: bar
                Layout.fillWidth: true

                Repeater {
                    id: additionalTabsRepeater
                    TabButton {
                        text: modelData
                    }
                }

                TabButton {
                    text: qsTr("Results")
                    implicitWidth: implicitContentWidth + 20
                }

                TabButton {
                    id: errorsTabButton
                    text: qsTr("Errors")
                    implicitWidth: implicitContentWidth + 20
                    visible: errorsModel ? true : false
                }

                Component.onCompleted: {
                    bar.currentIndex = 0;
                }
            }

            StackLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true
                currentIndex: bar.currentIndex

                Repeater {
                    id: additionalTabComponentsRepeater

                    Loader {
                        sourceComponent: modelData
                    }
                }

                GridLayout {
                    id: resultsGrid
                    columns: 1
                    Layout.margins: 0

                    Repeater {
                        id: resultsRepeater

                        model: [plotter1ListFilterModel, plotter2ListFilterModel, plotter3ListFilterModel]

                        ListView {
                            Layout.row: 2 * index + 1
                            Layout.column: 0
                            Layout.margins: 0
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            id: listView

                            model: modelData

                            spacing: 5

                            ScrollBar.vertical: ScrollBar {
                                anchors.right: parent.right
                                id: verticalScroll
                                width: 7
                            }

                            clip: true

                            delegate: CheckBox {
                                id: delegate
                                property var modelIndex: ListView.view.model.index(index, 0)
                                property var sourceModelIndex: ListView.view.model.mapToSource(delegate.modelIndex)
                                width: listView.width
                                text: model.type
                                checked: model.enabled
                                onClicked: {
                                    model.enabled = delegate.checked;
                                }

                                contentItem: ColumnLayout {
                                    anchors {
                                        leftMargin: delegate.indicator.width + delegate.spacing
                                        rightMargin: verticalScroll.width + 3
                                    }

                                    spacing: 0

                                    Label {
                                        id: resultLabel
                                        Layout.leftMargin: delegate.indicator.width + delegate.spacing
                                        Layout.rightMargin: verticalScroll.width + 3
                                        Layout.fillWidth: true
                                        text: delegate.text
                                        font.bold: true
                                        font.pixelSize: 14
                                        color: model.color
                                        wrapMode: Text.Wrap
                                        elide: Text.ElideRight
                                        maximumLineCount: 2

                                        onImplicitHeightChanged: {
                                            delegate.height = Math.max(resultLabel.implicitHeight + positionLabel.implicitHeight, delegate.implicitIndicatorHeight) + delegate.topPadding + delegate.bottomPadding;
                                        }
                                    }
                                    Label {
                                        id: positionLabel
                                        property var selectedNumber: root.verticalMarkerVisible ? root.resultsModel.valueAtPosition(delegate.sourceModelIndex, root.verticalMarkerPosition) : undefined
                                        property var number: root.verticalMarkerVisible ? positionLabel.selectedNumber : model.value
                                        Layout.leftMargin: delegate.indicator.width + delegate.spacing
                                        Layout.rightMargin: verticalScroll.width + 3
                                        Layout.fillWidth: true
                                        font.pixelSize: 14
                                        text: Number(positionLabel.number).toLocaleString(locale, "f", 3)
                                        visible: !isNaN(positionLabel.number)
                                    }
                                }
                            }
                        }
                    }

                    Repeater {
                        model: resultsRepeater.model ? resultsRepeater.model.length - 1 : 0

                        Rectangle {
                            Layout.row: 2 * index + 2
                            Layout.column: 0
                            Layout.fillWidth: true
                            Layout.preferredHeight: 2
                            Layout.margins: 0
                            color: PrecitecApplication.Settings.alternateBackground
                        }
                    }

                    Item {
                        height: xLegend.height + xLegendUnitBelow.height + resultsGrid.rowSpacing
                    }
                 }

                ListView {
                    id: errorList
                    spacing: 5
                    clip: true

                    ScrollBar.vertical: ScrollBar {
                        anchors.right: parent.right
                        id: errorScroll
                        width: 7
                    }
                    delegate: ItemDelegate {
                        id: errorsDelegate
                        function selectErrorPosition() {
                            verticalMarkerPosition = model.position
                            return model.position
                        }
                        implicitHeight: errorsDelegateGrid.implicitHeight
                        width: errorList.width
                        onClicked: selectErrorPosition()
                        GridLayout {
                            id: errorsDelegateGrid
                            columns: 2
                            width: errorList.width
                            Rectangle {
                                Layout.rowSpan: 3
                                Layout.alignment: Qt.AlignTop
                                color: "red"
                                width: 25
                                height: 25
                                radius: width * 0.5
                            }
                            Label {
                                Layout.fillWidth: true
                                text: model.name
                                color: model.color
                                font.bold: true
                                font.pixelSize: 14
                                wrapMode: Text.Wrap
                                elide: Text.ElideRight
                                maximumLineCount: 2
                            }
                            Label {
                                text: model.seam
                                font.pixelSize: 14
                            }
                            Label {
                                text: "Position:  %1".arg(model.position.toFixed(2))
                                font.pixelSize: 14
                            }
                        }
                    }
                }
            }
        }

        GridLayout {
            columns: 2
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                Layout.preferredHeight: bar.height
            }

            RowLayout {
                layoutDirection: Qt.RightToLeft
                Layout.preferredHeight: bar.height

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
                        if (root.resultsModel)
                        {
                            root.resultsModel.clear();
                        }
                        if (root.errorsModel)
                        {
                            root.errorsModel.clear();
                        }
                        root.clear();
                    }
                }
                ToolButton {
                    Layout.alignment: Qt.AlignRight
                    icon.name: "show_boundary"
                    checkable: true
                    checked: GuiConfiguration.displayErrorBoundariesInPlotter
                    onClicked: {
                        GuiConfiguration.displayErrorBoundariesInPlotter = checked;
                        GuiConfiguration.sync();
                    }
                }
                ToolButton {
                    id: externalConfigureButton
                    display: AbstractButton.IconOnly
                    visible: false
                    icon.name: "document-edit"
                    onClicked: root.configure()
                }
                ToolButton {
                    id: nextButton
                    display: AbstractButton.IconOnly
                    icon.name: "go-next"
                    visible: moveButtonsVisible
                    onClicked: root.goNext()
                }
                ToolButton {
                    id: previousButton
                    display: AbstractButton.IconOnly
                    icon.name: "go-previous"
                    visible: moveButtonsVisible
                    onClicked: root.goPrevious()
                }
                SpinBox {
                    id: numberOfSeamBox
                    visible: false
                    from: 1
                    value: GuiConfiguration.numberOfSeamsInPlotter
                    onValueModified: {
                        GuiConfiguration.numberOfSeamsInPlotter = value;
                        GuiConfiguration.sync();
                    }
                }
                Item {
                    Layout.preferredHeight: bar.height
                    Layout.fillWidth: true
                }
                RowLayout {
                    Layout.fillWidth: true
                    Label {
                        Layout.fillWidth: true
                        Layout.maximumWidth: implicitWidth + 1
                        elide: Text.ElideRight
                        text: (labelVisible && root.resultsModel) ? root.resultsModel.seamLabel + (seriesText.text != "" ? "," : ""): ""
                    }
                    Label {
                        id: seriesText
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                        visible: labelVisible && GuiConfiguration.seamSeriesOnProductStructure && root.resultsModel
                        text: (labelVisible && GuiConfiguration.seamSeriesOnProductStructure && root.resultsModel) ? root.resultsModel.seriesLabel : ""
                    }
                }
            }

            Repeater {
                model: [plotter1FilterModel, plotter2FilterModel, plotter3FilterModel]

                ColumnLayout {
                    Layout.row: 2 * index + 1
                    Layout.column: 0
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignRight

                    Item {
                        Component.onCompleted: {
                            Layout.prefferedHeight = Qt.binding(function() { return repeater.itemAt(index).height; });
                        }
                    }

                    YLegend {
                        id: yLegend
                        Layout.fillHeight: true
                        implicitWidth: minimumWidth
                        precision: plotterControl.yLegendPrecision

                        Component.onCompleted: {
                            model.min = Qt.binding(function() { return repeater.itemAt(index).yAxisController.yMinVisual; });
                            model.max = Qt.binding(function() { return repeater.itemAt(index).yAxisController.yMaxVisual; });
                            model.zoom = Qt.binding(function() { return repeater.itemAt(index).yAxisController.zoomY; });
                            model.panning = Qt.binding(function() { return repeater.itemAt(index).yAxisController.panning + repeater.itemAt(index).yAxisController.zoomOffset; });
                        }

                        model: YLegendModel {
                            sections: plotterControl.rows
                        }
                        sectionHeight: (height - 2 * plotterControl.frameHeight) / plotterControl.rows
                        frameHeight: plotterControl.frameHeight
                    }
                }
            }

            Repeater {
                id: repeater

                model: [plotter1FilterModel, plotter2FilterModel, plotter3FilterModel]

                BinaryTrackPlotter {
                    id: plotter
                    Layout.row: 2 * index + 1
                    Layout.column: 1
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    xLegendVisible: false
                    yLegendVisible: false
                    xAxisController: xControl
                    controller: plotterControl
                    model: modelData
                    roleNames: root.roleNames
                    binaryTrackRoleName: "binaryPlot"
                    yAxisController {
                        autoAdjustYAxis: true
                        constantYRange: GuiConfiguration.scalePlotterFromSettings
                        zoomMode: root.zoomMode
                    }
                    menuVisible: configureDialog.visible
                    inputEnabled: root.inputEnabled
                    toolTipEnabled: root.toolTipEnabled

                    Connections {
                        target: root
                        function onClear() {
                            plotter.clear()
                        }
                        function onResetPlotterView() {
                            plotter.resetPlotterView()
                        }
                        function onUpdateSettings() {
                            plotter.updateSettings()
                        }
                    }
                }
            }

            Repeater {
                id: separatorRepeater
                model: resultsRepeater.model ? resultsRepeater.model.length - 1 : 0

                Rectangle {
                    Layout.row: 2 * index + 2
                    Layout.column: 0
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    Layout.preferredHeight: 2
                    color: PrecitecApplication.Settings.alternateBackground
                }
            }

            Item {
                height: xLegend.height
            }

            XLegend {
                id: xLegend
                Layout.fillWidth: true
                visible: root.xLegendVisible
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

            Item {
                height: xLegendUnitBelow.height
                visible: xLegendUnitBelow.visible
            }

            Label {
                id: xLegendUnitBelow
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                visible: true
                text: qsTr("mm")
                height: implicitHeight
            }
        }
    }
}
