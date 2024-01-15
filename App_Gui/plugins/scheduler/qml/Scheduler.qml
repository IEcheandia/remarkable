import QtQuick 2.14
import QtQuick.Controls 2.8
import QtQuick.Layouts 1.4

import precitec.gui.components.scheduler 1.0
import precitec.gui.general 1.0

StackView {
    id: root

    AvailableTasksModel {
        id: availableTasksModel
    }

    AvailableTriggersModel {
        id: availableTriggersModel
    }

    SchedulerModel {
        id: scheduledTasksModel
        configFile: WeldmasterPaths.configurationDir + "scheduler.json"
    }

    FontMetrics {
        id: fontMetrics
    }
    FontMetrics {
        id: headerFontMetrics
        font.bold: true
    }

    Component {
        id: deleteDialogComponent
        Dialog {
            id: deleteDialog
            property int row: -1
            parent: Overlay.overlay
            anchors.centerIn: parent
            modal: true
            //: Title for a dialog
            title: qsTr("Delete scheduler entry?")
            standardButtons: Dialog.Yes | Dialog.No
            closePolicy: Popup.CloseOnEscape

            onAccepted: {
                scheduledTasksModel.remove(deleteDialog.row);
                destroy();
            }
            onRejected: {
                destroy();
            }

            Label {
                //: Message text in a dialog
                text: qsTr("Do you really want to delete this scheduled task?")
            }
        }
    }

    initialItem: ColumnLayout {
        ListView {
            id: listView
            clip: true
            model: scheduledTasksModel

            Layout.fillWidth: true
            Layout.fillHeight: true

            //: title of a table column
            property string taskHeader: qsTr("Task")
            //: title of a table column
            property string triggerHeader: qsTr("Trigger")
            property real taskColumn: headerFontMetrics.advanceWidth(listView.taskHeader)
            property real triggerColumn: headerFontMetrics.advanceWidth(listView.triggerHeader)

            function calculatePreferredWidth() {
                listView.taskColumn = headerFontMetrics.advanceWidth(listView.taskHeader);
                listView.triggerColumn = headerFontMetrics.advanceWidth(listView.triggerHeader);

                for (var i = 0; i < availableTasksModel.rowCount(); i++)
                {
                    var index = availableTasksModel.index(i, 0);
                    listView.taskColumn = Math.max(listView.taskColumn, fontMetrics.advanceWidth(availableTasksModel.data(index, Qt.DisplayRole)));
                }
                for (var i = 0; i < availableTriggersModel.rowCount(); i++)
                {
                    var index = availableTriggersModel.index(i, 0);
                    listView.triggerColumn = Math.max(listView.taskColumn, fontMetrics.advanceWidth(availableTriggersModel.data(index, Qt.DisplayRole)));
                }
            }

            Component.onCompleted: calculatePreferredWidth()

            header: Control {
                padding: 10
                width: ListView.view.width
                contentItem: RowLayout {
                    spacing: 5
                    Item {
                        Layout.preferredWidth: 25
                    }
                    Label {
                        id: taskHeader
                        text: listView.taskHeader
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        Layout.preferredWidth: listView.taskColumn
                    }
                    Rectangle {
                        color: "black"
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true
                    }
                    Label {
                        id: triggerHeader
                        text: listView.triggerHeader
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        Layout.preferredWidth: listView.triggerColumn
                    }
                    Rectangle {
                        color: "black"
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true
                    }
                    Label {
                        //: title of a table column
                        text: qsTr("Parameters")
                        font.bold: true
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                }
            }
            delegate: ItemDelegate {
                property var triggerIndex: model.hasTriggerEvent ? availableTriggersModel.indexForIdentifier(model.triggerName, model.triggerEvent) : availableTriggersModel.indexForIdentifier(model.triggerName)
                property var taskIndex: availableTasksModel.indexForIdentifier(model.taskName)
                width: ListView.view.width
                objectName: "scheduler-listing-" + index
                icon.name: "application-menu"
                RowLayout {
                    anchors {
                        fill: parent
                        verticalCenter: parent.verticalCenter
                        leftMargin: 40
                    }
                    spacing: 5
                    Label {
                        text: availableTasksModel.data(taskIndex, Qt.DisplayRole)
                        Layout.preferredWidth: listView.taskColumn
                    }
                    Rectangle {
                        color: "black"
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true
                    }
                    Label {
                        text: availableTriggersModel.data(triggerIndex, Qt.DisplayRole)
                        Layout.preferredWidth: listView.triggerColumn
                    }
                    Rectangle {
                        color: "black"
                        Layout.preferredWidth: 1
                        Layout.fillHeight: true
                    }
                    Label {
                        text: model.parameters
                        Layout.fillWidth: true
                    }
                    ToolButton {
                        objectName: "scheduler-listing-" + index + "-delete"
                        icon.name: "edit-delete"
                        onClicked: deleteDialogComponent.createObject(root, {"row": index}).open()
                    }
                }

                onClicked: {
                    root.push(editElementComponent,
                              {
                                    "trigger": availableTriggersModel.data(triggerIndex, Qt.UserRole + 1),
                                    "task": availableTasksModel.data(taskIndex, Qt.UserRole + 1),
                                    "triggerSettings": model.triggerSettings,
                                    "taskSettings": model.taskSettings,
                                    "controller": scheduledTasksModel,
                                    "index": index
                            });
                }
            }
        }

        Button {
            objectName: "scheduler-add-task"
            icon.name: "list-add"
            //: Button to add a new scheduled task
            text: qsTr("Add scheduled task")
            onClicked: root.push(addNewElementComponent, {
                "availableTasksModel": availableTasksModel,
                "availableTriggersModel": availableTriggersModel,
                "controller": scheduledTasksModel
            })

            Layout.alignment: Qt.AlignHCenter
        }
    }

    Component {
        id: addNewElementComponent
        AddTask {
            onBack: root.pop()
        }
    }

    Component {
        id: editElementComponent
        EditTask {
            onBack: root.pop()
        }
    }
}
