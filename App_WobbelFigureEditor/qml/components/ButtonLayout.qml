import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0

ToolBar {
    property var figureView: null
    property var figureEditor: null
    property var figureAnalyzer: null
    property var productModel: null
    property var openFileDialog: null
    property var importFileDialog: null
    property var importDxfDialog: null
    property var figureFreeDialog: null
    property var figurePropertiesDialog: null
    property var figurePropertieTableDialog: null
    property var powerRampDialog: null
    property var requestChangesDialog: null
    property var requestChangesManager: null
    property var simulationController: null
    property var figurePropertieInformationControl: null
    property var screenshotTool: null
    property alias showHeatMap: showHeatMapLegend.checked
    property var commandManager: null
    property alias laserPowerController: laserPowerController
    property alias powerRampModel: powerRampModel

    signal newFigure
    signal centerOnOrigin
    signal open
    signal redrawGrid

    id: functionsButtonsInterface
    spacing: 6

    RowLayout {
        anchors.fill: parent
        Layout.alignment: Qt.AlignLeft

        ToolButton {
            id: clearCurrentFigure
            enabled: figureEditor && figureEditor.fileModel
            objectName: "figureEditor-buttonLayout-document-new"
            display: AbstractButton.IconOnly
            icon.name: "document-new"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                requestChangesManager.createNewFigure()
            }
            ToolTip {
                parent: parent
                text: qsTr("Create new document") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            objectName: "figureEditor-buttonLayout-document-open"
            display: AbstractButton.IconOnly
            icon.name: "document-open"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                requestChangesManager.openNewFigure()
            }
            ToolTip {
                parent: parent
                text: qsTr("Open document") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            enabled: FigureEditorSettings.fileType != FileType.Basic
                     && !simulationController.simulationMode
            objectName: "figureEditor-buttonLayout-create-figure"
            display: AbstractButton.IconOnly
            icon.name: "draw-freehand"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                figureEditor.figureCreator.fileType = figureEditor.fileType
                figureFreeDialog.opened ? figureFreeDialog.close(
                                              ) : figureFreeDialog.open()
                figurePropertieTableDialog.visible = false
            }
            ToolTip {
                parent: parent
                text: qsTr("Draw figure") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            objectName: "figureEditor-buttonLayout-document-edit"
            display: AbstractButton.IconOnly
            enabled: figureView && !simulationController.simulationMode
            icon.name: "document-edit"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                figurePropertiesDialog.open()
            }
        }
        ToolButton {
            id: saveButton
            objectName: "figureEditor-buttonLayout-save"
            enabled: figureEditor && figureEditor.fileModel
                     && !figureEditor.fileModel.loading
                     && FigureEditorSettings.fileType != FileType.Basic
                     && !simulationController.simulationMode
            display: AbstractButton.IconOnly
            icon.name: "document-save"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                var newSaveAsDialog = saveAsFigureDialogComponent.createObject(
                            applicationWindow, {
                                "fileEditor": figureEditor,
                                "requestChangesManager": requestChangesManager,
                                "saveAs": false
                            })
                figurePropertieTableDialog.visible = false
                newSaveAsDialog.open()
            }
            ToolTip {
                parent: parent
                text: qsTr("Save current document") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: saveAsButton
            objectName: "figureEditor-buttonLayout-saveAs"
            enabled: figureEditor && figureEditor.fileModel
                     && !figureEditor.fileModel.loading
                     && !simulationController.simulationMode
            opacity: enabled ? 1.0 : 0.5
            display: AbstractButton.IconOnly
            icon.name: "document-save-as"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked: {
                var newSaveAsDialog = saveAsFigureDialogComponent.createObject(
                            applicationWindow, {
                                "fileEditor": figureEditor,
                                "requestChangesManager": requestChangesManager,
                                "saveAs": true
                            })
                figurePropertieTableDialog.visible = false
                newSaveAsDialog.open()
            }
            ToolTip {
                parent: parent
                text: qsTr("Save current document as ...") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            objectName: "figureEditor-buttonLayout-document-import"
            display: AbstractButton.IconOnly
            icon.name: "document-import"
            icon.color: PrecitecApplication.Settings.alternateBackground
            enabled: RemovableDevices.Service.path != ""
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
              requestChangesManager.importNewFigure()
            }
        }
        ToolButton {
            id: exportButton
            objectName: "figureEditor-buttonLayout-export"
            icon.name: "document-export"
            display: AbstractButton.IconOnly
            icon.color: PrecitecApplication.Settings.alternateBackground
            enabled: figureEditor && figureEditor.fileModel
                     && !figureEditor.fileModel.loading
                     && (figureEditor.fileType == FileType.Seam || figureEditor.fileType == FileType.Wobble || figureEditor.fileType == FileType.Overlay)
                     && RemovableDevices.Service.path != ""
            opacity: enabled ? 1.0 : 0.5
            onClicked:
            {
              var outDir = RemovableDevices.Service.path
              var exportPath = figureEditor.exportFigure(outDir + figureEditor.fileModel.exportInfixForFigureType(figureEditor.fileType))

              if (exportPath)
              {
                //: notification that a figure got exported
                Notifications.NotificationSystem.information(qsTr("Figure exported to %1").arg(exportPath));
              }
              else
              {
                //: message that figure export failed
                Notifications.NotificationSystem.error(qsTr("Export failed"));
              }
            }
            ToolTip {
                parent: parent
                text: qsTr("Export figure") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }

        Dialog {
            id: deleteDialog
            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true
            title: qsTr("Delete figure?")
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape

            onAccepted: {
                figureEditor.deleteFigure()
                requestChangesManager.createNewFigure()
                close()
            }
            onRejected: close()
            Label {
                text: qsTr("Do you really want to delete this figure? Products that use it will malfunction.")
            }
        }

        ToolButton {
            id: deleteFile
            objectName: "figureEditor-buttonLayout-delete"
            enabled: figureEditor && figureEditor.canDeleteFigure
                     && !simulationController.simulationMode
            opacity: enabled ? 1.0 : 0.5
            display: AbstractButton.IconOnly
            icon.name: "edit-delete"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked: deleteDialog.open()
            ToolTip {
                parent: parent
                text: qsTr("Delete current document") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }

        ToolSeparator {}
        ToolButton {
            id: undo
            objectName: "edit-undo"
            enabled: commandManager && commandManager.undoIsPossible
                     && !simulationController.simulationMode
            opacity: enabled ? 1.0 : 0.5
            display: AbstractButton.IconOnly
            icon.name: "edit-undo"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked: {
                commandManager.undo()
            }
            ToolTip {
                parent: parent
                text: qsTr("Undo last action") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: redo
            objectName: "edit-redo"
            enabled: commandManager && commandManager.redoIsPossible
                     && !simulationController.simulationMode
            opacity: enabled ? 1.0 : 0.5
            display: AbstractButton.IconOnly
            icon.name: "edit-redo"
            icon.color: PrecitecApplication.Settings.alternateBackground
            onClicked: {
                commandManager.redo()
            }
            ToolTip {
                parent: parent
                text: qsTr("Redo last action") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolSeparator {}

        ToolButton {
            id: linkProductAndSeam
            objectName: "figureEditor-buttonLayout-importDataFromProduct"
            display: AbstractButton.IconOnly
            visible: functionsButtonsInterface.productModel
            enabled: !simulationController.simulationMode
            icon.name: "select-product"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                var dialog = productAndSeamDialog.createObject(Overlay.overlay)
                dialog.open()
            }
        }
        ToolButton {
            id: showHeatMapLegend
            enabled: FigureEditorSettings.fileType != FileType.Basic
                     && !simulationController.simulationMode
            objectName: "figureEditor-buttonLayout-show-heat-map-legend"
            display: AbstractButton.IconOnly
            icon.name: "view-help"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            checkable: true
            checked: true
            ToolTip {
                parent: parent
                text: qsTr("Display heatmap") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: zoomIn
            objectName: "figureEditor-buttonLayout-zoom-in"
            Layout.alignment: Qt.AlignVCenter
            display: AbstractButton.IconOnly
            enabled: figureEditor
            icon.name: "zoom-in"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                if (figureEditor.figureScale * 10 > 1000) {
                    return
                }
                figureEditor.figureScale = figureEditor.figureScale * 10
                FigureEditorSettings.increaseScaleByScaleFactor()
            }
            ToolTip {
                parent: parent
                text: qsTr("Zoom in") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: zoomOut
            objectName: "figureEditor-buttonLayout-zoom-out"
            display: AbstractButton.IconOnly
            enabled: figureEditor
            icon.name: "zoom-out"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                if (figureEditor.figureScale * 0.1 < 10) {
                    return
                }
                figureEditor.figureScale = figureEditor.figureScale / 10
                FigureEditorSettings.decreaseScaleByScaleFactor()
            }
            ToolTip {
                parent: parent
                text: qsTr("Zoom out") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: coordinateSystem
            objectName: "figureEditor-buttonLayout-show-coordinate-system"
            icon.name: "view+y"
            enabled: !simulationController.simulationMode
            opacity: enabled ? 1.0 : 0.5
            checkable: true
            checked: true
            onToggled: {
                figureView.grid.showCoordinateSystem = coordinateSystem.checked
                functionsButtonsInterface.redrawGrid()
            }
            ToolTip {
                parent: parent
                text: qsTr("Display coordinate system") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: scanfield
            objectName: "figureEditor-buttonLayout-show-scanfield"
            icon.name: "draw-ellipse"
            enabled: !simulationController.simulationMode
            opacity: enabled ? 1.0 : 0.5
            checkable: true
            checked: false
            onToggled: {
                figureView.grid.showScanField = scanfield.checked
                functionsButtonsInterface.redrawGrid()
            }
            ToolTip {
                parent: parent
                text: qsTr("Display scanfield") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: showRampInSeamFigure
            enabled: FigureEditorSettings.fileType == FileType.Seam
                     && !simulationController.simulationMode
            opacity: enabled ? 1.0 : 0.5
            objectName: "figureEditor-buttonLayout-show-ramps"
            icon.name: "power-ramp-visible"
            checkable: true
            checked: true
            onToggled: {
                simulationController.laserPointController.visualizeRamps
                        = showRampInSeamFigure.checked
            }
            ToolTip {
                parent: parent
                text: qsTr(
                          "Display power ramps in figure") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: showPowerGraph
            enabled: functionsButtonsInterface.figureEditor.fileType == FileType.Seam
                     && !functionsButtonsInterface.simulationController.simulationMode
                     && !simulationController.simulationMode
            objectName: "figureEditor-buttonLayout-show-power"
            icon.name: "laserpower"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                var powerInfo = showPower.createObject(Overlay.overlay)
                powerInfo.open()
            }
            ToolTip {
                parent: parent
                text: qsTr("Display power graph") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: mirrorY
            objectName: "figureEditor-buttonLayout-mirror-y"
            display: AbstractButton.IconOnly
            enabled: figureEditor
                     && FigureEditorSettings.fileType == FileType.Seam
                     && !simulationController.simulationMode
            icon.name: "view-y"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                figureEditor.mirrorYPosition()
            }
            ToolTip {
                parent: parent
                text: qsTr("Mirror figure on y-axis") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: rampButton
            objectName: "figureEditor-buttonLayout-power-ramp"
            display: AbstractButton.IconOnly
            enabled: figureEditor
                     && FigureEditorSettings.fileType == FileType.Seam
                     && figureEditor.numberOfPoints >= 2
                     && !simulationController.simulationMode
            icon.name: "menu-icon_powertec"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            checked: functionsButtonsInterface.powerRampDialog.visible
            onClicked: {
                if (null != functionsButtonsInterface.powerRampDialog) {
                    if (functionsButtonsInterface.powerRampDialog.visible) {
                        functionsButtonsInterface.powerRampDialog.visible = false
                    } else {
                        functionsButtonsInterface.powerRampDialog.visible = true
                    }
                }
            }
            ToolTip {
                parent: parent
                text: qsTr("Power ramp settings for global figure") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: centerOnOrigin
            objectName: "figureEditor-buttonLayout-show-origin"
            display: AbstractButton.IconOnly
            enabled: figureView
            icon.name: "zoom-original"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                figureView.zoom = 1.
                figureView.containerItem.x = functionsButtonsInterface.parent.width * 0.5
                figureView.containerItem.y = functionsButtonsInterface.parent.height * 0.5
                figureView.containerItemModified()
                functionsButtonsInterface.redrawGrid()
            }
            ToolTip {
                parent: parent
                text: qsTr("Center view on origin and zoom to 1.0") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolSeparator {}
        ToolButton {
            id: figureProperieTable
            display: AbstractButton.IconOnly
            icon.name: "table"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            enabled: figureAnalyzer && figureAnalyzer.pointCount > 0
                     && FigureEditorSettings.fileType != FileType.Basic
                     && !simulationController.simulationMode
            checked: functionsButtonsInterface.figurePropertieTableDialog.visible
            onClicked: {
                if (null != functionsButtonsInterface.figurePropertieTableDialog) {
                    if (functionsButtonsInterface.figurePropertieTableDialog.visible) {
                        functionsButtonsInterface.figurePropertieTableDialog.visible = false
                    } else {
                        functionsButtonsInterface.figurePropertieTableDialog.visible = true
                    }
                }
            }
            ToolTip {
                parent: parent
                text: qsTr("Display property table of all points") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: figureProperieInformation
            display: AbstractButton.IconOnly
            enabled: figureView
                     && FigureEditorSettings.fileType != FileType.Basic
            icon.name: "software_details"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                figurePropertieInformationControl.visible
                        = !figurePropertieInformationControl.visible
                checked = figurePropertieInformationControl.visible
            }
            ToolTip {
                parent: parent
                text: qsTr("Display figure information") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolButton {
            id: figureSimulation
            display: AbstractButton.IconOnly
            enabled: figureView
                     && FigureEditorSettings.fileType != FileType.Basic
                     && !simulationController.simulationMode
            icon.name: "view-video"
            icon.color: PrecitecApplication.Settings.alternateBackground
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                requestChangesManager.startSimulation()
            }
            ToolTip {
                parent: parent
                text: qsTr("Open simulation") //: This is a tooltip of a Toolbar button
                visible: parent.hovered
                delay: 350
            }
        }
        ToolSeparator {}

        Slider {
            id: zoom
            enabled: figureView
            from: 0.1
            value: figureView ? figureView.zoom : 0.0
            to: 6.0
            stepSize: figureView ? figureView.zoomIncrement : 0.0
            snapMode: Slider.SnapAlways
            live: true

            background: Rectangle {
                x: zoom.leftPadding
                y: zoom.topPadding + zoom.availableHeight / 2 - height / 2
                implicitWidth: 200
                implicitHeight: 4
                width: zoom.availableWidth
                height: implicitHeight
                radius: 2
                color: "lightgrey"

                Rectangle {
                    width: zoom.visualPosition * parent.width
                    height: parent.height
                    color: "darkgrey"
                    radius: 2
                }
            }

            onValueChanged: {
                figureView.zoom = zoom.value
            }
            ToolTip {
                parent: parent
                text: qsTr("Zoom in or out") //: This is a tooltip of a Toolbar button
                visible: zoom.hovered
                delay: 350
            }
        }

        Label {
            id: showZoom
            text: figureView ? qsTr("Zoom: %1").arg(
                                   figureView.zoom.toLocaleString(locale,
                                                                  "f", 2)) : ""

            MouseArea {
                anchors.fill: parent
                enabled: figureView
                onClicked: {
                    var zoomPropertiesDialog = zoomProperties.createObject(
                                functionsButtonsInterface.parent, {
                                    "figureView": figureView
                                })
                    zoomPropertiesDialog.open()
                }
            }
        }
        Item {
            Layout.fillWidth: true
        }
    }

    LaserPowerController {
        id: laserPowerController
        figureEditor: functionsButtonsInterface.figureEditor
        powerRampModel: powerRampModel
    }

    PowerRampModel {
        id: powerRampModel
    }

    Component {
        id: zoomProperties
        ZoomConfiguration {
            anchors.centerIn: parent
            width: parent.width * 0.4
            height: parent.height * 0.45
            screenshotTool: functionsButtonsInterface.screenshotTool
        }
    }

    Component {
        id: productAndSeamDialog
        ProductAndSeamDialog {
            anchors.centerIn: Overlay.overlay
            width: parent.width * 0.4
            height: parent.height * 0.4
            productModel: functionsButtonsInterface.productModel
        }
    }

    Connections {
        target: functionsButtonsInterface

        function onCenterOnOrigin() {
            centerOnOriginTimer.start()
        }
    }

    Timer {
        id: centerOnOriginTimer
        interval: 1
        repeat: false
        running: false
        onTriggered: {
            centerOnOrigin.clicked()
        }
    }

    Component {
        id: saveAsFigureDialogComponent
        SaveAsFigureDialog {
            id: saveAsFigureDialog

            anchors.centerIn: parent
            width: parent.width * 0.3
            height: parent.height * 0.4
            screenshotTool: functionsButtonsInterface.screenshotTool
        }
    }

    // PowerGraph
    PlotHandler {
        id: powerPlotHandler
        figureEditor: functionsButtonsInterface.figureEditor
        figureAnalyzer: functionsButtonsInterface.figureAnalyzer
        type: PlotHandler.Power
    }

    Component {
        id: showPower
        ShowPlot {
            anchors.centerIn: parent
            width: parent.width * 0.5
            height: parent.height * 0.7
            handler: powerPlotHandler
            screenshotTool: functionsButtonsInterface.screenshotTool
        }
    }
}
