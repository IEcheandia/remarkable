import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Item {
    id: productHardwarePage
    property alias currentProduct: hardwareParameterProductModel.product
    property alias attributeModel: hardwareParameterProductModel.attributeModel
    property alias title: hardwareParameters.title
    property alias filterKeys: hardwareParameters.filterKeys
    property alias showIdm: hardwareParameters.showIdm
    property alias screenshotTool: hardwareParameters.screenshotTool

    signal markAsChanged()

    HardwareParameterProductModel {
        id: hardwareParameterProductModel
        onMarkAsChanged: productHardwarePage.markAsChanged()
    }

    HardwareParametersItem {
        anchors {
            fill: parent
            margins: 0
        }
        id: hardwareParameters
        hardwareParametersModel: hardwareParameterProductModel
    }
}

