/**
 *  @file
 *  @copyright      Precitec Vision GmbH & Co. KG
 *  @author         Wolfgang Reichl (WoR), Ralf Kirchner (KiR), Stefan Birmanns (SB)
 */

#include <signal.h>

// WM includes
#include "module/baseModule.h"
#include "module/moduleLogger.h"
#include "common/connectionConfiguration.h"
// Poco includes
#include "Poco/Thread.h"

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

namespace precitec
{
	using namespace interface;
	using system::module::InterfaceName;

namespace framework {
namespace module {

	/// Klasse die einen Signalhandler installiert der fuer geordnetes Aufraeumen sorgt
	SignalHandler	BaseModule::signalHandler__;

	/// hier werden die ModulPointer gespeichert
	SignalHandler::ResourceList	SignalHandler::resourceList__;

	/// cleanUp ruft virtual uninitialize() auf
	void SignalHandler::cleanUp(int sigNo) {
		// Da der Hanlder nur fuer diese Signale registriert ist und wird immer das Gleiche tun
		// ist diese Abfrage etwas redundant
		if ( (sigNo==SIGTERM) || (sigNo=SIGINT) ) {
			for (ResourceIter r(resourceList__.begin()); r!=resourceList__.end(); ++r) {
				//std::cout << "SignalHandler cleaning up: " << InterfaceName[(*r)->getMyAppId()] << std::endl;
				(*r)->uninitialize(); // run cleanup Code
				(*r)->markForExit(); // allow routines polling Flag to clear
			}
		}
	}

	void SignalHandler::activateHandler(Handler h) {
		//std::cout << "SignalHandler activating handler: " << std::endl;
		// create a signal-set for SIGUSER1 and SIGKILL
		sigset_t set;
		sigemptyset(&set);
		// nur diese beiden Signale kann der Handler bekommen ???? oder!?
		sigaddset(&set, SIGTERM);
		sigaddset(&set, SIGINT);

		// bind handler to both signals
		struct sigaction act;
		act.sa_flags = 0;
		act.sa_mask = set; // mask for flags in above signal-set
		act.sa_handler = h; // the callback a.k.a. handler
		sigaction(SIGINT, &act, NULL);
		sigaction(SIGTERM, &act, NULL);
	}


	void SignalHandler::registerModule(BaseModule *module) {
		//std::cout << "SignalHandler registring: " << InterfaceName[module->getMyAppId()] << std::endl;
		resourceList__.push_back(module);
		//std::cout << "SignalHandler registered: " << InterfaceName[module->getMyAppId()] << std::endl;
	}

	std::unique_ptr<precitec::ModuleLogger> BaseModule::m_pBaseModuleLogger = nullptr;

	/// AppId wird von 'oben' durchgereicht, weil sie zum abgeleiteten Modul gehoert
	BaseModule::BaseModule(Modules modId)
	: ModuleManagerConnector(modId),
		exitFlag_(false),
		m_oReadyFd(-1)
	{
		// register exit-handler (defined in derived module class)
		signalHandler__.registerModule(this);

		// BaseModule Logger
		m_pBaseModuleLogger = std::unique_ptr<precitec::ModuleLogger>(new precitec::ModuleLogger( system::module::ModuleName[modId] ));

		// init message
		wmLogTr( eStartup, "BaseModul.Startup", "Modul %s wurde gestartet.\n", system::module::ModuleName[modId].c_str() );
  	}

	BaseModule::~BaseModule()
	{
	}

	SmpProtocolInfo  BaseModule::readConfig() {
		PvString loopBackDevice("127.0.0.1");
		PvString serverPort = PvString(config().getString("BaseModul.ip", ""));
		if (serverPort.empty())
			return SmpProtocolInfo();
		else
			return SmpProtocolInfo (new SocketInfo(Udp, loopBackDevice, serverPort) );
	}

	/// generate ModuleSpec for current module
	ModuleSpec BaseModule::getMyModuleSpec() {
		PvString appPath(config().getString("application.path", ""));
		return ModuleSpec(getMyAppId(), appPath);
	}

	/// debug routine to output list of exported USED interfaces of module
	void BaseModule::listProxies(std::ostream &stream) {
		// \todo turn into logMessages
		stream << "ProxyList: " << std::endl;
		for (ProxyList::iterator proxy=proxyList_.begin(); (proxy!=proxyList_.end()); ++proxy) {
			int interfaceId = proxy->first;
			stream << "\tpublishing " << InterfaceName[interfaceId] << std::endl;
		}
	}

//	/// wait for program to be killed or user kill-request on console (typing 'q')
//	void BaseModule::pollForExitOrUserQuit()  {
//		char ch('n');
//		while ( !exitLock_.tryReadLock() ) {
//			// if module is still running
//			// ... wait a while
//			int timeOut=250; // ms;
//			Poco::Thread::sleep(timeOut);
//			// then check for user-input
//			std::cin >> ch;
//			// if user flagged his will to terminate, tell the other quys
//			if (ch=='q') markForExit();
//		}
//	}

	/**
	 * Wenn die Server schon laufen, kann endlich der eigentliche
	 * Client-Code aufgerufen werden. Da die BasisKlasse BaseModule
	 * diesen Cod natuerlich selbst nicht kennt, wird er in der
	 * abgeleiteten Klasse mit der dort zu ueberladenden Meemberfunktion
	 * runClientCode ausgefuehrt. Die implemenierung der Basisklasse tut nichts
	 * (reiner Serverbetrieb)
	 */
	int BaseModule::run() {
		//std::cout << "BaseModule::run()<" << ModuleName[getMyAppId()] << "> " << std::endl;
		// alle Interfaces werden einzeln gepublished und die Server gestartet
		// diese Funktion muss natuerlich von den abgeleiteten Klassen ueberschrieben werden
		//std::cout << "BaseModule::run()<" << ModuleName[getMyAppId()] << "> subscribing" << std::endl;
		subscribeAllInterfaces();
		//std::cout << "BaseModule::run()<" << ModuleName[getMyAppId()] << "> subscribed" << std::endl;
		// nun werden alle benoetigten Schnittstellen angemeldet
		if (publishAllInterfaces()) {
			// nur wenn wir nicht schon bei der Anmeldung abgeschossen wurden
			//wmLog( eDebug, "BaseModule::run()<%s> starting client code.\n", ModuleName[getMyAppId()].c_str() );
			runClientCode();
			//wmLog( eDebug, "BaseModule::client done, only server running.\n" );
//			pollForExitOrUserQuit();

			while ( !markedForExit() ) { usleep( 500*1000 ); }

			//moduleServer_.waitForAllServers();
			//unpublishAllInterfaces();
			//unsubscribeAllInterfaces();

			// meldet Modul bei Receptor UND Registrar ab!
			//receptor().unregisterModule(getModuleHandle());
		} else {
			// Abgemeldung hat schon ein anderer veranlasst, e.g. Partner-Pub/Sub hat Schluss gemacht
		    wmLog( eInfo, "BaseModule::client shutting down per request while publishing.\n" );
		}
		//moduleServer_.waitForAllServers();

		//std::cout << "BaseModule::end" << std::endl;
		return 0;
	}

	/**
	 * overloaded  function allowing specific initialization tasks to be executed
	 * @param app sort of a this-pointer (Poco::interface)
	 */
	void BaseModule::initialize(Application *app) {
        initModuleManager();
    }

    void BaseModule::initModuleManager()
    {
        ModuleManagerConnector::initModuleManager(ConnectionConfiguration::instance());
    }


/*
	void BaseModule::unpublishAllInterfaces() {
		for (ProxyList::iterator proxy=proxyList_.begin(); proxy!=proxyList_.end(); ++proxy) {
			int interfaceId = proxy->first;
			ProxyEntry &proxEntry(proxy->second);
			registrar().unpublish(getModuleHandle(), interfaceId, proxEntry.numMessages());
		}
		return notShuttingDown;
	}
*/

	void BaseModule::unsubscribeAllInterfaces() {
		for (MessageHandlerList::iterator handler=mHandlerList_.begin(); handler!=mHandlerList_.end(); ++handler) {
			int interfaceId = handler->first;
			AnalyzerEntry &msgEntry(handler->second);
			registrar().unsubscribe(getModuleHandle(), interfaceId, msgEntry.handler->info_.numMessages, getMyAppId());
		}
		for (EventHandlerList::iterator handler=eHandlerList_.begin(); handler!=eHandlerList_.end(); ++handler) {
			int interfaceId = handler->first;
			EventServerEntry &evntEntry(handler->second);
			registrar().unsubscribe(getModuleHandle(), interfaceId, evntEntry.handler->info_.numMessages, getMyAppId());
		}
	}


	void BaseModule::processCommandLineArguments(int argc, char *argv[])
	{
		for (int i = 0; i < argc; i++)
		{
			if (strcmp("-pipePath", argv[i]) == 0)
			{
				poco_assert(i + 1 < argc);
				m_oReadyFd = open(argv[i+1], O_WRONLY);
				unlink(argv[i+1]);
				break;
			}
		}
	}

	void BaseModule::notifyStartupFinished()
	{
		if (m_oReadyFd == -1)
		{
			// didn't get an fd as command line argument, cannot notify parent process
            std::cout << "Cannot notify parent process that startup has finished - " << ModuleName[getMyAppId()] << std::endl;
			return;
		}
		write(m_oReadyFd, "1", 1);
		close(m_oReadyFd);
		// setting fd to -1 to ensure that we don't write in the closed pipe again.
		m_oReadyFd = -1;
		std::cout << "Notify startup finished - " << ModuleName[getMyAppId()] << std::endl;
	}

} // namespace module
} // namespace framework
} // namespace precitec

