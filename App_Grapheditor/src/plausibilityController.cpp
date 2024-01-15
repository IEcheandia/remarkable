#include "plausibilityController.h"
#include "FilterConnector.h"
#include "FilterGraph.h"
#include "filterMacro.h"
#include "FilterNode.h"
#include "FilterPort.h"
#include "graphHelper.h"

#include "fliplib/graphContainer.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

PlausibilityController::PlausibilityController(QObject *parent)
    : AbstractNodeController(parent)
{
}

PlausibilityController::~PlausibilityController() = default;

bool PlausibilityController::checkGroup(FilterGroup *group)
{
    bool groupHasNodes = false;
    for (const auto &nodePtr : qAsConst(filterGraph()->get_nodes()))
    {
        auto node = nodePtr;
        if (!group->hasNode(node))
        {
            continue;
        }
        if (qobject_cast<FilterNode*>(node))
        {
            groupHasNodes = true;
            if (!checkFilter(node) || !checkNodeInGroup(node))
            {
                return false;
            }
        }
        else if (qobject_cast<FilterMacro*>(node))
        {
            // macros are not allowed in a group.
            return false;
        }
        else if (dynamic_cast<FilterPort*>(node))
        {
            if (checkPort(node) || !checkNodeInGroup(node))
            {
                return false;
            }
        }
    }
    return groupHasNodes;
}

bool PlausibilityController::plausibilityCheck(bool checkInsignificantStuff)
{
    auto graphIsValid = true;
    m_invalidNodes.clear();
    // Check every filter and port in the filter graph
    for (const auto &nodePtr : qAsConst(filterGraph()->get_nodes()))
    {
        auto node = nodePtr;
        //Check if node is filter or port
        if (dynamic_cast<FilterNode*>(node))
        {
            //Check out and in connectors
            if (!checkFilter(node))      //Filter is invalid so add to vector so they are recognizes for later messages or anything else
            {
                m_invalidNodes.push_back(node);
            }
        }
        else if (qobject_cast<FilterMacro*>(node))
        {
            //Check out and in connectors
            if (!checkFilter(node))      //Filter is invalid so add to vector so they are recognizes for later messages or anything else
            {
                m_invalidNodes.push_back(node);
            }
        }
        else if (dynamic_cast<FilterPort*>(node))
        {
            //Check if port is connected and has a valid type
            auto checkedPort = checkPort(node, checkInsignificantStuff);
            if (checkedPort)
            {
                m_invalidNodes.push_back(checkedPort);
            }
        }
    }

    if (m_invalidNodes.size() != 0)
    {
        graphIsValid = false;       //Add a tag to the graph name or graph file name, something like [invalid]
    }
    return graphIsValid;
}

bool PlausibilityController::checkFilter(qan::Node* node) const
{
    for (const auto &connector : qAsConst(node->getItem()->getPorts()))
    {
        const auto connectorItem = qobject_cast<FilterConnector*>(connector);
        if (static_cast<int>(connectorItem->getType()) == 1)  //InputConnector
        {
            //Not connected if the in going edge vector is empty
            if (connectorItem->getInEdgeItems().size() == 0 && connectorItem->connectionType() != fliplib::PipeConnector::ConnectionType::Optional)        //Not connected
            {
                return false;
            }
        }
        else if (static_cast<int>(connectorItem->getType()) == 2)   //OutputConnector
        {
            //Not connected if the out going edge vector is empty , but can be ignored
            if (connectorItem->getOutEdgeItems().size() == 0)       //Not connected
            {
                if (connectorItem->connectionType() == fliplib::PipeConnector::ConnectionType::Mandatory)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool PlausibilityController::checkNodeInGroup(qan::Node* node) const
{
    const auto group = node->getGroup();
    for (auto edgeItem : node->get_in_edges())
    {
        const auto edge = edgeItem;
        if (!edge)
        {
            continue;
        }
        const auto source = edge->getSource();
        if (!source)
        {
            continue;
        }
        if (source->getGroup() != group)
        {
            return false;
        }
    }
    for (auto edgeItem : node->get_out_edges())
    {
        const auto edge = edgeItem;
        if (!edge)
        {
            continue;
        }
        const auto destination = edge->getDestination();
        if (!destination)
        {
            continue;
        }
        if (destination->getGroup() != group)
        {
            return false;
        }
    }
    return true;
}

FilterPort* PlausibilityController::checkPort(qan::Node* node, bool checkInsignificantStuff)
{
    //TODO check this stuff
    auto port = dynamic_cast<FilterPort*>(node);

    if (!port->gotPartner() && checkInsignificantStuff)        //Has no port partner
    {
        return port;
    }

    if (!checkInsignificantStuff)
    {
        if (!port->gotPartner() && port->type() == 2)
        {
            return port;
        }
    }

    //check if port partner exists?
    GraphHelper helper{graph()};
    auto portPartners = port->getFilterPortPartner();
    for (const auto &partner : portPartners)
    {
        if (!helper.find(partner))
        {
            return port;
        }
    }

    if (port->type() == 3)      //Input port with one to multiple port partners
    {
        //
        if (port->get_in_nodes().size() == 0)
        {
            return port;
        }
    }
    else        //Output port with one port partner
    {
        //
        if (port->get_out_edges().size() == 0)
        {
            return port;
        }
    }

    if (port->dataType() < 0 && port->dataType() > 11)
    {
        return port;
    }

    return {};
}


void PlausibilityController::addIDToInvalidIDs(const Poco::UUID &id, InvalidIDType type)
{
    auto foundID = std::find_if(m_invalidIDs.begin(), m_invalidIDs.end(), [id](const auto &currentPair){return currentPair.first == id;});
    if (foundID == m_invalidIDs.end())
    {
        m_invalidIDs.emplace_back(id, type);
    }
}

bool PlausibilityController::checkIDs()
{
    m_invalidIDs.clear();

    checkFilterAndAttributeIDs();

    checkInstanceID(graph()->pipes.begin(), graph()->pipes.end(), InvalidIDType::Pipe);
    checkInstanceID(graph()->ports.begin(), graph()->ports.end(), InvalidIDType::Port);
    checkInstanceID(graph()->sensors.begin(), graph()->sensors.end(), InvalidIDType::Sensor);
    checkInstanceID(graph()->errors.begin(), graph()->errors.end(), InvalidIDType::Error);
    checkInstanceID(graph()->results.begin(), graph()->results.end(), InvalidIDType::Result);
    checkInstanceID(graph()->macros.begin(), graph()->macros.end(), InvalidIDType::Macro);

    return m_invalidIDs.empty();
}

void PlausibilityController::checkFilterAndAttributeIDs()
{
    std::set<Poco::UUID> filterIDs;
    std::set<Poco::UUID> attributeIDs;

    for (const auto &filter : graph()->instanceFilters)
    {
        if (!filterIDs.insert(filter.id).second)
        {
            addIDToInvalidIDs(filter.id, InvalidIDType::Filter);
        }

        for (const auto &attribute : filter.attributes)
        {
            if (!attributeIDs.insert(attribute.instanceAttributeId).second)
            {
                addIDToInvalidIDs(attribute.instanceAttributeId, InvalidIDType::Attribute);
            }
        }
    }
}

template<typename InputIterator> void PlausibilityController::checkInstanceID(InputIterator first, InputIterator last, PlausibilityController::InvalidIDType type)
{
    for (; first != last; first++)
    {
        for (auto searchIterator = first + 1; searchIterator != last; searchIterator++)
        {
            if (searchIterator->id == first->id)
            {
                addIDToInvalidIDs(searchIterator->id, type);
            }
        }
    }
}

}
}
}
}
