import QtQuick 2.15
import QtQml 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.image 1.0 as PrecitecImage
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0

/**
 * Control to configure a SeamSeries.
 **/
BreadCrumpGroupBox {
    id: root
    property var controller: null
    /**
     * The SeamSeries which should be configured
     **/
    property var seamSeries: null
    property var attributeModel: null
    property var wizardFilterModel: null
    property var screenshotTool: null
    readonly property bool scanMasterMode: SystemConfiguration.Scanner2DEnable && SystemConfiguration.ScannerGeneralMode == SystemConfiguration.ScannerGenMode.ScanMaster

    property Component additionalButtonsComponent: ColumnLayout {
        enabled: root.state == "default"
        Repeater {
            id: wizardButtons
            model: wizardFilterModel
            property real preferredButtonWidth: 0
            Button {
                id: wizardButton
                display: AbstractButton.TextUnderIcon
                text: model.display
                icon.name: model.icon
                icon.color: PrecitecApplication.Settings.iconColor
                onClicked: {
                    if (model.component == WizardModel.SeamSeriesError)
                    {
                        root.errorsSelected();
                    }
                    else if (model.component == WizardModel.SeamSeriesAcquireScanField)
                    {
                        scanFieldController.startCalibration();
                    }
                    else if (model.component == WizardModel.SeamSeriesScanTracker)
                    {
                        root.scanTrackerSelected();
                    }
                    else if (model.component == WizardModel.SeamSeriesLaserControl)
                    {
                        root.laserControlSelected();
                    }
                    else if (model.component == WizardModel.SeamSeriesLaserWeldingMonitor)
                    {
                        root.lwmSelected();
                    }
                    else if (model.component == WizardModel.SeamSeriesScanLabScanner)
                    {
                        root.scanlabSelected();
                    }
                    else if (model.component == WizardModel.SeamSeriesIDM)
                    {
                        root.idmSelected();
                    }
                    else if (model.component == WizardModel.SeamSeriesZCollimator)
                    {
                        root.zCollimatorSelected();
                    }
                    else if (model.component == WizardModel.SeamSeriesScanTracker2D)
                    {
                        root.scanTracker2DSelected();
                    }
                }
                Component.onCompleted: {
                    wizardButtons.preferredButtonWidth = Math.max(wizardButtons.preferredButtonWidth, implicitWidth)
                }
                Layout.preferredWidth: wizardButtons.preferredButtonWidth
            }
        }
    }

    Connections {
        target: HardwareModule.systemStatus
        function onReturnedFromCalibration() {
            scanFieldController.endCalibration();
        }
    }

    signal seamSelected(var uuid)
    signal seamDeleted(var uuid)
    signal seamCreated()
    signal laserControlSelected()
    signal errorsSelected()
    signal lwmSelected()
    signal scanTrackerSelected()
    signal scanlabSelected()
    signal idmSelected()
    signal zCollimatorSelected()
    signal scanTracker2DSelected()

    product: root.seamSeries ? root.seamSeries.product : null
    title: qsTr("Details of seam series %1 (#%2)").arg(root.seamSeries ? root.seamSeries.name : "")
                                                  .arg(root.seamSeries ? root.seamSeries.visualNumber : -1)

    state: "default"
    states: [
        State {
            name: "default"
            PropertyChanges {
                target: properties
                enabled: true
            }
            PropertyChanges {
                target: seamList
                enabled: true
            }
            PropertyChanges {
                target: selectSeamBox
                visible: false
            }
            PropertyChanges {
                target: moveLabel
                visible: false
            }
            PropertyChanges {
                target: cancelButton
                visible: false
            }
            PropertyChanges {
                target: selectButton
                visible: true
            }
            PropertyChanges {
                target: configureButton
                visible: true
            }
            PropertyChanges {
                target: selectSeamBox
                currentIndex: -1
            }
        },
        State {
            name: "camera"
            PropertyChanges {
                target: properties
                enabled: false
            }
            PropertyChanges {
                target: seamList
                enabled: false
            }
            PropertyChanges {
                target: selectSeamBox
                visible: true
            }
            PropertyChanges {
                target: moveLabel
                visible: true
            }
            PropertyChanges {
                target: cancelButton
                visible: true
            }
            PropertyChanges {
                target: selectButton
                visible: false
            }
            PropertyChanges {
                target: configureButton
                visible: false
            }
            PropertyChanges {
                target: selectSeamBox
                currentIndex: selectSeamBox.find("Seam %1".arg(imageController.seam.visualNumber))
            }
        }
    ]

    onVisibleChanged: {
        root.state = "default"
    }

    ScanfieldSeamModel {
        id: imageController
        scanfieldModule {
            grabberDeviceProxy: HardwareModule.grabberDeviceProxy
            calibrationCoordinatesRequestProxy: HardwareModule.calibrationCoordinatesRequestProxy
        }
        hardwareParametersModule {
            attributeModel: root.attributeModel
        }
        seamSeries: root.seamSeries
        transformation: image.transformation
        showAllSeams: showAllSeamsButton.checked
        octWithReferenceArms: SystemConfiguration.OCT_with_reference_arms
    }

    ScanfieldSeamFilterModel {
        id: scanfieldValidFilter
        scanfieldSeamModel: imageController
    }

    ScanImageCalibrationController {
        id: scanFieldController
        seamSeries: root.seamSeries
        enabled: SystemConfiguration.Scanner2DEnable && SystemConfiguration.ScannerGeneralMode == SystemConfiguration.ScannerGenMode.ScanMaster
        inspectionCmdProxy: HardwareModule.inspectionCmdProxy
        calibrationDevice: HardwareModule.calibrationDeviceProxy

        onCalibratingChanged: {
            if (!calibrating)
            {
                productStructurePage.enabled = true;
                image.updateImage();
                imageController.scanfieldModule.loadCalibration();
            } else
            {
                productStructurePage.enabled = false;
            }
        }
    }

    Component {
        id: newSeamDialogComponent
        NewSeamDialog {
            onSeamCreated: {
                if (root.scanMasterMode)
                {
                    imageController.selectSeam(root.controller.currentSeam.uuid)
                    if (newSeam)
                    {
                        root.state = "camera"
                    }
                } else
                {
                    root.seamCreated()
                }
            }
        }
    }

    Component {
        id: deleteSeamSeriesDialogComponent
        DeleteSeamSeriesDialog {
            onAccepted: root.back()
        }
    }

    Component {
        id: popupComponent
        Popup {
            id: control
            property alias model: seamsList.model

            height: Math.min(contentItem.implicitHeight + topMargin + bottomMargin + topPadding + bottomPadding, Overlay.overlay.height - topMargin - bottomMargin - topPadding - bottomPadding)
            topMargin: 6
            bottomMargin: 6

            contentItem: ListView {
                id: seamsList
                clip: true
                implicitHeight: contentHeight
                implicitWidth: 100
                highlightMoveDuration: 0

                delegate: ItemDelegate {
                    text: qsTr("Seam %1").arg(modelData.visualNumber)
                    onClicked: {
                        imageController.selectSeam(modelData.uuid)
                        close();
                    }
                }

                ScrollBar.vertical: ScrollBar {}
            }
        }
    }

    Component {
        id: copyScanFieldImageDialogComponent
        CopyScanFieldImageDialog {
            width: Overlay.overlay.width * 0.5
            height: Overlay.overlay.height * 0.75

            onSeriesSelected: imageController.scanfieldModule.copyFromOtherSeries(uuid)
        }
    }

    contentItem: ColumnLayout {
        spacing: 10

        GridLayout {
            id: properties

            columns: 2

            Label {
                text: qsTr("Name:")
            }
            TextField {
                id: nameField
                selectByMouse: true
                text: root.seamSeries ? root.seamSeries.name : ""
                validator: FileSystemNameValidator {
                    allowWhiteSpace: true
                }
                palette.text: nameField.acceptableInput ? "black" : "red"
                Layout.fillWidth: true
                onEditingFinished: {
                    if (root.seamSeries && root.controller && root.seamSeries.name != text)
                    {
                        root.seamSeries.name = text;
                        root.controller.markAsChanged();
                    }
                }
            }
            Label {
                text: qsTr("Number:")
            }
            TextField {
                id: numberBox
                Layout.fillWidth: true
                text: root.seamSeries ? root.seamSeries.visualNumber : "0"
                validator: MeasureTaskNumberValidator {
                    measureTask: root.seamSeries
                }
                selectByMouse: true
                onEditingFinished: {
                    if (root.seamSeries)
                    {
                        root.seamSeries.number = root.seamSeries.numberFromVisualNumber(Number.fromLocaleString(locale, numberBox.text));
                        Notifications.NotificationSystem.warning(qsTr("Changing number of seam series results in all historic result and video data becoming inaccessable"));
                        root.controller.markAsChanged();

                    }
                }
                palette.text: numberBox.acceptableInput ? "black" : "red"
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            SeamListing {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.maximumWidth: root.scanMasterMode ? (seamList.expanded ? 0.25 * root.width : seamList.collapsedWidth) : root.width

                Behavior on Layout.maximumWidth {
                    PropertyAnimation {
                        id: animation
                    }
                }

                id: seamList

                leftPadding: 0
                model: root.seamSeries ? root.seamSeries.seams : undefined
                collapsable: root.scanMasterMode
                expanded: !root.scanMasterMode

                onSelected: {
                    if (root.scanMasterMode)
                    {
                        imageController.selectSeam(uuid)
                    } else
                    {
                        root.seamSelected(uuid)
                    }
                }
                onDeleted: root.seamDeleted(uuid)
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true

                id: scanfieldImageContainer

                visible: root.scanMasterMode
                border.color: PrecitecApplication.Settings.alternateBackground
                border.width: 1
                clip: true

                ColumnLayout {
                    anchors.centerIn: parent
                    visible: !scanFieldController.calibrating && (!image.imageValid || !imageController.scanfieldModule.configurationValid)
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("No Image available.")
                    }
                    Label {
                        Layout.alignment: Qt.AlignHCenter
                        text: qsTr("Please acquire Scan Field Image.")
                    }
                    RowLayout {
                        Button {
                            icon.name: root.wizardFilterModel ? root.wizardFilterModel.sourceModel.data(root.wizardFilterModel.sourceModel.indexForComponent(WizardModel.SeamSeriesAcquireScanField), Qt.DecorationRole) : ""
                            text: qsTr("Acquire")
                            onClicked: scanFieldController.startCalibration()
                        }
                        Button {
                            icon.name: "edit-copy"
                            text: qsTr("Copy from other Seam Series")
                            onClicked: {
                                var dialog = copyScanFieldImageDialogComponent.createObject(root, {"product": root.seamSeries.product});
                                dialog.open();
                            }
                        }
                    }
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: scanFieldController.calibrating
                }

                MouseArea {
                    id: globalMouseArea
                    anchors.fill: parent
                    enabled: image.visible
                    onWheel: {
                        image.zoom += (wheel.angleDelta.y / 120.0) * 0.01;
                    }
                }

                DragHandler {
                    id: globalDragHandler
                    property point startPanning
                    property bool cameraMove: false
                    property bool delayPanel: false
                    enabled: image.visible
                    target: null
                    minimumPointCount: 1
                    maximumPointCount: 1
                    onActiveChanged: {
                        if (active)
                        {
                            if (ApplicationWindow.activeFocusControl)
                            {
                                ApplicationWindow.activeFocusControl.focus = false;
                            }

                            if (root.state == "camera")
                            {
                                cameraMove = false;
                                startPanning = image.panning;
                                return;
                            }

                            delayPanel = !imageController.seam;
                            cameraMove = true;

                            if (imageController.seam && imageController.pointInCurrentSeam(centroid.position))
                            {
                                startPanning = imageController.currentPaintedCenter;
                            } else
                            {
                                var seams = imageController.selectSeams(centroid.position);
                                if (seams.length === 1)
                                {
                                    imageController.selectSeam(seams[0].uuid);
                                    startPanning = imageController.currentPaintedCenter;
                                } else
                                {
                                    cameraMove = false;
                                    startPanning = image.panning;
                                }
                            }
                        } else
                        {
                            cameraMove = false;
                            delayPanel = false;
                        }
                    }
                    onTranslationChanged: {
                        if (cameraMove)
                        {
                            imageController.setCameraCenter(Qt.point(startPanning.x + translation.x, startPanning.y + translation.y));
                            root.controller.markAsChanged();
                        } else
                        {
                            image.panning = Qt.point(startPanning.x + translation.x, startPanning.y + translation.y);
                        }
                    }
                }

                PinchHandler {
                    id: globalPinchHandler
                    property real startZoom: 1.0
                    enabled: image.visible
                    target: null
                    maximumRotation: 0
                    onActiveChanged: {
                        if (active)
                        {
                            startZoom = image.zoom;
                            if (ApplicationWindow.activeFocusControl)
                            {
                                ApplicationWindow.activeFocusControl.focus = false;
                            }
                        }
                    }
                    onActiveScaleChanged: {
                        image.zoom = startZoom * activeScale;
                    }
                }

                PrecitecImage.SourceImageItem {
                    anchors.centerIn: parent

                    id: image
                    thumbnail: imageController.scanfieldModule.cameraSize
                    thumbnailOriginalSize: imageController.scanfieldModule.imageSize
                    visible: !scanFieldController.calibrating && (image.imageValid || image.loading) && imageController.scanfieldModule.configurationValid
                    pauseUpdate: animation.running || animationPanel.running || productStackView.busy
                    width: parent.width - 10
                    height: parent.height - 10

                    Binding {
                        target: image
                        property: "source"
                        delayed: true
                        restoreMode: Binding.RestoreValue
                        value: imageController.scanfieldModule.sourceImageDir
                        when: !scanFieldController.calibrating && imageController.scanfieldModule.configurationValid && !imageController.scanfieldModule.loading
                    }

                    onImageChanged: image.zoomToFit();
                    Component.onCompleted: image.zoomToFit();

                    Repeater {
                        model: imageController

                        Rectangle {
                            x: model.paintedCameraRect.x
                            y: model.paintedCameraRect.y
                            width: model.paintedCameraRect.width
                            height: model.paintedCameraRect.height
                            visible: model.cameraCenterValid && model.showSeam

                            color: model.seam == imageController.seam ? Qt.rgba(1, 1, 0, 0.2) : "transparent"

                            border {
                                width: 2
                                color: "yellow"
                            }

                            Label {
                                anchors.centerIn: parent
                                text: model.text
                                color: "yellow"
                            }
                        }
                    }

                    TapHandler {
                        target: image
                        enabled: image.visible
                        onSingleTapped: {
                            if (root.state == "camera")
                            {
                                imageController.setCameraCenter(eventPoint.position);
                                root.controller.markAsChanged();
                                root.state = "default";
                            } else
                            {
                                var seams = imageController.selectSeams(eventPoint.position);
                                switch (seams.length)
                                {
                                    case 0:
                                        imageController.selectSeam("")
                                        break;
                                    case 1:
                                        imageController.selectSeam(seams[0].uuid)
                                        break;
                                    default:
                                    {
                                        var popup = popupComponent.createObject(root, {"x": eventPoint.position.x, "y": eventPoint.position.y, "model": seams});
                                        popup.open();
                                    }
                                }
                            }
                        }
                    }

                    BusyIndicator {
                        anchors.centerIn: parent
                        running: image.loading
                    }
                }

                RowLayout {
                    visible: image.visible
                    anchors {
                        margins: 10
                        top: parent.top
                        right: parent.right
                    }
                    ToolButton {
                        id: showAllSeamsButton
                        icon.name: checked ? "distribute-vertical-margin" : "box"
                        display: AbstractButton.IconOnly
                        checkable: true
                        checked: imageController.showAllSeams
                    }
                    ToolButton {
                        icon.name: "zoom-original"
                        display: AbstractButton.IconOnly
                        onClicked: {
                            image.zoom = 1.0;
                            image.panning = Qt.point(0,0);
                        }
                    }
                    ToolButton {
                        icon.name: "zoom-fit-best"
                        display: AbstractButton.IconOnly
                        onClicked: image.zoomToFit()
                    }
                }
            }

            Rectangle {
                property int maxWidth: 0.15 * root.width

                Layout.fillHeight: true
                Layout.preferredWidth: imageController.seam && !globalDragHandler.delayPanel ? Math.max(panel.maxWidth, layout.implicitWidth + 10) : 0

                id: panel

                border {
                    width: 1
                    color: PrecitecApplication.Settings.alternateBackground
                }
                ScrollView {
                    id: commentTextView
                    anchors.fill: parent
                    clip: true
                    ColumnLayout {
                        id: layout

                        anchors {
                            fill: parent
                            margins: 5
                        }

                        opacity: imageController.seam && !globalDragHandler.delayPanel && !animationPanel.running ? 1 : 0

                        Label {
                            Layout.fillWidth: true
                            Layout.maximumWidth: panel.width - 10
                            font.bold: true
                            text: imageController.seam ? "%1: %2".arg(imageController.seam.visualNumber).arg(imageController.seam.name) : ""
                            wrapMode: Text.Wrap
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Item {
                            Layout.fillHeight: true
                        }

                        Label {
                            Layout.fillWidth: true
                            Layout.maximumWidth: panel.width - 10
                            visible: !imageController.currentCenterValid
                            font.bold: true
                            text: qsTr("Invalid Camera Position!\n%1").arg(imageController.lastValidSeam ? ("Using last valid seam position (Seam %1)").arg(imageController.lastValidSeam.visualNumber) : "Please place Seam onto Image.")
                            wrapMode: Text.Wrap
                            horizontalAlignment: Text.AlignHCenter
                        }

                        PrecitecApplication.ArrowButtonGrid {
                            Layout.alignment: Qt.AlignHCenter

                            visible: imageController.currentCenterValid

                            leftEnabled: imageController.leftEnabled
                            rightEnabled: imageController.rightEnabled
                            upEnabled: imageController.topEnabled
                            downEnabled: imageController.bottomEnabled

                            onMoveLeft: {
                                var center = imageController.currentPaintedCenter;
                                imageController.setCameraCenter(Qt.point(center.x - 1, center.y));
                                root.controller.markAsChanged();
                            }
                            onMoveDown: {
                                var center = imageController.currentPaintedCenter;
                                imageController.setCameraCenter(Qt.point(center.x, center.y + 1));
                                root.controller.markAsChanged();
                            }
                            onMoveUp: {
                                var center = imageController.currentPaintedCenter;
                                imageController.setCameraCenter(Qt.point(center.x, center.y - 1));
                                root.controller.markAsChanged();
                            }
                            onMoveRight: {
                                var center = imageController.currentPaintedCenter;
                                imageController.setCameraCenter(Qt.point(center.x + 1, center.y));
                                root.controller.markAsChanged();
                            }
                        }
                        ColumnLayout {
                                Layout.alignment: Qt.AlignHCenter
                                clip: true
                                visible: imageController.currentCenterValid
                                Label {
                                    Layout.alignment: Qt.AlignHCenter
                                    visible: imageController.currentCenterValid
                                    text: qsTr("Camera Center:")
                                    font.bold: true
                                }
                            GridLayout {
                                Layout.alignment: Qt.AlignHCenter
                                columns: 3
                                visible: imageController.currentCenterValid
                                Label {
                                    visible: imageController.currentCenterValid
                                    text: qsTr("X:")
                                }

                                TextField {
                                    id: xPosition
                                    visible: imageController.currentCenterValid
                                    text: imageController.selectedNode ? imageController.selectedNode.item.x.toLocaleString(locale, 'f', 2) : imageController.currentCenterInMM.x.toLocaleString(locale, 'f', 2)
                                    validator: DoubleValidator {
                                        bottom: -10000
                                        top: 10000
                                        decimals: 2
                                    }
                                    onEditingFinished: {
                                        var centerInMM = imageController.currentCenterInMM;
                                        var centerInImageCoordinates = imageController.scannerToImageCoordinates(Qt.point(Number.fromLocaleString(xPosition.text), centerInMM.y));
                                        var paintedPoint = imageController.imageCoordinatesToPaintedPoint(centerInImageCoordinates);
                                        imageController.setCameraCenter(paintedPoint);
                                        root.controller.markAsChanged();
                                    }
                                }
                                Label {
                                    visible: imageController.currentCenterValid
                                    text: qsTr("mm")
                                }
                                Label {
                                    visible: imageController.currentCenterValid
                                    text: qsTr("Y:")
                                }
                                TextField {
                                    id: yPosition
                                    visible: imageController.currentCenterValid
                                    text: imageController.selectedNode ? imageController.selectedNode.item.y.toLocaleString(locale, 'f', 2) : imageController.currentCenterInMM.y.toLocaleString(locale, 'f', 2)
                                    validator: DoubleValidator {
                                        bottom: -10000
                                        top: 10000
                                        decimals: 2
                                    }
                                    onEditingFinished: {
                                        var centerInMM = imageController.currentCenterInMM;
                                        var centerInImageCoordinates = imageController.scannerToImageCoordinates(Qt.point(centerInMM.x, Number.fromLocaleString(yPosition.text)));
                                        var paintedPoint = imageController.imageCoordinatesToPaintedPoint(centerInImageCoordinates);
                                        imageController.setCameraCenter(paintedPoint);
                                        root.controller.markAsChanged();
                                    }
                                }
                                Label {
                                    visible: imageController.currentCenterValid
                                    text: qsTr("mm")
                                }
                            }
                        }


                        CheckBox {
                            id: driveWithOCTReference
                            visible: imageController.currentCenterValid && SystemConfiguration.OCT_with_reference_arms
                            text: qsTr("Adjust OCT Reference")
                            checked: imageController.currentDriveWithOCTReference
                            onToggled: {
                                imageController.setDriveWithOCT(driveWithOCTReference.checked);
                                root.controller.markAsChanged();
                            }
                        }

                        Item {
                            Layout.preferredHeight: 50
                            visible: !imageController.currentCenterValid && imageController.lastCenterValid
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            visible: imageController.currentCenterValid || imageController.lastCenterValid
                            text: "X: %1 px (%2 mm)".arg(imageController.currentCenterValid ? imageController.currentCenter.x : imageController.lastCenter.x).arg(imageController.currentCenterValid ? imageController.currentCenterInMM.x.toLocaleString(locale, 'f', 2) : imageController.lastCenterInMM.x.toLocaleString(locale, 'f', 2))
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            visible: imageController.currentCenterValid || imageController.lastCenterValid
                            text: "Y: %1 px (%2 mm)".arg(imageController.currentCenterValid ? imageController.currentCenter.y : imageController.lastCenter.y).arg(imageController.currentCenterValid ? imageController.currentCenterInMM.y.toLocaleString(locale, 'f', 2) : imageController.lastCenterInMM.y.toLocaleString(locale, 'f', 2))
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            visible: imageController.currentCenterValid
                            text: qsTr("Camera Boundary:")
                            font.bold: true
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            visible: imageController.currentCenterValid
                            text: "X: %1 px".arg(imageController.currentRect.x)
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            visible: imageController.currentCenterValid
                            text: "Y: %1 px".arg(imageController.currentRect.y)
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            visible: imageController.currentCenterValid
                            text: "Width: %1 px".arg(imageController.currentRect.width)
                        }

                        Label {
                            Layout.alignment: Qt.AlignHCenter
                            visible: imageController.currentCenterValid
                            text: "Height: %1 px".arg(imageController.currentRect.height)
                        }

                        Item {
                            Layout.fillHeight: true
                        }

                        Button {
                            Layout.fillWidth: true
                            id: selectButton
                            icon.name: "select"
                            text: qsTr("Position Seam")
                            onClicked: root.state = "camera"
                        }
                        Button {
                            Layout.fillWidth: true
                            id: resetButton
                            icon.name: "edit-undo"
                            text: qsTr("Reset Position")
                            visible: imageController.currentCenterValid && root.state == "default"
                            onClicked: {
                                imageController.resetCameraCenter();
                                root.controller.markAsChanged();
                            }
                        }
                        Label {
                            id: moveLabel
                            text: qsTr("Move to Center of Seam:")
                            font.bold: true
                        }
                        ComboBox {
                            Layout.fillWidth: true
                            id: selectSeamBox
                            textRole: "text"
                            model: scanfieldValidFilter
                            onActivated: {
                                imageController.setCameraCenterToSeam(scanfieldValidFilter.mapToSource(scanfieldValidFilter.index(index, 0)).row);
                                root.controller.markAsChanged();
                                root.state = "default";
                            }
                        }
                        Button {
                            Layout.fillWidth: true
                            id: configureButton
                            icon.name: "application-menu"
                            text: qsTr("Configure Seam")
                            onClicked: root.seamSelected(imageController.seam.uuid)
                        }
                        Button {
                            Layout.fillWidth: true
                            id: cancelButton
                            text: qsTr("Cancel")
                            onClicked: root.state = "default"
                        }
                    }

                    Behavior on Layout.preferredWidth {
                        PropertyAnimation {
                            id: animationPanel
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Button {
                text: qsTr("Add new seam")
                display: AbstractButton.TextBesideIcon
                icon.name: "list-add"
                enabled: !root.scanMasterMode || !scanFieldController.calibrating && image.imageValid && imageController.scanfieldModule.configurationValid
                onClicked: {
                    var dialog = newSeamDialogComponent.createObject(root, {"product": root.seamSeries.product, "controller": root.controller});
                    dialog.open();
                }
            }
            Button {
                display: AbstractButton.TextBesideIcon
                text: qsTr("Delete this seam series")
                icon.name: "edit-delete"
                onClicked: {
                    var dialog = deleteSeamSeriesDialogComponent.createObject(root, {"seamSeries": root.seamSeries});
                    dialog.open();
                }
            }
        }
    }
}
