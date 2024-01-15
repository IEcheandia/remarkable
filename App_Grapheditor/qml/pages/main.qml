import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0

ApplicationWindow
{
    visible: true
    id: applicationWindow
    title: qsTr("Graph editor")
    visibility: Window.Maximized




    GraphModel
    {
        id: rootGraphModel
        Component.onCompleted:
        {
            rootGraphModel.loadGraphs(WeldmasterPaths.systemGraphDir);
        }
    }

    SubGraphModel
    {
        id: rootSubGraphModel
    }

    AttributeModel
    {
        id: rootAttributeModel
        Component.onCompleted:
        {
            rootAttributeModel.loadDefault();
        }
    }

    ResultSettingModel
    {
        id: rootResultsSettingsModel
    }

    SensorSettingsModel
    {
        id: rootSensorSettingsModel
        configurationDirectory: WeldmasterPaths.configurationDir
    }

    MainView {
        id: mainView
        anchors.fill: parent
        attributeModel: rootAttributeModel
        resultModel: rootResultsSettingsModel
        sensorModel: rootSensorSettingsModel
        screenshotTool: screenshot
    }

    /**********************************************************************************************************************************************************************************************/
    //Precitec-Stuff (footer and header) stuff I didn't need is comment out.
    Component
    {
        id: screenshotAndHelpToolComponent
        RowLayout
        {
            ToolButton
            {
                id: screenshotButton
                objectName: "topBar-screenshotButton"
                icon.name: "camera-photo"
                icon.color: PrecitecApplication.Settings.alternateText
                icon.width: Math.min(availableWidth, availableHeight)
                icon.height: Math.min(availableWidth, availableHeight)
                flat: true
                onClicked: screenshot.takeScreenshot()
                palette.button: "transparent"
            }
        }
    }

    PrecitecApplication.ScreenshotTool
    {
        id: screenshot
        maximumNumberOfScreenshots: 30
    }

    QuitMessage
    {
        id: quitMessage
    }

    onClosing:
    {
        if (graphVisualizer.graphEdited)
        {
            close.accepted = false;
            onTriggered:
            {
                var quitDialog = quitMessage.createObject(applicationWindow);
                quitDialog.open();
            }
        }
    }

    /**********************************************************************************************************************************************************************************************/
    //Header
    header: PrecitecApplication.TopBar
    {
        id: topBar
        objectName: "topBar"
        enabled: LanguageSupport.ready
        systemInformationDialogTitle: qsTr("General system information")
        model: ListModel {
            ListElement {
                text: qsTr("Home")
                iconSource: "qrc:/icons/home"
                permission: -1
                enabled: true
                objectName: "topBar-overview"
            }
        }
        systemInformationModel: ListModel
        {
        }
        Component.onCompleted:
        {
            [
                {
                    packageName: qsTr("Graph editor"),
                    //version: weldmasterVersion,
                    //changesetId: weldmasterChangesetId,
                    //buildTime: weldmasterBuildTimestamp
                }
            ].forEach(function(e) { topBar.systemInformationModel.append(e); });
            topBar.customElements.sourceComponent = screenshotAndHelpToolComponent;
        }
    }

    /**********************************************************************************************************************************************************************************************/
    Notifications.NotificationsOverlay{}
    //Footer
    footer: PrecitecApplication.StatusBar
    {
        id: statusBar
        Label {
            color: PrecitecApplication.Settings.alternateText
            text: qsTr("System ready!")
            maximumLineCount: 1
            elide: Text.ElideRight
            Layout.fillWidth: true
            Layout.leftMargin: 5
        }
        ToolSeparator {}
        RemovableDevices.DeviceIndicator {
            id: deviceIndicator
            canMount: UserManagement.currentUser && UserManagement.hasPermission(App.MountPortableDevices)
            palette.buttonText: PrecitecApplication.Settings.text
            palette.windowText: PrecitecApplication.Settings.alternateText
        }
        ToolSeparator {
            visible: deviceIndicator.visible
        }
        PrecitecApplication.StatusBarClock {
            id: clock
            Layout.alignment: Qt.AlignVCenter
        }
    }

}
