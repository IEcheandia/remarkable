import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0

/**
 * A visualization for the axis.
 *
 * Shows the current position and the soft limits if enabled.
 *
 * Requires a context property yAxisInformation.
 **/
Control {
    padding: 5
    contentItem: Item {
        id: visualization
        property real distance: Math.abs(yAxisInformation.maximumPosition - yAxisInformation.minimumPosition) / 1000.0
        property real offsetFactor: axisStatusPreview.width / distance
        Rectangle {
            id: axisStatusPreview
            anchors.centerIn: parent
            width: parent.width - 2 * labels.spacing
            height: 20
            color: "white"
        }
        Rectangle {
            width: 3
            height: axisStatusPreview.height
            x: axisStatusPreview.x + (yAxisInformation.lowerLimit - yAxisInformation.minimumPosition) / 1000.0 * visualization.offsetFactor
            y: axisStatusPreview.y
            visible: yAxisInformation.softwareLimitsEnabled && (yAxisInformation.lowerLimit >= yAxisInformation.minimumPosition && yAxisInformation.lowerLimit <= yAxisInformation.maximumPosition)
            color: "red"
        }
        Rectangle {
            width: 3
            height: axisStatusPreview.height
            x: axisStatusPreview.x + (yAxisInformation.upperLimit - yAxisInformation.minimumPosition) / 1000.0 * visualization.offsetFactor
            y: axisStatusPreview.y
            visible: yAxisInformation.softwareLimitsEnabled && (yAxisInformation.upperLimit >= yAxisInformation.minimumPosition && yAxisInformation.upperLimit <= yAxisInformation.maximumPosition)
            color: "red"
        }
        Rectangle {
            width: 3
            height: axisStatusPreview.height
            x: axisStatusPreview.x + (yAxisInformation.positionUserUnit - yAxisInformation.minimumPosition) / 1000.0 * visualization.offsetFactor
            y: axisStatusPreview.y
            visible: yAxisInformation.positionUserUnit >= yAxisInformation.minimumPosition && yAxisInformation.positionUserUnit <= yAxisInformation.maximumPosition
            color: "black"
        }
        RowLayout {
            id: labels
            width: parent.width
            anchors.top: axisStatusPreview.bottom
            Label {
                text: Number(yAxisInformation.minimumPosition / 1000.0).toLocaleString(locale, 'f', 1)
                color: Settings.alternateText
            }
            Label {
                text: Number(yAxisInformation.positionUserUnit / 1000.0).toLocaleString(locale, 'f', 3) + " mm"
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                color: Settings.alternateText
            }
            Label {
                text: Number(yAxisInformation.maximumPosition / 1000.0).toLocaleString(locale, 'f', 1)
                Layout.alignment: Qt.AlignRight
                color: Settings.alternateText
            }
        }
    }
}
