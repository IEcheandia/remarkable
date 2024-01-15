import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.image 1.0 as PrecitecImage
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

/**
 * Live image control combined with an control to move the axis.
 * Requires the axisController property to be set
 **/
Item {
    id: root
    property var axisController: null
    property var scanLabController: null
    property alias updating: busyIndicator.running
    property alias handlersEnabled: image.handlersEnabled
    property alias hasInfoBox: image.hasInfoBox
    property alias filterInstances: image.filterInstances
    property alias screenshotTool: image.screenshotTool

    signal closeInfoBox()
    signal updateInfoBox()

    onCloseInfoBox: image.closeInfoBox()
    onUpdateInfoBox: image.updateInfoBox()

    PrecitecImage.Image {
        id: image
        anchors {
            fill: parent
            margins: 0
        }
        clip: true
        opacity: busyIndicator.running ? 0.5 : 1.0

        RowLayout {
            anchors {
                horizontalCenter: parent. horizontalCenter
                bottom: parent.bottom
                bottomMargin: 5
            }
            visible: yAxisInformation.axisEnabled
            enabled: axisController && axisController.systemStatus && axisController.systemStatus.state == SystemStatusServer.Live

            spacing: 0

            ToolButton {
                icon.name: "arrow-left"
                display: Button.IconOnly
                enabled: yAxisInformation.positionUserUnit - 100 > (yAxisInformation.softwareLimitsEnabled ? yAxisInformation.lowerLimit : yAxisInformation.minimumPosition)
                onClicked: axisController.moveAxis(yAxisInformation.positionUserUnit - 100)
            }
            TextField {
                Layout.alignment: Qt.AlignHCenter
                selectByMouse: true
                placeholderText: Number(yAxisInformation.positionUserUnit / 1000.0).toLocaleString(busyIndicator.locale, 'f', 3) + " mm"
                validator: DoubleValidator {
                    bottom: (yAxisInformation.softwareLimitsEnabled ? yAxisInformation.lowerLimit : yAxisInformation.minimumPosition) / 1000.0
                    top: (yAxisInformation.softwareLimitsEnabled ? yAxisInformation.upperLimit : yAxisInformation.maximumPosition) / 1000.0
                }
                onAccepted: {
                    if (axisController)
                    {
                        axisController.moveAxis(Number.fromLocaleString(busyIndicator.locale, text) * 1000.0);
                    }
                }
                onActiveFocusChanged: {
                    if (!activeFocus) {
                        clear();
                    }
                }
            }
            ToolButton {
                icon.name: "arrow-right"
                display: Button.IconOnly
                enabled: yAxisInformation.positionUserUnit + 100 < (yAxisInformation.softwareLimitsEnabled ? yAxisInformation.upperLimit : yAxisInformation.maximumPosition)
                onClicked: axisController.moveAxis(yAxisInformation.positionUserUnit + 100)
            }
        }

        GridLayout {
            anchors.fill: parent
            anchors.margins: 5
            visible: SystemConfiguration.Scanner2DEnable            
            enabled: root.scanLabController && root.scanLabController.ready && !root.scanLabController.updating 

            columns: 3

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            PrecitecApplication.ArrowButton {
                enabled: root.scanLabController && root.scanLabController.canIncrementY
                direction: "up"
                onClicked: scanLabController.incrementYPosition()

                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }

            PrecitecApplication.ArrowButton {
                enabled: root.scanLabController && root.scanLabController.canDecrementX
                direction: "left"
                onClicked: scanLabController.decrementXPosition()

                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            PrecitecApplication.ArrowButton {
                enabled: root.scanLabController && root.scanLabController.canIncrementX
                direction: "right"
                onClicked: scanLabController.incrementXPosition()

                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
            }

            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            PrecitecApplication.ArrowButton {
                enabled: root.scanLabController && root.scanLabController.canDecrementY
                direction: "down"
                onClicked: scanLabController.decrementYPosition()

                Layout.alignment: Qt.AlignBottom | Qt.AlignHCenter
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }
    BusyIndicator {
        id: busyIndicator
        anchors.centerIn: parent
        running: false
    }
}
