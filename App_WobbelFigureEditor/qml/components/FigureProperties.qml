import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication

Item {
    id: figurePropertiesItem
    property var figureAnalyzer: null
    property var simulationController: null
    property var requestChangesManager: null
    property var screenshotTool: null
    height: grid.implicitHeight

    ColumnLayout {
        id: grid
        anchors.fill: parent
        anchors.margins: 10

        GroupBox {
            //:Title for a group box where the scanner property is shown.
            title: qsTr("Scanner properties")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent

                Label {
                    id: scannerSpeedLabel
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    //:Title for a label where the current simulation speed of the figure editor is shown.
                    text: qsTr("Simulation speed [mm/s]")
                }

                TextField {
                    id: scannerSpeed
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    text: Number(
                              FigureEditorSettings.scannerSpeed).toLocaleString(
                              locale, 'f', 3)
                    palette.text: scannerSpeed.acceptableInput ? "black" : "red"
                    validator: DoubleValidator {
                        bottom: 0.001
                        top: 10000.0
                    }
                    onEditingFinished: {
                        FigureEditorSettings.scannerSpeed = Number.fromLocaleString(
                                    locale, scannerSpeed.text)
                        figurePropertiesItem.figureAnalyzer.updateProperties()
                    }
                    selectByMouse: true
                }
            }
        }

        GroupBox {
            visible: false
            //:Title for a group box where the laser property is shown.
            title: qsTr("Laser properties")
            Layout.fillWidth: true

            ColumnLayout {
                anchors.fill: parent

                Label {
                    id: laserMaxPowerLabel
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    //:Title for a label where the maximum simulation power for the figure editor is shown.
                    text: qsTr("Maximum simulation power [W]")
                }

                TextField {
                    id: laserMaxPower
                    Layout.fillWidth: true
                    Layout.columnSpan: 2
                    text: Number(
                              FigureEditorSettings.laserMaxPower).toLocaleString(
                              locale, 'f', 3)
                    palette.text: laserMaxPower.acceptableInput ? "black" : "red"
                    validator: DoubleValidator {
                        bottom: 0
                        top: 100000
                    }
                    onEditingFinished: {
                        FigureEditorSettings.laserMaxPower = Number.fromLocaleString(
                                    locale, laserMaxPower.text)
                    }
                    //                     onTextChenaged:{
                    //                         FigureEditorSettings.laserMaxPower = Number.fromLocaleString(locale, laserMaxPower.text);
                    //                     }
                    selectByMouse: true
                }
            }
        }

        ToolButton {
            visible: figurePropertiesItem.simulationController
                     && figurePropertiesItem.simulationController.laserPointController
            Layout.preferredWidth: parent.width * 0.25
            Layout.preferredHeight: parent.width * 0.25
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.margins: 10
            Layout.rowSpan: 3
            display: AbstractButton.IconOnly
            icon {
                width: parent.width * 0.25
                height: parent.width * 0.25
                name: "view-video"
                color: PrecitecApplication.Settings.alternateBackground
            }
            onClicked: {
                requestChangesManager.startSimulation()
            }
        }

        Item {
            id: wildcard
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
