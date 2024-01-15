import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.notifications 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0

GroupBox {
    id: root

    property var selectedSerialNumber: undefined

    property date selectedDate

    property var selectedProduct: null

    property var resultsExporter: null

    property var selectedSeam: null

    property alias selectedSeamSeries: seamModel.seamSeries

    property alias selectedProductInstance: seamModel.productInstance

    property alias labelHeight: titleLabel.height

    property alias labelVisible: titleLabel.visible

    property alias currentIndex: seamSelector.currentIndex

    property alias model: seamModel

    property alias count: seamSelector.count

    property string selectedPartNumber: ""

    onSelectedProductInstanceChanged: {
        root.selectedSeam = null;
    }

    signal selectSeam()

    function selectNext()
    {
        seamSelector.incrementCurrentIndex();
        root.selectedSeam = seamSelector.currentItem.seam;
        root.selectSeam();
    }

    function selectPrevious()
    {
        seamSelector.decrementCurrentIndex();
        root.selectedSeam = seamSelector.currentItem.seam;
        root.selectSeam();
    }

    label: Label {
        id: titleLabel
        text: qsTr("Select Seam of %1Product \"%2\" (SN: %3 from %4)").arg(GuiConfiguration.seamSeriesOnProductStructure ? qsTr("Series %1, ").arg(root.selectedSeamSeries ? root.selectedSeamSeries.visualNumber : "") : "").arg(selectedProduct ? selectedProduct.name : "").arg(selectedSerialNumber).arg(Qt.formatDateTime(selectedDate, "yyyy-MM-dd hh:mm:ss.zzz"))
        font.family: root.font.family
        font.pixelSize: root.font.pixelSize
        font.bold: true
        verticalAlignment: Text.AlignVCenter

        leftPadding: root.leftPadding
    }
    ProductInstanceSeamModel {
        id: seamModel
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
            Layout.fillWidth: true
            visible: RemovableDevices.Service.udi == ""
            text: qsTr("Please connect a removable device in order to export seams")
            wrapMode: Text.WordWrap
        }
        Label {
            Layout.fillWidth: true
            visible: RemovableDevices.Service.udi != "" && RemovableDevices.Service.path == ""
            text: qsTr("Please mount the removable device in order to export seams")
            wrapMode: Text.WordWrap
        }
        ScrollView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            ListView {
                id: seamSelector
                spacing: 5
                clip: true
                model: ProductInstanceSeamSortModel {
                    productInstanceSeamModel: seamModel
                }
                Component.onCompleted: {
                    seamSelector.objectName = UserManagement.currentUser.name + "-results-seam-selector";
                }
                delegate: RowLayout {
                    id: delegateItem
                    property var parentObjectName: ListView.view.objectName
                    property var seam: (root.selectedProduct && root.selectedSeamSeries) ? root.selectedProduct.findSeam(root.selectedSeamSeries.number, model.number) : null
                    enabled: seam != null
                    width: ListView.view.width
                    ToolButton {
                        icon.name: "document-export"
                        text: qsTr("Export")
                        display: Button.IconOnly
                        visible: UserManagement.currentUser && UserManagement.hasPermission(App.MountPortableDevices) && UserManagement.hasPermission(App.ExportResults)
                        enabled: !resultsExporter.exporting && resultsExporter.exportDirectory != ""
                        onClicked: {
                            resultsExporter.performExport(root.selectedProductInstance, root.selectedDate, root.selectedProduct, root.selectedSerialNumber, seamModel.seamSeries, model.number)
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
                        property var linkTo: delegateItem.seam && delegateItem.seam.linkTo
                        objectName: parent.parentObjectName + "-item-" + index
                        icon.name: linkTo != undefined ? "link" : ""
                        indicator: Rectangle {
                            id: indicator
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
                        text: (linkTo != undefined ? linkTo.visualNumber + ": " + linkTo.name + "\n" : "") + model.visualNumber + (delegateItem.seam ? ": " +  delegateItem.seam.name : "")
                        onClicked: {
                            root.selectedSeam = delegateItem.seam;
                            seamSelector.currentIndex = index;
                            root.selectSeam();
                        }
                        Component.onCompleted: {
                            contentItem.leftPadding = Qt.binding(function() { return indicator.width + spacing; });
                        }
                        Layout.fillWidth: true
                    }
                }
            }
        }
    }
}
