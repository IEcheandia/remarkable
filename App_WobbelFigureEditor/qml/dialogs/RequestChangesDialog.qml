import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

import wobbleFigureEditor.components 1.0

Dialog {
    id: requestChangesDialog
    modal: true

    property var screenshotTool: null
    property var requestChangesManager: null
    property var figureEditor: null
    property bool offerSave: true

    // workaround: the signal of DialogButtonBox can't checked which role is emit when push a button
    signal cancel
    signal saveAs(var isSaveAs)
    closePolicy: Popup.NoAutoClose

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        title: qsTr("Save your Changes?")
        screenshotTool: requestChangesDialog.screenshotTool
    }

    Column {
        Text {
            text: qsTr("This figure has unsaved modifications.")
        }
        Text {
            text: qsTr("Do you want to save your changes?")
        }
    }

    // workaround for https://bugreports.qt.io/browse/QTBUG-72372
    footer: DialogButtonBox {
        id: buttonBox
        alignment: Qt.AlignRight
        Button {
            text: qsTr("Save")
            icon.name: "document-save"
            visible: offerSave
            onReleased: {
                saveAs(false)
            }
        }
        Button {
            text: qsTr("Save As ..")
            icon.name: "document-save-as"
            onReleased: {
                saveAs(true)
            }
        }
        Button {
            text: qsTr("Discard")
            icon.name: "edit-delete"
            onReleased: {
                reject()
            }
        }
        Button {
            text: qsTr("Cancel")
            icon.name: "window-close"
            onReleased: {
                cancel()
            }
        }
    }

    onSaveAs: {
        var newSaveAsDialog = saveAsFigureDialogComponent.createObject(
                    applicationWindow, {
                        "fileEditor": figureEditor,
                        "requestChangesManager": requestChangesManager,
                        "saveAs": isSaveAs
                    })
        newSaveAsDialog.open()
        close()
    }

    onRejected: {
        requestChangesManager.startAction()
    }

    onCancel: {
        requestChangesManager.resetAction()
        close()
    }

    Component {
        id: saveAsFigureDialogComponent
        SaveAsFigureDialog {
            id: saveAsFigureDialog

            anchors.centerIn: parent
            width: parent.width * 0.3
            height: parent.height * 0.4
            screenshotTool: requestChangesDialog.screenshotTool
        }
    }
}
