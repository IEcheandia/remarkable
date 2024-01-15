import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

/**
 * Control to visualize the SeamIntervals of a seam
 **/
Control {
    id: root

    property alias seam : controller.currentSeam
    property bool readOnly: false

    height: layout.implicitHeight + root.topPadding + root.bottomPadding
    visible: seam ? seam.length != 0 : false

    signal seamIntervalSelected(var uuid)
    signal markAsChanged()

    SeamIntervalsVisualizeController {
        id: controller
    }

    contentItem: ColumnLayout {
        id: layout

        ListView {
            Layout.fillWidth: true

            orientation: ListView.Horizontal
            model: controller
            height: 40

            delegate: Rectangle {
                color: controller.colorForLevel(model.interval.level)
                border.width: 1
                height: 40
                width: model.relativeLength * ListView.view.width

                MouseArea {
                    anchors.fill: parent
                    onClicked: root.seamIntervalSelected(model.uuid)
                }
            }
        }

        Row {
            Layout.fillWidth: true

            id: row

            Rectangle {
                id: originRect

                width: originItem.contentWidth + 8
                height: originItem.contentHeight + 4
                border.width: 1

                Text {
                    anchors {
                        fill: parent
                        leftMargin: 4
                        rightMargin: 4
                        topMargin: 2
                        bottomMargin: 2
                    }
                    id: originItem
                    text: "0"
                }
            }

            Repeater {
                id: repeater

                model: controller

                function widthOfRest(index)
                {
                    var result = 0;
                    for (var i = index; i < repeater.count; i++)
                    {
                        var item = repeater.itemAt(i)
                        result += item ? item.rectWidth + 1 : 0;

                    }
                    return result;
                }

                delegate: RowLayout {
                    property alias rectWidth: rect.width

                    id: delegateItem

                    spacing: 0

                    Item {
                        Layout.margins: 0
                        Layout.preferredWidth: Math.min(Math.max(1, model.relativeAccumulatedLength * row.width - (index == repeater.count - 1 ? 1 : 0.5) * rect.width - delegateItem.x), row.width - delegateItem.x - repeater.widthOfRest(index))
                    }

                    Rectangle {
                        Layout.preferredWidth: inputItem.contentWidth + 8
                        Layout.preferredHeight: inputItem.contentHeight + 4
                        Layout.margins: 0

                        id: rect

                        border {
                            width: inputItem.activeFocus && !root.readOnly ? 2 : 1
                            color: inputItem.activeFocus && !root.readOnly ? "blue" : "black"
                        }

                        TextInput {
                            anchors {
                                fill: parent
                                leftMargin: 4
                                rightMargin: 4
                                topMargin: 2
                                bottomMargin: 2
                            }

                            id: inputItem

                            text: model.accumulatedLength.toLocaleString(locale, 'f', 1)
                            readOnly: root.readOnly
                            color: acceptableInput ? "black" : "red"

                            validator: DoubleValidator {
                                bottom: 0.001 * model.minAccumulatedLength
                            }

                            onEditingFinished: {
                                model.accumulatedLength = Number.fromLocaleString(locale, text);
                                root.markAsChanged();
                            }
                        }
                    }
                }
            }
        }
    }
}

