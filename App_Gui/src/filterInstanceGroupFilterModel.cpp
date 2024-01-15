#include "filterInstanceGroupFilterModel.h"
#include "filterAttributeSortFilterModel.h"

namespace precitec
{
namespace gui
{

FilterInstanceGroupFilterModel::FilterInstanceGroupFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &FilterInstanceGroupFilterModel::groupChanged, this, &FilterInstanceGroupFilterModel::invalidate);
    connect(this, &FilterInstanceGroupFilterModel::sourceGraphChanged, this, &FilterInstanceGroupFilterModel::invalidate);
    connect(this, &FilterInstanceGroupFilterModel::filterOnGroupChanged, this, &FilterInstanceGroupFilterModel::invalidate);
    connect(this, &FilterInstanceGroupFilterModel::filterOnUserLevelChanged, this, &FilterInstanceGroupFilterModel::invalidate);
    connect(this, &FilterInstanceGroupFilterModel::userLevelChanged, this, &FilterInstanceGroupFilterModel::invalidate);
    connect(this, &FilterInstanceGroupFilterModel::modelReset, this, &FilterInstanceGroupFilterModel::updateNotGrouped);
    connect(this, &FilterInstanceGroupFilterModel::sourceGraphChanged, this, &FilterInstanceGroupFilterModel::updateNotGrouped);
    sort(0);
    setSortRole(Qt::DisplayRole);
}

FilterInstanceGroupFilterModel::~FilterInstanceGroupFilterModel() = default;

bool FilterInstanceGroupFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    const auto index = sourceModel()->index(source_row, 0, source_parent);
    if (!m_sourceGraph.isNull())
    {
        if (m_sourceGraph != index.data(Qt::UserRole + 4).toUuid())
        {
            return false;
        }
    }
    if (m_filterOnGroup)
    {
        if (m_group != index.data(Qt::UserRole + 1).toInt())
        {
            return false;
        }
    }
    if (!index.data(Qt::UserRole + 2).toBool())
    {
        return false;
    }
    if (m_filterOnUserLevel)
    {
        if (index.data(Qt::UserRole + 3).toInt() != m_userLevel)
        {
            return false;
        }
    }
    if (!FilterAttributeSortFilterModel::checkUserLevel(index.data(Qt::UserRole + 3).toInt()))
    {
        return false;
    }
    return true;
}

void FilterInstanceGroupFilterModel::setGroup(int group)
{
    if (m_group == group)
    {
        return;
    }
    m_group = group;
    emit groupChanged();
}

void FilterInstanceGroupFilterModel::setFilterOnGroup(bool set)
{
    if (m_filterOnGroup == set)
    {
        return;
    }
    m_filterOnGroup = set;
    emit filterOnGroupChanged();
}

void FilterInstanceGroupFilterModel::setSourceGraph(const QUuid& id)
{
    if (m_sourceGraph == id)
    {
        return;
    }
    m_sourceGraph = id;
    emit sourceGraphChanged();
}

void FilterInstanceGroupFilterModel::setFilterOnUserLevel(bool set)
{
    if (m_filterOnUserLevel == set)
    {
        return;
    }
    m_filterOnUserLevel = set;
    emit filterOnUserLevelChanged();
}

void FilterInstanceGroupFilterModel::setUserLevel(int level)
{
    if (m_userLevel == level)
    {
        return;
    }
    m_userLevel = level;
    emit userLevelChanged();
}

void FilterInstanceGroupFilterModel::updateNotGrouped()
{
    if (rowCount() != 1)
    {
        setNotGrouped(false);
        return;
    }
    if (index(0, 0).data(Qt::DisplayRole).toString().compare(QStringLiteral("Not grouped"), Qt::CaseInsensitive) != 0)
    {
        setNotGrouped(false);
        return;
    }
    setNotGrouped(true);
}

void FilterInstanceGroupFilterModel::setNotGrouped(bool set)
{
    if (m_notGrouped == set)
    {
        return;
    }
    m_notGrouped = set;
    emit notGroupedChanged();
}

}
}
