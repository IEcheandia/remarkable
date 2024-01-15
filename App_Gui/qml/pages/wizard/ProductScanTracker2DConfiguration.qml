import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Item {
    id: productScanTracker2DPage
    property alias currentProduct: hardwareParameterModel.product
    property alias attributeModel: hardwareParameterModel.attributeModel
    property alias title: hardwareParameters.title
    property alias filterKeys: hardwareParameters.filterKeys

    signal markAsChanged()

    HardwareParameterProductModel {
        id: hardwareParameterModel
        onMarkAsChanged: productScanTracker2DPage.markAsChanged()
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
