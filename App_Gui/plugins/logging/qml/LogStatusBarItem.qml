import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

RowLayout {
    id: root
    property var model: null
    property var view: null
    Label {
        color: PrecitecApplication.Settings.alternateText
        text: (root.model != null && root.model.latestWarningOrError != "") ? root.model.latestWarningOrError : qsTr("No warnings")
        maximumLineCount: 1
        elide: Text.ElideRight
        Layout.fillWidth: true
    }
    ToolButton {
        objectName: "logview-tool-button"
        checkable: true
        enabled: root.view != null
        checked: root.view != null && root.view.visible
        display: AbstractButton.IconOnly
        icon {
            name: checked ? "arrow-down" : "arrow-up"
            color: "transparent"
        }
        onClicked: root.view.swapStates()
        ToolTip.visible: hovered
        ToolTip.text: checked ? qsTr("Hide log view") : qsTr("Show log view")
    }
}
