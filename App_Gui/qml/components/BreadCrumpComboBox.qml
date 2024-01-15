import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.general 1.0
import precitec.gui.components.application 1.0

ComboBox {
    signal itemSelected(int index)
    id: root
    property bool measureTask: false
    property bool navigationEnabled: true
    enabled: root.model != undefined && root.navigationEnabled
    indicator: ToolButton {
        id: indicatorButton
        flat: true
        display: Button.IconOnly
        icon.name: "go-next"
        icon.color: Settings.iconColor
        background: Rectangle {
            implicitWidth: 40
            implicitHeight: 40

            opacity: indicatorButton.down ? 1.0 : 0.5
            visible: !indicatorButton.flat || indicatorButton.down || indicatorButton.checked || indicatorButton.highlighted
            color: indicatorButton.down || indicatorButton.checked || indicatorButton.highlighted ? indicatorButton.palette.mid : indicatorButton.palette.button
        }
        onClicked: root.popup.open()
    }
    delegate: ItemDelegate {
        function updateImplicitWidth()
        {
            if (delegate.implicitWidth > root.popup.width)
            {
                root.popup.width = delegate.implicitWidth
            }
        }
        id: delegate
        width: root.popup.width
        text: root.measureTask ? qsTr("%1 (%2)").arg(modelData.name).arg(modelData.visualNumber) : model.display
        icon.name: root.measureTask ? "" : model.icon
        onImplicitWidthChanged: updateImplicitWidth()
        Component.onCompleted: updateImplicitWidth()
        onClicked: root.itemSelected(index)
    }
    contentItem: Item {}
    background: Item {}
}
