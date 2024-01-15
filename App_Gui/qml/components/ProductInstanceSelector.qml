import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

import precitec.gui.components.application 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.notifications 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0

/**
 * GroupBox providing selection of a product instance through a ProductInstanceModel.
 **/
GroupBox {
    id: root
    /**
     * The Product for which a product instance should be selected
     **/
    property alias product: productInstanceModel.product
    /**
     * The directory containing the product instances
     **/
    property alias directory: productInstanceModel.directory
    /**
     * The QFileInfo of the selected product instance
     **/
    property var selectedProductInstance: undefined

    /**
     * The serial number of the selected product instance
     **/
    property var selectedSerialNumber: undefined

    /**
     * The (optional) part number of the selected product instance
     **/
    property var selectedPartNumber: undefined

    /**
     * The directory name of the selected product instance
     **/
    property string selectedDirectoryName: ""

    /**
     * The date of the selected product instance
     **/
    property date selectedDate

    property alias productDirectoryName: productInstanceModel.productDirectoryName

    /**
     * Whether to show the nioIndicator, default is @c true
     **/
    property bool nioIndicator: true

    property alias monitoring: productInstanceModel.monitoring

    /**
     * Whether this ProductInstanceSelector provides download functionality
     * Default @c false
     **/
    property bool supportsDownload: false

    /**
     * Whether this ProductInstanceSelector provides upload functionality
     * Default @c false
     **/
    property bool supportsUpload: false

    /**
     * Whether this ProductInstanceSelector provides delete functionality
     * Default @c false
     **/
    property bool supportsDelete: false

    /**
     * The name for the download directory
     **/
    property string downloadDirectoryName: ""

    /**
     * PDF file name for online help
     **/
    property var pdfFile: OnlineHelp.HasNoPdf

    property var importService: null

    property var resultsExporter: null

    property alias filterOnNio: nioFilterBox.visible

    /**
     * The Permission whether download of product instances is supported
     **/
    property int downloadPermission: -1

    /**
     * The Permission whether delete of product instances is supported
     **/
    property int deletePermission: -1

    property alias videoRecorderProxy: deleteProductInstanceController.videoRecorderProxy

    property var screenshotTool: null
    property int selectedIndex: -1

    property bool hasNext: selectedIndex != -1 && selectedIndex < productInstanceListView.count -1
    property bool hasPrevious: selectedIndex > 0

    signal transferInProgressChanged(bool isFinished)

    onSelectedProductInstanceChanged: {
        root.selectedIndex = -1;
    }

    function selectNext()
    {
        if (!root.hasNext)
        {
            return;
        }
        selectByIndex(productInstanceModel.filterModel.index(root.selectedIndex + 1, 0));
    }

    function selectPrevious()
    {
        if (!root.hasPrevious)
        {
            return;
        }
        selectByIndex(productInstanceModel.filterModel.index(root.selectedIndex - 1, 0));
    }

    function selectByIndex(index)
    {
        root.selectedProductInstance = productInstanceModel.filterModel.data(index, Qt.UserRole);
        root.selectedSerialNumber = productInstanceModel.filterModel.data(index, Qt.Display);
        root.selectedPartNumber = productInstanceModel.filterModel.data(index, Qt.UserRole + 11);
        root.selectedDate = productInstanceModel.filterModel.data(index, Qt.UserRole + 1);
        root.selectedDirectoryName = productInstanceModel.filterModel.data(index, Qt.UserRole + 8);
        root.selectedIndex = index.row;
    }

    ProductInstanceModel {
        id: productInstanceModel
        stationName: GuiConfiguration.stationName
        extendedProductInfoHelper {
            serialNumberFromExtendedProductInfo: GuiConfiguration.serialNumberFromExtendedProductInfo != 0
            partNumberFromExtendedProductInfo: GuiConfiguration.partNumberFromExtendedProductInfo != 0
        }
    }
    Binding {
        target: productInstanceModel.extendedProductInfoHelper
        property: "serialNumberFromExtendedProductInfoField"
        value: GuiConfiguration.serialNumberFromExtendedProductInfo - 1
        when: GuiConfiguration.serialNumberFromExtendedProductInfo != 0
    }

    Binding {
        target: productInstanceModel.extendedProductInfoHelper
        property: "partNumberFromExtendedProductInfoField"
        value: GuiConfiguration.partNumberFromExtendedProductInfo - 1
        when: GuiConfiguration.partNumberFromExtendedProductInfo != 0
    }

    ProductInstancesTransferController {
        id: productInstancesTransferController
        indexFilterModel: checkedFilterModel
        onTransferInProgressChanged:
        {
            if (productInstancesTransferController.transferInProgress)
            {
                root.enabled = false;
                root.transferInProgressChanged(false);
            } else
            {
                root.enabled = true;
                root.transferInProgressChanged(true);
            }
        }
    }

    DeleteProductInstanceController {
        id: deleteProductInstanceController
        model: productInstanceModel
    }

    Component {
        id: importDialogComponent
        ImportVideoSequenceDialog {
            importService: root.importService
            directoryName: root.downloadDirectoryName
            screenshotTool: root.screenshotTool
            liveMode: root.product.defaultProduct
            productDirectory: productInstanceModel.productDirectory
            onClosed: destroy()
        }
    }
    Component {
        id: newProductInstanceDialogComponent
        Dialog {
            id: newProductDialog
            title: qsTr("Add to another product")
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: parent.width / 2
            modal: true
            standardButtons: Dialog.Ok | Dialog.Cancel
            onAccepted: {
                if (productCombo.currentIndex != -1)
                {
                    productInstancesTransferController.transfer();
                }
                destroy();
            }
            onRejected: {
                destroy();
            }
            RowLayout {
                width: parent.width
                Label {
                    text: qsTr("Product:")
                }
                ComboBox {
                    id: productCombo
                    enabled: true
                    model: SimulationModule.productModel
                    Layout.fillWidth: true
                    currentIndex: -1
                    textRole: "display"
                    onCurrentIndexChanged: {
                        productInstancesTransferController.targetProduct = model.data(model.index(productCombo.currentIndex, 0), Qt.UserRole + 1);
                    }
                }
            }
        }
    }

    Component {
        id: importServiceComponent
        RemovableDevices.DownloadService {
            property string uuid: root.product ? root.product.uuid : ""
            productName: uuid != "" ? uuid.slice(1, uuid.length -1 ) : ""
            backupPath: root.directory
            validate: false
        }
    }

    label: RowLayout {
        width: root.availableWidth
        Label {
            text: qsTr("Select Instance of Product \"%1\"").arg(root.product ? root.product.name : "")
            font.family: root.font.family
            font.pixelSize: root.font.pixelSize
            font.bold: true
            verticalAlignment: Text.AlignVCenter

            Layout.leftMargin: root.leftPadding
        }
        ToolButton {
            Layout.topMargin: 5
            Layout.alignment: Qt.AlignRight
            id: configureButton
            display: AbstractButton.IconOnly
            icon.name: "application-menu"
            checkable: true
        }
    }

    CheckedFilterModel {
        id: checkedFilterModel
        sourceModel: productInstanceModel
        roleName: "checked"
    }

    ColumnLayout {
        anchors.fill: parent
        ColumnLayout {
            id: configureControls
            visible: configureButton.checked
            Layout.fillWidth: true
            GridLayout {
                columns: 2
                ComboBox {
                    id: nioFilterBox
                    Layout.fillWidth: true
                    model: ["All results", "Faults Only", "Remove Faults"]
                    onCurrentIndexChanged: {
                        switch (currentIndex)
                        {
                            case 1:
                                productInstanceModel.filterModel.filterType = ProductInstanceSortModel.OnlyNIO;
                                break;
                            case 2:
                                productInstanceModel.filterModel.filterType = ProductInstanceSortModel.RemoveNIO;
                                break;
                            default:
                                productInstanceModel.filterModel.filterType = ProductInstanceSortModel.All;
                                break;
                        }
                    }
                }
                ToolButton {
                    id: sortOrderButton
                    display: AbstractButton.IconOnly
                    icon.name: productInstanceModel.filterModel.sortOrder == Qt.DescendingOrder ? "view-sort-descending" : "view-sort-ascending"
                    onClicked: {
                        if (productInstanceModel.filterModel.sortOrder == Qt.DescendingOrder)
                        {
                            productInstanceModel.filterModel.sortOrder = Qt.AscendingOrder;
                        } else
                        {
                            productInstanceModel.filterModel.sortOrder = Qt.DescendingOrder;
                        }
                    }
                }
                TextField {
                    Layout.fillWidth: true
                    Layout.columnSpan: filterOnNio ? 2 : 1
                    placeholderText: qsTr("Search for serial number")
                    selectByMouse: true
                    onTextChanged: {
                        productInstanceModel.filterModel.filter = text;
                    }
                }
                CheckBox {
                    id: dateFilterCheckbox
                    checked: productInstanceModel.filterModel.filterOnDate
                    text: qsTr("Filter on date")
                    Layout.columnSpan: 2
                    onClicked: {
                        productInstanceModel.filterModel.filterOnDate = !productInstanceModel.filterModel.filterOnDate;
                    }
                }
                RowLayout {
                    visible: dateFilterCheckbox.checked
                    Label {
                        text: qsTr("From:")
                    }
                    DateSelector {
                        onDateChanged: {
                            productInstanceModel.filterModel.from = date;
                        }
                    }
                }
                RowLayout {
                    visible: dateFilterCheckbox.checked
                    Label {
                        text: qsTr("To:")
                    }
                    DateSelector {
                        onDateChanged: {
                            productInstanceModel.filterModel.to = date;
                        }
                    }
                }
            }
        }

        Label {
            id: missingDeviceLabel
            visible: root.supportsDownload && supportsUpload && RemovableDevices.Service.udi == ""
            text: qsTr("Please connect a removable device in order to download or import product instances")
        }
        Label {
            id: missingMountLabel
            visible: root.supportsDownload && supportsUpload && RemovableDevices.Service.udi != "" && RemovableDevices.Service.path == ""
            text: qsTr("Please mount the removable device in order to download or import product instances")
        }
        Label {
            visible: root.resultsExporter != null && RemovableDevices.Service.udi == "" && !missingDeviceLabel.visible
            text: qsTr("Please connect a removable device in order to export product instances")
        }
        Label {
            visible: root.resultsExporter != null && RemovableDevices.Service.udi != "" && RemovableDevices.Service.path == "" && !missingMountLabel.visible
            text: qsTr("Please mount the removable device in order to export product instances")
        }

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ListView {
                id: productInstanceListView
                objectName: root.objectName + "-listview"
                anchors.fill: parent
                model: productInstanceModel.filterModel
                clip: true
                spacing: 5

                delegate: ItemDelegate {

                    property string diskUsage: ""

                    onClicked: {
                            root.selectedProductInstance = model.fileInfo
                            root.selectedSerialNumber = model.display
                            root.selectedPartNumber = model.partNumber
                            root.selectedDate = model.date
                            root.selectedDirectoryName = model.directoryName
                            root.selectedIndex = index;
                    }

                    id: delegateItem

                    width: ListView.view.width
                    height: layout.implicitHeight
                    objectName: ListView.view.objectName + "-item-" + index

                    GridLayout {
                        id: layout

                        anchors {
                            fill: parent
                            margins: 0
                        }

                        columns: 3

                        CheckBox {
                            Layout.alignment: Qt.AlignVCenter
                            checked: model.checked
                            visible: downloadButton.visible || exportButton.visible || deleteButton.visible
                            onClicked: productInstanceModel.toggleChecked(index)
                        }

                        Item {
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredWidth: 25
                            Layout.preferredHeight: 25

                            Rectangle {
                                anchors.fill: parent
                                visible: nioIndicator
                                radius: width * 0.5
                                color: model.nio == ProductInstanceModel.Nio ? "red" : (model.nio == ProductInstanceModel.Io ? "green" : "transparent")
                                border {
                                    color: "yellow"
                                    width: model.nioResultsSwitchedOff ? 5 : 0
                                }
                                BusyIndicator {
                                    anchors.fill: parent
                                    running: model.nio == ProductInstanceModel.Unknown
                                }
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 0
                            Label {
                                text: model.display
                                font.bold: true
                                Layout.alignment: Qt.AlignLeft
                                Layout.fillWidth: true
                            }
                            Label {
                                visible: model.partNumber != ""
                                //: a label containing a part number (argument is the part number)
                                text: qsTr("Part Number: %1").arg(model.partNumber)

                                Layout.alignment: Qt.AlignLeft
                                Layout.fillWidth: true
                            }
                            Label {
                                text: Qt.formatDateTime(model.date, "yyyy-MM-dd hh:mm:ss.zzz") + (delegateItem.diskUsage != "" ? " (%1)".arg(delegateItem.diskUsage) : "")
                                Layout.alignment: Qt.AlignLeft
                                Layout.fillWidth: true
                            }
                        }


                        RowLayout{
                            Layout.fillWidth: true
                            Layout.columnSpan: 3
                            visible: model.downloadService && model.downloadService.backupInProgress
                            Label{
                                text: model.downloadService ? model.downloadService.backupStatus : ""
                            }
                            ProgressBar {
                                value: model.downloadService ? model.downloadService.progress : 0.0
                            }
                        }

                        Component.onCompleted: {
                            productInstanceModel.ensureMetaDataLoaded(index);
                            if (downloadButton.visible)
                            {
                                model.downloadService.diskUsageAsync(model.path);
                            }
                        }
                        Connections {
                            target: productInstanceModel.filterModel
                            function onSortOrderChanged() {
                                productInstanceModel.ensureMetaDataLoaded(index)
                            }
                        }
                        Connections {
                            target: model.downloadService
                            function onDiskUsageCalculated(path, usage) {
                                if (model.path == path)
                                {
                                    delegateItem.diskUsage = usage;
                                }
                            }

                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignLeft
            visible: downloadButton.visible || exportButton.visible || deleteButton.visible
            Button {
                text: qsTr("Check all")
                onClicked: productInstanceModel.checkAll()
            }
            Button {
                text: qsTr("Uncheck all")
                onClicked: productInstanceModel.uncheckAll()
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            Button {
                text: qsTr("Add to another product")
                icon.name: "edit-copy"
                enabled: checkedFilterModel.rowCount != 0 && !productInstancesTransferController.transferInProgress
                onClicked: {
                    var dialog = newProductInstanceDialogComponent.createObject(root);
                    dialog.open();
                }

                BusyIndicator {
                    anchors.fill: parent
                    running: productInstancesTransferController.transferInProgress
                    visible: productInstancesTransferController.transferInProgress
                }
            }
            Button {
                text: qsTr("Import product instance")
                icon.name: "cloud-upload"
                visible: root.supportsUpload
                enabled: RemovableDevices.Service.path != ""

                onClicked: {
                    if (root.importService == null)
                    {
                        root.importService = importServiceComponent.createObject(root);
                    }
                    importDialogComponent.createObject(root).open();
                }
            }
            Button {
                id: downloadButton
                objectName: "download-button"
                property bool downloading: productInstanceModel.downloading
                visible: root.supportsDownload && UserManagement.currentUser && UserManagement.hasPermission(App.MountPortableDevices) && UserManagement.hasPermission(root.downloadPermission)
                text: downloading ? qsTr("Cancel Download") : (checkedFilterModel.rowCount > 0 ? qsTr("Download %1 items").arg(checkedFilterModel.rowCount) : qsTr("Download"))
                enabled: RemovableDevices.Service.path != "" && checkedFilterModel.rowCount != 0
                icon.name: "edit-download"
                onClicked: {
                    if (!UserManagement.hasPermission(App.MountPortableDevices) || !UserManagement.hasPermission(root.downloadPermission))
                    {
                        return
                    }
                    var path = root.downloadDirectoryName;
                    if (!downloading)
                    {
                        NotificationSystem.information(qsTr("Downloading %1 items to %2 on attached removable device")
                                                            .arg(checkedFilterModel.rowCount)
                                                            .arg(path));
                        productInstanceModel.downloadChecked(RemovableDevices.Service.path, path);
                    }
                    else
                    {
                        productInstanceModel.cancelDownload();
                    }
                }
            }
            Button {
                id: exportButton
                text: qsTr("Export to Spreadsheet")
                icon.name: "document-export"
                visible: root.resultsExporter != null && UserManagement.currentUser && UserManagement.hasPermission(App.MountPortableDevices) && UserManagement.hasPermission(App.ExportResults)
                enabled: root.resultsExporter != null && !root.resultsExporter.exporting && root.resultsExporter.exportDirectory != "" && checkedFilterModel.rowCount != 0
                onClicked: {
                    for (var i = 0; i < checkedFilterModel.rowCount; i++)
                    {
                        var index = checkedFilterModel.mapToSource(checkedFilterModel.index(i, 0));
                        root.resultsExporter.scheduleExport(productInstanceModel.data(index, Qt.UserRole), productInstanceModel.data(index, Qt.UserRole + 1), root.product, productInstanceModel.data(index, Qt.DisplayRole));
                    }
                    root.resultsExporter.exportScheduled();
                }

                BusyIndicator {
                    id: busyIndicator
                    anchors.fill: parent
                    running: root.resultsExporter != null && root.resultsExporter.exporting
                }
            }
            Button {
                id: deleteButton
                text: qsTr("Delete")
                icon.name: "edit-delete"
                visible: root.supportsDelete && UserManagement.currentUser && UserManagement.hasPermission(root.deletePermission)
                enabled: checkedFilterModel.rowCount != 0
                onClicked: {
                    var dialog = deleteInstancesDialogCompnent.createObject(root, {"model": checkedFilterModel});
                    dialog.open();
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: productInstanceModel.loading
    }

    onProductChanged: {
        root.selectedProductInstance = undefined;
        root.selectedDirectoryName = "";
    }

    Component.onCompleted: {
        productInstanceModel.filterModel.sortOrder = 1;
    }

    Component {
        id: deleteInstancesDialogCompnent
        Dialog {
            id: deleteInstancesDialog
            property alias model: toDeleteListView.model
            parent: Overlay.overlay
            anchors.centerIn: parent
            height: parent.height * 0.66

            modal: true
            title: qsTr("Delete product instances?")
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape

            header: DialogHeaderWithScreenshot {
                title: deleteInstancesDialog.title
                screenshotTool: root.screenshotTool
            }

            onAccepted: {
                deleteProductInstanceController.deleteAllChecked();
                destroy();
            }
            onRejected: {
                destroy();
            }
            ColumnLayout {
                anchors.fill: parent
                Label {
                    text: qsTr("Do you really want to delete the following product instances:")
                }
                ScrollView {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ListView {
                        id: toDeleteListView
                        clip: true
                        spacing: 5
                        anchors.fill: parent
                        delegate: Item {
                            width: ListView.view.width
                            height: childrenRect.height
                            Label {
                                id: nameLabel
                                text: model.display
                                font.bold: true
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                            }
                            Label {
                                text: Qt.formatDateTime(model.date, "yyyy-MM-dd hh:mm:ss.zzz")
                                Layout.alignment: Qt.AlignLeft
                                Layout.fillWidth: true
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: nameLabel.bottom
                            }
                        }
                    }
                }
            }
        }
    }
}
