import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

/**
 * A GroupBox with a back button in the title.
 * Emits a signal back when the back button got clicked.
 **/
GroupBox {
    id: root
    property string icon: ""
    /**
     * Emitted when the back button was clicked
     **/
    signal back()
    property alias backButton: backButton.visible
    label: RowLayout {
        width: root.availableWidth
        ItemDelegate {
            text: root.title
            icon.color: "transparent"
            icon.source: root.icon
            font.family: root.font.family
            font.pixelSize: root.font.pixelSize
            font.bold: true
            clip: true
            background: Item {}

            Layout.leftMargin: root.leftPadding
            Layout.fillWidth: true
        }
        ToolButton {
            id: backButton
            display: AbstractButton.IconOnly
            icon.name: "arrow-left"
            onClicked: {
                root.back();
            }
        }
    }
}
