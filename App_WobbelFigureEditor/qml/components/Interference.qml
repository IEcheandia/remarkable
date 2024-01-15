import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import wobbleFigureEditor.components 1.0

GridLayout
{
    id: attributesInterference
    property var fileEditor: null
    property var fileModel: null
    columns: 2

    ListModel
    {
        id: overlay

        ListElement
        {
            name: "Sin"
        }
        ListElement
        {
            name: "Free"
        }
        ListElement
        {
            name: "None"
        }
    }

    FileSortModel {
        id: fileOverlayModel
        fileType: FileType.Overlay
        sourceModel: attributesInterference.fileModel
    }

    Label
    {
        id: interferenceLabel
        Layout.fillWidth: true
        text: qsTr("Overlay figure:")
        font.bold: true
    }

    ComboBox
    {
        id: interference
        Layout.fillWidth: true
        currentIndex: attributesInterference.fileEditor.figureCreator.interference
        model: overlay
        onActivated:
        {
            attributesInterference.fileEditor.figureCreator.interference = interference.currentIndex;
        }
    }

    Label {}

    Label
    {
        property bool show: interference.currentIndex === 1
        id: freeInterferenceLabel
        Layout.fillWidth: true
        Layout.columnSpan: 2
        text: qsTr("Free Overlay functions:")
        font.bold: true
    }

    Label {property bool show: freeInterferenceLabel.show}

    ComboBox
    {
        property bool show: freeInterferenceLabel.show
        id: freeInterference
        Layout.fillWidth: true
        Layout.columnSpan: 2
        currentIndex: attributesInterference.fileEditor.figureCreator.freeInterference
        model: fileOverlayModel
        textRole: "name"
        onActivated:
        {
            attributesInterference.fileEditor.figureCreator.overlayFilename = freeInterference.textAt(freeInterference.currentIndex);
            attributesInterference.fileEditor.figureCreator.freeInterference = freeInterference.currentIndex;
        }
    }
    Label {property bool show: freeInterferenceLabel.show}
}
