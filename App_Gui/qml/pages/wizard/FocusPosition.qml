import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0

Item {
    id: focusPosition

    FocusPositionController {
        id: controller
        weldHeadDevice: HardwareModule.weldHeadDeviceProxy
    }

    ColumnLayout {
        anchors.centerIn: parent
        Image {
            Layout.preferredHeight: 0.8 * focusPosition.height
            Layout.preferredWidth: 0.8 * focusPosition.height
            id: image
            source: "../images/focusPosition.png"
            fillMode: Image.PreserveAspectFit
        }

        
        GridLayout {
            columns: 2
            Layout.alignment: Qt.AlignHCenter
            columnSpacing: 10
 
            Button {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                enabled: controller.ready && !controller.updating
                text: qsTr("Reference Run")
                onClicked: {
                    controller.performReferenceRun();
                }
            }
 
            Label {
                //: Label for height difference of the system with unit
                text: qsTr("System Z-Offset (mm)")
            }
            
            TextField {
                selectByMouse: true
                validator: DoubleValidator {
                    bottom: -50
                    top: 50
                }
                palette.text: acceptableInput ? "black" : "red"
                placeholderText: "0"
                text: Number(controller.systemOffset).toLocaleString(locale)
                onEditingFinished: controller.systemOffset = Number.fromLocaleString(locale, text)
            }
                        
        }
    }

}
