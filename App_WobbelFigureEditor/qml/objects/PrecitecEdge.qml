import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Shapes 1.0

import QuickQanava 2.0 as Qan

import wobbleFigureEditor.components 1.0

Item {
    id: edgeTemplate
    antialiasing: true
    property var edgeItem: undefined

    property color color: edgeItem
                          && edgeItem.style ? edgeItem.style.lineColor : Qt.rgba(
                                                  0., 0., 0., 1.)
    property bool isRampEdge: false
    property bool hasGradient: false
    property color startColor: Qt.rgba(0., 0., 0., 1.)
    property color endColor: Qt.rgba(0., 0., 0., 1.)
    property double startPosition: 0.0
    property double endPosition: 1.0
    property double recStrength: 1

    // Allow direct bypass of style
    property var lineType: edgeItem.style ? edgeItem.style.lineType : Qan.EdgeStyle.Straight
    property var dashed: edgeItem.style
                         && style.dashed ? ShapePath.DashLine : ShapePath.SolidLine

    visible: edgeItem.visible && !edgeItem.hidden

    Shape {
        id: dstShape
        antialiasing: true
        smooth: true
        visible: dstShapeType !== Qan.EdgeStyle.None
        transformOrigin: Item.TopLeft
        rotation: edgeItem.dstAngle
        x: edgeItem.p2.x
        y: edgeItem.p2.y

        property var dstArrow: undefined
        property var dstCircle: undefined
        property var dstRect: undefined
        property var dstShapeType: edgeItem.dstShape
        onDstShapeTypeChanged: {
            switch (dstShapeType) {
            case Qan.EdgeStyle.None:
                break
            case Qan.EdgeStyle.Arrow:
                // falltrought
            case Qan.EdgeStyle.ArrowOpen:
                if (dstCircle)
                    dstCircle.destroy()
                if (dstRect)
                    dstRect.destroy()
                dstShape.data = dstArrow = preEdgeDstArrowPathComponent.createObject(
                            dstShape, {
                                "edgeTemplate": edgeTemplate
                            })
                break
            case Qan.EdgeStyle.Circle:
                // falltrought
            case Qan.EdgeStyle.CircleOpen:
                if (dstArrow)
                    dstArrow.destroy()
                if (dstRect)
                    dstRect.destroy()
                dstShape.data = dstCircle = preEdgeDstCirclePathComponent.createObject(
                            dstShape, {
                                "edgeTemplate": edgeTemplate
                            })
                break
            case Qan.EdgeStyle.Rect:
                // falltrought
            case Qan.EdgeStyle.RectOpen:
                if (dstArrow)
                    dstArrow.destroy()
                if (dstCircle)
                    dstCircle.destroy()
                dstShape.data = dstRect = preEdgeDstRectPathComponent.createObject(
                            dstShape, {
                                "edgeTemplate": edgeTemplate
                            })
                break
            }
        }
    } // Shape: dstShape

    Shape {
        id: srcShape
        antialiasing: true
        smooth: true
        visible: srcShapeType !== Qan.EdgeStyle.None

        transformOrigin: Item.TopLeft
        rotation: edgeItem.srcAngle
        x: edgeItem.p1.x
        y: edgeItem.p1.y

        property var srcArrow: undefined
        property var srcCircle: undefined
        property var srcRect: undefined
        property var srcShapeType: edgeItem.srcShape
        onSrcShapeTypeChanged: {
            switch (srcShapeType) {
            case Qan.EdgeStyle.None:
                break
            case Qan.EdgeStyle.Arrow:
                // falltrought
            case Qan.EdgeStyle.ArrowOpen:
                if (srcCircle)
                    srcCircle.destroy()
                if (srcRect)
                    srcRect.destroy()
                srcShape.data = srcArrow = preEdgeSrcArrowPathComponent.createObject(
                            srcShape, {
                                "edgeTemplate": edgeTemplate
                            })
                break
            case Qan.EdgeStyle.Circle:
                // falltrought
            case Qan.EdgeStyle.CircleOpen:
                if (srcArrow)
                    srcArrow.destroy()
                if (srcRect)
                    srcRect.destroy()
                srcShape.data = srcCircle = preEdgeSrcCirclePathComponent.createObject(
                            dstShape, {
                                "edgeTemplate": edgeTemplate
                            })
                break
            case Qan.EdgeStyle.Rect:
                // falltrought
            case Qan.EdgeStyle.RectOpen:
                if (srcArrow)
                    srcArrow.destroy()
                if (srcCircle)
                    srcCircle.destroy()
                srcShape.data = srcRect = preEdgeSrcRectPathComponent.createObject(
                            dstShape, {
                                "edgeTemplate": edgeTemplate
                            })
                break
            }
        }
    } // Shape: srcShape

    Shape {
        id: edgeSelectionShape
        anchors.fill: parent

        visible: false //edgeItem && edgeItem.visible && !edgeItem.hidden
        //&& edgeItem.selected // Not very efficient, use a Loader there... // todo: commented cause property it is not available
        antialiasing: true
        smooth: true
        property var curvedLine: undefined
        property var straightLine: undefined
        property var orthoLine: undefined
        property var lineType: edgeTemplate.lineType
        property var lineWidth: edgeItem
                                && edgeItem.style ? edgeItem.style.lineWidth + 2. : 4.
        property var lineColor: edgeItem
                                && edgeItem.graph ? edgeItem.graph.selectionColor : Qt.rgba(
                                                        0.1176, 0.5647, 1.,
                                                        1.) // dodgerblue=rgb(30, 144, 255)
        onLineTypeChanged: updateSelectionShape()
        onVisibleChanged: updateSelectionShape()
        function updateSelectionShape() {
            if (!visible)
                return
            switch (lineType) {
            case Qan.EdgeStyle.Undefined:
                // falltrought
            case Qan.EdgeStyle.Straight:
                if (orthoLine)
                    orthoLine.destroy()
                if (curvedLine)
                    curvedLine.destroy()
                edgeSelectionShape.data = straightLine = preEdgeStraightPathComponent.createObject(
                            edgeSelectionShape, {
                                "edgeTemplate": edgeTemplate,
                                "strokeWidth": lineWidth,
                                "strokeColor": lineColor
                            })
                break
            case Qan.EdgeStyle.Ortho:
                if (straightLine)
                    straightLine.destroy()
                if (curvedLine)
                    curvedLine.destroy()
                edgeSelectionShape.data = orthoLine = preEdgeOrthoPathComponent.createObject(
                            edgeSelectionShape, {
                                "edgeTemplate": edgeTemplate,
                                "strokeWidth": lineWidth,
                                "strokeColor": lineColor
                            })
                break
            case Qan.EdgeStyle.Curved:
                if (straightLine)
                    straightLine.destroy()
                if (orthoLine)
                    orthoLine.destroy()
                edgeSelectionShape.data = curvedLine = preEdgeCurvedPathComponent.createObject(
                            edgeSelectionShape, {
                                "edgeTemplate": edgeTemplate,
                                "strokeWidth": lineWidth,
                                "strokeColor": lineColor
                            })
                break
            }
        }
    } // Shape: edgeSelectionShape

    Shape {
        id: edgeShape
        //anchors.fill: parent
        width: edgeItem.p2.x - edgeItem.p1.x
        height: edgeItem.p2.y - edgeItem.p1.y
        visible: edgeItem.visible && !edgeItem.hidden
        antialiasing: true
        smooth: true
        property var curvedLine: undefined
        property var straightLine: undefined
        property var orthoLine: undefined
        property var lineType: edgeTemplate.lineType
        property bool hasGradient: edgeTemplate.hasGradient
        property double startPosition: edgeTemplate.startPosition
        property double endPosition: edgeTemplate.endPosition
        property double recStrength: edgeTemplate.recStrength
        onRecStrengthChanged: updateLine()
        onStartPositionChanged: updateLine()
        onEndPositionChanged: updateLine()
        onHasGradientChanged: updateLine()
        onLineTypeChanged: updateLine()
        function updateLine() {
            switch (lineType) {
            case Qan.EdgeStyle.Undefined:
                // falltrought
            case Qan.EdgeStyle.Straight:
                if (orthoLine)
                    orthoLine.destroy()
                if (curvedLine)
                    curvedLine.destroy()
                edgeShape.data = straightLine
                        = edgeShape.hasGradient ? preEdgeStraightPathRampComponent.createObject(
                                                      edgeShape, {
                                                          "edgeTemplate": edgeTemplate,
                                                          "startColor": edgeTemplate.startColor,
                                                          "endColor": edgeTemplate.endColor,
                                                          "startPosition": edgeTemplate.startPosition,
                                                          "endPosition": edgeTemplate.endPosition,
                                                          "recStrength": edgeTemplate.recStrength
                                                      }) : preEdgeStraightPathComponent.createObject(
                                                      edgeShape, {
                                                          "edgeTemplate": edgeTemplate
                                                      })
                break
            case Qan.EdgeStyle.Ortho:
                if (straightLine)
                    straightLine.destroy()
                if (curvedLine)
                    curvedLine.destroy()
                edgeShape.data = orthoLine = preEdgeOrthoPathComponent.createObject(
                            edgeShape, {
                                "edgeTemplate": edgeTemplate
                            })
                break
            case Qan.EdgeStyle.Curved:
                if (straightLine)
                    straightLine.destroy()
                if (orthoLine)
                    orthoLine.destroy()
                edgeShape.data = curvedLine = preEdgeCurvedPathComponent.createObject(
                            edgeShape, {
                                "edgeTemplate": edgeTemplate
                            })
                break
            }
        }
    } // Shape: edgeShape
    Component {
        id: preEdgeCurvedPathComponent
        PrecitecEdgeCurvedPath {
            id: precitecEdgeCurvedPath
        }
    }
    Component {
        id: preEdgeOrthoPathComponent
        PrecitecEdgeOrthoPath {
            id: precitecEdgeOrthoPath
        }
    }
    Component {
        id: preEdgeStraightPathComponent
        PrecitecEdgeStraightPath {
            id: precitecEdgeStraightPath
        }
    }
    Component {
        id: preEdgeStraightPathRampComponent
        PrecitecEdgeStraightPathRamp {
            id: precitecEdgeStraightPathRamp
        }
    }

    Component {
        id: preEdgeSrcRectPathComponent
        PrecitecEdgeSrcRectPath {
            id: precitecEdgeSrcRectPath
        }
    }
    Component {
        id: preEdgeSrcCirclePathComponent
        PrecitecEdgeSrcCirclePath {
            id: precitecEdgeSrcCirclePath
        }
    }
    Component {
        id: preEdgeSrcArrowPathComponent
        PrecitecEdgeSrcArrowPath {
            id: precitecEdgeSrcArrowPath
        }
    }

    Component {
        id: preEdgeDstRectPathComponent
        PrecitecEdgeDstRectPath {
            id: precitecEdgeDstRectPath
        }
    }
    Component {
        id: preEdgeDstCirclePathComponent
        PrecitecEdgeDstCirclePath {
            id: precitecEdgeDstCirclePath
        }
    }
    Component {
        id: preEdgeDstArrowPathComponent
        PrecitecEdgeDstArrowPath {
            id: precitecEdgeDstArrowPath
        }
    }
} // Item: edgeTemplate
