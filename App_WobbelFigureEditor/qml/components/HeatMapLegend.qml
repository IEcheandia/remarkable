import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

Rectangle {
    id: heatMapLegend

    Rectangle {
        id: heatMapText
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        width: parent.width * 0.5
        color: "transparent"

        Label {
            anchors {
                top: parent.top
                right: parent.right
                rightMargin: 5
            }
            text: "%1 %".arg(Number(FigureEditorSettings.thresholdFromIndex(0) + 1).toLocaleString(locale, 'f', 0))
        }
        Label {
            anchors {
                verticalCenter: parent.verticalCenter
                right: parent.right
                rightMargin: 5
            }
            text: "%1 %".arg(Number(FigureEditorSettings.thresholdFromIndex(2)).toLocaleString(locale, 'f', 0))
        }
        Label {
            anchors {
                bottom: parent.bottom
                right: parent.right
                rightMargin: 5
            }
            text: "%1 %".arg(Number(FigureEditorSettings.thresholdFromIndex(4)).toLocaleString(locale, 'f', 0))
        }
    }

    Rectangle {
        id: heatMapColor
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
        border.width: 1
        border.color: "black"
        width: parent.width * 0.5
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: FigureEditorSettings.colorFromValue(0.0)
            }
            GradientStop {
                position: 0.25
                color: FigureEditorSettings.colorFromValue(25.0)
            }
            GradientStop {
                position: 0.5
                color: FigureEditorSettings.colorFromValue(50.0)
            }
            GradientStop {
                position: 0.75
                color: FigureEditorSettings.colorFromValue(75.0)
            }
            GradientStop {
                position: 1.0
                color: FigureEditorSettings.colorFromValue(100.0)
            }
        }
    }
}
