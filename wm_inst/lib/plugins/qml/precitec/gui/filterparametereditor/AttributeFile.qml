import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.general 1.0

/**
 * A file selection control. Shows the currently selected file name and offers:
 * @li a button to clear
 * @li a button to open a file selection dialog
 * @li a button to open an import dialog (only shown if removable device is attached)
 **/
Control {
    /**
     * Emitted when the TextField value gets modified.
     **/
    signal parameterValueModified()
    property var attribute: null
    property var parameter: null
    property string parameterValue: ""
    property var defaultValue: undefined
    readonly property string value: parameter ? parameter.value : (attribute ? attribute.defaultValue : "")
    id: file

    Component {
        id: fileDialogComponent
        RemovableDevices.FileSelectionDialog {
            id: fileDialog
            path: attribute ? WeldmasterPaths.baseDirectory + "/" + attribute.fileInformation.location : WeldmasterPaths.baseDirectory
            suffixes: attribute ? attribute.fileInformation.suffixes : []
            onAccepted: {
                file.parameterValue = fileDialog.selectedFile;
                file.parameterValueModified();
            }

            onClosed: destroy()
        }
    }

    Component {
        id: importFileDialogComponent
        RemovableDevices.ImportFileDialog {
            id: importFileDialog
            suffixes: attribute ? attribute.fileInformation.suffixes : []
            targetLocation: attribute ? WeldmasterPaths.baseDirectory + "/" + attribute.fileInformation.location : WeldmasterPaths.baseDirectory
            includeSubDirectories: true
            onAccepted: {
                if (importFileDialog.importedFile != "")
                {
                    file.parameterValue = importFileDialog.importedFile;
                    file.parameterValueModified();
                }
            }

            onClosed: destroy()
        }
    }

    contentItem: RowLayout {
        Label {
            text: file.value != "" ? file.value : qsTr("No file selected")
            Layout.fillWidth: true
        }
        ToolButton {
            icon.name: "edit-clear"
            enabled: file.value != ""
            display: ToolButton.IconOnly
            onClicked: {
                file.parameterValue = "";
                file.parameterValueModified();
            }
        }
        ToolButton {
            icon.name: "document-open"
            enabled: attribute != null
            display: ToolButton.IconOnly
            onClicked: fileDialogComponent.createObject(file).open()
        }
        ToolButton {
            icon.name: "cloud-upload"
            display: ToolButton.IconOnly
            visible: RemovableDevices.Service.udi != ""
            enabled: RemovableDevices.Service.path != ""
            onClicked: importFileDialogComponent.createObject(file).open()
        }
    }

    ToolTip.text: attribute ? LanguageSupport.getStringWithFallback(attribute.description) : ""
    ToolTip.visible: hovered && ToolTip.text != ""
    ToolTip.timeout: 5000
}

