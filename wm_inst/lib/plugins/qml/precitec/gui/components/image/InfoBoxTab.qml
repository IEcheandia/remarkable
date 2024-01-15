import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Item {
    property alias texts: tabModel.data
    property alias filterInstances: tabModel.filterInstances

    InfoTabModel {
        id: tabModel
    }

    GridLayout {
        anchors.fill: parent
        columns: 3

        Repeater {
            model: tabModel

            Label {
                Layout.fillWidth: index % 3 == 0
                text: model.text
                font.bold: model.bold
                font.italic: model.italic
                color: index % 3 == 2 ? "black" : model.color
            }
        }

        Item {
            Layout.columnSpan: 3
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
