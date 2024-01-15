import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.application 1.0
import Precitec.AppGui 1.0
import precitec.gui.components.userManagement 1.0
import precitec.gui.general 1.0

Item {
    property var currentProduct: null
    property var currentSeamSeries: null
    property alias currentSeam: curveModel.currentSeam
    property var resultsConfigModel: null
    property var productController: null
    property var screenshotTool: null
    property var graphModel: null
    property var subGraphModel: null

    id: root

    signal markAsChanged()
    signal plotterSettingsUpdated();
    signal updateSettings();
    signal referenceCurveSelected(var component);
    signal referenceCurveEditorSelected(var component);
    signal back();

    ReferenceCurvesModel {
        id: curveModel
        onMarkAsChanged: root.markAsChanged()
    }

    Component {
        id: newReferenceCurveDialog

        NewReferenceCurveDialog {
            anchors.centerIn: parent

            width: 0.25 * parent.width
            height: 0.5 * parent.height

            currentSeam: root.currentSeam
            resultsConfigModel: root.resultsConfigModel
            productController: root.productController
            graphModel: root.graphModel
            subGraphModel: root.subGraphModel
            referenceCurveModel: curveModel
            configurationPage: referenceCurveConfiguration
            screenshotTool: root.screenshotTool
            parentItem: root

            onReferenceCurveSelected: root.referenceCurveSelected(component)
        }
    }

    Component {
        id: deleteReferenceCurveDialog

        DeleteReferenceCurveDialog {
            anchors.centerIn: parent

            referenceCurveModel: curveModel
        }
    }

    Component {
        id: discardChangesDialog

        Dialog {
            property var editor: null
            anchors.centerIn: parent

            modal: true
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape

            onAccepted: {
                editor.letBackPass = true;
                editor.back();
            }

            Label {
                anchors.fill: parent

                text: qsTr("Discard Changes?")
                font.bold: true
                horizontalAlignment: Text.AlignHCenter
            }

            Connections {
                target: UserManagement
                function onCurrentUserChanged() {
                    discardChangesDialog.reject()
                }
            }
        }
    }

    Component {
        id: referenceCurveConfiguration

        BackButtonGroupBox {
            property alias referenceCurve: configuration.referenceCurve

            id: referenceCurveConfigurationGroup

            title: qsTr("Details of Reference Curve \"%1\" of Seam \"%2\"").arg(referenceCurve ? referenceCurve.name : "").arg(root.currentSeam ? root.currentSeam.name : "")

            Connections {
                target: root
                function onCurrentProductChanged() {
                    if (referenceCurveConfigurationGroup.visible)
                    {
                        referenceCurveConfigurationGroup.back();
                    }
                }
            }

            Connections {
                target: referenceCurve
                function onMarkAsChanged() {
                    root.markAsChanged()
                }
            }

            ColumnLayout {
                anchors.fill: parent

                ReferenceCurveConfiguration {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    id: configuration

                    currentProduct: root.currentProduct
                    resultsConfigModel: root.resultsConfigModel

                    onPlotterSettingsUpdated: root.plotterSettingsUpdated()

                    Connections {
                        target: root
                        function onUpdateSettings() {
                            configuration.updateSettings()
                        }
                    }
                }

                RowLayout {
                    Layout.alignment: Qt.AlignHCenter

                    Button {
                        display: AbstractButton.TextBesideIcon
                        text: qsTr("Edit")
                        icon.name: "editor"
                        onClicked: {
                            root.referenceCurveEditorSelected(referenceCurveEditor.createObject(root, {"referenceCurve": configuration.referenceCurve}));
                        }
                    }

                    Button {
                        display: AbstractButton.TextBesideIcon
                        text: qsTr("Delete")
                        icon.name: "edit-delete"
                        enabled: configuration.referenceCurve ? !configuration.referenceCurve.used : false
                        onClicked: {
                            var dialog = deleteReferenceCurveDialog.createObject(Overlay.overlay, {"referenceCurve": configuration.referenceCurve});
                            dialog.open();
                        }
                    }
                }
            }
        }
    }

    Component {
        id: referenceCurveEditor

        BackButtonGroupBox {
            id: editorGroup
            property bool letBackPass: false
            property bool automaticPop: !editor.hasChanges || editorGroup.letBackPass
            property bool saveButtonsVisible: false
            property alias referenceCurve: editor.referenceCurve

            title: qsTr("Results of Seam \"%1\" of Product \"%2\"").arg(root.currentSeam ? root.currentSeam.name : "").arg(root.currentProduct ? root.currentProduct.name : "")
            navigationEnabled: !editor.loading && !editor.updating

            onBack: {
                if (editor.hasChanges && !editorGroup.letBackPass)
                {
                    var dialog = discardChangesDialog.createObject(Overlay.overlay, {"editor": editorGroup});
                    dialog.open();
                }
            }

            ReferenceCurveEditor {
                anchors.fill: parent

                id: editor

                currentProduct: root.currentProduct
                currentSeam: root.currentSeam
                resultsConfigModel: root.resultsConfigModel

                onPlotterSettingsUpdated: root.plotterSettingsUpdated()
                onBack: editorGroup.back()

                Connections {
                    target: root
                    function onUpdateSettings() {
                        editor.updateSettings()
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: referenceCurveList

            model: curveModel

            ScrollBar.vertical: ScrollBar {
                id: scrollBar
            }

            delegate: RowLayout {

                width: referenceCurveList.width

                ItemDelegate {
                    Layout.fillWidth: true
                    text: ("<b>%1</b> (%2)").arg(model.curve.name).arg(resultsConfigModel.nameForResultType(model.curve.resultType))
                    icon.name: "application-menu"
                    onClicked: {
                        root.referenceCurveSelected(referenceCurveConfiguration.createObject(root, {"referenceCurve": model.curve}));
                    }
                }

                ToolButton {
                    Layout.rightMargin: scrollBar.width
                    display: AbstractButton.IconOnly
                    icon.name: "edit-delete"
                    palette.button: "white"
                    enabled: !model.curve.used
                    onClicked: {
                        var dialog = deleteReferenceCurveDialog.createObject(Overlay.overlay, {"referenceCurve": model.curve});
                        dialog.popOnAccept = false;
                        dialog.open();
                    }
                }
            }
        }

        Button {
            Layout.alignment: Qt.AlignHCenter
            text: qsTr("Create new Reference Curve")
            display: AbstractButton.TextBesideIcon
            icon.name: "list-add"
            onClicked: {
                var dialog = newReferenceCurveDialog.createObject(Overlay.overlay);
                dialog.open();
            }
        }
    }

    Label {
        anchors.centerIn: parent
        visible: referenceCurveList.count == 0
        text: qsTr("No Reference Curves defined for this Seam")
    }

}

