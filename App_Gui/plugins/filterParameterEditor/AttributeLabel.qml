import QtQuick 2.5
import QtQuick.Controls 2.3

import Precitec.AppGui 1.0
import precitec.gui.general 1.0

Label {
    property var attribute: null
    id: label
    text: attribute ? LanguageSupport.getString(attribute.contentName) : ""
}
