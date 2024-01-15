import QtQuick 2.7
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

Dialog {
    id: dialog
    property alias graphView: graphLoaderComponent.graphView
    property alias graphLoader: graphLoaderComponent.graphLoader
    property alias graphVisualizer: graphLoaderComponent.graphVisualizer
    property alias directoryModel: graphLoaderComponent.directoryModel
    property var screenshotTool: null

    title: qsTr("Open existing Graph")
    modal: true

    onAccepted: destroy()
    onRejected: destroy()

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        title: dialog.title
        screenshotTool: dialog.screenshotTool
    }
    // workaround for https://bugreports.qt.io/browse/QTBUG-72372
    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        Button {
            text: qsTr("Close")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

    GraphLoaderComponent {
        id: graphLoaderComponent
        anchors.fill: parent
        window: ApplicationWindow.window
        onGraphSelected: dialog.accept()
    }
}
