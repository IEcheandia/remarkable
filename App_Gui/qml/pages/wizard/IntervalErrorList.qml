import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

BreadCrumpGroupBox {
    id: intervalError

    property alias currentSeam: visulaizationItem.seam
    property alias attributeModel: levelsItem.attributeModel
    property alias resultsConfigModel: levelsItem.resultsConfigModel
    property alias errorConfigModel: levelsItem.errorConfigModel
    property alias currentProduct: levelsItem.currentProduct
    property alias qualityNorm: levelsItem.qualityNorm
    property alias souvisPreInspectionEnabled: levelsItem.souvisPreInspectionEnabled
    property alias screenshotTool: levelsItem.screenshotTool
    property alias graphModel: levelsItem.graphModel
    property alias subGraphModel: levelsItem.subGraphModel

    signal markAsChanged()
    signal plotterSettingsUpdated();
    signal updateSettings();

    seam: currentSeam
    seamSeries: seam ? seam.seamSeries : null
    product: seamSeries ? seamSeries.product : null
    title: qsTr("Seam Interval Errors")

    ColumnLayout {
        anchors.fill: parent
        SeamIntervalsVisualize {
            id: visulaizationItem
            readOnly: true
            Layout.fillWidth: true
        }
        SeamIntervalLevels {
            id: levelsItem
            Layout.fillHeight: true
            Layout.fillWidth: true
            onMarkAsChanged: intervalError.markAsChanged()
            onPlotterSettingsUpdated: intervalError.plotterSettingsUpdated()
            currentSeam: intervalError.currentSeam

            Connections {
                target: intervalError
                function onUpdateSettings() {
                    levelsItem.updateSettings()
                }
            }
        }
    }
}
