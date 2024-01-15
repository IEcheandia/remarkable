import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Item {
    id: seriesHardwarePage
    property alias currentSeamSeries: hardwareParameterSeriesModel.seamSeries
    property alias attributeModel: hardwareParameterSeriesModel.attributeModel
    property alias title: hardwareParameters.title
    property alias filterKeys: hardwareParameters.filterKeys
    property alias showIdm: hardwareParameters.showIdm
    property alias screenshotTool: hardwareParameters.screenshotTool

    signal markAsChanged()

    HardwareParameterSeriesModel {
        id: hardwareParameterSeriesModel
        onMarkAsChanged: seriesHardwarePage.markAsChanged()
    }

    HardwareParametersItem {
        anchors {
            fill: parent
            margins: 0
        }
        id: hardwareParameters
        hardwareParametersModel: hardwareParameterSeriesModel
    }
}

