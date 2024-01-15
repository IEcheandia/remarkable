import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

import Precitec.AppGui 1.0

Item {
    id: hardwareRoiPage
    property alias screenshotTool: image.screenshotTool

    enabled: hardwareRoiController.ready && !hardwareRoiController.updating

    function pageBecameVisible()
    {
        liveModeEnableTimer.restart();

        hardwareRoiController.grabberDeviceProxy = HardwareModule.grabberDeviceProxy;
        fieldIlluminationModel.deviceProxy = HardwareModule.weldHeadDeviceProxy;
        lineLaserModel.deviceProxy = HardwareModule.weldHeadDeviceProxy;
    }

    onVisibleChanged: {
        if (visible)
        {
            hardwareRoiPage.pageBecameVisible()
        } else
        {
            liveModeEnableTimer.stop();
            hardwareRoiController.liveMode = false;
            hardwareRoiController.grabberDeviceProxy = null;
            lineLaserModel.deviceProxy = null;
            fieldIlluminationModel.deviceProxy = null;
        }
    }
    Component.onCompleted: {
        if (visible)
        {
            hardwareRoiPage.pageBecameVisible();
        }
    }

    LineLaserModel {
        id: lineLaserModel
        notificationServer: HardwareModule.deviceNotificationServer
    }

    HardwareRoiController {
        id: hardwareRoiController

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        productModel: HardwareModule.productModel
        deviceNotificationServer: HardwareModule.deviceNotificationServer
        systemStatus: HardwareModule.systemStatus
    }

    Timer {
        id: liveModeEnableTimer
        interval: 500
        repeat: false
        onTriggered: {
            hardwareRoiController.liveMode = true;
        }
    }

    FieldIlluminationModel {
        id: fieldIlluminationModel
        notificationServer: HardwareModule.deviceNotificationServer
    }

    RowLayout {
        anchors {
            fill: parent
            margins: spacing
        }
        PrecitecImage.Image {
            id: image
            clip: true
            opacity: hardwareRoiController.updating ? 0.5 : 1.0
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.8
            Layout.minimumWidth: parent.width * 0.25

            Rectangle {
                color: Qt.rgba(1.0, 1.0, 1.0, 0.5)
                border {
                    width: 1
                    color: PrecitecApplication.Settings.alternateBackground
                }
                visible: xSpinBox.value != hardwareRoiController.roi.x ||
                         ySpinBox.value != hardwareRoiController.roi.y ||
                         widthSpinBox.value != hardwareRoiController.roi.width ||
                         heightSpinBox.value != hardwareRoiController.roi.height
                x: image.x + image.paintedRect.x + (xSpinBox.value - hardwareRoiController.roi.x) * image.zoom
                y: image.y + image.paintedRect.y + (ySpinBox.value - hardwareRoiController.roi.y) * image.zoom
                width: widthSpinBox.value * image.zoom
                height: heightSpinBox.value * image.zoom
            }
        }
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            ToolButton {
                id: saveButton
                enabled: hardwareRoiController.readyToPersist && HardwareModule.systemStatus.state == SystemStatusServer.Live
                icon.name: "document-save"
                icon.width: 64
                icon.height: 64
                display: AbstractButton.IconOnly
                onClicked: {
                    hardwareRoiController.persistToCamera();
                }
                ToolTip.visible: hovered
                ToolTip.text: qsTr("Write camera parameters to persitent  internal memory.")
                Layout.alignment: Qt.AlignRight
            }
            PrecitecApplication.ActiveFocusControlAwareFlickable {
                Layout.fillHeight: true
                Layout.fillWidth: true
                flickableDirection: Flickable.VerticalFlick
                ColumnLayout {
                    anchors.fill: parent
                    enabled: HardwareModule.systemStatus.state == SystemStatusServer.Live
                    GroupBox {
                        Layout.minimumWidth: Math.max(Layout.minimumWidth, Math.max(laserControl.width, ledControl.width))
                        Layout.fillWidth: true
                        id: hardwareRoiGroup
                        title: qsTr("Hardware ROI [pixel]")
                        property rect currentRect: Qt.rect(xSpinBox.value, ySpinBox.value, widthSpinBox.value, heightSpinBox.value)

                        GridLayout {
                            anchors.fill: parent
                            columns: 2
                            Label {
                                text: qsTr("X:")
                                Layout.fillWidth: true
                            }
                            SpinBox {
                                id: xSpinBox
                                from: 0
                                to: widthSpinBox.to - widthSpinBox.from - 1
                                value: hardwareRoiController.roi.x
                                stepSize: 1
                                editable: true
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }

                            Label {
                                text: qsTr("Y:")
                                Layout.fillWidth: true
                            }
                            SpinBox {
                                id: ySpinBox
                                from: 0
                                to: heightSpinBox.to - heightSpinBox.from - 1
                                value: hardwareRoiController.roi.y
                                stepSize: 1
                                editable: true
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }

                            Label {
                                text: qsTr("Width:")
                                Layout.fillWidth: true
                            }
                            SpinBox {
                                id: widthSpinBox
                                from: 4
                                to: 1024
                                value: hardwareRoiController.roi.width
                                stepSize: 1
                                editable: true
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }

                            Label {
                                text: qsTr("Height:")
                                Layout.fillWidth: true
                            }
                            SpinBox {
                                id: heightSpinBox
                                from: 4
                                to: 1024
                                value: hardwareRoiController.roi.height
                                stepSize: 1
                                editable: true
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }
                            RowLayout {
                                Layout.columnSpan: 2
                                Button {
                                    Layout.fillWidth: true
                                    text: qsTr("Fullscreen")
                                    onClicked: {
                                        hardwareRoiController.updateToFullFrame();
                                    }
                                }
                                Button {
                                    Layout.fillWidth: true
                                    text: qsTr("Update Roi")
                                    enabled: hardwareRoiController.isRectValid(hardwareRoiGroup.currentRect)
                                    onClicked: {
                                        hardwareRoiController.updateCameraGeometry(hardwareRoiGroup.currentRect);
                                    }
                                }
                            }
                        }
                    }
                    GroupBox {
                        title: qsTr("Exposure Time")
                        RowLayout {
                            TextField {
                                id: exposureTimeSpinBox
                                selectByMouse: true
                                validator: DoubleValidator {
                                    bottom: 0.001
                                    top: 95.0
                                }
                                text: Number(hardwareRoiController.exposureTime).toLocaleString(locale, 'f', 3)
                                onEditingFinished: {
                                    hardwareRoiController.updateExposureTime(Number.fromLocaleString(locale, text));
                                }
                                palette.text: exposureTimeSpinBox.acceptableInput ? "black" : "red"
                            }
                            Label {
                                text: qsTr("ms")
                            }
                        }
                    }
                    GroupBox {
                        title: qsTr("Brightness")
                        visible: GuiConfiguration.configureBlackLevelOffsetVoltagesOnCamera
                        RowLayout {
                            SpinBox {
                                id: brightnessSpinBox
                                from: 0
                                to: 4000
                                stepSize: 1
                                value: hardwareRoiController.brightness
                                editable: true
                                onValueModified: {
                                    hardwareRoiController.updateBrightness(value);
                                }
                                Component.onCompleted: {
                                    contentItem.selectByMouse = true;
                                }
                            }
                        }
                    }
                    GridLayout {
                        columns: 2
                        visible: GuiConfiguration.configureLinLogOnCamera
                        GroupBox {
                            Layout.columnSpan: 2
                            Layout.fillWidth: true
                            title: qsTr("LinLog Mode")

                            ComboBox {
                                anchors.fill: parent
                                id: linLogModeComboBox
                                model: [ "Off", "Low Compression", "Normal Compression", "High Compression", "User Defined" ]
                                currentIndex: hardwareRoiController.linLogMode
                                onCurrentIndexChanged: {
                                    hardwareRoiController.updateLinLogMode(currentIndex);
                                }
                            }

                        }
                        GroupBox {
                            title: qsTr("LinLog Value 1")
                            enabled: linLogModeComboBox.currentIndex == 4
                            visible: linLogModeComboBox.currentIndex == 4
                            RowLayout {
                                SpinBox {
                                    id: linLogValue1SpinBox
                                    from: hardwareRoiController.linLogValue2
                                    to: 30
                                    stepSize: 1
                                    value: hardwareRoiController.linLogValue1
                                    editable: true
                                    onValueModified: {
                                        hardwareRoiController.updateLinLogValue1(value);
                                    }
                                    Component.onCompleted: {
                                        contentItem.selectByMouse = true;
                                    }
                                }
                            }
                        }
                        GroupBox {
                            title: qsTr("LinLog Value 2")
                            enabled: linLogModeComboBox.currentIndex == 4
                            visible: linLogModeComboBox.currentIndex == 4
                            RowLayout {
                                SpinBox {
                                    id: linLogValue2SpinBox
                                    from: 0
                                    to: hardwareRoiController.linLogValue1
                                    stepSize: 1
                                    value: hardwareRoiController.linLogValue2
                                    editable: true
                                    onValueModified: {
                                        hardwareRoiController.updateLinLogValue2(value);
                                    }
                                    Component.onCompleted: {
                                        contentItem.selectByMouse = true;
                                    }
                                }
                            }
                        }
                        GroupBox {
                            title: qsTr("LinLog Time 1")
                            enabled: linLogModeComboBox.currentIndex == 4
                            visible: linLogModeComboBox.currentIndex == 4
                            RowLayout {
                                SpinBox {
                                    id: linLogTime1SpinBox
                                    from: 0
                                    to: hardwareRoiController.linLogTime2
                                    stepSize: 1
                                    value: hardwareRoiController.linLogTime1
                                    editable: true
                                    onValueModified: {
                                        hardwareRoiController.updateLinLogTime1(value);
                                    }
                                    Component.onCompleted: {
                                        contentItem.selectByMouse = true;
                                    }
                                }
                            }
                        }
                        GroupBox {
                            title: qsTr("LinLog Time 2")
                            enabled: linLogModeComboBox.currentIndex == 4
                            visible: linLogModeComboBox.currentIndex == 4
                            RowLayout {
                                SpinBox {
                                    id: linLogTime2SpinBox
                                    from: hardwareRoiController.linLogTime1
                                    to: 1000
                                    stepSize: 1
                                    value: hardwareRoiController.linLogTime2
                                    editable: true
                                    onValueModified: {
                                        hardwareRoiController.updateLinLogTime2(value);
                                    }
                                    Component.onCompleted: {
                                        contentItem.selectByMouse = true;
                                    }
                                }
                            }
                        }
                    }
                    LineLaserControl {
                        id: laserControl
                        lineLaserModel: lineLaserModel
                    }
                    LedIlluminationControl {
                        id: ledControl
                        fieldIlluminationModel: fieldIlluminationModel
                    }
                }
            }
        }
    }
    BusyIndicator {
        anchors.centerIn: parent
        running: hardwareRoiController.updating
    }
}

