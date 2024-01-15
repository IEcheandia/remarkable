import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.general 1.0
import precitec.gui.filterparametereditor 1.0

import Precitec.AppGui 1.0
import precitec.gui 1.0

import grapheditor.components 1.0

import precitec.gui.components.postprocessing 1.0 as PostProcessing
/**
 * Wizard page for selecting the graph for a Seam
 **/
Item {
    id: detectionPage
    property alias currentSeam: controller.currentSeam
    property var triggerDelta: 0.0
    property alias attributeModel: filterParameterEditor.attributeModel
    property alias graphModel: controller.graphModel
    property alias subGraphModel: controller.subGraphModel
    property bool dialogVisible: false
    property alias resultsConfigModel: filterParameterEditor.resultsConfigModel
    property alias  errorConfigModel: filterParameterEditor.errorConfigModel
    property alias sensorConfigModel: filterParameterEditor.sensorConfigModel
    property int sideTabIndex
    property var modelIndexForGraphEditor: detectionPage.currentSeam ? controller.graphModel.indexFor(detectionPage.currentSeam.graph) : undefined;
    property alias onlineHelp: filterParameterEditor.onlineHelp
    property alias screenshotTool: image.screenshotTool

    property var sideTabModel: ListModel {
        ListElement {
            display: qsTr("Parameters")
            icon: "view-parameter"
        }
        ListElement {
            display: qsTr("Video\nPlotter")
            icon: "view-video-plot"
        }
        ListElement {
            display: qsTr("Video")
            icon: "view-video"
        }
        ListElement {
            display: qsTr("Plotter")
            icon: "view-plot"
        }
        ListElement {
            display: qsTr("Post Processing")
            icon: "view-parameter"
        }
    }

    signal markAsChanged()
    signal updateSettings()
    signal plotterSettingsUpdated()

    function handleLeave()
    {
        controller.liveMode = false;
        image.closeInfoBox();
    }

    function handleVisibleChanged() {
        controller.setHWResultsDisabled( disableAxisCheckBox.checked );
        if (!visible) {
            handleLeave()
        }
    }

    onVisibleChanged: handleVisibleChanged()
    Component.onCompleted: handleVisibleChanged()
    Component.onDestruction: handleLeave()

    SubGraphCheckedFilterModel {
        id: subGraphCheckedFilterModel
        sourceModel: controller.subGraphModel
    }

    AxisController {
        id: axisController
        axisInformation: yAxisInformation
        systemStatus: HardwareModule.systemStatus
    }

    ScanLabController {
        id: scanLabController

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        systemStatus: HardwareModule.systemStatus
        productModel: HardwareModule.productModel
        grabberDeviceProxy: SystemConfiguration.CameraInterfaceType == 1 ? HardwareModule.grabberDeviceProxy : null
        weldheadDeviceProxy: HardwareModule.weldHeadDeviceProxy
    }

    onCurrentSeamChanged: {
        if (currentSeam)
        {
            if (currentSeam.velocity)
            {
                triggerDelta = (currentSeam.triggerDelta / currentSeam.velocity) * 1000.0;
            } else {
                triggerDelta = 0.0;
            }
        }
        // back to first item
        stackView.pop(null);
    }

    DetectionController {
        id: controller
        attributeModel: detectionPage.attributeModel
        onMarkAsChanged: detectionPage.markAsChanged()

        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        productModel: HardwareModule.productModel
        systemStatus: HardwareModule.systemStatus
        workflowDeviceProxy: HardwareModule.workflowDeviceProxy
        grabberDeviceProxy: SystemConfiguration.CameraInterfaceType == 1 ? HardwareModule.grabberDeviceProxy : null
    }

    Connections {
        target: HardwareModule.recorder
        function onSampleData(sensorId, data, context) {
            latestResults.addSample(sensorId, data, context)
        }
    }

   
    
  /*  PostProcessing.PostProcessing {
        id: postProcessing
        dbProxy: SimulationModule.dbProxy        
        triggerCmdProxy: SimulationModule.triggerCmdProxy
        resultsProxy: SimulationModule.resultsProxy
        recorderProxy: SimulationModule.recorderProxy
        systemStatusProxy: SimulationModule.systemStatusProxy
        calibrationProxy: SimulationModule.calibrationProxy
        weldHeadMsgProxy: SimulationModule.weldHeadMsgProxy
        videoRecorderProxy: SimulationModule.videoRecorderProxy
        inspectionOutProxy: SimulationModule.inspectionOutProxy
        Component.onCompleted: {
            postProcessing.init();
        }
        
       // currentSeam: currentSeam
    }*/



    LatestResultsModel {
        id: latestResults
        resultsConfigModel: detectionPage.resultsConfigModel
        errorConfigModel: detectionPage.errorConfigModel
        sensorConfigModel: detectionPage.sensorConfigModel
        liveUpdate: detectionPage.visible
        resultsServer: HardwareModule.results
    }

    ColumnLayout {
        anchors.fill: parent
        CheckBox {
            id: disableAxisCheckBox
            text: qsTr("Disable Hardware")
            checked: true
            onClicked: {
                controller.setHWResultsDisabled( checked );
            }
        }
        RowLayout {
            id: mainPageLayout
            Layout.fillWidth: true
            Layout.fillHeight: true

            Item {
                id: imageItem
                visible: detectionPage.sideTabIndex != 3 && detectionPage.sideTabIndex != 4
                Layout.fillHeight: true
                Layout.fillWidth: detectionPage.sideTabIndex != 1
                Layout.minimumWidth: parent.width * 0.33
                ImageWithAxisControl {
                    id: image
                    axisController: axisController
                    scanLabController: scanLabController
                    opacity: controller.updating ? 0.5 : 1.0
                    anchors.fill: parent
                    hasInfoBox: true
                    filterInstances: controller.filterInstances
                    handlersEnabled: !detectionPage.dialogVisible
                }
                BusyIndicator {
                    anchors.centerIn: parent
                    running: controller.updating
                }
                TapHandler {
                    target: null
                    onSingleTapped: {
                        if (HardwareModule.systemStatus.state == SystemStatusServer.Live) {
                            controller.liveMode = false;
                        }
                    }
                }
                RoundButton {
                    id: imagePlayButton
                    objectName: "detection-image-playButton"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    visible: !controller.updating && HardwareModule.systemStatus.state != SystemStatusServer.Live
                    enabled: HardwareModule.systemStatus.state == SystemStatusServer.Normal
                    opacity: 0.8
                    icon.name: "media-playback-start"
                    icon.height: Math.min(parent.height, parent.width) / 6
                    icon.width: Math.min(parent.height, parent.width) / 6
                    icon.color: PrecitecApplication.Settings.iconColor
                    onClicked: controller.liveMode = true
                }
            }
            Item {
                Layout.fillHeight: true
                Layout.fillWidth: true
                visible: detectionPage.sideTabIndex == 1 ||detectionPage.sideTabIndex == 3
                PlotterComponent {
                    id: plotter
                    anchors.fill: parent
                    configFilePath: GuiConfiguration.configFilePath
                    resultsModel: latestResults
                    inputEnabled: !onlineHelp.visible && !detectionPage.dialogVisible
                    onPlotterSettingsUpdated: detectionPage.plotterSettingsUpdated()

                    Layout.preferredHeight: mainPageLayout.height - 2 * mainPageLayout.spacing
                    Layout.fillWidth: true

                    Connections {
                        target: latestResults.resultsServer
                        function onSeamInspectionStarted() {
                            latestResults.clear();
                            plotter.clear();
                        }
                    }
                    Connections {
                        target: detectionPage
                        function onUpdateSettings() {
                            plotter.updateSettings()
                        }
                    }
                }
                TapHandler {
                    target: null
                    onSingleTapped: {
                        if (HardwareModule.systemStatus.state == SystemStatusServer.Live) {
                            controller.liveMode = false;
                        }
                    }
                }
                RoundButton {
                    id: plotterPlayButton
                    objectName: "detection-plotter-playButton"
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    visible: !controller.updating && HardwareModule.systemStatus.state != SystemStatusServer.Live && !imageItem.visible
                    enabled: HardwareModule.systemStatus.state == SystemStatusServer.Normal
                    opacity: 0.8
                    icon.name: "media-playback-start"
                    icon.height: Math.min(parent.height, parent.width) / 6
                    icon.width: Math.min(parent.height, parent.width) / 6
                    icon.color: PrecitecApplication.Settings.iconColor
                    onClicked: controller.liveMode = true
                }
            }

            // Parameters
            StackView {
                id: stackView
                initialItem: algorithmComponent
                clip: true
                enabled: !controller.updating
                visible: detectionPage.sideTabIndex == 0 || detectionPage.sideTabIndex == 3 || detectionPage.sideTabIndex == 4

                Layout.fillHeight: true
                Layout.minimumWidth: detectionPage.width * 0.33
                Component.onCompleted: {
                    Layout.preferredWidth = Qt.binding(function() { return stackView.get(0).implicitWidth; });
                }
            }
            
            Item {
            id: postProcessingTabContent
            visible: detectionPage.sideTabIndex == 4 // This should be the index of your new tab
            Layout.fillHeight: true
            Layout.minimumWidth: detectionPage.width * 0.33
            // Define your layout and buttons here
            Column {
               
                Button {
                    text: "Start PostProcessing"
                    onClicked: {
                      //  console.log ("controller::currentSeam: " + controller.currentSeam);
                        SimulationModule.changeFilterParamAndTrigger(26, 0, filterParameterEditor.paramUuid, filterParameterEditor.paramValue);
                    }
                }
                anchors.fill: parent
            }
           // anchors.fill: parent
        }  
        }

        Label {
            text: qsTr("Processing-time: ") + HardwareModule.systemStatus.processingTime.toFixed(2) + "ms"
            color: HardwareModule.systemStatus.processingTime <= triggerDelta ? "black" : "red"
        }
        Label {
            text: qsTr("Available trigger-time: ") + triggerDelta.toFixed(2) + "ms"
        }
    }

    Component {
        id: configureSubGraphDialogComponent
        Dialog {
            id: configureSubGraphDialog
            property bool useSubGraph: detectionPage.currentSeam.usesSubGraph && detectionPage.subGraphModel.rowCount() > 0
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: Overlay.overlay.width * 0.8
            height: Overlay.overlay.height * 0.8
            modal: true
            standardButtons: Dialog.Cancel | Dialog.Ok
            onRejected: {
                controller.syncSubGraphToSeam();
                destroy();
            }
            onAccepted: {
                if (useSubGraph)
                {
                    controller.syncFromSubGraph();
                    controller.syncSubGraphToSeam();
                } else
                {
                    detectionPage.currentSeam.graph = preConfiguredGraphView.graphId;
                }
                destroy();
            }
            onAboutToShow: {
                detectionPage.dialogVisible = true;
            }
            onAboutToHide: {
                detectionPage.dialogVisible = false;
            }

            Connections {
                target: UserManagement
                function onCurrentUserChanged() {
                    configureSubGraphDialog.reject()
                }
            }

            function toggleUseSubGraph()
            {
                configureSubGraphDialog.useSubGraph = !configureSubGraphDialog.useSubGraph;
            }

            header: Control {
                padding: 5
                background: Rectangle {
                    color: PrecitecApplication.Settings.alternateBackground
                }
                contentItem: RowLayout {
                    width: parent.with
                    Button {
                        text: qsTr("Previous category")
                        icon.name: "go-previous"
                        visible: configureSubGraphDialog.useSubGraph
                        enabled: swipeView.currentIndex > 0
                        onClicked: swipeView.decrementCurrentIndex()
                    }
                    Label {
                        Layout.fillWidth: true
                        text: qsTr("Configure Processing Graph")
                        font.bold: true
                        color: PrecitecApplication.Settings.alternateText
                        horizontalAlignment: Text.AlignHCenter
                    }
                    Button {
                        id: preConfiguredGraphButton
                        text: configureSubGraphDialog.useSubGraph ? qsTr("Select pre-configured graph") : qsTr("Build graph from components")
                        onClicked: configureSubGraphDialog.toggleUseSubGraph()
                        visible: detectionPage.subGraphModel.rowCount() > 0
                    }
                    Button {
                        text: qsTr("Next category")
                        icon.name: "go-next"
                        visible: configureSubGraphDialog.useSubGraph
                        enabled: swipeView.currentIndex < swipeView.count - 1
                        onClicked: swipeView.incrementCurrentIndex()
                    }
                    ToolButton {
                        icon.name: "camera-photo"
                        icon.color: PrecitecApplication.Settings.alternateText
                        flat: true
                        palette.button: "transparent"
                        onClicked: {
                            if (detectionPage.screenshotTool)
                            {
                                detectionPage.screenshotTool.takeScreenshot();
                            }
                        }
                    }
                }
            }

            Item {
                anchors.fill: parent
                visible: !configureSubGraphDialog.useSubGraph

                ColumnLayout {
                    anchors.fill: parent

                    TextField {
                        id: searchGraph
                        placeholderText: qsTr("Search graph ...")
                        Layout.rightMargin: 5
                        Layout.fillWidth: true
                    }

                    GroupBox {
                        id: preConfiguredGraphView
                        property var graphId: detectionPage.currentSeam.graph
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        title: qsTr("Pre-configured graph")
                        ListView {
                            width: parent.width
                            height: parent.height

                            model: filterGraphFilterModel

                            delegate: RadioButton {
                                width: parent.width
                                checkable: true
                                checked: model.uuid == preConfiguredGraphView.graphId
                                text: model.name
                                onToggled: {
                                    preConfiguredGraphView.graphId = model.uuid
                                    modelIndexForGraphEditor = detectionPage.graphModel.index(model.index, 0)
                                }
                            }
                        }
                    }

                    FilterGraphFilterModel {
                        id: filterGraphFilterModel
                        sourceModel: detectionPage.graphModel
                        searchText: searchGraph.text
                    }
                }
            }




            Item {
                id: graphComponentsView
                anchors.fill: parent
                visible: configureSubGraphDialog.useSubGraph
                ColumnLayout{
                    anchors.fill : parent
                    RowLayout{
                        spacing: 20
                        TextField {
                            id: searchSubGraph
                            placeholderText: qsTr("Search graph ...")
                            Layout.fillWidth: true
                            Layout.fillHeight: false
                            Layout.rightMargin: 5
                            width: parent.width
                        }

                        GroupBox {
                            Layout.fillWidth: true
                            Layout.topMargin: searchSubGraph.height/2
                            label: CheckBox {
                                id: showDisabledheckBox
                                text: qsTr("Show disabled graphs")
                            }
                        }
                    }

                    SwipeView {
                        id: swipeView
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true

                        Repeater {
                            id: repeater
                            model: detectionPage.subGraphModel.availableCategories
                            GraphComponentSelector {
                                title: detectionPage.subGraphModel.categoryToName(modelData)
                                category: modelData
                                subGraphModel: detectionPage.subGraphModel
                                showDisabled: showDisabledheckBox.checked
                                searchText: searchSubGraph.text
                            }
                        }
                    }

                    FilterGraphFilterModel {
                        id: filterSubGraphFilterModel
                        sourceModel: detectionPage.subGraphModel
                        searchText: searchSubGraph.text
                    }

                    PageIndicator {
                        count: swipeView.count
                        currentIndex: swipeView.currentIndex
                        Layout.alignment: Qt.AlignBottom | Qt. AlignHCenter
                    }
                }
            }
        }
    }

    Component {
        id: algorithmComponent

        SubGraphFilterConfiguration {
            exportSupported: UserManagement.currentUser && UserManagement.hasPermission(App.ExportGraph) && RemovableDevices.Service.udi != ""
            graphModel: detectionPage.graphModel
            subGraphModel: detectionPage.subGraphModel
            currentSeam: detectionPage.currentSeam
            onPreconfiguredGraphSelected: filterParameterEditor.pushPreConfiguredGraph()
            onSubGraphSelected: filterParameterEditor.pushSubGraph(uuid, name)
            onExportSelected: controller.exportCurrentGraph(RemovableDevices.Service.path)
            onlineHelp: detectionPage.onlineHelp
            onEditGraphSelected: {
                var dialog = configureSubGraphDialogComponent.createObject(detectionPage);
                dialog.open();
            }
            onShowSelectedSubGraph:
            {
                var dialog = graphEditor.createObject(detectionPage);
                dialog.open();
            }
        }
    }

    FilterParameterEditor {
        id: filterParameterEditor
        property var paramUuid
        property var paramValue
        graphModel: detectionPage.graphModel
        subGraphModel: detectionPage.subGraphModel
        graphId: controller.currentGraphId
        view: stackView
        getFilterParameter: controller.getFilterParameter

        onParameterValueChanged: {
            controller.updateFilterParameter(uuid, value);
            SimulationModule.setSeam(controller.currentSeam);
         //   SimulationModule.changeFilterParamAndTrigger(26, 0,  uuid, value);
            paramUuid = uuid;
            paramValue = value;
            image.updateInfoBox();
        }
        onDialogVisibleChanged: {
            detectionPage.dialogVisible = filterParameterEditor.dialogVisible;
        }
    }

    Component
    {
        id: graphEditor
        Dialog
        {
            id: graphEditorDialog
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: Overlay.overlay.width * 0.8
            height: Overlay.overlay.height * 0.8
            modal: true
            onRejected:
            {
                filterGraphView.visualizedGraph.destroyPortDelegate();
                controller.syncSubGraphToInterval();
                destroy();
            }

            Connections
            {
                target: UserManagement
                function onCurrentUserChanged() {
                    configureSubGraphDialog.reject()
                }
            }

            footer: DialogButtonBox
            {
                alignment: Qt.AlignRight
                Button
                {
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                }
            }

            header: Control
            {
                padding: 5
                background: Rectangle
                {
                    color: PrecitecApplication.Settings.alternateBackground
                }
                contentItem: RowLayout
                {
                    width: parent.width
                    Label
                    {
                        Layout.fillWidth: true
                        text: qsTr("Show selected graph")
                        font.bold: true
                        color: PrecitecApplication.Settings.alternateText
                        horizontalAlignment: Text.AlignHCenter
                    }
                    ToolButton
                    {
                        icon.name: "camera-photo"
                        icon.color: PrecitecApplication.Settings.alternateText
                        flat: true
                        palette.button: "transparent"
                        onClicked: {
                            if (detectionPage.screenshotTool)
                            {
                                detectionPage.screenshotTool.takeScreenshot();
                            }
                        }
                    }
                    ToolButton
                    {
                        icon.name: "document-export"
                        icon.color: PrecitecApplication.Settings.alternateText
                        flat: true
                        palette.button: "transparent"
                        onClicked: graphImageSaver.getDynamicImage();
                    }
                    ToolButton
                    {
                        icon.name: "zoom-fit-best"
                        icon.color: PrecitecApplication.Settings.alternateText
                        flat: true
                        palette.button: "transparent"
                        onClicked: graphImageSaver.focusGraph();
                    }
                }
            }

            GraphLoader
            {
                id: rootGraphLoader
            }

            GraphModelVisualizer
            {
                id: graphVisualizer
                graphLoader: rootGraphLoader
                filterGraph: filterGraphView.visualizedGraph
                graphFilterModel: graphFilterModel
            }

            FilterGraphImageSaver
            {
                id: graphImageSaver
                graphView: filterGraphView.qanGraphView
                pathForImages: WeldmasterPaths.resultsBaseDir
            }

            GroupBox
            {
                id: graphEditorView
                anchors.fill: parent
                FilterGraphView
                {
                    id: filterGraphView
                    anchors.fill: parent
                }
            }

            GraphFilterModel
            {
                id: graphFilterModel
                filterImagePath: WeldmasterPaths.filterPictureDir
            }

            Connections
            {
                target: graphFilterModel
                function onReady()
                {
                    graphVisualizer.startUp();
                    graphVisualizer.showGraph();
                }
            }

            Component.onCompleted:
            {
                rootGraphLoader.loadGraphFromModel(modelIndexForGraphEditor);
                graphFilterModel.init(WeldmasterPaths.filterLibDir);
            }
        }   //Dialog
    }   //Component
}
