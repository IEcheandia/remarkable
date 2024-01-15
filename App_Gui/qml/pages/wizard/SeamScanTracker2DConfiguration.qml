import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Item {
    id: seamScanTracker2DPage
    property alias currentSeam: hardwareParameterModel.seam
    property alias attributeModel: hardwareParameterModel.attributeModel
    property alias title: hardwareParameters.title
    property alias filterKeys: hardwareParameters.filterKeys

    signal markAsChanged()

    HardwareParameterSeamModel {
        id: hardwareParameterModel
        onMarkAsChanged: seamScanTracker2DPage.markAsChanged()
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
