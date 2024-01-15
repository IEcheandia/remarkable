import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0 as PrecitecApplication

/**
 * List with thumbnail images and functionality to jump to an image.
 * Expects context property simulationController.
 **/
ColumnLayout {
    id: view

    signal frameSelected(int index)

    property alias model: listView.model

    function down()
    {
        scrollBar.position = 1.0 - scrollBar.size;
    }
    function up()
    {
        scrollBar.position = 0.0;
    }

    ToolButton {
        Layout.fillWidth: true

        icon.name: "arrow-up-dobule"
        onClicked: view.up()
    }

    ListView {
        Layout.fillWidth: true
        Layout.fillHeight: true

        id: listView

        orientation: ListView.Vertical
        clip: true
        highlightMoveDuration: 0
        currentIndex: model.currentFrameIndex

        onCurrentIndexChanged: listView.positionViewAtIndex(listView.currentIndex, ListView.Contain)

        ScrollBar.vertical: ScrollBar {
            id: scrollBar
            policy: ScrollBar.AlwaysOn

            Behavior on position {
                NumberAnimation {
                    duration: 100
                }
            }
        }

        delegate: ItemDelegate {
            width: ListView.view.width - scrollBar.width
            height: ListView.view.width
            highlighted: ListView.isCurrentItem
            onClicked: {
                view.frameSelected(index);
            }
            palette.light: PrecitecApplication.Settings.alternateBackground

            contentItem: Item {
                anchors {
                    fill: parent
                    margins: 5
                }
                Image {
                    id: image
                    anchors.fill: parent
                    asynchronous: true
                    fillMode: Image.PreserveAspectFit
                    source: "file://" + model.path
                }
                Label {
                    anchors.centerIn: parent
                    text: qsTr("No image")
                    visible: image.status == Image.Error
                }
                BusyIndicator {
                    running: image.progress != 1.0 && image.status != Image.Error
                    anchors.centerIn: parent
                }
                Label {
                    anchors {
                        bottom: parent.bottom
                        right: parent.right
                        margins: 5
                    }
                    horizontalAlignment: Text.AlignRight
                    text: model.image
                    color: PrecitecApplication.Settings.text
                    background: Rectangle {
                        color: PrecitecApplication.Settings.background
                        opacity: 0.5
                    }
                }
            }
        }
    }

    ToolButton {
        Layout.fillWidth: true

        icon.name: "arrow-down-dobule"
        onClicked: view.down()
    }
}
