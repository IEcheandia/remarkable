import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Shapes 1.0

import precitec.gui.components.image 1.0
import precitec.gui.components.application 1.0
import precitec.gui.components.plotter 1.0
import precitec.gui.components.userManagement 1.0

Dialog {
    id: measureDialog
    title: qsTr("Measure")
    property bool debugMeasurement: false
    property alias imageData: imageItem.imageData
    property alias paintedRect: imageItem.paintedRect
    property alias imageSize: imageItem.imageSize
    property alias zoom: imageItem.zoom
    property alias measurementFromGrayscaleImage: measurementController.grayscaleImage
    property alias calibrationCoordinatesRequestProxy: measurementController.calibrationCoordinatesRequestProxy
    property alias calibrationInitialized: measurementController.calibrationInitialized
    property alias lineLaser1Available: lineFilterModel.lineLaser1Available
    property alias lineLaser2Available: lineFilterModel.lineLaser2Available
    property alias lineLaser3Available: lineFilterModel.lineLaser3Available
    property alias screenshotTool: screenshotHeader.screenshotTool
    property bool validCoordinatesA: measurementController.coordinatesA.x != -1000 && measurementController.coordinatesA.y != -1000 && measurementController.coordinatesA.z != -1000
    property bool validCoordinatesB: measurementController.coordinatesB.x != -1000 && measurementController.coordinatesB.y != -1000 && measurementController.coordinatesB.z != -1000
    property bool validCoordinates: validCoordinatesA && validCoordinatesB //as defined in CalibrationCoordinatesRequestServer::get3DCoordinates


    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            measureDialog.reject()
        }
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    implicitWidth: 0.9 * parent.width
    implicitHeight: 0.8 * parent.height

    onVisibleChanged: {
        if (visible)
        {
            measurementController.updateTCP();
        }
    }

    onClosed: {
        measurementController.reset();
        measurementController.laserLine = lineFilterModel.mapToSource(lineFilterModel.index(0, 0)).row
        imageItem.panning.x = 0;
        imageItem.panning.y = 0;
        imageItem.zoomToFit();
        pointAButton.checked = false;
        pointBButton.checked = false;
        pointASelector.xPosition = 0;
        pointASelector.yPosition = 0;
        pointBSelector.xPosition = 0;
        pointBSelector.yPosition = 0;
        showCursorsCheckbox.checked = true;
        tcpCheckbox.checked = true;
    }

    header: DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: measureDialog.title
    }

    background: Rectangle {
        DragHandler {
            target: null
            grabPermissions: PointerHandler.CanTakeOverFromAnything
        }
        PinchHandler {
            target: null
            grabPermissions: PointerHandler.CanTakeOverFromAnything
        }
    }

    function vectorToString(coordinates)
    {
        return "(%1 / %2 / %3)".arg(coordinates.x.toLocaleString(measureDialog.locale))
                               .arg(coordinates.y.toLocaleString(measureDialog.locale))
                               .arg(coordinates.z.toLocaleString(measureDialog.locale));
    }

    function pointToString(point)
    {
        return "(%1 , %2 )".arg(point.x.toLocaleString(measureDialog.locale))
                            .arg(point.y.toLocaleString(measureDialog.locale));
    }
    
    modal: true

    // workaround for https://bugreports.qt.io/browse/QTBUG-72372
    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        Button {
            text: qsTr("Close")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

    ImageMeasurementController {
        id: measurementController
        pointA: pointASelector.selectedPoint
        pointB: pointBSelector.selectedPoint
        hwRoi: imageItem.hwROI
        hasScannerPosition : imageItem.hasScannerPosition
        scannerPosition: imageItem.scannerPosition
    }

    LineModel {
        id: lineModel
    }

    LineLaserFilterModel {
        id: lineFilterModel
        sourceModel: lineModel
    }

    RowLayout {
        anchors.fill: parent

        ColumnLayout {
            RowLayout {
                Item {
                    Layout.fillWidth: true
                }
                Label {
                    text: qsTr("Current zoom: %1 x").arg(Number(imageItem.zoom).toLocaleString(locale, "f", 1))
                    color: Settings.text
                }
                ToolButton {
                    icon.name: "zoom-original"
                    display: AbstractButton.IconOnly
                    onClicked: {
                        imageItem.zoom = 1.0;
                    }
                }
                ToolButton {
                    icon.name: "zoom-fit-best"
                    display: AbstractButton.IconOnly
                    onClicked: {
                        imageItem.panning.x = 0;
                        imageItem.panning.y = 0;
                        imageItem.zoomToFit();
                    }
                }
            }
            ImageItem {
                id: imageItem
                Layout.fillWidth: true
                Layout.fillHeight: true
                Item {
                    x: imageItem.paintedRect.x
                    y: imageItem.paintedRect.y
                    width: imageItem.paintedRect.width
                    height: imageItem.paintedRect.height

                    Rectangle {
                        width: 3
                        height: (100 / imageItem.imageSize.height) * imageItem.paintedRect.height
                        color: "lime"
                        visible: tcpCheckbox.checked && measurementController.tcpValid
                        x: (measurementController.tcp.x / imageItem.imageSize.width) * imageItem.paintedRect.width - 0.5 * width
                        y: (measurementController.tcp.y / imageItem.imageSize.height) * imageItem.paintedRect.height - 0.5 * height
                    }
                    Rectangle {
                        width: (100 / imageItem.imageSize.width) * imageItem.paintedRect.width
                        height: 3
                        color: "lime"
                        visible: tcpCheckbox.checked && measurementController.tcpValid
                        x: (measurementController.tcp.x / imageItem.imageSize.width) * imageItem.paintedRect.width - 0.5 * width
                        y: (measurementController.tcp.y / imageItem.imageSize.height) * imageItem.paintedRect.height - 0.5 * height
                    }
                    MouseArea {
                        anchors.fill: parent
                        onWheel: {
                            imageItem.zoom += (wheel.angleDelta.y / 120.0) * 0.1;
                        }
                    }
                    DragHandler {
                        property point startPanning
                        target: null
                        minimumPointCount: 1
                        maximumPointCount: 1
                        grabPermissions: PointerHandler.CanTakeOverFromAnything
                        onActiveChanged: {
                            if (active)
                            {
                                startPanning = imageItem.panning;
                                if (ApplicationWindow.activeFocusControl)
                                {
                                    ApplicationWindow.activeFocusControl.focus = false;
                                }
                            }
                        }
                        onTranslationChanged: {
                            imageItem.panning.x = startPanning.x + translation.x;
                            imageItem.panning.y = startPanning.y + translation.y;
                        }
                    }
                    PinchHandler {
                        property real startZoom: 1.0
                        target: null
                        maximumScale: imageItem.maxZoom
                        minimumScale: imageItem.minZoom
                        maximumRotation: 0
                        grabPermissions: PointerHandler.CanTakeOverFromAnything
                        onActiveChanged: {
                            if (active)
                            {
                                startZoom = imageItem.zoom;
                                if (ApplicationWindow.activeFocusControl)
                                {
                                    ApplicationWindow.activeFocusControl.focus = false;
                                }
                            }
                        }
                        onActiveScaleChanged: {
                            imageItem.zoom = startZoom * activeScale;
                        }
                    }
                }
                PointSelector {
                    id: pointASelector
                    image: imageItem
                    enabled: pointAButton.checked
                    visible: showCursorsCheckbox.checked
                }
                PointSelector {
                    id: pointBSelector
                    image: imageItem
                    enabled: pointBButton.checked
                    visible: showCursorsCheckbox.checked
                }

                Shape {
                    anchors.fill: parent
                    ShapePath {
                        id: connectionLine
                        function mapToImage(selectedPoint, rect)
                        {
                            // rect not used, just for property binding
                            return imageItem.mapToPaintedImage(selectedPoint);
                        }
                        strokeColor: Settings.alternateBackground
                        strokeWidth: 1
                        startX: imageItem.paintedRect.x + mapToImage(measurementController.pointA, imageItem.paintedRect).x
                        startY: imageItem.paintedRect.y + mapToImage(measurementController.pointA, imageItem.paintedRect).y
                        PathLine {
                            x: imageItem.paintedRect.x + connectionLine.mapToImage(measurementController.pointB, imageItem.paintedRect).x
                            y: imageItem.paintedRect.y + connectionLine.mapToImage(measurementController.pointB, imageItem.paintedRect).y
                        }
                    }
                }

                Component.onCompleted: {
                    imageItem.overlayGroupModel.sourceModel.enableAllOverlays(showAllOverlays.checked);
                }
            }
        }

        GridLayout {
            id: grid
            columns: 2
            Layout.fillHeight: true
            CheckBox {
                Layout.columnSpan: 2
                id: debugCheckbox
                checked: debugMeasurement
                text: "Show Advanced Information"
                onCheckedChanged: {
                    debugMeasurement = checked;
                }
            }
            CheckBox {
                Layout.columnSpan: 2
                id: showAllOverlays
                checked: false
                text: "Show Image Overlays"
                visible: debugMeasurement
                onCheckedChanged: {
                    imageItem.overlayGroupModel.sourceModel.enableAllOverlays(showAllOverlays.checked)
                }
            }
            CheckBox {
                Layout.columnSpan: 2
                id: showCursorsCheckbox
                checked: true
                visible: debugMeasurement
                text: "Show Point Cursors"
            }
            CheckBox {
                Layout.columnSpan: 2
                id: tcpCheckbox
                checked: true
                text: "Show TCP"
            }
            CheckBox {
                Layout.columnSpan: 2
                id: grayscaleCheckbox
                text: "2D Measurement"
                enabled: lineLaser1Available || lineLaser2Available || lineLaser3Available
                checked: measurementController.grayscaleImage
                onCheckedChanged: {
                    measurementController.grayscaleImage = checked;
                }
            }
            ComboBox {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                id: laserLineMenu
                visible: lineLaser1Available || lineLaser2Available || lineLaser3Available
                currentIndex: 0
                model: lineFilterModel
                textRole: "display"
                enabled: !measurementController.grayscaleImage
                onCurrentIndexChanged: {
                    measurementController.laserLine = lineFilterModel.mapToSource(lineFilterModel.index(laserLineMenu.currentIndex, 0)).row
                }
            }
            Button {
                Layout.preferredWidth: debugCheckbox.width - grid.columnSpacing - tcpButton.width
                id: pointAButton
                checkable: true
                icon.name: "measure"
                text: qsTr("Set 1. point")
                onClicked: {
                    if (checked && pointBButton.checked)
                    {
                        pointBButton.checked = false;
                    }
                    if (checked && !showCursorsCheckbox.checked)
                    {
                        showCursorsCheckbox.checked = true;
                    }
                }
            }
            ToolButton {
                id: tcpButton
                icon.name: "crosshairs"
                enabled: measurementController.tcpValid
                display: AbstractButton.IconOnly
                onClicked: {
                    pointAButton.checked = true;
                    pointBButton.checked = false;
                    pointASelector.selectedPoint = measurementController.tcp;
                    pointASelector.mapToImage();
                }
            }
            Button {
                Layout.preferredWidth: debugCheckbox.width - grid.columnSpacing - tcpButton.width
                id: pointBButton
                checkable: true
                icon.name: "measure"
                text: qsTr("Set 2. point")
                onClicked: {
                    if (checked && pointAButton.checked)
                    {
                        pointAButton.checked = false;
                    }
                    if (checked && !showCursorsCheckbox.checked)
                    {
                        showCursorsCheckbox.checked = true;
                    }
                }
            }
            ToolButton {
                icon.name: "crosshairs"
                enabled: measurementController.tcpValid
                display: AbstractButton.IconOnly
                onClicked: {
                    pointBButton.checked = true;
                    pointAButton.checked = false;
                    pointBSelector.selectedPoint = measurementController.tcp;
                    pointBSelector.mapToImage();
                }
            }
            Item {
                Layout.columnSpan: 2
                Layout.fillHeight: true
            }
            ArrowButtonGrid {
                Layout.columnSpan: 2
                Layout.alignment: Qt.AlignHCenter
                enabled: pointAButton.checked || pointBButton.checked
                leftEnabled: pointAButton.checked ? pointASelector.canMoveLeft : (pointBButton.checked ? pointBSelector.canMoveLeft : false)
                rightEnabled: pointAButton.checked ? pointASelector.canMoveRight : (pointBButton.checked ? pointBSelector.canMoveRight : false)
                upEnabled: pointAButton.checked ? pointASelector.canMoveUp : (pointBButton.checked ? pointBSelector.canMoveUp : false)
                downEnabled: pointAButton.checked ? pointASelector.canMoveDown : (pointBButton.checked ? pointBSelector.canMoveDown : false)

                onMoveLeft: pointAButton.checked ? pointASelector.moveLeft() : pointBSelector.moveLeft()
                onMoveDown: pointAButton.checked ? pointASelector.moveDown() : pointBSelector.moveDown()
                onMoveUp: pointAButton.checked ? pointASelector.moveUp() : pointBSelector.moveUp()
                onMoveRight: pointAButton.checked ? pointASelector.moveRight() : pointBSelector.moveRight()
            }
            GridLayout
            {
                Layout.columnSpan: 2
                Layout.fillHeight: true
                columns: 2
                visible: debugCheckbox.checked
                
                function updateX(pointSelector, x)
                {
                    pointAButton.checked = (pointSelector == pointASelector)
                    pointBButton.checked = (pointSelector == pointBSelector)
                    pointSelector.selectedPoint.x = x
                    pointSelector.mapToImage()
                }
                function updateY(pointSelector, y)
                {                    
                    pointAButton.checked = (pointSelector == pointASelector)
                    pointBButton.checked = (pointSelector == pointBSelector)
                    pointSelector.selectedPoint.y = y
                    pointSelector.mapToImage()
                }
                
                Label {
                    text: qsTr("1. X")
                    Layout.alignment: Qt.AlignRight
                }
                SpinBox {
                    editable: true
                    from: 0
                    to: imageItem.imageSize.width
                    value: measurementController.pointA.x
                    onValueModified: parent.updateX( pointASelector, value)
                
                }
                Label {
                    text: qsTr("1. Y")
                    Layout.alignment: Qt.AlignRight
                }
                SpinBox {
                    property bool editing: activeFocus || up.pressed || down.pressed
                    editable: true
                    from: 0
                    to: imageItem.imageSize.height
                    value: measurementController.pointA.y
                    onValueModified: parent.updateY( pointASelector,value)
                    onEditingChanged: {
                        if (editing)
                        {
                            pointAButton.checked = true;
                            pointBButton.checked = false;
                        }
                    }
                }
                Label {
                    text: qsTr("2. X")
                    Layout.alignment: Qt.AlignRight
                }
                SpinBox {
                    property bool editing: activeFocus || up.pressed || down.pressed
                    editable: true
                    from: 0
                    to: imageItem.imageSize.width - 1
                    value: measurementController.pointB.x
                    onValueModified: parent.updateX( pointBSelector, value)
                    onEditingChanged: {
                        if (editing)
                        {
                            pointAButton.checked = false;
                            pointBButton.checked = true;
                        }
                    }
                }
                Label {
                    text: qsTr("2. Y")
                    Layout.alignment: Qt.AlignRight
                }
                SpinBox {
                    property bool editing: activeFocus || up.pressed || down.pressed
                    editable: true
                    from: 0
                    to: imageItem.imageSize.height - 1
                    value: measurementController.pointB.y
                    onValueModified: parent.updateY( pointBSelector, value)
                    onEditingChanged: {
                        if (editing)
                        {
                            pointAButton.checked = false;
                            pointBButton.checked = true;
                        }
                    }
                }
            }
    
            Item {
                Layout.columnSpan: 2
                Layout.fillHeight: true
            }
        }
            
        
        ColumnLayout {
            Layout.maximumWidth: measureDialog.width * 0.3
            Layout.fillHeight: true

            GroupBox {
                Layout.fillWidth: true
                //title: qsTr("Coordinates")

                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    Label {
                        text: qsTr("Scanner Position")
                        Layout.alignment: Qt.AlignRight
                        visible: measurementController.hasScannerPosition && debugMeasurement
                    }
                    Label {
                        text: pointToString(measurementController.scannerPosition)
                        font.family: "monospace"
                        visible: measurementController.hasScannerPosition && debugMeasurement
                    }
                    
                    Label {
                        text: qsTr("HW ROI")
                        Layout.alignment: Qt.AlignRight
                        visible: debugMeasurement
                    }
                    Label {
                        text: pointToString(imageItem.hwROI)
                        font.family: "monospace"
                        visible: debugMeasurement
                    }
                    Label {
                        text: qsTr("1. point:")
                        Layout.alignment: Qt.AlignRight
                        visible: debugMeasurement
                    }
                    Label {
                        text: validCoordinatesA ? measureDialog.vectorToString(measurementController.coordinatesA) : "-"
                        font.family: "monospace"
                        visible: debugMeasurement
                    }
                    Label {
                        text: qsTr("2. point:")
                        Layout.alignment: Qt.AlignRight
                        visible: debugMeasurement
                    }
                    Label {
                        text: validCoordinatesB ? measureDialog.vectorToString(measurementController.coordinatesB) : "-"
                        font.family: "monospace"
                        visible: debugMeasurement
                    }
                    Label {
                        text: qsTr("1. point TCP distance [mm]:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        property var tcpDistance : measurementController.coordinatesA.minus(measurementController.coordinatesTCP)
                        text:  validCoordinatesA ? measureDialog.vectorToString(tcpDistance) : "-"
                        font.family: "monospace"
                        
                    }
                    Label {
                        text: qsTr("2. point TCP distance [mm]:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        property var tcpDistance : measurementController.coordinatesB.minus(measurementController.coordinatesTCP)
                        text:  validCoordinatesB ? measureDialog.vectorToString(tcpDistance) : "-"
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("TCP (pixel):")
                        Layout.alignment: Qt.AlignRight
                        visible: debugMeasurement
                    }
                    Label {
                        text: pointToString(measurementController.tcp)
                        font.family: "monospace"
                        visible: debugMeasurement
                    }
                    Label {
                        text: qsTr("Width (pixel):")
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        text: Math.round(measurementController.pointB.x - measurementController.pointA.x)
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("Height (pixel):")
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        text: Math.round(measurementController.pointB.y - measurementController.pointA.y)
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("Distance X (mm):")
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        text: validCoordinates ? measurementController.coordinatesB.minus(measurementController.coordinatesA).x.toLocaleString(measureDialog.locale) : "-"
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("Distance Y (mm):")
                        Layout.alignment: Qt.AlignRight
                        visible: debugMeasurement || measurementFromGrayscaleImage
                    }
                    Label {
                        text: validCoordinates ? measurementController.coordinatesB.minus(measurementController.coordinatesA).y.toLocaleString(measureDialog.locale) : "-"
                        font.family: "monospace"
                        visible: debugMeasurement || measurementFromGrayscaleImage
                    }
                    Label {
                        text: qsTr("Distance Z (mm):")
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        text: validCoordinates ? measurementController.coordinatesB.minus(measurementController.coordinatesA).z.toLocaleString(measureDialog.locale) : "-"
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("Length (mm):")
                        Layout.alignment: Qt.AlignRight
                        visible: debugMeasurement || measurementFromGrayscaleImage
                    }
                    Label {
                        text: validCoordinates ? measurementController.coordinatesB.minus(measurementController.coordinatesA).length().toLocaleString(measureDialog.locale) : "-"
                        font.family: "monospace"
                        visible: debugMeasurement || measurementFromGrayscaleImage
                    }
                    Label {
                        text: qsTr("1. point [pixel on Sensor]:")
                        Layout.alignment: Qt.AlignRight
                        visible: debugMeasurement
                    }
                    Label {
                        text: measureDialog.pointToString(Qt.point(measurementController.pointA.x + imageItem.hwROI.x, measurementController.pointA.y + imageItem.hwROI.y)) 
                        font.family: "monospace"
                        visible: debugMeasurement
                    }
                    Label {
                        text: qsTr("2. point [pixel on Sensor]:")
                        Layout.alignment: Qt.AlignRight
                        visible: debugMeasurement
                    }
                    Label {
                        text: measureDialog.pointToString(Qt.point(measurementController.pointB.x + imageItem.hwROI.x, measurementController.pointB.y + imageItem.hwROI.y))
                        font.family: "monospace"
                        visible: debugMeasurement
                    }
                        Label {
                            text: qsTr("Angle to x axis (xy plane, pixel) [deg]:")
                            Layout.alignment: Qt.AlignRight
                            visible: debugMeasurement
                        }
                        Label {
                            property real dx : measurementController.pointB.x - measurementController.pointA.x
                            property real dy : measurementController.pointB.y - measurementController.pointA.y
                            text: Number(Math.atan2(dy,dx)  * 180 / Math.PI ).toLocaleString()
                            font.family: "monospace"
                            visible: debugMeasurement
                        }
                        Label {
                            text: qsTr("Angle to x axis (xz plane, mm) [deg]:")
                            Layout.alignment: Qt.AlignRight
                            visible: debugMeasurement
                        }
                        Label {
                            property vector3d segment: measurementController.coordinatesA.minus(measurementController.coordinatesB)
                            text: validCoordinates ? Number(Math.atan2(segment.z, segment.x)  * 180 / Math.PI ).toLocaleString() : "-"
                            font.family: "monospace"
                            visible: debugMeasurement
                        }
                }
            }
            TabBar {
                id: selectionTabBar
                Layout.fillWidth: true
                TabButton {
                    text: qsTr("Histogram")
                }
                TabButton {
                    text: qsTr("Line Profile")
                }
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                PlotterChart {
                    id: histogramPlotter
                    visible: selectionTabBar.currentIndex == 0
                    anchors.fill: parent

                    yLegendRightVisible: false
                    yLeftLegendUnitVisible: true
                    yRightLegendUnitVisible: false
                    xUnit: qsTr("Grey level")
                    yUnit: qsTr("Count")

                    controller {
                        pointsVisible: false
                        lineSize: 2
                        xLegendPrecision: 0
                        yLegendPrecision: 0
                    }
                    xAxisController {
                        xRange: 256
                    }
                    yAxisController {
                        autoAdjustYAxis: true
                    }
                    zoomEnabled: false
                    panningEnabled: false
                    toolTipEnabled: true
                    menuAvailable: false
                    backgroundBorderColor: "transparent"
                    xLegendUnitBelowVisible: true
                    xLegendUnitVisible: false
                    restoreEnabled: false


                    ImageHistogramModel {
                        id: histogramModel
                        imageData: measureDialog.imageData
                        roi: Qt.rect(Math.min(measurementController.pointA.x, measurementController.pointB.x),
                                    Math.min(measurementController.pointA.y, measurementController.pointB.y),
                                    Math.abs(measurementController.pointB.x - measurementController.pointA.x),
                                    Math.abs(measurementController.pointB.y - measurementController.pointA.y))
                    }
                    Component.onCompleted: histogramPlotter.plotter.addDataSet(histogramModel.dataSet)
                }
                PlotterChart {
                    id: lineProfilePlotter
                    visible: selectionTabBar.currentIndex == 1
                    anchors.fill: parent

                    yLegendRightVisible: false
                    yLeftLegendUnitVisible: true
                    yRightLegendUnitVisible: false
                    xUnit: qsTr("Pixel")
                    yUnit: qsTr("Grey level")

                    controller {
                        pointsVisible: false
                        lineSize: 2
                        xLegendPrecision: 0
                        yLegendPrecision: 0
                    }
                    xAxisController {
                        autoAdjustXAxis: true
                    }
                    yAxisController {
                        yMin: 0
                        yMax: 260
                    }

                    zoomEnabled: false
                    panningEnabled: false
                    toolTipEnabled: true
                    menuAvailable: false
                    backgroundBorderColor: "transparent"
                    xLegendUnitBelowVisible: true
                    xLegendUnitVisible: false
                    restoreEnabled: false

                    IntensityProfileModel {
                        id: intensityProfileModel
                        imageData: measureDialog.imageData
                        startPoint: Qt.vector2d(measurementController.pointA.x, measurementController.pointA.y)
                        endPoint: Qt.vector2d(measurementController.pointB.x, measurementController.pointB.y)
                    }
                    Component.onCompleted: lineProfilePlotter.plotter.addDataSet(intensityProfileModel.dataSet)
                }
            }
        }
    }
}
