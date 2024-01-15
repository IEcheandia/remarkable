import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0 as WobbleFigureEditor
import precitec.gui.components.application 1.0 as PrecitecApplication

Dialog {
    required property PrecitecApplication.ScreenshotTool screenshotTool
    required property WobbleFigureEditor.WobbleFigureEditor figureEditor
    property string uiFilename: "???"
    property alias importController: importController

    id: dxfImportDialog

    parent: Overlay.overlay
    width: parent.width * 0.9
    height: parent.height * 0.9
    anchors.centerIn: parent
    modal: true

    WobbleFigureEditor.WobbleFigureView {
        id: wobbleFigureView
        figureEditor: dxfImportDialog.figureEditor
        anchors.fill: parent
        allowOverlappingPointsPopup: false
    }

    WobbleFigureEditor.LaserPointController {
        id: laserPointController
        figure: wobbleFigureView.actualFigure

        onFigureScaleChanged: {
            dxfImportDialog.figureEditor.figureScale = figureScale
            wobbleFigureView.view.grid.visible = false
            wobbleFigureView.view.grid.visible = true
        }
    }

    WobbleFigureEditor.DxfImportController {
        id: importController
        laserPointController: laserPointController
        figureEditor: dxfImportDialog.figureEditor

        onUpdatedImport: {
          wobbleFigureView.view.fitInView()
        }
    }

    DxfImportProperties {
        id: properties
        wobbleFigureView: wobbleFigureView
        Layout.fillHeight: true
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        z: 2
        screenshotTool: screenshotTool
        importController: importController
    }


    header: PrecitecApplication.DialogHeaderWithScreenshot {
        title: qsTr("Import ") + uiFilename
        screenshotTool: dxfImportDialog.screenshotTool
    }

    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        Button {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
        Button {
            text: qsTr("Import")
            enabled: !importController.errorMsg
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
    }

    onAccepted: {
      importController.finalizeImport()
      close()
    }
}
