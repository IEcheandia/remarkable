import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import precitec.gui.filterparametereditor 1.0
import precitec.gui.general 1.0

BackButtonGroupBox {
    id: filterInstanceGroupGroupBox
    property string groupName
    property alias model: listView.model
    title: groupName == "" ? qsTr("All Filters") : qsTr("Filters of\n%1").arg(groupName)

    signal instanceSelected(var uuid, string filterName, string filterId)

    ScrollView {
        anchors.fill: parent
        ListView {
            id: listView
            anchors.fill: parent
            clip: true
            delegate: ImplicitWidthDelegate {
                implicitWidth: label.implicitWidth
                ItemDelegate {
                    id: label
                    width: parent.width
                    text: model.display
                    icon.source: WeldmasterPaths.filterPictureDir + model.filterId + ".png"
                    icon.color: "transparent"
                    onClicked: {
                        filterInstanceGroupGroupBox.instanceSelected(model.uuid, model.display, model.filterId)
                    }
                }
            }
        }
    }
}
