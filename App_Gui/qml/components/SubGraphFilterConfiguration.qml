import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import Precitec.AppGui 1.0
import precitec.gui 1.0
import precitec.gui.general 1.0

GroupBox {
    id: root

    property var graphModel
    property alias subGraphModel: subGraphCheckedFilterModel.sourceModel
    property alias currentSeam: subGraphCheckedFilterModel.seam
    property alias editGraph: editButton.visible
    property alias showGraph: showButton.visible
    property alias exportSupported: exportButton.visible
    property var onlineHelp: null

    signal preconfiguredGraphSelected()
    signal subGraphSelected(var uuid, var name)
    signal editGraphSelected()
    signal exportSelected()
    signal showSelectedSubGraph()

    SubGraphCheckedFilterModel {
        id: subGraphCheckedFilterModel
    }

    label: RowLayout {
        width: root.availableWidth
        Label {
            text: qsTr("Algorithm")
            font.family: root.font.family
            font.pixelSize: root.font.pixelSize
            font.bold: true
            verticalAlignment: Text.AlignVCenter

            Layout.leftMargin: root.leftPadding
            Layout.fillWidth: true
        }
        ToolButton {
            id: showButton
            display: AbstractButton.IconOnly
            icon.name: "quickview"
            visible: preconfiguredGraphLabel.visible && GuiConfiguration.formatHardDisk
            onClicked: root.showSelectedSubGraph()
        }
        ToolButton {
            id: editButton
            display: AbstractButton.IconOnly
            icon.name: "document-edit"
            onClicked: root.editGraphSelected()
        }
        ToolButton {
            id: exportButton
            icon.name: "document-export"
            display: AbstractButton.IconOnly
            enabled: RemovableDevices.Service.path != ""
            visible: false
            onClicked: root.exportSelected()
        }
        ToolButton {
            display: AbstractButton.IconOnly
            icon.name: "application-menu"
            visible: preconfiguredGraphLabel.visible
            onClicked: root.preconfiguredGraphSelected()
        }
    }

    RowLayout {
        width: root.availableWidth -10
        visible: root.currentSeam && !root.currentSeam.usesSubGraph
        Label {
            id: preconfiguredGraphLabel
            text: qsTr("Using pre-configured graph:\n %1").arg(visible ? root.graphModel.data(root.graphModel.indexFor(root.currentSeam.graph)) :"")
            verticalAlignment: Text.AlignVCenter
            Layout.leftMargin: root.leftPadding
            Layout.fillWidth: true
            MouseArea{
                anchors.fill: parent
                onClicked: root.preconfiguredGraphSelected()
            }
        }

        ToolButton {
            Layout.alignment : Qt.AlignRight
            icon.name: "view-help"
            icon.width: Math.min(availableWidth, availableHeight)
            icon.height: Math.min(availableWidth, availableHeight)
            visible: root.graphModel.data(root.graphModel.indexFor(root.currentSeam.graph),Qt.UserRole + 7);
            onClicked: {
                if (onlineHelp)
                {
                    onlineHelp.file =
                    root.graphModel.data(root.graphModel.indexFor(root.currentSeam.graph));
                    onlineHelp.open();
                }
            }
        }
    }

    ScrollView {
        visible: !preconfiguredGraphLabel.visible
        anchors.fill: parent
        clip: true
        ListView {
            id: subGraphListView
            anchors.fill: parent
            model: subGraphCheckedFilterModel
            spacing: 10
            delegate: MouseArea {
                width: ListView.view.width - 10
                height: 64
                RowLayout {
                    anchors {
                        fill: parent
                        margins: 0
                    }
                    Image {
                        Layout.preferredWidth: 64
                        Layout.preferredHeight: 64
                        Layout.leftMargin: 5

                        source: model.image
                        fillMode: Image.PreserveAspectFit
                    }
                    Label {
                        text: model.name
                        elide: Text.ElideRight
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        id: helpButton
                        icon.name: "view-help"
                        icon.width: Math.min(availableWidth, availableHeight)
                        icon.height: Math.min(availableWidth, availableHeight)
                        visible: model.pdfAvailable
                        onClicked: {
                            if (onlineHelp)
                            {
                                onlineHelp.file = model.name;
                                onlineHelp.open();
                            }
                        }
                    }
                }
                onClicked: root.subGraphSelected(model.uuid, model.name)
            }
            section {
                property: "category"
                delegate: Control {
                    width: ListView.view.width - 10
                    topInset: 2
                    bottomInset: 2
                    padding: 5
                    background: Rectangle {
                        color: PrecitecApplication.Settings.alternateBackground
                    }
                    contentItem: Label {
                        text: root.subGraphModel.categoryToName(section)
                        color: PrecitecApplication.Settings.alternateText
                    }
                }
            }
        }
    }
}
