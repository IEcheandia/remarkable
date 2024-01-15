import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui 1.0
import Precitec.AppGui 1.0
import precitec.gui.components.image 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.userManagement 1.0


Dialog {
    id: infoBoxDialog
    property alias imageData: imageItem.imageData
    property var filterInstances: null
    property alias simulationController: playerControl.controller
    property alias screenshotTool: screenshotHeader.screenshotTool

    parent: Overlay.overlay
    x: parent ? 0.05 * parent.width : 1
    y: parent ? 0.1 * parent.height : 1
    width: parent ? 0.9 * parent.width : 1
    height: parent ? 0.8 * parent.height : 1

    Behavior on width { PropertyAnimation {} }

    signal play()
    signal pause()
    signal stop()
    signal nextSeam()
    signal previousSeam()
    signal next()
    signal previous()

    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            infoBoxDialog.close()
        }
    }

    onClosed: {
        stateController.state = 'expanded';
        infoBoxModel.clear();
        infoBoxFilterModel.clear();
    }

    Item {
        id: stateController
        state: "expanded"
        states: [
            State {
                name: "expanded"
                PropertyChanges {
                    target: infoBoxDialog;
                    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside;
                    width: 0.9 * parent.width;
                }
                PropertyChanges {
                    target: imagePanel;
                    visible: true
                }
                PropertyChanges {
                    target: panel;
                    Layout.maximumWidth: infoBoxDialog.width * 0.35;
                }
                PropertyChanges {
                    target: arrowButton
                    icon.name: "arrow-left"
                }
            },
            State {
                name: "collapsed"
                PropertyChanges {
                    target: infoBoxDialog;
                    closePolicy: Popup.CloseOnEscape;
                    width: 0.545 * parent.width
                }
                PropertyChanges {
                    target: imageItem;
                    visible: false
                }
                PropertyChanges {
                    target: panel;
                    Layout.preferredWidth: infoBoxDialog.width - 10;
                }
                PropertyChanges {
                    target: arrowButton
                    icon.name: "arrow-right"
                }
            }
        ]
    }

    InfoBoxModel {
        id: infoBoxModel
    }

    InfoBoxFilterModel {
        id: infoBoxFilterModel
        sourceModel: infoBoxModel
    }

    header: RowLayout {
        PrecitecApplication.DialogHeaderWithScreenshot {
            id: screenshotHeader
            Layout.fillWidth: true
        }
        ToolButton {
            id: arrowButton
            Layout.topMargin: 5
            Layout.rightMargin: 5
            onClicked: {
                if (stateController.state == 'expanded')
                {
                    stateController.state = 'collapsed';
                } else
                {
                    stateController.state = 'expanded';
                }
            }
        }
    }

    footer: RowLayout {
        Item {
            Layout.fillWidth: true
        }
        Button {
            Layout.bottomMargin: 5
            text: qsTr("Clear")
            onClicked: infoBoxFilterModel.clear();
        }
        Button {
            Layout.bottomMargin: 5
            Layout.rightMargin: 5
            text: qsTr("Close")
            onClicked: infoBoxDialog.close()
        }
    }

    contentItem: RowLayout {
        anchors {
            fill: parent
            margins: 5
            topMargin: implicitHeaderHeight + 5
            bottomMargin: implicitFooterHeight + 5
        }

        ColumnLayout {
            id: imagePanel
            ImageItem {
                id: imageItem
                Layout.fillWidth: true
                Layout.fillHeight: true

                onImageDataChanged: infoBoxModel.setImageData(imageItem.imageData)

                Item {
                    x: imageItem.paintedRect.x
                    y: imageItem.paintedRect.y
                    width: imageItem.paintedRect.width
                    height: imageItem.paintedRect.height

                    MouseArea {
                        anchors.fill: parent
                        enabled: false
                        propagateComposedEvents: true
                        onWheel: {
                            imageItem.zoom += (wheel.angleDelta.y / 120.0) * 0.1;
                        }
                    }
                    TapHandler {
                        grabPermissions: PointerHandler.CanTakeOverFromAnything
                        onSingleTapped: infoBoxFilterModel.addPosition(imageItem.mapFromPaintedImage(eventPoint.position))
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
                Component.onCompleted: {
                        imageItem.overlayGroupModel.sourceModel.prepareInfoboxOverlays();
                }
            }
            PrecitecApplication.PlayerControls {
                Layout.alignment: Qt.AlignHCenter
                id: playerControl
                visible: infoBoxDialog.simulationController ? true : false
                onPlay: infoBoxDialog.play()
                onPause: infoBoxDialog.pause()
                onStop: infoBoxDialog.stop()
                onNextSeam: infoBoxDialog.nextSeam()
                onPreviousSeam: infoBoxDialog.previousSeam()
                onNext: infoBoxDialog.next()
                onPrevious: infoBoxDialog.previous()
            }
        }

        Rectangle {
            id: panel
            Layout.preferredWidth: infoBoxDialog.width * 0.35
            Layout.fillHeight: true
            border.width: 2

            Behavior on width { PropertyAnimation {} }

            ColumnLayout {
                anchors{
                    fill: parent
                    margins: 2
                }

                TabBar {
                    id: bar
                    clip: true
                    Layout.fillWidth: true

                    onCountChanged: {
                        bar.setCurrentIndex(-1);
                        bar.setCurrentIndex(0);
                    }

                    Repeater {
                        model: infoBoxFilterModel

                        TabButton {
                            id: tabLabel
                            text: model.id + " " + model.type
                            font.bold: bar.currentIndex == index
                            width: implicitContentWidth + 24
                        }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 15
                    currentIndex: bar.currentIndex

                    Repeater {
                        id: repeater
                        model: infoBoxFilterModel

                        InfoBoxTab {
                            texts: model.texts
                            filterInstances: infoBoxDialog.filterInstances

                            ToolButton {
                                anchors {
                                    bottom: parent.bottom
                                    left: parent.left
                                }
                                icon.name: "arrow-left"
                                visible: index != 0
                                onClicked: bar.decrementCurrentIndex()
                            }
                            ToolButton {
                                anchors {
                                    bottom: parent.bottom
                                    right: parent.right
                                }
                                icon.name: "arrow-right"
                                visible: index != repeater.count - 1
                                onClicked: bar.incrementCurrentIndex()
                            }
                        }
                    }
                }

                CheckBox {
                    text: qsTr("Multiple Selection Enabled")
                    checked: infoBoxFilterModel.multipleSelection
                    onClicked: infoBoxFilterModel.multipleSelection = checked;
                }
            }
        }
    }
}
