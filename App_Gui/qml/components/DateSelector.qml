import QtQuick 2.5
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

/**
 * Component to select a Date.
 * The selected date is provided by readonly property @link{date}. The initial date is the current date.
 **/
RowLayout {
    /**
     * The selected date.
     **/
    readonly property var date: new Date(yearSelector.value, monthSelector.currentIndex, daySelector.currentIndex + 1)
    Tumbler {
        id: daySelector
        property int day: -1
        property bool resettingModel: false
        visibleItemCount: 3
        model: initModel(yearSelector.value, monthSelector.currentIndex)
        onModelChanged: Qt.callLater(keepIndex)
        function keepIndex()
        {
            while (daySelector.day >= daySelector.count)
            {
                daySelector.day--;
            }
            daySelector.currentIndex = daySelector.day;
        }
        onCurrentIndexChanged: {
            if (daySelector.resettingModel)
            {
                daySelector.resettingModel = false;
                return;
            }
            day = currentIndex;
        }
        function initModel(year, month)
        {
            daySelector.resettingModel = true;
            var date = new Date(year, month, 31);
            if (date.getDate() === 31)
            {
                return [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31];
            }
            date = new Date(year, month, 30);
            if (date.getDate() === 30)
            {
                return [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30];
            }
            date = new Date(year, month, 29);
            if (date.getDate() === 29)
            {
                return [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29];
            }
            return [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28];
        }
        Component.onCompleted: {
            daySelector.currentIndex = new Date().getDate() - 1;
            daySelector.day = daySelector.currentIndex;
        }
    }
    Tumbler {
        id: monthSelector
        TextMetrics {
            id: metrics
            font: monthSelector.font
        }
        function init()
        {
            var width = 0;
            for (var i = 0; i < 12; i++)
            {
                metrics.text = monthSelector.model[i];
                width = Math.max(width, metrics.width);
            }
            monthSelector.implicitWidth = width + 2 * monthSelector.spacing;
            currentIndex = new Date().getMonth();
            monthSelector.positionViewAtIndex(monthSelector.currentIndex, Tumbler.Center);
        }
        model: [
            qsTr("January"),
            qsTr("February"),
            qsTr("March"),
            qsTr("April"),
            qsTr("May"),
            qsTr("June"),
            qsTr("July"),
            qsTr("August"),
            qsTr("September"),
            qsTr("October"),
            qsTr("November"),
            qsTr("December")
        ]
        visibleItemCount: 3
        Component.onCompleted: monthSelector.init()
    }
    SpinBox {
        id: yearSelector
        function init()
        {
            value = new Date().getFullYear();
        }
        from: 1970
        to: 3000
        textFromValue: function(value, locale) { return Number(value).toString(); }
        Component.onCompleted: {
            yearSelector.init();
            contentItem.selectByMouse = true;
        }
    }
}
