import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.logging 1.0 as Logging
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui 1.0
import precitec.gui.general 1.0

Control {
    id: scanTrackerPage
    property var logModel: null
    property alias screenshotTool: image.screenshotTool

    function pageBecameVisible()
    {
        trackerLogModel.startDate = new Date();
        liveModeEnableTimer.restart();
        controller.weldHeadDevice = HardwareModule.weldHeadDeviceProxy;
    }

    ScanTrackerInformation {
        id: information
        pollHeadInfo: scanTrackerPage.visible
        weldHeadServer: HardwareModule.weldHeadServer
        weldHeadSubscribeProxy: HardwareModule.weldHeadSubscribeProxy
    }

    Logging.LogFilterModel {
        id: trackerLogModel
        property var startDate: new Date()
        sourceModel: scanTrackerPage.visible ? scanTrackerPage.logModel : null
        moduleNameFilter: "VIWeldHeadControl"
        includeInfo: false
        includeWarning: false
        includeError: false
        includeTracker: true
        onRowsInserted: {
            for (var i = first; i <= last; i++)
            {
                if (trackerLogModel.startDate > trackerLogModel.data(trackerLogModel.index(i, 0), Qt.UserRole))
                {
                    continue;
                }
                Notifications.NotificationSystem.information(trackerLogModel.data(trackerLogModel.index(i, 0)), "wizard-scantracker");
            }
        }
    }

    onVisibleChanged: {
        if (visible)
        {
            scanTrackerPage.pageBecameVisible();
        } else
        {
            liveModeEnableTimer.stop();
            controller.liveMode = false;
            controller.weldHeadDevice = null;
        }
    }

    Component.onCompleted: {
        if (visible)
        {
            scanTrackerPage.pageBecameVisible();
        }
    }

    ScanTrackerController {
        id: controller

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        productModel: HardwareModule.productModel
        systemStatus: HardwareModule.systemStatus
        deviceNotificationServer: HardwareModule.deviceNotificationServer
        grabberDeviceProxy: SystemConfiguration.CameraInterfaceType == 1 ? HardwareModule.grabberDeviceProxy : null
    }

    Timer {
        id: liveModeEnableTimer
        interval: 500
        repeat: false
        onTriggered: {
            controller.liveMode = true;
        }
    }

    RowLayout {
        anchors {
            fill: parent
            margins: spacing
        }

        Item {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.8
            Layout.minimumWidth: parent.width * 0.25
            PrecitecImage.Image {
                id: image
                anchors.fill: parent
                opacity: busyIndicator.running ? 0.5 : 1.0
            }
            BusyIndicator {
                id: busyIndicator
                anchors.centerIn: parent
                running: false
            }
        }
        ColumnLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            enabled: HardwareModule.systemStatus.state == SystemStatusServer.Live
            CheckBox {
                Layout.fillWidth: true
                id: manualModeCheckBox
                checked: information.expertMode
                text: qsTr("Manual mode")
                onToggled: controller.setExpertMode(!information.expertMode)
            }
            CheckBox {
                Layout.fillWidth: true
                enabled: information.expertMode
                text: qsTr("Scan Tracker driver enabled")
                checked: controller.driverEnabled
                onToggled: controller.toggleDriverEnabled()
            }
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Input")
                GridLayout {
                    anchors.fill: parent
                    columns: 2
                    enabled: information.expertMode
                    Label {
                        text: qsTr("Frequency:")
                    }
                    ComboBox {
                        model: controller.frequencyModel
                        textRole: "display"
                        onActivated: controller.setFrequencyIndex(index)
                        currentIndex: controller.frequencyIndex
                        Layout.fillWidth: true
                    }
                    Label {
                        text: qsTr("Fix position (µm):")
                    }
                    TextField {
                        selectByMouse: true
                        palette.text: acceptableInput ? "black" : "red"
                        placeholderText: "0"
                        onEditingFinished: controller.setScanPosFixed(Number.fromLocaleString(locale, text))
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                    }
                    Label {
                        text: qsTr("Fix scan width (µm):")
                    }
                    TextField {
                        selectByMouse: true
                        validator: IntValidator {
                            bottom: 0
                        }
                        palette.text: acceptableInput ? "black" : "red"
                        placeholderText: "0"
                        onEditingFinished: controller.setScanWidthFixed(Number.fromLocaleString(locale, text))
                        Layout.fillWidth: true
                        Layout.minimumWidth: 0
                    }
                }
            }
            GroupBox {
                Layout.fillWidth: true
                title: qsTr("Output")
                GridLayout {
                    columns: 2
                    anchors.fill: parent
                    Label {
                        text: qsTr("Scan width limited")
                        visible: information.scanWidthLimited
                        Layout.columnSpan: 2
                    }
                    Label {
                        text: qsTr("Scan position limited")
                        visible: information.scanPosLimited
                        Layout.columnSpan: 2
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Scan position:")
                    }
                    Label {
                        text: "%1 µm / %2 V".arg(information.scanPosUm).arg(information.scanPosVolt)
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Scan width:")
                    }
                    Label {
                        text: "%1 µm / %2 V".arg(information.scanWidthUm).arg(information.scanWidthVolt)
                    }
                }
            }
            GroupBox {
                title: "Query Scan Tracker"
                ColumnLayout {
                    anchors.fill: parent
                    Button {
                        text: qsTr("Status")
                        onClicked: controller.queryStatus()
                        Layout.fillWidth: true
                    }
                    Button {
                        text: qsTr("Version")
                        onClicked: controller.queryVersion()
                        Layout.fillWidth: true
                    }
                    Button {
                        text: qsTr("Serial number")
                        onClicked: controller.querySerialNumber()
                        Layout.fillWidth: true
                    }
                }
            }
            Item {
                Layout.fillHeight: true
            }
        }
    }
}
