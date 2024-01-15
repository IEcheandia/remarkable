import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0
import precitec.gui.components.userManagement 1.0
import Precitec.AppGui 1.0
import precitec.gui.components.ethercat 1.0 as EtherCAT
import precitec.gui.components.userLog 1.0 as UserLog
import precitec.gui.components.ssh 1.0 as SSH
import precitec.gui.components.scheduler 1.0 as Scheduler
import precitec.gui 1.0
import precitec.gui.general 1.0
import precitec.gui.configuration 1.0

import precitec.laserHeadMonitor 1.0 as HeadMonitor

SideTabView {
    id: pageItem
    property var resultsConfigModel
    property var errorSettingModel
    property var sensorConfigModel
    property bool restartRequired: false
    /**
     * PDF file name for online help
     **/
    property var pdfFile: OnlineHelp.HasNoPdf
    property var remoteDesktop
    property var laserHeadsModel: null
    property var screenshotTool: null
    property var keyValueAttributeModel: null

    signal dateTimeChanged()

    model: SecurityFilterModel {
        filterAvailable: true
        sourceModel: tabModel
    }
    ConfigurationTabModel {
        id: tabModel
    }
    Component {
        id: changePasswordView
         ContainerItemLoader {
            sourceComponent: ChangePasswordView {}
        }
    }
    Component {
        id: userManagementView
        ContainerItemLoader {
            sourceComponent: UserManagementView {
                model: UserManagement.users
                screenshotTool: pageItem.screenshotTool
            }
        }
    }
    Component {
        id: roleManagementView
        ContainerItemLoader {
            sourceComponent: RoleManagementView {
                screenshotTool: pageItem.screenshotTool
            }
        }
    }
    Component {
        id: shutdownPage
        ContainerItemLoader {
            sourceComponent: ShutdownPage {
                shutdownSystemPermission: App.ShutdownSystem
                restartSystemPermission: App.ShutdownSystem
            }
        }
    }
    Component {
        id: licenseView
        ContainerItemLoader {
            sourceComponent: OpenSourceLicenseView {}
        }
    }
    Component {
        id: devicesView
        Loader {
            id: devicesViewLoader
            active: false
            visible: ListView.isCurrentItem
            width: loader.width
            height: loader.height
            source: "DevicesPage.qml"
            onVisibleChanged: {
                if (visible) {
                    active = true;
                    item.deviceProxies = [
                        HardwareModule.guiDeviceProxy,
                        HardwareModule.grabberDeviceProxy,
                        HardwareModule.calibrationDeviceProxy,
                        HardwareModule.videoRecorderDeviceProxy,
                        HardwareModule.weldHeadDeviceProxy,
                        HardwareModule.serviceDeviceProxy,
                        HardwareModule.workflowDeviceProxy,
                        HardwareModule.inspectionDeviceProxy,
                        HardwareModule.storageDeviceProxy,
                        HardwareModule.idmDeviceProxy
                    ];
                    item.notificationServer = HardwareModule.deviceNotificationServer;
                    item.attributeModel = Qt.binding(function() { return pageItem.keyValueAttributeModel; });
                } else if (item)
                {
                    item.deviceProxies = [];
                    item.notificationServer = null;
                }
            }
            Connections {
                target: devicesViewLoader.item
                function onRestartRequired() {
                    pageItem.restartRequired = true;
                }
            }
        }
    }

    Component {
        id: headMonitorSettings
        Loader {
            active: false
            visible: ListView.isCurrentItem
            width: loader.width
            height: loader.height
            sourceComponent: HeadMonitor.LaserHeadsSettings {
                model: pageItem.laserHeadsModel
                showIconEditButton: true
            }
            onVisibleChanged: {
                if (visible) {
                    active = true;
                }
            }
        }
    }

    Component {
        id: configResultsView
        ContainerItemLoader {
            Component.onCompleted: {
                setSource("ConfigResultsPage.qml", {
                    "resultSettingModel": pageItem.resultsConfigModel,
                    "screenshotTool": pageItem.screenshotTool
                });
            }
        }
    }
    Component {
        id: configErrorsView
        ContainerItemLoader {
            Component.onCompleted: {
                setSource("ConfigErrorsPage.qml", {
                    "errorSettingModel": pageItem.errorSettingModel,
                    "screenshotTool": pageItem.screenshotTool
                });
            }
        }
    }
    Component {
        id: configSensorView
        ContainerItemLoader {
            Component.onCompleted: {
                setSource("ConfigResultsPage.qml", {
                    "resultSettingModel": pageItem.sensorConfigModel
                });
            }
        }
    }

    Component {
        id: backupView
        ContainerItemLoader {
            sourceComponent: BackupPage {}
        }
    }
    Component {
        id: restoreView
        ContainerItemLoader {
            sourceComponent: RestorePage {
                prepareFunction: HardwareModule.prepareRestore
                sideTabView: pageItem
                restorePermission: App.PerformRestore
                screenshotTool: pageItem.screenshotTool
                onRestartRequiredChanged: {
                    pageItem.restartRequired = restartRequired;
                }
            }
        }
    }
    Component {
        id: updateView
        ContainerItemLoader {
            sourceComponent: UpdatePage {
                sideTabView: pageItem
                archiveDirectoryPath: WeldmasterPaths.updateArchiveDir
                updatePermission: App.PerformUpdate
                onRestartRequiredChanged: {
                    pageItem.restartRequired = restartRequired;
                }
            }
        }
    }
    Component {
        id: ioView
        ContainerItemLoader {
            sourceComponent: EtherCAT.SlaveOverview {
                id: ioSlaveOverview
                serviceServer: HardwareModule.serviceServer
                additionalTabsModel: ListModel {
                    Component.onCompleted: {
                        append({
                            tabTitle: qsTr("Sps Simulation"),
                            source: "qrc:///resources/qml/SpsSimulation.qml",
                            visible: UserManagement.currentUser && UserManagement.hasPermission(App.ViewSpsSimulation),
                            sourceProperties: {
                                resultsConfigModel: pageItem.resultsConfigModel,
                                productModel: HardwareModule.productModel,
                                graphDir: WeldmasterPaths.graphDir
                            }
                        })
                    }
                }
                Connections {
                    target: UserManagement
                    function onCurrentUserChanged() {
                        ioSlaveOverview.additionalTabsModel.setProperty(0, "visible", UserManagement.currentUser && UserManagement.hasPermission(App.ViewSpsSimulation));
                    }
                }
            }
        }
    }
    Component {
        id: userLogView
        ContainerItemLoader {
            sourceComponent: UserLog.UserLogView {
                productName: Qt.application.name
                // from BackupPage
                logDirNameInBackup: "logs"
                screenshotTool: pageItem.screenshotTool
            }
        }
    }

    Component {
        id: localization
        ContainerItemLoader {
            sourceComponent: Localization {
                configFilePath: GuiConfiguration.configFilePath
                languages: GuiConfiguration.availableLanguages
                currentLanguage: GuiConfiguration.language

                onDateTimeChanged: pageItem.dateTimeChanged()
                onRestartRequired: {
                    pageItem.restartRequired = true;
                    GuiConfiguration.retranslate();
                }
            }
        }
    }

    Component {
        id: scheduler
        ContainerItemLoader {
            sourceComponent: Scheduler.Scheduler {
            }
        }
    }

    Component {
        id: networkConfiguration
        Loader {
            active: ListView.isCurrentItem
            width: loader.width
            height: loader.height

            sourceComponent: ColumnLayout {
                ScreenConfigurationController {
                    id: screenConfigurationController
                }
                TabBar {
                    id: systemConfigurationTabBar
                    Layout.fillWidth: true
                    TabButton {
                        text: qsTr("Network")
                        icon.name: "network-wired"
                    }
                    TabButton {
                        text: qsTr("Universal Power Supply")
                    }
                    TabButton {
                        text: qsTr("EATON UPS")
                        visible: SystemConfiguration.SOUVIS6000_Application
                        width: SystemConfiguration.SOUVIS6000_Application ? undefined : 0
                    }
                    TabButton {
                        text: qsTr("Hardware configuration backup")
                        icon.name: "document-save-all"
                    }
                    TabButton {
                        text: qsTr("SSH")
                        icon.name: "object-locked"
                    }
                    TabButton {
                        text: qsTr("Screen")
                        icon.name: "computer"
                    }
                }
                Component {
                    id: networkComponent
                    NetworkConfigurationPage {
                        externalConfiguration: UserManagement.currentUser && UserManagement.hasPermission(App.OpenSystemNetworkConfiguration) && screenConfigurationController.isNetworkManagementConfigurationAvailable()
                        onRestartRequired: {
                            pageItem.restartRequired = true;
                        }
                        onConfigurationRequested: screenConfigurationController.startNetworkConfiguration()
                    }
                }
                Component {
                    id: upsComponent
                    ColumnLayout {
                        UpsModel {
                            id: upsModel
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Label {
                                text: qsTr("Uninterruptible power supply:")
                            }
                            ComboBox {
                                id: upsComboBox
                                model: upsModel
                                textRole: "display"
                                currentIndex: upsModel.selectedIndex.row
                                onActivated: upsModel.select(index)
                                Layout.fillWidth: true
                            }
                        }
                        Image {
                            visible: upsComboBox.currentIndex != 0
                            source: visible ? "file://" + WeldmasterPaths.languageDir + "/images/ups/" + upsModel.data(upsModel.index(upsComboBox.currentIndex, 0), Qt.DecorationRole) : ""
                            fillMode: Image.PreserveAspectFit
                            Layout.maximumWidth: loader.width
                            Layout.fillHeight: true
                            Layout.alignment: Qt.AlignCenter
                        }
                        Item {
                            Layout.fillHeight: true
                        }
                        Button {
                            text: qsTr("Save UPS configuration")
                            icon.name: "document-save"
                            enabled: upsModel.modified
                            onClicked: {
                                upsModel.save();
                                pageItem.restartRequired = true;
                            }
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
                Component {
                    id: ippConfigurationComponent
                    IntelligentPowerProtector {
                    }
                }
                Component {
                    id: hardwareConfigurationComponent
                    HardwareConfigurationBackup {
                        onRestartRequired: {
                            pageItem.restartRequired = true;
                        }
                    }
                }
                Component {
                    id: sshConfigurationComponent

                    ColumnLayout {
                        SSH.HostKeys {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                        SSH.AuthorizedKeys {
                            relativePath: "/weldmaster/ssh/"
                            importKeyPermission: App.ImportSSHAuthorizedKey
                            removeKeyPermission: App.RemoveSSHAuthorizedKey
                            screenshotTool: pageItem.screenshotTool
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                        }
                    }
                }
                Component {
                    id: screenConfigurationComponent

                    Item {
                        ColumnLayout {
                            anchors.fill: parent
                            Label {
                                text: qsTr("External tool to configure screen. Only intended for virtual machines.")
                            }
                            Button {
                                text: qsTr("Start")
                                onClicked: screenConfigurationController.startExternalTool()
                            }
                            Button {
                                objectName: "configuration-screen-keyboard"
                                text: qsTr("Open external tool to configure keyboard layout")
                                onClicked: screenConfigurationController.startKeyboardConfiguration()
                                Component.onCompleted: {
                                    visible = screenConfigurationController.isKeyboardConfigurationAvailable()
                                }
                            }
                            Button {
                                text: qsTr("Open System debug console")
                                onClicked: screenConfigurationController.startDebugConsole()
                            }
                            Item {
                                Layout.fillHeight: true
                            }
                        }
                    }
                }
                Loader {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    function componentForIndex(index)
                    {
                        switch (index)
                        {
                        case 0:
                            if (UserManagement.currentUser && UserManagement.hasPermission(App.ConfigureNetworkDevices))
                            {
                                return networkComponent;
                            }
                            break;
                        case 1:
                            if (UserManagement.currentUser && UserManagement.hasPermission(App.ConfigureUps))
                            {
                                return upsComponent;
                            }
                            break;
                        case 2:
                            if (UserManagement.currentUser && SystemConfiguration.SOUVIS6000_Application)
                            {
                                return ippConfigurationComponent;
                            }
                            break;
                        case 3:
                            if (UserManagement.currentUser && UserManagement.hasPermission(App.BackupRestoreHardwareConfiguration))
                            {
                                return hardwareConfigurationComponent;
                            }
                            break;
                        case 4:
                            if (UserManagement.currentUser && UserManagement.hasPermission(App.ViewSSHConfiguration))
                            {
                                return sshConfigurationComponent;
                            }
                        case 5:
                            if (UserManagement.currentUser && UserManagement.hasPermission(App.ConfigureScreen))
                            {
                                return screenConfigurationComponent;
                            }
                        }
                        return undefined;
                    }
                    sourceComponent: componentForIndex(systemConfigurationTabBar.currentIndex)
                }
            }
        }
    }


    Component {
        id: remoteDesktop
        ContainerItemLoader {
            id: remoteDesktopLoader
            Component.onCompleted: {
                remoteDesktopLoader.setSource("RemoteDesktop.qml", {
                    "remoteDesktop": pageItem.remoteDesktop
                });
            }
            Connections {
                target: remoteDesktopLoader.item
                function onRestartRequired() {
                    pageItem.restartRequired = true;
                }
            }
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
            loader.addItem(changePasswordView.createObject(loader));
            loader.addItem(userManagementView.createObject(loader));
            loader.addItem(roleManagementView.createObject(loader));
            loader.addItem(devicesView.createObject(loader));
            loader.addItem(headMonitorSettings.createObject(loader));
            loader.addItem(configResultsView.createObject(loader));
            loader.addItem(configErrorsView.createObject(loader));
            loader.addItem(configSensorView.createObject(loader));
            loader.addItem(backupView.createObject(loader));
            loader.addItem(restoreView.createObject(loader));
            loader.addItem(updateView.createObject(loader));
            loader.addItem(licenseView.createObject(loader));
            loader.addItem(ioView.createObject(loader));
            loader.addItem(userLogView.createObject(loader));
            loader.addItem(localization.createObject(loader));
            loader.addItem(scheduler.createObject(loader));
            loader.addItem(networkConfiguration.createObject(loader));
            loader.addItem(remoteDesktop.createObject(loader));
        }
    }
}
