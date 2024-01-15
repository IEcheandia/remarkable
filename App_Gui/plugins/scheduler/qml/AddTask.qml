import QtQuick 2.14
import QtQuick.Controls 2.8
import QtQuick.Layouts 1.4

import precitec.gui.components.removableDevices 1.0
import precitec.gui.components.scheduler 1.0
import precitec.gui.components.application 1.0

BackButtonGroupBox {
    id: root
    //: title of a dialog
    title: qsTr("Add new scheduled task")
    property alias availableTasksModel: tasksCombo.model
    property alias availableTriggersModel: filteredTriggerModel.sourceModel
    property var controller: null

    TriggerFilterByTaskModel {
        id: filteredTriggerModel
        property int workaround: 0
        taskIdentifier: availableTasksModel.data(availableTasksModel.index(tasksCombo.currentIndex, 0), Qt.UserRole)
        onLayoutChanged: function() { filteredTriggerModel.workaround++; }
    }

    ActiveFocusControlAwareFlickable {
        anchors.fill: parent

        clip: true
        contentHeight: layout.implicitHeight
        contentWidth: Math.max(width - ScrollBar.vertical.width, layout.implicitWidth)
        interactive: contentHeight > height

        ScrollBar.vertical: ScrollBar{}

        ColumnLayout {
            id: layout
            anchors.fill: parent

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                Label {
                    //: Title for a ComboBox
                    text: qsTr("Task:")
                    Layout.alignment: Qt.AlignRight
                }
                ComboBox {
                    id: tasksCombo
                    objectName: "scheduler-add-task-task-combo"
                    textRole: "name"
                    Layout.fillWidth: true
                }
                Label {
                    //: Title for a ComboBox
                    text: qsTr("Trigger:")
                    Layout.alignment: Qt.AlignRight
                }
                ComboBox {
                    id: triggersCombo
                    objectName: "scheduler-add-task-trigger-combo"
                    textRole: "name"
                    model: filteredTriggerModel
                    Layout.fillWidth: true
                }
            }
            Loader {
                id: triggerLoader
                Layout.fillWidth: true
                Layout.preferredHeight: item ? item.implicitHeight: 0
                source: filteredTriggerModel.workaround, filteredTriggerModel.data(filteredTriggerModel.index(triggersCombo.currentIndex, 0), Qt.UserRole + 1)
            }
            Loader {
                id: taskLoader
                Layout.fillWidth: true
                Layout.preferredHeight: item ? item.implicitHeight: 0
                source: availableTasksModel.data(availableTasksModel.index(tasksCombo.currentIndex, 0), Qt.UserRole + 1)
            }
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            DialogButtonBox {
                id: buttonBox
                position: DialogButtonBox.Footer
                standardButtons: DialogButtonBox.Save | DialogButtonBox.Cancel

                onAccepted: {
                    var triggerSettings = [];
                    if (triggerLoader.item && triggerLoader.item.save !== undefined)
                    {
                        triggerSettings = triggerLoader.item.save();
                    }
                    var taskSettings = [];
                    if (taskLoader.item && taskLoader.item.save !== undefined)
                    {
                        taskSettings = taskLoader.item.save();
                    }
                    var trigger = filteredTriggerModel.data(filteredTriggerModel.index(triggersCombo.currentIndex, 0), Qt.UserRole);
                    var triggerEvent = filteredTriggerModel.data(filteredTriggerModel.index(triggersCombo.currentIndex, 0), Qt.UserRole + 2);
                    var task = availableTasksModel.data(availableTasksModel.index(tasksCombo.currentIndex, 0), Qt.UserRole);
                    controller.add(trigger, triggerSettings, triggerEvent, task, taskSettings);
                    root.back();
                }
                onRejected: root.back()

                Component.onCompleted: {
                    const button = buttonBox.standardButton(DialogButtonBox.Save);
                    button.enabled = Qt.binding(() => taskLoader.item && ((taskLoader.item.acceptableInput !== undefined && taskLoader.item.acceptableInput) || taskLoader.item.acceptableInput === undefined));
                }
            }
        }
    }
}
