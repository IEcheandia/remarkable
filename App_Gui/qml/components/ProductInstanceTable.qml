import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0
import precitec.gui.components.application 1.0
import precitec.gui.components.notifications 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui 1.0

/**
 * GroupBox providing selection of a product instance or any of its seams through a ProductInstanceTableModel.
 **/
GroupBox {
    /**
     * The Product for which a product instance should be selected
     **/
    property alias product: productInstanceModel.product

    /**
     * How the Product directory is named, either Uuid or ProductName
     **/
    property alias productDirectoryName: productInstanceModel.productDirectoryName

    /**
     * Whether the ProductInstanceModel should monitor the directory for new product instances.
     **/
    property alias monitoring: productInstanceModel.monitoring

    /**
     * Weather the nio indicators are shown
     **/
    property bool nioIndicatorVisible: true

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
    property string selectedPartNumber: ""

    /**
     * The seam series of the selected seam
     **/
    property var selectedSeamSeries: null

    /**
     * The selected seam
     **/
    property var selectedSeam: null

    /**
     * The position of the seam within the series
     **/
    property int seamInSeriesIndex: -1

    /**
     * The directory name of the selected product instance
     **/
    property string selectedDirectoryName: ""

    /**
     * The date of the selected product instance
     **/
    property date selectedDate

    /**
     * Weather the user should be able to filter on nio/io
     **/
    property alias filterOnNio: nioFilterBox.visible

    /**
     * Component for exporting results to a spreadsheet
     **/
    property var resultsExporter: null

    /**
     * Pdf help file
     **/
    property var pdfFile: OnlineHelp.HasNoPdf

    /**
     * Component for saving screenshots
     **/
    property var screenshotTool: null

    id: root

    onProductChanged: {
        root.selectedProductInstance = undefined;
        root.selectedDirectoryName = "";
        root.selectedSerialNumber = undefined;
        root.selectedPartNumber = "";
        root.selectedSeamSeries = null;
        root.seamInSeriesIndex = -1;
        root.selectedSeam = null;
    }

    Component {
        id: assemblyImageDialogComponent
        Dialog {
            id: assemblyImageDialog
            property string serialNumber
            property int productInstanceRow: -1

            parent: Overlay.overlay
            width: parent.width * 0.8
            height: parent.height * 0.8
            anchors.centerIn: parent

            standardButtons: Dialog.Close
            modal: true

            header: DialogHeaderWithScreenshot {
                screenshotTool: root.screenshotTool
                title: qsTr("Assembly image for product instance %1").arg(serialNumber)
            }

            AssemblyImageFromProductInstanceTableModel {
                id: assemblyImageFromProductInstanceTableModel
                productInstanceTableModel: productInstanceModel
            }

            AssemblyImageItem {
                id: assemblyImageItem
                assemblyImage: root.product ? root.product.assemblyImage : ""
                assemblyImagesDirectory: WeldmasterPaths.assemblyImagesDir
                model: assemblyImageFromProductInstanceTableModel

                anchors.fill: parent
                onSeamClicked: {
                    var modelIndex = productInstanceModel.index(assemblyImageDialog.productInstanceRow, 0);
                    root.selectedProductInstance = productInstanceModel.data(modelIndex, Qt.UserRole);
                    root.selectedSerialNumber = assemblyImageDialog.serialNumber;
                    root.selectedPartNumber = productInstanceModel.data(modelIndex, Qt.UserRole + 11);
                    root.selectedDate = productInstanceModel.data(modelIndex, Qt.UserRole + 1);
                    root.selectedDirectoryName = productInstanceModel.data(modelIndex, Qt.UserRole + 8);
                    root.selectedSeamSeries = seam.seamSeries;
                    root.seamInSeriesIndex = productInstanceModel.seamResultIndex(assemblyImageDialog.productInstanceRow, seam);
                    root.selectedSeam = seam;

                    close();
                }
            }

            Component.onCompleted: assemblyImageFromProductInstanceTableModel.init(productInstanceRow)

            onClosed: destroy()
        }
    }

    ProductInstanceTableModel {
        id: productInstanceModel
        directory: WeldmasterPaths.resultsBaseDir
        stationName: GuiConfiguration.stationName
        extendedProductInfoHelper {
            serialNumberFromExtendedProductInfo: GuiConfiguration.serialNumberFromExtendedProductInfo != 0
            partNumberFromExtendedProductInfo: GuiConfiguration.partNumberFromExtendedProductInfo != 0
        }
        onSortingDataChanged: productInstanceSortFilterModel.forceSort()
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

    ProductInstanceSortModel {
        id: productInstanceSortFilterModel
        sourceModel: productInstanceModel
        sortRole: Qt.UserRole + 1
        sortOrder: Qt.DescendingOrder
    }

    CheckedFilterModel {
        id: checkedFilterModel
        sourceModel: productInstanceModel
        roleName: "checked"
    }

    Component {
        id: importDialogComponent
        ImportVideoSequenceDialog {
            importService: RemovableDevices.DownloadService {
                property string uuid: root.product ? root.product.uuid : ""
                productName: uuid != "" ? uuid.slice(1, uuid.length -1 ) : ""
                backupPath: WeldmasterPaths.resultsBaseDir
                validate: false
            }
            directoryName: "results"
            screenshotTool: root.screenshotTool
            onClosed: destroy()
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

    ColumnLayout {
        anchors.fill: parent

        ColumnLayout {
            Layout.fillWidth: true

            visible: configureButton.checked

            GridLayout {
                columns: 2

                ComboBox {
                    Layout.fillWidth: true

                    id: nioFilterBox

                    model: [qsTr("All results"), qsTr("Faults Only"), qsTr("Remove Faults")]
                    onCurrentIndexChanged: {
                        switch (currentIndex)
                        {
                            case 1:
                                productInstanceSortFilterModel.filterType = ProductInstanceSortModel.OnlyNIO;
                                break;
                            case 2:
                                productInstanceSortFilterModel.filterType = ProductInstanceSortModel.RemoveNIO;
                                break;
                            default:
                                productInstanceSortFilterModel.filterType = ProductInstanceSortModel.All;
                                break;
                        }
                    }
                }

                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: productInstanceSortFilterModel.sortOrder == Qt.DescendingOrder ? "view-sort-descending" : "view-sort-ascending"
                    onClicked: {
                        if (productInstanceSortFilterModel.sortOrder == Qt.DescendingOrder)
                        {
                            productInstanceSortFilterModel.sortOrder = Qt.AscendingOrder;
                        } else
                        {
                            productInstanceSortFilterModel.sortOrder = Qt.DescendingOrder;
                        }
                    }
                }

                TextField {
                    Layout.fillWidth: true
                    Layout.columnSpan: filterOnNio ? 2 : 1

                    placeholderText: qsTr("Search for serial number")
                    selectByMouse: true
                    onTextChanged: {
                        productInstanceSortFilterModel.filter = text;
                    }
                }

                CheckBox {
                    Layout.columnSpan: 2
                    id: dateFilterCheckbox
                    checked: productInstanceSortFilterModel.filterOnDate
                    text: qsTr("Filter on date")
                    onClicked: {
                        productInstanceSortFilterModel.filterOnDate = !productInstanceSortFilterModel.filterOnDate;
                    }
                }

                RowLayout {
                    Layout.columnSpan: 2
                    Layout.alignment: Qt.AlignHCenter

                    visible: dateFilterCheckbox.checked
                    spacing: 20

                    Label {
                        text: qsTr("From:")
                        font.bold: true
                    }
                    DateSelector {
                        onDateChanged: {
                            productInstanceSortFilterModel.from = date;
                        }
                    }
                    Label {
                        text: qsTr("To:")
                        font.bold: true
                    }
                    DateSelector {
                        onDateChanged: {
                            productInstanceSortFilterModel.to = date;
                        }
                    }
                }
            }
        }

        Label {
            id: missingDeviceLabel
            visible: serviceButtons.visible && RemovableDevices.Service.udi == ""
            text: root.resultsExporter ? qsTr("Please connect a removable device in order to download or export product instances") : qsTr("Please connect a removable device in order to download product instances")
        }
        Label {
            id: missingMountLabel
            visible: serviceButtons.visible && RemovableDevices.Service.udi != "" && RemovableDevices.Service.path == ""
            text: root.resultsExporter ? qsTr("Please mount the removable device in order to download or export product instances") : qsTr("Please mount the removable device in order to download product instances")
        }

        Flickable {
            Layout.fillWidth: true

            implicitHeight: horizontalHeader.implicitHeight

            clip: true

            Row {
                id: horizontalHeader

                x: -table.contentX
                z: 3

                spacing: 10

                Repeater {
                    id: horizontalHeaderRepeater

                    model: table.columns

                    ColumnLayout {
                        width: index === 0 ? table.mainItemDelegateWidth : Math.max(implicitWidth, 40)
                        Label {
                            Layout.fillWidth: true
                            horizontalAlignment: index === 0 ? Text.AlignRight : Text.AlignHCenter
                            font.bold: true
                            text: productInstanceSortFilterModel.headerData(modelData, Qt.Horizontal)
                        }
                        Label {
                            Layout.fillWidth: true
                            horizontalAlignment: index === 0 ? Text.AlignRight : Text.AlignHCenter
                            font.bold: true
                            text: productInstanceSortFilterModel.headerData(modelData, Qt.Horizontal, Qt.UserRole)
                        }

                        Component.onCompleted: {
                            if (index === 0)
                            {
                                // ensure that the mainItemDelegateWidth is set. Otherwise, if the table is empty, the first item width will be 0
                                table.mainItemDelegateWidth = Math.max(table.mainItemDelegateWidth, implicitWidth, 40)
                            }
                        }
                    }
                }
            }
        }

        TableView {
            property int mainItemDelegateWidth: 0

            Layout.fillWidth: true
            Layout.fillHeight: true

            id: table

            ScrollBar.horizontal: ScrollBar {}
            ScrollBar.vertical: ScrollBar {}

            clip: true

            boundsBehavior: Flickable.StopAtBounds
            rowSpacing: 10
            columnSpacing: 10
            objectName: root.objectName + "-tableview"

            model: productInstanceSortFilterModel

            columnWidthProvider: function (column) {
                if (column === 0)
                {
                    return table.mainItemDelegateWidth;
                }
                return horizontalHeaderRepeater.itemAt(column).width;
            }

            delegate: ItemDelegate {
                property bool isMainItem: column === 0
                property string diskUsage: ""

                id: delegateItem

                implicitHeight: mainItemGrid.implicitHeight
                implicitWidth: table.columnWidthProvider(column)

                enabled: isMainItem || model.nio != ProductInstanceTableModel.Unknown
                objectName: TableView.view.objectName + "-item-" + index

                onClicked: {
                    root.selectedProductInstance = model.fileInfo
                    root.selectedSerialNumber = model.display
                    root.selectedPartNumber = model.partNumber
                    root.selectedDate = model.date
                    root.selectedDirectoryName = model.directoryName

                    if (!isMainItem)
                    {
                        root.selectedSeamSeries = model.seamSeries
                        root.seamInSeriesIndex = productInstanceModel.seamResultIndex(row, model.seam)
                        root.selectedSeam = model.seam
                    }
                }

                Component.onCompleted: {
                    table.mainItemDelegateWidth = Math.max(table.mainItemDelegateWidth, mainItemGrid.implicitWidth)

                    if (downloadButton.visible && isMainItem)
                    {
                        model.downloadService.diskUsageAsync(model.path);
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

                GridLayout {
                    anchors.fill: parent

                    id: mainItemGrid

                    columns: 1 + (nioIndicatorVisible ? 1 : 0) + (downloadExportBox.visible ? 1 : 0) + (assemblyImageButton.visible ? 1 : 0)

                    visible: isMainItem

                    CheckBox {
                        Layout.rowSpan: model.partNumber != "" ? 3 : 2
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                        id: downloadExportBox

                        checked: model.checked
                        visible: serviceButtons.visible
                        onClicked: {
                            model.checked = downloadExportBox.checked;
                        }
                    }

                    ToolButton {
                        Layout.rowSpan: model.partNumber != "" ? 3 : 2
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                        id: assemblyImageButton
                        icon.name: "view-assembly-image"
                        text: qsTr("View in assembly image")
                        display: Button.IconOnly
                        visible: root.product && root.product.assemblyImage != ""

                        onClicked: {
                            var dialog = assemblyImageDialogComponent.createObject(root, {serialNumber: model.display, productInstanceRow: productInstanceSortFilterModel.mapToSource(productInstanceSortFilterModel.index(row, 0)).row});
                            dialog.open();
                        }
                    }

                    Rectangle {
                        Layout.preferredWidth: 25
                        Layout.preferredHeight: 25
                        Layout.rowSpan: model.partNumber != "" ? 3 : 2
                        Layout.alignment: Qt.AlignVCenter | Qt.AlignLeft

                        visible: root.nioIndicatorVisible
                        radius: width * 0.5

                        color: model.nio == ProductInstanceTableModel.Nio ? "red" : (model.nio == ProductInstanceTableModel.Io ? "green" : "transparent")
                        border {
                            color: "yellow"
                            width: model.nioResultsSwitchedOff ? 5 : 0
                        }
                        BusyIndicator {
                            anchors.fill: parent
                            running: model.nio == ProductInstanceModel.Unknown
                        }
                    }

                    Label {
                        Layout.alignment: Qt.AlignLeft

                        text: model.display
                        font.bold: true
                    }
                    Label {
                        Layout.alignment: Qt.AlignLeft

                        visible: model.partNumber != ""
                        //: a label containing a part number (argument is the part number)
                        text: qsTr("Part Number: %1").arg(model.partNumber)

                        onImplicitWidthChanged: {
                            var maxWidth = Math.max(table.mainItemDelegateWidth, mainItemGrid.implicitWidth);
                            if (table.mainItemDelegateWidth != maxWidth)
                            {
                                table.mainItemDelegateWidth = maxWidth;
                                table.forceLayout();
                            }
                        }
                    }

                    Label {
                        Layout.fillWidth: true

                        text: Qt.formatDateTime(model.date, "yyyy-MM-dd hh:mm:ss.zzz") + (delegateItem.diskUsage != "" ? " (%1)".arg(delegateItem.diskUsage) : "")

                        onImplicitWidthChanged: {
                            var maxWidth = Math.max(table.mainItemDelegateWidth, mainItemGrid.implicitWidth);
                            if (table.mainItemDelegateWidth != maxWidth)
                            {
                                table.mainItemDelegateWidth = maxWidth;
                                table.forceLayout();
                            }
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
                }

                Rectangle {
                    anchors.centerIn: parent

                    id: tableItem

                    visible: !isMainItem
                    width: 25
                    height: 25
                    radius: 0.5 * width

                    color: model.nio == ProductInstanceTableModel.Nio ? "red" : (model.nio == ProductInstanceTableModel.Io ? "green" : "transparent")
                    border {
                        color: "yellow"
                        width: model.nioResultsSwitchedOff ? 5 : 0
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignLeft
            visible: serviceButtons.visible

            Button {
                text: qsTr("Check All")
                onClicked: {
                    for (var i = 0; i < productInstanceSortFilterModel.rowCount; i++)
                    {
                        var index = productInstanceSortFilterModel.mapToSource(productInstanceSortFilterModel.index(i, 0));
                        productInstanceModel.setData(index, true, Qt.UserRole + 7);
                    }
                }
            }

            Button {
                text: qsTr("Uncheck All")
                onClicked: {
                    for (var i = 0; i < productInstanceSortFilterModel.rowCount; i++)
                    {
                        var index = productInstanceSortFilterModel.mapToSource(productInstanceSortFilterModel.index(i, 0));
                        productInstanceModel.setData(index, false, Qt.UserRole + 7);
                    }
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            id: serviceButtons

            visible: UserManagement.currentUser && UserManagement.hasPermission(App.MountPortableDevices)

            Button {
                text: qsTr("Import Product Instance")
                icon.name: "cloud-upload"
                visible: UserManagement.currentUser && UserManagement.hasPermission(App.UploadResults)
                enabled: RemovableDevices.Service.path != ""

                onClicked: {
                    importDialogComponent.createObject(root).open();
                }
            }

            Button {
                id: downloadButton
                objectName: "download-button"
                property bool downloading: productInstanceModel.downloading

                text: downloading ? qsTr("Cancel Download") : (checkedFilterModel.rowCount > 0 ? qsTr("Download %1 items").arg(checkedFilterModel.rowCount) : qsTr("Download"))
                visible: UserManagement.currentUser && UserManagement.hasPermission(App.ExportResults)
                enabled: RemovableDevices.Service.path != "" && checkedFilterModel.rowCount != 0
                icon.name: "edit-download"
                onClicked: {
                    if (!UserManagement.hasPermission(App.MountPortableDevices) || !UserManagement.hasPermission( App.ExportResults))
                    {
                        return
                    }
                    var path = "results";
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
                visible: root.resultsExporter && UserManagement.currentUser && UserManagement.hasPermission(App.ExportResults)
                enabled: root.resultsExporter && !root.resultsExporter.exporting && root.resultsExporter.exportDirectory != "" && checkedFilterModel.rowCount != 0
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
                    running: root.resultsExporter && root.resultsExporter.exporting
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: productInstanceModel.loading
    }
}

