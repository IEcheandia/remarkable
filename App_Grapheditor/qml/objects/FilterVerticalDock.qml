
import QtQuick 2.2
import QtQuick.Layouts 1.1

import QuickQanava 2.0 as Qan

ColumnLayout {
    id: root
    spacing: 3      //5
    z: 1.5   // Selection item z=1.0, dock must be on top of selection
    states: [
        State {
            name: "left"
            when: hostNodeItem && dockType === Qan.NodeItem.Left

            AnchorChanges {
                target: root
                anchors {
                    right: hostNodeItem.left
                    //verticalCenter: hostNodeItem.verticalCenter
                    bottom: hostNodeItem.bottom
                }
            }

            PropertyChanges {
                target: root
                rightMargin: root.rightMargin
            }
        },
        State {
            name: "right"
            when: hostNodeItem && dockType === Qan.NodeItem.Right

            AnchorChanges {
                target: root
                anchors {
                    left: hostNodeItem.right
                    //verticalCenter: hostNodeItem.verticalCenter
                    bottom: hostNodeItem.bottom
                }
            }

            PropertyChanges {
                target: root
                leftMargin: root.leftMargin
            }
        }
    ]

    property var hostNodeItem
    property int dockType: -1
    property int leftMargin: 7
    property int rightMargin: 7
}
