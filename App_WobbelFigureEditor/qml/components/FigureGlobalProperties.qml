import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

Control {
    id: figureGlobalProperties
    property var figureEditor: null
    height: figureGlobalPropertiesGrid.implicitHeight

    rightPadding: 10
    leftPadding: 10

    background: Rectangle {
        id: background
        border.color: "lightgrey"
        border.width: 1
        radius: 3
    }

    contentItem: ColumnLayout {
        id: figureGlobalPropertiesGrid

        Label {
            id: figureGlobalLabel
            Layout.fillWidth: true
            text: qsTr("Figure global properties")
            font.pixelSize: 18
            font.bold: true
        }

        CheckBox {
            id: powerType
            enabled: figureEditor && figureEditor.figure
            Layout.fillWidth: true
            text: qsTr("Show power as analog?")
            checked: figureEditor.figure ? figureEditor.figure.analogPower : false
            onToggled: {
                if (figureEditor && figureEditor.figure)
                {
                    figureEditor.figure.analogPower = powerType.checked;
                }
            }
        }
    }
}
