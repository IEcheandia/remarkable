import QtQuick 2.0
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0
import precitec.gui 1.0
import precitec.gui.components.userManagement 1.0

GroupBox {
    id: root

    property alias controller: resultsStatisticsSeamsModel.resultsStatisticsController

    property var errorConfigModel: null

    property var seamSeries: null

    signal clicked(var seamId)

    ResultsStatisticsSeamsModel {
        seamSeriesId: root.seamSeries ? root.seamSeries.uuid : ""
        id: resultsStatisticsSeamsModel
    }

    label: Label {
        text: qsTr("Seams Statistics of Seam Series \"%1\" (%2)").arg(root.seamSeries ? root.seamSeries.name : "").arg(root.seamSeries ? root.seamSeries.visualNumber : -1)
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

            spacing: 20
            clip: true
            model: resultsStatisticsSeamsModel

            ScrollBar.vertical: ScrollBar {
                id: bar
            }

            delegate: ItemDelegate {
                property var seam: controller && controller.currentProduct ? controller.currentProduct.findSeam(model.id) : null

                id: delegateItem

                width: ListView.view.width
                height: layout.implicitHeight

                onClicked: {
                    root.clicked(model.id)
                }

                GridLayout {
                    property bool includesLinkedSeams: model.includesLinkedSeams

                    anchors {
                        fill: parent
                        rightMargin: bar.width
                    }

                    id: layout

                    columns: 4

                    Label {
                        Layout.fillWidth: true
                        Layout.columnSpan: 4

                        font.bold: true
                        text: qsTr("Seam \"%1\" (%2)").arg(model.name).arg(model.visualNumber)
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

                    PieChart {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        visible: layout.includesLinkedSeams

                        slices: controller && controller.empty ? [] : [
                            { name: ("Io"), value: model.ioIncludingLinkedSeams, percentValue: model.ioInPercentIncludingLinkedSeams, color: "green"},
                            { name: ("Nio"), value: model.nioIncludingLinkedSeams, percentValue: model.nioInPercentIncludingLinkedSeams, color: "red"}
                        ]

                        title: qsTr("Result Statistics including Linked Seams")
                    }

                    PieChartView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        visible: !errorModel.empty

                        model: ResultsStatisticsSeamErrorModel {
                            id: errorModel

                            seamId: delegateItem.seam ? delegateItem.seam.uuid : ""
                            seamSeriesId: delegateItem.seam ? delegateItem.seam.seamSeries.uuid : ""
                            resultsStatisticsController: root.controller
                            errorSettingModel: root.errorConfigModel
                        }

                        title: qsTr("Error Statistics")
                    }

                    PieChartView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        visible: layout.includesLinkedSeams && !linkedErrorModel.empty

                        model: ResultsStatisticsSeamErrorModel {
                            id: linkedErrorModel

                            seamId: delegateItem.seam ? delegateItem.seam.uuid : ""
                            seamSeriesId: delegateItem.seam ? delegateItem.seam.seamSeries.uuid : ""
                            resultsStatisticsController: root.controller
                            errorSettingModel: root.errorConfigModel
                            includeLinkedSeams: true
                        }

                        title: qsTr("Error Statistics including Linked Seams")
                    }
                }
            }
        }
    }
}
