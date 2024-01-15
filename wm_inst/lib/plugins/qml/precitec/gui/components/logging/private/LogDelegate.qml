import QtQuick 2.7
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3
import precitec.gui.components.logging 1.0
import "./" as Private

Rectangle {
    property int timeStampWidth: 0
    property int levelWidth: 0
    property int moduleWidth: 0

    id: root

    color: (model.level == LogModel.Error || model.level == LogModel.Fatal) ? "#ff6868" : (model.level == LogModel.Warning ? "#fdff7e" : "#f0f0f0")
    height: layout.implicitHeight
    RowLayout {
        anchors.fill: parent
        id: layout
        // TODO: get this in a global way
        spacing: 8
        Private.LogLabel {
            Layout.leftMargin: 2
            Layout.preferredWidth: root.timeStampWidth
            text: Qt.formatTime(model.dateTime, "hh:mm:ss.zzz")
        }
        Label {
            text: "|"
        }
        Private.LogLabel {
            Layout.preferredWidth: root.levelWidth
            function levelToText(level)
            {
                switch (level)
                {
                case LogModel.Info:
                    return qsTr("Info");
                case LogModel.Warning:
                    return qsTr("Warning");
                case LogModel.Error:
                    return qsTr("Error");
                case LogModel.Fatal:
                    return qsTr("Fatal");
                case LogModel.Startup:
                    return qsTr("Startup");
                default:
                    return level;
                }
            }
            text: levelToText(model.level)
        }
        Label {
            text: "|"
        }
        Private.LogLabel {
            Layout.preferredWidth: root.moduleWidth
            text: model.module
        }
        Label {
            text: "|"
        }
        Private.LogLabel {
            Layout.fillWidth: true
            text: model.display
            elide: Text.ElideRight
        }
    }
}
