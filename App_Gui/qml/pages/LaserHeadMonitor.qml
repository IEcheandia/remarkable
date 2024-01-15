import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.application 1.0
import precitec.gui.components.userManagement 1.0
import Precitec.AppGui 1.0
import precitec.gui 1.0

import precitec.laserHeadMonitor 1.0 as HeadMonitor

Control {
    id: root
    property alias laserHeadModel: laserHeadOverview.model

    Component {
        id: detailsComponent
        BackButtonGroupBox {
            property alias currentLaserHeadIndex: headDetailsPage.currentLaserHeadIndex
            onBack: stackView.pop()
            title: qsTr("Details of LaserHead %1").arg(headDetailsPage.currentLaserHead ? headDetailsPage.currentLaserHead.name : "")

            HeadMonitor.LaserHeadsDetailsModule {
                id: headDetailsPage
                anchors.fill: parent
                model: root.laserHeadModel
            }
        }
    }

    contentItem: StackView {
        id: stackView
        anchors.fill: parent

        initialItem: HeadMonitor.LaserHeadOverview {
            id: laserHeadOverview

            onClickedLaserHead: stackView.push(detailsComponent, {"currentLaserHeadIndex": index})
        }
    }
}
