#include "NodeModel.h"
#include "filterMacro.h"

using namespace precitec::gui::components::grapheditor;

NodeModel::NodeModel(QObject* parent) : QAbstractListModel(parent)
{   }

NodeModel::~NodeModel() = default;

GraphModelVisualizer * NodeModel::graphModelVisualizer() const
{
    return m_graphVisualizer;
}

void NodeModel::setGraphModelVisualizer(GraphModelVisualizer* filterGraph)
{
    if (m_graphVisualizer != filterGraph)
    {
        m_graphVisualizer = filterGraph;
        emit graphModelVisualizerChanged();
        if (m_graphVisualizer)
        {
            m_destroyConnection = connect(m_graphVisualizer, &GraphModelVisualizer::destroyed, this, std::bind(&NodeModel::setGraphModelVisualizer, this, nullptr));
        }
    }
}

QHash<int, QByteArray> NodeModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("id")},
        {Qt::UserRole, QByteArrayLiteral("name")},
        {Qt::UserRole + 1, QByteArrayLiteral("nodeType")},
        {Qt::UserRole + 2, QByteArrayLiteral("type")},
        {Qt::UserRole + 3, QByteArrayLiteral("group")},
        {Qt::UserRole + 4, QByteArrayLiteral("pointer")}
    };
}

int NodeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_graphVisualizer->filterGraph()->get_node_count();
}

QVariant NodeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &node = m_graphVisualizer->filterGraph()->get_nodes().at(index.row());

    FilterNode* filterNode{nullptr};
    FilterPort* filterPort{nullptr};
    FilterComment* filterComment{nullptr};
    FilterGroup* filterGroup{nullptr};

    FilterMacro *filterMacro = qobject_cast<FilterMacro*>(node);

    if (dynamic_cast<FilterNode*>(node))
    {
        filterNode = dynamic_cast<FilterNode*>(node);
    }
    else if (dynamic_cast<FilterPort*>(node))
    {
        filterPort = dynamic_cast<FilterPort*>(node);
    }
    else if (dynamic_cast<FilterComment*>(node))
    {
        filterComment = dynamic_cast<FilterComment*>(node);
    }
    else if (dynamic_cast<FilterGroup*>(node))
    {
        filterGroup = dynamic_cast<FilterGroup*>(node);
    }

    if (filterNode)
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return filterNode->ID();
            case Qt::UserRole:
                return filterNode->getLabel();
            case Qt::UserRole + 1:
                return QStringLiteral("Filternode");
            case Qt::UserRole + 2:
                return filterNode->type().remove("precitec::filter::");
            case Qt::UserRole + 3:
                return filterNode->getGroup() ? filterNode->getGroup()->getLabel().remove(3,filterNode->getGroup()->getLabel().size()) : QStringLiteral("No G");
            case Qt::UserRole + 4:
                return QVariant::fromValue(filterNode);
        }
    }

    if (filterMacro)
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return filterMacro->ID();
            case Qt::UserRole:
                return filterMacro->getLabel();
            case Qt::UserRole + 1:
                return QStringLiteral("Filtermacro");
            case Qt::UserRole + 4:
                return QVariant::fromValue(filterMacro);
        }
    }

    if (filterPort)
    {
        bool outputPort = true;
        if (filterPort->type() == 3)
        {
            outputPort = false;
        }

        switch (role)
        {
            case Qt::DisplayRole:
                return filterPort->ID();
            case Qt::UserRole:
                return filterPort->getLabel();
            case Qt::UserRole + 1:
                return QStringLiteral("Filterport");
            case Qt::UserRole + 2:
                return outputPort ? QStringLiteral("Output port") : QStringLiteral("Input port");
            case Qt::UserRole + 3:
                return filterPort->getGroup() ? filterPort->getGroup()->getLabel().remove(3,filterPort->getGroup()->getLabel().size()) : QStringLiteral("No G");
            case Qt::UserRole + 4:
                return QVariant::fromValue(filterPort);
        }
    }

    if (filterComment)
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return filterComment->ID();
            case Qt::UserRole:
                return filterComment->getLabel();
            case Qt::UserRole + 1:
                return QStringLiteral("Filtercomment");
            case Qt::UserRole + 2:
                return QStringLiteral("Comment");
            case Qt::UserRole + 3:
                return filterComment->getGroup() ? filterComment->getGroup()->getLabel().remove(3,filterComment->getGroup()->getLabel().size()) : QStringLiteral("No G");
            case Qt::UserRole + 4:
                return QVariant::fromValue(filterComment);
        }
    }

    if (filterGroup)
    {
        switch (role)
        {
            case Qt::DisplayRole:
                return filterGroup->ID();
            case Qt::UserRole:
                return filterGroup->getLabel();
            case Qt::UserRole + 1:
                return QStringLiteral("Filtergroup");
            case Qt::UserRole + 2:
                return QStringLiteral("Group");
            case Qt::UserRole + 3:
                return QStringLiteral("");
            case Qt::UserRole + 4:
                return QVariant::fromValue(filterGroup);
        }
    }

    return {};
}

qan::Node * NodeModel::getClickedNode ( const QModelIndex& index )
{
    return m_graphVisualizer->filterGraph()->get_nodes().at(index.row());
}

