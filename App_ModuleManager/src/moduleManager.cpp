#include "moduleManager.h"
#include "mmAccessor.h"
#include "protocol/protocol.info.h"
#include "common/connectionConfiguration.h"

#include "Poco/Path.h"
#include "Poco/NamedMutex.h"
#include "Poco/NumberFormatter.h"
#include "Poco/ScopedLock.h"
#include "Poco/Environment.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Util/Application.h"
#include "Poco/Util/OptionException.h"

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

using Poco::Path;
using Poco::NumberFormatter;
using Poco::Environment;
using Poco::Util::Application;

using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;


namespace precitec
{
	using system::message::SmpProtocol;
	using system::module::ModuleManager;
namespace interface
{

ModuleManager *ModuleManager::s_self = nullptr;

void ModuleManager::signalHandler(int signalNumber)
{
    if (!ModuleManager::s_self)
    {
        return;
    }
    if (signalNumber != SIGTERM && signalNumber != SIGINT)
    {
        return;
    }
    s_self->shutdownSync_.set();
    s_self = nullptr;
}

	ModuleManager::ModuleManager()
		: /*moduleHandle_(return Poco::Process::id()),*/
			moduleId_(system::module::ModuleManager),
			shutdownSync_(0, 1),
			receptorServer_(mmAccessor_),
			receptorHandler_(&receptorServer_)
	{
        s_self = this;
		std::cout << "MM INIT" << std::endl;
		SmpProtocolInfo protInfo(new SocketInfo(Udp, "127.0.0.1", ConnectionConfiguration::instance().getString("Receptor.Port", "49900")));
		receptorHandler_.activate(protInfo);
		std::cout << "MM INIT" << std::endl;
	}

	void ModuleManager::initialize(Application& self)	{
		Application::initialize(self);
        initializeSignalHandler();

		loadConfiguration(); // load default configuration files, if present

		ConnectionConfiguration::instance().setInt( pidKeys[MODULEMANAGER_KEY_INDEX], getpid() ); // let ConnectServer know our pid
	}

void ModuleManager::initializeSignalHandler()
{
    // install signal handler
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGINT);

    struct sigaction act;
    act.sa_flags = 0;
    act.sa_mask = set;
    act.sa_handler = &signalHandler;
    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
}

	void ModuleManager::uninitialize() {
		//logger().information("shutting down ModuleManager");
		killAllServers();
		Application::uninitialize();
	}

	void ModuleManager::defineOptions(OptionSet& options) {
		Application::defineOptions(options);
		// need to specify the argument connectserver might pass
		// otherwise app crashes on startup
		options.addOption(Poco::Util::Option(std::string("pipePath"), std::string("pipePath")));
	}

	int ModuleManager::main(const std::vector<std::string>& args) {
		std::cout << "MM::starting up" << std::endl;
		initialize(*this);
		std::cout << "receptor running -> MM::locked" << std::endl;

		notifyStartupFinished();

        shutdownSync_.wait();
		receptorHandler_.stop();
		std::cout << "MM ending regularily" << std::endl;
		return Application::EXIT_OK;
	}

	void ModuleManager::notifyStartupFinished()
	{
		// find the argument we are looking for
		const int argc = config().getInt(std::string("application.argc"));
		for (int i = 0; i < argc; i++)
		{
			const auto arg = config().getString(std::string("application.argv[") + NumberFormatter::format(i) + std::string("]"));
			if (arg == "-pipePath")
			{
				poco_assert(i+1 < argc);
				const auto nextArg = config().getString(std::string("application.argv[") + NumberFormatter::format(i + 1) + std::string("]"));
				const int fd = open(nextArg.c_str(), O_WRONLY);
				unlink(nextArg.c_str());
				if (fd != -1)
				{
					write(fd, "1", 1);
					close(fd);
				}
				break;
			}
		}
	}

} // namespace interface
} // namespace precitec


 POCO_APP_MAIN (precitec::interface::ModuleManager)
