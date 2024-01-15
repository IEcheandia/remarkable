import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

BreadCrumpGroupBox {
    id: root
    title: qsTr("Overview of Detection Parameters")
    productToNextLevelEnabled: false
    property var graphModel: null
    property var subGraphModel: null

    signal graphDeltaSelected(var seam)
    signal deltaSelected(var seam)

    ProductSeamModel {
        id: productSeamModel
        product: root.product
    }
    ListView {
        id: seamListView
        property real seamNameImplicitWidth: 0

        function updateSeamNameImplicitWidth(labelWidth)
        {
            seamListView.seamNameImplicitWidth = Math.max(seamListView.seamNameImplicitWidth, labelWidth);
        }

        anchors.fill: parent
        clip: true
        spacing: 5

        model: productSeamModel
        section.property: GuiConfiguration.seamSeriesOnProductStructure ? "seamSeriesName" : ""
        section.delegate: Label {
            bottomPadding: seamListView.spacing * 2
            topPadding: seamListView.spacing
            bottomInset: seamListView.spacing
            width: ListView.view.width - verticalScrollBar.width
            text: section
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            color: PrecitecApplication.Settings.alternateText
            background: Rectangle {
                color: PrecitecApplication.Settings.alternateBackground
            }
        }
        delegate: Control {
            width: ListView.view.width - verticalScrollBar.width
            implicitHeight: layout.implicitHeight + seamListView.spacing
            RowLayout {
                id: layout
                property var seam: model.seam
                anchors.fill: parent
                anchors.bottomMargin: seamListView.spacing
                Label {
                    id: seamLabel
                    text: model.seam.name + " (" + model.seam.visualNumber + "):"
                    font.bold: true
                    horizontalAlignment: Text.AlignRight
                    onImplicitWidthChanged: seamListView.updateSeamNameImplicitWidth(implicitWidth)
                    Layout.preferredWidth: seamListView.seamNameImplicitWidth
                }
                Label {
                    visible: model.linkedSeam
                    text: visible ? qsTr("Linked to seam %1 (%2)").arg(model.seam.linkTo.name).arg(model.seam.linkTo.visualNumber) : ""
                    Layout.fillWidth: true
                }
                Label {
                    visible: !model.seam.usesSubGraph
                    text: visible ? root.graphModel.data(root.graphModel.indexFor(model.seam.graph)) : ""
                    elide: Text.ElideMiddle
                    Layout.fillWidth: true
                }
                ScrollView {
                    visible: model.seam.usesSubGraph
                    clip: true
                    Layout.maximumWidth: layout.width - seamLabel.implicitWidth - compareWithGraphButton.implicitWidth - compareWithSeamsButton.implicitWidth - layout.spacing * 5
                    Layout.fillWidth: true
                    RowLayout {
                        Repeater {
                            id: subGraphRepeater
                            model: layout.seam.subGraphs
                            Label {
                                property bool isLast: index == subGraphRepeater.count - 1
                                text: root.subGraphModel.data(root.subGraphModel.indexFor(modelData)) + (isLast ? "" : ", ")
                                Layout.fillWidth: isLast
                            }
                        }
                    }
                }
                Button {
                    id: compareWithGraphButton
                    visible: !model.linkedSeam
                    text: qsTr("Compare with default")
                    onClicked: root.graphDeltaSelected(model.seam)
                }
                Button {
                    id: compareWithSeamsButton
                    visible: !model.linkedSeam
                    text: qsTr("Compare with seams")
                    onClicked: root.deltaSelected(model.seam)
                }
            }
            Rectangle {
                color: PrecitecApplication.Settings.alternateBackground
                height: 1
                width: parent.width
                anchors.bottom: parent.bottom
            }
        }

        ScrollBar.vertical: ScrollBar {
            id: verticalScrollBar
        }
    }
}
