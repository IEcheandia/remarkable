#include "graphSortModel.h"

namespace precitec
{
namespace storage
{

GraphSortModel::GraphSortModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    sort(0);
}

GraphSortModel::~GraphSortModel() = default;

bool GraphSortModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    const auto groupLeft = source_left.data(Qt::UserRole + 8).toString();
    const auto groupRight = source_right.data(Qt::UserRole + 8).toString();
    const auto compare = QString::compare(groupLeft, groupRight);
    if (compare == 0)
    {
        return QSortFilterProxyModel::lessThan(source_left, source_right);
    }
    return compare < 0;
}

}
}
