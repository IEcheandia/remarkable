import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

GroupBox {
    id: sizeGroupBox
    property var selectedNode: null
    title: qsTr("Size")

    signal sizeChanged(size size)

    RowLayout {
        width: parent.width

        PrecitecApplication.ArrowButtonGrid {
            leftEnabled: selectedNode ? selectedNode.item.width > widthField.validator.bottom : false
            rightEnabled: selectedNode ? selectedNode.item.width < widthField.validator.top: false
            upEnabled: selectedNode ? selectedNode.item.height > heightField.validator.bottom : false
            downEnabled: selectedNode ? selectedNode.item.height < heightField.validator.top : false

            onMoveLeft: sizeGroupBox.sizeChanged(Qt.size(selectedNode.item.width - 1, selectedNode.item.height))
            onMoveDown: sizeGroupBox.sizeChanged(Qt.size(selectedNode.item.width, selectedNode.item.height + 1))
            onMoveUp: sizeGroupBox.sizeChanged(Qt.size(selectedNode.item.width, selectedNode.item.height - 1))
            onMoveRight: sizeGroupBox.sizeChanged(Qt.size(selectedNode.item.width + 1, selectedNode.item.height))
        }
        ColumnLayout {
            TextField {
                id: widthField
                text: sizeGroupBox.selectedNode ? sizeGroupBox.selectedNode.item.width.toLocaleString(locale, 'f', 0) : "0"
                validator: IntValidator {
                    bottom: 1
                    top: 1000
                }
                onEditingFinished: sizeGroupBox.sizeChanged(Qt.size(Number.fromLocaleString(widthField.text), Number.fromLocaleString(heightField.text)))
            }
            TextField {
                id: heightField
                text: sizeGroupBox.selectedNode ? sizeGroupBox.selectedNode.item.height.toLocaleString(locale, 'f', 0) : "0"
                validator: IntValidator {
                    bottom: 1
                    top: 1000
                }
                onEditingFinished: sizeGroupBox.sizeChanged(Qt.size(Number.fromLocaleString(widthField.text), Number.fromLocaleString(heightField.text)))
            }
        }
    }
}
