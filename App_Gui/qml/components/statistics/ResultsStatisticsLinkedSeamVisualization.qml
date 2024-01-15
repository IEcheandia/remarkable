import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

GroupBox {
    id: root

    property alias controller: resultsStatisticsSeamModel.resultsStatisticsController

    property var errorConfigModel: null

    property var linkedSeam: null

    ResultsStatisticsSeamModel {
        id: resultsStatisticsSeamModel
        linkedSeamId: root.linkedSeam ? root.linkedSeam.uuid : ""
        seamId: root.linkedSeam ? root.linkedSeam.linkTo.uuid : ""
        seamSeriesId: root.linkedSeam ? root.linkedSeam.seamSeries.uuid : ""
    }

    label: Label {
        text: qsTr("Linked Seam \"%1\" (%2) Statistics to Seam \"%3\" (%4)")
                .arg(root.linkedSeam ? root.linkedSeam.name : "")
                .arg(root.linkedSeam ? root.linkedSeam.visualNumber : -1)
                .arg(root.linkedSeam ? root.linkedSeam.linkTo.name : "")
                .arg(root.linkedSeam ? root.linkedSeam.linkTo.visualNumber : -1)
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

            Item {
                RowLayout {
                    anchors {
                        top: parent.top
                        left: parent.left
                        right: parent.right
                    }

                    PieChart {
                        Layout.fillWidth: true

                        slices: controller && controller.empty ? [] : [
                            { name: ("Io"), value: resultsStatisticsSeamModel.seamIo, percentValue: resultsStatisticsSeamModel.seamIoInPercent, color: "green"},
                            { name: ("Nio"), value: resultsStatisticsSeamModel.seamNio, percentValue: resultsStatisticsSeamModel.seamNioInPercent, color: "red"}
                        ]

                        title: qsTr("Result Statistics")
                    }

                    PieChartView {
                        Layout.fillWidth: true

                        visible: !errorModel.empty

                        model: ResultsStatisticsSeamErrorModel {
                            id: errorModel

                            linkedSeamId: root.linkedSeam ? root.linkedSeam.uuid : ""
                            seamId: root.linkedSeam ? root.linkedSeam.linkTo.uuid : ""
                            seamSeriesId: root.linkedSeam ? root.linkedSeam.seamSeries.uuid : ""
                            resultsStatisticsController: root.controller
                        }

                        title: qsTr("Error Statistics")
                    }

                    Item {
                        Layout.fillWidth: true

                        visible: errorModel.empty
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
                    currentSeam: root.linkedSeam
                    assemblyImage: root.controller && root.controller.currentProduct ? root.controller.currentProduct.assemblyImage : ""
                }
            }
        }
    }
}
