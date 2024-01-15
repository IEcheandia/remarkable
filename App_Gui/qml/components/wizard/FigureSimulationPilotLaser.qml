import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

import wobbleFigureEditor.components 1.0 as WobbleFigureEditor

GroupBox {
    //: title of a GroupBox
    title: qsTr("Simulate Figure with pilot laser")
    property alias dialogVisible: previewDialog.visible

    onVisibleChanged: {
        if (!visible)
        {
            controller.stop();
        }
    }

    FigureSimulationPilotLaserController {
        id: controller
        weldheadDeviceProxy: HardwareModule.weldHeadDeviceProxy
    }

    WobbleFigureEditor.FileModel {
        id: fileModel

        Component.onCompleted: fileModel.loadFiles()
    }

    WobbleFigureEditor.PreviewDialog {
        id: previewDialog
        fileModel: fileModel
    }

    ColumnLayout {
        anchors.fill: parent
        Label {
            //: title for a combo box to select a figure file
            text: qsTr("Seam figure:")
        }
        WobbleFigureEditor.FigureSelector {
            objectName: "figure-simulation-pilot-laser-seam"
            fileModel: fileModel
            fileType: WobbleFigureEditor.FileType.Seam
            previewDialog: previewDialog

            onFileModelIndexSelected: {
                    controller.seamId = fileModel.data(modelIndex, Qt.UserRole + 3);
            }
            Layout.fillWidth: true
        }
        GroupBox {
            Layout.fillWidth: true
            // TODO: enable once endless pilot laser simulation wobble works with MarkingEngine
            enabled: SystemConfiguration.Scanner2DController == SystemConfiguration.Scanlab
            label: CheckBox {
                id: overlayWithWobbleFigure
                objectName: "figure-simulation-pilot-laser-wobble-check"
                //: title of a check box
                text: qsTr("Overlay with wobble figure")
                checked: controller.wobble
                onToggled: {
                    controller.wobble = !controller.wobble;
                }
            }
            WobbleFigureEditor.FigureSelector {
                objectName: "figure-simulation-pilot-laser-wobble"
                fileModel: fileModel
                fileType: WobbleFigureEditor.FileType.Wobble
                previewDialog: previewDialog
                enabled:overlayWithWobbleFigure.checked
                width: parent.width

                onFileModelIndexSelected: {
                        controller.wobbleId = fileModel.data(modelIndex, Qt.UserRole + 3);
                }
            }
        }
        Label {
            text: qsTr("Velocity (m/s):")
        }
        TextField {
            id: velocity
            objectName: "figure-simulation-pilot-laser-velocity"
            text: Number(controller.velocity).toLocaleString(locale, "f", 2)
            validator: DoubleValidator {
                bottom: 0.01
                top: 100000.0
            }
            palette.text: velocity.acceptableInput ? "black" : "red"
            onAccepted: {
                controller.velocity = Number.fromLocaleString(locale, velocity.text);
            }

            Layout.fillWidth: true
        }
        DialogButtonBox {
            Button {
                objectName: "figure-simulation-pilot-laser-start"
                icon.name: "media-playback-start"
                text: qsTr("Start")
                enabled: controller.valid && !controller.running
                onClicked: controller.start()
            }
            Button {
                objectName: "figure-simulation-pilot-laser-stop"
                icon.name: "media-playback-stop"
                text: qsTr("Stop")
                enabled: controller.running
                onClicked: controller.stop()
            }

            Layout.fillWidth: true
        }
    }
}
