import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import precitec.gui.components.userManagement 1.0
import Precitec.AppGui 1.0
import precitec.gui 1.0

GroupBox {
    id: liveInformation
    property var systemStatus
    property alias serialNumber: serialNumberLabel.text
    property alias hasPreviousProductInstance: previousProductInstanceButton.enabled
    property alias hasNextProductInstance: nextProductInstanceButton.enabled
    title: qsTr("Live Information")
    signal quitSystemFault()
    signal previousProductInstanceSelected()
    signal nextProductInstanceSelected()

    GridLayout {
        anchors.fill: parent
        columns: 2
        Label {
            text: qsTr("Serial number:")
        }
        RowLayout {
            ToolButton {
                id: previousProductInstanceButton
                display: AbstractButton.IconOnly
                icon.name: "media-skip-backward"
                onClicked: liveInformation.previousProductInstanceSelected()
            }
            Label {
                id: serialNumberLabel
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }
            ToolButton {
                id: nextProductInstanceButton
                display: AbstractButton.IconOnly
                icon.name: "media-skip-forward"
                onClicked: liveInformation.nextProductInstanceSelected()
            }
        }
        Label {
            text: qsTr("Product:")
            visible: systemStatus.productValid
        }
        Label {
            text: systemStatus.productName
            visible: systemStatus.productValid
        }
        Label {
            text: qsTr("Measure task:")
            visible: systemStatus.productValid
        }
        Label {
            text: systemStatus.measureTaskName
            visible: systemStatus.productValid
        }
        Label {
            text: qsTr("Seam-series:")
            visible: systemStatus.productValid
        }
        Label {
            text: systemStatus.visualSeamSeries
            visible: systemStatus.productValid
        }
        Label {
            text: qsTr("Seam:")
            visible: systemStatus.productValid
        }
        Label {
            text: systemStatus.visualSeam
            visible: systemStatus.productValid
        }
        Label {
            text: qsTr("Graph:")
            visible: systemStatus.productValid
        }
        Label {
            text: systemStatus.graphName
            visible: systemStatus.productValid
        }

        Label {
            visible: !systemStatus.productValid
            text: qsTr("No product")
            Layout.columnSpan: 2
        }

        Label {
            text: qsTr("Simulation Status:")
        }
        ColumnLayout {
            Label {
                    function stateToText(state) {
                    switch (state) {
                    case SystemStatusServer.Normal:
                        return qsTr("Ready");
                    case SystemStatusServer.Live:
                        return qsTr("Live");
                    case SystemStatusServer.Automatic:
                        return qsTr("Automatic");
                    case SystemStatusServer.Calibration:
                        return qsTr("Calibration");
                    case SystemStatusServer.NotReady:
                        return qsTr("Not ready");
                    case SystemStatusServer.ProductTeachIn:
                        return qsTr("Product teach in");
                    case SystemStatusServer.EmergencyStop:
                        return qsTr("Emergency stop");
                    default:
                        return qsTr("Unknown");
                    }
                }
                text: stateToText(systemStatus.state)
            }
            Button {
                visible: systemStatus.state == SystemStatusServer.NotReady && UserManagement.currentUser && UserManagement.hasPermission(App.ResetSystemStatus)
                text: qsTr("Reset system error")
                onClicked: liveInformation.quitSystemFault()
            }
        }
    }
}
