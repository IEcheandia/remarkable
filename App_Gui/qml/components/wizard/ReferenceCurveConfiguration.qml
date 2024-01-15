import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

import precitec.gui.components.plotter 1.0
import Precitec.AppGui 1.0
import precitec.gui.general 1.0

GridLayout {
    property alias referenceCurve: curveController.referenceCurve
    property alias currentProduct: curveController.currentProduct
    property var resultsConfigModel: null
    property alias textFieldHeight: nameField.height

    id: root

    signal plotterSettingsUpdated();
    signal updateSettings();

    onUpdateSettings: plotter.updateSettings()

    columns: 3
    columnSpacing: 10

    ReferenceCurvesController {
        id: curveController
    }

    Label {
        text: qsTr("Name:")
    }

    TextField {
        Layout.fillWidth: true
        Layout.columnSpan: 2

        id: nameField

        selectByMouse: true
        text: referenceCurve ? referenceCurve.name : ""

        onEditingFinished: {
            if (referenceCurve)
            {
                referenceCurve.name = text;
            }
        }
    }

    Label {
        Layout.preferredHeight: textFieldHeight
        text: qsTr("Result Value:")
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        Layout.preferredHeight: textFieldHeight
        font.bold: true
        text: referenceCurve && resultsConfigModel ? resultsConfigModel.nameForResultType(referenceCurve.resultType) : ""
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        Layout.fillWidth: true
        Layout.preferredHeight: textFieldHeight
        text: referenceCurve ? ("(Enum: %1)").arg(referenceCurve.resultType) : ""
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        Layout.preferredHeight: textFieldHeight
        text: qsTr("Type:")
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        Layout.preferredHeight: textFieldHeight
        Layout.columnSpan: 2
        text: referenceCurve ? referenceCurve.referenceTypeString : ""
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        Layout.preferredHeight: textFieldHeight
        text: qsTr("Jitter Value:")
        verticalAlignment: Text.AlignVCenter
        visible: referenceCurve && referenceCurve.referenceType != ReferenceCurve.MinMax
    }

    Label {
        Layout.preferredHeight: textFieldHeight
        Layout.columnSpan: 2
        text: referenceCurve ? Number(referenceCurve.jitter).toLocaleString(locale, 'f', 2) : ""
        verticalAlignment: Text.AlignVCenter
        visible: referenceCurve && referenceCurve.referenceType != ReferenceCurve.MinMax
    }

    PlotterChart {
        Layout.columnSpan: 3
        Layout.fillWidth: true
        Layout.fillHeight: true

        id: plotter

        controller {
            configFilePath: GuiConfiguration.configFilePath
        }
        xAxisController {
            autoAdjustXAxis: true
        }
        yAxisController {
            autoAdjustYAxis: true
        }
        yLegendRightVisible: false
        xLegendUnitVisible: false
        yLeftLegendUnitVisible: false
        backgroundBorderColor: "white"

        onPlotterSettingsUpdated: root.plotterSettingsUpdated()

        Component.onCompleted: {
            plotter.plotter.addDataSet(curveController.lower);
            plotter.plotter.addDataSet(curveController.middle);
            plotter.plotter.addDataSet(curveController.upper);
        }
    }
}

