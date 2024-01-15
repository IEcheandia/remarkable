
import QtQuick 2.0
import QtQml 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import precitec.gui.components.application 1.0
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.logging 1.0 as Logging
import precitec.gui.components.userManagement 1.0
import precitec.gui.general 1.0
import Precitec.AppGui 1.0
import "qrc:/resources/qml/"
import precitec.gui.components.plotter 1.0
import precitec.gui 1.0
import precitec.gui.filterparametereditor 1.0

SideTabView {
    property alias PostProcessingController: imageFilterModel.PostProcessingController
    property alias logModel: logView.model
    property alias moduleModel: logView.moduleModel
    property alias currentProduct: latestResults.currentProduct
    property alias errorConfigModel: latestResults.errorConfigModel
    property alias sensorConfigModel: latestResults.sensorConfigModel
    property alias attributeModel: filterParameterEditor.attributeModel
    property var keyValueAttributeModel: null
    property alias graphModel: filterParameterEditor.graphModel
    property alias subGraphModel: filterParameterEditor.subGraphModel
    property alias serialNumber: systemStatusItem.serialNumber
    property var pdfFile: OnlineHelp.HasNoPdf
    property alias resultsConfigModel: filterParameterEditor.resultsConfigModel
    property alias onlineHelp: filterParameterEditor.onlineHelp
    property alias screenshotTool: imageItem.screenshotTool
    property alias hasNextProductInstance: systemStatusItem.hasNextProductInstance
    property alias hasPreviousProductInstance: systemStatusItem.hasPreviousProductInstance
    property string partNumber: ""

    signal updateSettings()
    signal plotterSettingsUpdated()
    signal previousProductInstanceSelected()
    signal nextProductInstanceSelected()

    id: PostProcessingViewer

    onUpdateSettings: plotter.updateSettings()

    alignment: Qt.AlignRight

    model: ListModel {
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
            display: qsTr("Parameters")
            icon: "view-parameter"
        }
    }

    onVisibleChanged: {
        if (!visible && PostProcessingController && PostProcessingController.isPlaying)
        {
            PostProcessingController.pause();
        }
        if (!visible)
        {
            imageItem.closeInfoBox();
        } else
        {
            forceActiveFocus();
        }
    }

    Connections {
        target: HardwareModule.systemStatus
        enabled: GuiConfiguration.blockedAutomatic && PostProcessingController.isPlaying
        function onStateChanged()
        {
            if (HardwareModule.systemStatus.state == SystemStatusServer.Automatic)
            {
                PostProcessingController.pause();
            }
        }
    }

    function clearResults() {
        latestResults.clear();
        latestProductErrors.clear();
    }

    function previousImage()
    {
        PostProcessingViewer.clearResults();
        PostProcessingController.previous();
    }

    function nextImage()
    {
        PostProcessingController.next();
    }

    function previousSeam()
    {
        PostProcessingViewer.clearResults();
        PostProcessingController.previousSeam();
    }

    function nextSeam()
    {
        PostProcessingViewer.clearResults();
        PostProcessingController.nextSeam();
    }

    function play()
    {
        PostProcessingController.play();
    }

    function pause()
    {
        PostProcessingController.pause();
    }

    function stop()
    {
        PostProcessingViewer.clearResults();
        PostProcessingController.stop();
    }

    Keys.onPressed: {
        if ((event.key == Qt.Key_Left || event.key == Qt.Key_MediaPrevious) && PostProcessingController.hasPrevious)
        {
            PostProcessingViewer.previousImage();
        }
        if ((event.key == Qt.Key_Right || event.key == Qt.Key_MediaNext) && PostProcessingController.hasNext)
        {
            PostProcessingViewer.nextImage();
        }
        if (event.key == Qt.Key_Home)
        {
            PostProcessingViewer.clearResults();
            PostProcessingController.jumpToFrame(0);
        }
        if (event.key == Qt.Key_End)
        {
            PostProcessingViewer.clearResults();
            PostProcessingController.jumpToFrame(PostProcessingController.pathModel.rowCount() - 1);
        }
        if (event.key == Qt.Key_PageUp)
        {
            PostProcessingViewer.previousSeam();
        }
        if (event.key == Qt.Key_PageDown)
        {
            PostProcessingViewer.nextSeam();
        }
        if (event.key == Qt.Key_Space || event.key == Qt.Key_MediaTogglePlayPause)
        {
            if (PostProcessingController.isPlaying)
            {
                PostProcessingViewer.play();
            } else
            {
                PostProcessingViewer.pause();
            }
        }
        if (event.key == Qt.Key_MediaPlay && !PostProcessingController.isPlaying)
        {
            PostProcessingViewer.play();
        }
        if (event.key == Qt.Key_MediaStop)
        {
            PostProcessingViewer.stop();
        }
        if (event.key == Qt.Key_MediaPause && PostProcessingController.isPlaying)
        {
            PostProcessingViewer.pause();
        }
        event.accepted = true;
    }

    Connections {
        target: PostProcessingController
        function onProductInstanceChanged() {
            PostProcessingViewer.clearResults()
        }
    }

    FilterParameterEditor {
        id: filterParameterEditor

        graphId: PostProcessingController.currentGraphId
        view: filterParamsStackView
        sensorConfigModel: PostProcessingViewer.sensorConfigModel
        errorConfigModel: PostProcessingViewer.errorConfigModel
        getFilterParameter: PostProcessingController.getFilterParameter

        onParameterValueChanged: {
            PostProcessingController.updateFilterParameter(uuid, value);
            imageItem.updateInfoBox();
        }
    }

    Connections {
        target: SimulationModule.recorder
        function onSampleData(sensorId, data, context) {
            latestResults.addSample(sensorId, data, context)
        }
        function onImageDataChanged(imageNumber) {
            PostProcessingViewer.PostProcessingController.lastProcessedImage = imageNumber;
        }
    }

    LatestResultsModel {
        id: latestResults
        resultsServer: SimulationModule.results
        resultsConfigModel: PostProcessingViewer.resultsConfigModel
        liveUpdate: PostProcessingViewer.visible
    }

    LatestProductErrorsModel {
        id: latestProductErrors
        resultsServer: SimulationModule.results
        liveUpdate: PostProcessingViewer.visible
        errorConfigModel: PostProcessingViewer.errorConfigModel
    }

    Component {
        id: algorithmComponent

        SubGraphFilterConfiguration {
            editGraph: false
            graphModel: PostProcessingViewer.graphModel
            subGraphModel: PostProcessingViewer.subGraphModel
            onlineHelp: PostProcessingViewer.onlineHelp
            currentSeam: PostProcessingController.currentSeam
            onPreconfiguredGraphSelected: filterParameterEditor.pushPreConfiguredGraph()
            onSubGraphSelected: filterParameterEditor.pushSubGraph(uuid, name)
        }
    }

    Component {
        id: selectSeamDialogComponent
        Dialog {
            id: selectSeamDialog
            parent: Overlay.overlay
            width: parent.width * 0.5
            height: parent.height * 0.5
            anchors.centerIn: parent
            title: qsTr("Select Seam")
            modal: true
            closePolicy: Popup.CloseOnEscape

            // workaround for https://bugreports.qt.io/browse/QTBUG-72372
            footer: DialogButtonBox {
                alignment: Qt.AlignRight
                Button {
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                }
            }
            header: DialogHeaderWithScreenshot {
                title: selectSeamDialog.title
                screenshotTool: PostProcessingViewer.screenshotTool
            }

            onRejected: destroy()

            Connections {
                target: UserManagement
                function onCurrentUserChanged() {
                    selectSeamDialogComponent.reject()
                }
            }

            ProductSeamModel {
                id: productSeamModel
                product: seamLabel.product
            }

            ScrollView {
                anchors.fill: parent
                ListView {
                    anchors.fill: parent
                    model: productSeamModel
                    clip: true
                    delegate: RowLayout {
                        spacing: 0
                        width: ListView.view.width
                        enabled: PostProcessingController.pathModel.containsImagesForSeam(model.seam.seamSeries.number, model.seam.number)
                        ItemDelegate {
                            id: linkLabel
                            visible: model.seam.linkTo != undefined
                            icon.name: "link"
                            text: visible ? qsTr("%1 (%2)").arg(model.seam.linkTo.name).arg(model.seam.linkTo.visualNumber) : ""
                            down: pressed || mainLabel.pressed
                            onClicked: mainLabel.clicked()
                        }
                        ItemDelegate {
                            id: mainLabel
                            Layout.fillWidth: true
                            text: model.seam.name + " (" + model.seam.visualNumber + ")"
                            down: pressed || linkLabel.pressed
                            onClicked: {
                                PostProcessingViewer.clearResults();
                                PostProcessingController.jumpToFrame(PostProcessingController.pathModel.firstImageOfSeam(model.seamSeries.number, model.seam.number));
                                selectSeamDialog.destroy();
                            }
                        }
                    }
                    section.property: GuiConfiguration.seamSeriesOnProductStructure ? "seamSeriesName" : ""
                    section.delegate: Label {
                        height: implicitHeight
                        width: ListView.view.width
                        text: section
                        font.bold: true
                        horizontalAlignment: Text.AlignCenter
                    }
                }
            }
        }
    }

    Component {
        id: simulationSettingsDialogComponent
        Dialog {
            id: simulationSettingsDialog
            parent: Overlay.overlay
            width: parent.width * 0.2
            height: parent.height * 0.5
            anchors.centerIn: parent
            title: qsTr("Simulation Settings")

            // workaround for https://bugreports.qt.io/browse/QTBUG-72372
            footer: DialogButtonBox {
                alignment: Qt.AlignRight
                Button {
                    text: qsTr("Close")
                    DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
                }
            }
            header: DialogHeaderWithScreenshot {
                title: simulationSettingsDialog.title
                screenshotTool: PostProcessingViewer.screenshotTool
            }

            onRejected: destroy()

            Connections {
                target: UserManagement
                function onCurrentUserChanged() {
                    simulationSettingsDialog.reject()
                }
            }

            contentItem: GridLayout {
                columns: 2

                Label {
                    Layout.columnSpan: 2
                    text: qsTr("Simulation Duration")
                    font.bold: true
                }

                ComboBox {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    model: [qsTr("Current Seam"), qsTr("Entire Product")]
                    currentIndex: PostProcessingController.playEntireProduct
                    onCurrentIndexChanged: {
                        PostProcessingController.playEntireProduct = currentIndex == 1;
                    }
                }

                Label {
                    Layout.columnSpan: 2
                    text: qsTr("Simulation Speed")
                    font.bold: true
                }

                SpinBox {
                    Layout.fillWidth: true
                    value: PostProcessingController.framesPerSecond
                    from: 1
                    to: 999
                    editable: true
                    onValueModified: {
                        PostProcessingController.framesPerSecond = value;
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: qsTr("FPS")
                }

                Label {
                    Layout.columnSpan: 2
                    text: qsTr("Current Frame")
                    font.bold: true
                }

                TextField {
                    property bool frameFound: PostProcessingController.pathModel.indexOfFrame(imageFilterModel.seamSeries, imageFilterModel.seam, Number.fromLocaleString(locale, jumpToFrameField.text)).valid
                    Layout.fillWidth: true
                    id: jumpToFrameField
                    text: imageFilterModel.data(imageFilterModel.index(imageFilterModel.currentFrameIndex, 0), Qt.UserRole + 2)
                    validator: IntValidator {
                        bottom: imageFilterModel.data(imageFilterModel.index(0, 0), Qt.UserRole + 2)
                        top: imageFilterModel.data(imageFilterModel.index(imageFilterModel.rowCount() - 1, 0), Qt.UserRole + 2)
                    }
                    palette.text: jumpToFrameField.acceptableInput || frameFound ? "black" : "red"
                    onAccepted: {
                        if (acceptableInput && frameFound) {
                            PostProcessingController.jumpToFrame(PostProcessingController.pathModel.indexOfFrame(imageFilterModel.seamSeries, imageFilterModel.seam, Number.fromLocaleString(locale, jumpToFrameField.text)).row)
                        }
                    }
                }

                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "select"
                    enabled: jumpToFrameField.frameFound
                    onClicked: {
                        PostProcessingController.jumpToFrame(PostProcessingController.pathModel.indexOfFrame(imageFilterModel.seamSeries, imageFilterModel.seam, Number.fromLocaleString(locale, jumpToFrameField.text)).row)
                    }
                }

                Item {
                    Layout.columnSpan: 2
                    Layout.fillHeight: true
                }
            }
        }
    }

    SimulationImageFilterModel {
        id: imageFilterModel
    }

    Loader {
        id: referenceImageLoader

        sourceComponent: ReferenceImage {

            header: DialogHeaderWithScreenshot {
                screenshotTool: PostProcessingViewer.screenshotTool
            }

            property var product: PostProcessingController.productModel.findProduct(PostProcessingController.product)

            parent: Overlay.overlay
            anchors.centerIn: parent
            implicitWidth: 0.9 * parent.width
            implicitHeight: 0.9 * parent.height

            currentSeam: product ? product.findSeam(imageFilterModel.seamSeries, imageFilterModel.seam) : null
            attributeModel: PostProcessingViewer.keyValueAttributeModel
            screenshotTool: PostProcessingViewer.screenshotTool
        }
    }

    contentItem: Item {

        Logging.LogView {
            id: logView
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
                leftMargin: 5
                rightMargin: 5
            }
            expandedHeight: PostProcessingViewer.height * 0.4
            z: 10
            viewDebugMessagesPermission: App.ViewDebugLogMessages
            Binding {
                target: PostProcessingViewer.logModel
                when: logView.visible
                restoreMode: Binding.RestoreBinding
                property: "paused"
                value: logView.paused
            }
            onVisibleChanged: {
                if (PostProcessingViewer.logModel && !visible)
                {
                    PostProcessingViewer.logModel.paused = true;
                }
            }
        }

        GroupBox {
            anchors {
                fill: parent
                margins: 5
            }

            id: simulationBox

            label: ColumnLayout {
                Label {
                    id: seamLabel
                    property var product: PostProcessingController.productModel.findProduct(PostProcessingController.product)
                    property var seam : product ? product.findSeam(imageFilterModel.seamSeries, imageFilterModel.seam) : null
                    text: seam ? qsTr("Seam \"%1\" (#%2)").arg(seam.name).arg(seam.visualNumber) : qsTr("No Seam")
                    font.family: simulationBox.font.family
                    font.pixelSize: simulationBox.font.pixelSize
                    font.bold: true
                }
                Label {
                    visible: seamLabel.seam && seamLabel.seam.linkTo != undefined
                    text: visible ? qsTr("Linked to seam \"%1\" (#%2)").arg(seamLabel.seam.linkTo.name).arg(seamLabel.seam.linkTo.visualNumber) : ""
                }
                Label {
                    text: qsTr("Product \"%1\" (SN: %2)").arg(SimulationModule.systemStatus.productName).arg(PostProcessingViewer.serialNumber)
                }
                Label {
                    visible: PostProcessingViewer.partNumber != ""
                    //: a label containing a part number (argument is the part number)
                    text: qsTr("Part Number: %1").arg(PostProcessingViewer.partNumber)
                }
            }

            RowLayout {
                anchors.fill: parent

                opacity: enabled ? 1.0 : 0.5
                enabled: !PostProcessingController.loading && SimulationModule.systemStatus.productValid

                ColumnLayout {
                    Layout.maximumWidth: 150
                    Layout.minimumWidth: 150

                    SimulationThumbnailBar {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        model: imageFilterModel

                        onFrameSelected: {
                            PostProcessingViewer.clearResults();
                            PostProcessingController.jumpToFrame(imageFilterModel.mapToSource(imageFilterModel.index(index, 0)).row);
                        }
                    }

                    ToolButton {
                        Layout.fillWidth: true

                        display: AbstractButton.IconOnly
                        icon.name: "select"
                        onClicked: {
                            var dialog = selectSeamDialogComponent.createObject(PostProcessingViewer);
                            dialog.open();
                        }
                    }

                    ToolButton {
                        Layout.fillWidth: true

                        display: AbstractButton.IconOnly
                        icon.name: "application-menu"
                        onClicked: {
                            var dialog = simulationSettingsDialogComponent.createObject(PostProcessingViewer);
                            dialog.open();
                        }
                    }
                }

                GridLayout {

                    columns: 1 + (PostProcessingViewer.currentIndex == 0 || PostProcessingViewer.currentIndex == 3 ? 1 : 0)

                    PrecitecImage.Image {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        id: imageItem

                        visible: PostProcessingViewer.currentIndex != 2
                        hasInfoBox: true
                        referenceEnabled: true
                        handlersEnabled: !referenceImageLoader.item.visible
                        filterInstances: PostProcessingViewer.PostProcessingController.filterInstances
                        PostProcessingController: PostProcessingViewer.PostProcessingController
                        simulation: true

                        onCompareReference: referenceImageLoader.item.open()

                        onPlay: PostProcessingViewer.play()
                        onPause: PostProcessingViewer.pause()
                        onStop: PostProcessingViewer.stop()
                        onNextSeam: PostProcessingViewer.nextSeam()
                        onPreviousSeam: PostProcessingViewer.previousSeam()
                        onNext: PostProcessingViewer.nextImage()
                        onPrevious: PostProcessingViewer.previousImage()
                    }

                    PlotterComponent {
                        Layout.fillHeight: true
                        Layout.fillWidth: true

                        id: plotter

                        configFilePath: GuiConfiguration.configFilePath
                        visible: PostProcessingViewer.currentIndex == 0 || PostProcessingViewer.currentIndex == 2
                        resultsModel: latestResults
                        errorsModel: latestProductErrors
                        onPlotterSettingsUpdated: PostProcessingViewer.plotterSettingsUpdated()
                        inputEnabled: !onlineHelp.visible
                    }

                    ColumnLayout {
                        Layout.minimumWidth: PostProcessingViewer.width * 0.33
                        Layout.fillWidth: false
                        Layout.fillHeight: true

                        id: informationColumn

                        visible: PostProcessingViewer.currentIndex == 3

                        StatusInformation {
                            id: systemStatusItem
                            Layout.fillWidth: true
                            systemStatus: SimulationModule.systemStatus
                            onQuitSystemFault: SimulationModule.quitSystemFault()
                            onPreviousProductInstanceSelected: PostProcessingViewer.previousProductInstanceSelected()
                            onNextProductInstanceSelected: PostProcessingViewer.nextProductInstanceSelected()
                        }

                        RowLayout {
                            Layout.alignment: Qt.AlignRight

                            ToolButton {
                                display: AbstractButton.IconOnly
                                enabled: PostProcessingController.changes
                                icon {
                                    name: "edit-undo"
                                    width: 64
                                    height: 64
                                }
                                onClicked: PostProcessingController.discardChanges()

                                Layout.alignment: Qt.AlignRight
                            }

                            ToolButton {
                                display: AbstractButton.IconOnly
                                enabled: PostProcessingController.changes
                                icon {
                                    name: "document-save"
                                    width: 64
                                    height: 64
                                }
                                onClicked: {
                                    PostProcessingController.saveProductChange();
                                }

                                Layout.alignment: Qt.AlignRight
                            }
                        }

                        StackView {
                            id: filterParamsStackView
                            clip: true
                            initialItem: algorithmComponent

                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            Connections {
                                target: PostProcessingController
                                function onProductInstanceChanged() {
                                    filterParamsStackView.pop(null)
                                }
                                function onCurrentGraphIdChanged() {
                                    filterParamsStackView.pop(null)
                                }
                            }
                        }
                    }

                    PrecitecApplication.PlayerControls {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.columnSpan: 1 + (PostProcessingViewer.currentIndex == 0 || PostProcessingViewer.currentIndex == 3 ? 1 : 0)
                        objectName: PostProcessingViewer.objectName + "-player-controls"
                        controller: PostProcessingController
                        onPlay: PostProcessingViewer.play()
                        onPause: PostProcessingViewer.pause()
                        onStop: PostProcessingViewer.stop()
                        onNextSeam: PostProcessingViewer.nextSeam()
                        onPreviousSeam: PostProcessingViewer.previousSeam()
                        onNext: PostProcessingViewer.nextImage()
                        onPrevious: PostProcessingViewer.previousImage()
                    }
                }
            }
        }

        ToolButton {
            id: showLoggerButton
            anchors {
                top: logView.visible ? logView.bottom : parent.top
                horizontalCenter: parent.horizontalCenter
            }
            display: AbstractButton.IconOnly
            icon.name: logView.visible ? "arrow-up" : "arrow-down"
            onClicked: logView.swapStates()

            background: Rectangle {
                id: backgroundRect

                implicitWidth: 40
                implicitHeight: 40

                opacity: showLoggerButton.down ? 1.0 : 0.5
                color: showLoggerButton.down || showLoggerButton.checked || showLoggerButton.highlighted ? showLoggerButton.palette.mid : showLoggerButton.palette.button

                SequentialAnimation {
                    loops: 3
                    running: visible && logView.hasUnreadWarning
                    ColorAnimation {target: backgroundRect; property: "color"; from: "red"; to: "transparent"; duration: 500 }
                    ColorAnimation {target: backgroundRect; property: "color"; from: "transparent"; to: "red"; duration: 500 }
                }
            }
        }

        BusyIndicator {
            anchors.centerIn: parent
            running: PostProcessingController.loading || !SimulationModule.systemStatus.productValid
        }
    }
}
