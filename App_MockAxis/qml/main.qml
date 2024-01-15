import QtQuick 2.7
import QtQuick.Window 2.2
import precitec.gui.components.application 1.0 as PrecitecApplication
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import "qrc:/resources/qml/"

import precitec.gui.components.notifications 1.0 as Notifications

import Precitec.MockAxis 1.0

ApplicationWindow {
    id: applicationWindow
    width: 1024
    height: 800

    Module {
        id: module
    }

    header: PrecitecApplication.TopBar {
        id: topBar
        model: ListModel {
            ListElement {
                text: qsTr("Axis")
                iconSource: "qrc:/icons/tool"
                permission: -1
                enabled: true
            }
        }
    }

    Container {
        id: container
        currentIndex: topBar.tabBar.currentIndex
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        contentItem: Item {
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (ApplicationWindow.activeFocusControl) {
                        ApplicationWindow.activeFocusControl.focus = false;
                    }
                }
            }
            MultiPointTouchArea {
                mouseEnabled: false
                anchors.fill: parent
                onPressed: {
                    if (ApplicationWindow.activeFocusControl) {
                        ApplicationWindow.activeFocusControl.focus = false;
                    }
                }
            }
            ListView {
                anchors.fill: parent
                model: container.contentModel
                currentIndex: container.currentIndex
                interactive: false
                snapMode: ListView.SnapOneItem
                orientation: ListView.Horizontal
                boundsBehavior: Flickable.StopAtBounds

                highlightRangeMode: ListView.StrictlyEnforceRange
                preferredHighlightBegin: 0
                preferredHighlightEnd: 0
                highlightMoveDuration: 250
            }
        }

        ColumnLayout {
            RowLayout {
                Label {
                    text: "Mode of operation"
                }
                ComboBox {
                    model: [
                        "Pending",
                        "Offline",
                        "Position",
                        "Position_Relative",
                        "Position_Absolute",
                        "Velocity",
                        "Home"
                    ]
                    onActivated: module.sendOperationMode(index)
                }
            }
            RowLayout {
                Label {
                    text: "Position in um:"
                }
                TextField {
                    validator: IntValidator { bottom: -50000; top: 50000 }
                    onAccepted: module.sendPosition(Number.fromLocaleString(locale, text))
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: !container.visible
    }

    ScrollView {
        // parent to Overlay so that Notifications are shown above the Overlay screen
        parent: Overlay.overlay
        spacing: notificationsView.spacing
        anchors {
            bottom: parent.bottom
            horizontalCenter: parent.horizontalCenter
            bottomMargin: spacing + ApplicationWindow.footer.height
            topMargin: spacing
        }
        width: ApplicationWindow.contentItem.width * 0.5
        height: Math.min(contentHeight, ApplicationWindow.contentItem.height - 2 * spacing)
        z: Overlay.overlay.z + 1

        Notifications.NotificationsView {
            id: notificationsView
            height: ApplicationWindow.contentItem.height
        }
    }
    footer: PrecitecApplication.StatusBar {
        id: statusBar
        Item {
            Layout.fillWidth: true
        }
        ToolSeparator {}
        PrecitecApplication.StatusBarClock {
            Layout.alignment: Qt.AlignVCenter
        }
    }

    Component.onCompleted: {
        applicationWindow.showNormal();
    }
}
