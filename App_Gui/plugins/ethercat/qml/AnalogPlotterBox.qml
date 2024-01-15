import QtQuick 2.10
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.plotter 1.0
import precitec.gui.components.application 1.0

/**
 * GroupBox containing a Plotter which is setup in a way to support an Ethercat analog in channel.
 **/
GroupBox {
    id: root
    /**
     * The DataSet for this Plotter
     **/
    property var dataSet
    property int range: 0

    PlotterChart {
        id: plotter
        zoomEnabled: false
        panningEnabled: false
        restoreEnabled: false
        backgroundBorderColor: "white"
        yAxisController {
            yMin: -10
            yMax: 10
        }
        xAxisController {
            xRange: root.range
            leadingEdge: true
        }
        xUnit: "s"
        yUnit: "V"
        toolTipEnabled: false
        anchors.fill: parent

        Component.onCompleted: {
            root.dataSet.color = Settings.alternateBackground;
            plotter.plotter.addDataSet(root.dataSet);
        }
    }
}
