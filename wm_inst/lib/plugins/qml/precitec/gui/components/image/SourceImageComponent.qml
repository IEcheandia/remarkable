import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import precitec.gui.components.image 1.0
import precitec.gui.components.application 1.0

/**
 * @brief Component to render a precitec image with controls for zooming and overlays
 *
 * The main difference to ImageItem is that it provides the control for zoom and overlays.
 **/
Item {
    id: root
    property alias source: imageItem.source
    property alias paintedRect: imageItem.paintedRect
    property alias imageSize: imageItem.imageSize
    property alias zoom: imageItem.zoom
    property bool handlersEnabled: true
    property alias menuVisible: menuButton.visible

    function mapToPaintedImage(point)
    {
        return imageItem.mapToPaintedImage(point);
    }

    function mapFromPaintedImage(point)
    {
        return imageItem.mapFromPaintedImage(point);
    }

    Rectangle {
        width: imageItem.width
        height: imageItem.height
        x: imageItem.x
        y: imageItem.y
        color: "white"
        border {
            width: 1
            color: Settings.alternateBackground
        }
    }
    
    SourceImageItem {
        id: imageItem
        anchors.fill: parent
        Item {
            x: imageItem.paintedRect.x
            y: imageItem.paintedRect.y
            width: imageItem.paintedRect.width
            height: imageItem.paintedRect.height
            MouseArea {
                anchors.fill: parent
                onWheel: {
                    imageItem.zoom += (wheel.angleDelta.y / 120.0) * 0.1;
                }
            }
            DragHandler {
                property point startPanning
                enabled: root.handlersEnabled
                target: null
                minimumPointCount: 1
                maximumPointCount: 1
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
                enabled: root.handlersEnabled
                maximumScale: imageItem.maxZoom
                minimumScale: imageItem.minZoom
                maximumRotation: 0
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
    }
    BusyIndicator {
        running: imageItem.loading
        anchors.centerIn: parent
    }
    Label {
        visible: !imageItem.loading && !imageItem.imageValid
        text: qsTr("Image not available")
        anchors.centerIn: parent
    }

    RowLayout {
        id: configRow
        enabled: imageItem.imageValid && imageItem.visible
        z: 1
        anchors {
            top: parent.top
            right: parent.right
            topMargin: spacing
            rightMargin: spacing
        }
        Label {
            visible: menuButton.checked
            text: qsTr("%1 x").arg(Number(imageItem.zoom).toLocaleString(locale, "f", 1))
            color: Settings.text
        }
        Slider {
            visible: menuButton.checked
            from: imageItem.minZoom
            to: imageItem.maxZoom
            stepSize: 0.1
            value: imageItem.zoom
            onValueChanged: {
                imageItem.zoom = value;
            }
        }
        ToolButton {
            id: zoomOrigButton
            visible: menuButton.checked
            icon.name: "zoom-original"
            display: AbstractButton.IconOnly
            onClicked: {
                imageItem.zoom = 1.0;
                imageItem.panning.x = 0;
                imageItem.panning.y = 0;
            }
        }
        ToolButton {
            id: zoomFitButton
            visible: menuButton.checked
            icon.name: "zoom-fit-best"
            display: AbstractButton.IconOnly
            onClicked: imageItem.zoomToFit()
        }
        ToolButton {
            id: menuButton
            checkable: true
            checked: false
            icon.name: "application-menu"
            display: AbstractButton.IconOnly
        }
    }
}
