import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0
import precitec.gui 1.0

Item {
    property alias screenshotTool: image.screenshotTool

    id: root

    Connections {
        target: HardwareModule.systemStatus
        function onReturnedFromCalibration() {
            cameraCalibrationModel.endCalibration();
        }
    }

    CameraCalibrationModel {
        id: cameraCalibrationModel

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        calibrationDeviceProxy: HardwareModule.calibrationDeviceProxy
        workflowDeviceProxy: HardwareModule.workflowDeviceProxy
        grabberDeviceProxy: HardwareModule.grabberDeviceProxy
    }


    RowLayout {
        anchors {
            fill: parent
            margins: spacing
        }

        PrecitecImage.Image {
            Layout.fillHeight: true
            Layout.minimumWidth: parent.width * 0.8

            id: image
            clip: true
            handlersEnabled: false
        }

        Button {
            Layout.fillWidth: true
            objectName: "calibration-start-calibGridChessboard"
            text: qsTr("Start Chessboard Calibration")
            visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
            enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig)
            onClicked: {
                cameraCalibrationModel.startChessboardCalibration();
            }
        }
    }
}
