import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

GridLayout
{
  id: attributesCircle
  required property WobbleFigureEditor fileEditor
  property bool isCircle: attributesCircle.fileEditor.figureCreator.figureShape === FigureCreator.Circle
  property bool isEight: attributesCircle.fileEditor.figureCreator.figureShape === FigureCreator.Eight

  anchors.margins: 10
  columns: 3

  Label
  {
      id: coordinateCenterLabel
      Layout.fillWidth: true
      Layout.columnSpan: 2
      horizontalAlignment: Text.AlignHCenter
      text: qsTr("Center X Y [mm]")
      font.bold: true
  }

  Label {}

  TextField
  {
      Layout.fillWidth: true
      id: xCoordinateCenter
      selectByMouse: true
      text: Number(attributesCircle.fileEditor.figureCreator.coordinateCenter.x).toLocaleString(locale, 'f', 2);
      palette.text: xCoordinateCenter.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: -10000
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesCircle.fileEditor.figureCreator.coordinateCenter = Qt.point(Number.fromLocaleString(locale, xCoordinateCenter.text),Number.fromLocaleString(locale, yCoordinateCenter.text));
      }
  }

  TextField
  {
      Layout.fillWidth: true
      id: yCoordinateCenter
      selectByMouse: true
      text: Number(attributesCircle.fileEditor.figureCreator.coordinateCenter.y).toLocaleString(locale, 'f', 2);
      palette.text: yCoordinateCenter.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: -10000
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesCircle.fileEditor.figureCreator.coordinateCenter = Qt.point(Number.fromLocaleString(locale, xCoordinateCenter.text),Number.fromLocaleString(locale, yCoordinateCenter.text));
      }
  }

  Label {}

  Label
  {
      Layout.fillWidth: true
      property bool show: isCircle
      text: qsTr("Radius [mm]")
      font.bold: true
  }

  Label
  {
      Layout.fillWidth: true
      property bool show: isCircle
      text: qsTr("Arc angle [°]")
      font.bold: true
  }

  Label {property bool show: isCircle}

  TextField
  {
      id: radius
      property bool show: isCircle
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

  TextField
  {
      id: arcAngle
      property bool show: isCircle
      selectByMouse: true
      Layout.fillWidth: true
      text: Number(attributesCircle.fileEditor.figureCreator.angle).toLocaleString(locale, 'f', 2);
      palette.text: arcAngle.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: -10000
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesCircle.fileEditor.figureCreator.angle = Number.fromLocaleString(locale, arcAngle.text);
      }
  }

  Label {property bool show: isCircle}

  Label
  {
      Layout.fillWidth: true
      property bool show: !isCircle
      text: qsTr("Width [mm]")
      font.bold: true
  }

  Label
  {
      Layout.fillWidth: true
      property bool show: !isCircle
      text: isEight ? qsTr("Height (2x Width) [mm]") : qsTr("Height [mm]")
      font.bold: true
  }

  Label {property bool show: !isCircle}

  TextField
  {
      id: figureWidth
      property bool show: !isCircle
      selectByMouse: true
      Layout.fillWidth: true
      text: Number(attributesCircle.fileEditor.figureCreator.figureWidth).toLocaleString(locale, 'f', 2);
      palette.text: figureWidth.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: 0
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesCircle.fileEditor.figureCreator.figureWidth = Number.fromLocaleString(locale, figureWidth.text);
      }
  }

  TextField
  {
      id: figureHeight
      property bool show: !isCircle
      selectByMouse: true
      Layout.fillWidth: true
      text: Number(attributesCircle.fileEditor.figureCreator.figureHeight).toLocaleString(locale, 'f', 2);
      palette.text: figureHeight.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: 0
          top: 10000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesCircle.fileEditor.figureCreator.figureHeight = Number.fromLocaleString(locale, figureHeight.text);
      }
  }

  Label {property bool show: !isCircle}
}
