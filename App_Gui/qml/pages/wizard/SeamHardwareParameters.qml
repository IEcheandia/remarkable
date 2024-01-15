import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Item {
    id: seamHardwarePage
    property alias currentSeam: hardwareParameterModel.seam
    property alias attributeModel: hardwareParameterModel.attributeModel
    property alias title: hardwareParameters.title
    property alias filterKeys: hardwareParameters.filterKeys
    property alias showIdm: hardwareParameters.showIdm
    property alias screenshotTool: hardwareParameters.screenshotTool

    signal markAsChanged()

    HardwareParameterSeamModel {
        id: hardwareParameterModel
        onMarkAsChanged: seamHardwarePage.markAsChanged()
    }

    HardwareParametersItem {
        anchors {
            fill: parent
            margins: 0
        }
        id: hardwareParameters
        hardwareParametersModel: hardwareParameterModel
    }
}

