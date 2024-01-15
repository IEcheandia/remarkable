import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0

Control {
    id: ledCalibrationPage
    property alias screenshotTool: image.screenshotTool

    Connections {
        target: HardwareModule.systemStatus
        function onReturnedFromCalibration() {
            ledCalibrationController.endLEDCalibration();
        }
    }

    function pageBecameVisible()
    {
        ledCalibrationController.calibrationDevice = HardwareModule.calibrationDeviceProxy;
        ledCalibrationController.serviceDevice = HardwareModule.serviceDeviceProxy;
        ledCalibrationController.weldHeadDevice = HardwareModule.weldHeadDeviceProxy;
        ledCalibrationController.grabberDevice = HardwareModule.grabberDeviceProxy;
    }

    onVisibleChanged: {
        if (visible)
        {
            ledCalibrationPage.pageBecameVisible();
        } else
        {
            ledCalibrationController.calibrationDevice = null;
            ledCalibrationController.serviceDevice = null;
            ledCalibrationController.weldHeadDevice = null;
            ledCalibrationController.grabberDevice = null;
        }
    }

    Component.onCompleted: {
        if (visible)
        {
            ledCalibrationPage.pageBecameVisible();
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
            Layout.preferredWidth: parent.width * 0.8
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ListView {
                id: listView
                model: ledCalibrationController
                anchors.fill: parent
                spacing: 25

                onCurrentIndexChanged: positionViewAtIndex(currentIndex, ListView.Contain)

                delegate: ChannelBox {
                    number: index + 1
                    visible: model.visible
                    anchors {
                        left: parent.left
                        right: parent.right
                    }
                    channelEnabled: model.enabled
                    minCurrent: model.minCurrent
                    maxCurrent: model.maxCurrent
                    referenceBrightness: model.referenceBrightness
                    measuringResult: model.measuredBrightness
                    currentValue: model.currentValue
                    originalValue: model.originalValue
                    onUpdateEnabled: {
                        if (isEnabled)
                        {
                            listView.currentIndex = index;
                        }
                        ledCalibrationController.updateEnabled(index, isEnabled);
                    }
                    onUpdateCurrentValue: ledCalibrationController.updateCurrentValue(index, newValue)
                    onUpdateReferenceBrightness: ledCalibrationController.updateReferenceBrightness(index, newValue)
                }
            }
        }
    }

    LEDCalibrationController {
        id: ledCalibrationController

        active: ledCalibrationPage.visible
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
    }

    onHeightChanged: listView.positionViewAtIndex(listView.currentIndex, ListView.Contain)
}


