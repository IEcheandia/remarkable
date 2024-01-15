#include "intervalErrorFilterModel.h"
#include "intervalErrorModel.h"

namespace precitec
{
namespace gui
{

IntervalErrorFilterModel::IntervalErrorFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(Qt::DisplayRole);
    sort(0);
}

IntervalErrorFilterModel::~IntervalErrorFilterModel() = default;

bool IntervalErrorFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }
    return true;
}

bool IntervalErrorFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    return QSortFilterProxyModel::lessThan(source_left, source_right);
}

}
}
