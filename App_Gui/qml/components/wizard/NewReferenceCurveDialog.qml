import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0
import precitec.gui.components.application 1.0
import precitec.gui.components.userManagement 1.0

Dialog {
    property alias currentSeam: availableResultsFilterModel.currentSeam
    property alias resultsConfigModel: availableResultsFilterModel.sourceModel
    property alias productController: productFilterModel.sourceModel
    property var referenceCurveModel: null
    property var configurationPage: null
    property var parentItem: null
    property alias screenshotTool: screenshotHeader.screenshotTool
    property alias graphModel: availableResultsFilterModel.graphModel
    property alias subGraphModel: availableResultsFilterModel.subGraphModel

    signal referenceCurveSelected(var component);

    id: root

    modal: true

    title: qsTr("Select Result Type")

    standardButtons: Dialog.Ok | Dialog.Cancel
    closePolicy: Popup.CloseOnEscape

    SeamErrorValueFilterModel {
        id: availableResultsFilterModel
        searchText: searchField.text
    }

    ProductFilterModel {
        id: productFilterModel
    }

    onAccepted: {
        if (referenceCurveModel && configurationPage && parentItem)
        {
            if (createNewCurve.checked)
            {
                root.referenceCurveSelected(configurationPage.createObject(parentItem, {"referenceCurve": referenceCurveModel.createReferenceCurve(resultTypeGroup.checkedButton.type)}));

            } else
            {
                root.referenceCurveSelected(configurationPage.createObject(parentItem, {"referenceCurve": referenceCurveModel.copyReferenceCurve(curveComboBox.model[curveComboBox.currentIndex])}));
            }
        }
    }

    header: DialogHeaderWithScreenshot {
        id: screenshotHeader
        title: root.title
    }

    ButtonGroup {
        id: resultTypeGroup
    }

    ColumnLayout {

        anchors.fill: parent

        RadioButton {
            id: copyFromExistingCurve
            text: qsTr("Copy Reference Curve")
        }

        ComboBox {
            Layout.fillWidth: true

            id: productComboBox

            visible: copyFromExistingCurve.checked
            model: productFilterModel
            textRole: "display"

            onVisibleChanged: {
                if (visible)
                {
                    currentIndex = root.currentSeam && root.currentSeam.product ? productComboBox.find(root.currentSeam.product.name) : -1;
                }
            }
        }

        ComboBox {
            property var product: productFilterModel.data(productFilterModel.index(productComboBox.currentIndex, 0), (Qt.UserRole + 1))

            Layout.fillWidth: true

            id: seamComboBox

            visible: copyFromExistingCurve.checked
            model: product ? product.allRealSeams : []

            displayText: model[currentIndex] ? ("%1: %2").arg(model[currentIndex].visualNumber).arg(model[currentIndex].name) : ""

            delegate: ItemDelegate {
                width: seamComboBox.width
                text: ("%1: %2").arg(modelData.visualNumber).arg(modelData.name)
                highlighted: seamComboBox.highlightedIndex === index
            }
        }

        ComboBox {
            Layout.fillWidth: true

            id: curveComboBox

            visible: copyFromExistingCurve.checked

            model: seamComboBox.currentIndex != -1 ? seamComboBox.product.allRealSeams[seamComboBox.currentIndex].referenceCurveList() : []

            displayText: model[currentIndex] ? ("%1 (%2)").arg(model[currentIndex].name).arg(resultsConfigModel ? resultsConfigModel.nameForResultType(model[currentIndex].resultType) : ("Enum %1").arg(modelData.resultType)) : ""

            delegate: ItemDelegate {
                width: seamComboBox.width
                text: ("%1 (%2)").arg(modelData.name).arg(resultsConfigModel ? resultsConfigModel.nameForResultType(modelData.resultType) : ("Enum %1").arg(modelData.resultType))
                highlighted: seamComboBox.highlightedIndex === index
            }
        }

        RadioButton {
            id: createNewCurve
            text: qsTr("Create Reference Curve")
            checked: true
        }

        TextField {
            Layout.fillWidth: true

            id: searchField

            visible: createNewCurve.checked
            selectByMouse: true
            placeholderText: qsTr("Search by Name or Enum")
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            id: availableResultsList

            visible: createNewCurve.checked

            model: availableResultsFilterModel
            clip: true

            ScrollBar.vertical: ScrollBar {
                id: scrollBar
            }

            delegate: RowLayout {
                width: availableResultsList.width

                RadioButton {
                    property var typeUuid: uuid
                    property int type: enumType
                    property string typeName: name

                    Layout.fillWidth: true
                    ButtonGroup.group: resultTypeGroup

                    text: name
                }
                Label {
                    Layout.rightMargin: scrollBar.width
                    text: qsTr("(Enum: %2)").arg(enumType)
                }
            }
        }

        Item {
            Layout.fillHeight: true

            visible: copyFromExistingCurve.checked
        }
    }

    Component.onCompleted: {
        root.standardButton(Dialog.Ok).enabled = Qt.binding(function () {
            return (createNewCurve.checked && resultTypeGroup.checkState != Qt.Unchecked) || (copyFromExistingCurve.checked && curveComboBox.currentIndex != -1);
        });
    }
    Connections {
        target: UserManagement
        function onCurrentUserChanged() {
            root.reject()
        }
    }
}
