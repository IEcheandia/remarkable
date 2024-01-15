import QtQuick 2.12
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.5

import grapheditor.components 1.0

import precitec.gui.components.application 1.0 as PrecitecApplication
import precitec.gui.general 1.0

Control {
    property var rootWindow: null
    property alias searchFieldText: filterComponent.searchFieldText
    property alias model: filterComponent.model

    property var graphEditorObject: null

    property alias filterGraphView: filterComponent.filterGraphView

    property bool hidden: false

    property alias minimumWidth: leftHideButton.implicitWidth

    property alias macroGraphModel: macroComponent.macroGraphModel

    property alias macroController: macroComponent.macroController

    property bool graphIsMacro: false

    property alias onlineHelp: macroComponent.onlineHelp

    id: interfaceInsertObjects

    contentItem: Item {

        ToolButton {
            anchors {
                top: parent.top
                right: parent.right
                margins: 5
            }

            id: leftHideButton

            display: AbstractButton.IconOnly
            icon.name: "application-menu"
            icon.color: PrecitecApplication.Settings.alternateText
            visible: interfaceInsertObjects.hidden

            onClicked: {
                interfaceInsertObjects.hidden = !interfaceInsertObjects.hidden;
                leftHideButton.height = leftHideButton.implicitHeight;
            }
        }
        ColumnLayout {
            visible: !interfaceInsertObjects.hidden
            opacity: interfaceInsertObjects.hidden ? 0 : 1
            Behavior on opacity {
                NumberAnimation {}
            }
            anchors.fill: parent
            RowLayout {
                Layout.fillWidth: true
                TabBar {
                    id: tabBar
                    visible: macroComponent.count > 0 && !interfaceInsertObjects.graphIsMacro
                    Layout.fillWidth: true
                    TabButton {
                        //: title of a tab button for all filters in GraphEditor
                        text: qsTr("Filter")
                    }
                    TabButton {
                        //: title of a tab button for all macros in GraphEditor
                        text: qsTr("Macros")
                    }
                }
                ToolButton {
                    display: AbstractButton.IconOnly
                    icon.name: "application-menu"
                    icon.color: PrecitecApplication.Settings.alternateText

                    onClicked: {
                        interfaceInsertObjects.hidden = !interfaceInsertObjects.hidden;
                        leftHideButton.height = leftHideButton.implicitHeight;
                    }
                }
            }
            FilterComponent {
                id: filterComponent
                visible: tabBar.currentIndex == 0 || interfaceInsertObjects.graphIsMacro
                onlineHelp: interfaceInsertObjects.onlineHelp

                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            MacroComponent {
                id: macroComponent
                visible: tabBar.currentIndex == 1 && !interfaceInsertObjects.graphIsMacro

                Layout.fillWidth: true
                Layout.fillHeight: true

            }
        }
    }
}
