import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Item {
    id: seamSeriesScanTracker2DPage
    property alias currentSeamSeries: hardwareParameterModel.seamSeries
    property alias attributeModel: hardwareParameterModel.attributeModel
    property alias title: hardwareParameters.title
    property alias filterKeys: hardwareParameters.filterKeys

    signal markAsChanged()

    HardwareParameterSeriesModel {
        id: hardwareParameterModel
        onMarkAsChanged: seamSeriesScanTracker2DPage.markAsChanged()
    }

    ScanTracker2DConfigurationItem {
        anchors {
            fill: parent
            margins: 0
        }
        id: hardwareParameters
        hardwareParametersModel: hardwareParameterModel
    }
}
