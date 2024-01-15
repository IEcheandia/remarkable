#include "checkedFilterModel.h"

namespace precitec
{
namespace gui
{

CheckedFilterModel::CheckedFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &CheckedFilterModel::roleNameChanged, this, &CheckedFilterModel::findRole);
    connect(this, &CheckedFilterModel::sourceModelChanged, this, &CheckedFilterModel::findRole);

    connect(this, &CheckedFilterModel::modelReset, this, &CheckedFilterModel::rowCountChanged);
    connect(this, &CheckedFilterModel::rowsInserted, this, &CheckedFilterModel::rowCountChanged);
    connect(this, &CheckedFilterModel::rowsRemoved, this, &CheckedFilterModel::rowCountChanged);
}

CheckedFilterModel::~CheckedFilterModel() = default;

bool CheckedFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_role == -1)
    {
        return false;
    }
    return sourceModel()->index(source_row, 0, source_parent).data(m_role).toBool();
}

void CheckedFilterModel::setRoleName(const QByteArray& name)
{
    if (m_roleName == name)
    {
        return;
    }
    m_roleName = name;
    emit roleNameChanged();
}

void CheckedFilterModel::findRole()
{
    int role = -1;
    if (auto model = sourceModel())
    {
        role = model->roleNames().key(m_roleName, -1);
    }
    if (m_role != role)
    {
        m_role = role;
        invalidateFilter();
    }
}

}
}
