import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.filterparametereditor 1.0
import precitec.gui.general 1.0

BackButtonGroupBox {
    id: root
    property var screenshotTool: null
    property alias seam: parameterSetToGraphDeltaModel.seam
    property alias graphModel: parameterSetToGraphDeltaModel.graphModel
    property alias subGraphModel: parameterSetToGraphDeltaModel.subGraphModel
    property alias attributeModel: parameterSetToGraphDeltaModel.attributeModel
    property var resultsConfigModel: null
    property var sensorConfigModel: null

    signal markAsChanged()

    title: qsTr("Delta of detection parameters of \"%1\" (%2) to default of graph").arg(root.seam ? root.seam.name : "").arg(root.seam ? root.seam.visualNumber : -1)

    onSeamChanged: {
        if (!seam)
        {
            root.back();
        }
    }

    Component {
        id: editDialogComponent
        Dialog {
            id: editDialog
            property int column: -1
            property int row: -1
            property var modelIndex: parameterSetToGraphDeltaModel.index(editDialog.row, editDialog.column)
            property var parameter: parameterSetToGraphDeltaModel.getFilterParameter(editDialog.modelIndex)
            property var attribute: parameterSetToGraphDeltaModel.headerData(row, Qt.Vertical, Qt.UserRole + 2)
            property var defaultValue: parameterSetToGraphDeltaModel.headerData(row, Qt.Vertical, Qt.UserRole + 4)
            property var seam: parameterSetToGraphDeltaModel.headerData(column, Qt.Horizontal, Qt.UserRole + 1)
            title: qsTr("Edit filter parameter")
            parent: Overlay.overlay
            anchors.centerIn: parent
            width: root.width * 0.8
            height: root.height * 0.8
            modal: true
            standardButtons: Dialog.Close

            header: RowLayout {
                ItemDelegate {
                    icon.source: "file://" + WeldmasterPaths.filterPictureDir + tableView.model.headerData(editDialog.row, Qt.Vertical, Qt.UserRole + 1) + ".png"
                    icon.color: "transparent"
                    text: tableView.model.headerData(editDialog.row, Qt.Vertical, Qt.UserRole) + "\n" + tableView.model.headerData(editDialog.row, Qt.Vertical, Qt.DisplayRole)
                    background: Item { }
                    Layout.column: 0
                    Layout.row: editDialog.row
                    Layout.fillHeight: true
                }
                AttributeLabel {
                    attribute: editDialog.attribute
                    font.bold: true
                    Layout.fillWidth: true
                }
                ToolButton {
                    id: screenshotToolButton
                    visible: root.screenshotTool != null
                    icon.name: "camera-photo"
                    icon.width: 32
                    icon.height: 32
                    flat: true
                    onClicked: root.screenshotTool.takeScreenshot()
                    palette.button: "transparent"
                }
            }

            onAccepted: destroy()
            onRejected: destroy()

            ColumnLayout {
                anchors.fill: parent
                Label {
                    visible: editDialog.seam != null
                    text: editDialog.seam ? qsTr("Seam") + ": " + editDialog.seam.name + " (" + editDialog.seam.visualNumber + ")" : ""
                }
                Label {
                    visible: editDialog.seam != null && GuiConfiguration.seamSeriesOnProductStructure
                    text: editDialog.seam ? qsTr("Seam series") + ": " + editDialog.seam.seamSeries.name + " (" + editDialog.seam.seamSeries.visualNumber + ")" : ""
                }
                RowLayout {
                    ParameterEditor {
                        id: editor
                        attribute: editDialog.attribute
                        parameter: editDialog.parameter
                        defaultValue: editDialog.defaultValue
                            resultsConfigModel: root.resultsConfigModel
                            sensorConfigModel: root.sensorConfigModel

                            onParameterValueModified: {
                                parameterSetToGraphDeltaModel.updateFilterParameter(editDialog.modelIndex, editor.item.parameterValue);
                                root.markAsChanged();
                            }
                            Layout.fillWidth: true
                    }
                    Label {
                        id: unitLabel
                        Layout.leftMargin: 10
                        font.pixelSize: 14
                        text: editDialog.attribute ? LanguageSupport.getString(editDialog.attribute.unit) : ""
                        visible: editDialog.attribute && editDialog.attribute.unit != text && text != ""
                    }
                }
                Item {
                    Layout.fillHeight: true
                }
            }
        }
    }

    ParameterSetToGraphDeltaModel {
        id: parameterSetToGraphDeltaModel
    }

    TableView {
        id: tableView
        anchors.fill: parent
        clip: true
        columnSpacing: 5
        rowSpacing: 0
        interactive: true
        flickableDirection: Flickable.VerticalFlick
        leftMargin: verticalHeader.implicitWidth + columnSpacing
        topMargin: horizontalHeader.implicitHeight + 5

        ScrollBar.horizontal: ScrollBar{}
        ScrollBar.vertical: ScrollBar{}

        TextMetrics {
            id: seamValueMetrics
            text: parameterSetToGraphDeltaModel.longestValueSeam
        }
        TextMetrics {
            id: graphValueMetrics
            text: parameterSetToGraphDeltaModel.longestValueGraph
        }

        columnWidthProvider: function (column)
        {
            if (column == 0)
            {
                return Math.max(seamValueMetrics.advanceWidth, seamHeader.implicitWidth);
            }
            if (column == 1)
            {
                return Math.max(graphValueMetrics.advanceWidth, graphHeader.implicitWidth);
            }
            return 1;
        }

        rowHeightProvider: function (row)
        {
            return verticalHeaderRepeater.itemAt(row).height;
        }

        model: parameterSetToGraphDeltaModel
        delegate: Label {
            id: delegate
            property bool canEdit: verticalHeaderRepeater.itemAt(row).canEdit
            property bool isGraphColumn: model.column === 1
            text: model.display.value != undefined ? model.display.value : " "
            width: tableView.columnWidthProvider(column)
            height: tableView.rowHeightProvider(row)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            color: canEdit ? "black" : "darkgrey"
            MouseArea {
                id: mouseArea
                anchors.fill: parent
                enabled: delegate.canEdit && !delegate.isGraphColumn
                onClicked: {
                    var dialog = editDialogComponent.createObject(root, {"column": column, "row": row});
                    dialog.open();
                }
            }
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

            Label {
                id: seamHeader
                width: tableView.columnWidthProvider(0)
                text: qsTr("Seam")
                font.bold: true
            }
            Label {
                id: graphHeader
                width: tableView.columnWidthProvider(1)
                text: qsTr("Graph")
                font.bold: true
            }
        }

        // background below the vertical header, to "clip" the table view while scrolling
        Rectangle {
            z: 2
            color: "white"
            anchors.fill: verticalHeader
        }

        Column {
            id: verticalHeader

            x: tableView.contentX
            z: 3
            spacing: tableView.rowSpacing

            GridLayout {
                columns: 2
                rowSpacing: 0
                Repeater {
                    id: verticalHeaderRepeater
                    model: tableView.rows >= 0 ? tableView.rows : 0
                    ItemDelegate {
                        property string groupName: tableView.model.headerData(modelData, Qt.Vertical, Qt.UserRole)
                        property string subGraphName: root.seam.usesSubGraph ? tableView.model.headerData(modelData, Qt.Vertical, Qt.UserRole + 5) + "\n" : ""
                        property string instanceFilterName: tableView.model.headerData(modelData, Qt.Vertical, Qt.DisplayRole)
                        property bool canEdit: parameterSetToGraphDeltaModel.headerData(modelData, Qt.Vertical, Qt.UserRole + 3)
                        icon.source: "file://" + WeldmasterPaths.filterPictureDir + tableView.model.headerData(modelData, Qt.Vertical, Qt.UserRole + 1) + ".png"
                        icon.color: "transparent"
                        icon.width: 64
                        icon.height: 64
                        text: subGraphName + groupName + "\n" + instanceFilterName
                        background: Item { }
                        Layout.column: 0
                        Layout.row: modelData
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
                Repeater {
                    model: verticalHeaderRepeater.model
                    AttributeLabel {
                        attribute: tableView.model.headerData(modelData, Qt.Vertical, Qt.UserRole + 2)
                        font.bold: true
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        Layout.column: 1
                        Layout.row: modelData
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                    }
                }
            }
        }

        // horizontal seperator lines
        Repeater {
            model: verticalHeaderRepeater.model
            Rectangle {
                height: 1
                visible: y >= (tableView.contentY + tableView.topMargin)
                width: tableView.contentWidth + tableView.leftMargin
                x: tableView.contentX
                y: verticalHeaderRepeater.itemAt(modelData).y
                z: 4
                color: PrecitecApplication.Settings.alternateBackground
            }
        }

        // vertical seperator line 1
        Rectangle {
            height: tableView.contentHeight + tableView.topMargin
            width: 1
            x: seamHeader.x - tableView.columnSpacing / 2
            y: tableView.contentY
            z: 4
            color: PrecitecApplication.Settings.alternateBackground
        }

        // vertical seperator line 1
        Rectangle {
            height: tableView.contentHeight + tableView.topMargin
            width: 1
            x: graphHeader.x - tableView.columnSpacing / 2
            y: tableView.contentY
            z: 4
            color: PrecitecApplication.Settings.alternateBackground
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
    }
}

