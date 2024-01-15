import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.image 1.0 as PrecitecImage
import Precitec.AppGui 1.0

Item{
    id: calibration

    property alias screenshotTool: image.screenshotTool

    Connections {
        target: HardwareModule.systemStatus
        function onReturnedFromCalibration() {
            idmCalibrationController.endOctLineCalibration();
        }
    }

    RowLayout {
        anchors {
            fill: parent
            margins: spacing
        }
        PrecitecImage.Image {
            id: image
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.33
            Layout.minimumWidth: parent.width * 0.33
        }
        Item {
            Layout.fillHeight: true
            Layout.fillWidth: true
            ColumnLayout {
                id: calibrationSelection
                spacing: 5
                anchors {
                    top: parent.top
                    left: parent.left
                }
                Text {
                    text: "OCT Line Calibration"
                    font.bold: true
                }
                Button {
                    text: "Start Calibration"
                    enabled: idmCalibrationController.canCalibrate && (HardwareModule.systemStatus.state != SystemStatusServer.Calibration) && (HardwareModule.systemStatus.state != SystemStatusServer.Automatic) && (HardwareModule.systemStatus.state != SystemStatusServer.EmergencyStop)
                    onClicked: {
                        idmCalibrationController.octLineCalibration();
                    }
                }
            }
        }
    }

    IDMCalibrationController {
        id: idmCalibrationController
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
    }
}
