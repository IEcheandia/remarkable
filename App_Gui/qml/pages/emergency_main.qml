import QtQuick 2.7
import QtQuick.Window 2.2
import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.logging 1.0 as Logging
import precitec.gui.general 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.configuration 1.0
import precitec.gui 1.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import "qrc:/resources/qml/"

import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.notifications 1.0 as Notifications


ApplicationWindow {
    id: applicationWindow
    property var lastUser: null
    width: 1024
    height: 1024

    PrecitecApplication.InputMethodEventFilter {
    }

    Component {
        id: screenshotAndHelpToolComponent
        RowLayout {
            ToolButton {
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

    header: PrecitecApplication.TopBar {
        id: topBar
        objectName: "topBar"
        enabled: LanguageSupport.ready
        screenshotTool: screenshot
        systemInformationDialogTitle: qsTr("General system information")
        model: ListModel {
            ListElement {
                text: qsTr("Home")
                iconSource: "qrc:/icons/home"
                permission: -1
                enabled: true
                objectName: "topBar-overview"
            }
            ListElement {
                text: qsTr("Login")
                iconSource: "qrc:/icons/user"
                permission: -1
                enabled: true
                objectName: "topBar-login"
            }
            ListElement {
                text: qsTr("Results")
                iconSource: "qrc:/icons/fileopen"
                permission: -1
                enabled: false
                objectName: "topBar-results"
            }
            ListElement {
                text: qsTr("Simulation")
                iconSource: "qrc:/icons/video"
                permission: -1
                enabled: false
                objectName: "topBar-simulation"
            }
            ListElement {
                text: qsTr("Configuration")
                iconSource: "qrc:/icons/wizard"
                permission: App.RunHardwareAndProductWizard
                enabled: false
                objectName: "topBar-configuration"
            }
            ListElement {
                text: qsTr("Settings")
                iconSource: "qrc:/icons/tool"
                permission: -1
                enabled: true
                objectName: "topBar-settings"
            }
        }
        version: Qt.application.version
        systemInformationModel: ListModel {
        }
        Component.onCompleted: {
            [
                {
                    packageName: qsTr("Weldmaster"),
                    version: Qt.application.version,
                    changesetId: weldmasterChangesetId,
                    buildTime: weldmasterBuildTimestamp
                }
            ].forEach(function(e) { topBar.systemInformationModel.append(e); });
            topBar.customElements.sourceComponent = screenshotAndHelpToolComponent;
        }
    }

    Component {
        id: homeComponent
        Item {
        }
    }

    Component {
        id: userComponent
        Loader {
            active: false
            visible: ListView.isCurrentItem
            width: container.width
            height: container.height
            onVisibleChanged: {
                if (visible)
                {
                    active = true;
                }
            }
            sourceComponent: LoginView {
                model: UserManagement.users
            }
        }
    }

    Component {
        id: configurationComponent
        Loader {
            active: false
            visible: ListView.isCurrentItem
            width: container.width
            height: container.height
            onVisibleChanged: {
                if (visible)
                {
                    active = true;
                }
            }
            sourceComponent: PrecitecApplication.SideTabView {
                id: pageItem
                model: SecurityFilterModel {
                    sourceModel: tabModel
                }
                EmergencyConfigurationTabModel {
                    id: tabModel
                }

                Component {
                    id: shutdownPage
                    Loader {
                        active: false
                        visible: ListView.isCurrentItem
                        width: loader.width
                        height: loader.height
                        onVisibleChanged: {
                            if (visible)
                            {
                                active = true;
                            }
                        }
                        sourceComponent: ShutdownPage {
                            shutdownSystemPermission: App.ShutdownSystem
                            restartSystemPermission: App.ShutdownSystem
                        }
                    }
                }

                Component {
                    id: backupView
                    Loader {
                        active: false
                        visible: ListView.isCurrentItem
                        width: loader.width
                        height: loader.height
                        onVisibleChanged: {
                            if (visible)
                            {
                                active = true;
                            }
                        }
                        sourceComponent: BackupPage {}
                    }
                }

                contentItem: Container {
                    id: loader
                    currentIndex: pageItem.model.mapToSource(pageItem.model.index(pageItem.currentIndex, 0)).row
                    anchors.fill: parent

                    contentItem: ListView {
                        model: loader.contentModel
                        currentIndex: loader.currentIndex
                        interactive: false
                        snapMode: ListView.SnapOneItem
                        orientation: ListView.Horizontal
                        boundsBehavior: Flickable.StopAtBounds

                        highlightRangeMode: ListView.StrictlyEnforceRange
                        preferredHighlightBegin: 0
                        preferredHighlightEnd: 0
                        highlightMoveDuration: 250
                        clip: true
                    }

                    Component.onCompleted: {
                        loader.addItem(shutdownPage.createObject(loader));
                        loader.addItem(backupView.createObject(loader));
                    }
                }
            }
        }
    }


    Container {
        function addItems() {
            if (container.count == 0)
            {
                container.addItem(homeComponent.createObject(container));
            }
            if (container.count == 1)
            {
                container.addItem(userComponent.createObject(container));
            }
            container.addItem(homeComponent.createObject(container));
            container.addItem(homeComponent.createObject(container));
            container.addItem(homeComponent.createObject(container));
            container.addItem(configurationComponent.createObject(container));
        }
        id: container
        visible: LanguageSupport.ready
        currentIndex: topBar.tabBar.currentIndex
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        contentItem: Item {
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (ApplicationWindow.activeFocusControl) {
                        ApplicationWindow.activeFocusControl.focus = false;
                    }
                }
            }
            MultiPointTouchArea {
                mouseEnabled: false
                anchors.fill: parent
                onPressed: {
                    if (ApplicationWindow.activeFocusControl) {
                        ApplicationWindow.activeFocusControl.focus = false;
                    }
                }
            }
            ListView {
                anchors.fill: parent
                model: container.contentModel
                currentIndex: container.currentIndex
                interactive: false
                snapMode: ListView.SnapOneItem
                orientation: ListView.Horizontal
                boundsBehavior: Flickable.StopAtBounds

                highlightRangeMode: ListView.StrictlyEnforceRange
                preferredHighlightBegin: 0
                preferredHighlightEnd: 0
                highlightMoveDuration: 250
            }
        }

        Component.onCompleted: {
            container.addItems();
        }
    }

    /**
     * This item functions as a not visible parent for all the items added to users on User switching.
     * It ensures that the items are properly destroyed when the application exits.
     **/
    Item {
        id: trashBin
        visible: false
        anchors.fill: parent
    }

    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            // detach all pages after UserManagement
            var items = [];
            while (container.count > 2)
            {
                items[items.length] = container.takeItem(container.count - 1);
            }
            for (var i = 0; i < items.length; i++)
            {
                items[i].parent = trashBin;
            }
            // and add them to the lastUser and store the current tab
            applicationWindow.lastUser.pages = items;
            applicationWindow.lastUser.tabIndex = topBar.tabBar.currentIndex;
            // now update last user for next time the currentUser changes
            applicationWindow.lastUser = UserManagement.currentUser;

            // do we already have pages stored?
            if (UserManagement.currentUser.pages !== undefined)
            {
                // if yes add them back to the container
                for (var i = UserManagement.currentUser.pages.length - 1; i >= 0; i--)
                {
                    container.addItem(UserManagement.currentUser.pages[i]);
                }
                UserManagement.currentUser.pages = undefined;
            } else
            {
                // if not add default pages
                container.addItems();
            }
            // and restore the tab
            // go to the current page by default or if on the login page
            var index = 0;
            if (UserManagement.currentUser.tabIndex !== undefined)
            {
                // not for the login page
                if (UserManagement.currentUser.tabIndex != 1)
                {
                    index = UserManagement.currentUser.tabIndex;
                }
                UserManagement.currentUser.tabIndex = undefined;
            }
            topBar.tabBar.setCurrentIndex(index);
        }
    }

    Notifications.NotificationsOverlay {
    }
    footer: PrecitecApplication.StatusBar {
        id: statusBar
        SystemStatusLabel {
            Layout.fillHeight: true
            systemStatus: SystemStatusServer.Unknown
            textColor: PrecitecApplication.Settings.alternateText
        }
        ToolSeparator {}
        Label {
            color: PrecitecApplication.Settings.alternateText
            text: qsTr("An internal error occurred, please restart system")
            maximumLineCount: 1
            elide: Text.ElideRight
            Layout.fillWidth: true
        }
        ToolSeparator {}
        UserStatus {
            color: PrecitecApplication.Settings.alternateText
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
        PrecitecApplication.StatusBarRestartRequired {
            id: restartRequiredItem
            restartRequired: true
        }
        ToolSeparator {
            visible: restartRequiredItem.restartRequired
        }
        PrecitecApplication.StatusBarClock {
            id: clock
            Layout.alignment: Qt.AlignVCenter
        }
    }

    Component.onCompleted: {
        applicationWindow.lastUser = UserManagement.currentUser;
        applicationWindow.showFullScreen();
        Notifications.NotificationSystem.error(qsTr("An internal error occurred, please restart system"));
    }

    PrecitecApplication.ScreenshotTool {
        id: screenshot
        maximumNumberOfScreenshots: GuiConfiguration.maximumNumberOfScreenshots
    }
}
