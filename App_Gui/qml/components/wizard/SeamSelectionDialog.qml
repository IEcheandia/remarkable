import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.filterparametereditor 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

Dialog {
    property alias productController: productFilterModel.sourceModel
    property bool scanlabScannerAvailable: false
    property bool laserControlAvailable: false
    property alias graphModel: propertyModel.graphModel
    property alias subGraphModel: filterParameterController.subGraphModel
    property alias screenshotTool: screenshotHeader.screenshotTool
    property alias attributeModel: filterParameterEditor.attributeModel
    property alias resultsConfigModel: filterParameterEditor.resultsConfigModel
    property alias sensorConfigModel: filterParameterEditor.sensorConfigModel
    property alias errorConfigModel: filterParameterEditor.errorConfigModel
    property alias onlineHelp: filterParameterEditor.onlineHelp

    signal save()

    id: root

    title: (swipeView.currentIndex == 0 ? qsTr("Select Settings of \"%1\" (# %2) to copy") : qsTr("Select Seams to copy \"%1\" (# %2) Settings to")).arg(seamConfiguration.seam ? seamConfiguration.seam.name : "").arg(seamConfiguration.seam ? seamConfiguration.seam.visualNumber : "")

    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            root.reject()
        }
    }

    closePolicy: Popup.CloseOnEscape

    onAccepted: {
        root.save();
        destroy();
    }
    onRejected: {
        destroy();
    }

    header: PrecitecApplication.DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: root.title
    }

    ProductFilterModel {
        id: productFilterModel
    }

    SeamPropertyModel {
        id: propertyModel

        currentSeam: productController.currentSeam
        subGraphModel: root.subGraphModel
        filterAttributeModel: filterParameterEditor.filterAttributeModel
    }

    FilterParameterOnSeamConfigurationController {
        id: filterParameterController
        currentSeam: productController.currentSeam
    }

    Component {
        id: propertyComponent

        SeamPropertySelection {
            seamPropertyModel: propertyModel
            scanlabScannerAvailable: SystemConfiguration.Scanner2DEnable
            laserControlAvailable: SystemConfiguration.LaserControlEnable

            onEdit: {
                stackView.push(algorithmComponent);
            }
        }
    }

    Component {
        id: algorithmComponent

        ColumnLayout {
            CheckBox {
                id: copyAllBox

                text: qsTr("Copy entire Filter Parameter Set")
                checked: propertyModel.copyAllFilterParameters

                onToggled: {
                    propertyModel.copyAllFilterParameters = copyAllBox.checked;
                }
            }
            SubGraphFilterConfiguration {
                Layout.fillWidth: true
                Layout.fillHeight: true

                enabled: !propertyModel.copyAllFilterParameters
                graphModel: root.graphModel
                subGraphModel: root.subGraphModel
                currentSeam: productController.currentSeam
                showGraph: false
                editGraph: false
                exportSupported: false
                onlineHelp: root.onlineHelp

                onPreconfiguredGraphSelected: filterParameterEditor.pushPreConfiguredGraph()
                onSubGraphSelected: filterParameterEditor.pushSubGraph(uuid, name)
            }
        }
    }

    FilterParameterEditor {
        id: filterParameterEditor

        graphModel: root.graphModel
        subGraphModel: root.subGraphModel

        graphId: filterParameterController.currentGraphId
        view: stackView
        getFilterParameter: filterParameterController.getFilterParameter
        editParameters: false
    }

    footer: DialogButtonBox {
        alignment: Qt.AlignRight
        Button {
            text: qsTr("Copy")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            enabled: swipeView.currentIndex != 0
        }
        Button {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }
    }

    ColumnLayout {
        anchors.fill: parent

        SwipeView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: swipeView

            clip: true

            StackView {
                id: stackView
                initialItem: propertyComponent
                clip: true
            }

            ColumnLayout {

                TabBar {
                    id: bar
                    clip: true
                    Layout.fillWidth: true

                    onCountChanged: {
                        bar.setCurrentIndex(-1);
                        bar.setCurrentIndex(productFilterModel.mapFromSource(productController.index(productController.currentProductIndex(),0)).row);
                    }

                    Repeater {
                        model: productFilterModel

                        TabButton {
                            property bool changed: false

                            text: model.display + (changed ? " *" : "")
                            font.bold: bar.currentIndex == index
                            width: implicitContentWidth + 24
                        }
                    }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    currentIndex: bar.currentIndex

                    Repeater {
                        model: productFilterModel

                        SeamSelectionTab {
                            id: tab

                            Layout.fillWidth: true
                            Layout.fillHeight: true

                            product: model.product
                            font: root.font
                            productController: root.productController
                            currentSeam: root.productController.currentSeam
                            seamPropertyModel: propertyModel

                            onChanged: {
                                if (bar.currentItem)
                                {
                                    bar.currentItem.changed = true;
                                }
                            }
                            onMarkAsChanged: {
                                if (productController)
                                {
                                    productController.markProductAsChanged(model.product.uuid);
                                }
                            }

                            Connections {
                                target: root
                                function onSave() {
                                    tab.save()
                                }
                            }
                        }
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            ToolButton {
                display: AbstractButton.IconOnly
                icon.name: "arrow-left"
                enabled: swipeView.currentIndex != 0
                onClicked: swipeView.decrementCurrentIndex()
                background: Item {}
            }
            Item {
                Layout.fillWidth: true
            }
            PageIndicator {
                count: swipeView.count
                currentIndex: swipeView.currentIndex
            }
            Item {
                Layout.fillWidth: true
            }
            ToolButton {
                display: AbstractButton.IconOnly
                icon.name: "arrow-right"
                enabled: swipeView.currentIndex == 0
                onClicked: swipeView.incrementCurrentIndex()
                background: Item {}
            }
        }
    }
}
