import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

GroupBox {
    property var controller: null

    property alias errorConfigModel: statisticsErrorModel.errorSettingModel

    id: root

    signal clicked()

    label: Label {
        text: qsTr("Product Statistics")
        font.family: root.font.family
        font.pixelSize: root.font.pixelSize
        font.bold: true
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.leftPadding
    }

    onVisibleChanged: {
        if (root.visible)
        {
            tabBar.currentIndex = 0;
        }
    }

    ResultsStatisticsSeamSeriesErrorModel {
        id: statisticsErrorModel
        resultsStatisticsController: root.controller
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
            Layout.alignment: Qt.AlignHCenter
            Layout.maximumHeight: root.height * 0.5

            currentIndex: tabBar.currentIndex

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: root.height * 0.5

                RowLayout {
                    anchors.fill: parent

                    PieChart {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        slices: controller && controller.empty ? [] : [
                            { name: ("Io"), value: controller ? controller.io : 0, percentValue: controller ? controller.ioInPercent : 0.0, color: "green"},
                            { name: ("Nio"), value: controller ? controller.nio : 0, percentValue: controller ? controller.nioInPercent : 0.0, color: "red"}
                        ]

                        title: qsTr("Result Statistics")
                    }

                    PieChartView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        model: statisticsErrorModel

                        title: qsTr("Error Statistics")
                    }
                }

                MouseArea {
                    anchors.fill: parent
                    enabled: controller && !controller.empty
                    onClicked: {
                        root.clicked()
                    }
                }
            }

            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.preferredHeight: root.height * 0.5

                Image {
                    anchors {
                        top: parent.top
                        bottom: parent.bottom
                        horizontalCenter: parent.horizontalCenter
                    }

                    source: root.controller && root.controller.currentProduct && root.controller.currentProduct.assemblyImage != "" ? ("file://" + WeldmasterPaths.assemblyImagesDir + "/" + root.controller.currentProduct.assemblyImage) : ""
                    visible: root.controller && root.controller.currentProduct && root.controller.currentProduct.assemblyImage != ""

                    asynchronous: true
                    fillMode: Image.PreserveAspectFit
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            RowLayout {
                Label {
                    text: qsTr("From:")
                    font.family: root.font.family
                    font.pixelSize: root.font.pixelSize
                    font.bold: true
                }
                DateSelector {
                    id: startDateSelector
                }
            }

            Item {
                Layout.preferredWidth: 200
            }

            RowLayout {
                Label {
                    text: qsTr("To:")
                    font.family: root.font.family
                    font.pixelSize: root.font.pixelSize
                    font.bold: true
                }
                DateSelector {
                    id: endDateSelector
                }
            }
        }

        Button {
            text: qsTr("Calculate statistics")
            enabled: !controller.calculating
            Layout.alignment: Qt.AlignHCenter
            onClicked: {
                controller.calculate(startDateSelector.date, endDateSelector.date)
            }
        }
    }
}
