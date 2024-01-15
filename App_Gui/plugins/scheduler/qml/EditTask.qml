import QtQuick 2.14
import QtQuick.Controls 2.8
import QtQuick.Layouts 1.4

import precitec.gui.components.removableDevices 1.0
import precitec.gui.components.application 1.0

BackButtonGroupBox {
    id: root
    property alias trigger: triggerLoader.source
    property alias task: taskLoader.source
    property var triggerSettings
    property var taskSettings
    property var controller: null
    property int index: -1

    //: title of a dialog
    title: qsTr("Edit scheduled task")

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
            Loader {
                id: triggerLoader
                onLoaded: {
                    if (item.settings !== undefined)
                    {
                        item.settings = Qt.binding(function() { return root.triggerSettings; })
                    }
                }
                Layout.fillWidth: true
                Layout.preferredHeight: item ? item.implicitHeight: 0
            }
            Loader {
                id: taskLoader
                onLoaded: {
                    if (item.settings !== undefined)
                    {
                        item.settings = Qt.binding(function() { return root.taskSettings; })
                    }
                }
                Layout.fillWidth: true
                Layout.preferredHeight: item ? item.implicitHeight: 0
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
                    var trigggerSettings = root.triggerSettings;
                    if (triggerLoader.item && triggerLoader.item.save !== undefined)
                    {
                        triggerSettings = triggerLoader.item.save();
                    }
                    var taskSettings = root.taskSettings;
                    if (taskLoader.item && taskLoader.item.save !== undefined)
                    {
                        taskSettings = taskLoader.item.save();
                    }
                    controller.edit(root.index, triggerSettings, taskSettings);
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
