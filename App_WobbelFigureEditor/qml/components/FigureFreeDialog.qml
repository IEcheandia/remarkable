import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

import wobbleFigureEditor.components 1.0

Dialog {
    required property FileModel fileModel
    required property WobbleFigureEditor fileEditor
    required property PrecitecApplication.ScreenshotTool thisScreenshotTool

    id: fileAndFigureItem

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        title: qsTr("Figure And Free")
        screenshotTool: thisScreenshotTool
    }

    Flickable {
      id: flickable
      width: parent.width
      height: parent.height
      clip: true
      flickableDirection: Flickable.VerticalFlick
      contentWidth: width - verticalScrollBar.width -5
      contentHeight: figureCreatorInterface.implicitHeight

      ScrollBar.vertical: ScrollBar {
        id: verticalScrollBar
        width: 15
        policy: flickable.height > flickable.contentHeight ? ScrollBar.AsNeeded : ScrollBar.AlwaysOn
      }

      FigureCreatorInterface
      {
          id: figureCreatorInterface
          fileEditor: fileAndFigureItem.fileEditor
          fileModel: fileAndFigureItem.fileModel
          width: parent.width
      }
    }
// workaround for https://bugreports.qt.io/browse/QTBUG-72372
    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        Button {
            text: qsTr("Create")
            enabled: fileAndFigureItem.fileEditor.figureCreator.fileType !== FileType.None
            onClicked:
            {
                fileAndFigureItem.fileEditor.figureCreator.createFigure();
                fileAndFigureItem.fileEditor.showFromFigureCreator();
            }
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
        Button {
            text: qsTr("Close")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

}

