import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import Precitec.AppGui 1.0

Item {
    id: page
    property alias assemblyImagesDirectory: assemblyImageItem.assemblyImagesDirectory
    property alias resultsServer: assemblyImageModel.resultsServer

    AssemblyImageInspectionModel {
        id: assemblyImageModel
    }

    AssemblyImageItem {
        id: assemblyImageItem
        assemblyImage: assemblyImageModel.product ? assemblyImageModel.product.assemblyImage : ""

        model: assemblyImageModel

        anchors.fill: parent
    }
}
