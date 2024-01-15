import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import Precitec.AppGui 1.0

ColumnLayout {
    GroupBox {
        title: qsTr("Result")
        GridLayout {
            columns: 2
            Label {
                text: qsTr("Uuid:")
            }
            Label {
                text: (change && change.result) ? change.result.uuid : ""
            }
            Label {
                text: qsTr("Enum:")
            }
            Label {
                text: (change && change.result) ? change.result.enumType : ""
            }
            Label {
                text: qsTr("Name:")
            }
            Label {
                text: (change && change.result) ? change.result.name : ""
            }
            Label {
                text: qsTr("Plotter:")
            }
            Label {
                text: (change && change.result) ? change.result.plotterNumber : ""
            }
            Label {
                text: qsTr("Plottable:")
            }
            Label {
                text: (change && change.result) ? change.result.plottable : ""
            }
            Label {
                text: qsTr("Min:")
            }
            Label {
                text: (change && change.result) ? change.result.min : ""
            }
            Label {
                text: qsTr("Nax:")
            }
            Label {
                text: (change && change.result) ? change.result.max : ""
            }
            Label {
                text: qsTr("Color:")
            }
            Label {
                text: (change && change.result) ? change.result.lineColor : ""
                color: (change && change.result) ? change.result.lineColor : "black"
            }
            Label {
                text: qsTr("Visible:")
            }
            Label {
                text: (change && change.result) ? change.result.visibleItem : ""
            }
        }
    }
    GroupBox {
        title: qsTr("Changes")
        GridLayout {
            columns: 2
            Label {
                text: qsTr("Modified property:")
            }
            Label {
                function toText(enumValue)
                {
                    if (enumValue == ResultSetting.Uuid)
                    {
                        return qsTr("uuid");
                    } else if (enumValue == ResultSetting.EnumType)
                    {
                        return qsTr("enum");
                    } else if (enumValue == ResultSetting.Name)
                    {
                        return qsTr("name");
                    } else if (enumValue == ResultSetting.PlotterNumber)
                    {
                        return qsTr("plotter");
                    } else if (enumValue == ResultSetting.Plottable)
                    {
                        return qsTr("plottable");
                    } else if (enumValue == ResultSetting.Min)
                    {
                        return qsTr("min");
                    } else if (enumValue == ResultSetting.Max)
                    {
                        return qsTr("max");
                    } else if (enumValue == ResultSetting.LineColor)
                    {
                        return qsTr("color");
                    } else if (enumValue == ResultSetting.Visible)
                    {
                        return qsTr("visible");
                    }
                    return "";
                }
                text: change ? toText(change.change) : ""
            }
            Label {
                text: qsTr("Previous value:")
            }
            Label {
                text: change ? change.oldValue : ""
            }
            Label {
                text: qsTr("New value:")
            }
            Label {
                text: change ? change.newValue : ""
            }
        }
    }
}
