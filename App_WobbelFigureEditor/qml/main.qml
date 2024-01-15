import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0

import wobbleFigureEditor.components 1.0

ApplicationWindow {
    visible: true
    id: applicationWindow
    title: qsTr("Figure editor")
    visibility: Window.Maximized

    MainView {
      id: mainView
      anchors.fill: parent

      screenshotTool: screenshot
    }

    /**********************************************************************************************************************************************************************************************/
    //Precitec-Stuff (footer and header) just stuff I didn't need is comment out.
    Component
    {
        id: screenshotAndHelpToolComponent
        RowLayout
        {
            ToolButton
            {
                id: laserPowerButton
                objectName: "topBar-laserPower"
                icon.name: "menu-icon_calibration"
                icon.color: PrecitecApplication.Settings.alternateText
                icon.width: Math.min(availableWidth, availableHeight)
                icon.height: Math.min(availableWidth, availableHeight)
                flat: true
                onClicked: wobbleFigureEditor.changePower();
                palette.button: "transparent"
            }
            ToolButton
            {
                id: offsetFigure
                objectName: "topBar-offset"
                icon.name: "menu-icon_pre-adjust"
                icon.color: PrecitecApplication.Settings.alternateText
                icon.width: Math.min(availableWidth, availableHeight)
                icon.height: Math.min(availableWidth, availableHeight)
                flat: true
                onClicked:
                {
                     var newOffsetDialog = offset.createObject(applicationWindow, {"handler": wobbleFigureEditor});
                     newOffsetDialog.open();
                }
                palette.button: "transparent"
            }
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

    PrecitecApplication.ScreenshotTool {
        id: screenshot
        maximumNumberOfScreenshots: GuiConfiguration.maximumNumberOfScreenshots
    }

    /**********************************************************************************************************************************************************************************************/
    //Header
    header: PrecitecApplication.TopBar
    {
        id: topBar
        objectName: "topBar"
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
                    packageName: qsTr("Figure editor")
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
            text: qsTr("System ready")
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
            Layout.rightMargin: 5
        }
    }
}
