import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

/**
 * Row layout containing the Save to seam and save to all seams buttons.
 **/
RowLayout {
    id: buttonGroup

    /**
     * Emitted when the save to seam button is clicked
     **/
    signal saveToSeam()
    /**
     * Emitted when the save to all seams button is clicked
     **/
    signal saveToAllSeams()

    property bool changes: false
    ToolButton {
        display: AbstractButton.IconOnly
        enabled: buttonGroup.changes
        // TODO: implement
        visible: false
        icon {
            name: "document-save-all"
            width: 64
            height: 64
        }
        onClicked: buttonGroup.saveToAllSeams()

        ToolTip.text: qsTr("Save parameters to all seams.")
        ToolTip.visible: hovered
        ToolTip.delay: 200
        ToolTip.timeout: 5000
    }
    ToolButton {
        display: AbstractButton.IconOnly
        enabled: buttonGroup.changes
        icon {
            name: "document-save"
            width: 64
            height: 64
        }
        onClicked: buttonGroup.saveToSeam()

        ToolTip.text: qsTr("Save parameter changes for this seam.")
        ToolTip.visible: hovered
        ToolTip.delay: 200
        ToolTip.timeout: 5000
    }
}
