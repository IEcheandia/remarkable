import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui 1.0

import wobbleFigureEditor.components 1.0

PrecitecApplication.SideTabView {
    id: figureEditorPage
    property var screenshot: null

    anchors.fill: parent

    ProductFilterModel {
        id: productFilterModel
        sourceModel: HardwareModule.productModel
    }

    MainView {
      id: mainView

      anchors.fill: parent

      screenshotTool: figureEditorPage.screenshot
      productModel: productFilterModel
    }
}
