import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0

Item {
    id: focusPosition

    LaserControlDelayController {
        id: controller
        weldHeadDevice: HardwareModule.weldHeadDeviceProxy
        visible: focusPosition.visible
    }

    ColumnLayout {
        anchors {
            fill: parent
            margins: 10
        }
        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.preferredWidth: 2 * (height * sourceSize.width) / sourceSize.height
            id: image
            source: "../images/LaserControlDelay_1.png"
            fillMode: Image.PreserveAspectFit
        }
        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Start and Endpoint are symmetric to the turning point - delay value has been properly set")
        }
        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.preferredWidth: 2 * (height * sourceSize.width) / sourceSize.height
            id: image2
            source: "../images/LaserControlDelay_2.png"
            fillMode: Image.PreserveAspectFit
        }
        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Start and Endpoint are asymmetric to the turning point - delay value is too low")
        }
        Image {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillHeight: true
            Layout.preferredWidth: 2 * (height * sourceSize.width) / sourceSize.height
            id: image3
            source: "../images/LaserControlDelay_3.png"
            fillMode: Image.PreserveAspectFit
        }
        Label {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Start and Endpoint are asymmetric to the turning point - delay value is too high")
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 20
            spacing: 30

            GridLayout {
                columns: 3
                columnSpacing: 5
                Label {
                    text: qsTr("Scan-Frequency")
                }
                SpinBox {
                    value: controller.frequency
                    to: 500
                    from: 0
                    editable: true
                    enabled: controller.ready && !controller.updating
                    onValueModified: {
                        controller.frequency = value;
                    }
                    Component.onCompleted: {
                        contentItem.selectByMouse = true;
                    }
                }
                Label {
                    text: qsTr("Hz")
                }
            }

            GridLayout {
                columns: 3
                columnSpacing: 5
                Label {
                    text: qsTr("Scan-Width")
                }                
                TextField {
                    selectByMouse: true
                    validator: DoubleValidator {
                        bottom: 0
                        top: 10
                    }
                    palette.text: acceptableInput ? "black" : "red"
                    placeholderText: "0"
                    text: controller.amplitude
                    onEditingFinished: controller.amplitude = Number.fromLocaleString(locale, text)
                    Layout.preferredWidth: 50
                    horizontalAlignment: TextInput.AlignRight
                }
                Label {
                    text: qsTr("mm")
                }                
            }
            
            GridLayout {
                columns: 3
                columnSpacing: 5
                Label {
                    text: qsTr("Laser Power")
                }
                SpinBox {
                    value: controller.power
                    to: 100
                    from: 0
                    editable: true
                    enabled: controller.ready && !controller.updating
                    onValueModified: {
                        controller.power = value;
                    }
                    Component.onCompleted: {
                        contentItem.selectByMouse = true;
                    }
                }
                Label {
                    text: "%"
                }
            }
            
            GridLayout {
                columns: 2
                Layout.leftMargin: 30
                columnSpacing: 5
                Label {
                    font.bold: true
                    text: qsTr("Laser Control Delay")
                }
                SpinBox {
                    value: controller.delay
                    to: 999
                    from: -999
                    editable: true
                    enabled: controller.ready && !controller.updating
                    onValueModified: {
                        controller.delay = value;
                    }
                    Component.onCompleted: {
                        contentItem.selectByMouse = true;
                    }
                }
            }

        }
    }
}

