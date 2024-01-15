import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

GroupBox {
    id: positionGroupBox
    property var selectedNode: null
    title: qsTr("Position")

    signal positionChanged(point point)

    RowLayout {
        width: parent.width

        PrecitecApplication.ArrowButtonGrid {
            leftEnabled: selectedNode ? selectedNode.item.x > xPosition.validator.bottom : false
            rightEnabled: selectedNode ? selectedNode.item.x < xPosition.validator.top: false
            upEnabled: selectedNode ? selectedNode.item.y > yPosition.validator.bottom : false
            downEnabled: selectedNode ? selectedNode.item.y < yPosition.validator.top : false

            onMoveLeft: positionGroupBox.positionChanged(Qt.point(selectedNode.item.x - 1, selectedNode.item.y))
            onMoveDown: positionGroupBox.positionChanged(Qt.point(selectedNode.item.x, selectedNode.item.y + 1))
            onMoveUp: positionGroupBox.positionChanged(Qt.point(selectedNode.item.x, selectedNode.item.y - 1))
            onMoveRight: positionGroupBox.positionChanged(Qt.point(selectedNode.item.x + 1, selectedNode.item.y))
        }
        ColumnLayout {
            TextField {
                id: xPosition
                text: positionGroupBox.selectedNode ? positionGroupBox.selectedNode.item.x.toLocaleString(locale, 'f', 0) : "0"
                validator: IntValidator {
                    bottom: -10000
                    top: 10000
                }
                onEditingFinished: positionGroupBox.positionChanged(Qt.point(Number.fromLocaleString(xPosition.text), Number.fromLocaleString(yPosition.text)))
            }
            TextField {
                id: yPosition
                text: positionGroupBox.selectedNode ? positionGroupBox.selectedNode.item.y.toLocaleString(locale, 'f', 0) : "0"
                validator: IntValidator {
                    bottom: -10000
                    top: 10000
                }
                onEditingFinished: positionGroupBox.positionChanged(Qt.point(Number.fromLocaleString(xPosition.text), Number.fromLocaleString(yPosition.text)))
            }
        }
    }
}
