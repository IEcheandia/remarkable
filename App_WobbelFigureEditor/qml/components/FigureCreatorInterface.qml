import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

ColumnLayout {

    spacing: 3
    id: figureCreatorModule
    required property WobbleFigureEditor fileEditor
    required property FileModel fileModel

    Item {
        id: attribs

    AttributesDefault {
        id: attributesDefault
        fileEditor: figureCreatorModule.fileEditor
    }

    AttributesLine {
        visible: figureCreatorModule.fileEditor.figureCreator.figureShape === FigureCreator.Line
        id: attributesLine
        fileEditor: figureCreatorModule.fileEditor
    }

    AttributesCircle {
        visible: [FigureCreator.Circle, FigureCreator.Eight, FigureCreator.Ellipse].includes(figureCreatorModule.fileEditor.figureCreator.figureShape)
        id: attributesCircle
        fileEditor: figureCreatorModule.fileEditor
    }

    AttributesSpiral {
        visible: figureCreatorModule.fileEditor.figureCreator.figureShape === FigureCreator.ArchSpiral
        id: attributesSpiral
        fileEditor: figureCreatorModule.fileEditor
    }

    Item
    {
        Button {
            Layout.fillWidth: true
            Layout.columnSpan: 2
            text: qsTr("Start at last point")
            enabled: fileEditor.numberOfPoints
            onClicked: {
                figureCreatorModule.fileEditor.figureCreator.startAtPoint(fileEditor.getLastPosition())
            }
        }

        Label {}
    }

    Interference {
        id: interference
        fileEditor: figureCreatorModule.fileEditor
        fileModel: figureCreatorModule.fileModel
    }

    AttributesFrequence {
        visible: figureCreatorModule.fileEditor.figureCreator.interference === 0// || figureCreatorModul.fileEditor.figureCreator.figureShape === 11
        id: attributesFrequence
        fileEditor: figureCreatorModule.fileEditor
        isSin: figureCreatorModule.fileEditor.figureCreator.interference === 0
    }
    }

    GridLayout {
        id: grid
        columns: 3
    }

    Item {
        id: wildcard
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    Component.onCompleted: {
        for (const i in attribs.children)
        {
            let a = attribs.children[i]
            while (a.children.length)
            {
                let c = a.children[0]
                c.visible = Qt.binding(function(){return a.visible && c.show !== false})
                c.parent = grid
            }
        }
    }
}
