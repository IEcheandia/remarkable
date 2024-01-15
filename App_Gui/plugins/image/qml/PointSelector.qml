import QtQuick 2.0
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3
import precitec.gui.components.application 1.0

CrossCursor {
    id: crossCursor
    opacity: enabled ? 1.0 : 0.5
    property bool updateBlocked: true
    property point selectedPoint: Qt.point(0, 0)
    property var image

    function mapToImage()
    {
        updateBlocked = true;
        var mapped = image.mapToPaintedImage(crossCursor.selectedPoint);
        xPosition = mapped.x;
        yPosition = mapped.y;
        updateBlocked = false;
    }

    function updatePoint()
    {
        if (updateBlocked)
        {
            return;
        }
        crossCursor.selectedPoint = image.mapFromPaintedImage(Qt.point(Math.round(crossCursor.xPosition), Math.round(crossCursor.yPosition)));
    }

    x: image.paintedRect.x
    y: image.paintedRect.y
    width: image.paintedRect.width
    height: image.paintedRect.height

    Component.onCompleted: crossCursor.mapToImage()
    onWidthChanged: crossCursor.mapToImage()
    onHeightChanged: crossCursor.mapToImage()

    onXPositionChanged: crossCursor.updatePoint()
    onYPositionChanged: crossCursor.updatePoint()
}
