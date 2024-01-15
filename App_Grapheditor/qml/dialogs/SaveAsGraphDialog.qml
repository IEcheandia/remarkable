import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0
import grapheditor.components 1.0 as Components

Dialog {
    id: saveAsDialog
    property var graphLoader: null
    property var graphVisualizer: null
    property var directoryModel: null
    property bool plausible: false
    property var plausibilityController: null
    property bool textFieldValid: true
    modal: true
    standardButtons: Dialog.Cancel | Dialog.Save

    header: Control {
        padding: 5
        background: Rectangle
        {
            color: PrecitecApplication.Settings.alternateBackground
        }
        contentItem: Label
        {
            Layout.fillWidth: true
            text: qsTr("Save graph as...")
            font.bold: true
            color: PrecitecApplication.Settings.alternateText
            horizontalAlignment: Text.AlignHCenter
        }
    }

    onAccepted:
    {
        graphVisualizer.setGraphName(exportGraphName.text);
        graphVisualizer.updateGraphLoaderWithModifiedGraph(true);
        Notifications.NotificationSystem.information(qsTr("Graph is exported with name \"" + exportGraphName.text + ".xml\""));
        graphLoader.saveGraph(exportGraphName.text, true);
        graphVisualizer.graphEdited = false;
        graphVisualizer.clearFilterGraph();
    }
    onRejected:
    {
        destroy();
    }
    ColumnLayout
    {
        anchors.fill: parent
        Label {
            id: isGraphValid
            Layout.fillWidth: true
            Layout.preferredHeight: 50

            text: saveAsDialog.plausible ? qsTr("Graph is valid!") : qsTr("Graph is invalid!")
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            background: Rectangle {
                color: saveAsDialog.plausible ? "green" : "red"
                radius: 3

                border.width: 3
                border.color: "grey"
            }
        }
        ColumnLayout {
            Label {
                id: exportGraphNameText
                text: "Export graph name:"
                font.pixelSize: 18
                font.bold: true
                Layout.fillWidth: true
            }
            TextField {
                id: exportGraphName
                objectName: "grapheditor-export-graph-name"
                Layout.fillWidth: true
                selectByMouse: true
                validator: Components.ExistingFileValidator {
                    id: fileValidator
                    Component.onCompleted: {
                        fileValidator.setCurrentDirectory(graphVisualizer.getFilePath());
                    }
                }
                palette.text: exportGraphName.acceptableInput ? "black" : "red"
                Component.onCompleted:{
                    validator.setDirectoryModel(directoryModel);
                    validator.setDirectoryIndex(graphLoader.saveAsDirectoryIndex());
                    text = graphVisualizer.filterGraph.graphName
                }

            }
        }
        Item {
            Layout.fillHeight: true
        }
    }
    Component.onCompleted:
    {
        plausible = plausibilityController.plausibilityCheck();
        standardButton(Dialog.Save).enabled = Qt.binding(function() { return exportGraphName.acceptableInput});
    }
}
