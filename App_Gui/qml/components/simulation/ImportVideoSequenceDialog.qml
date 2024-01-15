import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

import precitec.gui.components.application 1.0
import precitec.gui.components.removableDevices 1.0 as RemovableDevices
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.notifications 1.0
import precitec.gui 1.0

Dialog {
    id: importDialog
    parent: Overlay.overlay
    anchors.centerIn: parent
    width: parent.width * 0.8
    height: parent.height * 0.8
    modal: true
    enabled: !importService.backupInProgress
    title: qsTr("Import product instance")
    property var importService
    property alias liveMode: productInstancesCacheController.liveMode
    property string productDirectory: ""
    property alias directoryName: stationImportModel.directoryName
    property alias screenshotTool: screenshotHeader.screenshotTool
    onClosed: {
       productInstancesCacheController.cacheBuffer();
       productInstancesCacheController.clearCacheBuffer();
    }
    header: DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: importDialog.title
    }

    ProductInstancesCacheController {
        id: productInstancesCacheController
    }
    // workaround for https://bugreports.qt.io/browse/QTBUG-72372
    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        Button {
            text: qsTr("Close")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

    VideoImportProductInstanceModel {
        id: videoImportModel
    }

    VideoImportProductInstanceModel {
        id: productImportModel
    }

    VideoImportProductModel {
        id: stationImportModel
        basePath: RemovableDevices.Service.path
    }

    Label {
        id: explanation
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: 5
        }
        wrapMode: Text.WordWrap
        textFormat: Text.RichText
        text: qsTr("Product instances need to be provided on the removable device in folder weldmaster/%1.<br/>The structure must match the download of the product instance. Otherwise it is not possible to import the product instance.<br/>The expected format is: <ul><li>weldmaster</li><li>&#8627;%1</li><li>&#8627;Station name</li><li>&#8627;Product name</li><li>&#8627;UUID-SN-serial</li><li>&#8627;seam_series####</li><li>&#8627;seam####</li></ul><br/>Seam series and seam can be provided multiple times, e.g. seam_series0001 and seam_series0002. It is important to provide the leading 0 digits.<br/>An example path is <strong>weldmaster/%1/System_1/Overlap/e86048ce-b3cd-4378-886e-0b24435e77fb-SN-1234/seam_series0001/seam0001</strong>").arg(importDialog.directoryName)
    }

    Rectangle {
        id: separator
        anchors {
            left: parent.left
            right: parent.right
            top: explanation.bottom
            margins: 20
        }
        height: 1
        color: Settings.alternateBackground
    }

    StackLayout {
        id: importStackLayout
        anchors {
            left: parent.left
            right: parent.right
            top: separator.bottom
            bottom: parent.bottom
            margins: 5
            topMargin: 20
        }
        GroupBox {
            title: qsTr("Select station")

            ScrollView {
                anchors.fill: parent

                ListView {
                    anchors.fill: parent
                    model: stationImportModel

                    delegate: ItemDelegate {
                        width: ListView.view.width
                        text: model.name
                        icon.name: "application-menu"
                        onClicked: {
                            productImportModel.basePath = model.fileInfo
                            importStackLayout.currentIndex++;
                        }
                    }
                }
            }
        }
        BackButtonGroupBox {
            title: qsTr("Select product")
            onBack: {
                parent.currentIndex--;
            }

            ScrollView {
                anchors.fill: parent

                ListView {
                    anchors.fill: parent
                    model: productImportModel

                    delegate: ItemDelegate {
                        width: ListView.view.width
                        text: model.name
                        icon.name: "application-menu"
                        onClicked: {
                            videoImportModel.basePath = model.fileInfo
                            importStackLayout.currentIndex++;
                        }
                    }
                }
            }
        }
        BackButtonGroupBox {
            title: qsTr("Select product instance to import")
            onBack: {
                parent.currentIndex--;
            }

            ScrollView {
                anchors.fill: parent

                ListView {
                    anchors.fill: parent
                    model: videoImportModel

                    delegate: ItemDelegate {
                        id: delegate
                        property string path: model.path
                        property string size: ""
                        width: ListView.view.width
                        text: model.name + (size != "" ? " (%1)".arg(size) : "")
                        icon.name: "cloud-upload"
                        onClicked: {
                            var productInstanceDirectory = importDialog.productDirectory + "/" + model.name;
                            if (!productInstancesCacheController.exists(productInstanceDirectory))
                            {
                                productInstancesCacheController.addProductInstancePathToBuffer(productInstanceDirectory);
                            }
                            importService.performDownload(model.fileInfo, model.name);
                        }
                        Component.onCompleted: {
                            if (importService)
                            {
                                importService.diskUsageAsync(delegate.path);
                            }
                        }
                        Connections {
                            target: importService
                            function onDiskUsageCalculated(path, usage) {
                                if (delegate.path == path)
                                {
                                    delegate.size = usage;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    BusyIndicator {
        anchors.centerIn: parent
        running: importService.backupInProgress
    }
}
