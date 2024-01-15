/**
 *  @file
 *  @copyright      Precitec Vision GmbH & Co. KG
 *  @author         Wolfgang Reichl (WoR), Ralf Kirchner (KiR), Stefan Birmanns (SB)
 */
#ifndef BASE_MODULE_APP_H_
#define BASE_MODULE_APP_H_

// stl includes
#include <iostream>
#include <list>
// Poco includes
#include "Poco/Path.h"
#include "Poco/Util/Application.h"
// WM includes
#include "module/moduleManagerConnector.h"
#include "message/module.handler.h"
#include "message/module.server.h"
#include "server/proxy.h"
#include "server/eventProxy.h"
#include "message/registrar.proxy.h"
#include "message/receptor.proxy.h"
#include "protocol/protocol.info.h"
#include "module/moduleLogger.h"

namespace precitec {
namespace interface
{
class ConnectionConfiguration;
}

    using namespace interface;

	using system::message::ProtocolInfo;
	using system::message::SmpProtocolInfo;

	using system::module::AnyModule;
	using system::module::InvalidModule;

namespace framework
{
namespace module
{
	using Poco::Util::Application;

	/**
	 *  Klasse, die einen Signalhandler installiert, der fuer geordnetes Aufraeumen sorgt.
	 *  Sie wird als statischer Member in die Modulklasse aufgenommen, damit der Konstruktor
	 *  vor allem andern aufgerufen wird, und der Namespace nicht durch einen Haufen
	 *  globaler Variabler versaut wird.
	 *  So wird ueber das void Handler(void)-Interface eine ueberladene virtuelle Funktion
	 *  (uninitialize()) der Klasse BaseModul aufgerufen. Diese Funktion wird auch dann aufgerufen,
	 *  wenn das Programm irregulaer (kill, Ctrl-C, ...) beendet wird. Im wEsentlichen soll hier
	 *  Hardware aufgeraeumt werden, die sonst auf einen irregulaeren Abbruch beleidigt reagiert (so
	 *  etwa der SISO-Grbber), oder eine Datensicherung ausgefuehrt werden (Logger-Eintraege).
	 */
	class BaseModule;
	class SignalHandler {
	public:
		SignalHandler() { activateHandler(cleanUp); }
		/// falls mehrere Module in einem Executable sind, werden alle beruecksichtigt
		typedef std::list<BaseModule*> ResourceList;
		typedef ResourceList::iterator ResourceIter;

		/// meldet das Modul beim SignalHandler an
		void registerModule(BaseModule *module);
	private:
		/// Std-Handler-Interface
		typedef void (*Handler) (int);
		/// meldet den Handler beim System an
		void activateHandler(Handler h);
		/// ruft uninitialize des BaseModuls auf
		static void cleanUp(int signalNo);

		/// BaseModul-function(s), mutliple functions may be registered, though per default only one is
		static ResourceList	resourceList__;
	};

	/**
	 * Mit dieser Klasse kann e.g. ein Server mit einem Client initialisiert werden,
	 * selbst wenn dieser noch nicht verbunden ist.
	 * Der eigentliche Client aktiviert den Stub in der Routine 'runClientCode'
	 * Greift der Server davor auf den Client zu blockiert der Zugriff.
	 * Mit und nach der aktivierung werden Zugriffe unmittelbar auf den Client weitergeleitet
	 * Diese Klasse ist nur fuer Message-proxies gedacht. Event-proxies blockieren nicht (im
	 * Zweifelsfalle schicken sie ihre Nachrichten ins Nirvana, aber es passiert nichts).
	 * Auch bei MessageProxies dient der Stub nur dazu Extremfaelle abzufangen, da auch ein
	 * realer Server nach der Initialisierung nicht gleich anfaengt loszuschiessen. Dennoch sollte
	 * doeseLoesung verzwickte Synchronisationsprobleme vermeiden helfen.
	 */
//	template <class ClientProxy>
//	class ProxyStub {
//		ProxyStub(ClientProxy &p) : proxy_(p), isActive_(false), active_(p.addObserver()) {}
//	public:
//		ClientProxy &operator () () {
//			if (!isActive_) {
//				active_->lock();
//				delete active_;
//				isActive_ = true;
//			}
//			return proxy_; }
//	public:
//	private:
//		ClientProxy &proxy_;
//		bool 				 isActive_;
//		Poco::FastMutex *active_;
//	};

	/**
	 * BaseModule ist die Basisklasse aller User-Server-Module
	 * Sie bindet das Modul-Interface und ie Poco::Application ein
	 * Sie handelt das REgistreieren von Schnittstellen
	 */
	class BaseModule : public Application, public ModuleManagerConnector {
	public:
		BaseModule(Modules appId);
		virtual ~BaseModule();
	public:
		/// read configuration
		virtual void initialize(Application *app);
        void initModuleManager();
		/// override this to ensure cleanup code is executed even during abnormal termination (curr. used only in Mod_Grabber)
		virtual void uninitialize() { std::cout << "unitinializing module " << ModuleName[getMyAppId()] << std::endl; }
		/// kommt nie zum Zuge
		virtual void reinitialize() {}
		/// initialisiert System und wartet dann bis jemand anderes die Server herunterfaehrt
		int run();
		/**
		 * Processes the command line arguments.
		 *
		 * Implementing classes should invoke this method with the @p argc, @p argv
		 * passed from application startup (e.g. main).
		 *
		 * This method processes the arguments and looks for options it wants to handle.
		 */
		void processCommandLineArguments(int argc, char *argv[]);
	protected:
		/// muss ueberschrieben werden, wenn tatsaechlich Clientcode vorhanden ist
		virtual void runClientCode() {}

		/**
		 *
		 * Notifies the ConnectServer that startup for this BaseModule finished.
		 *
		 * Implementing classes should call this when it is safe to connect to
		 * the application.
		 */
		void notifyStartupFinished();

	public:
//		/// wait for exitLock_ to be write-unlocked; block until this happens
//		void waitForExit() { return exitLock_.readLock(); }
//		/// wait for exitLock_ to be write-unlocked; block until this happens
//		void pollForExitOrUserQuit();
//		/// is exitLock read-lockable
//		bool markedForExit()  { return exitLock_.tryReadLock(); }
//		/// set e.g. by SignalHandler; remove wrielock; anyone waitng for a readlock will be activated
//		void markForExit() { exitLock_.unlock(); }
		/// ist exit-Flag gesetzt
		bool markedForExit() const { return exitFlag_; }
		/// set e.g. by SignalHandler
		void markForExit() { exitFlag_ = true; }

	protected:
		// Subsystem-Interface

		/// wird nach der Initialiseirung der Server-Threads vom Hauptprogramm aufgrufen und wartet auf das Ende der Server
		//void syncAllServers();
		/// Pfad + AppId in Spec-Struktur packen
		ModuleSpec getMyModuleSpec() override;

		SmpProtocolInfo  readConfig() override;
	private:
		// locale Implementation Funktionen

		// nun werden alle benoetigten (die in den Listen) Schnittstellen angemeldet
		void unsubscribeAllInterfaces();
		void listProxies(std::ostream &stream);
		/// Remote-Proxy des Registrars (Modul des ModuleManager, der mit Clients redet)

	private:
		/// Liste aller exportierten Interfaces
		MessageHandlerList	mHandlerList_;
		EventHandlerList		eHandlerList_;
		/// Liste aller importierten Interfaces
		ProxyList						proxyList_;
		/// eine Art globale Variable; der Handler ruft uninitialize fuer das Modul auf
		static SignalHandler	signalHandler__;
//		/// write-lock is set on prog-start; any number of routines waiting for prog end will tryReadLock()
//		Poco::RWLock				exitLock_;
		/// gesetzt -> Programmaustieg \todo convert to mutex
		bool exitFlag_;

		/**
		 * File descriptor to named pipe passed through command line argument.
		 * Set by processCommandLineArguments and used by notifyStartupFinished.
		 */
		int m_oReadyFd;

	public:

	    static std::unique_ptr<precitec::ModuleLogger> m_pBaseModuleLogger; ///< Module logger, messages get collected by LoggerServer process and then send to windows.
};

} // namespace module
} // namespace framework
} // namespace precitec


#endif /*BASE_MODULE_APP_H_*/
