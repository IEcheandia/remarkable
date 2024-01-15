import QtQuick 2.7
import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import Qt.labs.qmlmodels 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

import wobbleFigureEditor.components 1.0

Control {
    id: figurePropertieTableDialog

    property var thisScreenshotTool: null
    property var actualFigure: null
    property var figureEditor: null
    property var figurePropertiesDialog: null
    property var graphView: null
    property alias wobbleFigureDataModel: tableView.model

    property int currentSelectionId: -1
    property int contentWidthProperty: 0
    property int contentYProperty: (currentSelectionId * 35) - 45

    signal dataChanged

    onDataChanged: {
        wobbleFigureDataModel.searchForNewLaserPoints()
        contentWidthProperty = tableView.model.contentWidth(
                    ) + ((tableView.model.activeColumnsCount(
                              ) - 1) * tableView.columnSpacing)
        implicitWidth = contentWidthProperty + (2 * contentPage.padding)
                + (verticalScrollBar.width + 10)
    }
    z: 2

    onCurrentSelectionIdChanged: {
        if (-1 != currentSelectionId) {
            var cheight = ((tableView.height - columnsHeader.height
                            - tableView.rowSpacing) / 35) / 2
            contentYProperty = (currentSelectionId * 35) - 45 - (cheight * 35)
            tableView.contentY = contentYProperty
            if (visible) {
                focus = true
            }
        }
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Down) {
            if (currentSelectionId < tableView.model.rowCount() - 1) {
                currentSelectionId++
            }
        }
        if (event.key === Qt.Key_Up) {
            if (currentSelectionId > 0) {
                currentSelectionId--
            }
        }
    }

    onVisibleChanged: {
        if (visible) {
            focus = true
            dataChanged()
            if (figurePropertiesDialog != null
                    && figurePropertiesDialog.visible) {
                figurePropertiesDialog.visible = false
            }
        } else {
            wobbleFigureDataModel.resetAllData()
        }
    }

    function readArrayValue(anArray) {
        return anArray[0]
    }
    function readArrayBool(anArray) {
        return anArray[1]
    }
    function isDualChannel() {
        if (figurePropertieTableDialog.figureEditor.figure.analogPower
                && FigureEditorSettings.dualChannelLaser
                && figurePropertieTableDialog.figureEditor.fileType != FileType.Overlay) {
            return true
        }
        return false
    }
    function getAndPrint(id) {
        if (id === undefined)
            return ""
        return id
    }
    contentItem: Page {
        id: contentPage

        padding: 10

        header: PrecitecApplication.DialogHeaderWithScreenshot {
            id: preHeader
            title: qsTr("Figure Properties")
            screenshotTool: thisScreenshotTool
        }
        footer: DialogButtonBox {
            id: preFooter
            alignment: Qt.AlignRight
            Button {
                text: qsTr("Close")
                onPressed: {
                    figurePropertieTableDialog.visible = false
                    if (currentSelectionId != -1) {
                        figurePropertiesDialog.visible = true
                    }
                }
            }
        }

        background: Rectangle {
            anchors.fill: parent
            color: "white"
            border.width: 1
            border.color: "grey"
        }

        contentItem: TableView {
            id: tableView

            topMargin: columnsHeader.height + rowSpacing
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

            model: WobbleFigureDataModel {
                figure: figurePropertieTableDialog.actualFigure
                graphView: figurePropertieTableDialog.graphView
                figureEditor: figurePropertieTableDialog.figureEditor
            }

            columnWidthProvider: function (column) {
                // Workaround: columnWidthProvider return invalid width
                return model.columnWidth(column)
            }

            rowHeightProvider: function (column) {
                return 30
            }

            Item {
                id: columnsHeader
                y: tableView.contentY
                width: contentWidthProperty
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

                            Label {
                                id: headerLabel
                                width: tableView.model.columnWidth(index)
                                height: columnsHeaderItem.height
                                text: tableView.model.isColumnVisible(
                                          index) ? tableView.model.headerData(
                                                       modelData,
                                                       Qt.Horizontal) : "0"
                                visible: tableView.model.isColumnVisible(index)
                                color: "white"
                                font.pixelSize: 15
                                //padding: 10
                                verticalAlignment: Text.AlignVCenter
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }
            }

            delegate: DelegateChooser {
                id: delegateChooser

                DelegateChoice {
                    column: 0
                    delegate: Control {
                        background: Rectangle {
                            id: back
                            color: currentSelectionId === row ? "white" : PrecitecApplication.Settings.alternateBackground
                            border.width: currentSelectionId === row ? 2 : 0
                            border.color: PrecitecApplication.Settings.alternateBackground
                        }
                        contentItem: Text {
                            text: getAndPrint(Id)
                            color: currentSelectionId === row ? PrecitecApplication.Settings.alternateBackground : "white"
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                        }
                        MouseArea {
                            anchors.fill: parent
                            onClicked: {
                                tableView.model.setSelection(
                                            row, graphView.parent.width,
                                            graphView.parent.height,
                                            figurePropertieTableDialog.width)
                                currentSelectionId = row
                                graphView.grid.visible = false
                                graphView.grid.visible = true
                            }
                        }
                    }
                }
                DelegateChoice {
                    column: 1
                    delegate: TextField {
                        id: xCoordinateLaserPoint
                        text: xCoord
                        palette.text: xCoordinateLaserPoint.acceptableInput ? "black" : "red"
                        validator: DoubleValidator {
                            bottom: -10000
                            top: 10000
                            decimals: 2
                            locale: Qt.locale().name
                        }
                        onEditingFinished: {
                            xCoord = text
                        }
                    }
                }
                DelegateChoice {
                    column: 2
                    delegate: TextField {
                        id: yCoordinateLaserPoint
                        text: yCoord
                        palette.text: yCoordinateLaserPoint.acceptableInput ? "black" : "red"
                        validator: DoubleValidator {
                            bottom: -10000
                            top: 10000
                            decimals: 2
                            locale: Qt.locale().name
                        }
                        onEditingFinished: {
                            yCoord = text
                        }
                    }
                }
                DelegateChoice {
                    column: 3
                    delegate: CheckBox {
                        id: lchecker
                        enabled: FigureEditorSettings.fileType == FileType.Seam
                        checkState: laserPowerCheckable ? Qt.Checked : Qt.Unchecked
                        onClicked: {
                            // Workaround - onCheckedStateChanged by inizilize the value this signal will be emit at start
                            laserPowerCheckable = checked
                        }
                    }
                }
                DelegateChoice {
                    column: 4
                    delegate: TextField {
                        id: laserPowerId
                        text: readArrayValue(laserPower)
                        enabled: readArrayBool(laserPower)
                        selectByMouse: true
                        validator: IntValidator {
                            bottom: figureEditor && figureEditor.fileType
                                    == FileType.Seam ? PowerLimits.Default : PowerLimits.MinValue
                            top: figureEditor
                                 && figureEditor.figure.analogPower ? PowerLimits.AnalogMaxValue : PowerLimits.DigitalMaxValue
                        }
                        onEditingFinished: {
                            laserPower = text
                        }
                    }
                }
                DelegateChoice {
                    column: 5
                    delegate: CheckBox {
                        property bool initValue: true
                        enabled: FigureEditorSettings.fileType == FileType.Seam
                        checkState: ringPowerCheckable ? Qt.Checked : Qt.Unchecked
                        onClicked: {
                            // Workaround - onCheckedStateChanged by inizilize the value this signal will be emit at start
                            ringPowerCheckable = checked
                        }
                    }
                }
                DelegateChoice {
                    column: 6
                    delegate: TextField {
                        id: ringPowerId
                        text: readArrayValue(ringPower)
                        enabled: readArrayBool(ringPower)
                        selectByMouse: true
                        validator: IntValidator {
                            bottom: figureEditor && figureEditor.fileType
                                    == FileType.Seam ? PowerLimits.Default : PowerLimits.MinValue
                            top: PowerLimits.AnalogMaxValue
                        }
                        onEditingFinished: {
                            ringPower = text
                        }
                    }
                }
                DelegateChoice {
                    column: 7
                    delegate: CheckBox {
                        property bool initValue: true
                        checkState: velocityCheckable ? Qt.Checked : Qt.Unchecked
                        onClicked: {
                            // Workaround - onCheckedStateChanged by inizilize the value this signal will be emit at start
                            velocityCheckable = checked
                        }
                    }
                }
                DelegateChoice {
                    column: 8
                    delegate: TextField {
                        id: velocityId
                        text: readArrayValue(velocity)
                        enabled: readArrayBool(velocity)
                        selectByMouse: true
                        validator: IntValidator {
                            bottom: VelocityLimits.MinValue
                            top: VelocityLimits.JumpMaxValue
                        }
                        onEditingFinished: {
                            velocity = text
                        }
                    }
                }
            }
        }
    }
}
