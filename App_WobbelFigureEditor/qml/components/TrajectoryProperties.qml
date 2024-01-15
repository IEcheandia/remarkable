import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

Control
{
    id: laserTrajectoryPropertiesItem
    property var attributeController: null
    property var figureEditor: null
    property var screenshotTool: null
    height: grid.implicitHeight

    rightPadding: 10
    leftPadding: 10

    background: Rectangle {
        id: background
        border.color: "lightgrey"
        border.width: 1
        radius: 3
    }

    contentItem: ColumnLayout {
        id: grid

        Label
        {
            id: trajectoryLabel
            Layout.fillWidth: true
            text: qsTr("Laser trajectory properties")
            font.pixelSize: 18
            font.bold: true
        }

        Label
        {
            id: trajectorySpeedLabel
            Layout.fillWidth: true
            text: qsTr("Trajectory speed [mm/s]:")
            font.bold: true
        }

        TextField
        {
            id: trajectorySpeed
            enabled: laserTrajectoryPropertiesItem.attributeController ? laserTrajectoryPropertiesItem.attributeController.selectedTrajectory : false
            Layout.fillWidth: true
            text: laserTrajectoryPropertiesItem.attributeController.selectedTrajectory ?  Number(laserTrajectoryPropertiesItem.attributeController.selectedTrajectory.speed).toLocaleString(locale, 'f', 2) : "0.0";
            palette.text: trajectorySpeed.acceptableInput ? "black" : "red"
            validator: DoubleValidator
            {
                bottom: -10000
                top: 10000
                decimals: 2
            }
        }

        Label
        {
            id: trajectoryLengthLabel
            Layout.fillWidth: true
            text: qsTr("Trajectory length [mm]:")
            font.bold: true
        }

        TextField
        {
            id: trajectoryLength
            enabled: laserTrajectoryPropertiesItem.attributeController ? laserTrajectoryPropertiesItem.attributeController.selectedTrajectory : false
            Layout.fillWidth: true
            text: laserTrajectoryPropertiesItem.attributeController.selectedTrajectory ?  Number(laserTrajectoryPropertiesItem.attributeController.selectedTrajectory.trajectoryLength).toLocaleString(locale, 'f', 3) : "0.0";
            readOnly: true
            validator: DoubleValidator
            {
                bottom: -10000
                top: 10000
                decimals: 2
            }
        }

        Label
        {
            id: trajectoryTimeLabel
            Layout.fillWidth: true
            text: qsTr("Trajectory time [s]:")
            font.bold: true
        }

        TextField
        {
            id: trajectoryTime
            enabled: laserTrajectoryPropertiesItem.attributeController ? laserTrajectoryPropertiesItem.attributeController.selectedTrajectory : false
            Layout.fillWidth: true
            text: laserTrajectoryPropertiesItem.attributeController.selectedTrajectory ?  Number(laserTrajectoryPropertiesItem.attributeController.selectedTrajectory.time).toLocaleString(locale, 'f', 6) : "0.0";
            readOnly: true
            validator: DoubleValidator
            {
                bottom: -10000
                top: 10000
                decimals: 2
            }
        }

        Button
        {
            enabled: laserTrajectoryPropertiesItem.attributeController ? laserTrajectoryPropertiesItem.attributeController.selectedTrajectory : false
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            text: "Update"
        }

        Button
        {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            text: "Velocity graph"
            onClicked:
            {
                var velocityInfo = showVelocity.createObject(Overlay.overlay, {"handler": velocityPlotHandler});
                velocityInfo.open();
            }
        }

        PlotHandler
        {
            id: velocityPlotHandler
            figureEditor: laserTrajectoryPropertiesItem.figureEditor

            type: PlotHandler.Velocity
        }

        Component {
            id: showVelocity
            ShowPlot
            {
                anchors.centerIn: parent
                width: parent.width * 0.5
                height: parent.height * 0.7
                handler: velocityPlotHandler
                screenshotTool: laserTrajectoryPropertiesItem.screenshotTool
            }
        }

        Item
        {
            id: wildcard
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
