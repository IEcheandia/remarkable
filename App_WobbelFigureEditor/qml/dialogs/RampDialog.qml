import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications

import wobbleFigureEditor.components 1.0

Dialog {
    id: rampDialog

    property var figureEditor: null
    property var rampModel: null
    property var rampValidator: null
    property var attributeController: null
    property alias screenshotTool: screenshotHeader.screenshotTool

    parent: Overlay.overlay
    width: parent.width * 0.3
    height: parent.height * 0.6
    anchors.centerIn: parent
    modal: true

    footer: DialogButtonBox {
        ToolButton {
            id: closeButton
            objectName: "figureEditor-rampDialog-close-dialog"
            display: AbstractButton.TextUnderIcon
            icon.name: "window-close"
            icon.color: PrecitecApplication.Settings.alternateBackground
            //: Close button of a dialog
            text: qsTr("Close")
            onClicked: {
                attributeController.resetSelection()
                rampDialog.reject()
            }
        }
        ToolButton {
            id: deleteRamp
            objectName: "figureEditor-rampDialog-remove-ramp"
            display: AbstractButton.TextUnderIcon
            enabled: rampValidator.isPointAStartPoint
            icon.name: "edit-delete"
            icon.color: PrecitecApplication.Settings.alternateBackground
            //: Delete the current selected ramp
            text: qsTr("Delete")
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                rampModel.eraseRamp()
                attributeController.resetSelection()
                rampDialog.reject()
            }
        }
        ToolButton {
            id: saveRamp
            objectName: "figureEditor-rampDialog-save-update-ramp"
            display: AbstractButton.TextUnderIcon
            icon.name: "document-save"
            icon.color: PrecitecApplication.Settings.alternateBackground
            //: Save selected ramp or create new ramp.
            text: qsTr("Save")
            opacity: enabled ? 1.0 : 0.5
            onClicked: {
                rampModel.updateRamps()
                attributeController.resetSelection()
                rampDialog.reject()
            }
        }
    }

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        id: screenshotHeader
        //: title of a dialog to add, update or delete ramps.
        title: qsTr("Ramp")
    }

    function openDialog() {
        if (!figureEditor || !rampModel) {
            return
        }
        updateLength()
        rampDialog.open()
    }

    function updateLength() {
        length.text = rampValidator.getRampLenght(
                    rampModel.rampLength).toLocaleString(locale, 'f', 2)
    }

    GroupBox {
        id: gridPropertiesView
        implicitWidth: parent.width
        implicitHeight: parent.height
        GridLayout {
            anchors.fill: parent
            columns: 3

            Label {
                id: startIDLabel
                Layout.preferredWidth: parent * 0.5
                text: qsTr("Start ID:")
                font.bold: true
            }
            Label {
                id: startID
                Layout.columnSpan: 2
                Layout.preferredWidth: parent * 0.5
                text: Number(rampModel.startPointID).toLocaleString(locale,
                                                                    'f', 0)
            }
            Label {
                id: lengthLabel
                Layout.preferredWidth: parent * 0.5
                text: rampValidator.lengthLabel
                font.bold: true
            }
            TextField {
                id: length
                Layout.columnSpan: 2
                Layout.preferredWidth: parent * 0.5
                text: rampValidator.getRampLenght(
                          rampModel.rampLength).toLocaleString(locale, 'f', 2)
                palette.text: length.acceptableInput ? "black" : "red"
                validator: DoubleValidator {
                    bottom: 0.00
                    top: rampValidator.maxRampLength
                }
                onEditingFinished: {
                    rampModel.rampLength = rampValidator.getValueInMillimeter(
                                Number.fromLocaleString(locale, length.text))
                }
            }
            Label {
                id: maxLengthLabel
                Layout.preferredWidth: parent * 0.5
                text: "max. " + rampValidator.lengthLabel
                font.bold: true
            }
            Label {
                id: maxLength
                Layout.columnSpan: 2
                Layout.preferredWidth: parent * 0.5
                text: rampValidator.maxRampLength.toLocaleString(locale, 'f', 2)
            }
            RadioButton {
                id: lengthInMM
                Layout.preferredWidth: parent * 0.33
                //: Label for a radio button to be able to set a length
                text: qsTr("Length [mm]")
                checked: true //rampValidator ? !rampValidator.enterLengthInDegree : false
                onClicked: {
                    if (!rampValidator) {
                        return
                    }
                    rampValidator.enterLengthInDegree = RampValidator.Millimeter
                    updateLength()
                }
            }
            RadioButton {
                id: lengthInDegree
                Layout.preferredWidth: parent * 0.33
                //: Label for a radio button to be able to set a length in degree
                text: qsTr("Degree [°]")
                //checked: rampValidator ? rampValidator.enterLengthInDegree : true
                onClicked: {
                    if (!rampValidator) {
                        return
                    }
                    rampValidator.enterLengthInDegree = RampValidator.Degree
                    updateLength()
                }
            }
            RadioButton {
                id: lengthInPercent
                Layout.preferredWidth: parent * 0.33
                //: Label for a radio button to be able to set a length in degree
                text: qsTr("Percent [%]")
                onClicked: {
                    if (!rampValidator) {
                        return
                    }
                    rampValidator.enterLengthInDegree = RampValidator.Percent
                    updateLength()
                }
            }
            Label {
                id: startPowerLabel
                Layout.preferredWidth: parent * 0.5
                text: qsTr("Start power [%]:")
                font.bold: true
            }
            TextField {
                id: startPower
                Layout.columnSpan: 2
                Layout.preferredWidth: parent * 0.5
                text: Number(ValueConverter.convertDoubleToPercent(
                                 rampModel.startPower)).toLocaleString(locale,
                                                                       'f', 1)
                validator: DoubleValidator {
                    bottom: 0.0
                    top: 100.0
                }
                onEditingFinished: {
                    rampModel.startPower = ValueConverter.convertFromPercentDouble(
                                Number.fromLocaleString(locale,
                                                        startPower.text))
                }
            }
            Label {
                id: endPowerLabel
                Layout.preferredWidth: parent * 0.5
                text: qsTr("End power [%]:")
                font.bold: true
            }
            TextField {
                id: endPower
                Layout.columnSpan: 2
                Layout.preferredWidth: parent * 0.5
                text: Number(ValueConverter.convertDoubleToPercent(
                                 rampModel.endPower)).toLocaleString(locale,
                                                                     'f', 1)
                validator: DoubleValidator {
                    bottom: 0.0
                    top: 100.0
                }
                onEditingFinished: {
                    rampModel.endPower = ValueConverter.convertFromPercentDouble(
                                Number.fromLocaleString(locale, endPower.text))
                }
            }
            Label {
                visible: FigureEditorSettings.dualChannelLaser
                id: startRingPowerLabel
                Layout.preferredWidth: parent * 0.5
                text: qsTr("Start ring power [%]:")
                font.bold: true
            }
            TextField {
                visible: FigureEditorSettings.dualChannelLaser
                id: startRingPower
                Layout.columnSpan: 2
                Layout.preferredWidth: parent * 0.5
                text: Number(ValueConverter.convertDoubleToPercent(
                                 rampModel.startPowerRing)).toLocaleString(
                          locale, 'f', 1)
                validator: DoubleValidator {
                    bottom: 0.0
                    top: 100.0
                }
                onEditingFinished: {
                    rampModel.startPowerRing = ValueConverter.convertFromPercentDouble(
                                Number.fromLocaleString(locale,
                                                        startRingPower.text))
                }
            }
            Label {
                visible: FigureEditorSettings.dualChannelLaser
                id: endRingPowerLabel
                Layout.preferredWidth: parent * 0.5
                text: qsTr("End ring power [%]:")
                font.bold: true
            }
            TextField {
                visible: FigureEditorSettings.dualChannelLaser
                id: endRingPower
                Layout.columnSpan: 2
                Layout.preferredWidth: parent * 0.5
                text: Number(ValueConverter.convertDoubleToPercent(
                                 rampModel.endPowerRing)).toLocaleString(
                          locale, 'f', 1)
                validator: DoubleValidator {
                    bottom: 0.0
                    top: 100.0
                }
                onEditingFinished: {
                    rampModel.endPowerRing = ValueConverter.convertFromPercentDouble(
                                Number.fromLocaleString(locale,
                                                        endRingPower.text))
                }
            }
        }
    }
}
