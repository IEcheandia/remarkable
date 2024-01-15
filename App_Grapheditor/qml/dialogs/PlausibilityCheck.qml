import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import grapheditor.components 1.0

Component {
    id: plausibilityCheck
    Dialog {
        id: plausibilityCheckDialog
        property var graphVisualizer: null
        property var plausibilityController: null
        property var navigable: null
        property bool graphValid: false
        property bool idsValid: false

        property var actualNode: null

        anchors.centerIn: parent
        width: parent.width * 0.5
        height: parent.height * 0.6
        modal: true
        standardButtons: Dialog.Close | Dialog.Ok
        header: Control {
            padding: 5
            background: Rectangle {
                color: PrecitecApplication.Settings.alternateBackground
                height: 40
            }
            contentItem: Label {
                Layout.fillWidth: true
                text: qsTr("Invalid nodes")
                font.pixelSize: 18
                font.bold: true
                color: PrecitecApplication.Settings.alternateText
                horizontalAlignment: Text.AlignHCenter
            }
        }

        onAccepted:
        {
            filterGraphView.visualizedGraph.clearSelection();
            if (actualNode === null)
            {
                return;
            }
            if (actualNode.group)
            {
                filterGraphView.qanGraphView.centerOn(actualNode.group.item);
                filterGraphView.visualizedGraph.selectNode(actualNode);
            }
            else
            {
                filterGraphView.qanGraphView.centerOn(actualNode.item);
                filterGraphView.visualizedGraph.selectNode(actualNode);
            }
        }
        onRejected:
        {
            destroy();
        }

        GroupBox {
            id: plausibilityCheckView
            implicitWidth: parent.width
            implicitHeight: parent.height
            ColumnLayout {
                id: plausibilityCheckInterface
                anchors.fill: parent

                Label {
                    id: isGraphValid
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    text: plausibilityCheckDialog.graphValid ? qsTr("Graph valid!") : qsTr("Graph invalid!")

                    font.bold: true

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    background: Rectangle {
                        color: plausibilityCheckDialog.graphValid ? "green" : "red"
                        radius: 3

                        border.width: 3
                        border.color: "grey"
                    }
                }

                ListView {
                    id: invalidNodesSearching
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ScrollBar.vertical: ScrollBar {}
                    model: invalidNodeModel
                    spacing: 5
                    clip: true
                    currentIndex: -1
                    delegate: ItemDelegate {
                        id: invalidNodeSelectionButton
                        width: ListView.view.width - 15
                        checkable: true
                        padding: 1
                        contentItem: RowLayout {
                            id: rowLayout
                            Label {
                                id: nodeLabels
                                Layout.preferredWidth: 0.3 * invalidNodeSelectionButton.width
                                Layout.leftMargin: 3
                                text: model.name
                                verticalAlignment: Text.AlignVCenter
                                font.bold: true
                            }
                            Rectangle {
                                id: leftBorder
                                width: 1
                                Layout.fillHeight: true
                                color: "darkgrey"
                            }
                            Label {
                                id: nodeType
                                Layout.preferredWidth: 0.25 * invalidNodeSelectionButton.width
                                text: model.nodeType
                                verticalAlignment: Text.AlignVCenter
                            }
                            Rectangle {
                                id: rightBorder
                                width: 1
                                Layout.fillHeight: true
                                color: "darkgrey"
                            }
                            Label {
                                id: type
                                Layout.preferredWidth: 0.25 * invalidNodeSelectionButton.width
                                text: model.type
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        background: Rectangle {
                            id: itemDelegateBackground
                            color: "transparent"
                            border.width: 1
                            border.color: "black"
                            radius: 3
                        }
                        onClicked:
                        {
                            invalidNodesSearching.currentIndex = index;
                            actualNode = model.pointer;
                        }
                    }
                    highlight: Rectangle { color: "lightsteelblue"; radius: 3 }
                }
                Rectangle {
                    id: border
                    visible: invalidNodesSearching.count != 0
                    height: 1
                    Layout.fillWidth: true
                    color: "darkgrey"
                }
                Label {
                    id: graphIDsunique
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    text: plausibilityCheckDialog.idsValid ? qsTr("IDs valid") : qsTr("IDs invalid")

                    font.bold: true

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    background: Rectangle {
                        color: plausibilityCheckDialog.idsValid ? "green" : "red"
                        radius: 3

                        border.width: 3
                        border.color: "grey"
                    }
                }

                ListView {
                    id: invalidIDsSearching
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    ScrollBar.vertical: ScrollBar {}
                    model: invalidIDModel
                    spacing: 5
                    clip: false
                    currentIndex: -1
                    delegate: ItemDelegate {
                        id: invalidIDSelectionButton
                        width: ListView.view.width - 15
                        checkable: true
                        padding: 1
                        contentItem: RowLayout
                        {
                            id: idRowLayout
                            Label {
                                id: idNodeType
                                Layout.preferredWidth: 0.4 * invalidIDSelectionButton.width
                                Layout.leftMargin: 3
                                Layout.rightMargin: 10
                                text: model.nodeType
                                verticalAlignment: Text.AlignVCenter
                                font.bold: true
                            }
                            Rectangle {
                                id: border
                                width: 1
                                Layout.fillHeight: true
                                color: "darkgrey"
                            }
                            Label {
                                id: iDs
                                Layout.preferredWidth: 0.3 * invalidIDSelectionButton.width
                                Layout.rightMargin: 3
                                text: model.id
                                verticalAlignment: Text.AlignVCenter
                            }
                        }
                        background: Rectangle {
                            color: "transparent"
                            border.width: 1
                            border.color: "black"
                            radius: 3
                        }
                    }
                    highlight: Rectangle {
                        color: "lightsteelblue"
                        radius: 3
                    }
                }
                Item {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                }
            }
        }

        InvalidNodeModel {
            id: invalidNodeModel
            plausibilityController: plausibilityCheckDialog.plausibilityController
        }
        InvalidIDModel {
            id: invalidIDModel
            plausibilityController: plausibilityCheckDialog.plausibilityController
        }
    }
}
