import QtQuick 2.15
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.image 1.0 as PrecitecImage
import Precitec.AppGui 1.0
import QtQml 2.15

Dialog {
    id: root
    property var product: null

    signal seriesSelected(var uuid);

    title: qsTr("Copy Scan Field Image from existing seam series")
    parent: Overlay.overlay
    anchors.centerIn: Overlay.overlay
    modal: true
    standardButtons: Dialog.Ok | Dialog.Cancel
    onAccepted: {
        root.seriesSelected(seamSeriesCombo.model[seamSeriesCombo.currentIndex].uuid);
        destroy();
    }
    onRejected: {
        destroy();
    }

    Component.onCompleted: {
        root.standardButton(Dialog.Ok).enabled = Qt.binding(function() { return image.imageValid; });
    }

    ScanfieldSeamModel {
        id: imageController
        scanfieldModule {
            grabberDeviceProxy: HardwareModule.grabberDeviceProxy
            calibrationCoordinatesRequestProxy: HardwareModule.calibrationCoordinatesRequestProxy
        }
        seamSeries: seamSeriesCombo.currentIndex != -1 ? seamSeriesCombo.model[seamSeriesCombo.currentIndex] : null
        transformation: image.transformation
        showAllSeams: false
        octWithReferenceArms: false
    }

    ColumnLayout {
        anchors.fill: parent
        ComboBox {
            id: productCombo
            model: productFilterModel
            textRole: "display"

            Layout.fillWidth: true

            Component.onCompleted: {
                currentIndex = productCombo.find(root.product ? root.product.name : "")
            }
        }
        ComboBox {
            id: seamSeriesCombo
            model: productFilterModel.data(productFilterModel.index(productCombo.currentIndex, 0), (Qt.UserRole+1)) ? productFilterModel.data(productFilterModel.index(productCombo.currentIndex, 0), (Qt.UserRole+1)).seamSeries : []
            displayText: model[currentIndex] ? model[currentIndex].visualNumber + ": " + model[currentIndex].name  : ""

            delegate: ItemDelegate {
                width: seamSeriesCombo.width
                text: modelData.visualNumber + ": " + modelData.name
                highlighted: seamSeriesCombo.highlightedIndex === index
            }
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: scanfieldImageContainer

            border.color: PrecitecApplication.Settings.alternateBackground
            border.width: 1
            clip: true

            Label {
                visible: (!image.imageValid || !imageController.scanfieldModule.configurationValid)
                anchors.centerIn: parent
                text: qsTr("No Image available.")
            }

            PrecitecImage.SourceImageItem {

                id: image
                thumbnail: imageController.scanfieldModule.cameraSize
                thumbnailOriginalSize: imageController.scanfieldModule.imageSize
                visible: (image.imageValid || image.loading) && imageController.scanfieldModule.configurationValid
                anchors.fill: parent

                Binding {
                    target: image
                    property: "source"
                    delayed: true
                    value: imageController.scanfieldModule.sourceImageDir
                    when: imageController.scanfieldModule.configurationValid && !imageController.scanfieldModule.loading
                    restoreMode: Binding.RestoreBindingOrValue
                }

                onImageChanged: image.zoomToFit();
                Component.onCompleted: image.zoomToFit();

            }
        }

    }
    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            root.reject()
        }
    }
}

