import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

ColumnLayout {
    id: root
    Dialog {
        id: parametersDetailsDialog
        // workaround for https://bugreports.qt.io/browse/QTBUG-72372
        footer: DialogButtonBox {
            alignment: Qt.AlignRight
            Button {
                text: qsTr("Close")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
        }

        property var parameterSet: null
        title: qsTr("Parameter Set")
        width: root.width * 0.8
        height: root.height * 0.8
        modal: true
        ScrollView {
            anchors.fill: parent
            ListView {
                model: parametersDetailsDialog.parameterSet ? parametersDetailsDialog.parameterSet.parameters : []
                spacing: 5
                clip: true
                delegate: GridLayout {
                    columns: 2
                    Label {
                        text: qsTr("Uuid:")
                    }
                    Label {
                        text: modelData.uuid
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("Name:")
                    }
                    Label {
                        text: modelData.name
                    }
                    Label {
                        text: qsTr("Type id:")
                    }
                    Label {
                        text: modelData.typeId
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("Filter id:")
                    }
                    Label {
                        text: modelData.filterId
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("Value")
                    }
                    Label {
                        text: modelData.value
                    }
                }
            }
        }
    }
    property var product: (change && change.product) ? change.product : null
    GridLayout {
        columns: 2
        Label {
            text: qsTr("Uuid:")
        }
        Label {
            text: product ? product.uuid : ""
            font.family: "monospace"
        }
        Label {
            text: qsTr("Name:")
        }
        Label {
            text: product ? product.name : ""
        }
        Label {
            text: qsTr("Type:")
        }
        Label {
            text: product ? product.type : ""
        }
        Label {
            text: qsTr("Length unit:")
        }
        Label {
            text: product ? (product.lengthUnit == 0 ? "mm" : "°") : ""
        }
        Label {
            text: qsTr("Assembly image:")
        }
        Label {
            text: product ? product.assemblyImage : ""
        }
        Label {
            text: qsTr("Hardware parameters:")
        }
        RowLayout {
            property var hardwareParameters: product ? product.hardwareParameters : null
            Label {
                text: parent.hardwareParameters ? parent.hardwareParameters.uuid : ""
            }
            ToolButton {
                enabled: parent.hardwareParameters != null
                icon.name: "view-list-details"
                onClicked: {
                    parametersDetailsDialog.parameterSet = parent.hardwareParameters;
                    parametersDetailsDialog.open();
                }
            }
        }
    }
    GroupBox {
        title: qsTr("Seams")
        ScrollView {
            ListView {
                model: product ? product.allSeams : []
                spacing: 5
                delegate: GridLayout {
                    columns: 2
                    Label {
                        text: qsTr("Uuid:")
                    }
                    Label {
                        text: modelData.uuid
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("Name:")
                    }
                    Label {
                        text: modelData.name
                    }
                    Label {
                        text: qsTr("Number:")
                    }
                    Label {
                        text: modelData.number
                    }
                    Label {
                        text: qsTr("Trigger delta:")
                    }
                    Label {
                        text: modelData.triggerDelta
                    }
                    Label {
                        text: qsTr("Velocity:")
                    }
                    Label {
                        text: modelData.velocity
                    }
                    Label {
                        text: qsTr("Graph:")
                    }
                    Label {
                        text: modelData.firstSeamInterval.graph
                        font.family: "monospace"
                    }
                    Label {
                        text: qsTr("Graph parameter set:")
                    }
                    RowLayout {
                        property var parameterSet: product.filterParameterSet(modelData.firstSeamInterval.graphParamSet)
                        Label {
                            text: modelData.firstSeamInterval.graphParamSet
                            font.family: "monospace"
                        }
                        ToolButton {
                            enabled: parent.parameterSet != null
                            icon.name: "view-list-details"
                            onClicked: {
                                parametersDetailsDialog.parameterSet = parent.parameterSet;
                                parametersDetailsDialog.open();
                            }
                        }
                    }
                    Label {
                        text: qsTr("Length:")
                    }
                    Label {
                        text: modelData.length
                    }
                    Label {
                        text: qsTr("Position in assembly image:")
                    }
                    Label {
                        text: modelData.positionInAssemblyImage.x + ", " + modelData.positionInAssemblyImage.y
                    }
                    Label {
                        text: qsTr("Hardware parameters:")
                    }
                    RowLayout {
                        property var hardwareParameters: modelData.hardwareParameters
                        Label {
                            text: parent.hardwareParameters ? parent.hardwareParameters.uuid : ""
                        }
                        ToolButton {
                            enabled: parent.hardwareParameters != null
                            icon.name: "view-list-details"
                            onClicked: {
                                parametersDetailsDialog.parameterSet = parent.hardwareParameters;
                                parametersDetailsDialog.open();
                            }
                        }
                    }
                }
            }
        }
    }
}
