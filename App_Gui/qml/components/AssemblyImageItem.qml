import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Item {
    id: page
    property string assemblyImagesDirectory: ""
    property string assemblyImage: ""
    property alias model: filterModel.sourceModel

    signal seamClicked(var seam)

    SeamsOnAssemblyImageFilterModel {
        id: filterModel
    }

    ScrollView {
        anchors.fill: parent
        clip: true

        ScrollBar.horizontal.policy: contentWidth > width ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
        ScrollBar.vertical.policy: contentHeight > height ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

        Image {
            source: page.assemblyImage == "" ? "" : "file://" + page.assemblyImagesDirectory + "/" + page.assemblyImage

            Repeater {
                anchors.fill: parent
                model: filterModel
                delegate: Rectangle {
                    function getColor(current, state)
                    {
                        if (current)
                        {
                            return "orange";
                        }
                        if (state == AssemblyImageInspectionModel.Success)
                        {
                            return "green";
                        }
                        else if (state == AssemblyImageInspectionModel.Failure)
                        {
                            return "red";
                        }
                        else
                        {
                            return "gray";
                        }
                    }
                    x: model.position.x - width * 0.5
                    y: model.position.y - height * 0.5
                    width: 10
                    height: width
                    color: getColor(model.current, model.state)
                    radius: width * 0.5

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: page.seamClicked(model.seam)
                    }

                    ToolTip.text: model.seamName
                    ToolTip.visible: mouseArea.containsMouse
                    ToolTip.delay: 200
                    ToolTip.timeout: 5000
                }
            }
        }
    }
}
