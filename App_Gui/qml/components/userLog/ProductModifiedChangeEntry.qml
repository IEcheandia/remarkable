import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

ColumnLayout {
    id: rootItem
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
        title: qsTr("Parameter Set (%1)").arg(parameterSet ? parameterSet.uuid : "")
        modal: true
        onAboutToShow: {
            width = ApplicationWindow.contentItem.width * 0.9;
            height = ApplicationWindow.contentItem.height * 0.9;
            y = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).y + ApplicationWindow.contentItem.height * 0.05;
            x = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).x + ApplicationWindow.contentItem.width * 0.05;
        }
        ScrollView {
            anchors.fill: parent
            ListView {
                model: parametersDetailsDialog.parameterSet ? parametersDetailsDialog.parameterSet.parameters : []
                spacing: 5
                clip: true
                delegate: GridLayout {
                    width: ListView.view.width
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
    Dialog {
        id: seamDialog
        property var seam: null
        standardButtons: Dialog.Close
        modal: true
        onAboutToShow: {
            width = ApplicationWindow.contentItem.width * 0.9;
            height = ApplicationWindow.contentItem.height * 0.9;
            y = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).y + ApplicationWindow.contentItem.height * 0.05;
            x = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).x + ApplicationWindow.contentItem.width * 0.05;
        }
        GridLayout {
            columns: 2
            Label {
                text: qsTr("Uuid:")
            }
            Label {
                text: seamDialog.seam ? seamDialog.seam.uuid : ""
                font.family: "monospace"
            }
            Label {
                text: qsTr("Name:")
            }
            Label {
                text: seamDialog.seam ? seamDialog.seam.name : ""
            }
            Label {
                text: qsTr("Number:")
            }
            Label {
                text: seamDialog.seam ? seamDialog.seam.number : ""
            }
            Label {
                text: qsTr("Trigger delta:")
            }
            Label {
                text: seamDialog.seam ? seamDialog.seam.triggerDelta : ""
            }
            Label {
                text: qsTr("Velocity:")
            }
            Label {
                text: seamDialog.seam ? seamDialog.seam.velocity : ""
            }
            Label {
                text: qsTr("Graph:")
            }
            Label {
                text: seamDialog.seam ? seamDialog.seam.firstSeamInterval.graph : ""
                font.family: "monospace"
            }
            Label {
                text: qsTr("Graph parameter set:")
            }
            Label {
                text: seamDialog.seam ? seamDialog.seam.firstSeamInterval.graphParamSet : ""
                font.family: "monospace"
            }
            Label {
                text: qsTr("Length:")
            }
            Label {
                text: seamDialog.seam ? seamDialog.seam.length : ""
            }
            Label {
                text: qsTr("Position in assembly image:")
            }
            Label {
                text: seamDialog.seam ? (seamDialog.seam.positionInAssemblyImage.x + ", " + seamDialog.seam.positionInAssemblyImage.y) : ""
            }
            Label {
                text: qsTr("Hardware parameters:")
            }
            RowLayout {
                property var hardwareParameters: seamDialog.seam ? seamDialog.seam.hardwareParameters : null
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

    Dialog {
        id: seamChangesDialog
        property alias model: seamChangesListView.model
        standardButtons: Dialog.Close
        title: qsTr("Changes on seam")
        modal: true
        onAboutToShow: {
            width = ApplicationWindow.contentItem.width * 0.9;
            height = ApplicationWindow.contentItem.height * 0.9;
            y = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).y + ApplicationWindow.contentItem.height * 0.05;
            x = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).x + ApplicationWindow.contentItem.width * 0.05;
        }
        ScrollView {
            anchors.fill: parent
            ListView {
                id: seamChangesListView
                delegate: delegateLoaderComponent
            }
        }
    }

    Dialog {
        id: hardwareParametersChangesDialog
        property alias model: hardwareParametersChangesListView.model
        standardButtons: Dialog.Close
        modal: true
        onAboutToShow: {
            width = ApplicationWindow.contentItem.width * 0.9;
            height = ApplicationWindow.contentItem.height * 0.9;
            y = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).y + ApplicationWindow.contentItem.height * 0.05;
            x = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).x + ApplicationWindow.contentItem.width * 0.05;
        }
        ScrollView {
            anchors.fill: parent
            ListView {
                id: hardwareParametersChangesListView
                delegate: delegateLoaderComponent
            }
        }
    }


    Dialog {
        id: parametersChangesDialog
        property var parameter: null
        property alias model: parametersChangesListView.model
        standardButtons: Dialog.Close
        modal: true
        title: qsTr("Changes on parameter %1").arg(parametersChangesDialog.parameter ? parametersChangesDialog.parameter.name : "")
        onAboutToShow: {
            width = ApplicationWindow.contentItem.width * 0.9;
            height = ApplicationWindow.contentItem.height * 0.9;
            y = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).y + ApplicationWindow.contentItem.height * 0.05;
            x = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).x + ApplicationWindow.contentItem.width * 0.05;
        }
        ColumnLayout {
            GridLayout {
                columns: 2
                Label {
                    text: qsTr("Name:")
                }
                Label {
                    text: parametersChangesDialog.parameter ? parametersChangesDialog.parameter.name : ""
                }
                Label {
                    text: qsTr("Uuid:")
                }
                Label {
                    text: parametersChangesDialog.parameter ? parametersChangesDialog.parameter.uuid : ""
                }
                Label {
                    text: qsTr("Filter Id:")
                }
                Label {
                    text: parametersChangesDialog.parameter ? parametersChangesDialog.parameter.filterId : ""
                }
                Label {
                    text: qsTr("Type Id:")
                }
                Label {
                    text: parametersChangesDialog.parameter ? parametersChangesDialog.parameter.typeId : ""
                }
                Label {
                    text: qsTr("Type:")
                }
                Label {
                    text: parametersChangesDialog.parameter ? parametersChangesDialog.parameter.type : ""
                }
                Label {
                    text: qsTr("Value:")
                }
                Label {
                    text: parametersChangesDialog.parameter ? parametersChangesDialog.parameter.value : ""
                }
            }
            GroupBox {
                title: qsTr("Changes")
                ScrollView {
                    ListView {
                        id: parametersChangesListView
                        delegate: delegateLoaderComponent
                    }
                }
            }
        }
    }

     Dialog {
        id: parameterCreatedDialog
        property var parameter: null
        standardButtons: Dialog.Close
        modal: true
        title: qsTr("New parameter")
        onAboutToShow: {
            width = ApplicationWindow.contentItem.width * 0.9;
            height = ApplicationWindow.contentItem.height * 0.9;
            y = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).y + ApplicationWindow.contentItem.height * 0.05;
            x = - rootItem.mapToItem(ApplicationWindow.contentItem, 0, 0).x + ApplicationWindow.contentItem.width * 0.05;
        }
        GridLayout {
            columns: 2
            Label {
                text: qsTr("Name:")
            }
            Label {
                text: parameterCreatedDialog.parameter ? parameterCreatedDialog.parameter.name : ""
            }
            Label {
                text: qsTr("Uuid:")
            }
            Label {
                text: parameterCreatedDialog.parameter ? parameterCreatedDialog.parameter.uuid : ""
            }
            Label {
                text: qsTr("Filter Id:")
            }
            Label {
                text: parameterCreatedDialog.parameter ? parameterCreatedDialog.parameter.filterId : ""
            }
            Label {
                text: qsTr("Type Id:")
            }
            Label {
                text: parameterCreatedDialog.parameter ? parameterCreatedDialog.parameter.typeId : ""
            }
            Label {
                text: qsTr("Type:")
            }
            Label {
                text: parameterCreatedDialog.parameter ? parameterCreatedDialog.parameter.type : ""
            }
            Label {
                text: qsTr("Value:")
            }
            Label {
                text: parameterCreatedDialog.parameter ? parameterCreatedDialog.parameter.value : ""
            }
        }
    }

    Component {
        id: propertyChangeComponent
        Label {
            property var data
            text: data != undefined ? qsTr("\"%1\" changed from \"%2\" to \"%3\"").arg(data.name).arg(data.oldValue).arg(data.newValue) : ""
        }
    }
    Component {
        id: filterParameterSetAddedComponent
        RowLayout {
            property var parameterSet: null
            Label {
                text: qsTr("Filter parameter set added")
            }
            ToolButton {
                icon.name: "view-list-details"
                display: AbstractButton.IconOnly
                onClicked: {
                    parametersDetailsDialog.parameterSet = parent.parameterSet;
                    parametersDetailsDialog.open();
                }
            }
        }
    }
    Component {
        id: hardwareParameterSetAddedComponent
        RowLayout {
            property var parameterSet: null
            Label {
                text: qsTr("Hardware parameter set added")
            }
            ToolButton {
                icon.name: "view-list-details"
                display: AbstractButton.IconOnly
                onClicked: {
                    parametersDetailsDialog.parameterSet = parent.parameterSet;
                    parametersDetailsDialog.open();
                }
            }
        }
    }
    Component {
        id: filterParameterSetRemovedComponent
        RowLayout {
            property var parameterSet: null
            Label {
                text: qsTr("Filter parameter set removed")
            }
            ToolButton {
                icon.name: "view-list-details"
                display: AbstractButton.IconOnly
                onClicked: {
                    parametersDetailsDialog.parameterSet = parent.parameterSet;
                    parametersDetailsDialog.open();
                }
            }
        }
    }
    Component {
        id: seamRemovedComponent
        RowLayout {
            property var seam: null
            Label {
                text: qsTr("Seam removed")
            }
            ToolButton {
                icon.name: "view-list-details"
                display: AbstractButton.IconOnly
                enabled: parent.seam != null
                onClicked: {
                    seamDialog.seam = parent.seam;
                    seamDialog.open()
                }
            }
        }
    }
    Component {
        id: seamCreatedComponent
        RowLayout {
            property var seam: null
            Label {
                text: qsTr("Seam created")
            }
            ToolButton {
                icon.name: "view-list-details"
                display: AbstractButton.IconOnly
                enabled: parent.seam != null
                onClicked: {
                    seamDialog.seam = parent.seam;
                    seamDialog.open()
                }

            }
        }
    }
    Component {
        id: hardwareParametersModifiedComponent
        RowLayout {
            property var changes: []
            Label {
                text: qsTr("Hardware parameters modified")
                Layout.fillWidth: true
            }
            ToolButton {
                icon.name: "view-list-details"
                display: AbstractButton.IconOnly
                onClicked: {
                    hardwareParametersChangesDialog.model = parent.changes;
                    hardwareParametersChangesDialog.title = qsTr("Changes on hardware parameters");
                    hardwareParametersChangesDialog.open();
                }
            }
        }
    }
    Component {
        id: parameterSetModifiedComponent
        RowLayout {
            property var changes: []
            property var uuid: ""
            Label {
                text: qsTr("Parameter set %1 modified").arg(uuid)
            }
            ToolButton {
                icon.name: "view-list-details"
                display: AbstractButton.IconOnly
                onClicked: {
                    hardwareParametersChangesDialog.title = qsTr("Changes on parameter set %1").arg(parent.uuid);
                    hardwareParametersChangesDialog.model = parent.changes;
                    hardwareParametersChangesDialog.open();
                }
            }
        }
    }
    Component {
        id: parameterModificationComponent
        RowLayout {
            property var parameter: null
            property var changes: []
            Label {
                text: qsTr("Parameter %1 modified").arg(parameter ? parameter.name : "");
                Layout.fillWidth: true
            }
            ToolButton {
                icon.name: "view-list-details"
                display: AbstractButton.IconOnly
                onClicked: {
                    parametersChangesDialog.model = parent.changes;
                    parametersChangesDialog.parameter = parent.parameter;
                    parametersChangesDialog.open();
                }
            }
        }
    }
    Component {
        id: parameterCreatedComponent
        RowLayout {
            property var parameter: null
            Label {
                text: qsTr("Parameter %1 created").arg(parameter ? parameter.name : "");
                Layout.fillWidth: true
            }
            ToolButton {
                icon.name: "view-list-details"
                display: AbstractButton.IconOnly
                onClicked: {
                    parameterCreatedDialog.parameter = parent.parameter;
                    parameterCreatedDialog.open();
                }
            }
        }
    }

    Component {
        id: delegateLoaderComponent
        Loader {
            id: delegateLoader
            width: ListView.view.width
            Component.onCompleted: {
                if (modelData.description == "PropertyChange")
                {
                    delegateLoader.sourceComponent = propertyChangeComponent;
                    delegateLoader.item.data = modelData;
                } else if (modelData.description == "FilterParameterSetAddedChange")
                {
                    delegateLoader.sourceComponent = filterParameterSetAddedComponent;
                    delegateLoader.item.parameterSet = modelData.parameterSet;
                } else if (modelData.description == "FilterParameterSetRemovedChange")
                {
                    delegateLoader.sourceComponent = filterParameterSetRemovedComponent;
                    delegateLoader.item.parameterSet = modelData.parameterSet;
                } else if (modelData.description == "HardwareParametersCreatedChange")
                {
                    delegateLoader.sourceComponent = hardwareParameterSetAddedComponent;
                    delegateLoader.item.parameterSet = modelData.parameterSet;
                } else if (modelData.description == "SeamCreatedChange")
                {
                    delegateLoader.sourceComponent = seamCreatedComponent;
                    delegateLoader.item.seam = modelData.seam;
                } else if (modelData.description == "SeamRemovedChange")
                {
                    delegateLoader.sourceComponent = seamRemovedComponent;
                    delegateLoader.item.seam = modelData.seam;
                } else if (modelData.description == "HardwareParametersModificationChange")
                {
                    delegateLoader.sourceComponent = hardwareParametersModifiedComponent;
                    delegateLoader.item.changes = modelData.changes;
                } else if (modelData.description == "ParameterModificationChange")
                {
                    delegateLoader.sourceComponent = parameterModificationComponent;
                    delegateLoader.item.changes = modelData.changes;
                    delegateLoader.item.parameter = modelData.parameter;
                } else if (modelData.description == "ParameterSetModificationChange")
                {
                    delegateLoader.sourceComponent = parameterSetModifiedComponent;
                    delegateLoader.item.changes = modelData.changes;
                    delegateLoader.item.uuid = modelData.uuid;
                } else if (modelData.description == "ParameterCreatedChange")
                {
                    delegateLoader.sourceComponent = parameterCreatedComponent;
                    delegateLoader.item.parameter = modelData.parameter;
                } else
                {
                    delegateLoader.sourceComponent = undefined;
                    console.log(modelData.description);
                }
            }
        }
    }
    GroupBox {
        Layout.fillWidth: true
        title: qsTr("Changed product")
        GridLayout {
            columns: 2
            Label {
                text: qsTr("Uuid:")
            }
            Label {
                text: change ? change.uuid : ""
            }
            Label {
                text: qsTr("Name:")
            }
            Label {
                text: change ? change.productName : ""
            }
            Label {
                text: qsTr("Type:")
            }
            Label {
                text: change ? change.productType : ""
            }
        }
    }
    GroupBox {
        Layout.fillWidth: true
        title: "Changes on Product level"
        visible: productListView.count > 0
        ScrollView {
            anchors.fill: parent
            ListView {
                id: productListView
                width: rootItem.width
                spacing: 5
                model: change ? change.productChanges : undefined
                delegate: delegateLoaderComponent
            }
        }
    }
    GroupBox {
        title: qsTr("Changes on Seam level")
        visible: seamListView.count > 0
        ScrollView {
            anchors.fill: parent
            ListView {
                id: seamListView
                spacing: 5
                anchors.fill: parent
                model: change ? change.seamChanges : undefined
                delegate: RowLayout {
                    Label {
                        text: qsTr("Changes on seam %1").arg(modelData.number)
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        icon.name: "view-list-details"
                        display: AbstractButton.IconOnly
                        onClicked: {
                            seamChangesDialog.model = modelData.changes;
                            seamChangesDialog.open();
                        }
                    }
                }
            }
        }
    }
}
