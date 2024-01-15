import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import precitec.gui.filterparametereditor 1.0

BackButtonGroupBox {
    id: filterGroupGroupBox
    property alias model: listView.model
    property string subGraphName

    signal groupSelected(int index, string groupName)

    title: subGraphName == "" ? qsTr("Filter Groups") : qsTr("Filter Groups of %1").arg(filterGroupGroupBox.subGraphName)
    ScrollView {
        anchors.fill: parent
        ListView {
            id: listView
            anchors.fill: parent
            delegate: ImplicitWidthDelegate {
                id: delegate
                implicitWidth: label.implicitWidth
                ItemDelegate {
                    id: label
                    width: parent.width
                    text: model.name
                    onClicked: {
                        filterGroupGroupBox.groupSelected(model.number, model.name)
                    }
                }
            }
        }
    }
}
