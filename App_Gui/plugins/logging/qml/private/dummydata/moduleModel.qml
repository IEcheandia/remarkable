import QtQuick 2.3

ListModel {
    property string longestItem: "A Module with a very long name"
    ListElement {
        display: "All"
    }
    ListElement {
        display: "First Module"
    }
    ListElement {
        display: "Second Module"
    }
    ListElement {
        display: "A Module with a very long name"
    }
    ListElement {
        display: "Another Module"
    }
}
