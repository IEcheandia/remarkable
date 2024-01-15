import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

/**
 * Control to configure a SeamInterval
 **/
BreadCrumpGroupBox {
    id: seamIntervalConfiguration

    /**
     * The SeamInterval which should be configured
     **/
    property var seamInterval: null
    /**
     * Length unit description (mm or degree)
     **/
    property string unit

    /**
     * The name of the SeamInterval
     **/
    property alias name: nameField.text
    /**
     * The length of the SeamInterval in µm
     **/
    property int length: Number.fromLocaleString(locale, lengthField.text) * 1000

    signal markAsChanged()
    signal deleteSeamIntervalSelected(var uuid);

    seam: seamIntervalConfiguration.seamInterval ? seamIntervalConfiguration.seamInterval.seam : null
    seamSeries: seam ? seam.seamSeries : null
    product: seamSeries ? seamSeries.product : null
    title: qsTr("Details of seamInterval %1 (#%2)").arg(seamIntervalConfiguration.seamInterval ? seamIntervalConfiguration.seamInterval.name : "")
                                                   .arg(seamIntervalConfiguration.seamInterval ? seamIntervalConfiguration.seamInterval.visualNumber : -1)

    contentItem: ColumnLayout {
        GridLayout {
            Layout.fillWidth: true

            columns: 2
            Label {
                text: qsTr("Name:")
            }
            TextField {
                id: nameField
                selectByMouse: true
                text: seamIntervalConfiguration.seamInterval ? seamIntervalConfiguration.seamInterval.name : ""
                Layout.fillWidth: true
                onEditingFinished: {
                    if (seamInterval && seamInterval.name != text)
                    {
                        seamInterval.name = text;
                        seamIntervalConfiguration.markAsChanged();
                    }
                }
            }
            Label {
                text: qsTr("Length [%1]:").arg(seamIntervalConfiguration.unit)
            }
            TextField {
                id: lengthField
                selectByMouse: true
                validator: DoubleValidator {
                    bottom: 0
                }
                text: Number(seamIntervalConfiguration.seamInterval ? seamIntervalConfiguration.seamInterval.length / 1000 : 0).toLocaleString(locale, 'f', 3)
                onEditingFinished: {
                    if (seamInterval && seamInterval.length != seamIntervalConfiguration.length)
                    {
                        seamInterval.length = seamIntervalConfiguration.length;
                        seamIntervalConfiguration.markAsChanged();
                    }
                }
                palette.text: lengthField.acceptableInput ? "black" : "red"
            }
            Label {
                text: qsTr("Level assignment:")
            }
            RowLayout {
                spacing: 0
                Layout.preferredWidth: lengthField.width
                ComboBox {
                    id: seamIntervalLevelBox
                    Layout.preferredWidth: lengthField.width - colorBox.width
                    model: [qsTr("Level 1"), qsTr("Level 2"), qsTr("Level 3")]
                    currentIndex: seamIntervalConfiguration.seamInterval ? seamIntervalConfiguration.seamInterval.level : -1
                    onActivated: {
                        if (seamIntervalConfiguration.seamInterval && seamIntervalConfiguration.seamInterval.level != seamIntervalLevelBox.currentIndex)
                        {
                            seamIntervalConfiguration.seamInterval.level = seamIntervalLevelBox.currentIndex;
                            seamIntervalConfiguration.markAsChanged();
                        }
                    }
                }
                Rectangle {
                    id: colorBox
                    Layout.preferredWidth: 25
                    Layout.preferredHeight: seamIntervalLevelBox.implicitHeight
                    color: seamInterval ? seamInterval.color : "white"
                }
            }
        }
        Item {
            Layout.fillHeight: true
        }
        Button {
            Layout.alignment: Qt.AlignHCenter
            property int currentValue: seamIntervalConfiguration.seamInterval ? seamIntervalConfiguration.seamInterval.seam.seamIntervalsCount : 0
            enabled : currentValue <= 1 ? false : true
            display: AbstractButton.TextBesideIcon
            text: qsTr("Delete this Seam Interval")
            icon.name: "edit-delete"
            onClicked: {
                if (seamIntervalConfiguration.seamInterval)
                {
                    seamIntervalConfiguration.deleteSeamIntervalSelected(seamIntervalConfiguration.seamInterval.uuid);
                }
            }
        }
    }
}
