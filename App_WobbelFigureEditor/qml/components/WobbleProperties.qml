import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

import wobbleFigureEditor.components 1.0

Control {
    id: wobbleFigurePropertiesItem
    property var figureEditor: null
    property var figureAnalyzer: null
    height: grid.implicitHeight

    rightPadding: 10
    leftPadding: 10

    background: Rectangle {
        id: background
        border.color: "lightgrey"
        border.width: 1
        radius: 3
    }

    contentItem: GridLayout {
        id: grid
        columns: 3

        Label {
            id: frequencyRange
            Layout.fillWidth: true
            Layout.columnSpan: 3
            //: The customer should be able to narrow down the frequency range so that he can set his desired frequency more easily.
            text: qsTr("Available frequency range [Hz]")
            font.pixelSize: 18
            font.bold: true
        }

        TextField {
            id: lowestFrequency
            Layout.fillWidth: true
            Layout.columnSpan: 3
            text: Number(wobbleFigureModel.lowestFrequency).toLocaleString(locale, 'f', 0)
            selectByMouse: true
            palette.text: lowestFrequency.acceptableInput ? "black" : "red"
            validator: IntValidator {
                bottom: WobbleFigureModel.LowestFrequency
                top: WobbleFigureModel.HighestFrequency
            }
            onEditingFinished: {
                wobbleFigureModel.lowestFrequency = Number.fromLocaleString(locale, lowestFrequency.text)
            }
        }

        TextField {
            id: highestFrequency
            Layout.fillWidth: true
            Layout.columnSpan: 3
            text: Number(wobbleFigureModel.highestFrequency).toLocaleString(locale, 'f', 0)
            selectByMouse: true
            palette.text: highestFrequency.acceptableInput ? "black" : "red"
            validator: IntValidator {
                bottom: WobbleFigureModel.LowestFrequency
                top: WobbleFigureModel.HighestFrequency
            }
            onEditingFinished: {
                wobbleFigureModel.highestFrequency = Number.fromLocaleString(locale, highestFrequency.text);
            }
        }

        Label {
            id: wobbleFigureFrequencyLabel
            Layout.fillWidth: true
            Layout.columnSpan: 3
            //: Title for combo box
            text: qsTr("Frequency [Hz]")
            font.pixelSize: 18
            font.bold: true
        }

        ComboBox {
            id: wobbleFigureFrequency
            Layout.fillWidth: true
            Layout.columnSpan: 3
            currentIndex: wobbleFigureModel.frequencyIndex.row
            model: wobbleFigureModel
            textRole: "frequency"
            onActivated:
            {
                wobbleFigurePropertiesItem.figureEditor.microVectorFactor = wobbleFigureModel.data(wobbleFigureModel.index(currentIndex, 0), Qt.UserRole);
            }
        }

        Label {
            id: wobblePowerModulationModeLabel
            visible: FigureEditorSettings.dualChannelLaser
            Layout.fillWidth: true
            Layout.columnSpan: 3
            //: Title for group button to switch between different power modulation modes.
            text: qsTr("Power modulation mode")
            font.pixelSize: 18
            font.bold: true
        }

        RadioButton {
            id: powerModulationModeIsCore
            visible: FigureEditorSettings.dualChannelLaser
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            //: Title for radio button to switch to power modulation mode core. Core mode applies power modulation to core component of laser.
            text: qsTr("Core")
            checked: wobbleFigurePropertiesItem.figureEditor ? wobbleFigurePropertiesItem.figureEditor.powerModulationMode === PowerModulationMode.Core : false

            ButtonGroup.group: powerModulationModeGroup

            onClicked: {
                    wobbleFigurePropertiesItem.figureEditor.powerModulationMode = PowerModulationMode.Core
                    wobbleFigureModel.isDualChannelModulation = false;
            }
        }

        RadioButton {
            id: powerModulationModeIsRing
            visible: FigureEditorSettings.dualChannelLaser
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            //: Title for radio button to switch to power modulation mode ring. Ring mode applies power modulation to ring component of laser.
            text: qsTr("Ring")
            checked: wobbleFigurePropertiesItem.figureEditor ? wobbleFigurePropertiesItem.figureEditor.powerModulationMode === PowerModulationMode.Ring : false

            ButtonGroup.group: powerModulationModeGroup

            onClicked: {
                    wobbleFigurePropertiesItem.figureEditor.powerModulationMode = PowerModulationMode.Ring
                    wobbleFigureModel.isDualChannelModulation = false;
            }
        }

        RadioButton {
            id: powerModulationModeIsCoreAndRing
            visible: FigureEditorSettings.dualChannelLaser
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            //: Title for radio button to switch to power modulation mode core and ring. Core and ring mode applies power modulation to core and ring component of laser.
            text: qsTr("Core and ring")
            checked: wobbleFigurePropertiesItem.figureEditor ? wobbleFigurePropertiesItem.figureEditor.powerModulationMode === PowerModulationMode.CoreAndRing : false

            ButtonGroup.group: powerModulationModeGroup

            onClicked: {
                    wobbleFigurePropertiesItem.figureEditor.powerModulationMode = PowerModulationMode.CoreAndRing
                    wobbleFigureModel.isDualChannelModulation = true;
            }
        }

        ButtonGroup {
            id: powerModulationModeGroup
        }

        Item {
            id: wildcard
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    WobbleFigureModel {
        id: wobbleFigureModel

        pointCount: wobbleFigurePropertiesItem.figureEditor.numberOfPoints

        microVectorFactor: wobbleFigurePropertiesItem.figureEditor.microVectorFactor

        onMicroVectorFactorWillBeChanged: {
            wobbleFigurePropertiesItem.figureEditor.microVectorFactor = wobbleFigureModel.microVectorFactor;
        }
    }
}

