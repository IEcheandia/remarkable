#include "filterInstanceModel.h"
#include "graphModel.h"
#include "subGraphModel.h"

#include "../../App_Storage/src/compatibility.h"

namespace precitec
{
namespace storage
{

FilterInstanceModel::FilterInstanceModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(this, &FilterInstanceModel::graphIdChanged, this, &FilterInstanceModel::init);
    connect(this, &FilterInstanceModel::graphModelChanged, this, &FilterInstanceModel::init);
    connect(this, &FilterInstanceModel::subGraphModelChanged, this, &FilterInstanceModel::init);
}

FilterInstanceModel::~FilterInstanceModel() = default;

QHash<int, QByteArray> FilterInstanceModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("display")},
        {Qt::UserRole, QByteArrayLiteral("uuid")},
        {Qt::UserRole+1, QByteArrayLiteral("group")},
        {Qt::UserRole+2, QByteArrayLiteral("visibleAttributes")},
        {Qt::UserRole+3, QByteArrayLiteral("maxVisibleUserLevel")},
        {Qt::UserRole+4, QByteArrayLiteral("filterId")}
    };
}

void FilterInstanceModel::setGraphId(const QUuid &id)
{
    if (m_graphId == id)
    {
        return;
    }
    m_graphId = id;
    emit graphIdChanged();
}

QVariant FilterInstanceModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(m_graph.instanceFilters.at(index.row()).name);
    case Qt::UserRole:
        return compatibility::toQt(m_graph.instanceFilters.at(index.row()).id);
    case Qt::UserRole + 1:
        return m_graph.instanceFilters.at(index.row()).group;
    case Qt::UserRole + 2:
        return hasVisibleAttributes(index);
    case Qt::UserRole + 3:
        return maxVisibleUserLevel(index);
    case Qt::UserRole + 4:
        return compatibility::toQt(m_graph.instanceFilters.at(index.row()).filterId).toString(QUuid::WithoutBraces);
    }
    return QVariant{};
}

int FilterInstanceModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_graph.instanceFilters.size();
}

void FilterInstanceModel::setGraphModel(precitec::storage::GraphModel* model)
{
    if (m_graphModel == model)
    {
        return;
    }
    m_graphModel = model;
    disconnect(m_destroyConnection);
    disconnect(m_graphInit);
    m_destroyConnection = QMetaObject::Connection{};
    m_graphInit = {};
    if (m_graphModel)
    {
        m_destroyConnection = connect(m_graphModel, &GraphModel::destroyed, this, std::bind(&FilterInstanceModel::setGraphModel, this, nullptr));
        m_graphInit = connect(m_graphModel, &GraphModel::modelReset, this, &FilterInstanceModel::init);
    }
    emit graphModelChanged();
}

void FilterInstanceModel::setSubGraphModel(precitec::storage::SubGraphModel* model)
{
    if (m_subGraphModel == model)
    {
        return;
    }
    m_subGraphModel = model;
    disconnect(m_subGraphDestroyConnection);
    m_subGraphDestroyConnection = QMetaObject::Connection{};
    if (m_subGraphModel)
    {
        m_subGraphDestroyConnection = connect(m_subGraphModel, &SubGraphModel::destroyed, this, std::bind(&FilterInstanceModel::setSubGraphModel, this, nullptr));
    }
    emit subGraphModelChanged();
}

void FilterInstanceModel::init()
{
    beginResetModel();
    m_graph = fliplib::GraphContainer{};
    bool found = false;
    if (m_graphModel)
    {
        const auto index = m_graphModel->indexFor(m_graphId);
        if (index.isValid())
        {
            m_graph = m_graphModel->graph(index);
            found = true;
        }
    }
    if (!found && m_subGraphModel)
    {
        m_graph = m_subGraphModel->combinedGraph(m_graphId);
    }
    endResetModel();
}

bool FilterInstanceModel::hasVisibleAttributes(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return false;
    }
    return hasVisibleAttributes(m_graph.instanceFilters.at(index.row()));
}

bool FilterInstanceModel::hasVisibleAttributes(const fliplib::InstanceFilter &filter) const
{
    const auto &attributes = filter.attributes;
    return std::any_of(attributes.begin(), attributes.end(),
        [] (const auto &attribute)
        {
            return attribute.visible;
        });
}

int FilterInstanceModel::maxVisibleUserLevel(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return 0;
    }
    return maxVisibleUserLevel(m_graph.instanceFilters.at(index.row()));
}

int FilterInstanceModel::maxVisibleUserLevel(const fliplib::InstanceFilter &filter) const
{
    int userLevel = 0;
    const auto &attributes = filter.attributes;
    for (auto attribute : attributes)
    {
        if (!attribute.visible)
        {
            continue;
        }
        userLevel = std::max(userLevel, attribute.userLevel);
    }
    return userLevel;
}

}
}
