#include "Scheduler/backupToRemoteTaskProcess.h"

namespace precitec
{
namespace scheduler
{

std::string BackupToRemoteTaskProcess::name()
{
    return std::string("BackupToRemoteDirectory");
}

std::vector<std::string> BackupToRemoteTaskProcess::processArguments(const int loggerPipeFds[2])
{
    const auto &settings = info();
    std::vector<std::string> processArguments;
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

    processArguments.emplace_back(std::string{"--ip="} + info().at("TargetIpAddress"));
    processArguments.emplace_back(std::string{"--user="} + info().at("TargetUserName"));
    processArguments.emplace_back(std::string{"--password="} + info().at("TargetPassword"));
    processArguments.emplace_back(std::string{"--remotePath="} + info().at("TargetDirectoryPath"));

    if (auto it = settings.find("Port"); it != settings.end())
    {
        processArguments.emplace_back(std::string{"--port="} + it->second);
    }

    if (auto it = info().find("Protocol"); it != info().end())
    {
        processArguments.emplace_back(std::string{"--protocol="} + it->second);
    }

    if (auto it = info().find("HttpMethod"); it != info().end())
    {
        processArguments.emplace_back(std::string{"--httpMethod="} + it->second);
    }

    // following argument is optional
    if (auto it = settings.find("DebugOptionStatus"); it != settings.end() && it->second == std::string{"true"})
    {
        processArguments.emplace_back(std::string{"--debug"});
    }

    processArguments.emplace_back(std::to_string(loggerPipeFds[1]));

    return processArguments;
}

}
}

