import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.removableDevices 1.0 as RemovableDevices

import Precitec.AppGui 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0

Item {
    id: calibration
    property alias screenshotTool: image.screenshotTool

    Connections {
        target: HardwareModule.systemStatus
        function onReturnedFromCalibration() {
            scanfieldCalibrationController.endCalibration();
        }
    }

    FocusPositionController {
        id: controller
        weldHeadDevice: HardwareModule.weldHeadDeviceProxy
    }

    onVisibleChanged: {
        if (calibration.visible)
        {
            scanfieldCalibrationController.initValues();
        }
    }

    RemovableDevices.DownloadService {
        id: downloadService
        productName: Qt.application.name
        backupPath: RemovableDevices.Service.path
        validate: false
    }

    RowLayout {
        anchors {
            fill: parent
            margins: spacing
        }

        PrecitecImage.Image {
            Layout.fillHeight: true
            Layout.preferredWidth: calibration.width * 0.8

            id: image
            clip: true

            Rectangle {
                color: Qt.rgba(1.0, 1.0, 1.0, 0.5)
                border {
                    width: 1
                    color: PrecitecApplication.Settings.alternateBackground
                }
                visible: roiVisible.checked && selectionTabBar.currentIndex == 0 && !scanfieldCalibrationController.calibrating && UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                x: image.x + image.paintedRect.x + (scanfieldCalibrationController.searchROIX) * image.zoom
                y: image.y + image.paintedRect.y + (scanfieldCalibrationController.searchROIY) * image.zoom
                width: scanfieldCalibrationController.searchROIW * image.zoom
                height: scanfieldCalibrationController.searchROIH * image.zoom
            }
        }
        PrecitecApplication.ActiveFocusControlAwareFlickable {
            Layout.fillHeight: true
            Layout.fillWidth: true
            flickableDirection: Flickable.VerticalFlick
            ColumnLayout {
                property int spinBoxMaxSize: calibration.width * 0.2 - heightLabel.implicitWidth - 130

                id: settingsPanel

                GroupBox {
                    Layout.fillWidth: true

                    title: qsTr("Scanfield Range [mm]")

                    GridLayout {
                        anchors.fill: parent

                        columns: 2

                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Min X:")
                        }
                        PrecitecApplication.DoubleSpinBox {
                            Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                            to: 1000.0
                            from: -1000.0
                            stepSize: 5.0

                            enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                            value: scanfieldCalibrationController.minX

                            onSetValue: {
                                scanfieldCalibrationController.minX = newValue;
                            }
                        }
                        Label {
                            Layout.fillWidth: true
                            text: qsTr("Min Y:")
                        }
                        PrecitecApplication.DoubleSpinBox {
                            Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                            to: 1000.0
                            from: -1000.0
                            stepSize: 5.0

                            enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                            value: scanfieldCalibrationController.minY

                            onSetValue: {
                                scanfieldCalibrationController.minY = newValue;
                            }
                        }
                        Label {
                            text: qsTr("Max X:")
                        }
                        PrecitecApplication.DoubleSpinBox {
                            Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                            to: 1000.0
                            from: -1000.0
                            stepSize: 5.0

                            enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                            value: scanfieldCalibrationController.maxX

                            onSetValue: {
                                scanfieldCalibrationController.maxX = newValue;
                            }
                        }
                        Label {
                            text: qsTr("Max Y:")
                        }
                        PrecitecApplication.DoubleSpinBox {
                            Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                            to: 1000.0
                            from: -1000.0
                            stepSize: 5.0

                            enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                            value: scanfieldCalibrationController.maxY

                            onSetValue: {
                                scanfieldCalibrationController.maxY = newValue;
                            }
                        }
                    }
                }

                TabBar {
                    Layout.fillWidth: true
                    id: selectionTabBar
                        TabButton {
                            text: qsTr("Camera")
                        }
                        TabButton {
                            text: qsTr("IDM")
                        }
                        TabButton {
                            objectName: "scanfield-calibration-tabbar-scanner"
                            //: title of a tab button in a TabBar
                            text: qsTr("Scanner")
                            // TODO: remove once MarkingEngine supports calibration
                            enabled: SystemConfiguration.Scanner2DController == SystemConfiguration.Scanlab
                        }
                        TabButton {
                            text: qsTr("Verification")
                        }
                    }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    currentIndex: selectionTabBar.currentIndex

                    ColumnLayout {
                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Search ROI [pixel]")

                            GridLayout {
                                anchors.fill: parent

                                columns: 2

                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("X:")
                                }
                                SpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: scanfieldCalibrationController.sensorWidth
                                    from: 0
                                    stepSize: 10.0

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.searchROIX

                                    onValueChanged: {
                                        scanfieldCalibrationController.searchROIX = value;
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("Y:")
                                }
                                SpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: scanfieldCalibrationController.sensorHeight
                                    from: 0
                                    stepSize: 10.0

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.searchROIY

                                    onValueChanged: {
                                        scanfieldCalibrationController.searchROIY = value;
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("Width:")
                                }
                                SpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: scanfieldCalibrationController.sensorWidth
                                    from: 0
                                    stepSize: 10.0

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.searchROIW

                                    onValueChanged: {
                                        scanfieldCalibrationController.searchROIW = value;
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    id: heightLabel
                                    text: qsTr("Height:")
                                }
                                SpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: scanfieldCalibrationController.sensorHeight
                                    from: 0
                                    stepSize: 10.0

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.searchROIH

                                    onValueChanged: {
                                        scanfieldCalibrationController.searchROIH = value;
                                    }
                                }
                                CheckBox {
                                    Layout.columnSpan: 2
                                    id: roiVisible
                                    text: qsTr("ROI visible")
                                    checked: true
                                }
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Target Image Calibration")

                            Button {
                                anchors.fill: parent
                                text: qsTr("Start")
                                enabled: (SystemConfiguration.Type_of_Sensor == SystemConfiguration.TypeOfSensor.Coax) && !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                onClicked: scanfieldCalibrationController.startTargetImageCalibration()
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Chessboard Calibration")

                            Button {
                                anchors.fill: parent
                                text: qsTr("Start")
                                enabled: HardwareModule.coaxCameraEnabled && !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                onClicked: scanfieldCalibrationController.startCameraCalibration()
                            }
                        }
                    }

                    ColumnLayout {
                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Sample Distance [mm]")

                            GridLayout {
                                anchors.fill: parent

                                columns: 2

                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("\u0394 X:")
                                }
                                PrecitecApplication.DoubleSpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: 1000.0
                                    from: -1000.0
                                    stepSize: 5.0

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.idmDeltaX

                                    onSetValue: {
                                        scanfieldCalibrationController.idmDeltaX = newValue;
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("\u0394 Y:")
                                }
                                PrecitecApplication.DoubleSpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: 1000.0
                                    from: -1000.0
                                    stepSize: 5.0

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.idmDeltaY

                                    onSetValue: {
                                        scanfieldCalibrationController.idmDeltaY = newValue;
                                    }
                                }
                            }
                        }
                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("IDM Adaptive Exposure Mode:")

                            ColumnLayout {
                                anchors.fill: parent

                                CheckBox {
                                    Layout.fillWidth: true
                                    text: qsTr("Enabled")
                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    checked: scanfieldCalibrationController.adaptiveExposureMode
                                    onToggled: scanfieldCalibrationController.adaptiveExposureMode = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("Basic Value:")
                                }
                                SpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: 100
                                    from: 0
                                    stepSize: 5

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.adaptiveExposureBasicValue

                                    onValueChanged: {
                                        scanfieldCalibrationController.adaptiveExposureBasicValue = value;
                                    }
                                }
                            }
                        }
                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Repetitions per Sample")

                            SpinBox {
                                anchors.fill: parent

                                to: 100
                                from: 0

                                editable: true
                                enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                value: scanfieldCalibrationController.routineRepetitions
                                onValueModified: {
                                    scanfieldCalibrationController.routineRepetitions = value;
                                }
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }
                        }
                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Depth Field Calibration")

                            Button {
                                anchors.fill: parent
                                text: qsTr("Start")
                                enabled: SystemConfiguration.IDM_Device1Enable && !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                onClicked: scanfieldCalibrationController.startIdmZCalibration()
                            }
                        }
                    }
                    ColumnLayout {
                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Welding Parameters")

                            GridLayout {
                                anchors.fill: parent

                                columns: 2
                                Label {
                                    //: label for a spinbox
                                    text: qsTr("Z-Offset [mm]:")
                                    Layout.alignment: Qt.AlignLeft
                                }
                                PrecitecApplication.DoubleSpinBox {
                                    objectName: "scanfield-calibration-scanner-zOffset"
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize
                                    to: 50.0
                                    from: -50.0
                                    stepSize: 1.0
                                    precision: 3
                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    property double defaultValue: Number(controller.systemOffset)
                                    value: defaultValue
                                    onSetValue: {
                                        scanfieldCalibrationController.zCollDrivingRelative = newValue;
                                        value = newValue
                                    }
                                }

                                Label {
                                    //: label for a spinbox
                                    text: qsTr("Power [%]:")
                                    Layout.alignment: Qt.AlignLeft
                                }
                                SpinBox {
                                    objectName: "scanfield-calibration-scanner-power"
                                    from: 0
                                    to: 100
                                    editable: true
                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.laserPowerInPctForCalibration
                                    onValueModified: {
                                        scanfieldCalibrationController.laserPowerInPctForCalibration = value;
                                    }
                                    Component.onCompleted: {
                                        contentItem.selectByMouse = true;
                                    }
                                    Layout.fillWidth: true
                                }
                                Label {
                                    //: label for a spinbox
                                    text: qsTr("Duration [ms]:")
                                    Layout.alignment: Qt.AlignLeft
                                }
                                SpinBox {
                                    objectName: "scanfield-calibration-scanner-duration"
                                    from: 1
                                    to: 1000
                                    editable: true
                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.weldingDurationInMsForCalibration
                                    onValueModified: {
                                        scanfieldCalibrationController.weldingDurationInMsForCalibration = value;
                                    }
                                    Component.onCompleted: {
                                        contentItem.selectByMouse = true;
                                    }
                                    Layout.fillWidth: true
                                }
                                Label {
                                    //: label for a spinbox
                                    text: qsTr("Jump Speed [mm/s]:")
                                    Layout.alignment: Qt.AlignLeft
                                }
                                SpinBox {
                                    objectName: "scanfield-calibration-scanner-jump-speed"
                                    from: 1
                                    to: 2000
                                    editable: true
                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.jumpSpeedInMmPerSecForCalibration
                                    onValueModified: {
                                        scanfieldCalibrationController.jumpSpeedInMmPerSecForCalibration = value;
                                    }
                                    Component.onCompleted: {
                                        contentItem.selectByMouse = true;
                                    }
                                    Layout.fillWidth: true
                                }
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Welding")

                            Button {
                                objectName: "scanfield-calibration-scanner-start"
                                //: Button to start the scanner calibration
                                text: qsTr("Start")
                                enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                onClicked: scanfieldCalibrationController.startScannerWeldingCalibration()
                                anchors.fill: parent
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Create Correction File")

                            Button {
                                objectName: "scanfield-calibration-scanner-start"
                                //: Button to start the scanner calibration
                                text: qsTr("Start")
                                enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                onClicked: scanfieldCalibrationController.startScannerCalibrationMeausure()
                                anchors.fill: parent
                            }
                        }
                    }

                    ColumnLayout {
                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Sample Distance [mm]")

                            GridLayout {
                                anchors.fill: parent

                                columns: 2

                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("\u0394 X:")
                                }
                                PrecitecApplication.DoubleSpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: 1000.0
                                    from: -1000.0
                                    stepSize: 5.0

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.deltaX

                                    onSetValue: {
                                        scanfieldCalibrationController.deltaX = newValue;
                                    }
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("\u0394 Y:")
                                }
                                PrecitecApplication.DoubleSpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: 1000.0
                                    from: -1000.0
                                    stepSize: 5.0

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.deltaY

                                    onSetValue: {
                                        scanfieldCalibrationController.deltaY = newValue;
                                    }
                                }
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Flip Scanfield image")

                            RowLayout {
                                anchors.fill: parent

                                CheckBox {
                                    Layout.fillWidth: true
                                    text: qsTr("X")
                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    checked: scanfieldCalibrationController.flipX
                                    onToggled: scanfieldCalibrationController.flipX = checked
                                }

                                CheckBox {
                                    Layout.fillWidth: true
                                    text: qsTr("Y")
                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    checked: scanfieldCalibrationController.flipY
                                    onToggled: scanfieldCalibrationController.flipY = checked
                                }
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("IDM Adaptive Exposure Mode:")

                            ColumnLayout {
                                anchors.fill: parent

                                CheckBox {
                                    Layout.fillWidth: true
                                    text: qsTr("Enabled")
                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    checked: scanfieldCalibrationController.adaptiveExposureMode
                                    onToggled: scanfieldCalibrationController.adaptiveExposureMode = checked
                                }
                                Label {
                                    Layout.fillWidth: true
                                    text: qsTr("Basic Value:")
                                }
                                SpinBox {
                                    Layout.maximumWidth: settingsPanel.spinBoxMaxSize

                                    to: 100
                                    from: 0
                                    stepSize: 5

                                    enabled: !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                    value: scanfieldCalibrationController.adaptiveExposureBasicValue

                                    onValueChanged: {
                                        scanfieldCalibrationController.adaptiveExposureBasicValue = value;
                                    }
                                }
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Preview ScanField Image")

                            Button {
                                anchors.fill: parent
                                text: qsTr("Start")
                                enabled: SystemConfiguration.Type_of_Sensor == TypeOfSensor.eCoax && !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                onClicked: scanfieldCalibrationController.startAcquireScanFieldImage()
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Acquire Depth Image")

                            Button {
                                anchors.fill: parent
                                text: qsTr("Start")
                                enabled: SystemConfiguration.IDM_Device1Enable && !scanfieldCalibrationController.updating && !scanfieldCalibrationController.calibrating
                                onClicked: scanfieldCalibrationController.computeDepthImage()
                            }
                        }

                        GroupBox {
                            Layout.fillWidth: true

                            title: qsTr("Debug Data")

                            ColumnLayout {
                                anchors.fill: parent
                                Button {
                                    Layout.fillWidth: true
                                    text: qsTr("Download")
                                    enabled: RemovableDevices.Service.udi != "" && RemovableDevices.Service.path != "" && !downloadService.backupInProgress
                                    onClicked: downloadService.performDownload(scanfieldCalibrationController.scanfieldDataDirInfo,
                                                                    "scanfieldData/" + Qt.formatDateTime(new Date(), "yyyyMMdd-HHmmss"),
                                                                    qsTr("Download Scanfield Debug Data to attached removable device"));
                                }
                                Button {
                                    Layout.fillWidth: true
                                    text: qsTr("Delete from disk")
                                    enabled: !downloadService.backupInProgress
                                    onClicked: scanfieldCalibrationController.deleteScanfieldData();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ScanfieldCalibrationController {
        id: scanfieldCalibrationController

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        calibrationDeviceProxy: HardwareModule.calibrationDeviceProxy
        grabberDeviceProxy: HardwareModule.grabberDeviceProxy
    }
}


