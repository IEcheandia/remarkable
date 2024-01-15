#include "FilterGroupModel.h"

using namespace precitec::gui::components::grapheditor;

FilterGroupModel::FilterGroupModel(QObject* parent) : QAbstractListModel(parent)
{
    connect(this, &FilterGroupModel::graphModelVisualizerChanged, this, &FilterGroupModel::init);
}

FilterGroupModel::~FilterGroupModel() = default;

GraphModelVisualizer* FilterGroupModel::graphModelVisualizer() const
{
    return m_graphVisualizer;
}

void FilterGroupModel::setGraphModelVisualizer(GraphModelVisualizer* graphContainer)
{
    if (m_graphVisualizer == graphContainer)
    {
        return;
    }
    m_graphVisualizer = graphContainer;
    emit graphModelVisualizerChanged();
    if (m_graphVisualizer)
    {
        m_destroyConnection = connect(m_graphVisualizer, &GraphModelVisualizer::destroyed, this, std::bind(&FilterGroupModel::setGraphModelVisualizer, this, nullptr));
        connect(m_graphVisualizer, &GraphModelVisualizer::graphChanged, this, &FilterGroupModel::init);
        connect(m_graphVisualizer, &GraphModelVisualizer::graphEditedChanged, this, &FilterGroupModel::init);
    }
}

QVariant FilterGroupModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &groupInfo = m_graph->filterGroups.at(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return groupInfo.number;
        case Qt::UserRole:
            return QString::fromStdString(groupInfo.name);
    }

    return {};
}

QHash<int, QByteArray> FilterGroupModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("id")},
        {Qt::UserRole, QByteArrayLiteral("name")}
    };
}

int FilterGroupModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_graph->filterGroups.size();
}

void FilterGroupModel::init()
{
    if (m_graphVisualizer)
    {
        emit beginResetModel();

        m_graph = m_graphVisualizer->graph();
        emit graphChanged();

        emit endResetModel();
    }
}


