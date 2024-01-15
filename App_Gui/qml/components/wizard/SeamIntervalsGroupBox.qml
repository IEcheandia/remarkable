import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.userManagement 1.0
import precitec.gui.components.application 1.0 as PrecitecApplication
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

GroupBox {
    id: seamIntervalGroupBox
    property alias seam: visualize.seam
    /**
     * The SeamInterval which should be configured
     **/
    property var seamInterval: null
    /**
     * Length unit description
     **/
    property string unit

    property var visualizeLeftMargin: 0
    property var visualizeRightMargin: 0
    property var resultsConfigModel: null
    property var errorConfigModel: null
    property var attributeModel: null
    property var qualityNorm: null
    property bool souvisPreInspectionEnabled: false
    property var screenshotTool: null
    property var graphModel: null
    property var subGraphModel: null

    property Component configurationItem: seamIntervalConfiguration
    property Component seamIntervalErrorItem: seamIntervalError

    signal seamIntervalSelected(var uuid, var component);
    signal deleteSeamIntervalSelected(var uuid, bool pop);
    signal seamIntervalErrorSelected(var component);
    signal markAsChanged();
    signal plotterSettingsUpdated();
    signal updateSettings();

    title: qsTr("Seam Intervals")
    bottomPadding: 0

    Component {
        id: seamIntervalError
        IntervalErrorList {
            id: intervalError
            bottomPadding : 0
            currentProduct: seamIntervalGroupBox.seam ? seamIntervalGroupBox.seam.seamSeries.product : null
            resultsConfigModel: seamIntervalGroupBox.resultsConfigModel
            errorConfigModel: seamIntervalGroupBox.errorConfigModel
            qualityNorm: seamIntervalGroupBox.qualityNorm
            attributeModel: seamIntervalGroupBox.attributeModel
            currentSeam: seamIntervalGroupBox.seam
            souvisPreInspectionEnabled: SystemConfiguration.SOUVIS6000_Is_PreInspection
            screenshotTool: seamIntervalGroupBox.screenshotTool
            graphModel: seamIntervalGroupBox.graphModel
            subGraphModel: seamIntervalGroupBox.subGraphModel

            onMarkAsChanged: seamIntervalGroupBox.markAsChanged()
            onPlotterSettingsUpdated: seamIntervalGroupBox.plotterSettingsUpdated()

            Connections {
                target: seamIntervalGroupBox
                function onUpdateSettings() {
                    intervalError.updateSettings()
                }
            }
        }
    }

    Component {
        id: seamIntervalConfiguration
        SeamIntervalConfiguration {
            seamInterval: seamIntervalGroupBox.seamInterval
            unit: seamIntervalGroupBox.unit
            onMarkAsChanged: seamIntervalGroupBox.markAsChanged()
            onDeleteSeamIntervalSelected: seamIntervalGroupBox.deleteSeamIntervalSelected(uuid, true)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        SeamIntervalsVisualize {
            id: visualize
            Layout.fillWidth: true
            Layout.leftMargin : seamIntervalGroupBox.visualizeLeftMargin
            Layout.rightMargin : seamIntervalGroupBox.visualizeRightMargin

            onSeamIntervalSelected: seamIntervalGroupBox.seamIntervalSelected(uuid, seamIntervalConfiguration)
            onMarkAsChanged: seamIntervalGroupBox.markAsChanged()
        }
        PrecitecApplication.ActiveFocusControlAwareFlickable {
            flickableDirection: Flickable.VerticalFlick
            id: awareFlickable
            implicitHeight: seamItervalListView.myHeight
            Layout.fillHeight: true
            Layout.fillWidth: true
            ColumnLayout {
                Item {
                    implicitHeight: seamItervalListView.myHeight
                    implicitWidth: awareFlickable.width
                    ListView {
                        anchors.fill: parent
                        id: seamItervalListView
                        property int myHeight: 0
                        clip: true
                        interactive: false
                        model: seamIntervalGroupBox.seam ? seamIntervalGroupBox.seam.allSeamIntervals : undefined
                        delegate: RowLayout {
                            width: ListView.view.width
                            ItemDelegate {
                                id: myListDelegate
                                text: (modelData.name != "" ? (" " + modelData.name) : "")  + "  Length [" +  seamIntervalGroupBox.unit + "]: " + Number(modelData.length?  modelData.length / 1000 : 0).toLocaleString(locale, 'f', 3)
                                Layout.fillWidth: true
                                icon.name: "application-menu"
                                onClicked: seamIntervalGroupBox.seamIntervalSelected(modelData.uuid, seamIntervalConfiguration)
                                Component.onCompleted: {
                                    seamItervalListView.myHeight += height + seamItervalListView.spacing;
                                }
                                Component.onDestruction: {
                                    seamItervalListView.myHeight -= height + seamItervalListView.spacing;
                                }
                            }
                            ToolButton {
                                display: AbstractButton.IconOnly
                                icon.name: "edit-delete"
                                palette.button: "white"
                                property int currentValue: seamIntervalGroupBox.seam ? seamIntervalGroupBox.seam.seamIntervalsCount : 0
                                enabled : currentValue <= 1 ? false : true
                                onClicked: seamIntervalGroupBox.deleteSeamIntervalSelected(modelData.uuid, false)
                            }
                        }
                    }
                }
            }
        }
    }
}
