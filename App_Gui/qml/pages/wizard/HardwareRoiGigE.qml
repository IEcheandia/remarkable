import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0
import Precitec.AppGui 1.0
import precitec.gui.filterparametereditor 1.0

Item {
    id: hardwareRoiPage
    property bool souvis6000: false
    property alias screenshotTool: image.screenshotTool
    property alias keyValueAttributeModel: hardwareRoiController.attributeModel
    property bool liquidLensAvailable: SystemConfiguration.LiquidLensControllerEnable

    enabled: hardwareRoiController.ready

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
            hardwareRoiPage.pageBecameVisible();
        } else
        {
            liveModeEnableTimer.stop();
            hardwareRoiController.liveMode = false;
            hardwareRoiController.grabberDeviceProxy = null;
            lineLaserModel.deviceProxy = null;
            fieldIlluminationModel.deviceProxy = null;

            if (pingDialog.visible)
            {
                pingDialog.close()
            }
        }
    }

    Dialog {
        id: pingDialog
        parent: Overlay.overlay
        anchors.centerIn: parent
        title: qsTr("Test network connection")
        width: hardwareRoiPage.width * 0.75
        height: hardwareRoiPage.height * 0.75
        // workaround for https://bugreports.qt.io/browse/QTBUG-72372
        footer: DialogButtonBox {
            alignment: Qt.AlignRight
            Button {
                text: qsTr("Close")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
        }

        onAboutToShow: hardwareRoiController.startPingCamera()
        onAboutToHide: hardwareRoiController.stopPingCamera()

        ScrollView {
            anchors.fill: parent
            Label {
                text: hardwareRoiController.pingOutput
            }
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

    HardwareRoiGigEController {
        id: hardwareRoiController

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        productModel: HardwareModule.productModel
        deviceNotificationServer: HardwareModule.deviceNotificationServer
        systemStatus: HardwareModule.systemStatus
    }

    Binding {
        target: hardwareRoiController
        property: "weldHeadDeviceProxy"
        when: hardwareRoiPage.liquidLensAvailable
        value: HardwareModule.weldHeadDeviceProxy
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
        anchors.fill: parent
        PrecitecImage.Image {
            id: image
            clip: true
            opacity: hardwareRoiController.updating ? 0.5 : 1.0
            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: parent.width * 0.25
        }
        ColumnLayout {
            Layout.fillHeight: true
            PrecitecApplication.ActiveFocusControlAwareFlickable {
                Layout.fillHeight: true
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
                        property bool acceptableInput: xSpinBox.acceptableInput && ySpinBox.acceptableInput && widthSpinBox.acceptableInput && heightSpinBox.acceptableInput

                        GridLayout {
                            anchors.fill: parent
                            columns: 2
                            Label {
                                text: qsTr("X:")
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: xSpinBox
                                property int value: Number.fromLocaleString(locale, text)
                                selectByMouse: true
                                text: hardwareRoiController.roi.x
                                validator: DivideBy16Validator {
                                    bottom: 0
                                    top: hardwareRoiController.maxSize.width - widthSpinBox.value
                                }
                                palette.text: xSpinBox.acceptableInput ? "black" : "red"
                                onAccepted: {
                                    if (!hardwareRoiGroup.acceptableInput)
                                    {
                                        return;
                                    }
                                    hardwareRoiController.updateCameraGeometry(hardwareRoiGroup.currentRect);
                                }
                            }

                            Label {
                                text: qsTr("Y:")
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: ySpinBox
                                property int value: Number.fromLocaleString(locale, text)
                                selectByMouse: true
                                text: hardwareRoiController.roi.y
                                validator: IntValidator {
                                    bottom: 0
                                    top: hardwareRoiController.maxSize.height - heightSpinBox.value
                                }
                                palette.text: ySpinBox.acceptableInput ? "black" : "red"
                                onAccepted: {
                                    if (!hardwareRoiGroup.acceptableInput)
                                    {
                                        return;
                                    }
                                    hardwareRoiController.updateCameraGeometry(hardwareRoiGroup.currentRect);
                                }
                            }

                            Label {
                                text: qsTr("Width:")
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: widthSpinBox
                                property int value: Number.fromLocaleString(locale, text)
                                selectByMouse: true
                                text: hardwareRoiController.roi.width
                                validator: DivideBy16Validator {
                                    bottom: 16
                                    top: hardwareRoiController.maxSize.width - xSpinBox.value
                                }
                                palette.text: widthSpinBox.acceptableInput ? "black" : "red"
                                onAccepted: {
                                    if (!hardwareRoiGroup.acceptableInput)
                                    {
                                        return;
                                    }
                                    hardwareRoiController.updateCameraGeometry(hardwareRoiGroup.currentRect);
                                }
                            }

                            Label {
                                text: qsTr("Height:")
                                Layout.fillWidth: true
                            }
                            TextField {
                                id: heightSpinBox
                                property int value: Number.fromLocaleString(locale, text)
                                selectByMouse: true
                                text: hardwareRoiController.roi.height
                                validator: IntValidator {
                                    bottom: 16
                                    top: hardwareRoiController.maxSize.height - ySpinBox.value
                                }
                                palette.text: heightSpinBox.acceptableInput ? "black" : "red"
                                onAccepted: {
                                    if (!hardwareRoiGroup.acceptableInput)
                                    {
                                        return;
                                    }
                                    hardwareRoiController.updateCameraGeometry(hardwareRoiGroup.currentRect);
                                }
                            }
                            Button {
                                Layout.columnSpan: 2
                                Layout.fillWidth: true
                                text: qsTr("Fullscreen")
                                onClicked: {
                                    hardwareRoiController.updateToFullFrame();
                                }
                            }
                            Button {
                                Layout.columnSpan: 2
                                Layout.fillWidth: true
                                text: qsTr("Test Network Connection")
                                onClicked: pingDialog.open()
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
                        title: hardwareRoiController.liquidLensPositionLabel
                        visible: hardwareRoiPage.liquidLensAvailable
                        ParameterEditor {
                            attribute: hardwareRoiController.liquidLensPositionAttribute
                            parameter: hardwareRoiController.liquidLensPositionParameter
                            defaultValue: attribute ? attribute.defaultValue : 0.0
                            onParameterValueModified: hardwareRoiController.updateLiquidLensPosition(parameterValue)
                        }
                    }
                    GroupBox {
                        title: qsTr("Brightness")
                        visible: hardwareRoiPage.souvis6000
                        enabled: hardwareRoiPage.souvis6000
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
                        visible: hardwareRoiPage.souvis6000
                        enabled: hardwareRoiPage.souvis6000
                        GroupBox {
                            title: qsTr("LinLog Value 1")
                            RowLayout {
                                SpinBox {
                                    id: linLogValue1SpinBox
                                    from: 0
                                    to: hardwareRoiController.linLogValue2
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
                            RowLayout {
                                SpinBox {
                                    id: linLogValue2SpinBox
                                    from: hardwareRoiController.linLogValue1
                                    to: 30
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
