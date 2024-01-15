import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0
import wobbleFigureEditor.components 1.0

Control {
    id: objectPropertiesItem

    property var attributeController: null
    property var wobbleFigureEditor: null
    property var figureAnalyzer: null
    property var simulationController: null
    property alias screenshotTool: laserPointProperties.screenshotTool
    implicitWidth: content.implicitWidth + 2 * padding
    implicitHeight: content.implicitHeight + 2 * padding
    focus: true
    padding: 10
    topInset: 0

    background: Rectangle {
        id: backrec
        Layout.fillHeight: true
        Layout.fillWidth: true
        color: "white"
        border {
            color: PrecitecApplication.Settings.alternateBackground
            width: 2
        }
    }

    contentItem: ColumnLayout {
        id: content
        spacing: 5

        FigureGlobalProperties {
            visible: FigureEditorSettings.digitalLaserPower
            id: figureGlobalProperties
            figureEditor: objectPropertiesItem.wobbleFigureEditor
        }

        Rectangle {
            visible: FigureEditorSettings.digitalLaserPower
            id: upperBorder
            Layout.preferredHeight: 1
            color: "darkgrey"
        }

        LaserPointProperties {
            id: laserPointProperties

            attributeController: objectPropertiesItem.attributeController
            figureEditor: objectPropertiesItem.wobbleFigureEditor
            figureAnalyzer: objectPropertiesItem.figureAnalyzer
            simulationController: objectPropertiesItem.simulationController
        }

        WobbleProperties {
            visible: wobbleFigureEditor.fileType == FileType.Wobble
            id: wobbleFigureProperties
            figureEditor: objectPropertiesItem.wobbleFigureEditor
            figureAnalyzer: objectPropertiesItem.figureAnalyzer
        }
        Item {
            id: wildcard
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    // ToDo: is this connection unused?
    Connections {
        target: objectPropertiesItem.attributeController

        function onUpdated(object) {
            //Update point on the cpp side
            objectPropertiesItem.wobbleFigureEditor.updateProperties(object)
        }
    }
}
