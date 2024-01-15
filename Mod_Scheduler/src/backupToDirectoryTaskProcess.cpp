#include "Scheduler/backupToDirectoryTaskProcess.h"

namespace precitec
{
namespace scheduler
{

std::string BackupToDirectoryTaskProcess::name()
{
    return std::string("BackupToLocalDirectory");
}

std::vector<std::string> BackupToDirectoryTaskProcess::processArguments(const int loggerPipeFds[2])
{
    const auto &settings = info();
    std::vector<std::string> processArguments;
    processArguments.emplace_back(std::string{"--backupPath="} + settings.at("backupPath"));
    if (auto it = settings.find("config"); it != settings.end() && it->second == std::string{"true"})
    {
        processArguments.emplace_back(std::string{"--config"});
    }
    if (auto it = settings.find("logs"); it != settings.end() && it->second == std::string{"true"})
    {
        processArguments.emplace_back(std::string{"--logs"});
    }
    if (auto it = settings.find("screenshots"); it != settings.end() && it->second == std::string{"true"})
    {
        processArguments.emplace_back(std::string{"--screenshots"});
    }
    if (auto it = settings.find("software"); it != settings.end() && it->second == std::string{"true"})
    {
        processArguments.emplace_back(std::string{"--software"});
    }

    processArguments.emplace_back(std::to_string(loggerPipeFds[1]));

    return processArguments;
}

}
}
