import QtQuick 2.7
import QtQuick.Window 2.2
import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.logging 1.0 as Logging
import precitec.gui.general 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui 1.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import "qrc:/resources/qml/"

import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.components.userLog 1.0 as UserLog
import precitec.gui.general 1.0
import precitec.laserHeadMonitor 1.0 as HeadMonitor

ApplicationWindow {
    id: applicationWindow
    property var lastUser: null
    width: 1024
    height: 1024
    flags: Qt.FramelessWindowHint

    signal updateSettings()

    function configureSeam(productId, seamSeriesId, seamId)
    {
        var index = topBarButtonModel.indexForItem(TopBarButtonModel.Wizard);
        topBar.tabBar.setCurrentIndex(topBar.model.mapFromSource(index).row);
        container.currentItem.item.configureSeam(productId, seamSeriesId, seamId);
    }

    function configureSeamSeries(productId, seamSeriesId)
    {
        var index = topBarButtonModel.indexForItem(TopBarButtonModel.Wizard);
        topBar.tabBar.setCurrentIndex(topBar.model.mapFromSource(index).row);
        container.currentItem.item.configureSeamSeries(productId, seamSeriesId);
    }

    PrecitecApplication.InputMethodEventFilter {
        enabled: !remoteDesktopController.enabled
        virtualKeyboardEnabled: GuiConfiguration.virtualKeyboard
    }

    RemoteDesktopController {
        id: remoteDesktopController
    }

    PrecitecApplication.GlobalFlickableEventFilter {
        onTouchBeginOnFlickable: {
            if (applicationWindow.activeFocusControl) {
                applicationWindow.activeFocusControl.focus = false;
            }
        }
    }

    GraphModel {
        id: rootGraphModel
        pdfFilesDir: WeldmasterPaths.pdfFilesDir
        macroGraphModel: rootMacroGraphModel
    }

    SubGraphModel {
        id: rootSubGraphModel
        pdfFilesDir: WeldmasterPaths.pdfFilesDir
        macroGraphModel: rootMacroGraphModel
    }

    GraphModel {
        id: rootMacroGraphModel
        Component.onCompleted: {
            rootMacroGraphModel.loadGraphs(WeldmasterPaths.systemMacroDir, WeldmasterPaths.userMacroDir);
        }
        onLoadingChanged: {
            if (!rootMacroGraphModel.loading)
            {
               rootGraphModel.loadGraphs(WeldmasterPaths.systemGraphDir, WeldmasterPaths.graphDir);
               rootSubGraphModel.loadSubGraphs(WeldmasterPaths.subGraphDir, WeldmasterPaths.userSubGraphDir);
            }
        }
    }

    AttributeModel {
        id: rootAttributeModel
        Component.onCompleted: {
            rootAttributeModel.loadDefault();
        }
    }

    AttributeModel {
        id: rootKeyValueAttributeModel
        Component.onCompleted: {
            rootKeyValueAttributeModel.loadDefaultKeyValue();
        }
    }

    ResultSettingModel {
        id: rootResultsConfigModel
    }

    ErrorSettingModel {
        id: rootErrorConfigModel
        productModel: HardwareModule.productModel
    }

    SensorSettingsModel {
        id: rootSensorSettingsModel
        configurationDirectory: WeldmasterPaths.configurationDir
    }

    QualityNormModel {
        id: rootQualityNormModel
        configurationDirectory: WeldmasterPaths.configurationDir
    }

    AxisInformation {
        id: yAxisInformation
        deviceNotificationServer: HardwareModule.deviceNotificationServer
        weldHeadSubscribeProxy: HardwareModule.weldHeadSubscribeProxy
        weldHeadServer: HardwareModule.weldHeadServer
        weldHeadDevice: HardwareModule.weldHeadDeviceProxy
        pollHeadInfo: axisEnabled && (HardwareModule.systemStatus.state == SystemStatusServer.Live || HardwareModule.systemStatus.state == SystemStatusServer.Automatic || HardwareModule.systemStatus.state == SystemStatusServer.Calibration)
    }


    HeadMonitor.LaserHeadModel {
        id: rootLaserHeadsModel
    }

    Component {
        id: screenshotAndHelpToolComponent
        RowLayout {
            HelpButton {
                id: helpButton
                drawer: onlineHelpPopup
                pdfFile: container.currentItem != null ? container.currentItem.pdfFile : OnlineHelp.HasNoPdf
                icon.name: "view-help"
                icon.color: PrecitecApplication.Settings.alternateText
                icon.width: Math.min(availableWidth, availableHeight)
                icon.height: Math.min(availableWidth, availableHeight)
                flat: true
                palette.button: "transparent"
                visible: container.currentItem != null ? (container.currentItem.pdfFile != OnlineHelp.HasNoPdf ) : false
            }
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
        enabled: HardwareModule.ready && LanguageSupport.ready && (!GuiConfiguration.blockedAutomatic || HardwareModule.systemStatus.state != SystemStatusServer.Automatic) && !blocked
        screenshotTool: screenshot
        systemInformationDialogTitle: qsTr("General system information")
        configFilePath: WeldmasterPaths.configurationDir
        model: topBarButtonFilterModel
        property bool blocked: false

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
            topBarButtonModel.setSimulationEnabled(SimulationModule.ready);
        }

        Connections {
            target: SimulationModule
            function onReadyChanged() {
                topBarButtonModel.setSimulationEnabled(SimulationModule.ready)
            }
        }
    }

    TopBarButtonModel {
        id: topBarButtonModel
    }

    TopBarButtonFilterModel {
        id: topBarButtonFilterModel
        sourceModel: topBarButtonModel

        showGraphEditor: GuiConfiguration.formatHardDisk
        headMonitorAvailable: rootLaserHeadsModel.size > 0
    }

    Logging.LogModel {
        id: logModel
        property bool scanTracker: false
        station: HardwareModule.station
        paused: (!logView.visible || logView.paused) && !logModel.scanTracker
        clearLogMessagesPermission: App.ClearLogMessages
    }
    Logging.LogModel {
        id: simulationLogModel
        station: SimulationModule.station
        paused: true
        clearLogMessagesPermission: App.ClearLogMessages
    }

    Component {
        id: homeComponent
        Overview {
            id: overviewPage
            visible: ListView.isCurrentItem
            width: container.width
            height: container.height
            resultsConfigModel: rootResultsConfigModel
            errorConfigModel: rootErrorConfigModel
            sensorConfigModel: rootSensorSettingsModel
            onPlotterSettingsUpdated: applicationWindow.updateSettings()
            inputEnabled: !onlineHelpPopup.visible
            screenshotTool: screenshot

            onConfigureSeam: applicationWindow.configureSeam(productId, seamSeriesId, seamId)

            Connections {
                target: applicationWindow
                function onUpdateSettings() {
                    updateSettings()
                }
            }
        }
    }

    Component {
        id: resultsComponent
        Loader {
            property var pdfFile: item ? item.pdfFile : OnlineHelp.HasNoPdf
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
            sourceComponent: ResultsPage {
                productModel: HardwareModule.productModel
                errorConfigModel: rootErrorConfigModel
                resultsConfigModel: rootResultsConfigModel
                sensorConfigModel: rootSensorSettingsModel
                screenshotTool: screenshot
                onPlotterSettingsUpdated: applicationWindow.updateSettings()
                onConfigureSeam: applicationWindow.configureSeam(productId, seamSeriesId, seamId)
                onConfigureSeamSeries: applicationWindow.configureSeamSeries(productId, seamSeriesId)

                Connections {
                    target: applicationWindow
                    function onUpdateSettings() {
                        updateSettings()
                    }
                }
            }
        }
    }

    Component {
        id: statisticsComponent
        Statistics {
            visible: ListView.isCurrentItem
            width: container.width
            height: container.height
            productModel: HardwareModule.productModel
            errorConfigModel: rootErrorConfigModel
        }
    }

    Component {
        id: simulationComponent
        Loader {
            property var pdfFile: item ? item.pdfFile : OnlineHelp.HasNoPdf
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
            sourceComponent: Simulation {
                logModel: simulationLogModel
                moduleModel: simulationLogModel.moduleModel
                errorConfigModel: rootErrorConfigModel
                sensorConfigModel: rootSensorSettingsModel
                graphModel: rootGraphModel
                subGraphModel: rootSubGraphModel
                attributeModel: rootAttributeModel
                keyValueAttributeModel: rootKeyValueAttributeModel
                resultsConfigModel: rootResultsConfigModel
                onlineHelp: onlineHelpPopup
                onPlotterSettingsUpdated: applicationWindow.updateSettings()
                screenshotTool: screenshot
                onSimulationItemEnabledChanged: {
                    applicationWindow.header.enabled = enabled;
                }

                Connections {
                    target: applicationWindow
                    function onUpdateSettings() {
                        updateSettings()
                    }
                }
            }
        }
    }
    Component {
        id: userComponent
        Loader {
            property var pdfFile: OnlineHelp.HasNoPdf
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
            property var pdfFile: item ? item.pdfFile : OnlineHelp.HasNoPdf
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
            sourceComponent: Configuration {
                remoteDesktop: remoteDesktopController
                resultsConfigModel: rootResultsConfigModel
                errorSettingModel: rootErrorConfigModel
                sensorConfigModel: rootSensorSettingsModel
                laserHeadsModel: rootLaserHeadsModel
                screenshotTool: screenshot
                keyValueAttributeModel: rootKeyValueAttributeModel
                onRestartRequiredChanged: {
                    restartRequiredItem.restartRequired = restartRequired;
                }
                onDateTimeChanged: clock.updateTime()
            }
        }
    }

    Component {
        id: haedMonitorComponent
        Loader {
            property var pdfFile: OnlineHelp.HasNoPdf
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
            sourceComponent: LaserHeadMonitor {
                laserHeadModel: rootLaserHeadsModel
            }
        }
    }

    Component {
        id: wizardComponent
        Loader {
            property var pdfFile: item ? item.pdfFile : OnlineHelp.HasNoPdf
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
            sourceComponent: Wizard {
                graphModel: rootGraphModel
                subGraphModel: rootSubGraphModel
                attributeModel: rootAttributeModel
                keyValueAttributeModel: rootKeyValueAttributeModel
                resultsConfigModel: rootResultsConfigModel
                errorConfigModel: rootErrorConfigModel
                sensorConfigModel: rootSensorSettingsModel
                qualityNormModel: rootQualityNormModel
                loggerModel: logModel
                onlineHelp: onlineHelpPopup
                screenshotTool: screenshot
                onPlotterSettingsUpdated: applicationWindow.updateSettings()
                Connections {
                    target: applicationWindow
                    function onUpdateSettings() {
                        updateSettings()
                    }
                }

                onWizardPageEnabledChanged: {
                    applicationWindow.header.enabled = enabled;
                }

                onScanTrackerVisibleChanged: {
                    logModel.scanTracker = scanTrackerVisible;
                }
            }
        }
    }

    Component
    {
        id: graphEditorComponent
        Loader
        {
            property var pdfFile: item ? item.pdfFile : OnlineHelp.HasNoPdf
            active: false
            visible: ListView.isCurrentItem
            width: container.width
            height: container.height
            onVisibleChanged:
            {
                if (visible)
                {
                    active = true;
                }
            }
            sourceComponent: GraphEditor
            {
                attributeModel: rootAttributeModel
                resultModel: rootResultsConfigModel
                sensorModel: rootSensorSettingsModel
                errorConfigModel: rootErrorConfigModel
                screenshotTool: screenshot
                onlineHelp: onlineHelpPopup
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
            container.addItem(resultsComponent.createObject(container));
            container.addItem(statisticsComponent.createObject(container));
            container.addItem(simulationComponent.createObject(container));
            container.addItem(haedMonitorComponent.createObject(container));
            container.addItem(wizardComponent.createObject(container));
            container.addItem(graphEditorComponent.createObject(container));
            container.addItem(configurationComponent.createObject(container));
        }
        id: container
        visible: HardwareModule.ready && LanguageSupport.ready
        enabled: !GuiConfiguration.blockedAutomatic || HardwareModule.systemStatus.state != SystemStatusServer.Automatic
        currentIndex: topBarButtonFilterModel.mapToSource(topBarButtonFilterModel.index(topBar.tabBar.currentIndex, 0)).row
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }

        states: State {
            name: "logViewVisible"
            when: logView.visible || downloadOverview.visible
            AnchorChanges {
                target: container
                anchors.bottom: logView.visible ? logView.top : downloadOverview.top
            }
        }
        Behavior on height { PropertyAnimation {} }

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

        onVisibleChanged: {
            if (!container.visible)
            {
                return;
            }
            container.addItems();
        }
    }

    Shortcut {
        enabled: HardwareModule.ready && LanguageSupport.ready
        sequence: "Ctrl+G"
        onActivated: {
            GuiConfiguration.formatHardDisk = !GuiConfiguration.formatHardDisk;
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

    BusyIndicator {
        anchors.centerIn: parent
        running: !container.visible
    }

    Logging.LogView {
        id: logView
        expandedHeight: applicationWindow.contentItem.height / 3
        pauseAvailable: !logModel.scanTracker
        model: logModel
        moduleModel: logModel.moduleModel
        viewDebugMessagesPermission: App.ViewDebugLogMessages
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        onStateChanged: {
            if (downloadOverview == "Visible")
            {
                if (downloadOverview.state == "Visible")
                {
                    downloadOverview.swapStates();
                }
            }
        }
    }

    Control {
        id: downloadOverview

        function swapStates()
        {
            if (state == "Invisible")
            {
                state = "Visible";
            } else
            {
                state = "Invisible";
            }
        }

        property real expandedHeight: applicationWindow.contentItem.height / 3
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        background: Rectangle {
            color: "#f0f0f0"
            border {
                width: 5
                color: "#f0f0f0"
            }
        }
        contentItem: RemovableDevices.DownloadOverview {
            cancelDownloadPermission: App.MountPortableDevices
        }

        state: "Invisible"
        onStateChanged: {
            if (state == "Visible")
            {
                if (logView.state == "Visible")
                {
                    logView.swapStates();
                }
            }
        }
        states: [
            State {
                name: "Visible"
                PropertyChanges {
                    target: downloadOverview
                    height: downloadOverview.expandedHeight
                    visible: true
                }
            },
            State {
                name: "Invisible"
                PropertyChanges {
                    target: downloadOverview
                    height: 0
                    visible: false
                }
            }
        ]
        transitions: [
            Transition {
                from: "Visible"
                to: "Invisible"
                SequentialAnimation {
                    PropertyAnimation {
                        target: downloadOverview
                        property: "height"
                    }
                    NumberAnimation {
                        target: downloadOverview
                        property: "visible"
                    }
                }
            },
            Transition {
                from: "Invisible"
                to: "Visible"
                PropertyAnimation {
                    target: downloadOverview
                    property: "height"
                }
            }
        ]
    }

    Notifications.NotificationsOverlay {
    }
    footer: PrecitecApplication.StatusBar {
        id: statusBar
        RowLayout {
            SystemStatusLabel {
                Layout.fillHeight: true
                systemStatus: HardwareModule.systemStatus
                textColor: PrecitecApplication.Settings.alternateText
            }
            Button {
                visible: HardwareModule.systemStatus.state == SystemStatusServer.NotReady && UserManagement.currentUser && UserManagement.hasPermission(App.ResetSystemStatus)
                text: qsTr("Reset system error")
                onClicked: {
                    HardwareModule.quitSystemFault()
                }
            }
        }
        ToolSeparator {
        }
        RowLayout {
            visible: yAxisInformation.axisEnabled
            AxisVisualization {
                Layout.preferredWidth: statusBar.width * 0.1
                Layout.bottomMargin: padding
                Layout.topMargin: padding
                visible: !yAxisInformation.requiresHoming && yAxisInformation.modeOfOperation != AxisInformation.Home
            }
            Label {
                visible: yAxisInformation.requiresHoming && yAxisInformation.modeOfOperation != AxisInformation.Home
                text: qsTr("Please reference y axis")
                color: PrecitecApplication.Settings.alternateText
            }
            BusyIndicator {
                visible: yAxisInformation.modeOfOperation == AxisInformation.Home
                palette.dark: PrecitecApplication.Settings.alternateText
                Layout.alignment: Qt.AlignVCenter
            }
            Label {
                visible: yAxisInformation.modeOfOperation == AxisInformation.Home
                text: qsTr("Referencing y axis")
                color: PrecitecApplication.Settings.alternateText
            }
            ToolSeparator {
            }
        }
        Logging.LogStatusBarItem {
            model: logModel
            view: logView
            Layout.fillWidth: true
        }
        ToolSeparator {}
        UserStatus {
            color: PrecitecApplication.Settings.alternateText
        }
        ToolSeparator {}
        ColumnLayout {
            Label {
                visible: !HardwareModule.systemStatus.productValid
                text: qsTr("No product")
                color: PrecitecApplication.Settings.alternateText
            }
            Label {
                visible: HardwareModule.systemStatus.productValid
                color: PrecitecApplication.Settings.alternateText
                text: HardwareModule.systemStatus.productName
            }
            Label {
                visible: HardwareModule.systemStatus.productValid
                color: PrecitecApplication.Settings.alternateText
                text: qsTr("Seam %1").arg(HardwareModule.systemStatus.visualSeam)
            }
        }
        ToolSeparator {}
        ToolButton {
            id: downloadToolButton
            objectName: "download-tool-button"
            visible: !RemovableDevices.DownloadManager.empty
            icon.name: "edit-download"
            display: Button.IconOnly
            flat: true
            onClicked: downloadOverview.swapStates()
            onVisibleChanged: {
                if (!visible && downloadOverview.state == "Visible")
                {
                    downloadOverview.swapStates();
                }
            }
        }
        ToolSeparator {
            visible: downloadToolButton.visible
        }
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
        }
        ToolSeparator {
            visible: restartRequiredItem.restartRequired
        }
        Label {
            text: qsTr("Remote access enabled")
            visible: remoteDesktopController.enabled
            color: PrecitecApplication.Settings.alternateText
        }
        ToolSeparator {
            visible: remoteDesktopController.enabled
        }
        PrecitecApplication.StatusBarClock {
            id: clock
            Layout.alignment: Qt.AlignVCenter
        }
    }

    Component.onCompleted: {
        applicationWindow.lastUser = UserManagement.currentUser;
        UserLog.GuiLog.logDirectory = WeldmasterPaths.logfilesDir;
        HardwareModule.initialize();
        SimulationModule.initialize();
    }

    PrecitecApplication.ScreenshotTool {
        id: screenshot
        maximumNumberOfScreenshots: GuiConfiguration.maximumNumberOfScreenshots
    }
    PrecitecApplication.OnlineHelp {
        id: onlineHelpPopup
        language: GuiConfiguration.uiLanguage
        directory: WeldmasterPaths.pdfFilesDir
    }

    Connections {
        target: HardwareModule
        enabled: SystemConfiguration.HeadMonitorGatewayEnable
        function onReadyChanged() {
            HeadMonitor.LogMonitor.persistLogs = false;
            HeadMonitor.HMStorage.storageFilePath = WeldmasterPaths.configurationDir + "HeadmonitorStorage.db";
            HeadMonitor.HMStorage.init();
        }
    }

    Connections {
        target: HeadMonitor.LogMonitor
        function onLogUpdate() {
            if (message.logLevel == 1)
            {
                HardwareModule.logInfo(message.message);
            }
            else if (message.logLevel == 2)
            {
                HardwareModule.logWarning(message.message);
            }
            else if (message.logLevel == 3)
            {
                HardwareModule.logError(message.message);
            }
            else if (message.logLevel == 4)
            {
                HardwareModule.logDebug(message.message);
            }
        }
    }
    Connections {
        target: Notifications.NotificationSystem
        function onRowsInserted(modelIndex, first, last) 
        {
            for (var i = first; i <= last; i++)
            {
                var message = Notifications.NotificationSystem.data(Notifications.NotificationSystem.index(i,0), "notification")
                if (message.level == Notifications.Notification.Error)
                {
                    HardwareModule.logError(message.message+"\n");
                }
                else if (message.level == Notifications.Notification.Warning)
                {
                    HardwareModule.logWarning(message.message+"\n");
                }
                else
                {
                    HardwareModule.logInfo(message.message+"\n");
                }
            }
        }
    }
}
