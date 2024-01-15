import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications

import wobbleFigureEditor.components 1.0

Dialog {
    id: simulationDialog
    property var fileModel: null
    property var simulationController: null
    property alias screenshotTool: screenshotHeader.screenshotTool
    property bool showWobbleFiles: false
    modal: true

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: qsTr("Simulation menu")
    }

    footer: DialogButtonBox {
        id: buttons
        standardButtons: Dialog.Close | Dialog.Ok

        Component.onCompleted: {
            buttons.standardButton(Dialog.Ok).enabled = false;
        }
    }

    onAccepted:
    {
        if (simulationDialog.simulationController)
        {
            simulationDialog.simulationController.showSimulatedFigure();
        }
        close()
    }
    onRejected:
    {
        simulationDialog.simulationController.simulationMode = false;
    }

    Connections {
        target: simulationDialog.simulationController
        function onReadyChanged() {
            buttons.standardButton(Dialog.Ok).enabled = simulationDialog.simulationController.ready;
        }
    }

    GroupBox {
        id: gridPropertiesView
        implicitWidth: parent.width
        implicitHeight: parent.height
        GridLayout {
            anchors.fill: parent
            anchors.margins: 10
            columns: 3

            ListView {
                id: fileListView
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 2
                clip: true
                model: simulationDialog.showWobbleFiles ? filesSortModelSimulation : filesSeamSortModelSimulation
                currentIndex: -1
                delegate: ItemDelegate {
                    enabled: !simulationDialog.fileModel.loading
                    width: ListView.view.width
                    id: fileSelectionButton
                    checkable: true
                    text: model.name
                    onClicked:
                    {
                        if (simulationController && simulationController.simulationMode)
                        {
                            simulationController.getFigureForSimulation(model.fileName, model.type);
                            if (simulationDialog.showWobbleFiles)
                            {
                                showSelectedWobble.selectedWobbleName = model.name;
                                return;
                            }
                            showSelectedSeam.selectedSeamName = model.name;
                        }
                        simulationDialog.showWobbleFiles = true;
                    }
                }

                section.property: "typeName"
                section.criteria: ViewSection.FullString
                section.delegate: sectionHeading
            }

            Rectangle {
                id: simulationInformation
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.columnSpan: 1

                border.color: "black"
                border.width: 1
                radius: 5

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 25
                    columns: 3

                    ToolButton {
                        id: backButton
                        enabled: simulationDialog.showWobbleFiles
                        Layout.fillHeight: true
                        Layout.preferredWidth: simulationInformation.width * 0.3
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        display: AbstractButton.IconOnly
                        icon.name: "arrow-left"
                        icon.color: backButton.enabled ? PrecitecApplication.Settings.alternateBackground : "grey"
                        background: Rectangle {
                            color: "lightgrey"
                        }
                        onClicked: {
                            simulationDialog.showWobbleFiles = false;
                        }
                    }

                    Label {
                        id: showSelectedSeamLabel
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        //: title of a label which shows the selected seam for the simulation.
                        text: qsTr("Selected seam:")
                        font.bold: true
                    }

                    Label {
                        property string selectedSeamName: ""
                        id: showSelectedSeam
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                        text: showSelectedSeam.selectedSeamName
                    }

                    Label {
                        id: showSelectedWobbleLabel
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        //: title of a label which shows the selected wobble figure for the simulation.
                        text: qsTr("Selected wobble:")
                        font.bold: true
                    }

                    Label {
                        property string selectedWobbleName: ""
                        id: showSelectedWobble
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                        text: showSelectedWobble.selectedWobbleName
                    }

                    Label {
                        id: showLoopCountLabel
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        //: title of a label which shows the current loop count value. This value can be set to show just a few parts of the whole simulation.
                        text: qsTr("Loop count:")
                        font.bold: true
                    }

                    TextField {
                        id: showLoopCount
                        enabled: simulationDialog.simulationController.loopCount != 0
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 1
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        text: Number(simulationDialog.simulationController.loopCount).toLocaleString(locale, 'f', 0);
                        palette.text: showLoopCount.acceptableInput ? "black" : "red"
                        validator: IntValidator {
                            bottom: 0
                            top: 1000
                        }
                        onEditingFinished:
                        {
                            simulationDialog.simulationController.loopCount = Number.fromLocaleString(locale, showLoopCount.text);
                        }
                        selectByMouse: true
                    }

                    CheckBox {
                        id: showWholeFigure
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 2
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        //: title of a check box to show the whole simulated figure instead of a number of periods of the wobble figure.
                        text: qsTr("Whole figure")
                        checked: simulationDialog.simulationController.loopCount == 0
                        onToggled: {
                            if (showWholeFigure.checked)
                            {
                                simulationDialog.simulationController.loopCount = 0;
                                return;
                            }
                            simulationDialog.simulationController.loopCount = 1;
                        }
                    }

                    Label {
                        id: show10usFactorLabel
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        //: title of a label which shows the factor of the 10us in which the simulation is divided. This value can be set to show just every factor point.
                        text: qsTr("10us factor [10us]:")
                        font.bold: true
                    }

                    TextField {
                        id: show10usFactor
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        text: Number(simulationDialog.simulationController.tenMicroSecondsFactor).toLocaleString(locale, 'f', 0);
                        palette.text: show10usFactor.acceptableInput ? "black" : "red"
                        validator: IntValidator {
                            bottom: 1
                            top: 1000
                        }
                        onEditingFinished:
                        {
                            simulationDialog.simulationController.tenMicroSecondsFactor = Number.fromLocaleString(locale, show10usFactor.text);
                        }
                        selectByMouse: true
                    }

                    Label {
                        id: showSimulationSpeedLabel
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        //: title of a label which shows the simulation speed which is used in the simulation.
                        text: qsTr("Simulation speed [mm/s]:")
                        font.bold: true
                    }

                    TextField {
                        id: simulationSpeed
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.columnSpan: 3
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        text: Number(FigureEditorSettings.scannerSpeed).toLocaleString(locale, 'f', 3);
                        palette.text: simulationSpeed.acceptableInput ? "black" : "red"
                        validator: DoubleValidator {
                            bottom: 0.001
                            top: 10000.0
                        }
                        onEditingFinished:
                        {
                            FigureEditorSettings.scannerSpeed = Number.fromLocaleString(locale, simulationSpeed.text);
                        }
                        selectByMouse: true
                    }

                    Label {
                        id: showPointCountSimulationFigure
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
                        //: title of a label which shows the number of points of the simulation figure.
                        text: qsTr("Current points:")
                        font.bold: true
                    }

                    Item {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }

                    Label {
                        id: showMaxPointCountSimulationFigure
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        //: title of a label which shows the number of points of the simulation figure.
                        text: qsTr("Max points:")
                        font.bold: true
                    }

                    Label {
                        id: pointCountSimulationFigure
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        text: Number(simulationDialog.simulationController.pointCountSimulationFigure).toLocaleString(locale, 'f', 0);
                        background: Rectangle {
                            color: simulationDialog.simulationController.pointCountSimulationFigure < simulationDialog.simulationController.maxPointCountForSimulation ? "transparent" : "red"
                        }
                    }

                    Label {
                        id: conditionForSimulationCanBeDone
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                        text: "<"
                        font.bold: true
                    }

                    Label {
                        id: maxPointCountSimulationFigure
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        text: Number(simulationDialog.simulationController.maxPointCountForSimulation).toLocaleString(locale, 'f', 0);
                    }
                }
            }
        }
    }

    FileSortModel {
        id: filesSeamSortModelSimulation
        sourceModel: simulationDialog.fileModel ? simulationDialog.fileModel : null
        fileType: FileType.Seam
    }

    FileSortModel {
        id: filesSortModelSimulation
        sourceModel: simulationDialog.fileModel ? simulationDialog.fileModel : null
        fileType: FileType.Wobble
    }

    Component {
        id: sectionHeading
        Label {
            text: section
            font.bold: true
            font.pixelSize: 16
        }
    }

    Component.onCompleted: {
        simulationDialog.simulationController.clear();
    }
}
