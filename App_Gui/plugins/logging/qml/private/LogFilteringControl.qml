import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

/**
 * Row with controls to restrict the filtering of the log view.
 * Expects a property logFilterModel and moduleModel in the context.
 **/
RowLayout {
    property alias currentModuleIndex: moduleSelector.currentIndex
    property alias currentModuleText: moduleSelector.currentText
    property alias moduleModel: moduleSelector.model
    property alias paused: pause.checked
    property alias pauseAvailable: pause.visible
    TextMetrics {
        id: moduleMetrics
        text: " " + moduleModel.longestItem + " "
        font: moduleLabel.font
    }
    CheckBox {
        objectName: "debugCheckBox"
        text: qsTr("Debug")
        checked: logFilterModel.includeDebug
        visible: logFilterModel.canIncludeDebug
        onClicked: {
            logFilterModel.includeDebug = !logFilterModel.includeDebug
        }
    }
    CheckBox {
        objectName: "statusCheckBox"
        text: qsTr("Status")
        checked: logFilterModel.includeInfo
        onClicked: {
            logFilterModel.includeInfo = !logFilterModel.includeInfo
        }
    }
    CheckBox {
        objectName: "warningCheckBox"
        text: qsTr("Warning")
        checked: logFilterModel.includeWarning
        onClicked: {
            logFilterModel.includeWarning = !logFilterModel.includeWarning
        }
    }
    CheckBox {
        objectName: "errorCheckBox"
        text: qsTr("Error")
        checked: logFilterModel.includeError
        onClicked: {
            logFilterModel.includeError = !logFilterModel.includeError
        }
    }
    Label {
        id: moduleLabel
        text: qsTr("Module:")
    }
    ComboBox {
        objectName: "moduleSelector"
        id: moduleSelector
        textRole: "display"
        Layout.fillWidth: true
    }
    Button {
        text: qsTr("Clear")
        display: AbstractButton.TextBesideIcon
        visible: logFilterModel.sourceModel.canClear
        icon.name: "edit-clear-history"
        Layout.alignment: Qt.AlignRight
        onClicked: logFilterModel.sourceModel.clear()
    }
    ToolButton {
        id: pause
        icon.name: pause.checked ? "media-playback-start" : "media-playback-pause"
        checkable: true
    }
}
