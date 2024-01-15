#pragma once

#include <QAbstractListModel>

#include "message/loggerGlobal.interface.h"

#include <deque>

class TestLogModel;

namespace precitec
{
namespace gui
{
namespace components
{
namespace logging
{

class LogReceiver;
class ModuleModel;

/**
 * @brief Model of log messages.
 *
 * The following roles are defined:
 * @li display: the actual log message
 * @li dateTime: the timestamp of the log message
 * @li module: the name of the module which emitted the log message
 * @li level: The log level (see LogType)
 **/
class LogModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * Index to the latest log item which is warning or error
     **/
    Q_PROPERTY(QString latestWarningOrError READ latestWarningOrError NOTIFY latestWarningOrErrorChanged)
    /**
     * Model exposing just the unique module names of all the log items in this LogModel.
     **/
    Q_PROPERTY(QAbstractItemModel *moduleModel READ moduleModel CONSTANT)
    /**
     * The maximum number of log messages in the LogModel.
     * If the amount of log messages to be added results in the count exceeding maximumLogMessage, messages
     * are removed so that the limit is kept.
     * The default maximumLogMessage is @c 5000.
     **/
    Q_PROPERTY(quint32 maximumLogMessage MEMBER m_maximumLogMessage NOTIFY maximumLogMessageChanged)
    /**
     * Whether the current logged in user is allowed to clear log messages.
     * If there is no user logged in the property is @c true.
     **/
    Q_PROPERTY(bool canClear READ canClear NOTIFY canClearChanged)

    /**
     * Whether the LogModel is currently paused. While paused new messages are received, but not processed, the model is not updated.
     **/
    Q_PROPERTY(bool paused READ isPaused WRITE setPaused NOTIFY pausedChanged)

    /**
     * Permission to clear log messages
     **/
    Q_PROPERTY(int clearLogMessagesPermission READ clearLogMessagesPermission WRITE setClearLogMessagesPermission NOTIFY clearLogMessagesPermissionChanged)
    /**
     * The name of the station. Defaults to the value of environment variable WM_STATION_NAME.
     **/
    Q_PROPERTY(QByteArray station READ station WRITE setStation NOTIFY stationChanged)
public:
    /**
     * Enum describing the logging level.
     * Direct mapping from LogType enum to be able to expose to QML.
     **/
    enum class LogLevel {
        Info = 1,
        Warning = 2,
        Error = 4,
        Fatal = 8,
        Debug = 16,
        Startup = 32,
        Tracker = 64
    };
    Q_ENUM(LogLevel)
    explicit LogModel(QObject *parent = nullptr);
    ~LogModel() override;

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent) const override;

    QHash<int, QByteArray> roleNames() const override;

    QString latestWarningOrError() const
    {
        return m_latestWarningOrError;
    }

    QAbstractItemModel *moduleModel() const;

    /**
     * Clears the model by removing all log entries.
     **/
    Q_INVOKABLE void clear();

    bool canClear() const;

    bool isPaused() const
    {
        return m_paused;
    }
    void setPaused(bool paused);

    int clearLogMessagesPermission() const
    {
        return m_clearLogMessagesPermission;
    }
    void setClearLogMessagesPermission(int permission);

    QByteArray station() const
    {
        return m_stationName;
    }

    void setStation(const QByteArray &station);

Q_SIGNALS:
    void latestWarningOrErrorChanged();
    void maximumLogMessageChanged();
    void canClearChanged();
    void pausedChanged();
    void clearLogMessagesPermissionChanged();
    void stationChanged();

private:
    void initReceiverThread();
    /**
     * Adds the @p items to the LogModel.
     **/
    void addItems(std::deque<precitec::interface::wmLogItem> &&items);
    /**
     * Formats the given log @p item and returns as a QString.
     * The placeholders are replaced by the appropriate parameters of the @p item.
     **/
    QString format(const precitec::interface::wmLogItem &item) const;

    std::deque<precitec::interface::wmLogItem> m_logItems;
    std::deque<precitec::interface::wmLogItem> m_pauseQueue;
    QThread *m_receiverThread = nullptr;
    LogReceiver *m_receiver = nullptr;
    QString m_latestWarningOrError;
    ModuleModel *m_moduleModel;
    quint32 m_maximumLogMessage = 5000;
    bool m_paused = false;
    int m_clearLogMessagesPermission = -1;
    QByteArray m_stationName;

    friend TestLogModel;
};

}
}
}
}
