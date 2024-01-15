import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

ScrollView {
    property alias lineLaserModel: listView.model
    ListView {
        id: listView
        anchors.fill: parent
        interactive: false
        delegate: GroupBox {
            label: CheckBox {
                id: lineLaserGroup
                text: model.display
                checked: model.enabled
                onToggled: {
                    lineLaserModel.enable(index, checked);
                }
            }
            GridLayout {
                columns: 4
                enabled: lineLaserGroup.checked && !model.updating
                Label {
                    text: qsTr("%1 %").arg(slider.from)
                }
                Slider {
                    id: slider
                    Layout.columnSpan: 2
                    from: model.minimum
                    to: model.maximum
                    value: model.intensity
                    stepSize: 1.0
                    snapMode: Slider.SnapOnRelease
                    live: false
                    onValueChanged: {
                        if (parent.enabled)
                        {
                            lineLaserModel.setIntensity(index, value);
                        }
                    }
                }
                Label {
                    text: qsTr("%1 %").arg(slider.to)
                }
                SpinBox {
                    Layout.columnSpan: 2
                    from: slider.from
                    to: slider.to
                    stepSize: slider.stepSize
                    value: slider.value
                    editable: true
                    onValueModified: {
                        if (parent.enabled)
                        {
                            lineLaserModel.setIntensity(index, value);
                        }
                    }
                    Component.onCompleted: {
                        contentItem.selectByMouse = true;
                    }
                }
            }
        }
    }
}
