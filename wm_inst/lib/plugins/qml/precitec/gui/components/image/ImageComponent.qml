import QtQuick 2.15
import QtQml 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import precitec.gui.components.image 1.0
import precitec.gui.components.application 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0
import Precitec.AppGui 1.0

/**
 * @brief Component to render a precitec image with controls for zooming and overlays
 *
 * The main difference to ImageItem is that it provides the control for zoom and overlays.
 **/
Item {
    id: root
    property alias paintedRect: imageItem.paintedRect
    property alias imageSize: imageItem.imageSize
    property alias zoom: imageItem.zoom
    property bool debugMeasurement: false
    property alias filterInstances: infoBox.filterInstances
    property alias simulationController: infoBox.simulationController
    property bool hasInfoBox: false
    property bool handlersEnabled: true
    property bool overlaysEnabled: true
    property alias menuVisible: menuButton.visible
    property alias lineOverlayEnabled: imageItem.lineOverlayEnabled
    property alias contourOverlayEnabled: imageItem.contourOverlayEnabled
    property alias positionOverlayEnabled: imageItem.positionOverlayEnabled
    property alias textOverlayEnabled: imageItem.textOverlayEnabled
    property alias gridOverlayEnabled: imageItem.gridOverlayEnabled
    property alias imageOverlayEnabled: imageItem.imageOverlayEnabled
    property bool referenceEnabled: false
    property var screenshotTool: null
    property bool simulation: false
    property bool mirroringSupported: true
    readonly property bool inputHandlersActive: (!measureDialogLoader.active || !measureDialogLoader.item.visible) && (!extractTemplateDialogLoader.active || !extractTemplateDialogLoader.item.visible) && !infoBox.visible && root.handlersEnabled

    onOverlaysEnabledChanged: imageItem.overlayGroupModel.sourceModel.enableAllOverlays(root.overlaysEnabled)

    signal closeInfoBox()
    signal updateInfoBox()
    signal save(string path)
    signal compareReference()
    signal play()
    signal pause()
    signal stop()
    signal nextSeam()
    signal previousSeam()
    signal next()
    signal previous()

    onSave: {
        imageItem.save(path)
    }

    function mapToPaintedImage(point)
    {
        return imageItem.mapToPaintedImage(point);
    }

    function mapFromPaintedImage(point)
    {
        return imageItem.mapFromPaintedImage(point);
    }

    onCloseInfoBox: {
        infoBox.close();
    }

    onUpdateInfoBox: {
        if (infoBox.visible)
        {
            infoBox.imageData = imageItem.imageData;
        }
    }

    Loader {
        id: measureDialogLoader
        active: false
        asynchronous: true
        sourceComponent: MeasureDialog {
            id: measureDialog
            calibrationCoordinatesRequestProxy: simulation ? SimulationModule.calibrationCoordinatesRequestProxy : HardwareModule.calibrationCoordinatesRequestProxy
            debugMeasurement: root.debugMeasurement
            lineLaser1Available: SystemConfiguration.LineLaser1Enable
            lineLaser2Available: SystemConfiguration.LineLaser2Enable
            lineLaser3Available: HardwareModule.lineLaser3Enabled
            screenshotTool: root.screenshotTool
        }
        Connections {
            target: imageItem
            enabled: !measureDialogLoader.active
            function onImageDataChanged() {
                if (imageItem.valid)
                {
                    measureDialogLoader.active = true;
                }
            }
        }
    }

    Loader {
        id: extractTemplateDialogLoader
        active: false
        asynchronous: true
        sourceComponent: ExtractTemplateDialog {
            id: extractTemplate
            screenshotTool: root.screenshotTool
        }
        Connections {
            target: imageItem
            enabled: !extractTemplateDialogLoader.active
            function onImageDataChanged() {
                if (imageItem.valid)
                {
                    extractTemplateDialogLoader.active = true;
                }
            }
        }
    }

    InfoBoxDialog {
        id: infoBox
        screenshotTool: root.screenshotTool
        onPlay: root.play()
        onPause: root.pause()
        onStop: root.stop()
        onNextSeam: root.nextSeam()
        onPreviousSeam: root.previousSeam()
        onNext: root.next()
        onPrevious: root.previous()
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

    Binding {
        target: imageItem
        when: root.visible
        property: "imageData"
        restoreMode: Binding.RestoreBinding
        value: imageItem.recorder.imageData
    }

    ImageItem {
        property var recorder: root.simulation ? SimulationModule.recorder : HardwareModule.recorder

        id: imageItem
        onRendered: recorder.requestNextImage()
        onImageDataChanged: updateInfoBox()
        anchors.fill: parent
        Item {
            x: imageItem.paintedRect.x
            y: imageItem.paintedRect.y
            width: imageItem.paintedRect.width
            height: imageItem.paintedRect.height
            DragHandler {
                property point startPanning
                enabled: root.inputHandlersActive
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
            WheelHandler {
                enabled: root.inputHandlersActive
                target: null
                onWheel: {
                    imageItem.zoom += (event.angleDelta.y / 120.0) * 0.1;
                }
            }
            PinchHandler {
                property real startZoom: 1.0
                target: null
                enabled: root.inputHandlersActive
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

    RowLayout {
        id: configRow
        enabled: imageItem.valid
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
            id: mirrorXButton
            objectName: "image-component-mirror-x"
            enabled: root.mirroringSupported
            visible: menuButton.checked
            icon.name: "image-flip-horizontal-symbolic"
            display: AbstractButton.IconOnly
            checkable: true
            checked: imageItem.mirrorX
            onClicked: {
                imageItem.mirrorX = !imageItem.mirrorX;
            }
        }
        ToolButton {
            id: mirrorYButton
            objectName: "image-component-mirror-y"
            enabled: root.mirroringSupported
            visible: menuButton.checked
            icon.name: "image-flip-vertical-symbolic"
            display: AbstractButton.IconOnly
            checkable: true
            checked: imageItem.mirrorY
            onClicked: {
                imageItem.mirrorY = !imageItem.mirrorY;
            }
        }
        ToolButton {
            id: measureButton
            enabled: measureDialogLoader.item && measureDialogLoader.item.calibrationInitialized
            visible:  menuButton.checked
            display: AbstractButton.IconOnly
            onClicked: {
                measureDialogLoader.item.imageData = imageItem.imageData
                measureDialogLoader.item.open();
            }
            icon.name: "tool-measure"
        }
        ToolButton {
            id: extractTemplate
            enabled: extractTemplateDialogLoader.item
            visible: menuButton.checked && UserManagement.currentUser && UserManagement.hasPermission(App.SaveTemplate)
            display: AbstractButton.IconOnly
            objectName: "image-component-extract-template-button"
            icon.name: "transform-crop"
            onClicked: {
                extractTemplateDialogLoader.item.imageData = imageItem.imageData
                extractTemplateDialogLoader.item.open();
            }
        }
        ToolButton {
            id: saveButton
            enabled:  imageItem.valid
            visible:  menuButton.checked
            display: AbstractButton.IconOnly
            onClicked: imageItem.save()
            icon.name: "document-save"
        }
        ToolButton {
            objectName: "image-component-save-wtih-overlays"
            enabled:  imageItem.valid
            visible:  menuButton.checked
            display: AbstractButton.IconOnly
            onClicked: imageItem.saveWithOverlays()
            icon.name: "camera-photo"
        }
        ToolButton {
            id: infoButton
            enabled:  imageItem.valid
            visible:  menuButton.checked && hasInfoBox
            display: AbstractButton.IconOnly
            onClicked: {
                infoBox.imageData = imageItem.imageData
                infoBox.open();
            }
            icon.name: "document-properties"
        }
        ToolButton {
            id: referenceButton
            enabled: imageItem.valid
            visible:  menuButton.checked && root.referenceEnabled
            display: AbstractButton.IconOnly
            onClicked: {
                root.compareReference()
            }
            icon.name: "wizard-reference-image"
        }
        ToolButton {
            id: menuButton
            checkable: true
            checked: false
            icon.name: "application-menu"
            display: AbstractButton.IconOnly
        }
    }
    Control {
        anchors {
            top: configRow.bottom
            right: parent.right
        }
        leftInset: configRow.spacing
        rightInset: configRow.spacing
        topInset: configRow.spacing
        bottomInset: configRow.spacing
        visible: menuButton.checked
        background: Rectangle {
            color: parent.palette.button
            opacity: 0.5
        }
        contentItem: ListView {
            id: overlayGroupsList
            model: imageItem.overlayGroupModel
            width: implicitWidth
            implicitHeight: childrenRect.height
            interactive: false
            clip: true
            delegate: CheckBox {
                function updateImplicitWidth()
                {
                    if (delegate.implicitWidth > delegate.ListView.view.implicitWidth)
                    {
                        delegate.ListView.view.implicitWidth = delegate.implicitWidth
                    }
                }
                id: delegate
                width: ListView.view.width
                text: model.display
                checked: model.enabled
                onClicked: {
                    imageItem.overlayGroupModel.swapEnabled(index);
                }
                onImplicitWidthChanged: updateImplicitWidth()
                Component.onCompleted: updateImplicitWidth()
            }
        }
    }
}
