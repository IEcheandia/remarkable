#include "filterAttributeSortFilterModel.h"
#include "permissions.h"

#include <precitec/userManagement.h>


using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

FilterAttributeSortFilterModel::FilterAttributeSortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    setSortRole(Qt::UserRole);
    sort(0);
    connect(UserManagement::instance(), &UserManagement::currentUserChanged, this, &FilterAttributeSortFilterModel::invalidate);
    connect(this, &FilterAttributeSortFilterModel::filterOnUserLevelChanged, this, &FilterAttributeSortFilterModel::invalidate);
    connect(this, &FilterAttributeSortFilterModel::userLevelChanged, this, &FilterAttributeSortFilterModel::invalidate);
}

FilterAttributeSortFilterModel::~FilterAttributeSortFilterModel() = default;

bool FilterAttributeSortFilterModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    // AbstractFilterAttributeModel
    // Qt::UserRole + 5 - userLevel
    // Qt::UserRole + 9 - visible

    if (m_filterOnUserLevel)
    {
        if (sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 5).toInt() != m_userLevel)
        {
            return false;
        }
    }

    if (!checkUserLevel(sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 5).toInt()))
    {
        return false;
    }

    return sourceModel()->index(source_row, 0, source_parent).data(Qt::UserRole + 9).toBool();
}

bool FilterAttributeSortFilterModel::checkUserLevel(int userlevel)
{
    switch (userlevel)
    {
    case 0: // Administrator
        return UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterAdmin));
    case 1: // SuperUser
        return UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterAdmin)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterSuperUser));
    case 2: // GroupLeader
        return UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterAdmin)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterSuperUser)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterGroupLeader));
    case 3: // Operator
        return UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterAdmin)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterSuperUser)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterGroupLeader)) ||
               UserManagement::instance()->hasPermission(int(Permission::ViewFilterParameterOperator));
    default:
        return false;
    }
}

bool FilterAttributeSortFilterModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    // AbstractFilterAttributeModel
    // Qt::UserRole + 8 - editListOrder

    auto leftOrder = sourceModel()->data(source_left, Qt::UserRole + 8).toInt();
    auto rightOrder = sourceModel()->data(source_right, Qt::UserRole + 8).toInt();

    return leftOrder < rightOrder;
}

void FilterAttributeSortFilterModel::setFilterOnUserLevel(bool set)
{
    if (m_filterOnUserLevel == set)
    {
        return;
    }
    m_filterOnUserLevel = set;
    emit filterOnUserLevelChanged();
}

void FilterAttributeSortFilterModel::setUserLevel(int level)
{
    if (m_userLevel == level)
    {
        return;
    }
    m_userLevel = level;
    emit userLevelChanged();
}

}
}
