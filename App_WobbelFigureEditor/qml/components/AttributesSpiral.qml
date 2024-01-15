import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

GridLayout {
  id: attributesSpiral
  required property WobbleFigureEditor fileEditor
  anchors.margins: 10
  columns: 3

  Label {
      id: coordinateCenterLabel
      Layout.fillWidth: true
      Layout.columnSpan: 2
      horizontalAlignment: Text.AlignHCenter
      text: qsTr("Center X Y [mm]")
      font.bold: true
  }

  Label {}

  TextField {
      Layout.fillWidth: true
      id: xCoordinateCenter
      text:  Number(attributesSpiral.fileEditor.figureCreator.coordinateCenter.x).toLocaleString(locale, 'f', 2);
      selectByMouse: true
      palette.text: xCoordinateCenter.acceptableInput ? "black" : "red"
      validator: DoubleValidator {
          bottom: -5000
          top: 5000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesSpiral.fileEditor.figureCreator.coordinateCenter = Qt.point(Number.fromLocaleString(locale, xCoordinateCenter.text),Number.fromLocaleString(locale, yCoordinateCenter.text));
      }
  }

  TextField {
      Layout.fillWidth: true
      id: yCoordinateCenter
      text: Number(attributesSpiral.fileEditor.figureCreator.coordinateCenter.y).toLocaleString(locale, 'f', 2);
      selectByMouse: true
      palette.text: yCoordinateCenter.acceptableInput ? "black" : "red"
      validator: DoubleValidator {
          bottom: -5000
          top: 5000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesSpiral.fileEditor.figureCreator.coordinateCenter = Qt.point(Number.fromLocaleString(locale, xCoordinateCenter.text),Number.fromLocaleString(locale, yCoordinateCenter.text));
      }
  }

  Label {}

  Label
  {
      Layout.fillWidth: true
      text: qsTr("Half Width [mm]")
      font.bold: true
  }

  Label {
      id: windingsLabel
      Layout.fillWidth: true
      text: qsTr("Windings")
      font.bold: true
  }

  Label {}

  TextField
  {
      id: halfWidth
      selectByMouse: true
      Layout.fillWidth: true
      text: Number(attributesCircle.fileEditor.figureCreator.figureWidth / 2).toLocaleString(locale, 'f', 2);
      palette.text: halfWidth.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: 0
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesCircle.fileEditor.figureCreator.figureWidth = Number.fromLocaleString(locale, halfWidth.text) * 2;
      }
  }

  TextField {
      id: windings
      Layout.fillWidth: true
      text: Number(attributesSpiral.fileEditor.figureCreator.windings).toLocaleString(locale, 'f', 2);
      selectByMouse: true
      palette.text: windings.acceptableInput ? "black" : "red"
      validator: DoubleValidator {
          bottom: 0
          top: 100
          decimals: 2
      }
      onEditingFinished:
      {
          attributesSpiral.fileEditor.figureCreator.windings = Number.fromLocaleString(locale, windings.text);
      }
  }

  Label {}

  Label
  {
      Layout.fillWidth: true
      text: qsTr("Radius [mm]")
      font.bold: true
  }

  Label {}
  Label {}

  TextField
  {
      id: radius
      selectByMouse: true
      Layout.fillWidth: true
      text: Number(attributesCircle.fileEditor.figureCreator.radius).toLocaleString(locale, 'f', 2);
      palette.text: radius.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: 0
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesCircle.fileEditor.figureCreator.radius = Number.fromLocaleString(locale, radius.text);
      }
  }

  Label {}
  Label {}
}
