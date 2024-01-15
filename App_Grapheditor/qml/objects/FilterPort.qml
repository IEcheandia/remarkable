import QtQuick 2.12
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0

import grapheditor.components 1.0

import QuickQanava          2.0 as Qan
import "qrc:/QuickQanava"   as Qan

Qan.NodeItem
{
    id: filterPortItem
    Layout.preferredWidth: 50
    Layout.preferredHeight: 50
    width: Layout.preferredWidth
    height: Layout.preferredHeight
    Rectangle
    {
        id: filterPortBody
        anchors.fill: parent
        color: "transparent"

        Label
        {
            id: filterPortName
            anchors.left: filterPortBody.left
            anchors.right: filterPortBody.right
            anchors.top: filterPortBody.top
            height: filterPortName.font.pixelSize + 2
            text: filterPortItem.node.label
            font.pixelSize: 9
        }

        Rectangle
        {
            id: bodyRect
            anchors.top: filterPortName.bottom
            anchors.bottom: filterPortBody.bottom
            anchors.left: filterPortBody.left
            anchors.right: filterPortBody.right

            Rectangle
            {
                id: leftRect
                anchors.left: bodyRect.left
                anchors.top: bodyRect.top
                anchors.bottom: bodyRect.bottom

                width: filterPortBody.width/2.
                color: filterPortItem.node.rectColor

                Canvas
                {
                    id: leftInsideRect
                    anchors.fill: parent
                    onPaint:
                    {
                        var context = getContext("2d");
                        context.arc(0,0 + leftRect.height/2.,leftRect.height/2.5,1.5*Math.PI,0.5*Math.PI);
                        context.strokeStyle = filterPortItem.node.rectColor;
                        context.fillStyle = "white";
                        context.fill();
                        context.stroke();
                    }
                }
            }
            Rectangle
            {
                id: rightRect
                anchors.right: bodyRect.right
                anchors.top: bodyRect.top
                anchors.bottom: bodyRect.bottom
                width: filterPortBody.width/2.
                color: filterPortItem.node.rectColor

                Canvas
                {
                    id: rightInsideRect
                    anchors.fill: parent
                    onPaint:
                    {
                        var context = getContext("2d");
                        context.arc(0 + rightRect.width/2.,0 + rightRect.height/2.,rightRect.height/2.3,1.5*Math.PI,0.5*Math.PI);
                        context.strokeStyle = filterPortItem.node.rectColor;
                        context.fillStyle = filterPortItem.node.rectColor;
                        context.fill();
                        context.stroke();
                    }
                }

                DragHandler
                {
                    id: portHandler
                    target: null
                    //margin: 5     //Pixel

                    property point startPoint: "0,0"
                    property point endPoint: "0,0"
                    property size offset: Qt.size(filterPortBody.width/2., filterPortName.height)

                    onActiveChanged:
                    {
                        if (portHandler.active)
                        {
                            startPoint = centroid.position;
                            dndPortConnector.visible = true;
                            dndPortConnector.connectorPressed();
                            pipeController.startFromPort(filterPortItem.node);
                        }
                        else
                        {
                            //Check if over an other connector and try to connect (Search by position in graphModelVisualizer
                            pipeController.insertNewPipeVisualPort(filterPortItem.node, endPoint);

                            //Reset dndConnector
                            dndPortConnector.visible = false;
                            dndPortConnector.x = filterPortBody.x + offset.width;
                            dndPortConnector.y = filterPortBody.y + offset.height
                        }
                    }

                    onTranslationChanged:
                    {
                        //Calculate a arrow which shows where the mouse button is
                        endPoint.x = translation.x;
                        endPoint.y = translation.y;

                        dndPortConnector.updatePosition(endPoint, graphView.zoom, offset);
                    }
                }
            }
        }
    }

    DnDConnector
    {
        id: dndPortConnector
        visible: false
        selectable: false
        clip: false
        antialiasing: true
        x: filterPortBody.x + filterPortBody.width/2.
        y: filterPortBody.y + filterPortName.height
        width: rightRect.width
        height: rightRect.height

        connectorItem: Rectangle
        {
            id: defaultConnectorItem
            parent: dndPortConnector
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
            color: pipeController.isConnectionPossible(filterPortItem.node, portHandler.endPoint) ? "black" : "red"
        }}
        sourcePort: filterPortItem.node
    }

    Connections
    {
        target: filterPortItem.node
        function onTypeChanged()
        {
            if (filterPortItem.node.type === 3)
            {
                //Input filter port
                leftInsideRect.visible = false;
                rightRect.color = "transparent";
                leftRect.width = 0.75*leftRect.width;
                portHandler.enabled = false;
            }
            else
            {
                //Output filter port
                rightInsideRect.visible = false;
            }
        }
    }
    Connections
    {
        target: filterPortItem.node
        function onRectColorChanged()
        {
            if (filterPortItem.node.type === 3)
            {
                rightRect.color = "transparent";
                rightInsideRect.requestPaint();
            }
        }
    }
}

