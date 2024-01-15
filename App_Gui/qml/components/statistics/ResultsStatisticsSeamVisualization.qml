import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

GroupBox {
    id: root

    property alias controller: resultsStatisticsSeamModel.resultsStatisticsController

    property var errorConfigModel: null

    property var seam: null

    signal clicked(var seamId)

    ResultsStatisticsSeamModel {
        id: resultsStatisticsSeamModel
        seamId: root.seam ? root.seam.uuid : ""
        seamSeriesId: root.seam ? root.seam.seamSeries.uuid : ""
    }

    label: Label {
        text: qsTr("Seam \"%1\" (%2) Statistics in Seam Series \"%3\" (%4)")
                .arg(root.seam ? root.seam.name : "")
                .arg(root.seam ? root.seam.visualNumber : -1)
                .arg(root.seam ? root.seam.seamSeries.name : "")
                .arg(root.seam ? root.seam.seamSeries.visualNumber : -1)
        font.family: root.font.family
        font.pixelSize: root.font.pixelSize
        font.bold: true
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.leftPadding
    }

    ColumnLayout {
        anchors.fill: parent

        TabBar {
            Layout.fillWidth: true

            id: tabBar

            currentIndex: 0
            visible: controller && controller.currentProduct && controller.currentProduct.assemblyImage != ""

            TabButton {
                text: qsTr("Chart")
            }
            TabButton {
                text: qsTr("Assembly")
            }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: stack

            currentIndex: tabBar.currentIndex

            ListView {

                spacing: 20
                clip: true
                model: resultsStatisticsSeamModel

                ScrollBar.vertical: ScrollBar {
                    id: bar
                }

                header: RowLayout {
                    width: ListView.view.width

                    PieChart {
                        Layout.fillWidth: true

                        slices: controller && controller.empty ? [] : [
                            { name: ("Io"), value: resultsStatisticsSeamModel.seamIo, percentValue: resultsStatisticsSeamModel.seamIoInPercent, color: "green"},
                            { name: ("Nio"), value: resultsStatisticsSeamModel.seamNio, percentValue: resultsStatisticsSeamModel.seamNioInPercent, color: "red"}
                        ]

                        title: qsTr("Result Statistics")
                    }

                    PieChart {
                        Layout.fillWidth: true

                        visible: resultsStatisticsSeamModel.seamIncludesLinkedSeams

                        slices: controller && controller.empty ? [] : [
                            { name: ("Io"), value: resultsStatisticsSeamModel.seamIoIncludingLinkedSeams, percentValue: resultsStatisticsSeamModel.seamIoInPercentIncludingLinkedSeams, color: "green"},
                            { name: ("Nio"), value: resultsStatisticsSeamModel.seamNioIncludingLinkedSeams, percentValue: resultsStatisticsSeamModel.seamNioInPercentIncludingLinkedSeams, color: "red"}
                        ]

                        title: qsTr("Result Statistics including Linked Seams")
                    }

                    PieChartView {
                        Layout.fillWidth: true

                        visible: !errorModel.empty

                        model: ResultsStatisticsSeamErrorModel {
                            id: errorModel

                            seamId: root.seam ? root.seam.uuid : ""
                            seamSeriesId: root.seam ? root.seam.seamSeries.uuid : ""
                            resultsStatisticsController: root.controller
                            errorSettingModel: root.errorConfigModel
                        }

                        title: qsTr("Error Statistics")
                    }

                    PieChartView {
                        Layout.fillWidth: true

                        visible: resultsStatisticsSeamModel.seamIncludesLinkedSeams && !linkedErrorModel.empty

                        model: ResultsStatisticsSeamErrorModel {
                            id: linkedErrorModel

                            includeLinkedSeams: true
                            seamId: root.seam ? root.seam.uuid : ""
                            seamSeriesId: root.seam ? root.seam.seamSeries.uuid : ""
                            resultsStatisticsController: root.controller
                            errorSettingModel: root.errorConfigModel
                        }

                        title: qsTr("Error Statistics including Linked Seams")
                    }
                }

                delegate: ItemDelegate {

                    width: ListView.view.width
                    height: layout.implicitHeight - bar.width

                    onClicked: {
                        root.clicked(model.id)
                    }

                    GridLayout {
                        anchors {
                            fill: parent
                            rightMargin: bar.width
                        }

                        id: layout

                        columns: 2

                        Label {
                            Layout.fillWidth: true
                            Layout.columnSpan: 2

                            font.bold: true
                            text: qsTr("Linked seam \"%1\" (%2)").arg(model.name).arg(model.visualNumber)
                        }

                        PieChart {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            slices: controller && controller.empty ? [] : [
                                { name: ("Io"), value: model.io, percentValue: model.ioInPercent, color: "green"},
                                { name: ("Nio"), value: model.nio, percentValue: model.nioInPercent, color: "red"}
                            ]

                            title: qsTr("Result Statistics")
                        }

                        PieChartView {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            visible: !linkedSeamErrorModel.empty

                            model: ResultsStatisticsSeamErrorModel {
                                id: linkedSeamErrorModel

                                linkedSeamId: model.id
                                seamId: root.seam ? root.seam.uuid : ""
                                seamSeriesId: root.seam ? root.seam.seamSeries.uuid : ""
                                resultsStatisticsController: root.controller
                                errorSettingModel: root.errorConfigModel
                            }

                            title: qsTr("Error Statistics")
                        }

                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            visible: linkedSeamErrorModel.empty
                        }
                    }
                }
            }

            Item {
                SeamAssemblyImage {
                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                        horizontalCenter: parent.horizontalCenter
                    }

                    width: 0.5 * parent.width
                    assemblyImagesDirectory: WeldmasterPaths.assemblyImagesDir
                    editable: false
                    currentSeam: root.seam
                    assemblyImage: root.controller && root.controller.currentProduct ? root.controller.currentProduct.assemblyImage : ""
                }
            }
        }
    }
}
