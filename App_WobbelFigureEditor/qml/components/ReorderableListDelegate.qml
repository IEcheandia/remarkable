import QtQuick 2.15
import QtQuick.Layouts 1.15

Item {
    id: listItem
    width: innerItem.width
    height: dragHandler.active ? 0 : innerItem.height
    property ListView listView//: parent.parent

    default property Item userItem

    onUserItemChanged: {
        userItem.parent = innerItem;
    }

    // This Item has an animated y that slides downward if the gap for insertion should be above it.
    // The userItem is temporarily parented to this item while the user drags something. This is necessary
    // because there is no way to cancel the animation when the user finally drops the element. So the workaround
    // is to temporarily parent the visible item to an inivisible item with the desired animation.
    Item {
        id: yAnimated

        states: [
            State {
                when: !dragHandler.Drag.active &&
                      listView.dropIdx !== null &&
                      listView.dropSource !== null &&
                      listView.dropIdx <= index
                name: "gap-above"
                PropertyChanges {
                    target: yAnimated
                    y: listView.dropSource.height
                }
            }
        ]

        transitions: [
            Transition {
                from: ""
                to: "gap-above"
                reversible: true
                PropertyAnimation {
                    target: yAnimated
                    property: "y"
                }
            }
        ]
    }

    Item {
        id: innerItem
        width: listView.width
        height: userItem.height

        Drag.active: dragHandler.active
        Drag.supportedActions: Qt.MoveAction
        Drag.hotSpot: Qt.point(width / 2, height / 2)

        DragHandler {
            id: dragHandler
            cursorShape: Qt.DragMoveCursor
            xAxis.enabled: false
            onActiveChanged: {
                if (!active)
                {
                    let dstIdx = listView.dropIdx
                    if (listView.sourceIdx < listView.dropIdx)
                    {
                        dstIdx -= 1
                    }

                    listView.moveItem(listView.sourceIdx, dstIdx)
                    listView.dropIdx = null
                    listView.dropSource = null
                }
                else
                {
                    listView.sourceIdx = index
                }
            }
        }

        states: [
            State {
                when: dragHandler.active
                name: "dragging"

                // NOTE: Resetting the parent also causes the item to return to its original position in the list
                ParentChange {
                    target: innerItem
                    parent: listView.dragParent
                }
                PropertyChanges {
                    target: innerItem
                    anchors.fill: undefined
                    opacity: 0.7
                }
            },
            State {
                when: listView.dropSource !== null && !dragHandler.Drag.active
                name: "dragging-other-item"
                ParentChange {
                    target: innerItem
                    parent: yAnimated
                }
            }
        ]
    }

    DropArea {
        id: dropArea
        enabled: !dragHandler.active
        anchors.fill: listItem

        function update() {
            let atop = drag.y < height / 2
            listView.dropIdx = atop ? index : index + 1
            let pos = dropArea.mapToItem(listView, Qt.point(drag.x, drag.y))
            let autoScrollHeight = 20
            if (pos.y < autoScrollHeight)
            {
                listView.scrollingDirection = -1
            }
            else if (pos.y >= listView.height - autoScrollHeight)
            {
                listView.scrollingDirection = 1
            }
            else
            {
                listView.scrollingDirection = 0
            }
        }

        onPositionChanged: {
            update()
        }

        onContainsDragChanged: {
            if (containsDrag)
            {
                listView.dropSource = drag.source
                update()
            }
        }
    }

    //            Rectangle {
    //                id: debugDropArea
    //                anchors.fill: dropArea
    //                border.color: "red"
    //                color: "transparent"
    //                opacity: 0.8
    //                radius: 3

    //                Text {
    //                    x: 3
    //                    y: 3
    //                    text: index
    //                    color: parent.border.color
    //                }
    //            }
}
