#include "moveProductInstanceCommand.h"
#include "videoRecorder/fileCommand.h"

#include <QDir>
#include <QFile>

namespace precitec
{
namespace storage
{

MoveProductInstanceCommand::MoveProductInstanceCommand(const QString &source, const QString &destination, const QString &cacheFilePath, std::size_t maxCacheEntries, const std::shared_ptr<precitec::interface::TSchedulerEvents<interface::AbstractInterface>> &proxy)
    : m_source(source)
    , m_destination(destination)
    , m_cacheFilePath(cacheFilePath)
    , m_maxCacheEntries(maxCacheEntries)
    , m_schedulerEvent(proxy)
{
}

void MoveProductInstanceCommand::execute()
{
    QDir{}.mkpath(QDir::cleanPath(m_destination + QStringLiteral("../")));
    auto destination = QDir::cleanPath(m_destination);
    if (!QDir{}.rename(QDir::cleanPath(m_source), destination))
    {
        return;
    }
    if (!destination.endsWith(QDir::separator()))
    {
        destination.append(QDir::separator());
    }

    vdr::CachePath functor{m_cacheFilePath.toStdString(), destination.toStdString(), m_maxCacheEntries, std::string("results")};
    functor();

    if (m_schedulerEvent)
    {
        m_schedulerEvent->schedulerEventFunction(interface::SchedulerEvents::ProductInstanceResultsStored,
                                                 std::map<std::string, std::string>{{std::string{"path"}, destination.toStdString()}});
    }
}

}
}
