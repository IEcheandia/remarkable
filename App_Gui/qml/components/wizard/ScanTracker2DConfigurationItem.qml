import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.plotter 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication

import wobbleFigureEditor.components 1.0 as WobbleFigureEditor

Item {
    id: scanTracker2DConfiguration
    property alias hardwareParametersModel: controller.model
    property alias title: hardwareParameters.title
    property alias filterKeys: hardwareParametersEditor.filterKeys

    ScanTracker2DHardwareParameterController {
        id: controller
        productModel: HardwareModule.productModel
        filterModel: hardwareParametersEditor.filterModel
    }

    WobbleFigureEditor.FileModel {
        id: rootFileModel

        Component.onCompleted: rootFileModel.loadFiles()
    }

    WobbleFigureEditor.FileSortModel {
        id: systemFigureModel
        sourceModel: rootFileModel
        scanMasterMode: false
        fileType: WobbleFigureEditor.FileType.Basic
    }

    BasicFigureSelectionModel {
        id: figureSelectionModel
        sourceModel: systemFigureModel
        selectedId: controller.mode == ScanTracker2DHardwareParameterController.BasicFigure ? controller.fileId : -1
        onSelectedIdChanged: {
            if (controller.mode != ScanTracker2DHardwareParameterController.BasicFigure)
            {
                return;
            }
            controller.fileId = selectedId;
        }
    }

    WobbleFigureEditor.PreviewDialog {
        id: previewDialog
        fileModel: rootFileModel
    }

    ButtonGroup {
        id: customOrSystemFigureSelection
    }

    ButtonGroup {
        id: systemFigureSelection
    }

    RowLayout {
        anchors.fill: parent

        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            RadioButton {
                text: qsTr("No figure specified")
                ButtonGroup.group: customOrSystemFigureSelection
                checked: controller.mode == ScanTracker2DHardwareParameterController.NotSet
                onToggled: function() { controller.mode = ScanTracker2DHardwareParameterController.NotSet; }
            }
            GroupBox {
                id: userDefinedWobbleFigureGroup
                title: qsTr("User defined Wobble Figure")
                label: RadioButton {
                    id: userDefinedWobbleFigure
                    text: userDefinedWobbleFigureGroup.title
                    checked: controller.mode == ScanTracker2DHardwareParameterController.CustomFigure
                    onToggled: function() { controller.mode = ScanTracker2DHardwareParameterController.CustomFigure; }
                    ButtonGroup.group: customOrSystemFigureSelection
                }
                WobbleFigureEditor.FigureSelector {
                    id: figureSelector
                    objectName: "scan-tracker-2d-configuration-wobble-file-selector"
                    fileModel: rootFileModel
                    fileType: WobbleFigureEditor.FileType.Wobble
                    previewDialog: previewDialog
                    enabled: customOrSystemFigureSelection.checkedButton == userDefinedWobbleFigure

                    onFileModelIndexSelected: {
                        if (controller.mode != ScanTracker2DHardwareParameterController.CustomFigure)
                        {
                            return;
                        }
                        controller.fileId = rootFileModel.data(modelIndex, Qt.UserRole + 3);
                    }

                    function selectFromId()
                    {
                        figureSelector.selectIndex(rootFileModel.indexForWobbleFigure(controller.fileId));
                    }

                    Connections {
                        target: controller
                        enabled: controller.mode == ScanTracker2DHardwareParameterController.CustomFigure
                        function onFileIdChanged() {
                            figureSelector.selectFromId();
                        }
                    }
                    Connections {
                        target: rootFileModel
                        enabled: controller.mode == ScanTracker2DHardwareParameterController.CustomFigure
                        function onLoadingChanged()
                        {
                            if (rootFileModel.loading)
                            {
                                return;
                            }
                            figureSelector.selectFromId();
                        }
                    }

                    anchors.fill: parent
                }
                Layout.fillWidth: true
            }
            GroupBox {
                id: customWobbleFigureGroup
                title: qsTr("System provided Wobble Figure")
                label: RadioButton {
                    id: customWobbleFigure
                    text: customWobbleFigureGroup.title
                    checked: controller.mode == ScanTracker2DHardwareParameterController.BasicFigure
                    onToggled: function() { controller.mode = ScanTracker2DHardwareParameterController.BasicFigure; }
                    ButtonGroup.group: customOrSystemFigureSelection
                }

                GridLayout {
                    id: systemFigureLayout
                    enabled: customOrSystemFigureSelection.checkedButton == customWobbleFigure
                    anchors.fill: parent
                    columns: 3

                    Repeater {
                        id: repeater
                        model: figureSelectionModel
                        ItemDelegate {
                            id: delegate
                            down: pressed || indicator.pressed
                            checkable: true
                            checked: model.checked
                            ButtonGroup.group: systemFigureSelection
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            contentItem: RowLayout {
                                RadioButton {
                                    id: indicator
                                    checked: delegate.checked
                                    down: pressed || delegate.pressed
                                    onClicked: figureSelectionModel.markAsChecked(index)
                                    Layout.alignment: Qt.AlignVCenter
                                }

                                WobbleFigureEditor.LaserPointController {
                                    id: laserPointController
                                    figure: figureEditor.actualFigure
                                    figureScale: 100
                                }

                                WobbleFigureEditor.PreviewController {
                                    id: previewController
                                    laserPointController: laserPointController
                                    fileModel: rootFileModel
                                }
                                WobbleFigureEditor.WobbleFigureView {
                                    id: figureEditor

                                    Component.onCompleted: {
                                        clickedConnection.target = view;
                                    }
                                    Connections {
                                        id: clickedConnection
                                        target: null
                                        function onClicked()
                                        {
                                            if (!model.checked)
                                            {
                                                figureSelectionModel.markAsChecked(index);
                                            }
                                        }
                                    }
                                    Layout.fillHeight: true
                                    Layout.fillWidth: true

                                    Timer {
                                        id: helperTimer
                                        interval: 100
                                        onTriggered: {
                                            figureEditor.view.containerItem.x = figureEditor.view.width * 0.5;
                                            figureEditor.view.containerItem.y = figureEditor.view.height * 0.5;
                                        }
                                    }
                                }
                                Component.onCompleted: {
                                    var modelIndex = systemFigureModel.mapToSource(systemFigureModel.index(index, 0));
                                    previewController.previewBasicFigure(modelIndex);
                                    figureEditor.view.grid.followsOrigin = false;
                                    figureEditor.view.grid.coordinateSystemScale = 0.5;
                                    figureEditor.view.grid.wobbleFigure = true;
                                    // delay centering till layout is built up
                                    helperTimer.start();
                                }
                            }
                        }
                    }
                }

                Layout.fillHeight: true
                Layout.fillWidth: true
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.alignment: Qt.AlignTop
            GroupBox {
                id: hardwareParameters
                Layout.fillHeight: true
                HardwareParametersEditor {
                    id: hardwareParametersEditor
                    anchors.fill: parent
                    model: controller.model
                }
            }
        }
    }
}
