import QtQuick 2.15

ListView {
    id: listView
    property var dragParent: listView // used as parent during dragging

    // internal state
    property var dropIdx: null
    property var sourceIdx: null
    property var dropSource: null
    property int scrollingDirection: 0

    signal moveItem(int sourceIdx, int destinationIdx)

    // Animations implement automatic scroling when reaching the top/bottom of the ListView during dragging
    SmoothedAnimation {
        id: upAnimation
        target: listView
        property: "contentY"
        to: 0
        running: listView.scrollingDirection == -1
    }

    SmoothedAnimation {
        id: downAnimation
        target: listView
        property: "contentY"
        to: listView.contentHeight - listView.height
        running: listView.scrollingDirection == 1
    }
}
