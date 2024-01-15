import QtQuick 2.5

/**
 * Item supposed to be used as a delegate of a ListView.
 * The main feature is that it updates the implicitWidth of the ListView
 **/
Item {
    function updateImplicitWidth()
    {
        if (delegate.implicitWidth > delegate.ListView.view.implicitWidth)
        {
            delegate.ListView.view.implicitWidth = delegate.implicitWidth
        }
    }
    id: delegate
    width: ListView.view.width
    height: childrenRect.height
    onImplicitWidthChanged: updateImplicitWidth()
    Component.onCompleted: updateImplicitWidth()
}
