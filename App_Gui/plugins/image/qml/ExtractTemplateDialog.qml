import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5
import QtQuick.Shapes 1.0

import precitec.gui.components.image 1.0
import precitec.gui.components.application 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.general 1.0

Dialog {
    id: extractTemplateDialog
    //: title of a dialog to extract an image template
    title: qsTr("Extract Template")
    property alias imageData: imageItem.imageData
    property alias paintedRect: imageItem.paintedRect
    property alias imageSize: imageItem.imageSize
    property alias zoom: imageItem.zoom
    property alias screenshotTool: screenshotHeader.screenshotTool

    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            extractTemplateDialog.reject()
        }
    }

    parent: Overlay.overlay
    anchors.centerIn: parent
    implicitWidth: 0.9 * parent.width
    implicitHeight: 0.8 * parent.height
    standardButtons: Dialog.Cancel | Dialog.Save
    onAccepted: {
        imageItem.saveSubImage(imageItem.selection, WeldmasterPaths.configurationDir + fileName.text + ".bmp")
    }

    header: DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: extractTemplateDialog.title
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

    modal: true

    Component.onCompleted: {
        standardButton(Dialog.Save).enabled = Qt.binding(function() { return fileName.acceptableInput && imageItem.selection.width > 0 && imageItem.selection.height > 0; });
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
                property rect selection: Qt.rect(Math.min(pointASelector.selectedPoint.x, pointBSelector.selectedPoint.x), Math.min(pointASelector.selectedPoint.y, pointBSelector.selectedPoint.y), Math.abs(pointBSelector.selectedPoint.x - pointASelector.selectedPoint.x), Math.abs(pointBSelector.selectedPoint.y - pointASelector.selectedPoint.y))
                Layout.fillWidth: true
                Layout.fillHeight: true
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
                        target: null
                        minimumPointCount: 1
                        maximumPointCount: 1
                        grabPermissions: PointerHandler.CanTakeOverFromAnything
                        onActiveChanged: {
                            if (active)
                            {
                                if (mouseSelectionButton.checked)
                                {
                                    pointASelector.xPosition = centroid.position.x;
                                    pointASelector.yPosition = centroid.position.y;
                                    pointBSelector.xPosition = centroid.position.x;
                                    pointBSelector.yPosition = centroid.position.y;
                                }
                                else
                                {
                                    startPanning = imageItem.panning;
                                }
                                if (ApplicationWindow.activeFocusControl)
                                {
                                    ApplicationWindow.activeFocusControl.focus = false;
                                }
                            }
                        }
                        onTranslationChanged: {
                            if (mouseSelectionButton.checked)
                            {
                                pointBSelector.xPosition = pointASelector.xPosition + translation.x;
                                pointBSelector.yPosition = pointASelector.yPosition + translation.y;
                            }
                            else
                            {
                                imageItem.panning.x = startPanning.x + translation.x;
                                imageItem.panning.y = startPanning.y + translation.y;
                            }
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
                }
                PointSelector {
                    id: pointBSelector
                    image: imageItem
                    enabled: pointBButton.checked
                }
                PointSelector {
                    id: centerMarker
                    image: imageItem
                    enabled: false
                    visible: centerButton.checked
                    function updateFromOthers()
                    {
                        centerMarker.xPosition = pointASelector.xPosition + (pointBSelector.xPosition - pointASelector.xPosition) * 0.5;
                        centerMarker.yPosition = pointASelector.yPosition + (pointBSelector.yPosition - pointASelector.yPosition) * 0.5;
                    }
                    Connections {
                        target: pointASelector
                        function onXPositionChanged()
                        {
                            centerMarker.updateFromOthers();
                        }
                        function onYPositionChanged()
                        {
                            centerMarker.updateFromOthers();
                        }
                    }
                    Connections {
                        target: pointBSelector
                        function onXPositionChanged()
                        {
                            centerMarker.updateFromOthers();
                        }
                        function onYPositionChanged()
                        {
                            centerMarker.updateFromOthers();
                        }
                    }
                }
            }
        }

        ColumnLayout {
            ButtonGroup {
                id: pointSelectionGroup
            }
            RowLayout {
                Label {
                    //: title of a label for a TextField to specify a file name
                    text: qsTr("File:")
                }
                TextField {
                    id: fileName
                    objectName: "extract-template-dialog-file-name"
                    //: placeholder text on a TextField to specify a file name
                    placeholderText: qsTr("name without extention")
                    validator: FileSystemNameValidator {}
                    palette.text: acceptableInput ? undefined : "red"
                    Layout.fillWidth: true
                }
                Label {
                    text: ".bmp"
                }
            }
            Button {
                id: mouseSelectionButton
                objectName: "extract-template-dialog-mouse-selection-button"
                checkable: true
                icon.name: "select-rectangular"
                //: title of a button
                text: qsTr("Select using mouse")
                Layout.fillWidth: true
            }
            Button {
                id: pointAButton
                objectName: "extract-template-dialog-set-point-a"
                checkable: true
                icon.name: "measure"
                //: title of a button
                text: qsTr("Set 1. point")
                ButtonGroup.group: pointSelectionGroup
                Layout.fillWidth: true
            }
            Button {
                id: pointBButton
                objectName: "extract-template-dialog-set-point-b"
                checkable: true
                icon.name: "measure"
                //: title of a button
                text: qsTr("Set 2. point")
                ButtonGroup.group: pointSelectionGroup
                Layout.fillWidth: true
            }
            Button {
                id: centerButton
                objectName: "extract-template-dialog-center-button"
                checkable: true
                icon.name: "crosshairs"
                //: title of a button
                text: qsTr("Highlight center position")
                Layout.fillWidth: true
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
            GroupBox {
                //: Title of a GroupBox with information about selection (x, y, width, height)
                title: qsTr("Selection")
                GridLayout {
                    columns: 2
                    Label {
                        //: Title of a label for the center position of selection
                        text: qsTr("Center:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        text: Math.round(imageItem.selection.x + imageItem.selection.width * 0.5) + ", " + Math.round(imageItem.selection.y + imageItem.selection.height * 0.5)
                    }
                    Label {
                        //: Title of a label for the top left position of selection
                        text: qsTr("Top left:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        text: Math.round(imageItem.selection.x) + ", " + Math.round(imageItem.selection.y)
                    }
                    Label {
                        //: Title of a label for the size of selection
                        text: qsTr("Size:")
                        Layout.alignment: Qt.AlignRight
                    }
                    Label {
                        text: Math.round(imageItem.selection.width) + " x " + Math.round(imageItem.selection.height)
                    }
                }
            }
        }
    }
}
