import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

import precitec.gui.filterparametereditor 1.0
import precitec.gui.general 1.0

Item {
    id: devicePageRoot
    property alias attributeModel: deviceModel.attributeModel
    property var deviceProxies: []
    property alias notificationServer: deviceModel.notificationServer
    onDeviceProxiesChanged: {
        if (deviceProxies.length != 0)
        {
            devicesTabBar.setProxy();
        }
    }
    signal restartRequired()

    function updateValue(row, value)
    {
        deviceModel.updateValue(deviceKeySortFilterModel.mapToSource(deviceKeySortFilterModel.index(row, 0)).row, value);
    }

    DeviceKeyModel {
        id: deviceModel
        onRestartRequired: devicePageRoot.restartRequired()
    }

    DeviceKeySortFilterModel {
        id: deviceKeySortFilterModel
        sourceModel: deviceModel
        searchText: searchField.text
    }

    DevicesTabModel {
        id: devicesTabModel
    }

    SecurityFilterModel {
        id: devicesModel
        sourceModel: devicesTabModel
        filterAvailable: true
    }

    ColumnLayout {
        anchors.fill: parent
        TabBar {
            id: devicesTabBar
            function setProxy()
            {
                if (tabBarRepeater.model == undefined || !visible)
                {
                    return;
                }
                var index = tabBarRepeater.model.mapToSource(tabBarRepeater.model.index(devicesTabBar.currentIndex, 0)).row
                if (index < deviceProxies.length && index != -1)
                {
                    deviceModel.deviceProxy = deviceProxies[index];
                }
            }

            Repeater {
                id: tabBarRepeater

                TabButton {
                    property bool canEdit: model.canEdit
                    text: model.display
                }
            }
            onCurrentIndexChanged: {
                devicesTabBar.setProxy();
            }
            onVisibleChanged: {
                if (visible)
                {
                    tabBarRepeater.model = devicesModel;
                } else
                {
                    tabBarRepeater.model = undefined
                }
            }

            Component.onCompleted: {
                if (visible)
                {
                    tabBarRepeater.model = devicesModel;
                }
            }

            Layout.fillWidth: true
        }
        Label {
            visible: deviceModel.changesRequireRestart
            text: qsTr("Changes on this device are only applied after a system reboot!")
            Layout.alignment: Qt.AlignHCenter
        }
        TextField {
            id: searchField
            placeholderText: qsTr("Filter")
            selectByMouse: true
            Layout.margins: 5
            Layout.fillWidth: true
        }
        ScrollView {
            ListView {
                clip: true
                spacing: 5
                anchors.fill: parent
                model: deviceKeySortFilterModel
                delegate: GroupBox {
                    id: delegate
                    title: model.display
                    width: ListView.view.width
                    enabled: devicesTabBar.currentItem ? devicesTabBar.currentItem.canEdit : false
                    ColumnLayout {
                        spacing: 5
                        anchors.fill: parent
                        Label {
                            text: model.comment
                            wrapMode: Text.Wrap
                            visible: text != ""
                            Layout.maximumWidth: delegate.ListView.view.width - parent.spacing * 2
                        }
                        RowLayout {
                            visible: model.attribute.type != Parameter.Boolean
                            Label {
                                text: qsTr("Current value:")
                            }
                            Label {
                                readonly property string unit: attribute ? LanguageSupport.getStringWithFallback(attribute.unit, attribute.unit) : ""
                                text: model.attribute.type == Parameter.Enumeration ? model.attribute.fields()[model.attribute.convertFromValueToIndex(model.parameter.value)] : (unit != "" ? model.parameter.value + " " + unit : model.parameter.value)
                            }
                        }
                        Label {
                            visible: model.attribute.type != Parameter.Boolean && !model.attribute.readOnly
                            text: qsTr("New value:")
                        }
                        ParameterEditor {
                            attribute: model.attribute
                            parameter: model.parameter
                            defaultValue: model.attribute.defaultValue

                            onParameterValueModified: devicePageRoot.updateValue(index, parameterValue)
                            Layout.fillWidth: true
                        }
                        Button {
                            text: qsTr("Reset to default")
                            display: Button.TextOnly
                            visible: !model.attribute.readOnly
                            ToolTip.text: model.attribute.defaultValue
                            ToolTip.visible: hovered
                            onClicked: {
                                devicePageRoot.updateValue(index, model.attribute.defaultValue);
                            }
                        }
                    }
                }

                BusyIndicator {
                    anchors.centerIn: parent
                    running: deviceModel.loading
                }
            }

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
