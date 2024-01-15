#include "Scheduler/exportProductTaskProcess.h"
#include <unistd.h>
#include <module/moduleLogger.h>

namespace precitec
{
namespace scheduler
{

std::string ExportProductTaskProcess::name()
{
    return std::string("ExportProduct");
}

std::vector<std::string> ExportProductTaskProcess::processArguments(const int loggerPipeFds[2])
{
    std::vector<std::string> processArguments;
    close(loggerPipeFds[0]);
    processArguments.emplace_back(std::to_string(loggerPipeFds[1])); // argv[2] is the fd for the write side of a pipe
    processArguments.emplace_back(info().at("uuid"));
    processArguments.emplace_back(std::string{"--ip="} + info().at("TargetIpAddress"));

    if (auto it = info().find("Port"); it != info().end())
    {
        processArguments.emplace_back(std::string{"--port="} + it->second);
    }

    processArguments.emplace_back(std::string{"--user="} + info().at("TargetUserName"));
    processArguments.emplace_back(std::string{"--password="} + info().at("TargetPassword"));
    processArguments.emplace_back(std::string{"--remotePath="} + info().at("TargetDirectoryPath"));

    if (auto it = info().find("Protocol"); it != info().end())
    {
        processArguments.emplace_back(std::string{"--protocol="} + it->second);
    }

    if (auto it = info().find("HttpMethod"); it != info().end())
    {
        processArguments.emplace_back(std::string{"--httpMethod="} + it->second);
    }

    // following argument is optional
    if (info().find("DebugOptionStatus") != info().end())
    {
        processArguments.emplace_back(std::string{"--debug"});
    }
    return processArguments;
}

}
}
