#pragma once
#include "videoRecorder/baseCommand.h"
#include "event/schedulerEvents.interface.h"

#include <QString>

namespace precitec
{
namespace storage
{

/**
 * Command object to move a product instance from temporary to final location
 **/
class MoveProductInstanceCommand : public vdr::BaseCommand
{
public:
    explicit MoveProductInstanceCommand(const QString &source, const QString &destination, const QString &cacheFilePath, std::size_t maxCacheEntries, const std::shared_ptr<precitec::interface::TSchedulerEvents<interface::AbstractInterface>> &proxy);

    void execute() override;

private:
    QString m_source;
    QString m_destination;
    QString m_cacheFilePath;
    std::size_t m_maxCacheEntries;
    std::shared_ptr<precitec::interface::TSchedulerEvents<interface::AbstractInterface>> m_schedulerEvent;
};

}
}
