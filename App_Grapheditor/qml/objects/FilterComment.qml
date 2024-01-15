import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import QuickQanava          2.0 as Qan
import "qrc:/QuickQanava"   as Qan

Qan.NodeItem
{
    id: filterPortCommentItem
    Layout.preferredWidth: 100
    Layout.preferredHeight: 125
    width: Layout.preferredWidth
    height: Layout.preferredHeight

    Rectangle
    {
        anchors.fill: parent
        color: "yellow"//"#00000000"
        Label
        {
            id: filterComment
            anchors.fill: parent
            text: filterPortCommentItem.node.text
            font.pixelSize: 12
        }
    }
}
