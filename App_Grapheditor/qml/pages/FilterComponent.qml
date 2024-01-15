import QtQuick 2.12
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

Item
{
    id: itemFilterComponent
    property alias searchFieldText: searchField.searchFieldText
    property alias model: filterShowSearching.model
    property var filterGraphView: null
    property var onlineHelp: undefined

    GridLayout
    {
        id: layoutFilterComponent
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        columns: 2

        SearchGroupBox {
            id: searchField
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        ScrollView
        {
            id: scrollView
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: ScrollBar.AlwaysOn
            clip: true

            ListView
            {
                id: filterShowSearching
                Layout.fillWidth: true
                Layout.fillHeight: true
                model: filterSortModel
                spacing: 10
                currentIndex: -1
                delegate: ItemDelegate
                {
                    id: filterSelectionButton
                    width: ListView.view.width
                    checkable: true
                    padding: 1
                    contentItem: RowLayout {
                        GridLayout
                        {
                            id: delegate
                            columns: 2
                            rows: 2
                            width: parent.width
                            Drag.active: dragHandler.active
                            Drag.imageSource: filterImage.source
                            Drag.dragType: Drag.Automatic
                            Drag.supportedActions: Qt.CopyAction
                            Drag.hotSpot: Qt.point(-20, -20)
                            Image
                            {
                                id: filterImage
                                Layout.rowSpan: 2
                                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                                source: model.filterImagePath
                                sourceSize.width: 50
                                sourceSize.height: 50
                                onStatusChanged:
                                {
                                    if (filterImage.status != Image.Ready)
                                    {
                                        filterImage.source = "file://" + WeldmasterPaths.filterPictureDir + "dummyFilter.png"
                                    }
                                }
                            }
                            Label
                            {
                                id: labelName
                                Layout.fillWidth: true
                                text: model.filterName
                                font.bold: true
                                Layout.alignment: Qt.AlignVCenter
                            }
                            Label
                            {
                                id: labelType
                                Layout.alignment: Qt.AlignVCenter | Qt.AlignTop
                                Component.onCompleted:
                                {
                                    switch (model.filterType)
                                    {
                                        case 1:
                                            labelType.text = "FilterType: Source"
                                            break;
                                        case 2:
                                            labelType.text = "FilterType: Sink"
                                            break;
                                        case 3:
                                            labelType.text = "FilterType: Transform"
                                            break;
                                        case 4:
                                            labelType.text = "FilterType: Result"
                                            break;
                                        default:
                                            labelType.text = "FilterType: Not specified!"
                                    }
                                }
                            }
                            //Other place for the dragHandler
                        }
                        ToolButton {
                            Layout.rowSpan: 2
                            Layout.alignment: Qt.AlignBottom

                            id: helpButton
                            objectName: "grapheditor-filtercomponent-filter-list-view-help-" + index

                            icon.name: "help-hint"
                            visible: itemFilterComponent.onlineHelp != undefined && model.pdfAvailable
                            onClicked: {
                                itemFilterComponent.onlineHelp.alignment = Qt.AlignRight;
                                itemFilterComponent.onlineHelp.file = model.pdfFileName;
                                itemFilterComponent.onlineHelp.open();
                            }
                        }
                    }
                    DragHandler {
                        id: dragHandler
                        target: null
                        onActiveChanged: {
                            if (!active)
                            {
                                delegate.Drag.drop();
                            }
                            else
                            {
                                graphEditor.setFilterType(filterSortModel.mapToSource(filterSortModel.index(model.index, 0)));
                            }
                        }
                    }
                    onClicked: {
                        graphEditor.setFilterType(filterSortModel.mapToSource(filterSortModel.index(model.index, 0)));
                        graphEditor.insertNewFilterVisual(filterGraphView.getVisibleCenter());
                    }
                }

                section.property: "filterLibName"
                section.criteria: ViewSection.FullString
                section.delegate: sectionHeading
            }
        }

        Component
        {
            id:sectionHeading
            Label
            {
                text: section
                font.bold: true
                font.pixelSize: 18
            }
        }
    }

    Connections
    {
        target: graphEditor
        function onFilterTypeIndexChanged()
        {
            filterShowSearching.currentIndex = graphEditor.filterTypeIndex;
        }
    }
}

