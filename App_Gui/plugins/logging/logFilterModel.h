#pragma once

#include <QSortFilterProxyModel>

namespace precitec
{
namespace gui
{
namespace components
{
namespace logging
{

/**
 * @brief Filter model to restrict the items from LogModel.
 *
 * The main purpose is to restrict the modules for which to show the log messages.
 * Use property @link{moduleNameFilter} to filter on a module name.
 **/
class LogFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
    /**
     * The module name to filter on. If empty all log messages are included. Default is empty.
     **/
    Q_PROPERTY(QByteArray moduleNameFilter MEMBER m_moduleNameFilter NOTIFY moduleNameFilterChanged)
    /**
     * Include log messages with LogModel::LogLevel::Info and LogModel::LogLevel::Startup.
     * Default is @c true.
     **/
    Q_PROPERTY(bool includeInfo MEMBER m_includeInfo NOTIFY includeInfoChanged)
    /**
     * Include log messages with LogModel::LogLevel::Warning.
     * Default is @c true.
     **/
    Q_PROPERTY(bool includeWarning MEMBER m_includeWarning NOTIFY includeWarningChanged)
    /**
     * Include log messages with LogModel::LogLevel::Error and LogModel::LogLevel::Fatal.
     * Default is @c true.
     **/
    Q_PROPERTY(bool includeError MEMBER m_includeError NOTIFY includeErrorChanged)
    /**
     * Include log messages with LogModel::LogLevel::Error and LogModel::LogLevel::Debug.
     * Default is @c false.
     **/
    Q_PROPERTY(bool includeDebug MEMBER m_includeDebug NOTIFY includeDebugChanged)

    /**
     * Include log messages with LogModel::LogLevel::Tracker
     * Default is @c false
     **/
    Q_PROPERTY(bool includeTracker MEMBER m_includeTracker NOTIFY includeTrackerChanged)

    /**
     * Whether the current logged in user is allowed to view debug messages.
     * If there is no user logged in the property is @c true.
     **/
    Q_PROPERTY(bool canIncludeDebug READ canIncludeDebug NOTIFY canIncludeDebugChanged)

    /**
     * Permission to view debug messages
     **/
    Q_PROPERTY(int viewDebugMessagesPermission READ viewDebugMessagesPermission WRITE setViewDebugMessagesPermission NOTIFY viewDebugMessagesPermissionChanged)
public:
    explicit LogFilterModel(QObject *parent = nullptr);
    ~LogFilterModel() override;

    bool canIncludeDebug() const;

    int viewDebugMessagesPermission() const
    {
        return m_viewDebugMessagesPermission;
    }
    void setViewDebugMessagesPermission(int permission);

protected:
    bool filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

Q_SIGNALS:
    void moduleNameFilterChanged();
    void includeInfoChanged();
    void includeWarningChanged();
    void includeErrorChanged();
    void includeDebugChanged();
    void includeTrackerChanged();
    void canIncludeDebugChanged();
    void viewDebugMessagesPermissionChanged();

private:
    bool includeByErrorLevel(const QModelIndex &sourceIndex) const;
    QByteArray m_moduleNameFilter;
    bool m_includeInfo = true;
    bool m_includeWarning = true;
    bool m_includeError = true;
    bool m_includeDebug = false;
    bool m_includeTracker = false;
    int m_viewDebugMessagesPermission = -1;
};

}
}
}
}
