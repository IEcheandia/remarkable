import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.application 1.0 as PrecitecApplication

import Precitec.AppGui 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0



Item {
    property alias screenshotTool: image.screenshotTool
    property bool calibrationCoax: SystemConfiguration.Type_of_Sensor == SystemConfiguration.TypeOfSensor.Coax || SystemConfiguration.Type_of_Sensor == SystemConfiguration.TypeOfSensor.LED

    id: root

    onVisibleChanged: {
        if (visible)
        {
            cameraCalibrationModel.updateImageRoi();
        }
    }

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

    PrecitecImage.LineLaserFilterModel {
        id: lineLaserFilterModel

        sourceModel: cameraCalibrationModel
        lineLaser1Available: SystemConfiguration.LineLaser1Enable
        lineLaser2Available: SystemConfiguration.LineLaser2Enable
        lineLaser3Available: HardwareModule.lineLaser3Enabled
    }

    ButtonGroup {
        id: buttonGroup
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

            Rectangle {
                color: "white"
                opacity: 0.5
                border {
                    width: 2
                    color: PrecitecApplication.Settings.alternateBackground
                }
                visible: roiDragHandler.active
                x: image.x + image.paintedRect.x + cameraCalibrationModel.preview.x * image.zoom
                y: image.y + image.paintedRect.y + cameraCalibrationModel.preview.y * image.zoom
                width: cameraCalibrationModel.preview.width * image.zoom
                height: cameraCalibrationModel.preview.height * image.zoom

                Label {
                    anchors {
                        right: parent.right
                        bottom: parent.bottom
                        margins: 4
                    }
                    visible: width <= parent.width && height <= parent.height
                    font.pointSize: 8
                    font.bold: true
                    opacity: 1
                    text: ("%1, %2, %3x%4").arg(cameraCalibrationModel.preview.x).arg(cameraCalibrationModel.preview.y).arg(cameraCalibrationModel.preview.width).arg(cameraCalibrationModel.preview.height)
                }
            }

            Repeater {
                model: lineLaserFilterModel

                Rectangle {
                    color: "white"
                    opacity: laserLineList.currentIndex == index ? 0.5 : 0.2
                    border {
                        width: laserLineList.currentIndex == index ? 2 : 0
                        color: PrecitecApplication.Settings.alternateBackground
                    }
                    visible: model.checked && !cameraCalibrationModel.calibrating && UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig) && !(laserLineList.currentIndex == index && roiDragHandler.active)
                    x: image.x + image.paintedRect.x + model.x * image.zoom
                    y: image.y + image.paintedRect.y + model.y * image.zoom
                    width: model.width * image.zoom
                    height: model.height * image.zoom
                }
            }

            DragHandler {
                id: roiDragHandler

                property point start
                enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig) && !cameraCalibrationModel.calibrating
                target: null
                minimumPointCount: 1
                maximumPointCount: 1
                onActiveChanged: {
                    if (active)
                    {
                        start = Qt.point((centroid.pressPosition.x - image.x - image.paintedRect.x) / image.zoom , (centroid.pressPosition.y - image.y - image.paintedRect.y) / image.zoom)
                    } else {
                        cameraCalibrationModel.setPreviewToRoi(lineLaserFilterModel.mapToSource(lineLaserFilterModel.index(laserLineList.currentIndex, 0)).row);
                    }
                }
                onTranslationChanged: {
                    cameraCalibrationModel.preview = Qt.rect(start.x, start.y, translation.x / image.zoom, translation.y / image.zoom);
                }
            }
        }

        ColumnLayout
        {
            Layout.fillHeight: true
            Layout.topMargin: 5

            ColumnLayout {
                Layout.fillWidth: true
                Layout.leftMargin: 5
                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)

                Label {
                    Layout.fillWidth: true
                    text: qsTr("Target Width [mm]:")
                }
                TextField {
                    selectByMouse: true
                    validator: DoubleValidator {
                        bottom: 0.0
                        top: 10.0
                    }
                    palette.text: acceptableInput ? "black" : "red"
                    placeholderText: "0"
                    text: Number(cameraCalibrationModel.targetWidth).toLocaleString(locale, "f", 3)
                    onEditingFinished: cameraCalibrationModel.targetWidth = Number.fromLocaleString(locale, text)
                    enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig)
                }
                Label {
                    Layout.fillWidth: true
                    text: qsTr("Target Height [mm]:")
                }
                TextField {
                    selectByMouse: true
                    validator: DoubleValidator {
                        bottom: 0.0
                        top: 10.0
                    }
                    palette.text: acceptableInput ? "black" : "red"
                    placeholderText: "0"
                    text: Number(cameraCalibrationModel.targetHeight).toLocaleString(locale, "f", 3)
                    onEditingFinished: cameraCalibrationModel.targetHeight = Number.fromLocaleString(locale, text)
                    enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig)
                }
            }
            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true

                id: laserLineList

                enabled: !cameraCalibrationModel.calibrating
                clip: true

                ScrollBar.vertical: ScrollBar {
                    id: bar
                }

                model: lineLaserFilterModel

                delegate: ItemDelegate {

                    width: ListView.view.width - bar.width
                    height: layout.implicitHeight + 20

                    function selectItem() {
                        radioButton.checked = true;
                        laserLineList.currentIndex = index;
                    }

                    id: delegate

                    onClicked: selectItem()

                    Component.onCompleted: {
                        if (index === 0)
                        {
                            selectItem();
                        }
                    }

                    ColumnLayout {
                        anchors {
                            fill: parent
                            margins: 5
                        }

                        id: layout

                        RadioButton {
                            Layout.fillWidth: true

                            id: radioButton

                            font.bold: true
                            text: model.name
                            ButtonGroup.group: buttonGroup

                            onClicked: selectItem()
                        }

                        GridLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 5

                            columns: 2

                            opacity: radioButton.checked ? 1 : 0.5

                            Label {
                                Layout.fillWidth: true
                                text: qsTr("X:")
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                            }

                            SpinBox {
                                from: 0
                                to: cameraCalibrationModel.imageRoi.width - model.width
                                editable: true
                                value: model.x
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                                enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig)
                                onValueModified: {
                                    selectItem();
                                    model.x = value;
                                }

                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                text: qsTr("Y:")
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                            }

                            SpinBox {
                                from: 0
                                to: cameraCalibrationModel.imageRoi.height - model.height
                                editable: true
                                value: model.y
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                                enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig)
                                onValueModified: {
                                    selectItem();
                                    model.y = value;
                                }
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                text: qsTr("Width:")
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                            }

                            SpinBox {
                                from: 1
                                to: cameraCalibrationModel.imageRoi.width - model.x
                                editable: true
                                value: model.width
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                                enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig)
                                onValueModified: {
                                    selectItem();
                                    model.width = value;
                                }
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                text: qsTr("Height:")
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                            }

                            SpinBox {
                                from: 1
                                to: cameraCalibrationModel.imageRoi.height - model.y
                                editable: true
                                value: model.height
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                                enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig)
                                onValueModified: {
                                    selectItem();
                                    model.height = value;
                                }
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }

                            Label {
                                Layout.fillWidth: true
                                text: qsTr("Intensity [%]:")
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewWorkflowDeviceConfig)
                            }

                            SpinBox {
                                from: 0
                                to: 100
                                editable: true
                                value: model.intensity
                                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewWorkflowDeviceConfig)
                                enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditWorkflowDeviceConfig)
                                onValueModified: {
                                    selectItem();
                                    model.intensity = value;
                                }
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }
                        }

                        RowLayout {

                            visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewWorkflowDeviceConfig)

                            Label {
                                text: "0"
                                opacity: radioButton.checked ? 1 : 0.5
                            }

                            Slider {
                                Layout.fillWidth: true
                                from: 0
                                to: 100
                                stepSize: 1
                                value: model.intensity
                                enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditWorkflowDeviceConfig)
                                background.opacity: radioButton.checked ? 1 : 0.5
                                onMoved: {
                                    selectItem();
                                    model.intensity = value;
                                }
                            }

                            Label {
                                text: "100"
                                opacity: radioButton.checked ? 1 : 0.5
                            }
                        }

                        CheckBox {
                            Layout.fillWidth: true
                            text: qsTr("Show ROI")
                            checked: true
                            opacity: radioButton.checked ? 1 : 0.5
                            visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                            enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig)
                            onClicked: {
                                selectItem();
                                model.checked = checked;
                            }
                        }

                        Button {
                            Layout.fillWidth: true
                            text: qsTr("Start Line Calibration")
                            opacity: radioButton.checked ? 1 : 0.5
                            visible: calibrationCoax && UserManagement.currentUser && UserManagement.hasPermission(App.ViewCalibrationDeviceConfig)
                            enabled: UserManagement.currentUser && UserManagement.hasPermission(App.EditCalibrationDeviceConfig)
                            onClicked: {
                                selectItem();
                                cameraCalibrationModel.startCalibration(lineLaserFilterModel.mapToSource(lineLaserFilterModel.index(index, 0)).row);
                            }
                        }
                    }
                }
            }
        }
    }
}
