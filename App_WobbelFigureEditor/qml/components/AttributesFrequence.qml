import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0


GridLayout
{
  id: attributesFrequence
  property var fileEditor: null
  property bool isSin: false
  anchors.margins: 10
  columns: 3

  Label
  {
      id: amplitudeLabel
      Layout.fillWidth: true
      text: qsTr("Amplitude [mm]")
      font.bold: true
  }

  Label
  {
      id: phaseShiftLabel
      Layout.fillWidth: true
      text: qsTr("Phase shift [°]")
      font.bold: true
  }

  Label {}

  TextField
  {
      id: amplitude
      Layout.fillWidth: true
      text: Number(attributesFrequence.fileEditor.figureCreator.amplitude).toLocaleString(locale, 'f', 2)
      palette.text: amplitude.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: 0
          top: 1000
          decimals: 2
      }
      onEditingFinished:
      {
          attributesFrequence.fileEditor.figureCreator.amplitude = Number.fromLocaleString(locale, amplitude.text);
      }
  }

  TextField
  {
      id: phaseShift
      Layout.fillWidth: true
      text: Number(attributesFrequence.fileEditor.figureCreator.phaseShift).toLocaleString(locale, 'f', 0)
      palette.text: phaseShift.acceptableInput ? "black" : "red"
      validator: IntValidator
      {
          bottom: 0
          top: 360
      }
      onEditingFinished:
      {
          attributesFrequence.fileEditor.figureCreator.phaseShift = Number.fromLocaleString(locale, phaseShift.text);
      }
  }

  Label {}

  Label
  {
      id: frequenceLabel
      Layout.fillWidth: true
      Layout.columnSpan: 2
      text: attributesFrequence.isSin ? qsTr("Repeat") : qsTr("Frequence (Horizontal) [Hz]")
      font.bold: true
  }

  Label {}

  TextField
  {
      id: frequence
      Layout.fillWidth: true
      Layout.columnSpan: 2
      text: Number(attributesFrequence.fileEditor.figureCreator.frequence).toLocaleString(locale, 'f', 2)
      palette.text: frequence.acceptableInput ? "black" : "red"
      validator: DoubleValidator
      {
          bottom: 0
          top: 1000
      }
      onEditingFinished:
      {
          attributesFrequence.fileEditor.figureCreator.frequence = Number.fromLocaleString(locale, frequence.text);
      }
  }

  Label {}

  Label
  {
      property bool show: attributesFrequence.fileEditor.figureCreator.figureShape === 10
      id: frequenceVerLabel
      Layout.fillWidth: true
      Layout.columnSpan: 2
      text: qsTr("Frequence (Vertical) [Hz]")
      font.bold: true
  }

  Label { property bool show: frequenceVerLabel.show}

  TextField
  {
      property bool show: frequenceVerLabel.show
      id: frequenceVer
      Layout.fillWidth: true
      Layout.columnSpan: 2
      text: Number(attributesFrequence.fileEditor.figureCreator.frequenceVer).toLocaleString(locale, 'f', 0)
      palette.text: frequenceVer.acceptableInput ? "black" : "red"
      validator: IntValidator
      {
          bottom: 0
          top: 1000
      }
      onEditingFinished:
      {
          attributesFrequence.fileEditor.figureCreator.frequenceVer = Number.fromLocaleString(locale, frequenceVer.text);
      }
  }

  Label { property bool show: frequenceVerLabel.show}
}
