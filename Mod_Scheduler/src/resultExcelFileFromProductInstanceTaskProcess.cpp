#include "Scheduler/resultExcelFileFromProductInstanceTaskProcess.h"
#include <unistd.h>
namespace precitec
{
namespace scheduler
{

std::string ResultExcelFileFromProductInstanceTaskProcess::name()
{
    return std::string("ResultExcelFileFromProductInstance");
}

std::vector<std::string> ResultExcelFileFromProductInstanceTaskProcess::processArguments(const int loggerPipeFds[2])
{
    std::vector<std::string> processArguments;
    close(loggerPipeFds[0]);

    processArguments.emplace_back(std::to_string(loggerPipeFds[1])); // argv[1] is the fd for the write side of a pipe
    processArguments.emplace_back(info().at("ProductInstanceDirectory")); // argv[2] is the directory with result product instance

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
