import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.components.plotter 1.0

import wobbleFigureEditor.components 1.0

Dialog {
    id: showInformation
    property var handler: null
    property alias screenshotTool: screenshotHeader.screenshotTool
    modal: true

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: handler && handler.type ? qsTr("Velocity values") : qsTr(
                                             "Power values")
    }

    ColumnLayout {
        anchors.fill: parent

        Label {
            id: labelMainInfo
            Layout.fillWidth: true
            text: handler ? handler.type ? qsTr("Velocity") : qsTr("Power") : ""
            font.bold: true
        }

        PlotterChart {
            id: mainPlotter
            Layout.fillWidth: true
            Layout.fillHeight: true

            plotter.toolTipEnabled: true

            controller {
                id: controller
                xLegendPrecision: 0
                yLegendPrecision: 0
            }
            xAxisController {
                id: xAxisController
                autoAdjustXAxis: true
            }
            yAxisController {
                yMin: 0.0
                yMax: handler ? handler.type ? 1500.0 : handler.showYAxisInWatt ? FigureEditorSettings.laserMaxPower : 100.0 : 1.0
            }

            xUnit: handler ? handler.showMillimeters ? qsTr("[mm]") : qsTr(
                                                           "[ms]") : ""
            yUnit: handler ? handler.type ? qsTr("[mm/s]") : handler.showYAxisInWatt ? qsTr("[W]") : qsTr("[%]") : ""

            xLegendUnitBelowVisible: true

            yLegendRightVisible: false //if true X-unit is visible
            yRightLegendUnitVisible: false

            Component.onCompleted: {
                if (handler) {
                    handler.generateInfo()
                    handler.infoRamp.name = qsTr("Center")
                    mainPlotter.plotter.addDataSet(handler.infoRamp)
                    if (FigureEditorSettings.dualChannelLaser) {
                        handler.ringPower.name = qsTr("Ring")
                        mainPlotter.plotter.addDataSet(handler.ringPower)
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            ButtonGroup {
                id: unitYButtons

                onClicked: {
                    handler.showYAxisInWatt = showW.checked
                }
            }

            RadioButton {
                id: showW
                text: "W"
                checked: handler && handler.showYAxisInWatt
                ButtonGroup.group: unitYButtons
            }

            RadioButton {
                id: showPercent
                text: "%"
                checked: handler && !handler.showYAxisInWatt
                ButtonGroup.group: unitYButtons
            }

            Item {
                Layout.fillWidth: true
            }

            CheckBox {
                id: showCenterPower
                enabled: handler
                text: qsTr("Center")
                checked: handler && handler.infoRamp.enabled
                checkable: true
                onClicked: {
                    handler.infoRamp.enabled = !handler.infoRamp.enabled
                }
            }

            CheckBox {
                id: showRingPower
                visible: FigureEditorSettings.dualChannelLaser
                enabled: handler
                text: qsTr("Ring")
                checked: handler && handler.ringPower.enabled
                checkable: true
                onClicked: {
                    handler.ringPower.enabled = !handler.ringPower.enabled
                }
            }

            Item {
                Layout.fillWidth: true
            }

            ButtonGroup {
                id: unitXButtons

                onClicked: {
                    handler.showMillimeters = showMM.checked
                }
            }

            RadioButton {
                id: showMM
                text: "mm"
                checked: handler && handler.showMillimeters
                ButtonGroup.group: unitXButtons
            }

            RadioButton {
                id: showS
                text: "ms"
                checked: handler && !handler.showMillimeters
                ButtonGroup.group: unitXButtons
            }
        }
    }
}
