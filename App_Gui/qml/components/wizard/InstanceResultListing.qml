import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

ColumnLayout {
    property alias monitoring: productInstanceModel.monitoring
    property alias currentProduct: productInstanceModel.product
    property alias seam: instanceDataModel.seam
    property bool loading: productInstanceModel.loading || instanceDataModel.loading
    property alias implicitListWidth: instanceList.implicitWidth
    property alias resultType: instanceDataModel.resultType
    property alias triggerType: instanceDataModel.triggerType
    property alias threshold: instanceDataModel.threshold
    property alias result: instanceDataModel.result
    property alias trigger: instanceDataModel.trigger

    readonly property bool empty: instanceList.count === 0

    id: root

    ProductInstanceModel {
        id: productInstanceModel
        directory: WeldmasterPaths.resultsBaseDir
        extendedProductInfoHelper {
            serialNumberFromExtendedProductInfo: GuiConfiguration.serialNumberFromExtendedProductInfo != 0
            partNumberFromExtendedProductInfo: GuiConfiguration.partNumberFromExtendedProductInfo != 0
        }

        onLoadingChanged: {
            if (!productInstanceModel.loading)
            {
                productInstanceModel.ensureAllMetaDataLoaded();
            }
        }
    }

    InstanceResultModel {
        id: instanceDataModel
        productInstanceModel: productInstanceModel
    }

    InstanceResultSortModel {
        id: instanceDataSortModel
        sourceModel: instanceDataModel
    }

    InstanceListItem {
        Layout.fillHeight: true
        Layout.fillWidth: true

        id: instanceList

        model: instanceDataSortModel
        productInstanceModel: productInstanceModel
        currentIndex: instanceDataSortModel.mapFromSource(instanceDataModel.currentIndex).row
        loading: root.loading

        onItemClicked: {
            instanceDataModel.currentIndex = index;
            instanceDataModel.result.color = color;
        }
    }

    ComboBox {
        Layout.fillWidth: true

        visible: menuButton.checked
        model: [qsTr("All results"), qsTr("Faults Only"), qsTr("Remove Faults")]
        onCurrentIndexChanged: {
            switch (currentIndex)
            {
                case 1:
                    instanceDataSortModel.filterType = InstanceResultSortModel.OnlyNIO;
                    break;
                case 2:
                    instanceDataSortModel.filterType = InstanceResultSortModel.RemoveNIO;
                    break;
                default:
                    instanceDataSortModel.filterType = InstanceResultSortModel.All;
                    break;
            }
        }
    }

    CheckBox {
        Layout.fillWidth: true
        visible: menuButton.checked
        text: qsTr("Include linked seams")
        checked: instanceDataSortModel.includeLinkedSeams
        onToggled: {
            instanceDataSortModel.includeLinkedSeams = !instanceDataSortModel.includeLinkedSeams;
        }
    }

    RowLayout {
        Layout.fillWidth: true
        visible: menuButton.checked

        ComboBox {
            Layout.fillWidth: true
            model: [qsTr("Name"), qsTr("Date")]
            onCurrentIndexChanged: {
                instanceDataSortModel.sortOnDate = currentIndex !== 0;
            }
        }

        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: instanceDataSortModel.sortOrder === Qt.DescendingOrder ? "view-sort-descending" : "view-sort-ascending"
            onClicked: {
                if (instanceDataSortModel.sortOrder === Qt.DescendingOrder)
                {
                    instanceDataSortModel.sortOrder = Qt.AscendingOrder;
                } else
                {
                    instanceDataSortModel.sortOrder = Qt.DescendingOrder;
                }

                // reselect index after sorting
                var row = instanceDataModel.currentIndex.row;
                instanceDataModel.resetCurrentIndex();
                instanceDataModel.currentIndex = instanceDataModel.index(row, 0);
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true

        Button {
            Layout.fillWidth: true
            text: qsTr("Clear")
            onClicked: instanceDataModel.resetCurrentIndex()
        }
        ToolButton {
            id: menuButton
            display: AbstractButton.IconOnly
            icon.name: "application-menu"
            checkable: true
        }
    }
}

