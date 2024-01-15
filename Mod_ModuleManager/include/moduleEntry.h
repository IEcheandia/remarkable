#ifndef MODULEENTRY_H_
#define MODULEENTRY_H_

#include <list>
#include "Poco/SharedPtr.h"

#include "system/types.h"		// wg PvString
#include "message/module.h"	// wg ModuleSpec, ModuleType, ModuleInfo
// Vorwaertsdeklaration moeglich ???
#include "message/module.interface.h"
#include "message/registrar.interface.h"
#include "server/interface.h" // wg MsgProy, MsgHandler, MsgServer

namespace precitec
{

namespace interface
{
	typedef Poco::SharedPtr< TModule<MsgProxy> >			SmpModuleProxy;
	typedef Poco::SharedPtr< TRegistrar<MsgServer> >	SmpModuleServer;
	typedef Poco::SharedPtr< TRegistrar<MsgHandler> >	SmpModuleHandler;
	class MMAccessor;

	/**
	 * Eintrag dem Registrars (ModuleManagers) fuer jedes Modul
	 * name_, interfaceId_ identifizieren ein Modul fuer andere Module
	 * extension_ ist eine Art Dummy fuer zukuenftige Aufgaben
	 * interfaceId_ und processId werden vom MM intern verwendet und sind beide eindeutig
	 */
	class ModuleEntry	{
	public:
		/// ModuleEntry wird ganz oder gar nicht gesetzt
		ModuleEntry();
		/// der CTor fuer einen vollstaendigen Eintrag
		ModuleEntry(ModuleSpec const& spec, ModuleInfo const& info,
								MMAccessor &accessor, SmpProtocolInfo & proxyPInfo, SmpProtocolInfo & handlerPInfo);
		/// copy-CTor
		ModuleEntry(ModuleEntry const& rhs);
		friend std::ostream& operator << (std::ostream&os, ModuleEntry const&me);
	public:
		// diverse Accessoren
		PvString const& fileName() 		const { return spec_.fileName(); }
		int extension() 						const { return info_.extension(); }
		// == ApplicationId - Es gibt u.U. meherer Mods mit gleichem Id
		int moduleId() 							const { return spec_.moduleId(); }
		// Eindeutig! (PID)
		int handle() 								const { return info_.moduleHandle(); }
		bool isThread() 					 	const	{ return info_.isThread(); }
		TModule<MsgProxy>& proxy()				{ return *module_; }

		/// startet handler
		//void activate();
		/// startet Server des Moduls, gibt Protokol fuer Client zurueck
		SmpProtocolInfo	activateServer(int interfaceId, SmpProtocolInfo & serverProtInfo, const std::string &path);
		/// startet Server des Moduls, gibt Protokol fuer Client zurueck
		//SmpProtocolInfo	activateServer(int interfaceId, SmpProtocolInfo & serverProtInfo, PvString const& shMemName);
		/// startet Server des Moduls; Protokoll existiert bereits
		void reActivateServer(int interfaceId, SmpProtocolInfo & serverPInfo);
		/// konfiguriert Sender/Signaler des Client-Moduls
		void activateClient(int interfaceId, SmpProtocolInfo & serverProtInfo, int eId=module::AllEvents, int subAppId = module::AnyModule, const std::string &path = {});

		//SmpProtocolInfo	moduleProtocol() const { return moduleProtocol_; }
	private:
		/// diese Werte spezifizieren ein Module (etwa in der Konfig) nach aussen (viele Strings)
		ModuleSpec				spec_;
		/// die bein Laden des Moduls gesetzen Werte: pId, extension, ...
		ModuleInfo 				info_;
		/// Proxy des Moduls
		SmpModuleProxy		module_;
		/// Server fuer das Modul
		SmpModuleServer		server_;
		/// Handler fuer das Modul
		SmpModuleHandler	handler_;

		/// prototol zur Kommunikation zwischen Pub/Sub
		//SmpProtocolInfo moduleProtocol_;
	}; // ModuleEntry

	typedef std::list<ModuleEntry> 			ModuleList;
	typedef ModuleList::iterator 				ModuleIter;
	typedef ModuleList::const_iterator 	ModuleCIter;

} // namespace interface
} // namespace precitec

#endif /*MODULEENTRY_H_*/
