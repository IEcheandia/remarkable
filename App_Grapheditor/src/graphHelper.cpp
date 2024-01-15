#include "graphHelper.h"

#include "FilterComment.h"
#include "FilterGroup.h"
#include "filterMacro.h"
#include "filterMacroConnector.h"
#include "FilterNode.h"
#include "FilterPort.h"
#include "fliplib/graphContainer.h"
#include "../App_Storage/src/compatibility.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

GraphHelper::GraphHelper(fliplib::GraphContainer* graph)
    : m_actualGraph(graph)
{
}

namespace
{

template <typename T, typename U>
typename std::vector<T>::iterator findIterator(U *node, std::vector<T> &nodes)
{
    const auto filterNodeUuid = precitec::storage::compatibility::toPoco(node->ID());
    const auto it = std::find_if(nodes.begin(), nodes.end(),
        [&filterNodeUuid] (const T& instance)
        {
            return instance.id == filterNodeUuid;
        });
    return it;
}

template <typename T, typename U>
T *findImpl(U *node, std::vector<T> &nodes)
{
    const auto it = findIterator(node, nodes);
    if (it != nodes.end())
    {
        return &(*it);
    }
    return nullptr;
}

}

QPointF GraphHelper::positionToGroup(qan::Node *node, const QPointF &point) const
{
    if (node->getGroup())
    {
        return node->getGroup()->getItem()->mapToItem(node->getGroup()->getGraph()->getContainerItem(), point);
    }
    return point;
}

fliplib::InstanceFilter *GraphHelper::find(FilterNode *node)
{
    return findImpl(node, m_actualGraph->instanceFilters);
}

fliplib::Port *GraphHelper::find(FilterComment *node)
{
    return findImpl(node, m_actualGraph->ports);
}

fliplib::Port *GraphHelper::find(FilterPort *node)
{
    return findImpl(node, m_actualGraph->ports);
}

fliplib::Pipe *GraphHelper::find(qan::Edge *edge)
{
    auto it = findInternal(edge);
    if (it != m_actualGraph->pipes.end())
    {
        return &(*it);
    }
    return nullptr;
}

fliplib::Macro *GraphHelper::find(FilterMacro *macro)
{
    return findImpl(macro, m_actualGraph->macros);
}

fliplib::Macro::Connector *GraphHelper::find(FilterMacroConnector *macro)
{
    if (auto connector = findImpl(macro, m_actualGraph->inConnectors))
    {
        return connector;
    }
    return findImpl(macro, m_actualGraph->outConnectors);
}

std::vector<fliplib::FilterGroup>::iterator GraphHelper::findInternal(int groupId)
{
    return std::find_if(m_actualGraph->filterGroups.begin(), m_actualGraph->filterGroups.end(),
        [groupId] (const auto &actualGroup)
        {
            return actualGroup.number == groupId;
        });
}

std::vector<fliplib::FilterGroup>::iterator GraphHelper::findInternal(FilterGroup *node)
{
    return findInternal(node->ID());
}

fliplib::FilterGroup *GraphHelper::find(FilterGroup *node)
{
    const auto it = findInternal(node);
    if (it != m_actualGraph->filterGroups.end())
    {
        return &(*it);
    }
    return nullptr;
}

fliplib::FilterGroup *GraphHelper::findGroup(int groupId)
{
    const auto it = findInternal(groupId);
    if (it != m_actualGraph->filterGroups.end())
    {
        return &(*it);
    }
    return nullptr;
}

void GraphHelper::erase(FilterGroup *group)
{
    eraseGroup(group->ID());
}

void GraphHelper::eraseGroup(int groupId)
{
    auto it = findInternal(groupId);
    if (it != m_actualGraph->filterGroups.end())
    {
        m_actualGraph->filterGroups.erase(it);
    }
}

void GraphHelper::erase(FilterMacro *macro)
{
    eraseGroup(macro->groupID());
    if (auto it = findIterator(macro, m_actualGraph->macros); it != m_actualGraph->macros.end())
    {
        m_actualGraph->macros.erase(it);
    }
}

void GraphHelper::erase(FilterMacroConnector *macro)
{
    if (auto it = findIterator(macro, m_actualGraph->inConnectors); it != m_actualGraph->inConnectors.end())
    {
        m_actualGraph->inConnectors.erase(it);
    }
    if (auto it = findIterator(macro, m_actualGraph->outConnectors); it != m_actualGraph->outConnectors.end())
    {
        m_actualGraph->outConnectors.erase(it);
    }
}

std::vector<fliplib::Pipe>::iterator GraphHelper::findInternal(qan::Edge *edge)
{
    const auto pipeID = precitec::storage::compatibility::toPoco(QUuid::fromString(edge->getLabel()));
    return std::find_if(m_actualGraph->pipes.begin(), m_actualGraph->pipes.end(), [pipeID](fliplib::Pipe const &actualPipe){return actualPipe.id == pipeID;});
}

void GraphHelper::erase(qan::Edge *edge)
{
    auto it = findInternal(edge);
    if (it != m_actualGraph->pipes.end())
    {
        m_actualGraph->pipes.erase(it);
    }
}

template <typename T>
void GraphHelper::updatePositionImpl(T *node, const QPointF &point)
{
    const auto newPosition = positionToGroup(node, point);
    if (auto c = find(node))
    {
        c->position.x = newPosition.x();
        c->position.y = newPosition.y();
    }
    node->getItem()->setRect({point.x(), point.y(), node->getItemGeometry().width(), node->getItemGeometry().height()});
}

void GraphHelper::updatePosition(FilterNode *node, const QPointF &point)
{
    updatePositionImpl(node, point);
}

void GraphHelper::updatePosition(FilterComment* comment, const QPointF &point)
{
    updatePositionImpl(comment, point);
}

void GraphHelper::updatePosition(FilterPort *port, const QPointF &point)
{
    updatePositionImpl(port, point);
}

void GraphHelper::updatePosition(FilterMacro *macro, const QPointF &point)
{
    updatePositionImpl(macro, point);
}

void GraphHelper::updatePosition(FilterMacroConnector *macro, const QPointF &point)
{
    updatePositionImpl(macro, point);
}

void GraphHelper::updatePortIDs(FilterPort *filterPort)
{
    auto port = find(filterPort);
    if (!port)
    {
        qDebug() << "Port not found! (GraphModelVisualizer::updatePortIDs";
        return;
    }

    auto senderId = precitec::storage::compatibility::toPoco(filterPort->getUuid(UUIDs::senderFilterInstanceUUID));
    auto senderConnectorId = precitec::storage::compatibility::toPoco(filterPort->getUuid(UUIDs::senderFilterConnectorUUID));
    auto receiverId = precitec::storage::compatibility::toPoco(filterPort->getUuid(UUIDs::receiverFilterInstanceUUID));
    auto receiverConnectorId = precitec::storage::compatibility::toPoco(filterPort->getUuid(UUIDs::receiverFilterConnectorUUID));

    if (port->senderInstanceFilterId != senderId)
    {
        port->senderInstanceFilterId = senderId;
    }
    if (port->senderConnectorId != senderConnectorId)
    {
        port->senderConnectorId = senderConnectorId;
    }
    if (port->receiverInstanceFilterId != receiverId)
    {
        port->receiverInstanceFilterId = receiverId;
    }
    if (port->receiverConnectorId != receiverConnectorId)
    {
        port->receiverConnectorId = receiverConnectorId;
    }
}

}
}
}
}
