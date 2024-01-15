import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.userManagement 1.0
import precitec.gui.general 1.0
import precitec.gui.filterparametereditor 1.0

import Precitec.AppGui 1.0
import precitec.gui 1.0

Dialog {
    id: referenceImagePage

    property alias currentSeam: referenceImageController.currentSeam
    property alias attributeModel: referenceImageController.attributeModel
    property alias screenshotTool: image.screenshotTool

    ReferenceImageController {
        id: referenceImageController
        referenceImageDir: WeldmasterPaths.referenceImageDir
    }

    HardwareParameterFilterModel {
        id: hardwareParameterFilterModel
        sourceModel: referenceImageController
        deviceProxy: HardwareModule.serviceDeviceProxy
        weldHeadDeviceProxy: HardwareModule.weldHeadDeviceProxy
        filterKeys:[]
    }

    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            referenceImagePage.reject()
        }
    }

    ScrollBarSynchController {
        id: scrollBarController
    }

    GridLayout {
        anchors {
            fill: parent
            margins: spacing
        }

        columns: 4

        PrecitecImage.Image {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.35

            id: image
            clip: true

            menuVisible: false
            overlaysEnabled: false
            handlersEnabled: false
            simulation: true
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.15

            Label {
                text: qsTr("Seam Parameters")
                font.bold: true
            }

            ListView {
                id: seamParameterList

                Layout.fillHeight: true
                Layout.fillWidth: true

                clip: true
                model: hardwareParameterFilterModel

                ScrollBar.vertical: ScrollBar {
                    position: scrollBarController.position
                }

                visibleArea {
                    onYPositionChanged: scrollBarController.setPosition(yPosition)
                }

                delegate: GroupBox {

                    width: seamParameterList.width
                    enabled: false

                    label: CheckBox {
                        id: seamGroupCheckBox
                        width: parent.width
                        checked: model.seamEnabled
                        text: model.display
                    }

                    ParameterEditor {
                        attribute: model.attribute
                        parameter: model.seamParameter
                        milliFromMicro: model.milliFromMicro
                    }
                }
            }
        }

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.35
            color: "white"
            border {
                width: 1
                color: PrecitecApplication.Settings.alternateBackground
            }
            Image {
                id: referenceImage
                anchors {
                    fill: parent
                }
                source: referenceImageController.imageFilePath
                fillMode: Image.PreserveAspectFit
                asynchronous: true
            }
            Label {
                anchors.centerIn: parent
                text: qsTr("No image")
                visible: referenceImage.status == Image.Error || image.status == Image.Null
            }
        }

        ColumnLayout {
            Layout.fillHeight: true
            Layout.preferredWidth: parent.width * 0.15

            Label {
                text: qsTr("Reference Parameters")
                font.bold: true
            }

            ListView {
                id: referenceParameterList

                Layout.fillHeight: true
                Layout.fillWidth: true

                clip: true
                model: hardwareParameterFilterModel

                ScrollBar.vertical: ScrollBar {
                    position: scrollBarController.position
                }

                visibleArea {
                    onYPositionChanged: scrollBarController.setPosition(yPosition)
                }

                delegate: GroupBox {

                    width: referenceParameterList.width
                    enabled: false

                    label: CheckBox {
                        id: referenceGroupCheckBox
                        width: parent.width
                        checked: model.referenceEnabled
                        text: model.display
                    }

                    ParameterEditor {
                        attribute: model.attribute
                        parameter: model.referenceParameter
                        milliFromMicro: model.milliFromMicro
                    }
                }
            }
        }

        RowLayout {
            Layout.columnSpan: 4
            Layout.alignment: Qt.AlignHCenter
            Button {
                text: qsTr("Save Camera Image as Reference")
                onClicked: {
                    image.save(referenceImageController.imagePath);
                    referenceImageController.saveHardwareParameters();
                }
                visible: UserManagement.currentUser && UserManagement.hasPermission(App.EditReferenceImage)
            }
            Button {
                text: qsTr("Close")
                onClicked: referenceImagePage.reject()
            }
        }
    }
}
