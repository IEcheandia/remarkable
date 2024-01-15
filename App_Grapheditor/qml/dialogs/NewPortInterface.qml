import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

Component
{
    id: newPortUserInterface
    Dialog
    {
        id: portUserInterface
        anchors.centerIn: parent
        width: applicationWindow.width * 0.5
        height: applicationWindow.height * 0.5
        modal: true
        standardButtons: Dialog.Close | Dialog.Ok

        onAccepted:
        {
            if (commentCheckBox.checked == true)
            {
                graphEditor.insertComment(portLabel.text, newPortType.value, portText.text, newPortPosX.value, newPortPosY.value, newPortSizeWidth.value, newPortSizeHeight.value)
            }
            else
            {
                graphEditor.insertFilterPort(portLabel.text, newPortType.value, newPortPosX.value, newPortPosY.value)
            }
        }
        onRejected:
        {
            destroy();
        }
        GroupBox
        {
            implicitWidth: parent.width
            implicitHeight: parent.height
            GridLayout
            {
                id: gridLayoutInterface
                columns: 2
                rows: 3
                anchors.fill: parent
                columnSpacing: 3
                rowSpacing: 3
                ColumnLayout
                {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Layout.preferredWidth: gridLayoutInterface.width*0.5
                    Label
                    {
                        id: portIDLabel
                        text: "Port ID:"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    TextField
                    {
                        id: portID
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        Layout.preferredWidth: gridLayoutInterface.width*0.4
                        readOnly: true
                        text: graphEditor.newID
                    }
                }
                CheckBox
                {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    id: commentCheckBox
                    checked: false
                    checkable: true
                    text: qsTr("Comment")
                    onToggled:
                    {
                        if (commentCheckBox.checked == true)
                        {
                            newPortType.from = 5;
                            newPortType.to = 5;
                            newPortType.value = 5;
                            newPortSizeWidth.to = 1000;
                            newPortSizeWidth.value = 50;
                            newPortSizeHeight.to = 1000;
                            newPortSizeHeight.value = 50;
                            portLabel.readOnly = true;
                            portLabel.text = "";
                            portText.readOnly = false;
                        }
                        else
                        {
                            newPortType.from = 2;
                            newPortType.to = 3;
                            newPortType.value = 2;
                            newPortSizeWidth.from = 50;
                            newPortSizeWidth.to = 50;
                            newPortSizeWidth.value = 50;
                            newPortSizeHeight.from = 50;
                            newPortSizeHeight.to = 50;
                            newPortSizeHeight.value = 50;
                            portLabel.readOnly = false;
                            portText.readOnly = true;
                            portText.text = "";
                        }
                    }
                }
                ColumnLayout
                {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Layout.preferredWidth: gridLayoutInterface.width*0.5
                    Label
                    {
                        id: portLabelLabel
                        text: "Port Label:"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    TextField
                    {
                        id: portLabel
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        Layout.preferredWidth: gridLayoutInterface.width*0.4
                        //text:
                    }
                }
                ColumnLayout
                {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Layout.preferredWidth: gridLayoutInterface.width*0.5
                    Label
                    {
                        id: portTextLabel
                        text: "Comment text:"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    TextField
                    {
                        id: portText
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        Layout.preferredWidth: gridLayoutInterface.width*0.4
                        //text:
                        readOnly: true;
                    }
                }
                ColumnLayout
                {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Layout.preferredWidth: gridLayoutInterface.width*0.5
                    Label
                    {
                        id: newPortTypeLabel
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        Layout.preferredWidth: gridLayoutInterface.width*0.4
                        text: "Port type:"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    SpinBox
                    {
                        id: newPortType
                        value: 2
                        from: 2
                        to: 3
                    }
                }
                ColumnLayout
                {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Layout.preferredWidth: gridLayoutInterface.width*0.5
                    Label
                    {
                        id: newPortGroupIDLabel
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        Layout.preferredWidth: gridLayoutInterface.width*0.4
                        text: "Group ID:"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    SpinBox
                    {
                        id: newPortGroupID
                        value: -1
                        from: -1
                        to: 100
                        wrap: true
                    }
                }
                ColumnLayout
                {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                    Layout.preferredWidth: gridLayoutInterface.width*0.5
                    Label
                    {
                        id: newPortPositionLabel
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        Layout.preferredWidth: gridLayoutInterface.width*0.4
                        text: "Position:"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    RowLayout
                    {
                        SpinBox
                        {
                            id: newPortPosX
                            value: 0
                            from: -1000000
                            to: 1000000
                            wrap: true
                            editable: true
                        }
                        SpinBox
                        {
                            id: newPortPosY
                            value: 0
                            from: -1000000
                            to: 1000000
                            wrap: true
                            editable: true
                        }
                    }
                }
                ColumnLayout
                {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                    Layout.preferredWidth: gridLayoutInterface.width*0.5
                    Label
                    {
                        id: newPortSizeLabel
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft
                        Layout.preferredWidth: gridLayoutInterface.width*0.4
                        text: "Size:"
                        font.pixelSize: 18
                        font.bold: true
                    }
                    RowLayout
                    {
                        SpinBox
                        {
                            id: newPortSizeWidth
                            value: 50
                            from: 50
                            to: 50
                            editable: true
                        }
                        SpinBox
                        {
                            id: newPortSizeHeight
                            value: 50
                            from: 50
                            to: 50
                            editable: true
                        }
                    }
                }
            }
        }
        Component.onCompleted:
        {
            graphEditor.generateNewID();
        }
    }
}
