import QtQuick 2.12
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

import grapheditor.components 1.0

import precitec.gui.general 1.0

Control {
    id: root
    property var macroController: null
    property alias macroGraphModel: macroFilterModel.sourceModel
    property alias count: macroListView.count
    property var onlineHelp: undefined

    // alignment with the padding from FilterComponent
    leftPadding: 10
    rightPadding: 10

    FilterGraphFilterModel {
        id: macroFilterModel
        searchText: macroSearchGroupBox.searchFieldText
    }

    GraphSortModel {
        id: graphSortModel
        sourceModel: macroFilterModel
    }

    contentItem: ColumnLayout {
        SearchGroupBox {
            id: macroSearchGroupBox
            Layout.fillWidth: true
        }
        ListView {
            id: macroListView
            model: graphSortModel
            clip: true
            // same as spacing in FilterComponent
            spacing: 10
            delegate: ItemDelegate {
                property var sortModelIndex: graphSortModel.mapToSource(graphSortModel.index(index, 0))
                property var modelIndex: macroFilterModel.mapToSource(sortModelIndex)
                // same as in FilterComponent
                padding: 1
                width: ListView.view.width
                hoverEnabled: true
                contentItem: RowLayout {
                    property var modelIndex: parent.modelIndex
                    objectName: "grapheditor-macro-list-index-" + index
                    width: parent.width
                    Drag.active: dragHandler.active
                    Drag.imageSource: filterImage.source
                    Drag.dragType: Drag.Automatic
                    Drag.supportedActions: Qt.CopyAction
                    Drag.hotSpot: Qt.point(-20, -20)
                    Image {
                        id: filterImage
                        source: model.image
                        sourceSize.width: 50
                        sourceSize.height: 50
                        onStatusChanged: {
                            if (filterImage.status != Image.Ready)
                            {
                                filterImage.source = "file://" + WeldmasterPaths.filterPictureDir + "dummyFilter.png"
                            }
                        }
                        Component.onCompleted: {
                            if (filterImage.source.toString() == "")
                            {
                                filterImage.source = "file://" + WeldmasterPaths.filterPictureDir + "dummyFilter.png"
                            }
                        }
                    }
                    Label {
                        Layout.fillWidth: true
                        text: model.name
                    }
                    ToolButton {
                        Layout.rowSpan: 2
                        Layout.alignment: Qt.AlignBottom

                        id: helpButton
                        objectName: "grapheditor-macrocomponent-macro-list-view-help-" + index

                        icon.name: "help-hint"
                        visible: root.onlineHelp != undefined && model.pdfAvailable
                        onClicked: {
                            root.onlineHelp.alignment = Qt.AlignRight;
                            root.onlineHelp.file = model.name;
                            root.onlineHelp.open();
                        }
                    }
                }
                DragHandler {
                    id: dragHandler
                    target: null
                }
                onClicked: {
                    macroController.insertMacro(modelIndex, filterGraphView.getVisibleCenter());
                }

                ToolTip.text: model.comment
                ToolTip.visible: hovered && model.comment != ""
            }

            section.property: "groupName"
            section.criteria: ViewSection.FullString
            section.delegate: Label {
                text: section
                font.bold: true
                font.pixelSize: 18
            }

            ScrollBar.vertical: ScrollBar { }

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
