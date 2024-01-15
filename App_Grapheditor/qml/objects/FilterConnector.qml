import QtQuick                   2.12
import QtQuick.Controls          2.1
import QtQuick.Controls.Material 2.1
import QtQuick.Layouts           1.3
import Qt.labs.platform          1.0    // ColorDialog
import grapheditor.components 1.0

import QuickQanava 2.0 as Qan

//Change name from FilterNodeDock.qml to FilterNodeDockPort, because Port is the connector between nodes.
//Use portDelegate: (see customdocks.qml in the samples)
//Dock can be used for Labeling the different ports, but PortItem has own labeling

FilterPort
{
    id: portItem
    width: 10
    height: 10

    Rectangle
    {
        anchors.fill: parent
        id: colorRect
        color: portItem.colorValue
        opacity: (!pipeController.connecting || portItem.matchingInPipeConnection) ? 1.0 : 0.1
        border.color: "black"
        border.width: 1
    }

    DnDConnector
    {
        id: dndConnector
        visible: false
        selectable: false
        clip: false
        antialiasing: true
        width: colorRect.width
        height: colorRect.height
        x: colorRect.x
        y: colorRect.y

        connectorItem: Rectangle
        {
            id: defaultConnectorItem
            parent: dndConnector
            anchors.fill: parent
            visible: false
            color: "red"
            z: 15
            radius: parent.width/2.
            smooth: true;
            antialiasing: true
        }

        connectorGraph: filterGraph     //FIXME find an other way
        edgeComponent: Component{Qan.Edge{
            color: pipeController.isPipeConnectionPossible(portItem, handler.endPoint) ? "black" : "red"
        }}
        sourceConnector: portItem
    }

    DragHandler
    {
        id: handler
        target: null
        //margin: 5     //Pixel

        property point startPoint: "0,0"
        property point endPoint: "0,0"

        onActiveChanged:
        {
            if (handler.active)
            {
                startPoint = centroid.position;
                dndConnector.visible = true;
                dndConnector.connectorPressed();
                pipeController.startFromFilter(portItem);
            }
            else
            {
                //Check if over an other connector and try to connect (Search by position in graphModelVisualizer
                pipeController.insertNewPipeVisual(portItem, endPoint);

                //Reset dndConnector
                dndConnector.visible = false;
                dndConnector.x = colorRect.x;
                dndConnector.y = colorRect.y;
            }
        }

        onTranslationChanged:
        {
            //Calculate a arrow which shows where the mouse button is
            endPoint.x = translation.x;
            endPoint.y = translation.y;

            dndConnector.updatePosition(endPoint, graphView.zoom)         //Calculates the zoom to the endPoint
        }
    }
}
