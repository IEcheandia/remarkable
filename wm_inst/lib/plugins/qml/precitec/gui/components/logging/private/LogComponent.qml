import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import "./" as Private
import precitec.gui.components.application 1.0

/**
 * Item for viewing log messages.
 *
 * The log messages are represented in a tabular view. The log messages can be filtered
 * through various components offered in the view.
 **/
Control {
    property string currentModuleText: selectionControl.currentModuleIndex == 0 ? "" : selectionControl.currentModuleText
    property alias moduleModel: selectionControl.moduleModel
    property alias paused: selectionControl.paused
    property alias pauseAvailable: selectionControl.pauseAvailable
    background: Rectangle {
        color: "#f0f0f0"
        border {
            width: 5
            color: "#f0f0f0"
        }
    }
    FontMetrics {
        id: fontMetrics
        font.family: "Monospace"
    }
    contentItem: ColumnLayout {
        Private.LogFilteringControl {
            id: selectionControl
            Layout.fillWidth: true
        }
        ListView {
            property int timeStampWidth: fontMetrics.advanceWidth("88:88:88:888")
            property int levelWidth: fontMetrics.advanceWidth("Warning")
            property int moduleWidth: fontMetrics.advanceWidth(selectionControl.moduleModel.longestItem)

            Layout.fillHeight: true
            Layout.fillWidth: true
            id: logListView
            clip: true
            model: logFilterModel
            ScrollBar.vertical: ScrollBar {
                id: scroll
            }
            delegate: Private.LogDelegate {
                timeStampWidth: logListView.timeStampWidth
                levelWidth: logListView.levelWidth
                moduleWidth: logListView.moduleWidth

                width: ListView.view.width - scroll.width
            }
            onCountChanged: {
                if (count == 0)
                {
                    return;
                }
                positionViewAtEnd();
            }
        }
    }
}
