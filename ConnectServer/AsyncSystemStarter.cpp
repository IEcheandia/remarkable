/*
 * AsyncSystemStarter.cpp
 *
 *  Created on: 19.09.2012
 *      Author: admin
 */

#include "AsyncSystemStarter.h"
#include <config-connectServer.h>
#include "Poco/File.h"
#include "Poco/Thread.h"
#include "Poco/Util/PropertyFileConfiguration.h"

#include <cstring>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <spawn.h>
#include <algorithm>
#include <numeric>

#include <errno.h>

#include <sys/wait.h>
#include <unistd.h>

namespace Precitec {
namespace Service {
namespace Discovery {

AsyncSystemStarter::AsyncSystemStarter()
    : m_configFileName(std::string("startup.config"))
{

}

AsyncSystemStarter::~AsyncSystemStarter() {
	// TODO Auto-generated destructor stub
}

static std::string wmInstPath()
{
	std::string s_wmInst = std::string(getenv("WM_BASE_DIR"));
	return s_wmInst;
}
 
 void AsyncSystemStarter::run(){
	// try to start processes with an fd, blocks till notified
	const auto processes = loadConfiguration();
	const auto wmProcesses = std::count_if(processes.begin(), processes.end(), [] (const auto &process) { return process.weldMasterApplication; });
	Poco::ThreadPool::defaultPool().addCapacity(wmProcesses);
	for (const auto &process : processes)
	{
		startProcess(process);
	}
}

/**
 * Helper method which splits the argument string at whitespaces
 *
 * @returns vector of tokenized strings
 */
static std::vector<std::string> split(const std::string &string)
{
	std::vector<std::string> ret;
	std::string::size_type index = 0;
	while (index != std::string::npos && index < string.size())
	{
		// note: this splitting doesn't eliminate multiple white spaces
		// if there are multiple white spaces in the arguments the process
		// will fail to start
		const auto endIndex = string.find(' ', index);
		if (endIndex == std::string::npos)
		{
			ret.push_back(string.substr(index, endIndex));
			index = endIndex;
		} else
		{
			ret.push_back(string.substr(index, endIndex - index));
			index = endIndex;
			index++;
		}
	}
	return ret;
}

/**
 * Squashes multiple whitespaces in @p input into one whitespace.
 *
 * E.g. "  " is turned into " ". This is required to have proper
 * command line attribute forwarding.
 *
 * @returns a string with multiple whitespaces removed.
 */
static std::string removeWhitespaces(const std::string &input)
{
	std::string output = input;
	auto it = output.begin();
	while (it != output.end())
	{
		if (*it == ' ')
		{
			auto nextIt = it + 1;
			if (nextIt == output.end())
			{
				break;
			}
			if (*nextIt == ' ')
			{
				it = output.erase(it);
				continue;
			}
		}
		it++;
	}
	return output;
}

std::vector<AsyncSystemStarter::Process> AsyncSystemStarter::loadConfiguration()
{
	using namespace Poco::Util;
	Poco::AutoPtr<PropertyFileConfiguration> pConf{new PropertyFileConfiguration()};
	try
	{
		pConf->load(wmInstPath() + std::string("/system_config/") + m_configFileName);
	} catch (...)
	{
		std::cout << "Failed loading startup configuration! Does ${WM_BASE_DIR}/system_config/startup.config exist?" << std::endl;
		return std::vector<Process>();
	}

	const std::string oTermPath = pConf->getString(std::string("TerminalApplication.Path"));
    const bool terminalApplication = pConf->getBool(std::string("TerminalApplication.Use"), true);
	AbstractConfiguration::Keys startupKeys;
	pConf->keys(std::string("Startup"), startupKeys);
	std::vector<Process> processes;
	for (const auto &key : startupKeys) {
		try {
			const auto group = pConf->getString(std::string("Startup.") + key);
			Process process;
			process.processName = pConf->getString(group + std::string(".Name"));
            if (process.processName == std::string{"App_Communicator"})
            {
                continue;
            }
			process.terminalArguments = split(removeWhitespaces(pConf->getString(group + std::string(".TermArguments"), std::string())));
			process.arguments = split(removeWhitespaces(pConf->getString(group + std::string(".Arguments"), std::string())));
			process.terminalPath = oTermPath;
			process.batch = pConf->getBool(group + std::string(".Batch"), false);
			process.weldMasterApplication = pConf->getBool(group + std::string(".WeldMasterApplication"), true);
            process.useTerminal = pConf->getBool(group + std::string(".UseTerminal"), terminalApplication);
            process.enabled = pConf->getBool(group + std::string(".Enabled"), true);
            process.autoRestart = pConf->getBool(group + std::string(".AutoRestart"), false);
            process.ldPreload = pConf->getString(group + std::string(".LD_PRELOAD"), {"lib/libMod_SimulatedRTCBoard.so"});
			processes.push_back(process);
		} catch (Poco::NotFoundException&) {
			std::cout << "Not found for " << key << std::endl;
			continue;
		}
	}
	return processes;
}

void AsyncSystemStarter::startProcess(const Process &process)
{
    if (!process.enabled)
    {
        return;
    }
	if (process.batch)
	{
		startBatch(process);
		return;
	}
	const std::string pipePath = getenv("XDG_RUNTIME_DIR") + std::string("/wmpipes/") + getenv("WM_STATION_NAME") + std::string("/p") + process.processName;
	const int readFd = createPipe(pipePath);
	startWeldmasterProcess(process, readFd == -1 ? std::string() : pipePath);
	waitForProcessStarted(readFd, process.processName);
	// unlink the named pipe, will be deleted once both sides closed it
	unlink(pipePath.c_str());
}

int AsyncSystemStarter::createPipe(const std::string &pipePath)
{
	if (mkfifo(pipePath.c_str(), S_IRUSR | S_IWUSR) == 0)
	{
		// open nonblocking to not block during open
		// but open here, so that we know whether the read-back can work
		// if the open fails we better don't pass the pipe to the process at all
		return open(pipePath.c_str(), O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	} else
	{
		std::cout << "Creating named pipe failed for " << pipePath << std::endl;
		return -1;
	}
}

/**
 * Copy of FunctorRunnable from Poco::Thread.
 * Extracted as it is only provided as a protected class, thus cannot be used directly.
 **/
template <class Functor>
class FunctorRunnable: public Poco::Runnable
{
public:
    FunctorRunnable(const Functor& functor):
        _functor(functor)
    {
    }

    ~FunctorRunnable()
    {
    }

    void run()
    {
        _functor();
    }

private:
    Functor _functor;
};

void AsyncSystemStarter::startWeldmasterProcess(const Process &process, const std::string &pipePath, bool restarted)
{
	const auto pid = fork();
	if (pid == 0)
	{
		// child process
		// first unblock signals, ConnectServer as a Poco::ServerApplication has signals blocked
		sigset_t signals;
		sigfillset(&signals);
		sigprocmask(SIG_UNBLOCK, &signals, nullptr);
        
		// ConnectServer is a binary with capabilities and thus loses LD_LIBRARY_PATH
		// so the LD_LIBRARY_PATH for weldmaster system needs to be constructed manually and set in the forked process
		std::string ldLibraryPath = wmInstPath() + std::string("/lib:") + std::string(ETHERCAT_LIB) + std::string(":") + std::string(SISODIR_LIB);
		setenv("LD_LIBRARY_PATH", ldLibraryPath.c_str(), 1);
        if (restarted)
        {
            setenv("WM_RESTARTED", "1", 1);
        }
        if (!process.ldPreload.empty())
        {
            setenv("LD_PRELOAD", process.ldPreload.c_str(), 1);
        }

		// now start the actual process by exec
		std::vector<char*> arguments = createProcessArguments(process, pipePath);
        if (process.useTerminal)
        {
            execv(process.terminalPath.c_str(), &arguments[0]);
        } else
        {
            static const std::string s_directory = wmInstPath() + std::string("/bin/");
            execv(std::string(s_directory + process.processName).c_str(), &arguments[0]);
        }
	// cleanup
	for (auto it = arguments.begin(); it != arguments.end(); ++it)
	{
		delete[] *it;
	}
	}
	else if (pid == -1)
	{
		std::cout << "Fork failed" << std::endl;
	} else
    {
        // prevent zombies
        auto wait = [pid, process, this]
        {
            int status = 0;
            waitpid(pid, &status, 0);
            if (process.autoRestart)
            {
                if (WIFSIGNALED(status))
                {
                    int sig = WTERMSIG(status);
                    if (sig == SIGSEGV || sig == SIGBUS || sig == SIGFPE || sig == SIGILL || sig == SIGABRT)
                    {
                        std::lock_guard<std::mutex> lock{m_shutDownMutex};
                        if (!m_shuttingDown)
                        {
                            startWeldmasterProcess(process, {}, true);
                        }
                    }
                }
            }
        };
        std::unique_ptr<Runnable> runnable{new FunctorRunnable<decltype(wait)>(wait)};
        Poco::ThreadPool::defaultPool().start(*runnable);
        m_waitPidThreads.push_back(std::move(runnable));
    }
}

void AsyncSystemStarter::startBatch(const Process &process)
{
	std::string s_path = process.weldMasterApplication ? wmInstPath() + std::string("/batch/") + process.processName : process.processName;
	// for the system call all arguments need to be provided in one string
	// so merge them back together
	s_path = std::accumulate(process.arguments.begin(), process.arguments.end(), s_path,
			[] (std::string a, std::string b) { return a + " " + b; });
    const auto pid = fork();
    if (pid == 0)
    {
        // child process
        // unblock signals which got blocked by Poco::Util::ServerApplication
        sigset_t signals;
        sigfillset(&signals);
        sigprocmask(SIG_UNBLOCK, &signals, nullptr);
        execl("/bin/sh", "sh", "-c", s_path.c_str(), (char *) 0);
    } else if (pid > 0)
    {
        // parent process
        int status = 0;
        waitpid(pid, &status, 0);
    }
}

void AsyncSystemStarter::waitForProcessStarted(int readFileDescriptor, std::string processName, int timeOut)
{
	if (readFileDescriptor == -1)
	{
		// pipe failed
        std::cout << "waitForProcessStarted " << processName << " pipe failed" << std::endl;
		// just sleep instead to wait for process to be started
		Poco::Thread::sleep(5000);
		return;
	}
	char buffer;
	// max wait of timeout, if read failed, continue
	const int runWaitTime = 10;
	for (int i = 0; i < timeOut/runWaitTime; i++)
	{
		const auto ret = read(readFileDescriptor, &buffer, 1);
		// ret == 0 -> other side not opened
		// ret == -1 and EAGAIN -> would block without O_NONBLOCK
		// ret == -1 and not EAGAIN -> other error during read
		if (ret == 0 || (ret == -1 && errno == EAGAIN))
		{
			Poco::Thread::sleep(runWaitTime);
			continue;
		} else if (ret == -1)
		{
			std::cout <<  "WaitForProcessStarted " << processName << " Failed reading from pipe: " << errno << std::endl;
			break;
		}
		break;
	}
	close(readFileDescriptor);
	if (strncmp(&buffer, "1", 1) != 0)
	{
		std::cout << "WaitForProcessStarted " << processName << " Read unexpected value from pipe" << std::endl;
	}
}

static char *toCharArray(const std::string &string)
{
	char *cStr = new char[string.size() + 1];
	strcpy(cStr, string.c_str());
	return cStr;
}

std::vector<char*> AsyncSystemStarter::createProcessArguments(const Process &process, const std::string &pipePath)
{
	// HACK: create the char * const[] arguments in a c++ way
	std::vector<char*> arguments;
    if (process.useTerminal)
    {
        arguments.push_back(toCharArray(process.terminalPath));
        for (const auto &argument : process.terminalArguments)
        {
            arguments.push_back(toCharArray(argument));
        }
    }
	static const std::string s_directory = wmInstPath() + std::string("/bin/");
	arguments.push_back(toCharArray(std::string(s_directory + process.processName)));

	// if we can read on the pipe, we pass the path to the pipe
	if (!pipePath.empty())
	{
		arguments.push_back(toCharArray("-pipePath"));
		arguments.push_back(toCharArray(pipePath));
	}

	for (const auto &argument : process.arguments)
	{
		arguments.push_back(toCharArray(argument));
	}
	// last element in array needs to be a nullptr
	arguments.push_back(nullptr);
	return arguments;
}

void AsyncSystemStarter::setShuttingDown()
{
    std::lock_guard<std::mutex> lock{m_shutDownMutex};
    m_shuttingDown = true;
}
}
}
}
