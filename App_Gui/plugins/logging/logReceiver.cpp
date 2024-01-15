#include "logReceiver.h"

#include <QDir>
#include <QFileSystemWatcher>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

namespace precitec
{

using interface::wmLogItem;
using interface::wmLogParam;

namespace gui
{
namespace components
{
namespace logging
{

namespace
{

// taken from appLoggerServer.cpp

wmLogParam convertParam( LogParam& p_rParam )
{
	wmLogParam oParam;

	if (p_rParam.isString())
		oParam.setString( p_rParam.string() );
	else
		oParam.setValue( p_rParam.value() );

	return oParam;

} // convertParam

wmLogItem convertMsg( LogMessage& p_rMessage, unsigned int p_oIndex )
{
    // OK, now create the wmLogItem:
	if ( p_rMessage.m_oIntKey[0] == '\0' )
	{
		// if we do not have an internationalization key, then the message is send directly and convert all parameters into constant string:
		std::string oMessage = p_rMessage.formatParams();
		wmLogItem oItem( p_rMessage.m_oTimestamp, p_oIndex, oMessage, std::string(""), p_rMessage.m_oModule, p_rMessage.m_oType );
		return oItem;

	} else {

		wmLogItem oItem( p_rMessage.m_oTimestamp, p_oIndex, p_rMessage.m_oBuffer, p_rMessage.m_oIntKey, p_rMessage.m_oModule, p_rMessage.m_oType );
		// now we have to add all valid parameters
		for( unsigned int i=0; i<LogMessageParams; ++i )
			if (p_rMessage.m_oParams[i].isValid())
				oItem.addParam( convertParam( p_rMessage.m_oParams[i] ) );

		return oItem;

	}

} // convertMsg
}

static const QString s_devshm{QStringLiteral("/dev/shm")};

LogReceiver::LogReceiver(QObject *parent)
    : QObject(parent)
    , m_mutex(std::make_unique<QMutex>())
    , m_shmWatcher(new QFileSystemWatcher{{s_devshm}, this})
    , m_timer(new QTimer{this})
    , m_delayedCheckDirectoryTimer(new QTimer{this})
{
    connect(m_shmWatcher, &QFileSystemWatcher::directoryChanged, m_delayedCheckDirectoryTimer, qOverload<>(&QTimer::start));

    m_timer->setInterval(std::chrono::milliseconds{100});
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &LogReceiver::getLogs);

    m_delayedCheckDirectoryTimer->setSingleShot(true);
    m_delayedCheckDirectoryTimer->setInterval(std::chrono::milliseconds{100});
    connect(m_delayedCheckDirectoryTimer, &QTimer::timeout, this, &LogReceiver::checkForLogFiles);
}

LogReceiver::~LogReceiver() = default;

void LogReceiver::getLogs()
{
    m_timer->stop();
    bool gotLogs = false;
    while (true)
    {
        bool found = false;
        for (auto &sharedMemory : m_shmSections)
        {
            auto content = reinterpret_cast<LogShMemContent*>(sharedMemory.begin());
            if ( content->m_oWriteIndex != content->m_oReadIndexGui || content->m_bRollOverGui == true )
            {
                QMutexLocker locker(m_mutex.get());

                m_messages.emplace_back(convertMsg(content->m_oMessages[content->m_oReadIndexGui], content->m_oReadIndexGui));

                content->m_oReadIndexGui++;
                if (content->m_oReadIndexGui >= LogMessageCapacity)
                {
                    content->m_oReadIndexGui = 0;
                    content->m_bRollOverGui = false;
                }
                gotLogs = true;
                found = true;
            }
        }
        if (!found)
        {
            break;
        }
    }
    if (gotLogs)
    {
        emit logsReceived();
    }
    m_timer->start();
}

std::deque<precitec::interface::wmLogItem> LogReceiver::takeLogs()
{
    QMutexLocker locker(m_mutex.get());
    std::deque<precitec::interface::wmLogItem> messages = std::move(m_messages);
    m_messages.clear();
    return messages;
}

void LogReceiver::checkForLogFiles()
{
    if (m_stationName.isEmpty())
    {
        return;
    }
    bool found = false;
    const auto logFiles = QDir{s_devshm}.entryInfoList({QStringLiteral("wmLog") + m_stationName + QStringLiteral("*")}, QDir::Files);
    for (const auto &logFile : logFiles)
    {
        if (std::find(m_shmFiles.begin(), m_shmFiles.end(), logFile) == m_shmFiles.end())
        {
            if (logFile.size() == 0)
            {
                m_delayedCheckDirectoryTimer->start();
                continue;
            }
            m_shmFiles.push_back(logFile);
            m_shmSections.push_back(Poco::SharedMemory{logFile.fileName().toStdString(), sizeof(LogShMemContent), Poco::SharedMemory::AccessMode(Poco::SharedMemory::AM_READ | Poco::SharedMemory::AM_WRITE), 0, false});
            found = true;
        }
    }
    if (found)
    {
        getLogs();
    }
}

void LogReceiver::start()
{
    QMetaObject::invokeMethod(m_delayedCheckDirectoryTimer, qOverload<>(&QTimer::start), Qt::QueuedConnection);
}

}
}
}
}
