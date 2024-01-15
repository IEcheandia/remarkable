import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import precitec.gui.components.logging 1.0
import "private" as Private

/**
 * Item for viewing log messages.
 *
 * The log messages are represented in a tabular view. The log messages can be filtered
 * through various components offered in the view.
 **/
Item {
    id: logView
    /**
     * Model containing the module names
     **/
    property alias moduleModel: logComponent.moduleModel

    property alias model: logFilterModel.sourceModel

    property alias paused: logComponent.paused

    property alias pauseAvailable: logComponent.pauseAvailable

    property real expandedHeight

    property alias viewDebugMessagesPermission: logFilterModel.viewDebugMessagesPermission

    property bool hasUnreadWarning: false

    Component.onCompleted: {
        hasUnreadWarning = logView.model && logView.model.latestWarningOrError != ""
    }

    Connections {
        target: logView.model
        function onLatestWarningOrErrorChanged() {
            if (state == "Invisible")
            {
                logView.hasUnreadWarning = true;
            }
        }
    }

    function swapStates()
    {
        if (state == "Invisible")
        {
            state = "Visible";
            logView.hasUnreadWarning = false;
        } else
        {
            state = "Invisible";
        }
    }

    state: "Invisible"
    states: [
        State {
            name: "Visible"
            PropertyChanges {
                target: logView
                height: logView.expandedHeight
                visible: true
            }
        },
        State {
            name: "Invisible"
            PropertyChanges {
                target: logView
                height: 0
                visible: false
            }
        }
    ]
    transitions: [
        Transition {
            from: "Visible"
            to: "Invisible"
            SequentialAnimation {
                PropertyAnimation {
                    target: logView
                    property: "height"
                }
                NumberAnimation {
                    target: logView
                    property: "visible"
                }
            }
        },
        Transition {
            from: "Invisible"
            to: "Visible"
            PropertyAnimation {
                target: logView
                property: "height"
            }
        }
    ]

    LogFilterModel {
        id: logFilterModel
        moduleNameFilter: logComponent.currentModuleText
    }
    Private.LogComponent {
        id: logComponent
        anchors.fill: parent
    }
}
