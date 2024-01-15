import QtQuick 2.5
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

import wobbleFigureEditor.components 1.0 as WobbleFigureEditor

AttributeSeamFigure {
    id: control
    objectName: "attribute-wobble-figure"
    fileType: WobbleFigureEditor.FileType.Wobble
}
