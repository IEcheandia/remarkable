import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import QtGraphicalEffects 1.12

import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0

GroupBox {
    id: root
    property alias category: categoryFilterModel.categoryName
    property alias subGraphModel: categoryFilterModel.sourceModel
    property alias showDisabled: categoryFilterModel.disabledFilter
    property alias searchText: categoryFilterModel.searchText
    property alias filterModel: categoryFilterModel

    SubGraphCategoryFilterModel {
        id: categoryFilterModel
    }

    ScrollView {
        anchors.fill: parent
        clip: true
        ListView {
            id: listView
            anchors.fill: parent
            model: categoryFilterModel
            delegate: ItemDelegate {
                id: item
                property var modelIndex: categoryFilterModel.mapToSource(categoryFilterModel.index(index, 0))
                checkable: true
                checked: model.checked
                enabled: model.enabled
                width: ListView.view.width
                height: 160

                function handleToggle()
                {
                    root.subGraphModel.check(item.modelIndex, !model.checked);
                }

                contentItem: CheckBox {
                    id: control
                    checked: item.checked
                    onToggled: item.handleToggle()

                    indicator: Rectangle {
                        implicitWidth: 150
                        implicitHeight: 150
                        border.width: 3
                        border.color: control.checkState === Qt.Checked ? PrecitecApplication.Settings.alternateBackground : "white"
                        Image {
                            id: img
                            source: model.image
                            fillMode: Image.PreserveAspectFit
                            anchors {
                                fill: parent
                                margins: 5
                            }

                            Image {
                                id: checkSign
                                anchors {
                                    right: parent.right
                                    bottom: parent.bottom
                                }
                                source: "qrc:/qt-project.org/imports/QtQuick/Controls.2/images/check.png"
                                smooth: true
                                visible: false
                            }

                            ColorOverlay {
                                anchors.fill: checkSign
                                source: checkSign
                                color: PrecitecApplication.Settings.alternateBackground
                                visible: control.checkState === Qt.Checked
                            }
                        }

                        Colorize {
                            anchors.fill: img
                            source: img
                            visible: !model.enabled
                            hue: 0.0
                            saturation: 0.0
                            lightness: 0.0
                        }
                    }
                    contentItem: ColumnLayout {
                        anchors.leftMargin: control.indicator.width + control.spacing
                        Label {
                            id: titleLabel
                            Layout.leftMargin: control.indicator.width + control.spacing
                            Layout.fillWidth: true
                            font.bold: true
                            text: model.name

                        }
                        Label {
                            Layout.leftMargin: control.indicator.width + control.spacing
                            Layout.fillHeight: true
                            Layout.preferredWidth: titleLabel.width
                            wrapMode: Text.WordWrap
                            maximumLineCount: 5
                            elide: Text.ElideRight
                            text: model.comment
                        }
                        RowLayout{
                            Layout.leftMargin: control.indicator.width + control.spacing
                            Layout.fillWidth: true
                            visible: control.checked
                            enabled: alternativesCombo.count > 0
                            ComboBox {
                                id: alternativesCombo
                                Layout.fillWidth: true
                                model: SubGraphAlternativesModel {
                                    subGraphModel: control.checked ? root.subGraphModel : null
                                    selectedIndex: item.modelIndex
                                }
                                textRole: "name"
                            }
                            Button {
                                icon.name: "select"
                                text: qsTr("Change to alternative graph")
                                enabled: alternativesCombo.currentIndex != -1
                                onClicked: root.subGraphModel.switchToAlternative(item.modelIndex, alternativesCombo.model.mapToSource(alternativesCombo.model.index(alternativesCombo.currentIndex, 0)))
                            }
                        }
                    }
                }
                onToggled: item.handleToggle()
            }
        }
    }
}
