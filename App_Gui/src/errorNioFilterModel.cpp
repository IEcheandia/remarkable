#include "errorNioFilterModel.h"

namespace precitec {
    
namespace gui {

ErrorNioFilterModel::ErrorNioFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
    , m_sortAsc(true)
{   
    connect(this, &ErrorNioFilterModel::sortAscChanged, this, &ErrorNioFilterModel::invalidate);
    setSortRole(Qt::UserRole + 1);
    sort(0);
}

ErrorNioFilterModel::~ErrorNioFilterModel()
{
}

bool ErrorNioFilterModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    QVariant leftData = sourceModel()->data(source_left, sortRole());
    QVariant rightData = sourceModel()->data(source_right, sortRole());
    switch (sortRole())
    {
        case Qt::DisplayRole:
             return m_sortAsc ? leftData.toInt() <= rightData.toInt() : leftData.toInt() >= rightData.toInt();
        case Qt::UserRole + 1:
             return m_sortAsc ? leftData.toString() <= rightData.toString() : leftData.toString() >= rightData.toString();
        case Qt::UserRole + 7:
             return m_sortAsc ? leftData.toInt() <= rightData.toInt() : leftData.toInt() >= rightData.toInt();
        default:
             return m_sortAsc ? leftData.toInt() <= rightData.toInt() : leftData.toInt() >= rightData.toInt();
             break;
    }
}

int ErrorNioFilterModel::findIndex(int value)
{
    for (int i = 0; i < sourceModel()->rowCount(); i++)
    {
        if (index(i, 0).data().toInt() == value)
        {
            return i;
        }
    }
    return -1;
}

}
}
