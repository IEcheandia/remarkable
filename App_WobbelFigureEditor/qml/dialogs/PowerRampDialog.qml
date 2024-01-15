import QtQuick 2.7
import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import Qt.labs.qmlmodels 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0
import wobbleFigureEditor.components 1.0
import precitec.gui.components.plotter 1.0
import precitec.gui.components.plotter2d 1.0

Control {
    id: powerRampDialog

    property var thisScreenshotTool: null
    property LaserPowerController laserPowerController
    property LaserPointController laserPointController
    property PowerRampModel powerRampModel
    property var actualFigure: null
    property var figureEditor: null
    property var graphView: null
    property int colWidth: 120
    property int rowHeight: 37 // NOTE: The toolbutton width is 35
    property int maxPointId: figureEditor.numberOfPoints - 1

    property real laserBottom: /*figureEditor && figureEditor.fileType == FileType.Seam ? PowerLimits.Default :*/ PowerLimits.MinValue
    property real laserTop: figureEditor && figureEditor.figure.analogPower ? PowerLimits.AnalogMaxValue : PowerLimits.DigitalMaxValue

    property real ringBottom: /*figureEditor && figureEditor.fileType == FileType.Seam ? PowerLimits.Default :*/ PowerLimits.MinValue
    property real ringTop: PowerLimits.AnalogMaxValue

    width: 547

    z: 2

    onVisibleChanged: {
        if (visible)
        {
            powerRampModel.setPowerLimits(laserBottom, laserTop, ringBottom, ringTop)
            powerRampModel.dualChannel = figureEditor.figure.analogPower && FigureEditorSettings.dualChannelLaser && figureEditor.fileType != FileType.Overlay
            powerRampModel.modifyLaserPower = true
            powerRampModel.modifyDefault = true
            powerRampModel.asOffset = false
            powerRampModel.modifyRingPower = powerRampModel.dualChannel
        }
    }

    onMaxPointIdChanged: {
        powerRampModel.updateLaserPointCount(figureEditor.numberOfPoints)
    }

    function updateGraph() {
        let plt = plotterChart.plotter
        plt.clear()

        if (powerRampModel.modifyLaserPower)
        {
            plt.addDataSet(powerRampModel.laserPowers)
        }

        if (powerRampModel.modifyRingPower)
        {
            plt.addDataSet(powerRampModel.ringPowers)
        }
    }

    Connections {
        target: powerRampModel

        function onRampChanged() {
            updateGraph()
        }

        function onModifyLaserPowerChanged() {
            tableView.forceLayout()
            updateGraph()
        }

        function onModifyRingPowerChanged() {
            tableView.forceLayout()
            updateGraph()
        }
    }

    WobbleFigureDataModel {
        id: wobbleFigureDataModel
        figure: actualFigure
        graphView: powerRampDialog.graphView
        figureEditor: powerRampDialog.figureEditor
    }

    contentItem: Page {
        id: contentPage

        padding: 10

        header: PrecitecApplication.DialogHeaderWithScreenshot {
            id: preHeader
            title: qsTr("Set Power")
            screenshotTool: thisScreenshotTool
        }
        footer: DialogButtonBox {
            id: preFooter
            alignment: Qt.AlignRight
            Button {
                text: qsTr("Close")
                DialogButtonBox.buttonRole: DialogButtonBox.DestructiveRole
                onPressed: {
                    powerRampDialog.visible = false
                }
            }
            Button {
                text: qsTr("Apply")
                DialogButtonBox.buttonRole: DialogButtonBox.ApplyRole
                enabled: powerRampModel.valid
                onPressed: {
                    laserPowerController.applyPowerRampChanges(
                            powerRampModel.modifyDefault || !cbChangeDefault.visible,
                            powerRampModel.asOffset && cbOffset.visible)
                }
            }
        }

        background: Rectangle {
            anchors.fill: parent
            color: "white"
            border.width: 1
            border.color: "grey"
        }

        contentItem: ColumnLayout {
            GridLayout {
                columns: 4
                visible: powerRampModel.pointCount == 1 || powerRampModel.dualChannel

                CheckBox {
                    id: cbModifyLaserPower
                    visible: powerRampModel.dualChannel // without dual channel we always modify the laser power (otherwise the dialog would have no purpose)
                    onClicked: {
                        powerRampModel.modifyLaserPower = checked

                        if (!powerRampModel.modifyRingPower && !powerRampModel.modifyLaserPower)
                        {
                            powerRampModel.modifyRingPower = true
                        }
                    }
                    checked: powerRampModel.modifyLaserPower
                }

                Label {
                    visible: powerRampModel.dualChannel
                    text: qsTr("Modify Core Power")
                }

                CheckBox {
                    id: cbModifyRingPower
                    visible: powerRampModel.dualChannel
                    onClicked: {
                        powerRampModel.modifyRingPower = checked

                        if (!powerRampModel.modifyRingPower && !powerRampModel.modifyLaserPower)
                        {
                            powerRampModel.modifyLaserPower = true
                        }
                    }
                    checked: powerRampModel.modifyRingPower
                }

                Label {
                    visible: powerRampModel.dualChannel
                    text: qsTr("Modify Ring Power")
                }

                CheckBox {
                    id: cbChangeDefault
                    visible: powerRampModel.pointCount == 1
                    onClicked: {
                        powerRampModel.modifyDefault = checked
                    }
                    checked: powerRampModel.modifyDefault

                    ToolTip.text: qsTr("Modify powers that are set to -1.")
                    ToolTip.visible: hovered
                    ToolTip.delay: 350
                }

                Label {
                    text: qsTr("Modify default")
                    visible: powerRampModel.pointCount == 1
                }

                CheckBox {
                    id: cbOffset
                    visible: powerRampModel.pointCount == 1
                    onClicked: {
                        powerRampModel.asOffset = checked
                        tableView.forceLayout()
                    }
                    checked: powerRampModel.asOffset

                    ToolTip.text: qsTr("Add the powers instead of assigning them.")
                    ToolTip.visible: hovered
                    ToolTip.delay: 350
                }

                Label {
                    visible: powerRampModel.pointCount == 1
                    text: qsTr("Offset")
                }
            }

            TableView {
                Layout.fillWidth: true
                topMargin: columnsHeader.height + rowSpacing
                implicitHeight: Math.min(10, powerRampModel.pointCount) * (rowHeight + rowSpacing) + topMargin
                id: tableView

                boundsMovement: Flickable.StopAtBounds
                columnSpacing: 5
                rowSpacing: 5
                clip: true
                flickableDirection: Flickable.VerticalFlick
                ScrollBar.vertical: ScrollBar {
                    id: verticalScrollBar
                    width: 15
                    policy: tableView.height
                            > tableView.contentHeight ? ScrollBar.AsNeeded : ScrollBar.AlwaysOn
                }

                model: powerRampModel

                rowHeightProvider: function (row) {
                    return rowHeight
                }

                columnWidthProvider: function(column) {
                    if (powerRampModel.pointCount == 1 && (column == PowerRampModel.IdCol || column == PowerRampModel.ViewCol))
                    {
                        return 0; // "ID" and "View" are irrelevant when there is no ramp (only one point means "apply constant")
                    }

                    if ((powerRampModel.pointCount != 1 || powerRampModel.asOffset) && column == PowerRampModel.ResetCol)
                    {
                        return 0; // "Reset power" button should be only available for one global value (that is not used as offset)
                    }

                    if (column == PowerRampModel.IdCol)
                    {
                        return 60
                    }

                    if (column <= PowerRampModel.IdCol)
                    {
                        return 50
                    }

                    if (column >= PowerRampModel.ResetCol)
                    {
                        return 37 // NOTE: The toolbutton width is 35
                    }

                    if (column == PowerRampModel.PowerCol && !powerRampModel.modifyLaserPower)
                    {
                        return 0;
                    }

                    if (column == PowerRampModel.RingPowerCol && !powerRampModel.modifyRingPower)
                    {
                        return 0;
                    }

                    return colWidth
                }

                Item {
                    id: columnsHeader
                    y: tableView.contentY
                    width: tableView.width
                    height: 40
                    z: 2

                    Rectangle {
                        id: headerLabelBackground
                        anchors.fill: parent
                        border.width: 1
                        border.color: "darkgrey"
                        color: PrecitecApplication.Settings.alternateBackground
                    }

                    Item {
                        id: columnsHeaderItem
                        y: 1
                        x: 1
                        width: parent.width - 2 * headerLabelBackground.border.width
                        height: parent.height - 2 * headerLabelBackground.border.width
                        anchors.centerIn: parent

                        Row {
                            id: columnsHeaderCols

                            spacing: tableView.columnSpacing
                            Repeater {
                                model: tableView.columns > 0 ? tableView.columns : 1

                                Rectangle {
                                    width: tableView.columnWidthProvider(index)
                                    height: columnsHeaderItem.height
                                    visible: width != 0
//                                    border.width: 1
//                                    border.color: "darkgrey"
                                    color: "transparent"

                                    Label {
                                        anchors.fill: parent
                                        text: [
                                            "Name",
                                            "ID",
                                            "Power [%]",
                                            "Ring Power [%]",
                                            "", // Set -1
                                            "", // Add
                                            "", // View
                                            "" // Delete
                                        ][index]
                                        color: "white"
                                        font.pixelSize: 15
                                        verticalAlignment: Text.AlignVCenter
                                        horizontalAlignment: Text.AlignHCenter
                                    }
                                }
                            }
                        }
                    }
                }

                delegate: DelegateChooser {
                    id: delegateChooser

                    DelegateChoice {
                        column: 0
                        delegate: Label {
                            verticalAlignment: Text.AlignVCenter
                            text: name
                        }
                    }
                    DelegateChoice {
                        column: PowerRampModel.IdCol
                        delegate: Control {
                            TextField {
                                id: idField
                                selectByMouse: true
                                text: ID
                                width: tableView.columnWidthProvider(PowerRampModel.IdCol)
                                palette.text: idField.acceptableInput && powerRampModel.pointIdValidity[row] ? "black" : "red"
                                onEditingFinished: {
                                    ID = Number.fromLocaleString(locale, idField.text)
                                }
                                validator: IntValidator {
                                    bottom: 0
                                    top: maxPointId
                                    locale: Qt.locale().name
                                }
                            }
                        }
                    }
                    DelegateChoice {
                        column: PowerRampModel.PowerCol
                        delegate: Control {
                            TextField {
                                id: powerField
                                selectByMouse: true
                                text: Number(power).toLocaleString(locale)
                                width: tableView.columnWidthProvider(PowerRampModel.PowerCol)
                                onFocusChanged: {
                                    if (!focus && !acceptableInput)
                                    { // ensures that the current value is displayed when leaving the field with invalid input
                                        text = Number(power).toLocaleString(locale)
                                    }
                                }
                                onEditingFinished: {
                                    power = Number.fromLocaleString(locale, powerField.text)
                                }
                                palette.text: acceptableInput && powerRampModel.powerValidity[row] ? "black" : "red"
                                validator: DoubleValidator {
                                    bottom: -1000
                                    top: 1000
                                }
                            }
                        }
                    }
                    DelegateChoice {
                        column: PowerRampModel.RingPowerCol
                        delegate: Control {
                            TextField {
                                id: ringPowerField
                                selectByMouse: true
                                text: Number(ringPower).toLocaleString(locale)
                                width: tableView.columnWidthProvider(PowerRampModel.RingPowerCol)
                                onFocusChanged: {
                                    if (!focus && !acceptableInput)
                                    { // ensures that the current value is displayed when leaving the field with invalid input
                                        text = Number(ringPower).toLocaleString(locale)
                                    }
                                }
                                onEditingFinished: {
                                    ringPower = Number.fromLocaleString(locale, ringPowerField.text)
                                }
                                palette.text: acceptableInput && powerRampModel.ringPowerValidity[row] ? "black" : "red"
                                validator: DoubleValidator {
                                    bottom: -1000
                                    top: 1000
                                }
                            }
                        }
                    }

                    DelegateChoice {
                        column: PowerRampModel.ResetCol
                        delegate: Control {
                            ToolButton {
                                display: AbstractButton.TextOnly
                                text: "\u21B6"
                                icon.color: PrecitecApplication.Settings.alternateBackground
                                onClicked: {
                                    power = -1
                                    ringPower = -1
                                }

                                ToolTip.text: qsTr("Reset Power")
                                ToolTip.visible: parent.hovered
                                ToolTip.delay: 350
                            }
                        }
                    }
                    DelegateChoice {
                        column: PowerRampModel.AddCol
                        delegate: Control {
                            ToolButton {
                                display: AbstractButton.IconOnly
                                visible: !row || row != powerRampModel.pointCount - 1
                                icon.name: "list-add"
                                icon.color: PrecitecApplication.Settings.alternateBackground
                                onClicked: {
                                    powerRampModel.addPoint(row)
                                }

                                ToolTip.text: qsTr("Add point")
                                ToolTip.visible: parent.hovered
                                ToolTip.delay: 350
                            }
                        }
                    }
                    DelegateChoice {
                        column: PowerRampModel.ViewCol
                        delegate: Control {
                            ToolButton {
                                display: AbstractButton.IconOnly
                                icon.name: "quickview"
                                icon.color: PrecitecApplication.Settings.alternateBackground
                                onClicked: {
                                    // NOTE: This is copied from FigurePropertieTableDialog.qml
                                    wobbleFigureDataModel.setSelection(
                                                ID, graphView.parent.width,
                                                graphView.parent.height,
                                                powerRampDialog.width)
                                    graphView.grid.visible = false
                                    graphView.grid.visible = true
                                }

                                ToolTip.text: qsTr("View point")
                                ToolTip.visible: parent.hovered
                                ToolTip.delay: 350
                            }
                        }
                    }
                    DelegateChoice {
                        column: PowerRampModel.DeleteCol
                        delegate: Control {
                            ToolButton {
                                display: AbstractButton.IconOnly
                                visible: row != 0
                                icon.name: "edit-delete"
                                icon.color: PrecitecApplication.Settings.alternateBackground
                                onClicked: {
                                    powerRampModel.deletePoint(row)
                                }

                                ToolTip.text: qsTr("Remove point")
                                ToolTip.visible: parent.hovered
                                ToolTip.delay: 350
                            }
                        }
                    }
                }
            }

            RowLayout {
                visible: powerRampModel.periodAvailable

                CheckBox {
                    id: periodic
                    onClicked: {
                        powerRampModel.periodic = checked
                    }
                    checked: powerRampModel.periodic

                    ToolTip.text: qsTr("Apply ramp repeatedly.")
                    ToolTip.visible: hovered
                    ToolTip.delay: 350
                }

                Label {
                    id: periodicLabel
                    text: qsTr("Periodic")
                }
            }

            GridLayout {
                id: gridLayout
                visible: powerRampModel.periodAvailable && powerRampModel.periodic

                columns: 3
                rows: 2

                Label {
                    text: "Period start"
                    Layout.fillWidth: true
                }

                Label {
                    text: "Period end"
                    Layout.fillWidth: true
                }

                Label {
                    text: "Highest ID"
                    Layout.fillWidth: true
                }

                TextField {
                    id: periodStart
                    selectByMouse: true
                    text: powerRampModel.periodStart
                    implicitWidth: colWidth
                    palette.text: periodStart.acceptableInput && powerRampModel.periodValid ? "black" : "red"
                    onEditingFinished: {
                        powerRampModel.periodStart = Number.fromLocaleString(locale, periodStart.text)
                    }
                    validator: IntValidator {
                        bottom: 0
                        top: maxPointId
                        locale: Qt.locale().name
                    }

                    ToolTip.text: qsTr("ID of first point that gets modified.")
                    ToolTip.visible: hovered
                    ToolTip.delay: 350
                }

                TextField {
                    id: periodEnd
                    selectByMouse: true
                    text: powerRampModel.periodEnd
                    implicitWidth: colWidth
                    palette.text: periodEnd.acceptableInput  && powerRampModel.periodValid ? "black" : "red"
                    onEditingFinished: {
                        powerRampModel.periodEnd = Number.fromLocaleString(locale, periodEnd.text)
                    }
                    validator: IntValidator {
                        bottom: 0
                        top: maxPointId
                        locale: Qt.locale().name
                    }

                    ToolTip.text: qsTr("ID of last point that gets modified.")
                    ToolTip.visible: hovered
                    ToolTip.delay: 350
                }

                Label {
                    text: maxPointId
                }
            }

            PlotterChart {
                id: plotterChart
                Layout.fillWidth: true
                implicitHeight: 300

                controller {
                    id: controller
                    xLegendPrecision: 0
                    yLegendPrecision: 0
                }
                xAxisController {
                    id: xAxisController
                    autoAdjustXAxis: true
                }
                yAxisController {
                    yMin: 0.0
                    yMax: 100
                }

                xUnit: "ID"
                yUnit: "[%]"
                xLegendUnitBelowVisible: true
                yLegendRightVisible: false

                Component.onCompleted:
                {
                    updateGraph()
                }
            }

            Label {
                id: heightFiller
                Layout.fillHeight: true
            }

        }
    }
}
