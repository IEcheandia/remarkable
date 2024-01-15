import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.12

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication

Rectangle {
    property alias product: selectionModel.product
    property alias currentSeam: selectionModel.currentSeam
    property alias seamPropertyModel: selectionModel.propertyModel
    property alias font: selectionModel.font
    property alias productController: selectionModel.productController

    signal changed()
    signal save()
    signal markAsChanged()

    onSave: selectionModel.save()

    id: root

    color: "#3d4045"

    SeamSelectionModel {
        id: selectionModel
        onDataChanged: root.changed()
        onMarkAsChanged: root.markAsChanged()
    }

    FontMetrics {
        id: fontMetrics
        font: selectionModel.font
    }

    GridLayout {
        columns: 2

        anchors {
            fill: parent
            margins: 5
        }

        TableView {
            property color borderColor: "#63666b"
            property int seamLabelWidth: fontMetrics.advanceWidth(qsTr("Seam 123"))
            property int seriesLabelWidth: fontMetrics.advanceWidth(qsTr("Series 123"))
            property int cellMargin: 20

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.columnSpan: 2

            id: seamTable

            leftMargin: verticalHeader.implicitWidth + 5
            topMargin: horizontalHeader.implicitHeight + 5
            rightMargin: 5
            bottomMargin: 5

            columnWidthProvider: function (column) { return Math.min(Math.max(model.columnWidth(column), seamTable.seamLabelWidth) + seamTable.cellMargin, 400); }
            rowHeightProvider: function (row) { return fontMetrics.height + seamTable.cellMargin; }

            ScrollBar.horizontal: ScrollBar{}
            ScrollBar.vertical: ScrollBar{}

            clip: true

            boundsBehavior: Flickable.StopAtBounds

            model: selectionModel

            delegate: CheckBox {
                id: control
                checked: model.selected
                enabled: model.enabled && !model.current && !model.linked
                onToggled: model.selected = !model.selected;

                indicator: Rectangle {
                    implicitWidth: seamTable.columnWidthProvider(column)
                    implicitHeight: seamTable.rowHeightProvider(row)

                    color: model.enabled ? PrecitecApplication.Settings.alternateBackground : "white"

                    Rectangle {
                        anchors {
                            fill: parent
                            topMargin    : model.top ? 3 : 0
                            bottomMargin : model.bottom ? 3 : 0
                            leftMargin   : model.left ? 3 : 0
                            rightMargin  : model.right ? 3 : 0
                        }
                        color: "white"
                        visible: model.enabled
                    }

                    Label {
                        anchors {
                            fill: parent
                            margins: 5
                        }
                        text: model.name
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        elide: Text.ElideRight
                        visible: model.enabled
                    }

                    Image {
                        id: checkSign
                        anchors {
                            right: parent.right
                            bottom: parent.bottom
                        }
                        source: model.linked ? "qrc:/icons/precitec/scalable/link.svg" : "qrc:/qt-project.org/imports/QtQuick/Controls.2/images/check.png"
                        smooth: true
                        visible: false
                    }

                    ColorOverlay {
                        anchors.fill: checkSign
                        source: checkSign
                        color: model.linked ? "grey" : PrecitecApplication.Settings.alternateBackground
                        visible: model.selected || model.linked
                    }
                }
            }

            Rectangle {
                id: corner
                z: 3
                color: root.color
                y: seamTable.contentY
                x: seamTable.contentX
                width: seamTable.leftMargin - 5
                height: seamTable.topMargin - 5
            }

            Rectangle {
                z: 3
                color: root.color
                anchors {
                    left: corner.right
                    top: corner.top
                }
                width: 5
                height: seamTable.topMargin - 5
            }

            Rectangle {
                z: 3
                color: root.color
                anchors {
                    left: corner.left
                    top: corner.bottom
                }
                width: seamTable.leftMargin - 5
                height: 5
            }

            Rectangle {
                z: 2
                color: "white"
                anchors {
                    left: corner.right
                    right: horizontalHeader.right
                    top: horizontalHeader.bottom
                }
                height: 5
            }

            Row {
                id: horizontalHeader
                y: seamTable.contentY
                z: 2
                Repeater {
                    model: seamTable.columns > 0 ? seamTable.columns : 1

                    Rectangle {
                        width: seamTable.columnWidthProvider(modelData)
                        height: fontMetrics.height + seamTable.cellMargin
                        color: seamTable.borderColor

                        Label {
                            anchors {
                                fill: parent
                                leftMargin: modelData == 0 ? 2 : 1
                                rightMargin: modelData == selectionModel.columnCount - 1 ? 0 : 1
                            }
                            text: selectionModel.headerData(modelData, Qt.Horizontal)
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            color: "white"

                            background: Rectangle {
                                color: root.color
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: selectionModel.selectColumn(index)
                            }
                        }
                    }
                }
            }

            Rectangle {
                z: 2
                color: "white"
                anchors {
                    top: verticalHeader.top
                    bottom: verticalHeader.bottom
                    left: verticalHeader.right
                }
                width: 5
            }

            Column {
                id: verticalHeader
                x: seamTable.contentX
                z: 2
                Repeater {
                    model: seamTable.rows > 0 ? seamTable.rows : 1

                    Rectangle {
                        width: seamTable.seriesLabelWidth + seamTable.cellMargin
                        height: seamTable.rowHeightProvider(modelData)
                        color: seamTable.borderColor

                        Label {
                            anchors {
                                fill: parent
                                topMargin: modelData == 0 ? 2 : 1
                                bottomMargin: modelData == selectionModel.rowCount - 1 ? 0 : 1
                            }
                            text: selectionModel.headerData(modelData, Qt.Vertical)
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter

                            background: Rectangle {
                                color: root.color
                            }

                            MouseArea {
                                anchors.fill: parent
                                onClicked: selectionModel.selectRow(index)
                            }
                        }
                    }
                }
            }

            Rectangle {
                z: 2
                color: "white"
                anchors {
                    top: corner.bottom
                    bottom: bottomBorder.bottom
                    left: parent.right
                }
                width: 5
            }

            Rectangle {
                id: bottomBorder
                z: 2
                color: "white"
                anchors {
                    left: corner.right
                    right: parent.right
                    top: parent.bottom
                }
                height: 5
            }
        }

        Button {
            Layout.fillWidth: true

            text: qsTr("Select All")
            onClicked: selectionModel.selectAll()
        }

        Button {
            Layout.fillWidth: true

            text: qsTr("Select None")
            onClicked: selectionModel.selectNone()
        }
    }
}
