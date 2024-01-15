#include "logFilterModel.h"
#include "logModel.h"

#include <precitec/userManagement.h>

using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{
namespace components
{
namespace logging
{

LogFilterModel::LogFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    auto invalidate = [this]
    {
        invalidateFilter();
    };
    connect(this, &LogFilterModel::moduleNameFilterChanged, this, invalidate);
    connect(this, &LogFilterModel::includeInfoChanged, this, invalidate);
    connect(this, &LogFilterModel::includeWarningChanged, this, invalidate);
    connect(this, &LogFilterModel::includeErrorChanged, this, invalidate);
    connect(this, &LogFilterModel::includeDebugChanged, this, invalidate);
    connect(this, &LogFilterModel::includeTrackerChanged, this, invalidate);
    connect(UserManagement::instance(), &UserManagement::currentUserChanged, this, &LogFilterModel::canIncludeDebugChanged);
    connect(this, &LogFilterModel::canIncludeDebugChanged, this, invalidate);
}

LogFilterModel::~LogFilterModel() = default;

bool LogFilterModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    return sourceColumn == 0 && !sourceParent.isValid();
}

bool LogFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (sourceParent.isValid())
    {
        return false;
    }
    const QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!includeByErrorLevel(index))
    {
        return false;
    }
    if (m_moduleNameFilter.isEmpty())
    {
        return true;
    }
    const auto groupName = sourceModel()->data(index, Qt::UserRole + 1).toByteArray();
    return groupName == m_moduleNameFilter;
}

bool LogFilterModel::includeByErrorLevel(const QModelIndex &sourceIndex) const
{
    switch (sourceIndex.data(Qt::UserRole + 2).value<LogModel::LogLevel>())
    {
    case LogModel::LogLevel::Info:
    case LogModel::LogLevel::Startup:
        return m_includeInfo;
    case LogModel::LogLevel::Warning:
        return m_includeWarning;
    case LogModel::LogLevel::Error:
    case LogModel::LogLevel::Fatal:
        return m_includeError;
    case LogModel::LogLevel::Debug:
        return m_includeDebug && canIncludeDebug();
    case LogModel::LogLevel::Tracker:
        return m_includeTracker;
    default:
        return false;
    }
}

bool LogFilterModel::canIncludeDebug() const
{
    if (!UserManagement::instance()->currentUser())
    {
        return true;
    }
    return UserManagement::instance()->hasPermission(m_viewDebugMessagesPermission);
}

void LogFilterModel::setViewDebugMessagesPermission(int permission)
{
    if (m_viewDebugMessagesPermission == permission)
    {
        return;
    }
    m_viewDebugMessagesPermission = permission;
    emit viewDebugMessagesPermissionChanged();
}

}
}
}
}
