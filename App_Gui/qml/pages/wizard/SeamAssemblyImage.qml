import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Item {
    id: page
    property alias currentSeam: model.seam
    property string assemblyImage: currentSeam ? currentSeam.seamSeries.product.assemblyImage : ""
    property string assemblyImagesDirectory: ""
    property bool editable: false

    signal markAsChanged()

    SeamsOnAssemblyImageModel {
        id: model
        onMarkAsChanged: page.markAsChanged()
    }

    SeamsOnAssemblyImageFilterModel {
        id: filterModel
        sourceModel: model
    }

    ColumnLayout {
        anchors.fill: parent
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ScrollView {
                anchors.fill: parent
                clip: true
                visible: page.assemblyImage != ""

                ScrollBar.horizontal.policy: contentWidth > width ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
                ScrollBar.vertical.policy: contentHeight > height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

                Image {
                    source: page.assemblyImage == "" ? "" : "file://" + WeldmasterPaths.assemblyImagesDir + "/" + page.assemblyImage

                    Repeater {
                        anchors.fill: parent
                        model: filterModel
                        delegate: Rectangle {
                            x: model.position.x - width * 0.5
                            y: model.position.y - height * 0.5
                            width: 10
                            height: width
                            color: model.current ? "red" : (model.linkedToCurrent ? "orange" : "gray")
                            radius: width * 0.5

                            MouseArea {
                                id: mouseArea
                                enabled: page.editable
                                anchors.fill: parent
                                hoverEnabled: true
                            }

                            ToolTip.text: model.seamName
                            ToolTip.visible: mouseArea.containsMouse
                            ToolTip.delay: 200
                            ToolTip.timeout: 5000
                        }
                    }

                    MouseArea {
                        anchors.fill: parent
                        enabled: page.editable
                        cursorShape: Qt.CrossCursor
                        onClicked: {
                            model.setSeamPosition(mouse.x, mouse.y);
                        }
                    }
                }
            }
            Label {
                anchors.centerIn: parent
                text: qsTr("Please select an assembly image in Product Structure")
                visible: page.assemblyImage == ""
            }
        }
    }
}
