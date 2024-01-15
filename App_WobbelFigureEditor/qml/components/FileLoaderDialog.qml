import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

import wobbleFigureEditor.components 1.0

Dialog {
    /*
    If this dialog is used for the ordinary "open file" use case then only fileModel gets used. It is responsible
    for enumerating the files and loading the selected file.

    If this dialog is used to select a file to import then fileModel lists the files that are available for import
    and outFileModel is used to load the imported file.
    */
    property var fileModel: null
    property var outFileModel: null
    property var fileEditor: null
    property var simulationController: null
    property var commandManager: null
    property DxfImportDialog dxfImportDialog: null
    property alias screenshotTool: fileLoaderModul.screenshotTool

    Component {
        id: importPathInfo
        GridLayout {
            columns: 2

            Label {text: "Overlay and seam figures:"; Layout.alignment: Qt.AlignRight; font.bold: true}
            Label {text: "/weldmaster/import/weld_figure"}
            Label {text: "Wobble figures:"; Layout.alignment: Qt.AlignRight; font.bold: true}
            Label {text: "/weldmaster/import/laser_controls"}
            Label {text: "DXF files:"; Layout.alignment: Qt.AlignRight; font.bold: true}
            Label {text: "/weldmaster/import/dxf"}
        }
    }

    Component {
        id: importHelpDialog
        Dialog {
            header: PrecitecApplication.DialogHeaderWithScreenshot {
                title: qsTr("Import Search Paths")
                screenshotTool: fileLoaderModul.screenshotTool
            }
            anchors.centerIn: parent
            modal: true
            standardButtons: Dialog.Close

            Loader {sourceComponent: importPathInfo}
        }
    }

    Component {
        id: importHelpToolButton
        ToolButton {
            icon.name: "view-help"
            icon.width: 32
            icon.height: 32
            flat: true
            onClicked: importHelpDialog.createObject(ApplicationWindow.window).open();
            palette.button: "transparent"
        }
    }

    Component.onCompleted: {
        if (dxfImportDialog)
        {
            importHelpToolButton.createObject(header.contentItem)
        }
    }

    function createNew() {
        fileLoaderModul.createNew()
    }

    id: fileAndFigureItem

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        title: dxfImportDialog ? qsTr("Import Files") : qsTr("Files")
        screenshotTool: fileLoaderModul.screenshotTool
    }

    FileLoader {
        id: fileLoaderModul
        anchors.fill: parent
        fileModel: fileAndFigureItem.fileModel
        outFileModel: fileAndFigureItem.outFileModel
        fileEditor: fileAndFigureItem.fileEditor
        simulationController: fileAndFigureItem.simulationController
        fileAndFigure: fileAndFigureItem
        commandManager: fileAndFigureItem.commandManager
        dxfImportDialog: fileAndFigureItem.dxfImportDialog
    }

    // If this is an import dialog and no files are found: Show info about search paths
    ColumnLayout
    {
        visible: fileAndFigureItem.dxfImportDialog && fileAndFigureItem.fileModel && fileAndFigureItem.fileModel.empty
        Label {text: "No import files found on removable media."; Layout.alignment: Qt.AlignCenter; font.bold: true}
        Label {text: "\nImport search paths:"; Layout.alignment: Qt.AlignCenter; font.bold: true}
        Loader {sourceComponent: importPathInfo}
    }

    // workaround for https://bugreports.qt.io/browse/QTBUG-72372
    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        Button {
            text: qsTr("Close")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }
}
