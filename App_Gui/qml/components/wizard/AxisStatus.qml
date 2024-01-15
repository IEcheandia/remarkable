import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

/**
 * Provides status information about an axis in a group box
 **/

StackView {
    id: stackView
    implicitWidth: currentItem.implicitWidth
    implicitHeight: currentItem.implicitHeight
    clip: true
    initialItem: GroupBox {
        id: groupBox
        label: RowLayout {
            width: groupBox.availableWidth
            Label {
                text: qsTr("Axis state")
                font.family: groupBox.font.family
                font.pixelSize: groupBox.font.pixelSize
                font.bold: true
                verticalAlignment: Text.AlignVCenter

                Layout.leftMargin: groupBox.leftPadding
                Layout.fillWidth: true
            }
            ToolButton {
                display: AbstractButton.IconOnly
                icon.name: "application-menu"
                onClicked: {
                    stackView.push(stateComponent);
                }
            }
        }
        GridLayout {
            columns: 2
            Label {
                text: qsTr("positionUserUnit", "Precitec.Service.WeldHead.Position")
            }
            Label {
                text: qsTr("%1 µm").arg(yAxisInformation.positionUserUnit)
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("Operation mode", "Precitec.Service.WeldHead.ModeOfOperation")
            }
            Label {
                function mode(modeOfOperation)
                {
                    if (modeOfOperation == AxisInformation.Pending)
                    {
                        return qsTr("Pending");
                    }
                    if (modeOfOperation == AxisInformation.Offline)
                    {
                        return qsTr("Offline");
                    }
                    if (modeOfOperation == AxisInformation.Position)
                    {
                        return qsTr("Position");
                    }
                    if (modeOfOperation == AxisInformation.Position_Relative)
                    {
                        return qsTr("Relative Position");
                    }
                    if (modeOfOperation == AxisInformation.Position_Absolute)
                    {
                        return qsTr("Absolute Position");
                    }
                    if (modeOfOperation == AxisInformation.Velocity)
                    {
                        return qsTr("Velocity");
                    }
                    if (modeOfOperation == AxisInformation.Home)
                    {
                        return qsTr("Homing");
                    }
                    return qsTr("Unknown")
                }
                text: mode(yAxisInformation.modeOfOperation)
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("Error code", "Precitec.Service.WeldHead.ErrorCode")
            }
            Label {
                text: "0x" + yAxisInformation.errorCode.toString(16)
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("actVelocity", "Precitec.Service.WeldHead.ActVelocity")
            }
            Label {
                text: yAxisInformation.actVelocity
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("actTorque", "Precitec.Service.WeldHead.ActTorque")
            }
            Label {
                text: yAxisInformation.actTorque
                Layout.alignment: Qt.AlignRight
            }

            Label {
                text: qsTr("Input statusword", "Precitec.Service.WeldHead.InputStatusWord")
            }
            Label {
                text: "0x" + yAxisInformation.statusWord.toString(16)
                Layout.alignment: Qt.AlignRight
            }
        }
    }
    Component {
        id: stateComponent
        BackButtonGroupBox {
            title: qsTr("Axis state")
            onBack: stackView.pop()
            implicitWidth: stateListView.implicitWidth
            ScrollView {
                anchors.fill: parent
                ListView {
                    id: stateListView
                    anchors.fill: parent
                    model: AxisStatusModel {
                        weldHeadServer: yAxisInformation.weldHeadServer
                    }
                    spacing: 4
                    delegate: ImplicitWidthDelegate {
                        width: ListView.view.width
                        implicitWidth: label.implicitWidth + 20 + 2 * spacing
                        height: childrenRect.height
                        Rectangle {
                            id: indicator
                            width: 16
                            height: width
                            radius: width/2
                            color: model.flag ? "green" : "red"
                            anchors {
                                left: parent.left
                            }
                        }
                        Label {
                            id: label
                            text: model.display
                            anchors {
                                left: indicator.right
                                leftMargin: 4
                            }
                        }
                    }
                }
            }
        }
    }
}
