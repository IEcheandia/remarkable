import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

ScrollView {
    property alias fieldIlluminationModel: ledIlluminationListView.model
    ListView {
        id: ledIlluminationListView
        anchors.fill: parent
        delegate: GroupBox {
            label: CheckBox {
                id: fieldIlluminationChkBox
                text: model.display
                checked: model.enabled
                onToggled: {
                    fieldIlluminationModel.enable(index, checked);
                }
            }
            GridLayout {
                Layout.fillWidth: true
                columns: 4
                enabled: fieldIlluminationChkBox.checked
                Label {
                    text: qsTr("Intensity")
                    Layout.columnSpan: 4
                }
                Label {
                    text: qsTr("%1 %").arg(sliderIntensity.from)
                }
                Slider {
                    id: sliderIntensity
                    Layout.columnSpan: 2
                    from: model.minimumIntensity
                    to: model.maximumIntensity
                    value: model.intensity
                    stepSize: 1.0
                    snapMode: Slider.SnapOnRelease
                    live: false
                    onValueChanged: {
                        if (parent.enabled)
                        {
                            fieldIlluminationModel.setIntensity(index, value);
                        }
                    }
                }
                Label {
                    text: qsTr("%1 %").arg(sliderIntensity.to)
                }
                TextField {
                    Layout.columnSpan: 2
                    text: model.intensity
                    validator: IntValidator {
                        bottom: model.minimumIntensity
                        top: model.maximumIntensity
                    }
                    selectByMouse: true
                    onEditingFinished: {
                        if (parent.enabled)
                        {
                            fieldIlluminationModel.setIntensity(index, Number.fromLocaleString(locale, text));
                        }
                    }
                }
                Item {
                    Layout.columnSpan: 2
                }
                Label {
                    text: qsTr("Pulse width")
                    Layout.columnSpan: 4
                }
                Label {
                    text: qsTr("%1 us").arg(sliderPulsWidth.from)
                }
                Slider {
                    id: sliderPulsWidth
                    Layout.columnSpan: 2
                    from: model.minimumPulsWidth
                    to: model.maximumPulsWidth
                    value: model.pulsWidth
                    stepSize: 1.0
                    snapMode: Slider.SnapOnRelease
                    live: false
                    onValueChanged: {
                        if (parent.enabled)
                        {
                            fieldIlluminationModel.setPulseWidth(index, value);
                        }
                    }
                }
                Label {
                    text: qsTr("%1 us").arg(sliderPulsWidth.to)
                }
                TextField {
                    Layout.columnSpan: 2
                    text: model.pulsWidth
                    validator: DoubleValidator {
                        bottom: model.minimumPulsWidth
                        top:  model.maximumPulsWidth
                    }
                    selectByMouse: true
                    onEditingFinished: {
                        if (parent.enabled)
                        {
                            fieldIlluminationModel.setPulseWidth(index, Number.fromLocaleString(locale, text));
                        }
                    }
                }
            }
        }
    }
}
