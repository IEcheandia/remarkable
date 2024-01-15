import QtQuick 2.12
import QtQuick.Controls 2.5

GroupBox {
    property alias searchFieldText: searchField.text

    //: title of a group box for a search input field
    title: qsTr("Search")

    TextField {
        id: searchField

        //: placeholder text for a search input field
        placeholderText: qsTr("Search …")
        width: parent.width
    }
}
