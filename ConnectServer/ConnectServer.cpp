#include "ConnectServerHandler.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/Util/PropertyFileConfiguration.h"
#include "Poco/AutoPtr.h"
#include "ConnectServerHandler.h"

#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <sys/param.h>


using Poco::Util::ServerApplication;
using Poco::Util::Application;
using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::HelpFormatter;
using Precitec::Service::Discovery::ConnectServerHandler;



class ConnectServer: public Poco::Util::ServerApplication
{
public:
	ConnectServer():
		_helpRequested(false)
	{
		std::string homeDir(getenv("WM_BASE_DIR"));
		confFileName = homeDir + "/system_config/Connect.config";
	}
	virtual ~ConnectServer()
	{
	}

protected:
	void initialize(Application& self)
	{
		ServerApplication::initialize(self);
	}

	void uninitialize()
	{
		ServerApplication::uninitialize();
	}

	void defineOptions(OptionSet& options)
	{
		ServerApplication::defineOptions(options);

		options.addOption(
				Option("help", "h", "display help information on command line arguments")
					.required(false)
					.repeatable(false));
		options.addOption(
				Option("config", "c", "set the configuration file. Default is Connect.config")
					.required(false)
					.repeatable(false)
					.argument("Configuration Filename"));
        options.addOption(
            Option("autostart", "s", "automatic startup of system if set.")
                    .required(false)
                    .repeatable(false));
	}

	void handleOption(const std::string& name, const std::string& value)
	{
		ServerApplication::handleOption(name, value);

		if (name == "help")
			_helpRequested = true;
		if (name == "config")
			confFileName = value;
	}

	void displayHelp()
	{
		HelpFormatter helpFormatter(options());
		helpFormatter.setCommand(commandName());
		helpFormatter.setUsage("OPTIONS");
		helpFormatter.setHeader("The SGM2 connection server ( needs a Poco::Util::PropertyFileConfiguration file ./Connect.config");
		helpFormatter.format(std::cout);
	}

	int main(const std::vector<std::string>& args)
	{
		if (_helpRequested)
		{
			displayHelp();
		}
		else
		{
			Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> pConf ;

			// test for configuration file
			try
			{
				pConf = new Poco::Util::PropertyFileConfiguration(confFileName);
			}
			catch(...)
			{
				std::cout << confFileName << "  (a Poco::Util::PropertyFileConfiguration file) was not found, exiting... " << std::endl;
				return Application::EXIT_CONFIG;
			}

            sigset_t mask;
            sigemptyset(&mask);
            sigaddset(&mask, SIGTERM);
            sigaddset(&mask, SIGINT);
            sigaddset(&mask, SIGQUIT);
            sigaddset(&mask, SIGUSR1);
            sigaddset(&mask, SIGUSR2);
            sigprocmask(SIG_BLOCK, &mask, NULL);
            int sfd = signalfd(-1, &mask, SFD_CLOEXEC);

			Precitec::Service::Discovery::ConnectServerHandler sh{};
			sh.confFileName = confFileName;
			sh.start();

            while (true)
            {
                signalfd_siginfo sigInfo;
                if (read(sfd, &sigInfo, sizeof(struct signalfd_siginfo)) != sizeof(struct signalfd_siginfo))
                {
                    continue;
                }
                switch (sigInfo.ssi_signo)
                {
                case SIGUSR1:
                    sh.shutdownSystem();
                    break;
                case SIGUSR2:
                    sh.restartSystem();
                    break;
                default:
                    sh.stopAll();
                    break;
                }
                break;
            }

			sh.stop();
		}
		return Application::EXIT_OK;
	}

private:
	bool 		_helpRequested;
	std::string confFileName;
};


int main(int argc, char** argv)
{
	ConnectServer app;
	return app.run(argc, argv);
}


