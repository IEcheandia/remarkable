import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

import QuickQanava          2.0 as Qan
import "qrc:/QuickQanava"   as Qan

Component
{
    id: zoomInterface

    Dialog
    {
        id: zoomProperties
        property var graphView: null
        anchors.centerIn: parent
        width: parent.width * 0.4
        height: parent.height * 0.45
        modal: true
        standardButtons: Dialog.Close | Dialog.Ok
        header: Control
        {
            padding: 5
            background: Rectangle
            {
                color: PrecitecApplication.Settings.alternateBackground
                height: 40
            }
            contentItem: Label
            {
                Layout.fillWidth: true
                text: qsTr("Zoom configuration:")
                font.pixelSize: 18
                font.bold: true
                color: PrecitecApplication.Settings.alternateText
                horizontalAlignment: Text.AlignHCenter
            }
        }

        onAccepted:
        {
            destroy();
        }
        onRejected:
        {
            destroy();
        }

        GroupBox
        {
            id: zoomPropertiesView
            implicitWidth: parent.width
            implicitHeight: parent.height

            ColumnLayout
            {
                anchors.fill: parent
                Label
                {
                    id: autoFitModeLabel
                    Layout.fillWidth: true
                    text: qsTr("Activate auto fit:")
                    font.pixelSize: 18
                    font.bold: true
                }
                CheckBox
                {
                    id: autoFitModeCheckBox
                    Layout.fillWidth: true
                    text: qsTr("AutoFitMode?");
                    checked: zoomProperties.graphView ? zoomProperties.graphView.autoFitMode : false;
                    onToggled:
                    {
                        if (autoFitModeCheckBox.checked)
                        {
                            zoomProperties.graphView.autoFitMode = Qan.Navigable.AutoFit;
                            return;
                        }
                        zoomProperties.graphView.autoFitMode = Qan.Navigable.NoAutoFit;
                    }
                }
                Label
                {
                    id: zoomIncrementLabel
                    Layout.fillWidth: true
                    text: qsTr("Zoom increment:")
                    font.pixelSize: 18
                    font.bold: true
                }
                TextField
                {
                    id: zoomIncrementText
                    Layout.fillWidth: true
                    selectByMouse: true
                    text: zoomProperties.graphView ? zoomProperties.graphView.zoomIncrement.toLocaleString(locale, 'f', 2) : "0"
                    onEditingFinished:
                    {
                        zoomProperties.graphView.zoomIncrement = Number.fromLocaleString(locale, zoomIncrementText.text);
                    }
                }
                Label
                {
                    id: zoomLabel
                    Layout.fillWidth: true
                    text: qsTr("Zoom:")
                    font.pixelSize: 18
                    font.bold: true
                }
                TextField
                {
                    id: zoomText
                    Layout.fillWidth: true
                    selectByMouse: true
                    text: zoomProperties.graphView ? zoomProperties.graphView.zoom.toLocaleString(locale, 'f', 1) : "1.0"
                    onEditingFinished:
                    {
                        zoomProperties.graphView.zoom = Number.fromLocaleString(locale, zoomText.text);
                    }
                }
            }
        }
    }
}
