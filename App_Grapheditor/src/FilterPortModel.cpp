#include "FilterPortModel.h"

#include "GraphModelVisualizer.h"
#include "../App_Storage/src/compatibility.h"

#include <fliplib/graphContainer.h>

#include <QUuid>
#include <QRect>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{
FilterPortModel::FilterPortModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(this, &FilterPortModel::graphModelVisualizerChanged, this, &FilterPortModel::init);
}

FilterPortModel::~FilterPortModel() = default;

GraphModelVisualizer* FilterPortModel::graphModelVisualizer() const
{
    return m_graphVisualizer;
}

void FilterPortModel::setGraphModelVisualizer(GraphModelVisualizer* graphContainer)
{
    if (m_graphVisualizer != graphContainer)
    {
        m_graphVisualizer = graphContainer;
        disconnect(m_destroyConnection);
        if (m_graphVisualizer)
        {
            m_destroyConnection = connect(m_graphVisualizer, &GraphModelVisualizer::destroyed, this, std::bind(&FilterPortModel::setGraphModelVisualizer, this, nullptr));
            connect(m_graphVisualizer, &GraphModelVisualizer::graphChanged, this, &FilterPortModel::init);
            connect(m_graphVisualizer, &GraphModelVisualizer::graphEditedChanged, this, &FilterPortModel::init);
        }
        emit graphModelVisualizerChanged();
    }
}

QHash<int, QByteArray> FilterPortModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("id")},
        {Qt::UserRole, QByteArrayLiteral("name")},
        {Qt::UserRole + 1, QByteArrayLiteral("group")},
        {Qt::UserRole + 2, QByteArrayLiteral("portType")},
        {Qt::UserRole + 3, QByteArrayLiteral("position")},
        {Qt::UserRole + 4, QByteArrayLiteral("dataType")}
    };
}

int FilterPortModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    if (!m_graph)
    {
        return 0;
    }
    return m_graph->ports.size();
}

QVariant FilterPortModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_graph)
    {
        return {};
    }

    const auto &portInfo = m_graph->ports.at(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return precitec::storage::compatibility::toQt(portInfo.id);
        case Qt::UserRole:
            return QString::fromStdString(portInfo.receiverName);
        case Qt::UserRole + 1:
            return portInfo.group;
        case Qt::UserRole + 2:
            return portInfo.type;
        case Qt::UserRole + 3:
            return QRect{portInfo.position.x, portInfo.position.y, portInfo.position.width, portInfo.position.height};
        case Qt::UserRole + 4:
            return QString::fromStdString(portInfo.senderName);
    }

    return {};
}

QObject* FilterPortModel::getPort(const QModelIndex &index)
{
    if (!index.isValid() || !m_graph || m_graph->ports.empty())
    {
        return {};
    }

    auto portID = precitec::storage::compatibility::toQt(m_graph->ports.at(index.row()).id);
    for (auto const &node : m_graphVisualizer->filterGraph()->get_nodes())
    {
        if (dynamic_cast<FilterPort*>(node))
        {
            const auto port = dynamic_cast<FilterPort*>(node);
            if (port->ID() == portID)
            {
                return port;
            }
        }
    }
    return {};
}

void FilterPortModel::init()
{
    emit beginResetModel();
    if (m_graphVisualizer)
    {
        if (m_graph != m_graphVisualizer->graph())
        {
            m_graph = m_graphVisualizer->graph();
        }
    }
    else
    {
        m_graph = nullptr;
    }
    emit endResetModel();
}

}
}
}
}

