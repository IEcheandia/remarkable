import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0

import wobbleFigureEditor.components 1.0

Item {
    id: mainView
    anchors.fill: parent

    property var screenshotTool: screenshot
    property alias productModel: buttons.productModel

    onVisibleChanged: {
        if (!visible && sideDialog) {
            sideDialog.visible = false
        }
    }

    function forceRedrawingGrid() {
        if (!wobbleFigureView.view.grid) {
            return
        }
        wobbleFigureView.view.grid.visible = false
        wobbleFigureView.view.grid.visible = true
    }

    Connections {
        target: FigureEditorSettings
        function onFileTypeChanged() {
            if (!wobbleFigureView.view) {
                return
            }
            forceRedrawingGrid()
        }
    }

    // Should be traeted read-only, reports the currently visible dialog on the right side.
    property Item sideDialog:
          properties.visible ? properties
        : figurePropertieTableDialog.visible ? figurePropertieTableDialog
        : powerRampDialog.visible ? powerRampDialog
        : null

    property var visibleSideDialogWidth: sideDialog === null ? 0 : sideDialog.width

    Component.onCompleted: {
        buttons.centerOnOrigin()
    }

    PrecitecApplication.ScreenshotTool {
        id: screenshot
        maximumNumberOfScreenshots: GuiConfiguration.maximumNumberOfScreenshots
    }

    CommandManager {
        id: commandManager
        onExecutedUndo: {
            wobbleFigureEditor.figurePropertiesChanged()
            attributeController.selectionChanged()
        }
        onExecutedRedo: {
            wobbleFigureEditor.figurePropertiesChanged()
            attributeController.selectionChanged()
        }
        onStackCleared: wobbleFigureEditor.resetFigurePropertiesChange()
    }

    FileModel {
        id: fileModel

        fileType: wobbleFigureEditor.fileType

        Component.onCompleted: {
            //TODO load files only when it is request in dialog
            fileModel.loadFiles()
        }
    }

    Connections {
        target: fileModel
        function onLoadingChanged() {
            // TODO this property have to check by there self, not with connections in main
            if (!fileModel.loading) {
                fileDialog.enabled = true
                loadTarget.enabled = true
                return
            }
        }
    }

    Connections {
        id: loadTarget
        enabled: false

        target: fileModel
        function onFigureLoaded(type) {
            wobbleFigureEditor.showFigure(type)
            figurePropertieTableDialog.wobbleFigureDataModel.figureModelChanged(
                        )
            figurePropertieTableDialog.wobbleFigureDataModel.setFileType(type)
            buttons.powerRampModel.reset(wobbleFigureEditor.numberOfPoints)
        }
    }

    WobbleFigureEditor {
        id: wobbleFigureEditor
        figure: wobbleFigureView.actualFigure
        fileModel: fileModel
        figureCreator: figureCreator
        laserPointController: laserPointController
        commandManager: commandManager
        // TODO: workaround: check to find a simple solution to update the position of properties
        onNodePositionUpdated: {
            attributeController.emitSignalsToUpdatePointPosition()
        }
        onNodePropertiesUpdated: {
            attributeController.emitSignalsToUpdatePointPosition()
        }
    }

    FigureCreator {
        id: figureCreator
        fileModel: fileModel
    }

    FigureAnalyzer {
        id: figureAnalyzer
        figureEditor: wobbleFigureEditor
        simulationController: simulationController
    }

    Offset {
        id: offset
    }

    SelectionHandler {
        id: selectionHandler
        figure: wobbleFigureView.actualFigure
    }

    LaserPointController {
        id: laserPointController
        figure: wobbleFigureView.actualFigure
        figureScale: wobbleFigureEditor.figureScale

        onFigureScaleChanged: {
            forceRedrawingGrid()
        }
    }
    SimulationController {
        id: simulationController
        fileModel: fileModel
        laserPointController: laserPointController

        onSimulationModeChanged: {
            commandManager.clearStack()
            fileDialog.createNew()
            if (simulationMode) {
                figurePropertieTableDialog.visible = false
                simulationDialog.open()
            } else {
                if (simulationDialog.opened) {
                    simulationDialog.close()
                }
            }
        }
    }
    ButtonLayout {
        id: buttons
        Layout.rightMargin: 0
        contentWidth: parent.width

        figureView: wobbleFigureView.view
        figureEditor: wobbleFigureEditor
        figureAnalyzer: figureAnalyzer
        screenshotTool: mainView.screenshotTool
        openFileDialog: fileDialog
        importFileDialog: importFileDialog
        figureFreeDialog: figureFreeDialog
        figurePropertiesDialog: figurePropertiesDialog
        figurePropertieTableDialog: figurePropertieTableDialog
        powerRampDialog: powerRampDialog
        requestChangesDialog: requestChangesDialog
        requestChangesManager: requestChangesManager
        simulationController: simulationController
        figurePropertieInformationControl: figurePropertieInformation
        commandManager: commandManager
        // Todo: find another name for signal -> cause newFigure can also be load a figure
        onNewFigure: {
            figurePropertieTableDialog.visible = false
            simulationController.simulationMode = false
            powerRampDialog.visible = false
            wobbleFigureEditor.newFigure()
            fileModel.reset()
            fileDialog.createNew()
            powerRampDialog.powerRampModel.reset(0)
        }
        onRedrawGrid: {
            forceRedrawingGrid()
        }
    }

    RequestChangesManager {
        id: requestChangesManager

        onHaveToAskAboutSavingFigure: {
            if (commandManager.undoIsPossible) {
                requestChangesDialog.open()
            } else {
                startAction()
            }
        }
        onCreateNewFigureAllowed: {
            buttons.newFigure()
            resetAction()
        }
        onOpenNewFigureAllowed: {
            figurePropertieTableDialog.visible = false
            powerRampDialog.visible = false
            fileDialog.open()
        }
        onImportNewFigureAllowed: {
            figurePropertieTableDialog.visible = false
            importFileDialog.fileModel = ImportFileModelFactory.createImportFileModel(RemovableDevices.Service.path)
            importFileDialog.fileModel.loadFiles()
            importFileDialog.outFileModel = fileModel
            importFileDialog.open()
        }
        onSimulationStartAllowed: {
            if (null != simulationController) {
                commandManager.clearStack()
                if (simulationController.simulationMode) {
                    simulationController.simulationMode = false
                } else {
                    FigureEditorSettings.fileType = FileType.None
                    simulationController.simulationMode = true
                }
                resetAction()
            }
        }
        onResetedAction: wobbleFigureEditor.resetFigurePropertiesChange()
    }

    RequestChangesDialog {
        id: requestChangesDialog
        screenshotTool: mainView.screenshotTool
        requestChangesManager: requestChangesManager
        figureEditor: wobbleFigureEditor
        width: parent.width * 1 / 4
        height: parent.height * 1 / 5
        anchors.centerIn: parent
        offerSave: fileModel.filename.trim().length !== 0
    }

    FileLoaderDialog {
        id: fileDialog
        width: parent.width * 1 / 3
        height: parent.height * 2 / 3
        anchors.centerIn: parent

        fileModel: fileModel
        fileEditor: wobbleFigureEditor
        simulationController: simulationController
        commandManager: commandManager
        screenshotTool: mainView.screenshotTool
    }
    FileLoaderDialog {
        id: importFileDialog
        width: parent.width * 1 / 3
        height: parent.height * 2 / 3
        anchors.centerIn: parent

        fileModel: null // NOTE: Gets a fresh instance everytime this dialog gets opended!
        fileEditor: wobbleFigureEditor
        simulationController: simulationController
        commandManager: commandManager
        screenshotTool: mainView.screenshotTool
        dxfImportDialog: dxfImportDialog
    }
    DxfImportDialog {
        id: dxfImportDialog
        width: parent.width * .9
        height: parent.height * .9
        anchors.centerIn: parent
        figureEditor: wobbleFigureEditor
        screenshotTool: mainView.screenshotTool
    }
    FigureFreeDialog {
        id: figureFreeDialog
        width: parent.width * 1 / 3
        height: parent.height * 0.8
        anchors.centerIn: parent

        fileModel: fileModel
        fileEditor: wobbleFigureEditor
        thisScreenshotTool: mainView.screenshotTool
    }
    FigurePropertieTableDialog {
        id: figurePropertieTableDialog
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: buttons.height + wobbleFigureView.figureNameLabel.height
        thisScreenshotTool: mainView.screenshotTool
        actualFigure: wobbleFigureEditor.figure
        figureEditor: wobbleFigureEditor
        figurePropertiesDialog: properties
        graphView: wobbleFigureView.view
        visible: false
        onVisibleChanged: {
            if (visible) {
                properties.visible = false
                powerRampDialog.visible = false
            }
        }
    }
    PowerRampDialog {
        id: powerRampDialog
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.topMargin: buttons.height + wobbleFigureView.figureNameLabel.height
        thisScreenshotTool: mainView.screenshotTool
        actualFigure: wobbleFigureEditor.figure
        figureEditor: wobbleFigureEditor
        graphView: wobbleFigureView.view
        laserPowerController: buttons.laserPowerController
        laserPointController: laserPointController
        powerRampModel: buttons.powerRampModel
        visible: false
        onVisibleChanged: {
            if (visible) {
                properties.visible = false
                figurePropertieTableDialog.visible = false
            }
        }
    }
    FigurePropertiesDialog {
        id: figurePropertiesDialog
        anchors.centerIn: parent
        width: parent.width * 0.5
        height: parent.height * 0.75
        figureEditor: wobbleFigureEditor
        figureAnalyzer: figureAnalyzer
        screenshotTool: mainView.screenshotTool
    }

    AttributeController {
        id: attributeController

        figureScale: wobbleFigureEditor.figureScale
        figure: wobbleFigureEditor.figure
        selectionHandler: selectionHandler

        onSelectionChanged: {
            if (null != selectedPoint) {
                if (!figurePropertieTableDialog.visible) {
                    properties.visible = true
                }
                figurePropertieTableDialog.currentSelectionId = selectedPoint.ID
                attributeController.emitSignalsToUpdatePointPosition()
                attributeController.emitSignalsToUpdatePointProperties()
            } else {
                properties.visible = false
                figurePropertieTableDialog.currentSelectionId = -1
            }
        }
    }
    SimulationDialog {
        id: simulationDialog
        anchors.centerIn: parent
        width: parent.width * 0.5
        height: parent.height * 0.8
        fileModel: simulationController
                   && simulationController.fileModel ? simulationController.fileModel : null
        simulationController: simulationController
        screenshotTool: mainView.screenshotTool
    }
    //Right side of the GUI
    ObjectProperties {
        id: properties
        Layout.fillHeight: true
        anchors.top: buttons.bottom
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 2
        visible: false
        attributeController: attributeController
        wobbleFigureEditor: wobbleFigureEditor
        figureAnalyzer: figureAnalyzer
        simulationController: simulationController

        onVisibleChanged: {
            if (visible)
            {
                figurePropertiesDialog.visible = false
                powerRampDialog.visible = false
            }
        }
    }
    /**********************************************************************************************************************************************************************************************/
    //Left side of the GUI
    Item {
        id: contentItemOfWindow
        anchors.fill: parent
        anchors.topMargin: buttons.height
        /**********************************************************************************************************************************************************************************************/
        //Middle of the GUI to show the filterGraph
        WobbleFigureView {
            id: wobbleFigureView
            anchors.fill: parent

            figureEditor: wobbleFigureEditor
            selection: selectionHandler
            select: selectButton.checked
            screenshotTool: mainView.screenshotTool

            onActualObjectChanged: {
                attributeController.selection = actualObject
            }
            onActualFigureChanged: {
                attributeController.figure = actualFigure
            }
        }

        Control {
            id: figurePropertieInformation
            visible: false
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.rightMargin: visibleSideDialogWidth + 10
            anchors.topMargin: wobbleFigureView.figureNameLabel.height

            contentItem: ColumnLayout {

                FigurePropertieInformation {
                    visible: true
                    figureAnalyzer: figureAnalyzer
                    simulationController: simulationController
                    Layout.alignment: Qt.AlignRight
                }
                WobbleFigureLimitsInformation {
                    visible: FigureEditorSettings.fileType == FileType.Wobble
                             && !simulationController.simulationMode
                    width: parent.width

                    figureAnalyzer: figureAnalyzer
                }
            }
        }

        HeatMapLegend {
            visible: buttons.showHeatMap
                     && (wobbleFigureEditor.fileType == FileType.Seam || wobbleFigureEditor.fileType == FileType.Wobble)
            id: heatMapLegend

            anchors.bottom: selectButton.top
            anchors.right: parent.right
            anchors.rightMargin: visibleSideDialogWidth + 10
            anchors.bottomMargin: 10

            height: parent.height * 0.25
            width: parent.width * 0.04
            color: "transparent"
        }

        ToolButton {
            id: selectButton
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: visibleSideDialogWidth + 10
            anchors.bottomMargin: 10
            enabled: selectionHandler
            icon.name: "select"
            icon.color: selectButton.checked ? "black" : "darkgrey"
            flat: true
            checkable: true
            checked: false
            onToggled: {
                selectionHandler.resetLaserPoints()
            }
        }
        /**********************************************************************************************************************************************************************************************/
    }
}
