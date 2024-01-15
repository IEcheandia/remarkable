#include "Scheduler/transferDirectoryTaskProcess.h"

namespace precitec
{
namespace scheduler
{

std::string TransferDirectoryTaskProcess::name() {
    return std::string("TransferDirectory");
}

std::vector<std::string> TransferDirectoryTaskProcess::processArguments(const int *loggerPipeFds)
{
    std::vector<std::string> processArguments;
    processArguments.emplace_back(std::to_string(loggerPipeFds[0])); // argv[1] is the fd for the read side of a pipe
    processArguments.emplace_back(std::to_string(loggerPipeFds[1])); // argv[2] is the fd for the write side of a pipe

    processArguments.emplace_back(std::string{"--ip="} + info().at("TargetIpAddress"));

    if (auto it = info().find("Port"); it != info().end())
    {
        processArguments.emplace_back(std::string{"--port="} + it->second);
    }

    processArguments.emplace_back(std::string{"--user="} + info().at("TargetUserName"));
    processArguments.emplace_back(std::string{"--password="} + info().at("TargetPassword"));

    processArguments.emplace_back(std::string{"--sourcePath="} + info().at("SourceDirectoryPath"));
    processArguments.emplace_back(std::string{"--sourceDir="} + info().at("SourceDirectoryName"));

    processArguments.emplace_back(std::string{"--remotePath="} + info().at("TargetDirectoryPath"));
    processArguments.emplace_back(std::string{"--remoteDir="} + info().at("TargetDirectoryName"));

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


