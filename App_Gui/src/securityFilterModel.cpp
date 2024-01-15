#include "securityFilterModel.h"

namespace precitec
{
namespace gui
{

SecurityFilterModel::SecurityFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{

    connect(this, &SecurityFilterModel::filterAvailableChanged, this, &SecurityFilterModel::invalidate);
}

SecurityFilterModel::~SecurityFilterModel() = default;

bool SecurityFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_filterAvailable)
    {
        if (!sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 2).toBool())
        {
            return false;
        }
    }
    return sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole).toBool();
}

void SecurityFilterModel::setFilterAvailable(bool set)
{
    if (m_filterAvailable == set)
    {
        return;
    }
    m_filterAvailable = set;
    emit filterAvailableChanged();
}

}
}
