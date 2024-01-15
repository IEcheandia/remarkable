#include "Scheduler/deleteOldBackupsTaskProcess.h"
#include <unistd.h>

namespace precitec
{
namespace scheduler
{

std::string DeleteOldBackupsTaskProcess::name()
{
    return std::string("DeleteOldBackups");
}

std::vector<std::string> DeleteOldBackupsTaskProcess::processArguments(const int loggerPipeFds[2])
{
    std::vector<std::string> processArguments;

    close(loggerPipeFds[0]); // close read side of a pipe

    processArguments.emplace_back(std::to_string(loggerPipeFds[1]));
    processArguments.emplace_back(info().at("backupPath"));
    processArguments.emplace_back(info().at("TimeToLiveDays"));

    return processArguments;
}

}
}
