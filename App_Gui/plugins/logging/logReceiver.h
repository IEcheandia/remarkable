#pragma once

#include <QObject>

#include "message/loggerGlobal.interface.h"

#include <deque>
#include <vector>
#include <QFileInfo>

#include <Poco/SharedMemory.h>

class QMutex;
class QFileSystemWatcher;
class QTimer;

namespace precitec
{

namespace gui
{
namespace components
{
namespace logging
{

/**
 * @brief Periodically queries the logger shared memory for new log messages.
 *
 * The class is supposed to be used in a dedicated thread.
 * It emits a signal @link{logsReceived} once new log messages are available.
 * They can be obtained by invoking the takeLogs method.
 **/
class LogReceiver : public QObject
{
    Q_OBJECT
public:
    explicit LogReceiver(QObject *parent = nullptr);
    ~LogReceiver() override;

    /**
     * @returns all log messages which have been received since the last invokation of takeLogs.
     * @see logsReceived
     **/
    std::deque<precitec::interface::wmLogItem> takeLogs();

    void setStationName(const QByteArray &stationName)
    {
        m_stationName = stationName;
    }
    void start();

Q_SIGNALS:
    /**
     * Signal emitted whenever new log messages have been received.
     * They can be obtained by invoking the takeLogs method.
     **/
    void logsReceived();

private:
    void getLogs();
    void checkForLogFiles();
    std::unique_ptr<QMutex> m_mutex;
    std::deque<precitec::interface::wmLogItem> m_messages;
    QFileSystemWatcher *m_shmWatcher;
    std::vector<QFileInfo> m_shmFiles;
    std::vector<Poco::SharedMemory> m_shmSections;
    QTimer *m_timer;
    QTimer *m_delayedCheckDirectoryTimer;
    QByteArray m_stationName;
};

}
}
}
}
