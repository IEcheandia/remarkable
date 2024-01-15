import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications

import precitec.gui.general 1.0

Dialog
{
    id: saveDialog
    property var graphLoader: null
    property var graphVisualizer: null
    property var plausibilityController: null
    property bool plausible: false
    property string exportPath: ""

    modal: true
    //: title of a dialog
    title: qsTr("Save and overwrite graph?")
    standardButtons: Dialog.Cancel | Dialog.Save
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
            text: saveDialog.title
            font.pixelSize: 18
            font.bold: true
            color: PrecitecApplication.Settings.alternateText
            horizontalAlignment: Text.AlignHCenter
        }
    }

    onAccepted:
    {
        if (saveDialog.exportPath != "")
        {
            var customDirPath = saveDialog.exportPath;
            var fileName = saveDialog.graphVisualizer.getFileName();
            saveDialog.graphLoader.exportGraph(customDirPath, fileName);
            //: notification that a file got exported
            Notifications.NotificationSystem.information(qsTr("Graph exported to %1 with the filename %2.xml").arg(customDirPath).arg(fileName));
        }
        else
        {
            if (saveDialog.graphLoader.saveGraph(saveDialog.graphVisualizer.getFileName()))
            {
                Notifications.NotificationSystem.information(qsTr("Graph saved successfully"));
            }
            else
            {
                Notifications.NotificationSystem.warning(qsTr("Saving graph Failed."));
            }
        }
    }
    onRejected:
    {
        destroy();
    }

    GroupBox
    {
        id: saveView
        implicitWidth: parent.width
        implicitHeight: parent.height
        GridLayout
        {
            anchors.fill: parent
            columns: 2

            Label
            {
                id: isGraphValid
                Layout.fillWidth: true
                Layout.columnSpan: 2
                Layout.preferredHeight: 50
                text: saveDialog.plausible ? qsTr("Graph is valid!") : qsTr("Graph is invalid!")
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                background: Rectangle
                {
                    color: saveDialog.plausible ? "green" : "red"
                    radius: 3

                    border.width: 3
                    border.color: "grey"
                }
            }
        }
    }

    Component.onCompleted:
    {
        graphVisualizer.updateGraphLoaderWithModifiedGraph();
        plausible = plausibilityController.plausibilityCheck();
    }
}
