import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.filterparametereditor 1.0
import precitec.gui.general 1.0

BreadCrumpGroupBox {
    id: root
    property alias keyValueAttributeModel: overviewModel.keyValueAttributeModel
    title: qsTr("Overview of Hardware Parameters")
    productToNextLevelEnabled: false

    HardwareParametersOverviewModel {
        id: overviewModel
        product: root.product
    }

    HardwareParametersOverviewSortFilterModel {
        id: overviewSortFilterModel
        sourceModel: overviewModel
    }

    TableView {
        id: tableView
        anchors.fill: parent
        clip: true
        columnSpacing: 5
        rowSpacing: 5
        interactive: true
        boundsBehavior: Flickable.StopAtBounds

        leftMargin: verticalHeader.implicitWidth + 5
        topMargin: horizontalHeader.implicitHeight + 5

        ScrollBar.horizontal: ScrollBar {}
        ScrollBar.vertical: ScrollBar {}

        model: overviewSortFilterModel
        delegate: Label {
            text: model.display != undefined ? model.display : " "
            horizontalAlignment: Text.AlignHCenter
            color: PrecitecApplication.Settings.text
        }
        columnWidthProvider: function (column) { return overviewModel.columnWidth(column) }

        onLeftMarginChanged: {
            tableView.contentX = -tableView.leftMargin;
        }

        onTopMarginChanged: {
            tableView.contentY = -tableView.topMargin;
        }

        TextMetrics {
            id: textMetrics
            text: "Q"
        }

        // background below the horizontal header, to "clip" the table view while scrolling
        Rectangle {
            z: 2
            color: "white"
            anchors.fill: horizontalHeader
        }

        Row {
            id: horizontalHeader
            y: tableView.contentY
            z: 3
            spacing: tableView.columnSpacing
            Repeater {
                id: horizontalHeaderRepeater
                model: tableView.columns >= 0 ? tableView.columns : 0

                Label {
                    width: tableView.columnWidthProvider(modelData)
                    text: tableView.model.headerData(modelData, Qt.Horizontal)
                    font.bold: true
                    horizontalAlignment: Text.AlignHCenter
                }
            }
        }

        // background below the vertical header, to "clip" the table view while scrolling
        Rectangle {
            z: 2
            color: "white"
            anchors.fill: verticalHeader
        }
        ColumnLayout {
            id: verticalHeader
            x: tableView.contentX
            z: 3
            spacing: tableView.rowSpacing
            Repeater {
                id: verticalHeaderRepeater
                model: tableView.rows >= 0 ? tableView.rows : 0

                Label {
                    text: tableView.model.headerData(modelData, Qt.Vertical)
                    font.bold: true
                    Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                }
            }
        }

        // horizontal seperator lines
        Repeater {
            model: verticalHeaderRepeater.model
            Rectangle {
                height: 1
                width: tableView.contentWidth + tableView.leftMargin
                visible: y >= (tableView.contentY + tableView.topMargin)
                x: tableView.contentX
                y: verticalHeaderRepeater.itemAt(modelData).y
                z: 4
                color: PrecitecApplication.Settings.alternateBackground
            }
        }

        // vertical seperator lines
        Repeater {
            model: horizontalHeaderRepeater.model
            Rectangle {
                height: tableView.contentHeight + tableView.topMargin
                width: 1
                visible: x >= (tableView.contentX + tableView.leftMargin - tableView.columnSpacing / 2)
                x: horizontalHeaderRepeater.itemAt(modelData).x - tableView.columnSpacing / 2
                y: tableView.contentY
                z: 4
                color: PrecitecApplication.Settings.alternateBackground
            }
        }

        // an overlay in the top left corner to "clip" the headers while scrolling
        Rectangle {
            z: 5
            x: tableView.contentX
            y: tableView.contentY
            width: verticalHeader.width
            height: horizontalHeader.height
            color: "white"
        }

        Label {
            z: 5
            x: tableView.contentX
            y: tableView.contentY
            width: verticalHeader.width
            height: horizontalHeader.height
            text: GuiConfiguration.seamSeriesOnProductStructure ? qsTr("Seam Series\nSeam") : qsTr("Seam")
            font.bold: true
            horizontalAlignment: Text.AlignRight
        }
    }
}
