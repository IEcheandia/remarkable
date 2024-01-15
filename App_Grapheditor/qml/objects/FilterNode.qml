import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import QuickQanava          2.0 as Qan
import "qrc:/QuickQanava"   as Qan

Qan.NodeItem
{
    id: filterNodeItem
    Layout.preferredWidth: 100
    Layout.preferredHeight: 80
    width: Layout.preferredWidth
    height: Layout.preferredHeight

    Rectangle
    {
        z: 0
        anchors.fill: parent
        color: "#00000000"
        opacity: (!pipeController.connecting || filterNodeItem.node.matchingInPipeConnection) ? 1.0 : 0.1

        Rectangle
        {
            id: filterContent
            anchors.fill: parent
            z: 1
            color: "lightgrey"
            border.color: "black"
            border.width: 1
            radius: 2

            ColumnLayout
            {
                z: 2
                anchors.fill: parent
                spacing: 0
                Rectangle
                {
                    id: imageContent
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    z: 3
                    color: "#00000000"

                    Image
                    {
                        id: filterImage
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        sourceSize.width: imageContent.width - 3
                        sourceSize.height: imageContent.height - 3
                        source: filterNodeItem.node.image
                    }
                }
                Label
                {
                    id: filterNameLabel
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    z: 3
                    text: filterNodeItem.node.label
                    font.pixelSize: 9
                    color: "black"
                }
            }
        }
    }
}
