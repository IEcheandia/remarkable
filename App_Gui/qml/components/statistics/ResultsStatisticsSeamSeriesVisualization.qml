import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

GroupBox {
    id: root

    property alias controller: resultsStatisticsSeamSeriesModel.resultsStatisticsController

    property var errorConfigModel: null

    signal clicked(var seamSeriesId)

    ResultsStatisticsSeamSeriesModel {
        id: resultsStatisticsSeamSeriesModel
    }

    label: Label {
        text: qsTr("Seam Series Statistics")
        font.family: root.font.family
        font.pixelSize: root.font.pixelSize
        font.bold: true
        verticalAlignment: Text.AlignVCenter
        leftPadding: root.leftPadding
    }

    ColumnLayout {
        anchors.fill: parent

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: 1.5 * height

            spacing: 20
            clip: true
            model: resultsStatisticsSeamSeriesModel

            ScrollBar.vertical: ScrollBar {
                id: bar
            }

            delegate: ItemDelegate {

                width: ListView.view.width
                height: layout.implicitHeight

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
                        text: qsTr("SeamSeries \"%1\" (%2)").arg(model.name).arg(model.visualNumber)
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

                        visible: !errorModel.empty

                        model: ResultsStatisticsSeamsErrorModel {
                            id: errorModel

                            seamSeriesId: model.id
                            resultsStatisticsController: root.controller
                            errorSettingModel: root.errorConfigModel
                        }

                        title: qsTr("Error Statistics")
                    }
                }
            }
        }
    }
}
