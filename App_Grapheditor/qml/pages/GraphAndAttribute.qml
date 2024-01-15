import QtQuick 2.7
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3

import grapheditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.components.notifications 1.0 as Notifications
import precitec.gui.general 1.0

Control {
    property var filterGraphViewer: null
    property var rootGraphVisualizer: null
    property var groupController: null
    property var attributeModel: null
    property var resultSettingModel: null
    property var sensorSettingModel: null
    property var errorConfigModel: null
    property var macroController: null

    property bool hidden: false

    property alias minimumWidth: rightHideButton.implicitWidth

    id: interfaceGraphAndAttribute

    contentItem: Item {

        ToolButton {
            anchors {
                top: parent.top
                left: parent.left
                bottom: interfaceGraphAndAttribute.hidden ? parent.bottom : undefined
                margins: 5
            }

            id: rightHideButton

            display: AbstractButton.IconOnly
            icon.name: "application-menu"
            icon.color: PrecitecApplication.Settings.alternateText

            onClicked: {
                interfaceGraphAndAttribute.hidden = !interfaceGraphAndAttribute.hidden;
                rightHideButton.height = rightHideButton.implicitHeight;
            }
        }

        ColumnLayout {
            anchors {
                fill: parent
                margins: 5
            }

            opacity: interfaceGraphAndAttribute.hidden ? 0 : 1
            enabled: interfaceGraphAndAttribute.hidden ? 0 : 1

            Behavior on opacity {
                NumberAnimation {}
            }

            TabBar
            {
                id: bar
                Layout.fillWidth: true
                Layout.leftMargin: rightHideButton.width + 5
                currentIndex: 0

                TabButton
                {
                    text: qsTr("Attributes");
                    contentItem: Text
                    {
                        text: parent.text
                        font: parent.font
                        color: parent.checked ? "black" : "white"
                    }
                    font.bold: true
                    background: Rectangle
                    {
                        color: parent.checked ? "white" : PrecitecApplication.Settings.alternateBackground
                        border.width: parent.check ? 0 : 1
                        border.color: "white"
                    }
                }
                TabButton
                {
                    text: qsTr("Description");
                    contentItem: Text
                    {
                        text: parent.text
                        font: parent.font
                        color: parent.checked ? "black" : "white"
                    }
                    font.bold: true
                    background: Rectangle
                    {
                        color: parent.checked ? "white" : PrecitecApplication.Settings.alternateBackground
                        border.width: parent.check ? 0 : 1
                        border.color: "white"
                    }
                }
            }
            StackLayout
            {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: bar.currentIndex

                AttributesComponent
                {
                    id: layoutAttributes
                    selectedNodesModel: interfaceGraphAndAttribute.filterGraphViewer ? interfaceGraphAndAttribute.filterGraphViewer.visualizedGraph.selectedNodesModel : null
                    selectedGroupsModel: interfaceGraphAndAttribute.filterGraphViewer ? interfaceGraphAndAttribute.filterGraphViewer.visualizedGraph.selectedGroupsModel : null
                    graphVisualizer: interfaceGraphAndAttribute.rootGraphVisualizer
                    groupController: interfaceGraphAndAttribute.groupController
                    attributeModel: interfaceGraphAndAttribute.attributeModel
                    resultModel: interfaceGraphAndAttribute.resultSettingModel
                    sensorModel: interfaceGraphAndAttribute.sensorSettingModel
                    errorConfigModel: interfaceGraphAndAttribute.errorConfigModel
                    macroController: interfaceGraphAndAttribute.macroController
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
                FilterDescription
                {
                    id: filterDescription
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    filterGraph: interfaceGraphAndAttribute.filterGraphViewer ? interfaceGraphAndAttribute.filterGraphViewer.visualizedGraph : null
                }
            }
        }
    }
}
