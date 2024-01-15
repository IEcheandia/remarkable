import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication

Dialog {
  id: figureProperties
  property var figureEditor: null
  property var figureAnalyzer: null
  property var screenshotTool: null

  modal: true
  standardButtons: Dialog.Close | Dialog.Ok

  header: PrecitecApplication.DialogHeaderWithScreenshot {
    id: screenshotHeader
    title: qsTr("Change figure properties")
    screenshotTool: figureProperties.screenshotTool
  }

  onAccepted: {
    if (!figureEditor) {
      return;
    }
    figureEditor.setFigureProperties(figureName.text, Number.fromLocaleString(locale, figureID.text), figureDescription.text)
  }

  onRejected: {
    close();
  }

  ColumnLayout {
    id: figurePropertiesView
    anchors.fill: parent

    Label {
      id: figureNameLabel
      Layout.fillWidth: true
      text: qsTr("Name")
      font.pixelSize: 18
      font.bold: true
    }
    TextField {
      id: figureName
      Layout.fillWidth: true
      text: figureEditor && figureEditor.figure ? figureEditor.figure.name : ""
      selectByMouse: true
    }
    GroupBox {
      //:Title for a group box where the scanner property is shown.
      title: qsTr("Scanner properties")
      Layout.fillWidth: true

      ColumnLayout {
        anchors.fill: parent

        Label {
          id: scannerSpeedLabel
          Layout.fillWidth: true
          Layout.columnSpan: 2
          //:Title for a label where the current simulation speed of the figure editor is shown.
          text: qsTr("Simulation speed [mm/s]")
        }

        TextField {
          id: scannerSpeed
          Layout.fillWidth: true
          Layout.columnSpan: 2
          text: Number(FigureEditorSettings.scannerSpeed).toLocaleString(
                  locale, 'f', 3)
          palette.text: scannerSpeed.acceptableInput ? "black" : "red"
          validator: DoubleValidator {
            bottom: 0.001
            top: 10000.0
          }
          onEditingFinished: {
            FigureEditorSettings.scannerSpeed = Number.fromLocaleString(
                  locale, scannerSpeed.text)
            figureProperties.figureAnalyzer.updateProperties()
          }
          selectByMouse: true
        }
      }
    }
    RowLayout {
      Layout.fillWidth: true
      Label {
        id: fileNameLabel
        text: qsTr("File name:")
        font.pixelSize: 18
        font.bold: true
      }
      TextField {
        id: fileName
        Layout.fillWidth: true
        text: figureEditor
              && figureEditor.fileModel ? figureEditor.fileModel.filename : ""
        readOnly: true
        background: Item {}
      }
      Label {
        id: figureIDLabel
        text: qsTr("ID:")
        font.pixelSize: 18
        font.bold: true
      }
      TextField {
        id: figureID
        Layout.fillWidth: true
        text: figureEditor && figureEditor.figure ? figureEditor.figure.ID : ""
        selectByMouse: true
        readOnly: true
        background: Item {}
      }
    }
    Label {
      id: figureDescriptionLabel
      Layout.fillWidth: true
      text: qsTr("Description")
      font.pixelSize: 18
      font.bold: true
    }

    TextArea {
      id: figureDescription
      Layout.fillWidth: true
      Layout.fillHeight: true
      text: figureEditor
            && figureEditor.figure ? figureEditor.figure.description : ""
      selectByMouse: true
      wrapMode: TextEdit.WordWrap
      clip: true

      background: Rectangle {
        border.color: "lightgrey"
        border.width: 1
      }
    }
  }
}
