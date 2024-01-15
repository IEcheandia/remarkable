import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

Component
{
    id: gridInterface
    Dialog
    {
        id: graphProperties
        property var lineGrid: null
        property var graphVisualizer: null
        anchors.centerIn: parent
        width: parent.width * 0.3
        height: parent.height * 0.4
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
                text: qsTr("Change grid properties")
                font.pixelSize: 18
                font.bold: true
                color: PrecitecApplication.Settings.alternateText
                horizontalAlignment: Text.AlignHCenter
            }
        }

        onAccepted:
        {
            lineGrid.gridScale = Number.fromLocaleString(locale, gridMinorText.text);
            lineGrid.gridMajor = Number.fromLocaleString(locale, gridMajorText.text);
            graphVisualizer.gridSize = Number.fromLocaleString(locale, gridSizeText.text);
        }
        onRejected:
        {
            destroy();
        }

        GroupBox
        {
            id: gridPropertiesView
            implicitWidth: parent.width
            implicitHeight: parent.height
            GridLayout
            {
                id: gridPropertiesInterface
                anchors.fill: parent
                columns: 3
                Label
                {
                    id: gridMinorLabel
                    Layout.fillWidth: true
                    text: qsTr("Grid minor:")
                    font.pixelSize: 18
                    font.bold: true
                }
                TextField
                {
                    id: gridMinorText
                    Layout.fillWidth: true
                    selectByMouse: true
                    validator: DoubleValidator
                    {
                        bottom: 1.0
                    }
                    text: Number(lineGrid.gridScale ? lineGrid.gridScale : 25.0).toLocaleString(locale, 'f', 2)
                    onEditingFinished:
                    {
                        lineGrid.gridScale = Number.fromLocaleString(locale, gridMinorText.text);
                    }
                }
                Label
                {
                    id: gridMinorDefault
                    Layout.fillWidth: true
                    text: qsTr("Default: %1").arg(Number(25.0).toLocaleString(locale, 'f', 2))
                    font.pixelSize: 18
                    font.bold: true
                }
                Label
                {
                    id: gridMajorLabel
                    Layout.fillWidth: true
                    text: qsTr("Grid major:")
                    font.pixelSize: 18
                    font.bold: true
                }
                TextField
                {
                    id: gridMajorText
                    Layout.fillWidth: true
                    selectByMouse: true
                    validator: IntValidator
                    {
                        bottom: 1
                    }
                    text: Number(lineGrid.gridMajor ? lineGrid.gridMajor : 5.0).toLocaleString(locale, 'f', 0)
                    onEditingFinished:
                    {
                        lineGrid.gridMajor = Number.fromLocaleString(locale, gridMajorText.text);
                    }
                }
                Label
                {
                    id: gridMajorDefault
                    Layout.fillWidth: true
                    text: qsTr("Default: 5")
                    font.pixelSize: 18
                    font.bold: true
                }
                Label
                {
                    id: gridSizeLabel
                    Layout.fillWidth: true
                    text: qsTr("Grid size:")
                    font.pixelSize: 18
                    font.bold: true
                }
                TextField
                {
                    id: gridSizeText
                    Layout.fillWidth: true
                    selectByMouse: true
                    validator: IntValidator
                    {
                        bottom: 1
                    }
                    text: Number(graphVisualizer.gridSize ? graphVisualizer.gridSize : 1.0).toLocaleString(locale, 'f', 0)
                    onEditingFinished:
                    {
                        graphVisualizer.gridSize = Number.fromLocaleString(locale, gridSizeText.text);
                        gridMinorText.text = gridSizeText.text;
                        lineGrid.gridScale = Number.fromLocaleString(locale, gridSizeText.text);
                    }
                }
                Label
                {
                    id: gridSizeDefault
                    Layout.fillWidth: true
                    text: qsTr("Default: 1")
                    font.pixelSize: 18
                    font.bold: true
                }
                CheckBox
                {
                    id: useGridSizeAutomatically
                    Layout.fillWidth: true
                    Layout.columnSpan: 3
                    text: qsTr("Use grid size with DnD")
                    checked: graphVisualizer.useGridSizeAutomatically
                    onToggled: graphVisualizer.useGridSizeAutomatically = checked
                }
            }
        }
    }
}
