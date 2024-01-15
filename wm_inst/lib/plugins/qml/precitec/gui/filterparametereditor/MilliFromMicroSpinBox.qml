import QtQuick 2.5
import QtQuick.Controls 2.3

import precitec.gui.components.application 1.0 as PrecitecApplication

/**
 * A SpinBox customization taking a micrometer value and converting it to millimeter as visualization.
 **/
SpinBox {
    textFromValue: function(text, locale) {
        return Number(value / 1000.0).toLocaleString(locale, 'f', 3) + " mm";
    }
    valueFromText: function(text, locale) {
        if (text.endsWith(" mm"))
        {
            text = text.slice(0, -3);
        }
        if (text.endsWith("mm"))
        {
            text = text.slice(0, -2);
        }
        return Number.fromLocaleString(locale, text) * 1000;
    }
    down.indicator: SpinBoxIndicator {
        x: 1
        iconName: "arrow-left"
        onClicked: {
            decrease();
            valueModified();
        }
    }
    up.indicator: SpinBoxIndicator {
        x: parent.width - width - 1
        iconName: "arrow-right"
        onClicked: {
            increase();
            valueModified();
        }
    }
    Component.onCompleted: {
        contentItem.selectByMouse = true;
    }
}
