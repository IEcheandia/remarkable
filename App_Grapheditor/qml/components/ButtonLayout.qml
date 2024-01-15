import QtQuick 2.15
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0

ToolBar {
    property var graphView: null
    property var graphEditor: null
    property var nodesModel: null
    property var graphImageSaver: null
    property bool graphIsMacro: false
    property alias documentModified: saveButton.enabled
    property var groupController: null
    property var plausibilityController: null
    id: functionButtonsInterface
    spacing: 6

    signal newGraph()
    signal save()
    signal saveAs()
    signal open()
    signal reload()
    signal exportGraph()

    Component {
        id: exportMacroDialogComponent
        Dialog {
            id: exportMacroDialog
            property string macroName: ""
            property string comment: ""

            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true

            width: parent.width * 0.5
            height: parent.height * 0.5

            //: title of a dialog
            title: qsTr("Export group to macro")
            standardButtons: Dialog.Cancel | Dialog.Save

            onAccepted: {
                functionButtonsInterface.groupController.exportSelectedToMacro(WeldmasterPaths.userMacroDir, nameField.text, commentField.text);
                exportMacroDialog.destroy();
            }
            onRejected: exportMacroDialog.destroy()

            GridLayout {
                anchors.fill: parent
                columns: 2

                Label {
                    //: label for a text field
                    text: qsTr("Name:")
                    Layout.alignment: Qt.AlignRight
                }
                TextField {
                    id: nameField
                    text: exportMacroDialog.macroName
                    Layout.fillWidth: true
                }
                Label {
                    //: label for a text field
                    text: qsTr("Comment:")
                    Layout.alignment: Qt.AlignRight
                }
                TextField {
                    id: commentField
                    //: placeholder text shown in a text field
                    placeholderText: qsTr("Insert comment for new macro")
                    Layout.fillWidth: true
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.columnSpan: 2
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        ToolButton {
            objectName: "grapheditor-document-new"
            display: AbstractButton.IconOnly
            icon.name: "document-new"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: functionButtonsInterface.newGraph()
        }
        ToolButton {
            objectName: "grapheditor-document-open"
            display: AbstractButton.IconOnly
            icon.name: "document-open"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: functionButtonsInterface.open()
        }
        ToolButton {
            objectName: "grapheditor-document-edit"
            display: AbstractButton.IconOnly
            icon.name: "document-edit"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked:
            {
                var graphProperties = graphUserInterface.createObject(functionButtonsInterface.parent, {"filterGraph": graphView.graph, "graphVisualizer": graphEditor.graphModelVisualizer});
                graphProperties.open();
            }
        }
        ToolButton {
            id: saveButton
            objectName: "grapheditor-document-save"
            display: AbstractButton.IconOnly
            icon.name: "document-save"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: functionButtonsInterface.save()
        }
        ToolButton {
            objectName: "grapheditor-document-save-as"
            display: AbstractButton.IconOnly
            icon.name: "document-save-as"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked: functionButtonsInterface.saveAs()
        }
        ToolButton {
            id: exportButton
            objectName: "grapheditor-document-export"
            icon.name: "document-export"
            display: AbstractButton.IconOnly
            icon.color: PrecitecApplication.Settings.alternateBackground
            enabled: RemovableDevices.Service.path != ""
            opacity: enabled ? 1.0 : 0.5
            onClicked: functionButtonsInterface.exportGraph()
        }
        ToolButton {
            objectName: "grapheditor-document-reload"
            display: AbstractButton.IconOnly
            icon.name: "system-reboot"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked: functionButtonsInterface.reload()
        }
        ToolSeparator {
        }

        ToolButton {
            objectName: "grapheditor-extract-group-to-macro"
            icon.name: "run-build"
            visible: !functionButtonsInterface.graphIsMacro
            enabled: functionButtonsInterface.groupController.canBeExportedToMacro
            onClicked: {
                var model = graphView.graph.selectedGroupsModel;
                var index = model.index(0, 0);
                var dialog = exportMacroDialogComponent.createObject(functionButtonsInterface, {"macroName": model.data(index, Qt.UserRole + 1).label});
                dialog.open();
            }
        }

        ToolButton {
            objectName: "grapheditor-selection-mode"
            checkable: true
            icon.name: "select-rectangular"
            icon.color: PrecitecApplication.Settings.alternateBackground
            checked: !graphView.navigable
            onClicked: {
                graphView.navigable = !graphView.navigable;
            }
        }
        ToolButton {
            id: plausibilityCheckInfos
            enabled: graphEditor && plausibilityController && graphView
            property bool plausibility: false
            icon.name: "dialog-ok"
            icon.color: PrecitecApplication.Settings.alternateBackground

            onClicked: {
                plausibility = plausibilityController.plausibilityCheck();
                var idCheck = plausibilityController.checkIDs();
                var plausibilityCheck = plausibilityCheckDialog.createObject(functionButtonsInterface.parent, {"graphVisualizer": graphEditor.graphModelVisualizer, "navigable": graphView, "graphValid": plausibility, "idsValid": idCheck, "plausibilityController": functionButtonsInterface.plausibilityController});
                plausibilityCheck.open();
            }
        }
        ToolButton {
            id: searchObjectButton
            enabled: nodesModel && graphView
            icon.name: "menu-icon_pre-adjust"
            icon.color: PrecitecApplication.Settings.alternateBackground
            flat: true
            onClicked: {
                    var newSearchDialog = searchObjectInterface.createObject(functionButtonsInterface.parent, {"nodeModel": nodesModel, "navigable": graphView});
                    newSearchDialog.open();
            }
        }
        ToolButton {
            id: graphToImageButton
            enabled: graphImageSaver
            icon.name: "camera-photo"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked:
            {
                graphImageSaver.getDynamicImage();
                Notifications.NotificationSystem.information(qsTr("Image of graph saved in %1 with the name %2").arg(graphImageSaver.pathForImages).arg(graphImageSaver.imageName));
            }
        }

        ToolSeparator {
        }

        Button {
            flat: true
            text: graphView ? qsTr("%1 x").arg(Number(graphView.zoom).toLocaleString(locale, "f", 1)) : qsTr("Zoom: ?")
            enabled: graphView
            icon.name: "zoom"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked:
            {
                    var zoomPropertiesDialog = zoomProperties.createObject(functionButtonsInterface.parent, {"graphView": graphView});
                    zoomPropertiesDialog.open();
            }
        }

        Slider {
            from: graphView.zoomMin
            to: graphView.zoomMax
            stepSize: graphView.zoomIncrement
            value: graphView.zoom
            onMoved: {
                graphView.zoom = value;
            }
        }

        ToolButton {
            id: zoomOrigButton
            objectName: "grapheditor-zoom-original"
            icon.name: "zoom-original"
            icon.color: PrecitecApplication.Settings.alternateBackground
            display: AbstractButton.IconOnly
            onClicked: {
                graphView.zoom = 1.0;
            }
        }

        ToolButton {
            id: graphFitInView
            enabled: graphView
            objectName: "grapheditor-zoom-fit-best"
            icon.name: "zoom-fit-best"
            icon.color: PrecitecApplication.Settings.alternateBackground
            flat: true
            onClicked: {
                graphView.fitInView();
            }
        }

        Button {
            flat: true
            text: graphEditor && graphEditor.graphModelVisualizer ? qsTr("Grid size: %1").arg(graphEditor.graphModelVisualizer.gridSize.toString()) : qsTr("Grid size: ?")
            enabled: graphView && graphView.grid && graphEditor && graphEditor.graphModelVisualizer
            icon.name: "grid-rectangular"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked:
            {
                var gridPropertiesDialog = gridProperties.createObject(functionButtonsInterface.parent, {"lineGrid": graphView.grid, "graphVisualizer": graphEditor.graphModelVisualizer});
                gridPropertiesDialog.open();
            }
        }

        ToolSeparator {
        }

        ToolButton {
            objectName: "grapheditor-insert-object-new-inport"
            visible: !functionButtonsInterface.graphIsMacro
            Drag.active: dragHandlerInPort.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction
            Drag.hotSpot: Qt.point(-20, -20)

            indicator: Item {
                id: inputPortBody
                width: parent.availableWidth
                height: parent.availableHeight
                anchors.centerIn: parent

                Rectangle {
                    id: leftRect
                    anchors.left: inputPortBody.left
                    width: 0.1*inputPortBody.width
                    height: inputPortBody.height
                    color: PrecitecApplication.Settings.alternateBackground
                }
                Rectangle {
                    id: rightRect
                    anchors.left: leftRect.right
                    anchors.right: inputPortBody.right
                    height: inputPortBody.height
                    color: "transparent"

                    Canvas {
                        id: rightInsideRect
                        anchors.fill: parent
                        onPaint: {
                            var context = getContext("2d");
                            context.strokeStyle = PrecitecApplication.Settings.alternateBackground;
                            context.fillStyle = PrecitecApplication.Settings.alternateBackground;
                            context.fillRect(0, 0, rightRect.width/2., rightRect.height)
                            context.arc(0 + rightRect.width/2.,0 + rightRect.height/2.,rightRect.height/2.,1.5*Math.PI,0.5*Math.PI);
                            context.fill();
                            context.stroke();
                        }
                    }
                }
            }
            onClicked:  {
                // TODO: remove the generateNewId
                graphEditor.generateNewID();
                var centerPoint = filterGraphView.getVisibleCenter();
                graphEditor.insertNewPort("New Port", -1, 0, centerPoint.x, centerPoint.y);
            }
            DragHandler {
                id: dragHandlerInPort
                target: null
            }
        }
        ToolButton {
            objectName: "grapheditor-insert-object-new-outport"
            visible: !functionButtonsInterface.graphIsMacro
            Drag.active: dragHandlerOutPort.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction
            Drag.hotSpot: Qt.point(-20, -20)
            indicator: Item {
                id: outputPortBody
                width: parent.availableWidth
                height: parent.availableHeight
                anchors.centerIn: parent

                Rectangle {
                    id: leftRectOut
                    anchors.left: outputPortBody.left
                    width: outputPortBody.width
                    height: outputPortBody.height
                    color: PrecitecApplication.Settings.alternateBackground

                    Canvas {
                        id: leftInsideRectOut
                        anchors.fill: parent
                        onPaint: {
                            var context = getContext("2d");
                            context.arc(0,0 + leftRectOut.height/2.,leftRectOut.height/2.5,1.5*Math.PI,0.5*Math.PI);
                            context.strokeStyle = PrecitecApplication.Settings.alternateBackground;
                            context.fillStyle = "white";
                            context.fill();
                            context.stroke();
                        }
                    }
                }
                Rectangle {
                    id: rightRectOut
                    anchors.left: leftRectOut.right
                    anchors.right: outputPortBody.right
                    height: outputPortBody.height
                    color: PrecitecApplication.Settings.alternateBackground

                    Canvas {
                        id: rightInsideRectOut
                        visible: false
                        anchors.fill: parent
                        onPaint: {
                            var context = getContext("2d");
                            context.arc(0 + rightRectOut.width/2.,0 + rightRectOut.height/2.,rightRectOut.height/2.3,1.5*Math.PI,0.5*Math.PI);
                            context.strokeStyle = PrecitecApplication.Settings.alternateBackground;
                            context.fillStyle = PrecitecApplication.Settings.alternateBackground;
                            context.fill();
                            context.stroke();
                        }
                    }
                }
            }
            onClicked:  {
                // TODO: remove the generateNewId
                graphEditor.generateNewID();
                var centerPoint = filterGraphView.getVisibleCenter();
                graphEditor.insertNewPort("New Port", -1, 1, centerPoint.x, centerPoint.y);
            }
            DragHandler {
                id: dragHandlerOutPort
                target: null
            }
        }
        ToolButton {
            objectName: "grapheditor-insert-object-new-group"
            visible: !functionButtonsInterface.graphIsMacro
            icon.name: "object-group"
            icon.color: PrecitecApplication.Settings.alternateBackground
            Drag.active: dragHandlerGroup.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction
            Drag.hotSpot: Qt.point(-20, -20)
            onClicked: {
                graphEditor.insertNewGroup(filterGraphView.getVisibleCenter());
            }
            DragHandler {
                id: dragHandlerGroup
                target: null
            }
        }
        ToolButton {
            objectName: "grapheditor-insert-object-new-comment"
            icon.name: "draw-text"
            icon.color: PrecitecApplication.Settings.alternateBackground
            Drag.active: dragHandlerComment.active
            Drag.dragType: Drag.Automatic
            Drag.supportedActions: Qt.CopyAction
            Drag.hotSpot: Qt.point(-20, -20)
            onClicked:  {
                var centerPoint = filterGraphView.getVisibleCenter();
                graphEditor.insertNewComment("New Comment", -1, "", centerPoint.x, centerPoint.y, 100, 100);
            }
            DragHandler {
                id: dragHandlerComment
                target: null
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }

    ZoomConfiguration
    {
        id: zoomProperties
    }
    GraphProperties
    {
        id: graphUserInterface
    }
    GridProperties
    {
        id: gridProperties
    }

    PlausibilityCheck
    {
        id: plausibilityCheckDialog
    }
    SearchObjectInterface
    {
        id: searchObjectInterface
    }
}
