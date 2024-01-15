#include "FilterNodeModel.h"

#include "../App_Storage/src/compatibility.h"

#include <fliplib/graphContainer.h>

#include <QRect>
#include <QDebug>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

FilterNodeModel::FilterNodeModel(QObject* parent)
    : QAbstractListModel(parent)
{ }

FilterNodeModel::~FilterNodeModel() = default;

GraphModelVisualizer* FilterNodeModel::graphModelVisualizer() const
{
    return m_graphVisualizer;
}

void FilterNodeModel::setGraphModelVisualizer(GraphModelVisualizer* graphContainer)
{
    if (m_graphVisualizer != graphContainer)
    {
        m_graphVisualizer = graphContainer;
        emit graphModelVisualizerChanged();
        if (m_graphVisualizer)
        {
            connect(this, &FilterNodeModel::graphModelVisualizerChanged, this, &FilterNodeModel::init);
            m_destroyConnection = connect(m_graphVisualizer, &GraphModelVisualizer::destroyed, this, std::bind(&FilterNodeModel::setGraphModelVisualizer, this, nullptr));
        }
    }
}

QHash<int, QByteArray> FilterNodeModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("id")},
        {Qt::UserRole, QByteArrayLiteral("name")},
        {Qt::UserRole + 1, QByteArrayLiteral("group")},
        {Qt::UserRole + 2, QByteArrayLiteral("filterId")},
        {Qt::UserRole + 3, QByteArrayLiteral("position")}
    };
}

int FilterNodeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_graph->instanceFilters.size();
}

QVariant FilterNodeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &nodeInfo = m_graph->instanceFilters.at(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return precitec::storage::compatibility::toQt(nodeInfo.id);
        case Qt::UserRole:
            return QString::fromStdString(nodeInfo.name);
        case Qt::UserRole + 1:
            return nodeInfo.group;
        case Qt::UserRole + 2:
            return precitec::storage::compatibility::toQt(nodeInfo.filterId);
        case Qt::UserRole + 3:
            return QRect{nodeInfo.position.x, nodeInfo.position.y, nodeInfo.position.width, nodeInfo.position.height};
    }

    return {};
}

void FilterNodeModel::init()
{
    emit beginResetModel();
    if (m_graphVisualizer)
    {
        if (m_graph != m_graphVisualizer->graph())
        {
            m_graph = m_graphVisualizer->graph();
        }
    }
    emit endResetModel();
}

}
}
}
}
