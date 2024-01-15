#include "filterGroupsModel.h"

namespace precitec
{
namespace storage
{

FilterGroupsModel::FilterGroupsModel(QObject *parent)
    : FilterInstanceModel(parent)
{
    connect(this, &FilterGroupsModel::rowsInserted, this, &FilterGroupsModel::updateNotGrouped);
    connect(this, &FilterGroupsModel::rowsRemoved, this, &FilterGroupsModel::updateNotGrouped);
    connect(this, &FilterGroupsModel::modelReset, this, &FilterGroupsModel::updateNotGrouped);
}

FilterGroupsModel::~FilterGroupsModel() = default;

QVariant FilterGroupsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    const auto &groups = graph().filterGroups;
    if (groups.size() < std::size_t(index.row()))
    {
        return {};
    }
    const auto &group = groups.at(index.row());
    const auto &filters = graph().instanceFilters;
    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(group.name);
    case Qt::UserRole:
        return group.number;
    case Qt::UserRole + 2:
        return std::any_of(filters.begin(), filters.end(), [this, &group] (const auto &filter) { return filter.group == group.number && this->hasVisibleAttributes(filter); });
    case Qt::UserRole + 3:
        {
            int userLevel = 0;
            for (const auto &filter : filters)
            {
                if (filter.group != group.number)
                {
                    continue;
                }
                userLevel = std::max(userLevel, maxVisibleUserLevel(filter));
            }
            return userLevel;
        }
    case Qt::UserRole + 4:
        return QUuid{QString::fromStdString(group.sourceGraph.toString())};
    }
    return {};
}

int FilterGroupsModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return graph().filterGroups.size();
}

QHash<int, QByteArray> FilterGroupsModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("number")},
        {Qt::UserRole+2, QByteArrayLiteral("visibleAttributes")},
        {Qt::UserRole+3, QByteArrayLiteral("maxVisibleUserLevel")}
    };
}

void FilterGroupsModel::updateNotGrouped()
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

void FilterGroupsModel::setNotGrouped(bool set)
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
