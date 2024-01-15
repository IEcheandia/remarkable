#include "CopyPasteHandler.h"
#include "FilterGraph.h"
#include "GraphEditor.h"
#include "graphHelper.h"
#include "groupController.h"

#include "Poco/UUIDGenerator.h"

using namespace precitec::gui::components::grapheditor;

CopyPasteHandler::CopyPasteHandler(QObject* parent) : QObject(parent)
{ }

CopyPasteHandler::~CopyPasteHandler() = default;

FilterGraph* CopyPasteHandler::filterGraph() const
{
    return m_filterGraph;
}

void CopyPasteHandler::setFilterGraph(FilterGraph* newGraph)
{
    if (m_filterGraph != newGraph)
    {
        m_filterGraph = newGraph;
        emit filterGraphChanged();
    }
}

GraphEditor* CopyPasteHandler::graphEditor() const
{
    return m_graphEditor;
}

void CopyPasteHandler::setGraphEditor(GraphEditor* newEditor)
{
    if (m_graphEditor != newEditor)
    {
        m_graphEditor = newEditor;
        emit graphEditorChanged();
    }
}

QPointF CopyPasteHandler::copyPosition() const
{
    return m_copyPosition;
}

void CopyPasteHandler::setCopyPosition(const QPointF& newPosition)
{
    if (m_copyPosition != newPosition)
    {
        m_copyPosition = newPosition;
        emit copyPositionChanged();
    }
}

void CopyPasteHandler::copyObjects()
{
    resetGraphContainer();

    const auto &selectedNodes = m_filterGraph->getSelectedNodes();
    const auto &selectedGroups = m_filterGraph->getSelectedGroups();

    for (int i = 0; i < selectedNodes.getContainer().size(); i++)
    {
        auto node = selectedNodes.getContainer().at(i);
        if (dynamic_cast<FilterNode*>(node))
        {
            auto filter = dynamic_cast<FilterNode*>(node);
            m_copyGraphInstances.instanceFilters.push_back(m_graphEditor->graphModelVisualizer()->searchInstanceFilter(filter->ID()));
            m_copyGraphInstances.filterDescriptions.push_back(m_graphEditor->graphModelVisualizer()->searchFilterDescription(filter->typeID()));
            //Get pipes
            for (const auto &connector : qAsConst(filter->getItem()->getPorts()))
            {
                const auto connectorItem = qobject_cast<FilterConnector*>(connector);
                if (static_cast<int>(connectorItem->getType()) == 1)
                {
                    if (connectorItem->getInEdgeItems().size() != 0)
                    {
                        auto inPipe = connectorItem->getInEdgeItems().at(0);
                        for (const auto &element : selectedNodes)
                        {
                            if (checkNodes(inPipe->getEdge()->getSource(), element))
                            {
                                if (inPipe->getEdge()->getLabel().isEmpty())
                                {
                                    if (dynamic_cast<FilterPort*>(inPipe->getEdge()->getSource()))
                                    {
                                        auto port = dynamic_cast<FilterPort*>(inPipe->getEdge()->getSource());
                                        if (port->gotPartner())
                                        {
                                            auto portPartner = port->getOneFilterPortPartner();
                                            auto sourceFilter = dynamic_cast<FilterNode*> (portPartner->get_in_edges().at(0)->getSource());
                                            for (const auto &element : selectedNodes)
                                            {
                                                if (checkNodes(portPartner, element))
                                                {
                                                    for (const auto &element: selectedNodes)
                                                    {
                                                        if (checkNodes(sourceFilter, element))
                                                        {
                                                            auto sourceFilterConnector = dynamic_cast<FilterConnector*> (portPartner->get_in_edges().at(0)->getItem()->getSourceItem());
                                                            m_copyGraphInstances.pipes.push_back(m_graphEditor->graphModelVisualizer()->searchPipe(sourceFilter->ID(), sourceFilterConnector->ID(), filter->ID(), connectorItem->ID()));
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    if (auto pipe = GraphHelper{m_graphEditor->graphModelVisualizer()->graph()}.find(inPipe->getEdge()))
                                    {
                                        m_copyGraphInstances.pipes.push_back(*pipe);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (dynamic_cast<FilterPort*>(node))
        {
            auto port = dynamic_cast<FilterPort*>(node);
            m_copyGraphInstances.ports.push_back(m_graphEditor->graphModelVisualizer()->searchPortComment(port->ID()));
        }
        else if (dynamic_cast<FilterComment*>(node))
        {
            auto comment = dynamic_cast<FilterComment*>(node);
            m_copyGraphInstances.ports.push_back(m_graphEditor->graphModelVisualizer()->searchPortComment(comment->ID()));
        }
    }
    for (int i = 0; i < selectedGroups.getContainer().size(); i++)
    {
        auto group = dynamic_cast<FilterGroup*>(selectedGroups.getContainer().at(i));
        GraphHelper helper{m_graphEditor->graphModelVisualizer()->graph()};
        if (auto filterGroup = helper.find(group))
        {
            m_copyGraphInstances.filterGroups.push_back(*filterGroup);
        }
    }
}

void CopyPasteHandler::pasteObjects()
{
    if (m_copyGraphInstances.filterDescriptions.size() == 0 && m_copyGraphInstances.filterGroups.size() == 0 && m_copyGraphInstances.ports.size() == 0)
    {
        return;
    }

    std::vector<std::pair<Poco::UUID, Poco::UUID>> IDs;
    std::vector<std::pair<int,int>> groupIDs;
    std::vector<FilterNode*> copiedFilters;
    std::vector<FilterPort*> copiedPorts;
    QPointF firstObjectPosition;

    //TODO If group is selected copy the content too?
    for (auto &group : m_copyGraphInstances.filterGroups)
    {
        auto newGroupID = m_graphEditor->getLowestFilterGroupID(100, 0);    //Specify maximum
        groupIDs.push_back({group.number, newGroupID});
        group.number = newGroupID;
        auto newName = m_graphEditor->getGroupDefaultName(group.number);
        auto oldName = QString::fromStdString(group.name).remove(0,3);
        group.name = newName.toStdString() + " " + oldName.toStdString();
        m_graphEditor->graphModelVisualizer()->groupController()->insertNewGroup(group);
    }

    if (m_copyGraphInstances.instanceFilters.size() != 0)
    {
        firstObjectPosition.setX(m_copyGraphInstances.instanceFilters.at(0).position.x);
        firstObjectPosition.setY(m_copyGraphInstances.instanceFilters.at(0).position.y);
    }
    else if (m_copyGraphInstances.ports.size() != 0)
    {
        firstObjectPosition.setX(m_copyGraphInstances.ports.at(0).position.x);
        firstObjectPosition.setY(m_copyGraphInstances.ports.at(0).position.y);
    }

    for (unsigned int i = 0; i < m_copyGraphInstances.instanceFilters.size(); i++)
    {
        auto filter = m_copyGraphInstances.instanceFilters.at(i);
        QPointF diff = {firstObjectPosition.x() - filter.position.x, firstObjectPosition.y() - filter.position.y};
        filter.position.x = m_copyPosition.x() - diff.x();
        filter.position.y = m_copyPosition.y() - diff.y();
        auto generatedID = Poco::UUIDGenerator().createRandom();
        IDs.push_back({filter.id, generatedID});
        filter.id = generatedID;
        const auto &generatedVariantID = Poco::UUIDGenerator().createRandom();
        for (auto &element : filter.attributes)
        {
            element.instanceAttributeId = Poco::UUIDGenerator().createRandom();
            element.instanceVariantId = generatedVariantID;
        }
        for (auto &element : filter.extensions)
        {
            element.id = Poco::UUIDGenerator().createRandom();
        }
        auto foundGroup = std::find_if(groupIDs.begin(), groupIDs.end(), [filter](const auto &actualGroupIDPair){return actualGroupIDPair.first == filter.group;});
        if (foundGroup != groupIDs.end())
        {
            filter.group = foundGroup->second;
        }
        else
        {
            filter.group = -1;
        }
        copiedFilters.push_back(m_graphEditor->graphModelVisualizer()->insertOneFilterNode(filter, m_copyGraphInstances.filterDescriptions.at(i)));
    }
    for (auto &port : m_copyGraphInstances.ports)
    {
        auto generatedID = Poco::UUIDGenerator().createRandom();
        QPointF diff = {firstObjectPosition.x() - port.position.x, firstObjectPosition.y() - port.position.y};
        port.position.x = m_copyPosition.x() - diff.x();
        port.position.y = m_copyPosition.y() - diff.y();
        if (port.type != 5)  //Kein Comment!
        {
            IDs.push_back({port.id, generatedID});      //FIXME nötig? Wegen GraphItemExtension vmtl.
            port.id = generatedID;
            port.receiverName = port.receiverName + "_copy";      //port name
            if (port.type == 2)
            {
                auto foundID = std::find_if(IDs.begin(), IDs.end(), [port](const auto &actualIDPair){return actualIDPair.first == port.receiverInstanceFilterId;});
                if (foundID != IDs.end())
                {
                    port.receiverInstanceFilterId = foundID->second;
                }
                    port.receiverInstanceFilterId = {};     //FIXME check if there is an other connection
                }
            auto foundID = std::find_if(IDs.begin(), IDs.end(), [port](const auto &actualIDPair){return actualIDPair.first == port.senderInstanceFilterId;});
            if (foundID != IDs.end())
            {
                port.senderInstanceFilterId = foundID->second;
            }
            else
            {
                port.senderInstanceFilterId = {};           //FIXME check if there is an other connection
            }
            for (auto &element : port.extensions)
            {
                element.id = Poco::UUIDGenerator().createRandom();
            }
        }
        else        //Comment
        {
            port.id = generatedID;
        }
        auto foundGroup = std::find_if(groupIDs.begin(), groupIDs.end(), [port](const auto &actualGroupIDPair){return actualGroupIDPair.first == port.group;});
        if (foundGroup != groupIDs.end())
        {
            port.group = foundGroup->second;
        }
        else
        {
            port.group = -1;
        }
        if (auto visualizedPort = m_graphEditor->graphModelVisualizer()->insertOnePort(port))
        {
            copiedPorts.push_back(visualizedPort);
        }
    }
    m_graphEditor->graphModelVisualizer()->getPortPartners();
    m_graphEditor->graphModelVisualizer()->groupController()->connectNodesToFilterGroups(m_copyGraphInstances);
    m_graphEditor->graphModelVisualizer()->insertAllPipesWithPorts(copiedFilters, copiedPorts);
    for (auto &pipe : m_copyGraphInstances.pipes)
    {
        auto generatedID = Poco::UUIDGenerator().createRandom();
        pipe.id = generatedID;
        auto foundSenderID = std::find_if(IDs.begin(), IDs.end(), [pipe](const auto &actualIDPair){return actualIDPair.first == pipe.sender;});
        if (foundSenderID != IDs.end())
        {
            pipe.sender = foundSenderID->second;
        }
        auto foundReceiverID = std::find_if(IDs.begin(), IDs.end(), [pipe](const auto &actualIDPair){return actualIDPair.first == pipe.receiver;});
        if (foundReceiverID != IDs.end())
        {
            pipe.receiver = foundReceiverID->second;
        }
        for (auto &extension : pipe.extensions)
        {
            extension.id = Poco::UUIDGenerator().createRandom();
            if (extension.type == 1)
            {
                auto foundPortID = std::find_if(IDs.begin(), IDs.end(), [extension](const auto &actualIDPair){return actualIDPair.first == extension.localScope;});
                if (foundPortID != IDs.end())
                {
                    extension.localScope = foundPortID->second;
                }
            }
        }
        m_graphEditor->graphModelVisualizer()->insertPipes(pipe);
        m_graphEditor->graphModelVisualizer()->insertOnePipePortWithConnector(pipe, copiedPorts);
    }
    for (auto &port : m_copyGraphInstances.ports)
    {
        if (port.type == 2)
        {
            m_graphEditor->graphModelVisualizer()->checkOtherPortConnections(port.id);
        }
    }
}

void CopyPasteHandler::resetGraphContainer()
{
    m_copyGraphInstances.filterDescriptions.clear();
    m_copyGraphInstances.instanceFilters.clear();
    m_copyGraphInstances.pipes.clear();
    m_copyGraphInstances.filterGroups.clear();
    m_copyGraphInstances.ports.clear();
    m_copyGraphInstances.sensors.clear();
    m_copyGraphInstances.errors.clear();
    m_copyGraphInstances.results.clear();
}

bool CopyPasteHandler::checkNodes(qan::Node* edgeSource, qan::Node* node)
{
    if (edgeSource == node)
    {
        return true;
    }
    return false;
}

