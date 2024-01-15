import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications

import wobbleFigureEditor.components 1.0

Dialog {
    id: saveAsDialog
    property var requestChangesManager: null
    property var fileEditor: null
    property bool saveAs: false
    property alias screenshotTool: screenshotHeader.screenshotTool
    modal: true
    standardButtons: Dialog.Close | Dialog.Ok

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: saveAsDialog.saveAs ? qsTr("Save figure as...") : qsTr(
                                         "Save figure...")
    }

    onOpened: {
        fileSaveHandler.number = Number.fromLocaleString(locale,
                                                         fileNameID.text)
        fileSaveHandler.checkIfNumberIsAlreadyUsed()
    }

    onAccepted: {
        if (saveAsDialog.saveAs) {
            fileEditor.saveAsFigure(fileName.text,
                                    saveAsDialog.fileEditor.fileType)
            if (requestChangesManager.requestIsInAction()) {
                requestChangesManager.startAction()
            }
            return
        }
        fileEditor.saveFigure()
        if (requestChangesManager.requestIsInAction()) {
            requestChangesManager.startAction()
        }
    }
    onRejected: {
        if (requestChangesManager.requestIsInAction()) {
            requestChangesManager.resetAction()
        }
        destroy()
    }

    GroupBox {
        id: gridPropertiesView
        implicitWidth: parent.width
        implicitHeight: parent.height
        GridLayout {
            anchors.fill: parent
            columns: 3

            Label {
                visible: !saveAsDialog.saveAs
                id: saveDialogLabel
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                Layout.margins: parent.width * 0.3
                Layout.columnSpan: 3
                text: saveAsDialog.fileEditor
                      && saveAsDialog.fileEditor.fileModel
                      && saveAsDialog.fileEditor.fileModel.fileName
                      != "" ? qsTr("Overwrite file %1").arg(
                                  saveAsDialog.fileEditor.fileModel.filename) : qsTr(
                                  "No file selected")
            }
            Label {
                visible: saveAsDialog.saveAs
                id: selectTypeLabel
                Layout.fillWidth: true
                Layout.columnSpan: 3
                text: qsTr("File type:")
                font.bold: true
            }

            Label {
                visible: saveAsDialog.saveAs
                id: selectType
                Layout.leftMargin: 10
                Layout.fillWidth: true
                Layout.columnSpan: 3
                text: fileSaveHandler.fileTypeLabel
            }
            Label {
                visible: saveAsDialog.saveAs
                id: fileNameIDLabel
                Layout.fillWidth: true
                Layout.columnSpan: 3
                text: qsTr("Enter number:")
                font.bold: true
            }
            TextField {
                visible: saveAsDialog.saveAs
                id: fileNameID
                Layout.fillWidth: true
                text: Number(fileSaveHandler.number).toLocaleString(locale,
                                                                    'f', 0)
                palette.text: fileNameID.acceptableInput ? "black" : "red"
                validator: IntValidator {
                    bottom: 0
                    top: 1000
                }
                onEditingFinished: {
                    fileSaveHandler.number = Number.fromLocaleString(
                                locale, fileNameID.text)
                }
            }
            ToolButton {
                visible: saveAsDialog.saveAs
                id: decrementID
                enabled: fileEditor && fileSaveHandler
                icon.name: "arrow-down"
                icon.color: "black"
                flat: true
                onClicked: {
                    fileSaveHandler.searchLowerNumber = true
                    fileSaveHandler.searchAvailableNumber()
                }
                palette.button: "darkgrey"
            }
            ToolButton {
                visible: saveAsDialog.saveAs
                id: incrementID
                enabled: fileEditor && fileSaveHandler
                icon.name: "arrow-up"
                icon.color: "black"
                flat: true
                onClicked: {
                    fileSaveHandler.searchLowerNumber = false
                    fileSaveHandler.searchAvailableNumber()
                }
                palette.button: "darkgrey"
            }
            Label {
                visible: saveAsDialog.saveAs
                id: fileNameLabel
                Layout.fillWidth: true
                Layout.columnSpan: 3
                text: qsTr("File name:")
                font.bold: true
            }
            TextField {
                visible: saveAsDialog.saveAs
                property string name: fileSaveHandler.filePrefix
                id: fileName
                Layout.fillWidth: true
                Layout.columnSpan: 3
                text: ("%1%2").arg(fileName.name).arg(
                          Number.fromLocaleString(locale, fileNameID.text))
                readOnly: true
                background: Item {}
            }
            Item {
                visible: saveAsDialog.saveAs
                         && !fileSaveHandler.numberAlreadyUsed
                id: numberOkWildcard
                Layout.fillWidth: true
                Layout.columnSpan: 2
            }
            TextField {
                visible: saveAsDialog.saveAs
                         && fileSaveHandler.numberAlreadyUsed
                id: numberOkText
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: qsTr("Overwrite existing file?")
                palette.text: "red"
                font.bold: true
                readOnly: true
                background: Item {}
            }
            ToolButton {
                visible: saveAsDialog.saveAs
                enabled: false
                id: numberOkImage
                Layout.columnSpan: 1
                icon.name: fileSaveHandler.numberAlreadyUsed ? "dialog-error" : "dialog-ok"
                icon.color: fileSaveHandler.numberAlreadyUsed ? "transparent" : "green"
            }

            Item {
                id: wildcard
                Layout.columnSpan: 3
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    FileSaveHandler {
        id: fileSaveHandler
        fileModel: fileEditor
                   && fileEditor.fileModel ? fileEditor.fileModel : null
        type: fileEditor ? fileEditor.fileType : FileType.None
    }
}
