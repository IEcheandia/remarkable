#include "pipeController.h"
#include "filterMacro.h"
#include "filterMacroConnector.h"
#include "GraphModelVisualizer.h"
#include "graphHelper.h"

#include <Poco/UUIDGenerator.h>

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

PipeController::PipeController(QObject *parent)
    : QObject(parent)
{
}

PipeController::~PipeController() = default;

void PipeController::setFilterGraph(FilterGraph* filterGraph)
{
    if (m_filterGraph == filterGraph)
    {
        return;
    }
    disconnect(m_filterGraphDestroyedConnection);
    m_filterGraph = filterGraph;
    if (m_filterGraph)
    {
        m_filterGraphDestroyedConnection = connect(m_filterGraph, &QObject::destroyed, this, std::bind(&PipeController::setFilterGraph, this, nullptr));
    }
    else
    {
        m_filterGraphDestroyedConnection = {};
    }
    emit filterGraphChanged();
}

void PipeController::insertNewPipe(qan::Node* senderNode, FilterConnector* senderConnector, qan::Node* receiverNode, FilterConnector* receiverConnector)
{
    /* There are 3 different cases:
     * 1. Edge between two filters with connectors
     * 2. Edge between filter and port
     * 3. Edge between port and filter
     */

    if (!canBeConnected(senderNode, senderConnector, receiverNode, receiverConnector))
    {
        return;
    }

    // Check if sender connector is output connector
    if (senderConnector && senderConnector->getType() != qan::PortItem::Type::Out)
    {
        std::swap(senderNode, receiverNode);
        std::swap(receiverConnector, senderConnector);
    }

    const auto newId = QUuid::createUuid();
    auto senderNodeAsFilter = qobject_cast<FilterNode*>(senderNode);
    auto senderNodeAsMacro = qobject_cast<FilterMacro*>(senderNode);
    auto senderNodeAsMacroConnector = qobject_cast<FilterMacroConnector*>(senderNode);
    auto destinationNodeAsFilter = qobject_cast<FilterNode*>(receiverNode);
    auto destinationNodeAsMacro = qobject_cast<FilterMacro*>(receiverNode);
    auto destinationNodeAsMacroConnector = qobject_cast<FilterMacroConnector*>(receiverNode);

    if ((senderNodeAsFilter || senderNodeAsMacro || senderNodeAsMacroConnector) && (destinationNodeAsFilter || destinationNodeAsMacro || destinationNodeAsMacroConnector))
    {
        //Case 1
        //QML: Insert the qml edge, which is shown in the graph editor.
        auto insertedEdge = m_filterGraph->insertEdge(senderNode, receiverNode);
        if (insertedEdge)
        {
            m_filterGraph->bindEdge(insertedEdge, senderConnector, receiverConnector);
            insertedEdge->setLabel(newId.toString());
            //Cpp: Insert the cpp edge, which is inserted in the xml file. Function of graphModelVisualizer
            insertNewPipe(insertedEdge, senderNode, senderConnector, receiverNode, receiverConnector);
            return;
        }
    }
    if ((senderNodeAsFilter || senderNodeAsMacro) && qobject_cast<FilterPort*> (receiverNode))
    {
        //Case 2
        //QML: Insert the qml edge, which is shown in the graph editor.
        auto insertedEdge = m_filterGraph->insertEdge(senderNode, receiverNode);
        if (insertedEdge)
        {
            m_filterGraph->bindEdgeSource(insertedEdge, senderConnector);
            insertedEdge->setLabel(newId.toString());
            auto receiverPort = qobject_cast<FilterPort*> (receiverNode);
            receiverPort->setDataType(senderConnector->connectorType());
            receiverPort->setUuid(UUIDs::senderFilterInstanceUUID, senderNodeAsFilter ? senderNodeAsFilter->ID() : senderNodeAsMacro->ID());
            receiverPort->setUuid(UUIDs::senderFilterConnectorUUID, senderConnector->ID());
            //Update data type of port partners
            if (receiverPort->gotPartner())
            {
                receiverPort->setDataTypeOfFilterPortPartner();
            }
            //Cpp: Update port IDs
            GraphHelper{m_graph}.updatePortIDs(receiverPort);
            receiverPort->checkIfThereIsAnotherConnection();    //FIXME need new structure
            //Cpp: No insertion of a pipe (cpp edge) because the receiver node is missing
            return;
        }
    }
    if (qobject_cast<FilterPort*> (senderNode) && (destinationNodeAsFilter || destinationNodeAsMacro))
    {
        //Case 3
        //QML: Insert the qml edge, which is shown in the graph editor.
        auto insertedEdge = m_filterGraph->insertEdge(senderNode, receiverNode);
        if (insertedEdge)
        {
            m_filterGraph->bindEdgeDestination(insertedEdge, receiverConnector);
            insertedEdge->setLabel(newId.toString());
            auto senderPort = qobject_cast<FilterPort*> (senderNode);
            senderPort->setUuid(UUIDs::receiverFilterInstanceUUID, destinationNodeAsFilter ? destinationNodeAsFilter->ID() : destinationNodeAsMacro->ID());
            senderPort->setUuid(UUIDs::receiverFilterConnectorUUID, receiverConnector->ID());
            //Cpp: Update port IDs
            GraphHelper{m_graph}.updatePortIDs(senderPort);
            //Cpp: Insert the cpp edge, which is inserted in the xml file. Function of graphModelVisualizer
            //Check first if the cpp edge is ready to create! (Double check?) TODO
            insertNewPipe(insertedEdge, senderNode, senderConnector, receiverNode, receiverConnector);
            return;
        }
    }
}

namespace
{
QRectF calculateTargetBoundaries(const QPointF &destinationPoint, qreal offset)
{
    return QRectF{destinationPoint.x() - offset, destinationPoint.y() - offset, destinationPoint.x() + offset, destinationPoint.y() + offset};
}
}

bool PipeController::isPipeConnectionPossible(FilterConnector* senderConnector, const QPoint &translation) const
{
    if (!senderConnector)
    {
        return false;
    }

    //Get start point from filter node
    auto senderFilter = senderConnector->getNode();
    if (!senderFilter)
    {
        return false;
    }

    const auto destinationPointGlobal = calculateDestinationPoint(senderConnector, translation);

    //Find in an area around the point with the width and height of 100 the target node or target port
    auto target = findTargetNode(destinationPointGlobal);

    if (!target)
    {
        return false;
    }

    FilterConnector *targetConnector = nullptr;
    targetConnector = findTargetConnector(qobject_cast<FilterNode*>(target), destinationPointGlobal);
    if (!targetConnector)
    {
        targetConnector = findTargetConnector(qobject_cast<FilterMacro*>(target), destinationPointGlobal);
    }
    if (!targetConnector)
    {
        targetConnector = findTargetConnector(qobject_cast<FilterMacroConnector*>(target), destinationPointGlobal);
    }
    if (targetConnector)
    {
        return canBeConnected(senderFilter, senderConnector, target, targetConnector);
    }

    if (dynamic_cast<FilterPort*>(target))
    {
        return true;
    }

    return false;
}

QPointF PipeController::calculateDestinationPoint(FilterConnector* senderConnector, const QPoint &translation) const
{
    QPointF endPosition = translation;
    endPosition.setX(endPosition.x()/m_zoom);
    endPosition.setY(endPosition.y()/m_zoom);

    //Get start point from filter node
    auto portGeometry = senderConnector->getItemGeometry();
    auto senderFilter = senderConnector->getNode();
    QPointF portItemGlobal {0.0,0.0};
    if (senderFilter->getGroup())
    {
        QPointF nodePoint = {senderFilter->getItem()->x(), senderFilter->getItem()->y()};
        nodePoint = senderFilter->getGroup()->getItem()->mapToItem(senderFilter->getGroup()->getGraph()->getContainerItem(), nodePoint);
        portItemGlobal = {nodePoint.x() + portGeometry.x() + (portGeometry.width()/2), nodePoint.y() + portGeometry.y() + (portGeometry.height()/2)};
    }
    else
    {
        portItemGlobal = {senderFilter->getItem()->x() + portGeometry.x() + (portGeometry.width()/2), senderFilter->getItem()->y() + portGeometry.y() + (portGeometry.height()/2)};
    }

    return portItemGlobal + endPosition;
}

void PipeController::insertNewPipeVisual(FilterConnector* senderConnector, const QPoint &translation)
{
    setConnecting(false);

    //Get start point from filter node
    auto senderFilter = senderConnector->getNode();
    auto destinationPointGlobal = calculateDestinationPoint(senderConnector, translation);
    //Find in an area around the point with the width and height of 100 the target node or target port
    auto target = findTargetNode(destinationPointGlobal);

    if (!target)
    {
        return;
    }

    FilterConnector *targetConnector = nullptr;
    targetConnector = findTargetConnector(qobject_cast<FilterNode*>(target), destinationPointGlobal);
    if (!targetConnector)
    {
        targetConnector = findTargetConnector(qobject_cast<FilterMacro*>(target), destinationPointGlobal);
    }
    if (!targetConnector)
    {
        targetConnector = findTargetConnector(qobject_cast<FilterMacroConnector*>(target), destinationPointGlobal);
    }
    if (targetConnector)
    {
        insertNewPipe(senderFilter, senderConnector, target, targetConnector);
        return;
    }

    if (auto portTarget = qobject_cast<FilterPort*>(target))
    {
        insertNewPipe(senderFilter, senderConnector, portTarget, {});
        return;
    }
}

QPointF PipeController::calculateDestinationPoint(FilterPort* senderPort, const QPoint &translation) const
{
    QPointF endPosition = translation;
    endPosition.setX(endPosition.x()/m_zoom);
    endPosition.setY(endPosition.y()/m_zoom);

    //Get start point from filter port
    QRectF senderGeometry = senderPort->getItemGeometry();
    QPointF senderGlobalPosition = {senderGeometry.x() + senderGeometry.width()*0.75, senderGeometry.y() + 15};
    if (senderPort->getGroup())
    {
        senderGlobalPosition = senderPort->getGroup()->getItem()->mapToItem(senderPort->getGroup()->getGraph()->getContainerItem(), senderGlobalPosition);
    }

    return senderGlobalPosition + endPosition;
}

bool PipeController::isConnectionPossible(FilterPort* senderPort, const QPoint &translation) const
{
    if (!senderPort || !senderPort->gotPartner())
    {
        return false;
    }

    const auto destinationPointGlobal = calculateDestinationPoint(senderPort, translation);

    //Find in an area around the point with the width and height of 100 the target node or target port
    auto filterTarget = findTargetNode(destinationPointGlobal);

    if (!filterTarget)
    {
        return false;
    }

    FilterConnector *targetConnector = nullptr;
    targetConnector = findTargetConnector(qobject_cast<FilterNode*>(filterTarget), destinationPointGlobal);
    if (!targetConnector)
    {
        targetConnector = findTargetConnector(qobject_cast<FilterMacro*>(filterTarget), destinationPointGlobal);
    }
    if (!targetConnector)
    {
        targetConnector = findTargetConnector(qobject_cast<FilterMacroConnector*>(filterTarget), destinationPointGlobal);
    }

    if (!targetConnector)
    {
        return false;
    }

    return canBeConnected(senderPort, nullptr, filterTarget, targetConnector);
}

void PipeController::insertNewPipeVisualPort(FilterPort* senderPort, const QPoint& translation)
{
    setConnecting(false);
    if (!senderPort->gotPartner())
    {
        return;
    }

    const auto destinationPointGlobal = calculateDestinationPoint(senderPort, translation);

    //Find in an area around the point with the width and height of 100 the target node or target port
    auto filterTarget = findTargetNode(destinationPointGlobal);

    if (!filterTarget)
    {
        return;
    }

    FilterConnector *targetConnector = nullptr;
    targetConnector = findTargetConnector(qobject_cast<FilterNode*>(filterTarget), destinationPointGlobal);
    if (!targetConnector)
    {
        targetConnector = findTargetConnector(qobject_cast<FilterMacro*>(filterTarget), destinationPointGlobal);
    }

    if (!targetConnector)
    {
        return;
    }

    insertNewPipe(senderPort, nullptr, filterTarget, targetConnector);
}

bool PipeController::canBeConnected(qan::Node* senderNode, FilterConnector* senderConnector, qan::Node* receiverNode, FilterConnector* receiverConnector) const
{
    // Check if edge is possible
    if (!senderNode || !receiverNode)
    {
        return false;
    }

    if (senderNode == receiverNode)
    {
        return false;
    }

    if (!senderConnector && !receiverConnector)
    {
        return false;
    }

    if (senderConnector)
    {
        //Check if sender connector is output connector
        if (static_cast<int>(senderConnector->getType()) != 2)
        {
            //Change sender and receiver
            auto changeNode = senderNode;
            senderNode = receiverNode;
            receiverNode = changeNode;
            auto changeConnector = senderConnector;
            senderConnector = receiverConnector;
            receiverConnector = changeConnector;

            if (!senderConnector)
            {
                //Sender is port so check if port data type is connector data type
                auto senderPort = dynamic_cast<FilterPort*> (senderNode);
                if (senderPort->type() == 3)
                {
                    return false;
                }
                if (receiverConnector)
                {
                    if (senderPort->dataType() != receiverConnector->connectorType())
                    {
                        return false;
                    }
                    if (!senderPort->gotPartner())
                    {
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
        }
    }

    if (receiverConnector)
    {
        //Check if connector is already connected
        if (receiverConnector->getInEdgeItems().size() != 0)
        {
            return false;
        }
        if (receiverConnector->getType() != qan::PortItem::Type::In)
        {
            return false;
        }
    }

    if (senderConnector && receiverConnector)
    {
        //Check if both connectors are the same
        if (senderConnector->ID() == receiverConnector->ID())
        {
            return false;
        }
        //Check if both connector has the same type
        if (senderConnector->connectorType() != receiverConnector->connectorType())
        {
            return false;
        }
        if (senderConnector->getType() == qan::PortItem::Type::In)
        {
            return false;
        }
    }

    if (senderConnector && !receiverConnector)
    {
        //Check if receiver (receiver has to be a port) has no other connections and because of that a type
        if (receiverNode->get_in_edges().size() != 0)
        {
            return false;
        }
        //Check if the port has the right type for this constellation
        if (auto port = qobject_cast<FilterPort*> (receiverNode); port && port->type() == 2)
        {
            return false;
        }
    }

    if (!senderConnector)
    {
        auto senderPort = qobject_cast<FilterPort*> (senderNode);
        if (receiverConnector && senderPort)
        {
            if (senderPort->dataType() != receiverConnector->connectorType())
            {
                return false;
            }
            if (!senderPort->gotPartner())
            {
                return false;
            }
            if (senderPort->getOneFilterPortPartner()->get_in_edges().size() != 1)
            {
                return false;
            }
        }
    }

    return true;
}

namespace
{
template <typename T>
void setMatchingInPipeConnection(qan::Node *node, bool matching)
{
    if (auto filterNode = dynamic_cast<T*>(node))
    {
        filterNode->setMatchingInPipeConnection(matching);
    }
}
}

void PipeController::startFromFilter(FilterConnector* senderConnector)
{
    for (const auto &nodePtr : qAsConst(m_filterGraph->get_nodes()))
    {
        bool matching{false};
        for ( const auto connector : qAsConst(nodePtr->getItem()->getPorts()))
        {
            const auto connectorItem = qobject_cast<FilterConnector*>(connector);
            if (!connectorItem)
            {
                continue;
            }
            if (connectorItem == senderConnector)
            {
                connectorItem->setMatchingInPipeConnection(true);
            }
            else
            {
                connectorItem->setMatchingInPipeConnection(canBeConnected(senderConnector->getNode(), senderConnector, nodePtr, connectorItem));
            }
            matching = matching || connectorItem->isMatchingInPipeConnection();
        }
        setMatchingInPipeConnection<FilterNode>(nodePtr, matching);
        setMatchingInPipeConnection<FilterMacro>(nodePtr, matching);
        setMatchingInPipeConnection<FilterMacroConnector>(nodePtr, matching);
    }
    setConnecting(true);
}

void PipeController::startFromPort(FilterPort *senderPort)
{
    for (const auto &nodePtr : qAsConst(m_filterGraph->get_nodes()))
    {
        bool matching{false};
        for ( const auto connector : qAsConst(nodePtr->getItem()->getPorts()))
        {
            const auto connectorItem = qobject_cast<FilterConnector*>(connector);
            if (!connectorItem)
            {
                continue;
            }
            connectorItem->setMatchingInPipeConnection(canBeConnected(senderPort, nullptr, nodePtr, connectorItem));
            matching = connectorItem->isMatchingInPipeConnection();
        }
        setMatchingInPipeConnection<FilterNode>(nodePtr, matching);
        setMatchingInPipeConnection<FilterMacro>(nodePtr, matching);
    }
    setConnecting(true);
}

void PipeController::setConnecting(bool set)
{
    if (m_connecting == set)
    {
        return;
    }
    m_connecting = set;
    emit connectingChanged();
}

template <typename T>
qan::Node* PipeController::findTargetNode(qan::Node *nodePtr, const QRectF &boundaries) const
{
    if (auto node = dynamic_cast<T*>(nodePtr))
    {
        auto size = node->getItemGeometry();
        if (node->getGroup())
        {
            QPointF points = node->getGroup()->getItem()->mapToItem(node->getGroup()->getGraph()->getContainerItem(), {size.x(),size.y()});
            size = {points.x(), points.y(), size.width(), size.height()};
        }
        //Check if in a are with -50, +50 from the point in width and height
        if (areasOverlap({boundaries.x(), boundaries.y()},{boundaries.width(), boundaries.height()},{size.x(), size.y()},{size.x() + size.width(), size.y() + size.height()}))
        {
            return node;
        }
    }
    return nullptr;
}

qan::Node* PipeController::findTargetNode(const QRectF &boundaries) const
{
    for (const auto &nodePtr : qAsConst(m_filterGraph->get_nodes()))
    {
        if (auto node = findTargetNode<FilterNode>(nodePtr, boundaries))
        {
            return node;
        }
        if (auto node = findTargetNode<FilterPort>(nodePtr, boundaries))
        {
            return node;
        }
        if (auto node = findTargetNode<FilterMacro>(nodePtr, boundaries))
        {
            return node;
        }
        if (auto node = findTargetNode<FilterMacroConnector>(nodePtr, boundaries))
        {
            return node;
        }
    }
    return nullptr;
}

qan::Node* PipeController::findTargetNode(const QPointF &destinationPoint) const
{
    return findTargetNode(calculateTargetBoundaries(destinationPoint, 25));
}

template <typename T>
FilterConnector* PipeController::findTargetConnector(T* node, const QPointF &destinationPoint) const
{
    return findTargetConnector(node, calculateTargetBoundaries(destinationPoint, 2));
}

template <typename T>
FilterConnector* PipeController::findTargetConnector(T* node, const QRectF &boundaries) const
{
    if (!node)
    {
        return nullptr;
    }
    for (const auto &connectorItem : qAsConst(node->getItem()->getPorts()))
    {
        const auto connector = dynamic_cast<FilterConnector*> (connectorItem);
        auto size = connector->getItemGeometry();
        if (node->getGroup())
        {
            QPointF points = node->getGroup()->getItem()->mapToItem(node->getGroup()->getGraph()->getContainerItem(), {node->getItemGeometry().x(), node->getItemGeometry().y()});
            if (areasOverlap({boundaries.x(), boundaries.y()},{boundaries.width(), boundaries.height()},{points.x() + size.x(),points.y() + size.y()},{points.x() + size.x() + size.width(),points.y() + size.y() + size.height()}))
            {
                return connector;
            }
        }
        if (areasOverlap({boundaries.x(), boundaries.y()},{boundaries.width(), boundaries.height()},{node->getItemGeometry().x() + size.x(),node->getItemGeometry().y() + size.y()},{node->getItemGeometry().x() + size.x() + size.width(),node->getItemGeometry().y() + size.y() + size.height()}))
        {
            return connector;
        }
    }
    return nullptr;
}

bool PipeController::areasOverlap(const QPointF& topLeftPoint, const QPointF& bottomRightPoint, const QPointF& topLeftPointTwo, const QPointF& bottomRightPointTwo) const
{
    // If one rectangle is on left side of other
    if (topLeftPoint.x() >= bottomRightPointTwo.x() || topLeftPointTwo.x() >= bottomRightPoint.x() )
    {
        return false;
    }
    // If one rectangle is above other
    if (topLeftPoint.y() >= bottomRightPointTwo.y() || topLeftPointTwo.y() >= bottomRightPoint.y())
    {
        return false;
    }

    return true;
}

void PipeController::setZoom(qreal zoom)
{
    if (qFuzzyCompare(zoom, m_zoom))
    {
        return;
    }
    m_zoom = zoom;
    emit zoomChanged();
}

namespace
{
template <typename T, typename R>
void insertNewPipeImpl(fliplib::Pipe &newPipe, T *sourceFilter, FilterConnector *sourceConnector, R *destinationFilter)
{
    newPipe.sender = precitec::storage::compatibility::toPoco(sourceFilter->ID());
    newPipe.senderConnectorId = precitec::storage::compatibility::toPoco(sourceConnector->ID());
    newPipe.senderConnectorName = sourceConnector->getLabel().toStdString();

    newPipe.receiver = precitec::storage::compatibility::toPoco(destinationFilter->ID());
}

}

void PipeController::insertNewPipe(qan::Edge* edge, qan::Node* sourceNode, FilterConnector* sourceConnector, qan::Node* destinationNode, FilterConnector* destinationConnector)
{
    /* There are 3 different cases:
     * 1. Edge between two filters with connectors -> create pipe
     * 2. Edge between filter and port -> nothing to do or create pipe and search in case *    3 and fill the rest (1. Method)
     * 3. Edge between port and filter -> check if edge is valid and create a pipe
     *    (2. Method)
     */
    fliplib::Pipe newPipe;
    newPipe.id = precitec::storage::compatibility::toPoco(QUuid::fromString(edge->getLabel()));

    newPipe.receiverConnectorId = precitec::storage::compatibility::toPoco(destinationConnector->ID());
    newPipe.receiverConnectorName = destinationConnector->tag().toStdString();
    newPipe.receiverConnectorGroup = destinationConnector->group();

    fliplib::GraphItemExtension pipeExtension;
    Poco::UUIDGenerator uuidGenerator;
    pipeExtension.id = uuidGenerator.createRandom();
    pipeExtension.type = 7;
    pipeExtension.localScope = Poco::UUID("f132ea6a-0b0a-48bb-9335-d0a5ff8c31f6");

    newPipe.extensions.push_back(pipeExtension);

    auto sourceFilter = qobject_cast<FilterNode*>(sourceNode);
    auto sourceMacro = qobject_cast<FilterMacro*>(sourceNode);
    auto sourceMacroConnector = qobject_cast<FilterMacroConnector*>(sourceNode);
    auto destinationFilter = qobject_cast<FilterNode*>(destinationNode);
    auto destinationMacro = qobject_cast<FilterMacro*>(destinationNode);
    auto destinationMacroConnector = qobject_cast<FilterMacroConnector*>(destinationNode);
    auto sourcePort = qobject_cast<FilterPort*>(sourceNode);

    if ((sourceFilter || sourceMacro || sourceMacroConnector) && (destinationFilter || destinationMacro || destinationMacroConnector))
    {
        //Case 1
        //Cpp: Insert the pipe in the graph and for the xml file
        if (sourceFilter && destinationFilter)
        {
            insertNewPipeImpl(newPipe, sourceFilter, sourceConnector, destinationFilter);
        }
        else if (sourceMacro && destinationFilter)
        {
            insertNewPipeImpl(newPipe, sourceMacro, sourceConnector, destinationFilter);
        }
        else if (sourceFilter && destinationMacro)
        {
            insertNewPipeImpl(newPipe, sourceFilter, sourceConnector, destinationMacro);
        }
        else if (sourceMacro && destinationMacro)
        {
            insertNewPipeImpl(newPipe, sourceMacro, sourceConnector, destinationMacro);
        }
        else if (sourceFilter && destinationMacroConnector)
        {
            insertNewPipeImpl(newPipe, sourceFilter, sourceConnector, destinationMacroConnector);
        }
        else if (sourceMacroConnector && destinationFilter)
        {
            insertNewPipeImpl(newPipe, sourceMacroConnector, sourceConnector, destinationFilter);
        }
    }
    if (sourcePort && (destinationFilter || destinationMacro))
    {
        //Get port partner to get the sender filter node.
        auto sourceNode = sourcePort->getOneFilterPortPartner()->get_in_edges().at(0)->getSource();
        auto sourceFilter = qobject_cast<FilterNode*> (sourceNode);
        auto sourceMacro = qobject_cast<FilterMacro*>(sourceNode);
        auto sourceConnectorItem = sourcePort->getOneFilterPortPartner()->get_in_edges().at(0)->getItem()->getSourceItem();
        auto sourceConnectorRight = qobject_cast<FilterConnector*> (sourceConnectorItem);

        newPipe.sender = precitec::storage::compatibility::toPoco(sourceFilter ? sourceFilter->ID() : sourceMacro->ID());
        newPipe.senderConnectorId = precitec::storage::compatibility::toPoco(sourceConnectorRight->ID());
        newPipe.senderConnectorName = sourceConnectorRight->getLabel().toStdString();

        if (destinationFilter)
        {
            newPipe.receiver = precitec::storage::compatibility::toPoco(destinationFilter->ID());
        }
        if (destinationMacro)
        {
            newPipe.receiver = precitec::storage::compatibility::toPoco(destinationMacro->ID());
        }

        fliplib::GraphItemExtension pipePortExtension;
        pipePortExtension.id = uuidGenerator.createRandom();
        pipePortExtension.type = 1;
        pipePortExtension.localScope = precitec::storage::compatibility::toPoco(sourcePort->ID());

        newPipe.extensions.push_back(pipePortExtension);
    }

    m_graph->pipes.push_back(newPipe);
    emit changed();
}

void PipeController::setGraph(fliplib::GraphContainer *graph)
{
    if (m_graph == graph)
    {
        return;
    }
    m_graph = graph;
    emit graphChanged();
}

namespace
{
QUuid getId(qan::Node *node)
{
    if (auto filter = qobject_cast<FilterNode*>(node))
    {
        return filter->ID();
    }
    if (auto macro = qobject_cast<FilterMacro*>(node))
    {
        return macro->ID();
    }
    return {};
}
}

void PipeController::deleteEdge(qan::Edge* edge)
{
    auto source = edge->getSource();
    auto destination = edge->getDestination();

    if (dynamic_cast<FilterPort*> (source))     //type = 2
    {
        auto outputPort = dynamic_cast<FilterPort*> (source);
        if (outputPort->gotPartner())
        {
            //there could be a pipe
            auto partnerPort = outputPort->getOneFilterPortPartner();
            if (partnerPort->get_in_edges().size())
            {
                auto sourceEdge = partnerPort->get_in_edges().at(0);
                auto sourceFilterID = getId(sourceEdge->getSource());
                auto sourceConnectorID = dynamic_cast<FilterConnector*>(sourceEdge->getItem()->getSourceItem())->ID();
                auto destinationFilterID = getId(destination);
                auto destinationConnectorID = dynamic_cast<FilterConnector*>(edge->getItem()->getDestinationItem())->ID();
                auto filterConnector = dynamic_cast<FilterConnector*>(edge->getItem()->getDestinationItem());
                filterConnector->getInEdgeItems().clear();
                erasePipeSourceDestination(sourceFilterID, sourceConnectorID, destinationFilterID, destinationConnectorID);
            }
            m_filterGraph->removeEdge(edge);
            //Update UUIDs of the port qml
            outputPort->checkIfThereIsAnotherConnection();
            //Update UUIDs of the port cpp
            GraphHelper{m_graph}.updatePortIDs(outputPort);
            return;
        }
    }
    else if (dynamic_cast<FilterPort*> (destination))   //type = 3
    {
        GraphHelper helper{m_graph};
        auto inputPort = dynamic_cast<FilterPort*> (destination);
        if (inputPort->gotPartner())
        {
            //there could be a pipe
            auto partnerPorts = inputPort->getFilterPortPartner();
            for (const auto &partnerPort : partnerPorts)
            {
                for (const auto &outEdge : partnerPort->get_out_edges())
                {
                    auto destinationEdge = outEdge;
                    auto sourceFilterID = getId(source);
                    auto sourceConnectorID = dynamic_cast<FilterConnector*>(edge->getItem()->getSourceItem())->ID();
                    auto destinationFilterID = getId(destinationEdge->getDestination());
                    auto destinationConnectorID = dynamic_cast<FilterConnector*>(destinationEdge->getItem()->getDestinationItem())->ID();
                    erasePipeSourceDestination(sourceFilterID, sourceConnectorID, destinationFilterID, destinationConnectorID);
                    //Update UUIDs of the port qml
                    partnerPort->setUuid(UUIDs::senderFilterInstanceUUID, {});
                    partnerPort->setUuid(UUIDs::senderFilterConnectorUUID, {});
                    partnerPort->setUuid(UUIDs::receiverFilterInstanceUUID, {});
                    partnerPort->setUuid(UUIDs::receiverFilterConnectorUUID, {});
                    //Update UUIDs of the port cpp
                    helper.updatePortIDs(partnerPort);
                }
                //Delete edge between output port (port partner) and filter
                std::vector<qan::Edge*> outEdges;
                for (const auto &outEdge : partnerPort->get_out_edges())
                {
                    outEdges.push_back(outEdge);

                }
                for (const auto &outEdge : outEdges)
                {
                    //Find input connector of edge
                    auto filterConnector = dynamic_cast<FilterConnector*>(outEdge->getItem()->getDestinationItem());
                    //remove pipe between output port and filter
                    m_filterGraph->removeEdge(outEdge);
                    //Give free the input connector
                    filterConnector->getInEdgeItems().clear();
                }
            }
        }
        //Update UUIDs of the port qml
        inputPort->setUuid(UUIDs::senderFilterInstanceUUID, {});
        inputPort->setUuid(UUIDs::senderFilterConnectorUUID, {});
        //Reset data type of inputPort and port partners
        inputPort->setDataType(-1);
        inputPort->setDataTypeOfFilterPortPartner();
        //Update UUIDs of the port cpp
        helper.updatePortIDs(inputPort);
    }
    else
    {
        GraphHelper{m_graph}.erase(edge);
        emit changed();
    }
    if (!dynamic_cast<FilterPort*>(destination))
    {
        if (auto connector = dynamic_cast<FilterConnector*>(edge->getItem()->getDestinationItem()))
        {
            connector->getInEdgeItems().clear();
        }
    }
    //Delete edge (qml)
    m_filterGraph->removeEdge(edge);
}

void PipeController::erasePipeSourceDestination(const QUuid& sourceID, const QUuid& sourceConnectorID,  const QUuid& destinationID, const QUuid& destinationConnectorID)
{
    auto sourceId = precitec::storage::compatibility::toPoco(sourceID);
    auto sourceConnectorId = precitec::storage::compatibility::toPoco(sourceConnectorID);
    auto destinationId = precitec::storage::compatibility::toPoco(destinationID);
    auto destinationConnectorId = precitec::storage::compatibility::toPoco(destinationConnectorID);

    auto foundPipe = std::find_if(m_graph->pipes.begin(), m_graph->pipes.end(), [sourceId, sourceConnectorId, destinationId, destinationConnectorId](const fliplib::Pipe &actualPipe){return (actualPipe.sender == sourceId && actualPipe.senderConnectorId == sourceConnectorId && actualPipe.receiver == destinationId && actualPipe.receiverConnectorId == destinationConnectorId);});
    if (foundPipe != m_graph->pipes.end())
    {
        m_graph->pipes.erase(foundPipe);
        emit changed();
        return;
    }
    qDebug() << "File must be corrupted, no element found! (GraphModelVisualizer::erasePipeSourceDestination)";
}

void PipeController::deleteEdges(const qcm::Container< QVector, qan::Edge* > &edges)
{
    if (edges.size() == 0)
    {
        return;
    }
    std::vector<qan::Edge*> e;
    e.reserve(edges.size());
    std::copy(edges.begin(), edges.end(), std::back_inserter(e));
    //Filter is connected and starts a branch
    //Check to which object the output pipes goes and
    for (auto const &outEdge : e)
    {
        if (auto edge = outEdge)
        {
            deleteEdge(edge);
        }
    }
}

}
}
}
}
