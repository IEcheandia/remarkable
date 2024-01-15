/**
 * @defgroup ModuleManager ModuleManager
 */
#ifndef MODULE_MANAGER_H_
#define MODULE_MANAGER_H_


#include <vector>

#include "mmAccessor.h"
#include "message/receptor.handler.h"
#include "message/registrar.handler.h"
#include "registrar.server.h"
#include "receptor.server.h"
#include "module/interfaces.h" // Module-Liste
 

#include "Poco/Util/ServerApplication.h"


namespace precitec
{
namespace interface
{
	using Poco::Path;
	using Poco::Util::Application;


	/**
	 * der ModuleManager ist der zentrale Verwalter fuer alle Module
	 * Er laedt sie, vermitttelt die kommunikation und ueberwacht ihre
	 * Lebendigkeit.
	 * ModuleManager implementiert das registrar-Interface
	 */
	class ModuleManager : public Application {
	public:
		ModuleManager();
		/// der tut nix
		virtual ~ModuleManager() {}

	protected:
		int  main(const std::vector<PvString>& args);
        
		void defineOptions(Poco::Util::OptionSet& options);

	private:
		// das ServerApplication-Interface
		void initialize(Application& app);
		void uninitialize();
		/// beim Runterfahren natuerlich erst
		void killAllServers() {}

		void notifyStartupFinished();

        void initializeSignalHandler();

        static void signalHandler(int signalNumber);

	public:
		/// ein manuell gestarteter Client verbindet sich so mit dem MM
		static ProtocolInfo contactMM(module::Modules mmTyp);

	private:
		/// der MM ist selbst ein Modul hat daher auch ein modulId_ = ModulManager
		module::Modules  				moduleId_;
	private:

		/// kapselt alle Listen, die von mehreren Threads verwendet werden
		MMAccessor								mmAccessor_;
		/// hiermit koenen andere Prozesse den MM sauber killen
		//SmpNamedMutex						mmKillMutex_;
		/// ModuleHandle_ ist im Wesentlichen die ProcessId
		//int											moduleHandle_;
		/// Mit der Semaphore laesst man das Hauptprogramm warten
		Poco::Semaphore	 					shutdownSync_;

		/// Der MM ist Server und hat hier einen einzigen Server+Handler fuer die Anmeldungen
		TReceptor<MsgServer>			receptorServer_;
		TReceptor<MsgHandler>			receptorHandler_;
        static ModuleManager *s_self;
	};


} // namespace interface
} // namespace precitec

#endif /*MODULE_MANAGER_H_*/
