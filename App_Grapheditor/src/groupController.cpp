#include "groupController.h"
#include "FilterGraph.h"
#include "filterMacro.h"
#include "FilterPort.h"
#include "graphHelper.h"
#include "plausibilityController.h"

#include "fliplib/graphContainer.h"

#include "graphExporter.h"

#include <Poco/UUIDGenerator.h>

#include <vector>

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

GroupController::GroupController(QObject* parent)
    : AbstractNodeController(parent)
{
    connect(this, &GroupController::filterGraphChanged, this, &GroupController::initSelectedGroupsModel);
}

GroupController::~GroupController() = default;

void GroupController::insertAllFilterGroups()
{
    // TODO: iterator instead of index
    for (std::size_t actualGroup = 0; actualGroup < graph()->filterGroups.size(); actualGroup++)
    {
        insertOneFilterGroup(actualGroup);
    }
}

void GroupController::insertOneFilterGroup(int actualGroup)
{
    const auto groupId = getGroupID(actualGroup);
    if (groupId != -1 && std::none_of(graph()->macros.begin(), graph()->macros.end(), [groupId] (const auto &macro) { return macro.group == groupId; } ))
    {
        createGroupInGraph(groupId, QString::fromStdString(graph()->filterGroups.at(actualGroup).name), QPointF(actualGroup, actualGroup));
    }
}

int GroupController::getGroupID(int actualGroup) const
{
    //TODO stupid?
    if (actualGroup != -1 && actualGroup >= 0)
    {
        return graph()->filterGroups.at(actualGroup).number;
    }
    return -1;
}

void GroupController::updateGroupLabel(FilterGroup *group, const QString &label)
{
    if (auto g = GraphHelper{graph()}.find(group))
    {
        g->name = label.toStdString();
    }
    group->setLabel(label);
    emit changed();
}

namespace
{

template <typename T, typename U>
void updatePositionInFilterGraph(T *nodeCpp, const U &node, int gridSize, bool useGridSizeAutomatically)
{
    QPointF points = {node->getItemGeometry().x(),node->getItemGeometry().y()};
    if (node->getGroup())
    {
        points = node->getGroup()->getItem()->mapToItem(node->getGroup()->getGraph()->getContainerItem(), points);
    }
    if (gridSize != 1 && useGridSizeAutomatically)
    {
        points.setX(points.x() - (static_cast<int>(points.x()) % gridSize));
        points.setY(points.y() - (static_cast<int>(points.y()) % gridSize));
        node->getItem()->setRect({points.x(), points.y(), node->getItemGeometry().width(), node->getItemGeometry().height()});
    }
    nodeCpp->position.x = points.x();
    nodeCpp->position.y = points.y();
}

template <typename T>
void updatePositionInFilterGraph(qan::Node *node, GraphHelper &helper, int gridSize, bool useGridSizeAutomatically)
{
    if (auto filterNode = dynamic_cast<T*>(node))
    {
        if (auto nodeCpp = helper.find(filterNode))
        {
            updatePositionInFilterGraph(nodeCpp, filterNode, gridSize, useGridSizeAutomatically);
        }
    }
}

}

void GroupController::updateGroupPosition(FilterGroup *group, const QPointF &point)
{
    group->getItem()->setRect({point.x(), point.y(), group->getItem()->width(), group->getItem()->height()});
    updateContentPosition(group);
}

void GroupController::updateContentPosition(FilterGroup *group)
{
    GraphHelper helper{graph()};
    for (const auto &nodeInGroup : group->get_nodes())
    {
        updatePositionInFilterGraph<FilterNode>(nodeInGroup, helper, gridSize(), useGridSizeAutomatically());
        updatePositionInFilterGraph<FilterPort>(nodeInGroup, helper, gridSize(), useGridSizeAutomatically());
        updatePositionInFilterGraph<FilterComment>(nodeInGroup, helper, gridSize(), useGridSizeAutomatically());
    }
    updateGroupContent(group);
    emit changed();
}

void GroupController::updateGroupContent(QObject* node)
{
    auto group = dynamic_cast<FilterGroup*> (node);
    const QPointF leftUpperGroupPoint{group->getItem()->x(), group->getItem()->y()};
    const QPointF rightLowerGroupPoint{group->getItem()->x() + group->getItem()->width(), group->getItem()->y() + group->getItem()->height()};

    for (const auto &nodePtr : filterGraph()->get_nodes())
    {
        const auto node = nodePtr;
        QPointF leftUpperNodePoint{node->getItem()->x(), node->getItem()->y()};
        QPointF rightLowerNodePoint{node->getItem()->x() + node->getItem()->width(), node->getItem()->y() + node->getItem()->height()};

        if (!group->hasNode(node))
        {
            if (!node->getGroup() && !qobject_cast<FilterMacro*>(node))
            {
                if (leftUpperNodePoint.x() > leftUpperGroupPoint.x() && leftUpperNodePoint.y() > leftUpperGroupPoint.y())
                {
                    if (rightLowerNodePoint.x() < rightLowerGroupPoint.x() && rightLowerNodePoint.y() < rightLowerGroupPoint.y())
                    {
                        addToGroup(node, group);
                    }
                }
            }
        }
        /*else    //Nur relevant beim Ändern der Size!      //Funktioniert noch nicht! Min Group size muss geändert werden, auf frei und zwar immer. Geht das?
        {
            leftUpperNodePoint = group->getItem()->mapToItem(group->getGraph()->getContainerItem(), leftUpperNodePoint);
            //rightUpperNodePoint = leftUpperNodePoint
            if (leftUpperNodePoint.x() < leftUpperGroupPoint.x() && leftUpperNodePoint.y() < leftUpperGroupPoint.y())
            {
                if (rightLowerNodePoint.x() > rightLowerGroupPoint.x() && rightLowerNodePoint.y() > rightLowerGroupPoint.y())
                {
                    updateGeneralAttributes(node, node->getLabel(), -1, leftUpperNodePoint.x(), leftUpperNodePoint.y(), node->getItem()->width(), node->getItem()->height());       //UpdatePosition aufrufen?
                    filterGraph()->ungroupNode(node, group);
                }
            }
        }*/
    }
}

namespace
{
template <typename T>
void updateGroupIdAndPosition(qan::Node *object, int id, GraphHelper &helper)
{
    if (auto node = qobject_cast<T*>(object))
    {
        if (auto instance = helper.find(node))
        {
            instance->group = id;
            helper.updatePosition(node, QPointF{node->getItemGeometry().x(), node->getItemGeometry().y()});
        }
    }
}

}

void GroupController::updateGroup(qan::Node *node, int id)
{
    GraphHelper helper{graph()};
    updateGroupIdAndPosition<FilterNode>(node, id, helper);
    updateGroupIdAndPosition<FilterComment>(node, id, helper);
    updateGroupIdAndPosition<FilterPort>(node, id, helper);
    emit changed();
}

void GroupController::addToGroup(qan::Node *node, FilterGroup *group)
{
    if (node->isGroup())
    {
        return;
    }
    filterGraph()->groupNode(group, node, true);
    updateGroup(node, group->ID());
    if (auto filter = qobject_cast<FilterNode*>(node))
    {
        filter->nodeGroupInteraction();
    }
}

void GroupController::ungroup(qan::Node *node)
{
    filterGraph()->ungroupNode(node, node->getGroup());
    updateGroup(node, -1);
    if (auto filter = qobject_cast<FilterNode*>(node))
    {
        filter->nodeGroupInteraction();
    }
}

void GroupController::updateGroupSize(FilterGroup *group, const QSizeF &size)
{
    group->getItem()->setRect({group->getItem()->x(), group->getItem()->y(), size.width(), size.height()});
    group->getGroupItem()->setPreferredGroupWidth(size.width());
    group->getGroupItem()->setPreferredGroupHeight(size.height());
    updateGroupContent(group);
    emit changed();
}

namespace
{
template <typename T>
void updateGroupId(QObject *object, GraphHelper &helper)
{
    if (auto node = qobject_cast<T*>(object))
    {
        node->nodeGroupInteraction();

        if (auto instance = helper.find(node))
        {
            instance->group = node->groupID();
        }
    }
}

}

void GroupController::updateGroupProperty(QObject* node)
{
    GraphHelper helper{graph()};
    updateGroupId<FilterNode>(node, helper);
    updateGroupId<FilterPort>(node, helper);
    updateGroupId<FilterComment>(node, helper);
}

int GroupController::getLowestFilterGroupID(unsigned int maximum, unsigned int actualValue) const
{
    unsigned int counter = actualValue;
    const auto &groups = graph()->filterGroups;

    while (counter <= maximum)
    {
        auto ID = std::find_if(groups.begin(), groups.end(), [counter](const auto &actualGroup) {return actualGroup.number == static_cast<int>(counter);});
        if (ID != groups.end())
        {
            counter++;
        }
        else
        {
            return counter;
        }
    }
    return -1;
}

QString GroupController::getGroupDefaultName(int groupID)
{
    if (groupID < 10)
    {
        return {"G0" + QString::number(groupID)};
    }
    return {"G" + QString::number(groupID)};
}

FilterGroup *GroupController::insertNewGroup(const QPointF &point)
{
    const auto groupID = getLowestFilterGroupID(100, 1);
    const auto name = getGroupDefaultName(groupID) + QStringLiteral(" ") + tr("Group name");

    fliplib::FilterGroup group;
    group.number = groupID;
    group.parent = -1;
    group.name = name.toStdString();
    return insertNewGroup(group, point);
}

FilterGroup *GroupController::insertNewGroup(const fliplib::FilterGroup &newGroup, const QPointF &point)
{
    graph()->filterGroups.push_back(newGroup);
    auto group = createGroupInGraph(newGroup.number, QString::fromStdString(newGroup.name), point);

    emit changed();
    return group;
}

FilterGroup *GroupController::createGroupInGraph(int groupID, const QString &label, const QPointF &position)
{
    auto newGroup = qobject_cast<FilterGroup*>(filterGraph()->insertFilterGroup());
    newGroup->setID(groupID);
    newGroup->setLabel(label);

    initGroupItem(newGroup->getGroupItem(), position);
    return newGroup;
}

void GroupController::initGroupItem(qan::GroupItem *groupItem, const QPointF &position)
{
    if (!groupItem)
    {
        return;
    }
    groupItem->setMinimumGroupWidth(100);
    groupItem->setMinimumGroupHeight(50);
    groupItem->setDroppable(false);
    qobject_cast<FilterGroup*>(groupItem->getGroup())->setItemGeometry({position.x(), position.y(), 100, 50});
}

FilterGroup *GroupController::find(int id) const
{
    const auto &groups = filterGraph()->get_groups();
    for (auto weakGroup : groups)
    {
        if (auto filterGroup = dynamic_cast<FilterGroup*>(weakGroup))
        {
            if (filterGroup->ID() == id)
            {
                return filterGroup;
            }
        }
    }
    return nullptr;
}

int GroupController::idFromName(const QString &name) const
{
    if (name.isNull() || name == "No Group")
    {
        return -1;
    }
    const auto groupName = name.toStdString();
    auto it = std::find_if(graph()->filterGroups.begin(), graph()->filterGroups.end(), [groupName](fliplib::FilterGroup const &actualFilterGroup) {return actualFilterGroup.name == groupName;});
    if (it != graph()->filterGroups.end())
    {
        return it->number;
    }
    else
    {
        return -1;
    }
}


namespace
{
template <typename T>
void updateGeometry(qan::Node *nodesPtr, GroupController *groupController)
{
    if (auto node = dynamic_cast<T*>(nodesPtr))
    {
        if (node->groupID() != -1)
        {
            if (auto foundGroup = groupController->find(node->groupID()))
            {
                foundGroup->updateGroupGeometry( node->getItemGeometry() );
            }
        }
    }
}

template <typename T>
void nodeToAdd(const std::vector<int> &groupIDs, std::vector<qan::Node*> &nodes, qan::Node *nodesPtr, GroupController *groupController)
{
    if (auto node = dynamic_cast<T*>(nodesPtr))
    {
        if (node->groupID() != -1)
        {
            auto foundGroup = groupController->find(node->groupID());
            if (foundGroup && std::find(groupIDs.begin(), groupIDs.end(), foundGroup->ID()) != groupIDs.end())
            {
                foundGroup->updateGroupGeometry( node->getItemGeometry() );
                nodes.push_back(node);
            }
        }
    }
}

template <typename T>
void groupNode(qan::Node *node, GroupController *groupController, FilterGraph *filterGraph)
{
    if (auto filter = dynamic_cast<T*>(node))
    {
        if (auto foundGroup = groupController->find(filter->groupID()))
        {
            filterGraph->groupNode(foundGroup, filter, true);
        }
    }
}

template <typename T>
void addToGroupOnOverlap(qan::Node *nodesPtr, FilterGraph *filterGraph)
{
    if (auto node = dynamic_cast<T*>(nodesPtr))
    {
        if (node->groupID() == -1)
        {
            //Check if node and a group overlap
            for (const auto &groupPtr : qAsConst(filterGraph->get_groups().getContainer()))
            {
                if (auto group = dynamic_cast<FilterGroup*> (groupPtr); group->getItemGeometry().contains(node->getItemGeometry()))
                {
                    node->setGroupID(group->ID());
                }
            }
        }
    }
}

}

void GroupController::connectNodesToFilterGroups(const fliplib::GraphContainer &copyGraph)
{
    std::vector<int> groupIDs;
    std::vector<qan::Node*> nodes;
    for (const auto &group : copyGraph.filterGroups)
    {
        groupIDs.push_back(group.number);
    }
    for (const auto &nodesPtr : qAsConst(filterGraph()->get_nodes().getContainer()))
    {
        nodeToAdd<FilterNode>(groupIDs, nodes, nodesPtr, this);
        nodeToAdd<FilterPort>(groupIDs, nodes, nodesPtr, this);
        nodeToAdd<FilterComment>(groupIDs, nodes, nodesPtr, this);
    }
    for (const auto &node : nodes)
    {
        groupNode<FilterNode>(node, this, filterGraph());
        groupNode<FilterPort>(node, this, filterGraph());
        groupNode<FilterComment>(node, this, filterGraph());
    }
}

void GroupController::connectNodesToFilterGroups()
{
    for (const auto &nodesPtr : qAsConst(filterGraph()->get_nodes().getContainer()))
    {
        updateGeometry<FilterNode>(nodesPtr, this);
        updateGeometry<FilterPort>(nodesPtr, this);
        updateGeometry<FilterComment>(nodesPtr, this);
    }
    //Look if there is a filter node, port or comment in an area of a group
    for (const auto &nodesPtr : qAsConst(filterGraph()->get_nodes().getContainer()))
    {
        addToGroupOnOverlap<FilterNode>(nodesPtr, filterGraph());
        addToGroupOnOverlap<FilterPort>(nodesPtr, filterGraph());
        addToGroupOnOverlap<FilterComment>(nodesPtr, filterGraph());
    }
    //
    for (const auto &nodesPtr : qAsConst(filterGraph()->get_nodes().getContainer()))
    {
        groupNode<FilterNode>(nodesPtr, this, filterGraph());
        groupNode<FilterPort>(nodesPtr, this, filterGraph());
        groupNode<FilterComment>(nodesPtr, this, filterGraph());
    }
}

void GroupController::initSelectedGroupsModel()
{
    auto fg = filterGraph();
    if (!fg)
    {
        return;
    }
    auto model = fg->selectedGroupsModel();
    if (!model)
    {
        return;
    }
    connect(model, &QAbstractListModel::dataChanged, this, &GroupController::checkCanBeExportedToMacro);
    connect(model, &QAbstractListModel::modelReset, this, &GroupController::checkCanBeExportedToMacro);
    connect(model, &QAbstractListModel::rowsInserted, this, &GroupController::checkCanBeExportedToMacro);
    connect(model, &QAbstractListModel::rowsRemoved, this, &GroupController::checkCanBeExportedToMacro);
    checkCanBeExportedToMacro();
}

void GroupController::checkCanBeExportedToMacro()
{
    auto fg = filterGraph();
    if (!fg)
    {
        setCanBeExportedToMacro(false);
        return;
    }
    auto model = fg->selectedGroupsModel();
    if (!model)
    {
        setCanBeExportedToMacro(false);
        return;
    }
    if (model->rowCount() != 1)
    {
        setCanBeExportedToMacro(false);
        return;
    }
    if (!m_plausibilityController)
    {
        setCanBeExportedToMacro(false);
        return;
    }
    if (auto group = model->data(model->index(0,0), Qt::UserRole + 1).value<FilterGroup*>())
    {
        setCanBeExportedToMacro(m_plausibilityController->checkGroup(group));
        return;
    }
    setCanBeExportedToMacro(false);
}

void GroupController::exportSelectedToMacro(const QString &directory, const QString &name, const QString &comment)
{
    checkCanBeExportedToMacro();
    if (!canBeExportedToMacro())
    {
        return;
    }
    auto fg = filterGraph();
    if (!fg)
    {
        return;
    }
    auto model = fg->selectedGroupsModel();
    if (!model)
    {
        return;
    }
    if (model->rowCount() != 1)
    {
        return;
    }
    auto group = model->data(model->index(0,0), Qt::UserRole + 1).value<FilterGroup*>();
    if (!group)
    {
        return;
    }
    fliplib::GraphContainer macro;
    macro.id = Poco::UUIDGenerator::defaultGenerator().createOne();
    macro.parameterSet = Poco::UUIDGenerator::defaultGenerator().createOne();
    macro.name = name.toStdString();
    macro.comment = comment.toStdString();

    for (const auto &filter : graph()->instanceFilters)
    {
        if (filter.group != group->ID())
        {
            continue;
        }
        macro.instanceFilters.push_back(filter);
        macro.instanceFilters.back().group = -1;
    }
    for (const auto &description : graph()->filterDescriptions)
    {
        if (std::any_of(macro.instanceFilters.begin(), macro.instanceFilters.end(), [description] (const auto &filter) { return filter.filterId == description.id; }) &&
            std::none_of(macro.filterDescriptions.begin(), macro.filterDescriptions.end(), [description] (const auto &target) { return target.id == description.id; }))
        {
            macro.filterDescriptions.push_back(description);
        }
    }

    for (const auto &port : graph()->ports)
    {
        if (port.group != group->ID())
        {
            continue;
        }
        if (port.type == 5)
        {
            // a comment
            macro.ports.push_back(port);
            macro.ports.back().group = -1;
            continue;
        }

        // find port in model
        const auto &nodes = fg->get_nodes();
        const auto portId = QUuid::fromString(QString::fromStdString(port.id.toString()));
        auto it = std::find_if(nodes.begin(), nodes.end(),
            [&portId] (const auto &node)
            {
                if (auto filterPort = dynamic_cast<FilterPort*>(node); filterPort && filterPort->ID() == portId)
                {
                    return true;
                }
                return false;
            });
        if (it == nodes.end())
        {
            continue;
        }
        auto filterPort = dynamic_cast<FilterPort*>(*it);

        fliplib::Macro::Connector connector;
        connector.id = port.id;
        connector.name = filterPort->getLabel().toStdString();
        connector.type = fliplib::PipeConnector::DataType(filterPort->dataType());
        connector.position = port.position;
        if (filterPort->type() == 3)
        {
            macro.outConnectors.push_back(connector);
            fliplib::Pipe pipe;
            pipe.receiver = connector.id;
            pipe.receiverConnectorGroup = -1;
            pipe.receiverConnectorId = connector.id;
            pipe.receiverConnectorName = connector.name;
            for (const auto &edge : filterPort->get_in_edges())
            {
                auto edgePtr = edge;
                if (!edgePtr)
                {
                    continue;
                }
                auto *filterNode = qobject_cast<FilterNode*>(edgePtr->getSource());
                auto *filterConnector = qobject_cast<FilterConnector*>(edgePtr->getItem()->getSourceItem());

                pipe.id = Poco::UUIDGenerator::defaultGenerator().createOne();
                pipe.sender = Poco::UUID(filterNode->ID().toString(QUuid::WithoutBraces).toStdString());
                pipe.senderConnectorId = Poco::UUID(filterConnector->ID().toString(QUuid::WithoutBraces).toStdString());
                pipe.senderConnectorName = filterConnector->getLabel().toStdString();

                macro.pipes.push_back(pipe);
            }
        }
        else
        {
            macro.inConnectors.push_back(connector);
            fliplib::Pipe pipe;
            pipe.sender = connector.id;
            pipe.senderConnectorId = connector.id;
            pipe.senderConnectorName = connector.name;
            for (const auto &edge : filterPort->get_out_edges())
            {
                auto edgePtr = edge;
                if (!edgePtr)
                {
                    continue;
                }
                auto *filterNode = qobject_cast<FilterNode*>(edgePtr->getDestination());
                auto *filterConnector = qobject_cast<FilterConnector*>(edgePtr->getItem()->getDestinationItem());

                pipe.id = Poco::UUIDGenerator::defaultGenerator().createOne();
                pipe.receiver = Poco::UUID(filterNode->ID().toString(QUuid::WithoutBraces).toStdString());
                pipe.receiverConnectorId = Poco::UUID(filterConnector->ID().toString(QUuid::WithoutBraces).toStdString());
                pipe.receiverConnectorName = filterConnector->tag().toStdString();
                pipe.receiverConnectorGroup = filterConnector->group();

                macro.pipes.push_back(pipe);
            }
        }
    }

    for (const auto &pipe : graph()->pipes)
    {
        const auto sourceInFilters = std::any_of(macro.instanceFilters.begin(), macro.instanceFilters.end(), [pipe] (const auto &filter) { return filter.id == pipe.sender; });
        const auto destinationInFilters = std::any_of(macro.instanceFilters.begin(), macro.instanceFilters.end(), [pipe] (const auto &filter) { return filter.id == pipe.receiver; });

        if (sourceInFilters && destinationInFilters)
        {
            macro.pipes.push_back(pipe);
        }
    }

    storage::GraphExporter exporter{macro};
    auto filePath = directory;
    if (!directory.endsWith(QDir::separator()))
    {
        filePath.append(QDir::separator());
    }
    exporter.setFileName(filePath + name + QStringLiteral(".xml"));
    exporter.exportToXml();
}

void GroupController::setCanBeExportedToMacro(bool set)
{
    if (m_canBeExportedToMacro == set)
    {
        return;
    }
    m_canBeExportedToMacro = set;
    emit canBeExportedToMacroChanged();
}

void GroupController::setPlausibilityController(PlausibilityController *plausibilityController)
{
    if (m_plausibilityController == plausibilityController)
    {
        return;
    }
    m_plausibilityController = plausibilityController;
    disconnect(m_plausibilityDestroyed);
    if (m_plausibilityController)
    {
        m_plausibilityDestroyed = connect(m_plausibilityController, &QObject::destroyed, this, std::bind(&GroupController::setPlausibilityController, this, nullptr));
    }
    emit plausibilityControllerChanged();
}

}
}
}
}
