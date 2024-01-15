import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.ethercat 1.0 as EtherCAT
import precitec.gui.components.application 1.0
import precitec.gui.components.plotter 1.0

Item {
    id: root
    property alias viConfig: viConfigModel.viConfig
    property alias service: viConfigModel.service
    property alias storageDir: viConfigModel.storageDir

    onVisibleChanged: viConfigModel.clearDataSets()

    Component.onCompleted: viConfigModel.clearDataSets()

    EtherCAT.ViConfigModel {
        id: viConfigModel
        defaultDataSetColor: Settings.alternateBackground
        onRecordingChanged: {
            recordController.needsReload = true;
        }
    }

    EtherCAT.RecordedSignalAnalyzerController {
        id: recordController
        property bool needsReload: false
        storageDir: viConfigModel.storageDir
    }

    Component {
        id: recordView

        GridLayout {
            columns: 6

            Label {
                Layout.columnSpan: 4
                height: backButton.height
                text: qsTr("Recorded at: %1").arg(Qt.formatDateTime(recordController.recordingDate))
                visible: !recordController.loading && recordController.hasData
            }
            Item {
                Layout.fillWidth: true
                visible: !recordController.loading && recordController.hasData
            }
            ToolButton {
                id: backButton
                display: AbstractButton.IconOnly
                icon.name: "arrow-left"
                icon.color: Settings.alternateBackground
                onClicked: stackView.pop()
            }
            ToolButton {
                visible: !recordController.loading && recordController.hasData
                autoRepeat: true
                text: qsTr("Move left")
                icon.name: "arrow-left"
                icon.color: Settings.alternateBackground
                onClicked: multiTrackPlotter.moveLeft()
            }
            ToolButton {
                visible: !recordController.loading && recordController.hasData
                autoRepeat: true
                text: qsTr("Move right")
                icon.name: "arrow-right"
                icon.color: Settings.alternateBackground
                onClicked: multiTrackPlotter.moveRight()
            }
            ToolButton {
                visible: !recordController.loading && recordController.hasData
                autoRepeat: true
                text: qsTr("Zoom in")
                icon.name: "zoom-in"
                icon.color: Settings.alternateBackground
                enabled: (multiTrackPlotter.zoom + 0.1) < multiTrackPlotter.maxZoom
                onClicked: {
                    multiTrackPlotter.zoom += 0.1;
                }
            }
            ToolButton {
                visible: !recordController.loading && recordController.hasData
                autoRepeat: true
                text: qsTr("Zoom out")
                icon.name: "zoom-out"
                icon.color: Settings.alternateBackground
                enabled: (multiTrackPlotter.zoom - 0.1) > multiTrackPlotter.minZoom
                onClicked: {
                    multiTrackPlotter.zoom -= 0.1;
                }
            }
            Item {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                visible: !recordController.loading && recordController.hasData
            }
            Item {
                Layout.columnSpan: 6
                Layout.fillHeight: true
                Layout.fillWidth: true

                MultiTrackPlotter {
                    id: multiTrackPlotter
                    anchors.fill: parent
                    model: recordController.dataSets
                    rows: 2
                    plotterHeight: height / 8
                    xRange: recordController.xRange
                    autoAdjustYAxis: true
                    xLegendUnitVisible: false
                    yLegendUnitVisible: false
                    menuAvailable: false
                    showLabels: true
                }
            }
        }
    }

    StackView {
        id: stackView
        anchors{
            fill: parent
            margins: 5
        }
        clip: true
        initialItem: GridLayout {

            enabled: !viConfigModel.processing
            flow: GridLayout.TopToBottom
            rows: 3

            Label {
                text: qsTr("Record selected signals:")
            }
            SignalGroup {
                enabled: !viConfigModel.recording
                title: qsTr("Input Signals")
                model: viConfigModel
                signalType: EtherCAT.ViConfigService.Input
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.columnSpan: 3
            }
            SignalGroup {
                enabled: !viConfigModel.recording
                title: qsTr("Output Signals")
                model: viConfigModel
                signalType: EtherCAT.ViConfigService.Output
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.columnSpan: 3
            }
            Button {
                text: qsTr("Start recording")
                display: Button.TextBesideIcon
                icon.name: "media-record"
                icon.color: "transparent"
                visible: !viConfigModel.recording
                enabled: viConfigModel.itemsEnabled
                onClicked: viConfigModel.startRecording()
            }
            Button {
                text: qsTr("Stop recording")
                display: Button.TextBesideIcon
                icon.name: "media-playback-stop"
                icon.color: "transparent"
                visible: viConfigModel.recording
                onClicked: viConfigModel.stopRecording()
            }
            Button {
                text: qsTr("View latest recording")
                display: Button.TextBesideIcon
                icon.name: "document-open"
                icon.color: Settings.alternateBackground
                onClicked: {
                    if (recordController.needsReload)
                    {
                        recordController.load();
                        recordController.needsReload = false;
                    }
                    stackView.push(recordView);
                }
            }
            Item {
                Layout.rowSpan: 3
                Layout.fillHeight: true
                Layout.fillWidth: true

                MultiTrackPlotter {
                    anchors.fill: parent
                    model: viConfigModel.dataSets
                    plotterHeight: height / 8
                    clip: true
                    xRange: 10.0
                    rows: 2
                    autoAdjustYAxis: true
                    xLegendUnitVisible: false
                    yLegendUnitVisible: false
                    menuAvailable: false
                    showLabels: true
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: viConfigModel.processing
    }
}
