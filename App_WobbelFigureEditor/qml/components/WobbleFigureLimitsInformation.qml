import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication

GroupBox {

  id: wobbleFigureLimitsInformation

  property var figureAnalyzer: null

  //:Title for a group box where the limits of wobble figure are shown.
  title: qsTr("Limits (Center position)")

  background: Rectangle {
      color: "white"
  }

  GridLayout {
      id: gridLayout

      rows: 4
      flow: GridLayout.TopToBottom

      Label {
          id: amplitudeTitle
          Layout.preferredWidth: wobbleFigureLimitsInformation.width * 0.3
          Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
          //:Header for wobble limit entries to show in this column the values are the current minimum amplitude.
          text: qsTr("Amplitude\n[mm]")
      }

      Repeater {
          id: amplitudeRepeater
          model: plausibilityChecker

          Label {
              id: amplitudeLabel
              Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
              text: Number(amplitude).toLocaleString(locale, 'f', 0)
              palette.windowText: "lightgrey"
          }
      }

      Label {
          id: maxFrequencyTitle
          Layout.preferredWidth: wobbleFigureLimitsInformation.width * 0.3
          Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
          //:Header for wobble limit entries to show in this column the values are the current maximum frequency.
          text: qsTr("Maximum\nFrequency\n[Hz]")
      }

      Repeater {
          id: frequencyRepeater
          model: plausibilityChecker

          Label {
              id: maxFrequency
              Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
              text: Number(frequency).toLocaleString(locale, 'f', 0)
              palette.windowText: "lightgrey";
          }
      }

      Label {
          id: figureConformityTitle
          Layout.preferredWidth: wobbleFigureLimitsInformation.width * 0.3
          Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
          //:Header for wobble limit entries to show in this column the values are the current minimum conformity.
          text: qsTr("Minimum\nConformity\n[%]")
      }

      Repeater {
          id: conformityRepeater
          model: plausibilityChecker

          Label {
              id: figureConformity
              Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
              text: Number(conformity * 100).toLocaleString(locale, 'f', 0)
              palette.windowText: "lightgrey";
          }
      }
  }

  PlausibilityChecker {
      id: plausibilityChecker

      height: figureAnalyzer.figureHeight
      width: figureAnalyzer.figureWidth

      onCurrentRowChanged: {
          for (var i = 0; i < plausibilityChecker.rowCount(); i++)
          {
              var amplitudeItem = amplitudeRepeater.itemAt(i);
              if (amplitudeItem)
              {
                  amplitudeItem.palette.windowText = "lightgrey";
              }
              var frequencyItem = frequencyRepeater.itemAt(i);
              if (frequencyItem)
              {
                  frequencyItem.palette.windowText = "lightgrey";
              }
              var conformityItem = conformityRepeater.itemAt(i);
              if (conformityItem)
              {
                  conformityItem.palette.windowText = "lightgrey";
              }
          }

          if (plausibilityChecker.currentRow === -1)
          {
              return;
          }

          var selectedAmplitudeItem = amplitudeRepeater.itemAt(plausibilityChecker.currentRow);
          if (selectedAmplitudeItem)
          {
              selectedAmplitudeItem.palette.windowText = "black"
          }
          var selectedFrequencyItem = frequencyRepeater.itemAt(plausibilityChecker.currentRow);
          if (selectedFrequencyItem)
          {
              selectedFrequencyItem.palette.windowText = "black"
          }
          var selectedConformityItem = conformityRepeater.itemAt(plausibilityChecker.currentRow);
          if (selectedConformityItem)
          {
              selectedConformityItem.palette.windowText = "black"
          }
      }
  }
}
