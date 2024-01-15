import QtQuick 2.12
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtQuick.Window 2.12

import QuickQanava 2.0 as Qan
import "qrc:/QuickQanava" as Qan

import wobbleFigureEditor.components 1.0

Item {
    id: wobbleFigureView
    property var actualObject: null
    property alias actualFigure: figureView.graph
    property var view: figureView
    property var figureEditor: null
    property bool endPositionSet: false
    property var selection: null
    property bool select: false
    property var screenshotTool: null
    property alias figureNameLabel: figureNameLabel
    property bool allowOverlappingPointsPopup: true

    /*
    When a point is clicked that overlaps with other points then a popup is opended to choose to point to interact with.
    The QML items of all overlapping points are disabled, except for the one that was chosen. The disabled points are
    rembered and get enabled when the choice should be discarded (which is the case after many interactions).
    */
    property var overlappingPointsDisabled: []
    property var overlappingPointsPopup: null

    function closeOverlappingPointsPopup() {
        if (wobbleFigureView.overlappingPointsPopup)
            wobbleFigureView.overlappingPointsPopup.close()
    }

    function enableOverlappedPoints() {
        wobbleFigureView.overlappingPointsDisabled.forEach(node => {node.item.enabled = true})
        wobbleFigureView.overlappingPointsDisabled = []
    }

    function processNodeClicked(node) {
        closeOverlappingPointsPopup()
        getNodePositionForFreeFigureCreator(
                    Qt.point(node.item.x + node.item.width * 0.5,
                             node.item.y + node.item.height * 0.5))

        wobbleFigureView.actualObject = node

        if (select && selection) {
            selection.addLaserPoint(node)
        }
    }

    property Item pointTooltip: null
    property int pointTooltipId: -1

    Timer {
        id: pointTooltipTimer
        interval: 700 // NOTE: Copied from app.style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay) of a QApplication
        onTriggered: {
            if (pointTooltip)
            {
                pointTooltip.destroy()
            }

            let text = figureEditor.getLaserPointTooltip(pointTooltipId)
            if (!text.length)
            {
                return
            }

            let point = wobbleFigure.searchPoint(pointTooltipId)

            let parent = figureView.Window.window.contentItem
            let pos = parent.mapFromItem(figureView.containerItem, point.center)
            pointTooltip = pointTooltipComponent.createObject(parent, {"x": pos.x, "y": pos.y});
            text.forEach((element, index) => {
                             let comp = index % 2 ? pointTooltipValueComponent : pointTooltipLabelComponent
                             comp.createObject(pointTooltip.contentItem, {"text": element, "font" : pointTooltip.toolTip.font, "color": pointTooltip.toolTip.color})
                         })

            pointTooltip.toolTip.show("")

        }
    }

    function destroyPointTooltip() {
        pointTooltipTimer.stop()
        if (!pointTooltip)
        {
            return
        }

        pointTooltip.destroy()
    }

    function getNodePositionForFreeFigureCreator(globalPosition) {
        globalPosition = Qt.point(
                    (globalPosition.x / wobbleFigureView.figureEditor.figureScale),
                    (globalPosition.y
                     / wobbleFigureView.figureEditor.figureScale)) //Get from px to mm
        if (figureEditor.figureCreator.segmentType === 0) {
            figureEditor.figureCreator.coordinateEnd = globalPosition
            figureEditor.figureCreator.createSegment()
            figureEditor.showFromFigureCreatorSegment()
        }
        if (figureEditor.figureCreator.segmentType === 1) {
            if (wobbleFigureView.endPositionSet === true) {
                figureEditor.figureCreator.coordinateCenter = globalPosition
                wobbleFigureView.endPositionSet = false
                figureEditor.figureCreator.createSegment()
                figureEditor.showFromFigureCreatorSegment()
            } else {
                figureEditor.figureCreator.coordinateEnd = globalPosition
                wobbleFigureView.endPositionSet = true
            }
        }
    }

    Label {
        id: figureNameLabel
        Layout.preferredWidth: figureNameLabel.implicitWidth
        z: 1
        anchors.horizontalCenter: figureView.horizontalCenter
        text: figureView && figureView.graph ? figureView.graph.name : qsTr(
                                                   "Figure name")
        font.bold: true

        wrapMode: Text.WrapAnywhere
        horizontalAlignment: Text.AlignHCenter
    }

    Qan.GraphView {
        id: figureView
        anchors.fill: parent
        clip: true
        zoomIncrement: 0.1
        zoom: 1.0
        zoomMin: 0.01
        zoomMax: 6.09

        grid: GridWithOrigin {
            id: gridWithOrigin
            containerItem: wobbleFigure.containerItem
            wobbleFigure: FigureEditorSettings.fileType == FileType.Wobble || FigureEditorSettings.fileType == FileType.Basic
            figureScale: wobbleFigureView.figureEditor ? wobbleFigureView.figureEditor.figureScale : 1000

            onScaleChanged: {
                figureView.grid.visible = false //force redrawing of the background
                figureView.grid.visible = true
            }
        }

        graph: WobbleFigure {
            id: wobbleFigure

            onNodeMoved: {
                wobbleFigureView.figureEditor.updatePosition(node)
                wobbleFigureView.actualObject = node
            }

            WobbleFigureDataModel {
                id: overlappingPointsModel
                figure: wobbleFigure
                graphView: figureView
                figureEditor: wobbleFigureView.figureEditor
            }

            Component {
                id: popupComponent
                Popup {
                    id: control

                    topMargin: 6
                    bottomMargin: 6

                    contentItem: ListView {
                        id: pointsList
                        clip: true
                        model: overlappingPointsModel
                        implicitHeight: contentHeight
                        implicitWidth: 100
                        highlightMoveDuration: 0

                        delegate: ItemDelegate {
                            text: qsTr("Point %1").arg(model.Id)
                            onClicked: {
                                let node = wobbleFigure.searchPoint(model.Id)
                                node.item.enabled = true
                                processNodeClicked(node)
                                wobbleFigure.clearSelection()
                                wobbleFigure.selectNode(node)
                                close();
                            }
                        }

                        ScrollBar.vertical: ScrollBar {}
                    }
                }
            }

            onNodeAboutToBeMoved: {
                closeOverlappingPointsPopup()
                enableOverlappedPoints()
                enableOverlappedPoints()
            }

            onNodeClicked: {
                destroyPointTooltip()
                wobbleFigureView.actualObject = node
                if (select && selection) {
                    selection.addLaserPoint(node)
                }
                let posGlobal = node.item.mapToItem(figureView.containerItem, pos.x, pos.y)
                overlappingPointsModel.setFilterCircle(posGlobal, node.item.width * 0.5)
                overlappingPointsModel.searchForNewLaserPoints()

                if (allowOverlappingPointsPopup && overlappingPointsModel.rowCount() > 1)
                {
                    enableOverlappedPoints()
                    overlappingPointsDisabled = []
                    overlappingPointsModel.getIds().forEach(
                                (id) => {
                                        let node = wobbleFigure.searchPoint(id)
                                        node.item.enabled = false
                                        overlappingPointsDisabled.push(node)
                                    })


                    var popup = popupComponent.createObject(figureView.containerItem, {"x": posGlobal.x, "y": posGlobal.y});
                    wobbleFigureView.overlappingPointsPopup = popup
                    wobbleFigureView.actualObject = null

                    if (selection)
                        selection.resetLaserPoints()
                    popup.open();
                    return
                }

                enableOverlappedPoints()
                processNodeClicked(node)
            }
            onEdgeClicked: {
                destroyPointTooltip()
                wobbleFigure.clearSelection()
                wobbleFigureView.actualObject = edge
                closeOverlappingPointsPopup()
                enableOverlappedPoints()
            }

            // Shows a table with the properties of a LaserPoint. The Item parent is there to implement reliable
            // shifting the tooltip to the top left of its position if there is enough space.
            Component {
                id: pointTooltipComponent

                Item {
                    property alias contentItem: toolTip.contentItem
                    property alias toolTip: toolTip

                    ToolTip {
                        id: toolTip
                        enabled: false

                        contentItem: GridLayout {
                            columns: 2
                        }

                        x: parent.x >= width ? -width : 0
                        y: parent.y >= height ? -height : 0
                    }
                }
            }

            // Used to render labels (left column) of the tooltip
            Component {
                id: pointTooltipLabelComponent
                Label {
                    Layout.alignment: Qt.AlignRight
                }
            }

            // Used to render values (right column) of the tooltip
            Component {
                id: pointTooltipValueComponent
                Label {
                }
            }

            onMouseEnteredPoint: {
                destroyPointTooltip()
                pointTooltipId = id
                pointTooltipTimer.restart()
            }
            onMouseExitedPoint: {
                destroyPointTooltip()
            }
        }

        onClicked: {
            destroyPointTooltip()
            closeOverlappingPointsPopup()
            enableOverlappedPoints()
            var posGlobal = mapToItem(figureView.containerItem, pos.x, pos.y)
            //Global position of the graph
            getNodePositionForFreeFigureCreator(posGlobal)
            wobbleFigureView.actualObject = null
            if (select && selection) {
                selection.resetLaserPoints()
            }
        }

        PinchHandler {
            target: null
            property real startZoom: 1.0
            minimumScale: 0.1
            onActiveChanged: {
                startZoom = figureView.zoom
            }
            onActiveScaleChanged: {
                figureView.zoom = startZoom * activeScale
            }
        }
    }
}
