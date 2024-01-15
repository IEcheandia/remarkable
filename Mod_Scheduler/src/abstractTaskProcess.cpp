#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "module/moduleLogger.h"

#include "Scheduler/abstractTaskProcess.h"

namespace precitec
{

namespace scheduler
{

AbstractTaskProcess::~AbstractTaskProcess()
{
    usleep(20*1000);
}

void AbstractTaskProcess::setInfo(const std::map<std::string, std::string> &info)
{
    m_info = info;
}

const std::map<std::string, std::string> &AbstractTaskProcess::info() const
{
    return m_info;
}

void AbstractTaskProcess::setPath(const std::string &path)
{
    m_path = path;
}

std::string AbstractTaskProcess::path() const
{
    return m_path;
}

static char *toCharArray(const std::string &string)
{
    char *cStr = new char[string.size() + 1];
    strcpy(cStr, string.c_str());
    return cStr;
}

const int MAX_LOGGER_MESSAGE_LENGTH = 150;

bool AbstractTaskProcess::log(int loggerPipeFds[2])
{
    char oBuffer[MAX_LOGGER_MESSAGE_LENGTH + 1]{};
    bool result = true;
    long int oReturn{0};
    while ((oReturn = read(loggerPipeFds[0], oBuffer, (MAX_LOGGER_MESSAGE_LENGTH + 1))) > 0)
    {
        oBuffer[oReturn] = 0x00;
        if(oBuffer[0] == '#')
        {
            wmLog(eError, &oBuffer[1]);
            result = false;
        }
        else if(oBuffer[0] == '$')
        {
            wmLog(eWarning, &oBuffer[1]);
        }
        else if(oBuffer[0] == '&')
        {
            wmLog(eInfo, &oBuffer[1]);
        }
        else if(oBuffer[0] == '?')
        {
            wmLog(eDebug, &oBuffer[1]);
        }
        else
        {
            wmLog(eDebug, &oBuffer[1]);
            result = false;
        }
    }
    return result;
}


std::vector<char*> AbstractTaskProcess::transformedProcessArguments(const std::string &fullProcessName,
                                                                    const std::vector<std::string> &processArguments)
{
    // HACK: create the char * const[] arguments in a c++ way
    std::vector<char*> arguments;
    arguments.push_back(toCharArray(std::string(fullProcessName)));

    for (const auto &argument : processArguments)
    {
        arguments.push_back(toCharArray(argument));
    }
    // last element in array needs to be a nullptr
    arguments.push_back(nullptr);
    return arguments;
}

bool AbstractTaskProcess::run()
{
    if (m_path.empty()) m_path = std::string(getenv("WM_BASE_DIR")) + std::string("/bin/");;

    int loggerPipeFds[2]{};
    if (pipe(loggerPipeFds) < 0)
    {
        perror("pipe");
    }

    const auto pid = fork();
    if (pid == 0)
    {
        // child process
        // first unblock signals, ConnectServer as a Poco::ServerApplication has signals blocked
        sigset_t signals;
        sigfillset(&signals);
        sigprocmask(SIG_UNBLOCK, &signals, nullptr);

        // now start the actual process by exec
        auto fullProgramName = path() + name();
        std::vector<char*> arguments = transformedProcessArguments(fullProgramName, processArguments(loggerPipeFds));
        execv(fullProgramName.c_str(), &arguments[0]);
        // cleanup
        for (auto it = arguments.begin(); it != arguments.end(); ++it)
        {
            delete[] *it;
        }
        exit(EXIT_SUCCESS);
    }
    else if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else
    {
        close(loggerPipeFds[1]);
        auto resultLog = log(loggerPipeFds);
        close(loggerPipeFds[0]);
        wait(NULL);
        return resultLog;
    }
}

}
}
