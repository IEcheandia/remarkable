import QtQuick 2.15
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0
import wobbleFigureEditor.components 1.0
import precitec.gui.components.notifications 1.0 as Notifications
import QtQuick.Window 2.12

// This started as a copy of Properties.qml
Control {
    required property WobbleFigureView wobbleFigureView
    required property DxfImportController importController
    required property PrecitecApplication.ScreenshotTool screenshotTool

    property bool hidden: false

    id: dxfImportPropertiesItem

    implicitWidth: contentLayout.width + 2 * padding
    implicitHeight: contentLayout.height + 2 * padding
    padding: 10
    focus: true

    function fitView() {
      var s = 1000 / importController.size
      laserPointController.figureScale = s
      importController.renderSegments() // NOTE: This is necessary because LaserPointController::drawSeamFigure calls LaserPointController::setLaserPointInformation which uses the scale to create the visual elements!
      var c = importController.center
      var p = Qt.point(c.x * s, c.y * -s)
      wobbleFigureView.view.zoom = 1
      wobbleFigureView.view.centerOnPosition(p)
    }

    Component.onCompleted: {
      if (importController.errorMsg)
        Notifications.NotificationSystem.error(importController.errorMsg);
    }

    Connections {
      target: importController
      function onErrorMsgChanged()
      {
        if (importController.errorMsg)
          Notifications.NotificationSystem.error(importController.errorMsg);
        else
          Notifications.NotificationSystem.withdrawAll()
      }

      function onUpdatedImport()
      {
        segmentList.currentIndex = -1
        fitView()
      }

      function onMissingUnit()
      {
          bar.currentIndex = 0
          unitCombo.forceActiveFocus()
          unitHilighterAnimation.restart()
      }
    }

    background: Rectangle {
        id: backrec
        Layout.fillHeight: true
        Layout.fillWidth: true
        color: "white"
        border {
            color: PrecitecApplication.Settings.alternateBackground
            width: 2
        }
    }

    contentItem: Item {

        ColumnLayout {
            id: contentLayout
            anchors.fill: parent
            width: 320 // TODO: This is a quick and dirty solution. Need to figure out a sensible way to compute the layout!

            RowLayout
            {
              ToolButton {
                  id: zoomOut
                  objectName: "figureEditor-buttonLayout-zoom-out"
                  display: AbstractButton.IconOnly
                  enabled: figureEditor
                  icon.name: "zoom-out"
                  icon.color: PrecitecApplication.Settings.alternateBackground
                  opacity: enabled ? 1.0 : 0.5
                  onClicked: {
                    var s = Math.min(1000, laserPointController.figureScale * 10)
                    laserPointController.figureScale = s
                    importController.renderSegments()
                  }
                  ToolTip.text: qsTr("Zoom out") //: This is a tooltip of a Toolbar button
                  ToolTip.visible: hovered
                  ToolTip.delay: 350
              }
              ToolButton {
                  id: zoomIn
                  objectName: "figureEditor-buttonLayout-zoom-in"
                  Layout.alignment: Qt.AlignVCenter
                  display: AbstractButton.IconOnly
                  enabled: figureEditor
                  icon.name: "zoom-in"
                  icon.color: PrecitecApplication.Settings.alternateBackground
                  opacity: enabled ? 1.0 : 0.5
                  onClicked: {
                    var s = Math.max(10, laserPointController.figureScale / 10)
                    laserPointController.figureScale = s
                    importController.renderSegments()
                  }

                  ToolTip.text: qsTr("Zoom in") //: This is a tooltip of a Toolbar button
                  ToolTip.visible: hovered
                  ToolTip.delay: 350
              }
            }

            TabBar {
                id: bar
                Layout.fillWidth: true

                currentIndex: 0

                TabButton {
                    text: qsTr("Figure")
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: parent.checked ? "black" : "white"
                    }
                    font.bold: true
                    font.pixelSize: 18
                    background: Rectangle {
                        color: parent.checked ? "white" : PrecitecApplication.Settings.alternateBackground
                        border.width: parent.check ? 0 : 1
                        border.color: "white"
                    }
                }
                TabButton {
                    text: qsTr("Segment")
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: parent.checked ? "black" : "white"
                    }
                    font.bold: true
                    font.pixelSize: 18
                    background: Rectangle {
                        color: parent.checked ? "white" : PrecitecApplication.Settings.alternateBackground
                        border.width: parent.check ? 0 : 1
                        border.color: "white"
                    }
                }
            }

            StackLayout {
              id: stackLayout
              Layout.columnSpan: 2

              currentIndex: bar.currentIndex

              // Figure properties
              ColumnLayout {
                Layout.fillWidth: true
                Label {
                  id: unitLabel
                  Layout.fillWidth: true
                  //Layout.columnSpan: 2
                  text: qsTr("Unit of input (File: " + importController.fileUnit + ")")
                }
                ComboBox {
                    id: unitCombo
                    textRole: "text"
                    valueRole: "value"

                    model: ListModel {
                        ListElement { text: "From file"; value: DxfImportController.FromFile }
                        ListElement { text: "mm";        value: DxfImportController.Millimeter }
                        ListElement { text: "cm";        value: DxfImportController.Centimeter }
                        ListElement { text: "in";        value: DxfImportController.Inches }
                    }
                    onActivated: importController.unit = currentValue
                    currentIndex: indexOfValue(importController.unit)

                    Rectangle {
                        id: unitHilighter
                        border.color: "red"
                        border.width: 5
                        anchors.fill: parent
                        opacity: 0

                        SequentialAnimation {
                            id: unitHilighterAnimation
                            alwaysRunToEnd: true
                            NumberAnimation {target: unitHilighter; property: "opacity"; to: 1; duration: 500}
                            NumberAnimation {target: unitHilighter; property: "opacity"; to: 0; duration: 500}
                        }
                    }
                }

                Label {
                  id: accuracyLabel
                  Layout.fillWidth: true
                  //Layout.columnSpan: 2
                  text: qsTr("Accuracy [mm]")
                }
                TextField {
                  id: accuracy
                  Layout.fillWidth: true
                  //Layout.columnSpan: 2
                  text: Number(importController.accuracy).toLocaleString(
                          locale, 'f', 3)
                  palette.text: accuracy.acceptableInput ? "black" : "red"
                  validator: DoubleValidator {
                    bottom: 0.01
                    top: 100.0
                  }
                  onEditingFinished: {
                    importController.accuracy = Number.fromLocaleString(
                          locale, accuracy.text)
                  }
                  selectByMouse: true

                  ToolTip.text: qsTr("For approximating curved elements and joining nearby elements") //: This is a tooltip of a Toolbar button
                  ToolTip.visible: hovered
                  ToolTip.delay: 350
                }

                Label {
                  id: maxDistLabel
                  Layout.fillWidth: true
                  //Layout.columnSpan: 2
                  text: qsTr("Stepsize [mm]")
                }
                TextField {
                  id: maxDistance
                  Layout.fillWidth: true
                  placeholderText: qsTr("no limit")

                  function maxDistanceText() {
                      return importController.maxDist > 0 ? Number(importController.maxDist).toLocaleString(locale, 'f', 3) : ""
                  }

                  //Layout.columnSpan: 2
                  text: maxDistanceText()
                  onEditingFinished: {
                      let val = -1;
                      try {
                         val = Number.fromLocaleString(locale, maxDistance.text)
                      } catch(e)
                      {
                      }

                      importController.maxDist = val
                      text = maxDistanceText()
                  }
                  selectByMouse: true

                  Connections {
                      target: importController

                      function onMaxDistChanged() {
                          maxDistance.text = maxDistance.maxDistanceText()
                      }
                  }

                  ToolTip.text: qsTr("Maximum distance between successive points") //: This is a tooltip of a Toolbar button
                  ToolTip.visible: hovered
                  ToolTip.delay: 350
                }

                Label {
                    text: qsTr("%1 points").arg(importController.pointCount)
                }

                Label
                {
                  Layout.fillWidth: true
                  text: qsTr("Name")
                }

                TextField {
                  id: name
                  Layout.fillWidth: true
                  text: importController.name
                  onEditingFinished: importController.name = name.text
                }

                /*
                // TODO: Setting the ID had no obvious effect. Had to set it when doing "save as"... so at the moment it makes no sense to offer this option during import.
                Label
                {
                  Layout.fillWidth: true
                  text: qsTr("ID")
                }

                TextField {
                  id: figureId
                  Layout.fillWidth: true
                  text: importController.figureId

                  palette.text: figureId.acceptableInput ? "black" : "red"
                  validator: IntValidator {
                    bottom: 0
                  }
                  onEditingFinished: {
                    importController.figureId = figureId.text
                  }
                  selectByMouse: true
                }
                */

                Label
                {
                  Layout.fillWidth: true
                  text: qsTr("Description")
                }

                TextField {
                  id: description
                  Layout.fillWidth: true
                  text: importController.description
                  onEditingFinished: importController.description = description.text
                }

                Button {
                    id: fitButton
                    Layout.fillWidth: true
                    text: qsTr("Fit View")
                    onClicked: fitView()
                }

                Button {
                  id: restartWithDefaults
                  Layout.fillWidth: true
                  text: qsTr("Restart With Defaults");
                  onClicked: importController.restartWithDefaults()
                }
              }

              // per-segment properties
              ReorderableListView {
                id: segmentList
                flickableDirection: Flickable.VerticalFlick
                boundsBehavior: Flickable.StopAtBounds
                ScrollBar.vertical: ScrollBar {
                  active: true
                }
                clip: true
                model: importController

                Connections {
                  target: importController
                  function onSegmentSelected(index) {
                      segmentList.currentIndex = index
                      bar.currentIndex = 1 // switch to segment-tab
                  }
                }

                focus: true
                highlightMoveDuration: 0
                highlightResizeDuration: 0

                onMoveItem: {
                    importController.moveSegment(sourceIdx, destinationIdx)
                    importController.select(destinationIdx)
                }

                delegate: ReorderableListDelegate {
                    listView: segmentList

                    ItemDelegate {
                        onClicked: {
                            importController.select(index)
                        }

                        background: Rectangle {
                            id: background
                            width: segmentList.width // - padding
                            height: myContent.height + padding
                            border.color: PrecitecApplication.Settings.alternateBackground
                            color: segmentList.currentIndex == model.index ? "lightsteelblue" : "white"
                        }

                        contentItem: Control {
                            id: myContent
                            width: parent.width
                            padding: 10

                            contentItem: GridLayout
                            {
                                columns: 2
                                Label{
                                    Layout.alignment: Qt.AlignRight
                                    text: qsTr("Index:")
                                }

                                SpinBox {
                                    id: indexSpin
                                    value: model.index + 1
                                    from: 1
                                    to: importController.maxIndex + 1
                                    editable: true
                                    onValueModified: {
                                        let idx = value - 1
                                        segmentList.currentIndex = idx
                                        importController.swapSegments(model.index, idx)
                                        importController.select(idx)
                                    }
                                }

                                Label{
                                    Layout.alignment: Qt.AlignRight
                                    text: qsTr("Reverse:")
                                }
                                CheckBox {
                                    id: reverseFlag
                                    checked: model.reverse
                                    onToggled: model.reverse = checked
                                }

                                Label {
                                    Layout.alignment: Qt.AlignRight
                                    text: qsTr("Stepsize:")
                                }

                                TextField {
                                  id: segMaxDistance
                                  Layout.fillWidth: true
                                  text: model.maxDist <= 0 ? "" : Number(model.maxDist).toLocaleString(locale, 'f', 3)
                                  placeholderText: model.maxDist == -1 ? "use figure setting" : "no limit"
                                  hoverEnabled: true
                                  onEditingFinished: {
                                      if (text == "" && model.maxDist <= 0)
                                      {
                                          return;
                                      }

                                      let val = -1;
                                      try {
                                         val = Number.fromLocaleString(locale, text)
                                      } catch(e)
                                      {
                                      }

                                      model.maxDist = val;
                                  }
                                  selectByMouse: true
                                  ToolButton {
                                      visible: segMaxDistance.hovered
                                      display: AbstractButton.IconOnly
                                      icon.name: "edit-undo"
                                      opacity: 0.7
                                      x: segMaxDistance.width - implicitWidth
                                      onPressed: {
                                          model.maxDist = !model.maxDist ? -1 : 0
                                      }

                                      ToolTip.text: qsTr("Toggle between no value/global setting") //: This is a tooltip of a Toolbar button
                                      ToolTip.visible: hovered
                                      ToolTip.delay: 350

                                  }
                                  ToolTip.text: qsTr("Maximum distance between successive points") //: This is a tooltip of a Toolbar button
                                  ToolTip.visible: hovered
                                  ToolTip.delay: 350
                                }

                                Label {
                                  text: qsTr("Accuracy:")
                                  Layout.alignment: Qt.AlignRight
                                  visible: model.curved
                                }
                                TextField {
                                  id: segAccuracy
                                  visible: model.curved
                                  Layout.fillWidth: true
                                  text: model.accuracy <= 0 ? "" : Number(model.accuracy).toLocaleString(locale, 'f', 3)
                                  placeholderText: "use figure setting"
                                  onEditingFinished: {
                                      let val = -1;
                                      try {
                                         val = Number.fromLocaleString(locale, text)
                                      } catch(e)
                                      {
                                      }

                                      model.accuracy = val;
                                  }
                                  selectByMouse: true
                                  ToolTip.text: qsTr("Maximum deviation from ideal path") //: This is a tooltip of a Toolbar button
                                  ToolTip.visible: hovered
                                  ToolTip.delay: 350
                                }

                                Label{
                                    text: qsTr("Start Angle:")
                                    Layout.alignment: Qt.AlignRight
                                    visible: model.circle
                                }
                                TextField {
                                    id: startAngle
                                    visible: model.circle
                                    Layout.fillWidth: true
                                    text: Number(model.startAngle).toLocaleString(
                                              locale, 'f', 2)
                                    palette.text: startAngle.acceptableInput ? "black" : "red"
                                    validator: DoubleValidator {
                                        bottom: 0
                                        top: 360
                                    }
                                    onEditingFinished: {
                                        model.startAngle = Number.fromLocaleString(
                                                    locale, startAngle.text)
                                    }
                                    selectByMouse: true
                                }

                                Label {}

                                Label {
                                    text: qsTr("%1 points").arg(model.pointCount)
                                }
                            }
                        }
                    }
                }
              }
            }
        }
    }

}
