import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

import QuickQanava          2.0 as Qan
import "qrc:/QuickQanava"   as Qan

Dialog {
    id: zoomProperties
    property var figureView: null
    property alias screenshotTool: screenshotHeader.screenshotTool

    modal: true
    standardButtons: Dialog.Close | Dialog.Ok

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: qsTr("Zoom configuration:")
    }

    onAccepted: {
        destroy();
    }

    onRejected: {
        destroy();
    }

    GroupBox {
        id: zoomPropertiesView
        anchors.fill: parent

        ColumnLayout {
            anchors.fill: parent
            Label {
                id: autoFitModeLabel
                Layout.fillWidth: true
                text: qsTr("Activate auto fit:")
                font.pixelSize: 18
                font.bold: true
            }
            CheckBox {
                id: autoFitModeCheckBox
                Layout.fillWidth: true
                text: qsTr("AutoFitMode?")
                checked: zoomProperties.figureView && zoomProperties.figureView.autoFitMode
                onToggled: {
                    if (!zoomProperties.figureView)
                    {
                        return;
                    }
                    if (autoFitModeCheckBox.checked)
                    {
                        zoomProperties.figureView.autoFitMode = Qan.Navigable.AutoFit;
                        return;
                    }
                    zoomProperties.figureView.autoFitMode = Qan.Navigable.NoAutoFit;
                }
            }
            Label {
                id: zoomIncrementLabel
                Layout.fillWidth: true
                text: qsTr("Zoom increment:")
                font.pixelSize: 18
                font.bold: true
            }
            TextField {
                id: zoomIncrementText
                Layout.fillWidth: true
                selectByMouse: true
                text: zoomProperties.figureView ? zoomProperties.figureView.zoomIncrement.toLocaleString(locale, 'f', 2) : "0"
                onEditingFinished: {
                    if (zoomProperties.figureView)
                    {
                        zoomProperties.figureView.zoomIncrement = Number.fromLocaleString(locale, zoomIncrementText.text);
                    }
                }
            }
            Label {
                id: zoomLabel
                Layout.fillWidth: true
                text: qsTr("Zoom:")
                font.pixelSize: 18
                font.bold: true
            }
            TextField {
                id: zoomText
                Layout.fillWidth: true
                selectByMouse: true
                text: zoomProperties.figureView ? zoomProperties.figureView.zoom.toLocaleString(locale, 'f', 2) : "1.0"
                onEditingFinished: {
                    if (zoomProperties.figureView)
                    {
                        zoomProperties.figureView.zoom = Number.fromLocaleString(locale, zoomText.text);
                    }
                }
            }
        }
    }
}
