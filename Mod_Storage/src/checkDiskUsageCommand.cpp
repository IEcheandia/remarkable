#include "checkDiskUsageCommand.h"

#include "system/tools.h"

namespace precitec
{
namespace storage
{

CheckDiskUsageCommand::CheckDiskUsageCommand(const std::string &fileSystemPath, double relativeMaxDiskUsage, const std::function<void(double)> &callbackForDiskUsageReached)
    : vdr::BaseCommand()
    , m_fileSystemPath(fileSystemPath)
    , m_relativeMaxDiskUsage(relativeMaxDiskUsage)
    , m_callbackForDiskUsageReached(callbackForDiskUsageReached)
{
}

void CheckDiskUsageCommand::execute()
{
    const auto diskUsage = system::getDiskUsage(m_fileSystemPath);
    if (diskUsage > m_relativeMaxDiskUsage)
    {
        m_callbackForDiskUsageReached(diskUsage);
    }
}

}
}
