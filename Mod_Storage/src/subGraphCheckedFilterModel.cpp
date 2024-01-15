#include "subGraphCheckedFilterModel.h"
#include "subGraphModel.h"
#include "seam.h"

namespace precitec
{
namespace storage
{

SubGraphCheckedFilterModel::SubGraphCheckedFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &SubGraphCheckedFilterModel::seamChanged, this, &SubGraphCheckedFilterModel::invalidate);
    sort(0);
}

SubGraphCheckedFilterModel::~SubGraphCheckedFilterModel() = default;

bool SubGraphCheckedFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_seam)
    {
        const auto uuid = sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole).toUuid();
        const auto &subGraphs = m_seam->subGraphs();
        return std::find(subGraphs.begin(), subGraphs.end(), uuid) != subGraphs.end();
    }
    return sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 3).toBool();
}

bool SubGraphCheckedFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    const auto &availableCategories = qobject_cast<SubGraphModel*>(sourceModel())->availableCategories();
    const auto leftCategory = availableCategories.indexOf(sourceModel()->data(source_left, Qt::UserRole + 4).toString());
    const auto rightCategory = availableCategories.indexOf(sourceModel()->data(source_right, Qt::UserRole + 4).toString());

    if (leftCategory == rightCategory)
    {
        return sourceModel()->data(source_left).toString().compare(sourceModel()->data(source_right).toString()) < 0;
    }
    return leftCategory < rightCategory;
}

void SubGraphCheckedFilterModel::setSeam(precitec::storage::Seam *seam)
{
    if (m_seam == seam)
    {
        return;
    }
    m_seam = seam;
    disconnect(m_seamDestroyedConnection);
    if (m_seam)
    {
        m_seamDestroyedConnection = connect(m_seam, &QObject::destroyed, this, std::bind(&SubGraphCheckedFilterModel::setSeam, this, nullptr));
    }
    else
    {
        m_seamDestroyedConnection = {};
    }
    emit seamChanged();
}

}
}
