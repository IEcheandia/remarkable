import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

GridLayout
{
  id: attributesLine
  required property WobbleFigureEditor fileEditor
  anchors.margins: 10
  columns: 3

  Label {
      id: coordinateStartLabel
      Layout.fillWidth: true
      horizontalAlignment: Text.AlignHCenter
      Layout.columnSpan: 2
      text: qsTr("Start point X Y [mm]")
      font.bold: true
  }

  Label {}

  TextField {
      id: xCoordinateStart
      Layout.fillWidth: true
      selectByMouse: true
      text: Number(figureCreator.coordinateStart.x).toLocaleString(locale, 'f', 2)
      palette.text: xCoordinateStart.acceptableInput ? "black" : "red"
      validator: DoubleValidator {
          bottom: -10000
          top: 10000
          decimals: 2
      }
      onEditingFinished: {
          figureCreator.coordinateStart = Qt.point(
                      Number.fromLocaleString(locale, xCoordinateStart.text),
                      Number.fromLocaleString(locale, yCoordinateStart.text))
      }
  }

  TextField {
      id: yCoordinateStart
      Layout.fillWidth: true
      selectByMouse: true
      text: Number(figureCreator.coordinateStart.y).toLocaleString(locale, 'f', 2)
      palette.text: yCoordinateStart.acceptableInput ? "black" : "red"
      validator: DoubleValidator {
          bottom: -10000
          top: 10000
          decimals: 2
      }
      onEditingFinished: {
          figureCreator.coordinateStart = Qt.point(
                      Number.fromLocaleString(locale, xCoordinateStart.text),
                      Number.fromLocaleString(locale, yCoordinateStart.text))
      }
  }

  Label {}

  Label
  {
      id: coordinateEndLabel
      Layout.fillWidth: true
      Layout.columnSpan: 2
      horizontalAlignment: Text.AlignHCenter
      text: qsTr("End point X Y [mm]")
      font.bold: true
  }

  Label {}

  TextField
  {
      Layout.fillWidth: true
      id: xCoordinateEnd
      selectByMouse: true
      text: Number(attributesLine.fileEditor.figureCreator.coordinateEnd.x).toLocaleString(locale, 'f', 2);
      palette.text: xCoordinateEnd.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: -10000
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesLine.fileEditor.figureCreator.coordinateEnd = Qt.point(Number.fromLocaleString(locale, xCoordinateEnd.text),Number.fromLocaleString(locale, yCoordinateEnd.text));
      }
  }

  TextField
  {
      Layout.fillWidth: true
      id: yCoordinateEnd
      selectByMouse: true
      text: Number(attributesLine.fileEditor.figureCreator.coordinateEnd.y).toLocaleString(locale, 'f', 2);
      palette.text: yCoordinateEnd.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: -10000
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesLine.fileEditor.figureCreator.coordinateEnd = Qt.point(Number.fromLocaleString(locale, xCoordinateEnd.text),Number.fromLocaleString(locale, yCoordinateEnd.text));
      }
  }

  Label {}

  Label
  {
      Layout.fillWidth: true
      text: qsTr("Length [mm]")
      font.bold: true
  }

  Label {}
  Label {}

  TextField
  {
      Layout.fillWidth: true
      id: length
      selectByMouse: true
      text: Number(attributesLine.fileEditor.figureCreator.length).toLocaleString(locale, 'f', 2);
      palette.text: length.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: 0
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesLine.fileEditor.figureCreator.length = Number.fromLocaleString(locale, length.text);
      }
  }

  Label {}
  Label {}
}


