#include "Scheduler/testTaskProcess.h"

namespace precitec
{
namespace scheduler
{

std::string TestTaskProcess::name()
{
    return std::string("TestProgram");
}

std::vector<std::string> TestTaskProcess::processArguments(const int loggerPipeFds[2])
{
    std::vector<std::string> processArguments;
    processArguments.emplace_back(std::to_string(loggerPipeFds[0])); // argv[1] is the fd for the read side of a pipe
    processArguments.emplace_back(std::to_string(loggerPipeFds[1])); // argv[2] is the fd for the write side of a pipe

    return processArguments;
}

}
}
