#include "FilterGraph.h"
#include "filterMacro.h"
#include "filterMacroConnector.h"

using namespace precitec::gui::components::grapheditor;

FilterGraph::~FilterGraph() = default;

void FilterGraph::clear()
{
    clearSelection();
    clearGraph();
    setGraphName({});
    setGraphID({});
    setGraphComment({});
    setGraphPath({});
}

QString FilterGraph::graphName() const
{
    return m_graphName;
}

void FilterGraph::setGraphName(const QString& name)
{
    if (m_graphName != name)
    {
        m_graphName = name;
        emit graphNameChanged();
    }
}

QUuid FilterGraph::graphID() const
{
    return m_graphID;
}

void FilterGraph::setGraphID(const QUuid& id)
{
    if (m_graphID != id)
    {
        m_graphID = id;
        emit graphIDChanged();
    }
}

QString FilterGraph::graphComment() const
{
    return m_graphComment;
}

void FilterGraph::setGraphComment(const QString& comment)
{
    if (m_graphComment != comment)
    {
        m_graphComment = comment;
        emit graphCommentChanged();
    }
}

QString FilterGraph::graphPath() const
{
    return m_graphPath;
}

void FilterGraph::setGraphPath(const QString& path)
{
    if (m_graphPath != path)
    {
        m_graphPath = path;
        emit graphPathChanged();
    }
}

void FilterGraph::setPortDelegateToFilterPort()
{
    const auto engine = qmlEngine(this);
    qmlSetPortDelegate(FilterConnector::delegate(*engine));
}

void FilterGraph::destroyPortDelegate()
{
    if (getPortDelegate()->isReady())           //Fix for crashing of the gui after closing the view
    {
        QQmlEngine::setObjectOwnership(getPortDelegate(), QQmlEngine::JavaScriptOwnership);
    }
}

qan::Node *FilterGraph::insertMacro()
{
    return insertNode<FilterMacro>(nullptr);
}

qan::Node *FilterGraph::insertMacroConnector()
{
    return insertNode<FilterMacroConnector>(nullptr);
}

qan::Node * FilterGraph::insertFilterNode()
{
    return insertNode<FilterNode>(nullptr);
}

qan::Group * FilterGraph::insertFilterGroup()
{
    auto group = insertGroup<FilterGroup>();
    auto filterGroup = dynamic_cast<FilterGroup*>(group);

    connect(filterGroup, &qan::Group::labelChanged, this, std::bind(&FilterGraph::groupLabelChanged, this, filterGroup));
    connect(filterGroup, &FilterGroup::groupSizeChanged, this, &FilterGraph::groupSizeChanged);

    return filterGroup;
}

qan::PortItem * FilterGraph::insertFilterConnector(qan::Node* node, qan::NodeItem::Dock dock, qan::PortItem::Type portType, QString label, QString id)
{
    auto port = insertPort(node, dock, portType, label, id);
    auto connector = dynamic_cast<FilterConnector*> (port);
    auto notifyConnectorHovered =
    [this] (FilterConnector* connector){if (connector != nullptr && connector->getNode() != nullptr)
    {
        emit this->connectorHovered(connector);
    }
    else
    {
        emit this->connectorHovered(nullptr);
    }
    };
    connect( connector, &FilterConnector::hoverEvent, this, notifyConnectorHovered);
    return connector;
}

qan::Node * FilterGraph::insertFilterPort()
{
    return insertNode<FilterPort>(nullptr);
}

qan::Node * FilterGraph::insertFilterComment()
{
    return insertNode<FilterComment>(nullptr);
}

void FilterGraph::selectNode(qan::Node* node)
{
    addToSelection(*node);
}

void FilterGraph::selectNodes(const QRectF &boundary)
{
    clearSelection();
    std::list<qan::Node*> nodes;

    for (const auto &nodePtr : qAsConst(get_nodes()))
    {
        auto size = QRectF{nodePtr->getItem()->x(), nodePtr->getItem()->y(), nodePtr->getItem()->width(), nodePtr->getItem()->height()};
        if (nodePtr->getGroup())
        {
            QPointF points = nodePtr->getGroup()->getItem()->mapToItem(nodePtr->getGroup()->getGraph()->getContainerItem(), {size.x(),size.y()});
            size = {points.x(), points.y(), size.width(), size.height()};
        }
        if (boundary.contains(size))
        {
            nodes.emplace_back(nodePtr);
        }

    }

    for (const auto &nodePtr : nodes)
    {
        selectNode(nodePtr);
    }
}

QAbstractItemModel *FilterGraph::selectedNodesModel() const
{
    return getSelectedNodes().model();
}

QAbstractItemModel *FilterGraph::selectedGroupsModel() const
{
    return getSelectedGroups().model();
}

void FilterGraph::setGroupName(const QString &name)
{
    if (m_groupName != name)
    {
        m_groupName = name;
        emit groupNameChanged();
    }
}
