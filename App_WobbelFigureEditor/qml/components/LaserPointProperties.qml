import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

Control {
    id: laserPointPropertiesItem

    property var attributeController: null
    property var figureEditor: null
    property var figureAnalyzer: null
    property var simulationController: null
    property var screenshotTool: null
    height: grid.implicitHeight
    width: grid.implicitWidth + leftPadding + rightPadding
    property bool m_isLastPoint: false

    rightPadding: 10
    leftPadding: 10
    background: Rectangle {
        id: background
        border.color: "lightgrey"
        border.width: 1
        radius: 3
    }

    function checkAttributeController() {
        if (laserPointPropertiesItem.attributeController != null
                && laserPointPropertiesItem.attributeController.selectedPoint
                && FigureEditorSettings.fileType != FileType.Basic) {
            return true
        }
        return false
    }

    function setStringValueOfCheckBoxChecked(isChecked) {
        if (isChecked) {
            return "-1"
        } else {
            return "0"
        }
    }

    function disconnectSignalSlotConnection() {
        laserPowerEnabler.onCheckedChanged.disconnect(updateLaserPowerChecked)
        laserPower.onEditingFinished.disconnect(updateLaserPowerValue)
        ringPowerEnabler.onCheckedChanged.disconnect(updateRingPowerChecked)
        ringPower.onEditingFinished.disconnect(updateRingPowerValue)
        velocityEnabler.onCheckedChanged.disconnect(updateVelocityChecked)
        velocity.onEditingFinished.disconnect(updateVelocityValue)
    }

    function connectSignalSlotConnection() {
        laserPowerEnabler.onCheckedChanged.connect(updateLaserPowerChecked)
        laserPower.onEditingFinished.connect(updateLaserPowerValue)
        ringPowerEnabler.onCheckedChanged.connect(updateRingPowerChecked)
        ringPower.onEditingFinished.connect(updateRingPowerValue)
        velocityEnabler.onCheckedChanged.connect(updateVelocityChecked)
        velocity.onEditingFinished.connect(updateVelocityValue)
    }

    // Workaround - when you start to select a point, the onEditingFinished will be submit cause of init valus from current Point - but there currently no changes
    function initilizieLaserPoint() {
        disconnectSignalSlotConnection()
        if (checkAttributeController()) {
            m_isLastPoint = laserPointPropertiesItem.attributeController.isLastPointOfFigure()
            if (!m_isLastPoint) {
                laserPowerEnabler.checked = laserPointPropertiesItem.attributeController.isLaserPointCurrentLaserPowerDependOnThePreviousPoint()
                ringPowerEnabler.checked = laserPointPropertiesItem.attributeController.isLaserPointCurrentRingPowerDependOnThePreviousPoint()
                velocityEnabler.checked = laserPointPropertiesItem.attributeController.isLaserPointCurrentVelocityDependOnThePreviousPoint()
                laserPower.text = laserPointPropertiesItem.attributeController.laserPointLaserPower
                ringPower.text = laserPointPropertiesItem.attributeController.laserPointRingPower
                velocity.text = laserPointPropertiesItem.attributeController.laserPointVelocity
            }
            connectSignalSlotConnection()
        }
    }

    function updateLaserPowerChecked() {
        if (checkAttributeController()) {
            attributeController.laserPointLaserPower = setStringValueOfCheckBoxChecked(
                        laserPowerEnabler.checked)
            laserPower.text = attributeController.laserPointLaserPower
        }
    }

    function updateLaserPowerValue() {
        if (checkAttributeController()) {
            attributeController.laserPointLaserPower = laserPower.text
            // needed to update checkbox cause -1 is a valid value for textField
            laserPowerEnabler.checked = laserPointPropertiesItem.attributeController.isLaserPointCurrentLaserPowerDependOnThePreviousPoint()
        }
    }

    function updateRingPowerChecked() {
        if (checkAttributeController()) {
            attributeController.laserPointRingPower = setStringValueOfCheckBoxChecked(
                        ringPowerEnabler.checked)
            ringPower.text = attributeController.laserPointRingPower
        }
    }

    function updateRingPowerValue() {
        if (checkAttributeController()) {
            attributeController.laserPointRingPower = ringPower.text
            // needed to update checkbox cause -1 is a valid value for textField
            ringPowerEnabler.checked = laserPointPropertiesItem.attributeController.isLaserPointCurrentRingPowerDependOnThePreviousPoint()
        }
    }

    function updateVelocityChecked() {
        if (checkAttributeController()) {
            attributeController.laserPointVelocity = velocityEnabler.checked ? "-1" : "1"
            velocity.text = attributeController.laserPointVelocity
        }
    }

    function updateVelocityValue() {
        if (checkAttributeController()) {
            attributeController.laserPointVelocity = velocity.text
            // needed to update checkbox cause -1 is a valid value for textField
            velocityEnabler.checked = laserPointPropertiesItem.attributeController.isLaserPointCurrentVelocityDependOnThePreviousPoint()
        }
    }

    Component.onCompleted: {
        attributeController.onSelectionChanged.connect(initilizieLaserPoint)
    }

    contentItem: GridLayout {
        id: grid
        columns: 2

        Label {
            id: laserPointLabel
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: qsTr("Laser point properties")
            font.pixelSize: 18
            font.bold: true
        }

        Label {
            id: laserPointIDLabel
            Layout.fillWidth: true
            text: qsTr("Laser point ID:")
            font.bold: true
        }
        TextField {
            id: laserPointID
            enabled: laserPointPropertiesItem.attributeController
                     && laserPointPropertiesItem.attributeController.selectedPoint
            Layout.fillWidth: true
            text: checkAttributeController(
                      ) ? laserPointPropertiesItem.attributeController.laserPointID : ""
            readOnly: true
            background: Item {}
        }

        Label {
            id: laserPointPositionLabel
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: qsTr("Laser point position:")
            font.pixelSize: 18
            font.bold: true
        }

        Label {
            id: laserPointPositionXLabel
            Layout.fillWidth: true
            text: qsTr("X-Coord [mm]")
            font.bold: true
        }

        Label {
            id: laserPointPositionYLabel
            Layout.fillWidth: true
            text: qsTr("Y-Coord [mm]")
            font.bold: true
        }

        TextField {
            Layout.fillWidth: true
            id: xCoordinateLaserPoint
            enabled: checkAttributeController()
            text: checkAttributeController(
                      ) ? laserPointPropertiesItem.attributeController.laserPointCoordinateX : "0"
            palette.text: xCoordinateLaserPoint.acceptableInput ? "black" : "red"
            validator: DoubleValidator {
                bottom: -10000
                top: 10000
                decimals: 2
                locale: Qt.locale().name
            }
            onEditingFinished: {
                if (laserPointPropertiesItem.attributeController.selectedPoint != null) {
                    laserPointPropertiesItem.attributeController.laserPointCoordinateX = text
                }
            }
        }

        TextField {
            Layout.fillWidth: true
            id: yCoordinateLaserPoint
            enabled: checkAttributeController()
            text: checkAttributeController(
                      ) ? laserPointPropertiesItem.attributeController.laserPointCoordinateY : "0"
            palette.text: yCoordinateLaserPoint.acceptableInput ? "black" : "red"
            validator: DoubleValidator {
                bottom: -10000
                top: 10000
                decimals: 2
                locale: Qt.locale().name
            }
            onEditingFinished: {
                if (laserPointPropertiesItem.attributeController.selectedPoint != null) {
                    laserPointPropertiesItem.attributeController.laserPointCoordinateY = text
                }
            }
        }
        Label {
            id: laserPowerLabel
            visible: figureEditor && figureEditor.fileType != FileType.Overlay
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: figureEditor
                  && figureEditor.figure.analogPower ? qsTr("Laser center power [%]") : qsTr(
                                                           "Laser profile")
            font.bold: true
        }

        Label {
            id: currentLaserPowerLabel
            visible: figureEditor && figureEditor.fileType != FileType.Overlay
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: checkAttributeController(
                      ) ? laserPointPropertiesItem.attributeController.currentlaserPointLaserPower : ""
        }

        CheckBox {
            id: laserPowerEnabler

            Layout.fillWidth: true
            Layout.columnSpan: 2
            Layout.leftMargin: 0
            visible: checkAttributeController() && !m_isLastPoint
                     && figureEditor && figureEditor.fileType == FileType.Seam
            leftPadding: 0
            text: laserPointID.text == "0" ? qsTr("Use power-definition of the product") : qsTr(
                                                 "Use power of previous point")
        }

        TextField {
            id: laserPower
            Layout.fillWidth: true
            Layout.columnSpan: 2
            palette.text: laserPower.acceptableInput ? "black" : "red"
            visible: checkAttributeController() && !m_isLastPoint
                     && !laserPowerEnabler.checked
            selectByMouse: true
            validator: IntValidator {
                bottom: figureEditor && figureEditor.fileType
                        == FileType.Seam ? PowerLimits.Default : PowerLimits.MinValue
                top: figureEditor
                     && figureEditor.figure.analogPower ? PowerLimits.AnalogMaxValue : PowerLimits.DigitalMaxValue
            }
        }

        Label {
            visible: laserPointPropertiesItem.figureEditor
                     && laserPointPropertiesItem.figureEditor.figure.analogPower
                     && FigureEditorSettings.dualChannelLaser
                     && figureEditor.fileType != FileType.Overlay
            id: ringPowerLabel
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: qsTr("Laser ring power [%]")
            font.bold: true
        }

        Label {
            id: currentRingPowerLabel
            visible: ringPowerLabel.visible
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: checkAttributeController(
                      ) ? laserPointPropertiesItem.attributeController.currentlaserPointRingPower : ""
        }

        CheckBox {
            id: ringPowerEnabler

            Layout.fillWidth: true
            Layout.columnSpan: 2
            Layout.leftMargin: 0
            visible: checkAttributeController() && ringPowerLabel.visible
                     && !m_isLastPoint && laserPointPropertiesItem.figureEditor
                     && figureEditor.fileType == FileType.Seam
            leftPadding: 0
            text: laserPointID.text == "0" ? qsTr("Use power-definition of the product") : qsTr(
                                                 "Use power of previous point")
        }

        TextField {
            visible: checkAttributeController() && !m_isLastPoint
                     && ringPowerEnabler.visible && !ringPowerEnabler.checked
            id: ringPower
            enabled: laserPointPropertiesItem.attributeController
                     && laserPointPropertiesItem.attributeController.selectedPoint
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: "0"
            palette.text: ringPower.acceptableInput ? "black" : "red"
            selectByMouse: true
            validator: IntValidator {
                bottom: figureEditor && figureEditor.fileType
                        == FileType.Seam ? PowerLimits.Default : PowerLimits.MinValue
                top: PowerLimits.AnalogMaxValue
            }
        }

        Label {
            visible: figureEditor && figureEditor.fileType == FileType.Seam
            id: velocityLabel
            Layout.fillWidth: true
            Layout.columnSpan: 2
            //: This is a label and the title for a text field where the speed for a part of the seam can be changed.
            text: qsTr("Velocity [mm/s]")
            font.bold: true
        }

        Label {
            id: currentVelocityLabel
            visible: velocityLabel.visible
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: checkAttributeController(
                      ) ? laserPointPropertiesItem.attributeController.currentlaserPointVelocity : ""
        }

        CheckBox {
            id: velocityEnabler

            Layout.fillWidth: true
            Layout.columnSpan: 2
            Layout.leftMargin: 0
            visible: checkAttributeController() && !m_isLastPoint
                     && figureEditor && figureEditor.fileType == FileType.Seam
            leftPadding: 0
            text: laserPointID.text == "0" ? qsTr("Use velocity-definition of the product") : qsTr(
                                                 "Use velocity of previous point")
        }

        TextField {
            visible: !m_isLastPoint && !velocityEnabler.checked && figureEditor
                     && figureEditor.fileType == FileType.Seam
            id: velocity
            enabled: laserPointPropertiesItem.attributeController
                     && laserPointPropertiesItem.attributeController.selectedPoint
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: "0"
            palette.text: velocity.acceptableInput ? "black" : "red"
            selectByMouse: true
            validator: IntValidator {
                bottom: VelocityLimits.MinValue
                top: VelocityLimits.JumpMaxValue
            }
        }

        Button {
            visible: laserPointPropertiesItem.figureEditor.fileType == FileType.Seam
                     && !laserPointPropertiesItem.simulationController.simulationMode
            id: setNewStartPoint
            enabled: laserPointPropertiesItem.attributeController
                     && laserPointPropertiesItem.attributeController.selectedPoint
                     && laserPointPropertiesItem.attributeController.selectedPoint.ID > 0
            Layout.columnSpan: 2
            Layout.fillWidth: true
            //: title of a button. Button change the start point of a seam to another point.
            text: qsTr("New start")

            onClicked: {
                laserPointPropertiesItem.figureEditor.setNewStartPoint(
                            Number.fromLocaleString(locale, laserPointID.text))
            }
        }

        Button {
            visible: laserPointPropertiesItem.figureEditor.fileType == FileType.Seam
                     && !laserPointPropertiesItem.simulationController.simulationMode
            Layout.fillWidth: true
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignHCenter
            //: title of a button. Button removes a point.
            text: qsTr("Delete")
            onClicked: {
                figureEditor.deletePoint(Number.fromLocaleString(
                                             locale, laserPointID.text))
            }
        }

        Button {
            visible: laserPointPropertiesItem.figureEditor.fileType == FileType.Seam
                     && !laserPointPropertiesItem.simulationController.simulationMode
            enabled: rampValidator ? rampValidator.isPointAStartPoint
                                     || !rampValidator.isPointAlreadyInRamp : false
            Layout.fillWidth: true
            Layout.columnSpan: 2
            Layout.alignment: Qt.AlignHCenter
            //: title of a button. Button shows ramp dialog which is used to add, update or delete ramps.
            text: qsTr("Power ramp")
            onClicked: {
                rampDialog.openDialog()
            }
        }

        RampDialog {
            id: rampDialog
            figureEditor: laserPointPropertiesItem.figureEditor
            rampModel: rampModel
            rampValidator: rampValidator
            attributeController: laserPointPropertiesItem.attributeController
            screenshotTool: laserPointPropertiesItem.screenshotTool
        }

        RampModel {
            id: rampModel
            figureEditor: laserPointPropertiesItem.figureEditor
            startPointID: laserPointPropertiesItem.attributeController.selectedPoint ? laserPointPropertiesItem.attributeController.selectedPoint.ID : "-1"
        }

        RampValidator {
            id: rampValidator
            figureEditor: laserPointPropertiesItem.figureEditor
            startPointID: laserPointPropertiesItem.attributeController.selectedPoint ? laserPointPropertiesItem.attributeController.selectedPoint.ID : "-1"
            laserPointController: laserPointPropertiesItem.simulationController.laserPointController
        }

        Connections {
            target: laserPointPropertiesItem.attributeController

            function onLoadRampDialog() {
                rampDialog.openDialog()
            }
        }

        Item {
            id: wildcard
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
