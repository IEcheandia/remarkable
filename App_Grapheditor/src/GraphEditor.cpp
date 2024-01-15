#include "GraphEditor.h"
#include "graphHelper.h"
#include "groupController.h"
#include "filterMacro.h"
#include "filterMacroConnector.h"
#include "pipeController.h"

#include "Poco/UUIDGenerator.h"
#include "../App_Storage/src/compatibility.h"

using namespace precitec::gui::components::grapheditor;

GraphEditor::GraphEditor(QObject* parent) : QObject(parent)
{
    connect(this, &GraphEditor::actualNodeIndexChanged, this, &GraphEditor::initActualNode);
    connect(this, &GraphEditor::actualPortIndexChanged, this, &GraphEditor::initActualPort);
}

GraphEditor::~GraphEditor() = default;

GraphModelVisualizer* GraphEditor::graphModelVisualizer() const
{
    return m_graphModelVisualizer;
}

void GraphEditor::setGraphModelVisualizer(GraphModelVisualizer* visualizer)
{
    if (m_graphModelVisualizer == visualizer)
    {
        return;
    }
    disconnect(m_graphModelVisualizerDestroyedConnection);
    m_graphModelVisualizer = visualizer;
    if (m_graphModelVisualizer)
    {
        m_graphModelVisualizerDestroyedConnection = connect(m_graphModelVisualizer, &QObject::destroyed, this, std::bind(&GraphEditor::setGraphModelVisualizer, this, nullptr));
    }
    else
    {
        m_graphModelVisualizerDestroyedConnection = {};
    }

    emit graphModelVisualizerChanged();
}

Poco::UUID GraphEditor::generateID()
{
    return Poco::UUIDGenerator::defaultGenerator().createRandom();
}

QUuid GraphEditor::newID() const
{
    return m_newID;
}

int GraphEditor::filterTypeIndex() const
{
    return m_actualFilterType;
}

void GraphEditor::setFilterTypeIndex(int actualIndex)
{
    if (m_actualFilterType != actualIndex)
    {
        m_actualFilterType = actualIndex;
        emit filterTypeIndexChanged();
    }
}

QUuid GraphEditor::filterTypeID() const
{
    return m_graphModelVisualizer->getFilterUUID(m_actualFilterType);
}

QString GraphEditor::filterTypeName() const
{
    return m_graphModelVisualizer->getFilterType(m_actualFilterType);
}

void GraphEditor::setActualNodeIndex(const QModelIndex &actualIndex)
{
    if (m_actualNodeIndex != actualIndex.row())
    {
        m_actualNodeIndex = actualIndex.row();
        emit actualNodeIndexChanged();
    }
}

void GraphEditor::setActualPortIndex(const QModelIndex& actualIndex)
{
    if (m_actualPortIndex != actualIndex.row())
    {
        m_actualPortIndex = actualIndex.row();
        emit actualPortIndexChanged();
    }
}

int GraphEditor::actualNodeIndex() const
{
    return m_actualNodeIndex;
}

int GraphEditor::actualPortIndex() const
{
    return m_actualPortIndex;
}

QModelIndex GraphEditor::sourceConnectorIndex() const
{
    return m_sourceConnectorIndex;
}

QModelIndex GraphEditor::destinationConnectorIndex() const
{
    return m_destinationConnectorIndex;
}

FilterNode* GraphEditor::actualNode() const
{
    return m_actualNode;
}

FilterPort* GraphEditor::actualPort() const
{
    return m_actualPort;
}

void GraphEditor::setSourceConnectorIndex(const QModelIndex& actualIndex)
{
    if (m_sourceConnectorIndex != actualIndex)
    {
        m_sourceConnectorIndex = actualIndex;
    }
}

void GraphEditor::setDestinationConnectorIndex(const QModelIndex& actualIndex)
{
    if (m_destinationConnectorIndex != actualIndex)
    {
        m_destinationConnectorIndex = actualIndex;
    }
}

void GraphEditor::generateNewID()
{
    m_newID = QUuid::createUuid();
    emit newIDChanged();
}

void GraphEditor::insertNewFilter(const QString &ID, const QString& filterLabel, const QString& groupLabel, int x, int y, bool visualInsertionInGroup)
{
    //Create new filter (qml)
    auto newNode = dynamic_cast<FilterNode*> (m_graphModelVisualizer->filterGraph()->insertFilterNode());
    newNode->setLabel(filterLabel);
    newNode->setID(QUuid::fromString(ID));            //has to be created before
    newNode->setType(QString{"precitec::filter::"} + m_graphModelVisualizer->getFilterName(m_actualFilterType));
    newNode->setTypeID(m_graphModelVisualizer->getFilterID(m_actualFilterType));
    newNode->setImage(m_graphModelVisualizer->getFilterImagePathID(newNode->typeID()));
    newNode->setGroupID(m_graphModelVisualizer->groupController()->idFromName(groupLabel));
    newNode->setContentName("Precitec.Filter." + m_graphModelVisualizer->getFilterName(m_actualFilterType));
    newNode->getItem()->setRect({x - (defaultFilterSize() * 0.5), y - (defaultFilterSize() * 0.5), defaultFilterSize(), defaultFilterSize()});
    newNode->getItem()->setResizable(false);
    m_graphModelVisualizer->insertAllConnectors(newNode, m_actualFilterType);

    //Group things
    if (groupLabel != "No group" && m_graphModelVisualizer->filterGraph()->get_group_count() != 0)
    {
        for (const auto &groupPtr : qAsConst(m_graphModelVisualizer->filterGraph()->get_groups()))
        {
            if (dynamic_cast<FilterGroup*> (groupPtr))
            {
                auto group = dynamic_cast<FilterGroup*>(groupPtr);
                if (group->ID() == newNode->groupID())
                {
                    if (!visualInsertionInGroup)
                    {
                        auto geometry = group->getItemGeometry();
                        newNode->setItemGeometry(geometry.x() + geometry.width()/2,geometry.y() + geometry.height()/2,newNode->getItemGeometry().width(), newNode->getItemGeometry().height());
                    }
                    m_graphModelVisualizer->filterGraph()->groupNode(group, newNode, true);
                }
            }
        }
    }

    m_graphModelVisualizer->setGraphEdited(true);
    if (!visualInsertionInGroup)
    {
        m_graphModelVisualizer->newObjectInserted(newNode);
    }
    //CPP: HeaderFilterInfo && InstanceFilter && FilterAttributes
    m_graphModelVisualizer->insertNewCppFilter(newNode, m_actualFilterType);
}

void GraphEditor::setFilterType(const QModelIndex& index)
{
    m_actualFilterType = index.row();
}

void GraphEditor::setFilterType(const QUuid& uuid)
{
    m_actualFilterType = m_graphModelVisualizer->searchFilterType(uuid);
}

void GraphEditor::insertNewPort(const QString& label, const int groupID, const int type, int x, int y, int width, int height, bool visualInsertionInGroup)
{
    //QML stuff
    auto newPort = dynamic_cast<FilterPort*> (m_graphModelVisualizer->filterGraph()->insertFilterPort());
    newPort->setID(m_newID);
    newPort->setLabel(label);
    if (type == 0)
    {
        newPort->setType(3);
    }
    else if (type == 1)
    {
        newPort->setType(2);
    }
    newPort->getItem()->setMinimumSize({static_cast<qreal>(width),static_cast<qreal>(height)});
    newPort->getItem()->setRect({static_cast<qreal>(x), static_cast<qreal>(y), static_cast<qreal>(width), static_cast<qreal>(height)});
    newPort->getItem()->setResizable(false);
    newPort->setGroupID(groupID);

    if (groupID != -1 && m_graphModelVisualizer->filterGraph()->get_group_count() != 0)
    {
        for (const auto &groupPtr : qAsConst(m_graphModelVisualizer->filterGraph()->get_groups()))
        {
            if (dynamic_cast<FilterGroup*>(groupPtr))
            {
                auto group = dynamic_cast<FilterGroup*>(groupPtr);
                if (group->ID() == newPort->groupID())
                {
                    if (!visualInsertionInGroup)
                    {
                        auto geometry = group->getItemGeometry();
                        newPort->getItem()->setRect({geometry.x() + geometry.width()/2, geometry.y() + geometry.height()/2, newPort->getItemGeometry().width(), newPort->getItemGeometry().height()});
                    }
                    m_graphModelVisualizer->filterGraph()->groupNode(group, newPort, true);
                }
            }
        }
    }

    m_graphModelVisualizer->setGraphEdited(true);

    if (!visualInsertionInGroup)
    {
        m_graphModelVisualizer->newObjectInserted(newPort);
    }

    //Cpp stuff
    m_graphModelVisualizer->insertNewCppFilterPort(newPort);
}

void GraphEditor::insertFilterPort(const QString& label, int type, int x, int y, int width, int height)
{
    //QML stuff
    auto newPort = dynamic_cast<FilterPort*> (m_graphModelVisualizer->filterGraph()->insertFilterPort());
    newPort->setID(m_newID);
    newPort->setLabel(label);
    newPort->setType(type);
    newPort->getItem()->setMinimumSize({25,25});
    newPort->getItem()->setRect({static_cast<qreal>(x), static_cast<qreal>(y), static_cast<qreal>(width), static_cast<qreal>(height)});
    newPort->getItem()->setResizable(false);
    newPort->setGroupID(-1);

    m_graphModelVisualizer->setGraphEdited(true);
    m_graphModelVisualizer->newObjectInserted(newPort);

    //CPP stuff
    m_graphModelVisualizer->insertNewCppFilterPort(newPort);
}

void GraphEditor::insertNewComment(const QString& label, const int groupID, const QString& text, const int x, const int y, const int width, const int height)
{
    generateNewID();
    auto newComment = dynamic_cast<FilterComment*> (m_graphModelVisualizer->filterGraph()->insertFilterComment());
    newComment->setID(m_newID);
    newComment->setLabel(label);
    newComment->setText(text);
    newComment->setGroupID(groupID);
    newComment->setItemGeometry(x, y, width, height);

    m_graphModelVisualizer->setGraphEdited(true);
    m_graphModelVisualizer->newObjectInserted(newComment);

    m_graphModelVisualizer->insertNewCppFilterComment(newComment);
}

void GraphEditor::insertComment(const QString& label, int type, const QString& commentText, int x, int y, int width, int height)
{
    //QML stuff     //TODO type isn't used!
    auto newComment = dynamic_cast<FilterComment*> (m_graphModelVisualizer->filterGraph()->insertFilterComment());
    newComment->setID(m_newID);
    newComment->setLabel(label);
    newComment->setText(commentText);
    newComment->setGroupID(-1);
    newComment->getItem()->setRect({static_cast<qreal>(x), static_cast<qreal>(y), static_cast<qreal>(width), static_cast<qreal>(height)});

    m_graphModelVisualizer->setGraphEdited(true);
    m_graphModelVisualizer->newObjectInserted(newComment);

    //CPP stuff
    m_graphModelVisualizer->insertNewCppFilterComment(newComment);
}

int GraphEditor::getLowestFilterGroupID(unsigned int maximum, unsigned int actualValue)
{
    return m_graphModelVisualizer->groupController()->getLowestFilterGroupID(maximum, actualValue);
}

QString GraphEditor::getGroupDefaultName(int groupID)
{
    return m_graphModelVisualizer->groupController()->getGroupDefaultName(groupID);
}

void GraphEditor::insertNewGroup(const QPointF &point)
{
    auto newGroup = m_graphModelVisualizer->groupController()->insertNewGroup(point);
    m_graphModelVisualizer->newObjectInserted(newGroup);
}

void GraphEditor::insertNewFilterVisual(const QPointF& insertionPoint)
{
    if (m_actualFilterType == -1)
    {
        return;
    }

    generateNewID();
    auto groupName = QStringLiteral("No Group");
    if (auto group = m_graphModelVisualizer->filterGraph()->groupAt(insertionPoint, QSizeF(defaultFilterSize(), defaultFilterSize())))
    {
        groupName = group->getLabel();
    }
    insertNewFilter(m_newID.toString(), filterTypeName(), groupName, insertionPoint.x() + (defaultFilterSize() * 0.5), insertionPoint.y() + (defaultFilterSize() * 0.5), true);

    m_actualFilterType = -1;
    emit filterTypeIndexChanged();
}

void GraphEditor::insertNewFilterVisualGroup(FilterGroup* group, const QPointF& insertionGroupPoint)
{
    if (m_actualFilterType == -1)
    {
        //qDebug() << "Error no filter type is selected! (GraphEditor::insertNewFilterVisual())";
        return;
    }
    if (!group)
    {
        qDebug() << "Error no group is selected! (GraphEditor::insertNewFilterVisual())";
        return;
    }
    generateNewID();
    //Calculate global position for node insertion
    QPointF insertionPoint = group->getItem()->mapToItem(group->getGraph()->getContainerItem(), insertionGroupPoint);

    insertNewFilter(m_newID.toString(), filterTypeName(), group->getLabel(), insertionPoint.x(), insertionPoint.y(), true);

    m_actualFilterType = -1;
    emit filterTypeIndexChanged();
}

void GraphEditor::deleteObject(qan::Node* node)
{
    m_graphModelVisualizer->filterGraph()->clearSelection();
    if (auto filter = qobject_cast<FilterNode*>(node))
    {
        deleteFilter(filter);
    }
    else if (auto port = qobject_cast<FilterPort*>(node))
    {
        deletePort(port);
    }
    else if (auto comment = qobject_cast<FilterComment*>(node))
    {
        deleteComment(comment);
    }
    else if (auto macro = qobject_cast<FilterMacro*>(node))
    {
        deleteMacro(macro);
    }
    else if (auto connector = qobject_cast<FilterMacroConnector*>(node))
    {
        deleteMacroConnector(connector);
    }
    else if (auto group = qobject_cast<FilterGroup*>(node))
    {
        deleteGroup(group);
    }
}

void GraphEditor::deleteFilter(FilterNode* node)
{
    if (!m_pipeController)
    {
        return;
    }
    m_pipeController->deleteEdges(node->get_out_edges());
    m_pipeController->deleteEdges(node->get_in_edges());
    //Filter isn't connected (anymore), so just remove the filter in cpp and qml
    //Search and delete filter and headerfilterInfo in file (cpp)
    m_graphModelVisualizer->eraseSingleFilter(node->ID());
    //Delete filter (qml)
    m_graphModelVisualizer->filterGraph()->removeNode(node);
}


void GraphEditor::deleteMacro(FilterMacro *node)
{
    m_pipeController->deleteEdges(node->get_out_edges());
    m_pipeController->deleteEdges(node->get_in_edges());
    GraphHelper{m_graphModelVisualizer->graph()}.erase(node);

    m_graphModelVisualizer->filterGraph()->removeNode(node);
}

void GraphEditor::deleteMacroConnector(FilterMacroConnector *node)
{
    m_pipeController->deleteEdges(node->get_out_edges());
    m_pipeController->deleteEdges(node->get_in_edges());
    GraphHelper{m_graphModelVisualizer->graph()}.erase(node);

    m_graphModelVisualizer->filterGraph()->removeNode(node);
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

void GraphEditor::deletePort(FilterPort* node)
{
    if (!m_pipeController)
    {
        return;
    }
    if (node->type() == 3)
    {
        //Input port (type == 3)
        if (node->gotPartner() == true)
        {
            QUuid inputFilterID{};
            QUuid inputFilterConnectorID{};
            bool connected = false;
            //Filter has partner, so the portPartners have to be updated in qml and cpp
            //port can have multiple partnerPorts and there also can be multiple pipes
            if (node->get_in_nodes().size() == 1)       //Check if input port is connected
            {
                auto inEdge = node->get_in_edges().at(0);
                //Get input filter instance ID and input filter connector ID
                inputFilterID = getId(inEdge->getSource());
                inputFilterConnectorID = dynamic_cast<FilterConnector*>(inEdge->getItem()->getSourceItem())->ID();
                connected = true;
            }
            auto partnerPorts = node->getFilterPortPartner();
            for (auto const partnerPort : partnerPorts)
            {
                //instead of let the output ports stay, they will be removed now
                for (auto const &outEdge : partnerPort->get_out_edges())
                {
                    auto edge = outEdge;
                    auto outputFilterID = getId(edge->getDestination());
                    auto outputFilterConnector = dynamic_cast<FilterConnector*>(edge->getItem()->getDestinationItem());
                    auto outputFilterConnectorID = outputFilterConnector->ID();
                    //Because of removing the ports, the input connectors of the output filters have to be cleared (qml)
                    outputFilterConnector->getInEdgeItems().clear();
                    if (connected)
                    {
                        //If input port is connected a pipe is present if partnerPort is connected too --> remove this pipe (cpp)
                        //Remove the pipe (cpp)
                        m_pipeController->erasePipeSourceDestination(inputFilterID,   inputFilterConnectorID, outputFilterID, outputFilterConnectorID);
                    }
                }
                //remove partnerPorts cpp and qml
                m_graphModelVisualizer->erasePort(partnerPort->ID());
                m_graphModelVisualizer->filterGraph()->removeNode(partnerPort);
            }
        }
    }
    else
    {
        //Output port (type == 2)
        if (node->gotPartner() == true)
        {
            QUuid sourceFilterID{};
            QUuid sourceFilterConnectorID{};
            bool partnerConnected = false;
            //Filter has partner, so the portPartner has to be checked if he is connected
            //if the portPartner is connected there can be one or multiple pipes
            if (node->getOneFilterPortPartner()->get_in_edges().size() == 1)
            {
                auto inEdge = node->getOneFilterPortPartner()->get_in_edges().at(0);
                sourceFilterID = getId(inEdge->getSource());
                sourceFilterConnectorID = dynamic_cast<FilterConnector*>(inEdge->getItem()->getSourceItem())->ID();
                partnerConnected = true;
            }
            //First check if the port is connected
            if (node->get_out_nodes().size() > 0)
            {
                //Filter is connected, so one or multiple pipes are connected
                for (auto const &outEdge : node->get_out_edges())
                {
                    auto edge = outEdge;
                    auto destFilterID = getId(edge->getDestination());
                    auto destFilterConnector = dynamic_cast<FilterConnector*>(edge->getItem()->getDestinationItem());
                    auto destFilterConnectorID = destFilterConnector->ID();
                    //Clean the input connector of the destination filter, so an other pipe can be connected
                    destFilterConnector->getInEdgeItems().clear();
                    //if source port is also connected remove pipe (cpp)
                    if (partnerConnected)
                    {
                        //Remove the pipe (cpp)
                        m_pipeController->erasePipeSourceDestination(sourceFilterID, sourceFilterConnectorID, destFilterID, destFilterConnectorID);
                    }
                }
            }
            // Update/Modify partnerPort that this portPartner will be deleted
            node->getOneFilterPortPartner()->removeFilterPortPartner(node->ID());
        }
    }
    //Delete port (cpp)
    m_graphModelVisualizer->erasePort(node->ID());
    //Delete port (qml)
    m_graphModelVisualizer->filterGraph()->removeNode(node);
}

void GraphEditor::deleteComment(FilterComment* node)
{
    //Delete comment (cpp)
    m_graphModelVisualizer->eraseComment(node->ID());
    //Delete comment (qml)
    m_graphModelVisualizer->filterGraph()->removeNode(node);
}

void GraphEditor::deleteGroup(FilterGroup* group)
{
    // copy the current group nodes to a temporary vector
    // QVector is a shared container type and here we need a deep copy
    std::vector<QPointer<qan::Node>> groupContent;
    groupContent.reserve(group->get_nodes().size());
    std::copy(group->get_nodes().begin(), group->get_nodes().end(), std::back_inserter(groupContent));

    //Delete group and content (cpp)
    GraphHelper{m_graphModelVisualizer->graph()}.erase(group);
    m_graphModelVisualizer->setGraphEdited(true);

    //Delete content
    for (auto &nodeInGroup : groupContent)
    {
        if (auto *shared = nodeInGroup.data())
        {
            if (auto filter = dynamic_cast<FilterNode*>(shared))
            {
                deleteFilter(filter);
                continue;
            }
            if (auto port = dynamic_cast<FilterPort*>(shared))
            {
                deletePort(port);
                continue;
            }
            if (auto comment = dynamic_cast<FilterComment*>(shared))
            {
                deleteComment(comment);
                continue;
            }
        }
    }

    //Delete group (qml)
    m_graphModelVisualizer->filterGraph()->removeGroup(group);
}

namespace
{

template <typename T>
void performUngroup(qan::Node *element)
{
    if (auto node = dynamic_cast<T*>(element))
    {
        node->setGroupID(-1);
    }
}

}

void GraphEditor::unGroupContent(FilterGroup* group)
{
    // copy the current group nodes to a temporary vector
    // we need to iterate over the nodes in the group and cannot have the vector be modified while iterating
    std::vector<QPointer<qan::Node>> groupContent;
    groupContent.reserve(group->get_nodes().size());
    std::copy(group->get_nodes().begin(), group->get_nodes().end(), std::back_inserter(groupContent));

    for (auto& node : groupContent)
    {
        if (!node)
        {
            continue;
        }
        performUngroup<FilterNode>(node);
        performUngroup<FilterPort>(node);
        performUngroup<FilterComment>(node);
        m_graphModelVisualizer->groupController()->ungroup(node);
    }

    //Delete group and content (cpp)
    GraphHelper{m_graphModelVisualizer->graph()}.erase(group);
    m_graphModelVisualizer->setGraphEdited(true);
    //Delete group (qml)
    m_graphModelVisualizer->filterGraph()->removeGroup(group);
}

void GraphEditor::initActualNode()
{
    if (m_actualNodeIndex == -1)
    {
        return;
    }
    m_actualNode = m_graphModelVisualizer->getFilterGraphNode(m_actualNodeIndex);
    emit actualNodeChanged();
}

void GraphEditor::initActualPort()
{
    if (m_actualPortIndex == -1)
    {
        return;
    }
    //FIXME
    //m_actualPort = m_graphModelVisualizer->searchFilterPort();
    //m_actualPort = m_graphModelVisualizer->getFilterGraphPort(m_actualPortIndex);
    emit actualPortChanged();
}

void GraphEditor::setPortPartner(FilterPort* port, const QModelIndex& partnerIndex)
{
    auto partnerPort = m_graphModelVisualizer->getFilterGraphPort(partnerIndex.row());
    if (partnerPort && partnerPort->type() == 3)
    {
        port->insertPartner(partnerPort);
        port->setDataType(partnerPort->dataType());
        port->setLabel(partnerPort->getLabel());

        partnerPort->insertPartner(port);
    }
    else
    {
        qDebug() << "Something wrong with the partner port!";
    }
}

void GraphEditor::setPipeController(PipeController *controller)
{
    if (m_pipeController == controller)
    {
        return;
    }
    m_pipeController = controller;
    disconnect(m_pipeControllerDestroyed);
    if (m_pipeController)
    {
        m_pipeControllerDestroyed = connect(m_pipeController, &PipeController::destroyed, this, std::bind(&GraphEditor::setPipeController, this, nullptr));
    }
    else
    {
        m_pipeControllerDestroyed = {};
    }
    emit pipeControllerChanged();
}


