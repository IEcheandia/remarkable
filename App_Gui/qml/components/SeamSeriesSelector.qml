import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.notifications 1.0
import precitec.gui 1.0

/**
 * GroupBox providing selection of a seam series from a product instance through a ProductInstanceModel.
 **/
GroupBox {
    id: root

    property var selectedSerialNumber: undefined

    property date selectedDate

    property var selectedProduct: null

    property var resultsExporter: null

    property var selectedSeamSeries: null

    property alias selectedProductInstance: seriesModel.productInstance

    property alias labelHeight: titleLabel.height

    property string selectedPartNumber: ""

    onSelectedProductInstanceChanged: {
        root.selectedSeamSeries = -1;
    }

    signal selectSeries()

    label: Label {
        id: titleLabel
        text: qsTr("Select Seam Series of Product \"%2\" (SN: %3 from %4)").arg(selectedProduct ? selectedProduct.name : "").arg(selectedSerialNumber).arg(Qt.formatDateTime(selectedDate, "yyyy-MM-dd hh:mm:ss.zzz"))
        font.family: root.font.family
        font.pixelSize: root.font.pixelSize
        font.bold: true
        verticalAlignment: Text.AlignVCenter

        leftPadding: root.leftPadding
    }
    ColumnLayout {
        anchors.fill: parent
        Label {
            //: a label containing a part number (argument is the part number)
            text: qsTr("Part Number: %1").arg(root.selectedPartNumber)
            visible: root.selectedPartNumber != ""
            font.family: root.font.family
            font.pixelSize: root.font.pixelSize
            verticalAlignment: Text.AlignVCenter
        }
        Label {
            visible: RemovableDevices.Service.udi == ""
            text: qsTr("Please connect a removable device in order to export seam series")
        }
        Label {
            visible: RemovableDevices.Service.udi != "" && RemovableDevices.Service.path == ""
            text: qsTr("Please mount the removable device in order to export seam series")
        }
        ScrollView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            ListView {
                id: seriesSelector
                objectName: root.objectName + "-listview"
                anchors.fill: parent
                spacing: 5
                clip: true
                model: ProductInstanceSeriesModel {
                    id: seriesModel
                }
                delegate: RowLayout {
                    id: delegateItem
                    property var parentObjectName: ListView.view.objectName
                    property var seamSeries: root.selectedProduct ? root.selectedProduct.findSeamSeries(model.number) : null
                    enabled: seamSeries != null
                    width: ListView.view.width
                    ToolButton {
                        icon.name: "document-export"
                        text: qsTr("Export")
                        display: Button.IconOnly
                        visible: UserManagement.currentUser && UserManagement.hasPermission(App.MountPortableDevices) && UserManagement.hasPermission(App.ExportResults)
                        enabled: !resultsExporter.exporting && resultsExporter.exportDirectory != ""
                        onClicked: {
                            resultsExporter.performExport(seriesSelector.model.productInstance, root.selectedDate, root.selectedProduct, root.selectedSerialNumber, model.number)
                            if (resultsExporter.exporting)
                            {
                                busyIndicator.active = true;
                            }
                        }

                        BusyIndicator {
                            id: busyIndicator
                            property bool active: false
                            anchors.fill: parent
                            running: resultsExporter.exporting && active

                            Connections {
                                target: resultsExporter
                                function onExportingChanged() {
                                    if (!resultsExporter.exporting)
                                    {
                                        busyIndicator.active = false;
                                    }
                                }
                            }
                        }
                    }
                    ItemDelegate {
                        Layout.fillWidth: true
                        objectName: parent.parentObjectName + "-item-" + index
                        indicator: Rectangle {
                            id: seamIndicator
                            width: 25
                            height: 25
                            radius: width * 0.5
                            color: model.nio == ProductInstanceModel.Nio ? "red" : (model.nio == ProductInstanceModel.Io ? "green" : "transparent")
                            border {
                                color: "yellow"
                                width: model.nioResultsSwitchedOff ? 5 : 0
                            }
                            anchors {
                                left: parent.left
                                verticalCenter: parent.verticalCenter
                            }
                            BusyIndicator {
                                anchors.fill: parent
                                running: model.nio == ProductInstanceModel.Unknown
                            }
                        }
                        contentItem: Label {
                            text: model.visualNumber + (delegateItem.seamSeries ? ": " + delegateItem.seamSeries.name : "")
                            leftPadding: seamIndicator.width + spacing
                        }
                        onClicked: {
                            selectedSeamSeries = delegateItem.seamSeries;
                            root.selectSeries();
                        }
                    }
                }
            }
        }
    }
}

