#ifndef MODULE_STRUCTS_H_
#define MODULE_STRUCTS_H_

#include <map>
#include "message/serializer.h"
#include "server/interface.h"
#include "server/proxy.h"
#include "server/eventProxy.h"
#include "server/handler.h"
#include "protocol/protocol.h"
#include "protocol/protocol.info.h"
#include "module/interfaces.h"
#include "message/messageBuffer.h"

namespace precitec
{
//	using namespace system::message;
	using system::message::MessageBuffer;
	using system::message::Serializable;
namespace interface
{

	using system::module::Modules;
	using system::module::InvalidModule;
	using system::module::NumApplications;
	using system::module::ModuleName;

	/**
	 * definiert ein Module aus Anwendersicht
	 * Oft wird es fuer ein Interface nur eine Applikation geben. Da die
	 * Interfaces (repraesentiert durch einen Globalen enum InterfaceId) moeglichst
	 * abstrakt und vielseitig sein sollen, kann es etwa zum Interface Logger
	 * die Implementierungen RealTimeLogger, OffLineLogger geben.
	 * Zu jeder Applikation kann es wiederum mehrere konkrete Module geben, etwa
	 * mit verschiedenen Versionsnummer oder eine Debug-Version
	 *
	 * jeweil gilt fuer solche Eintraege: No<typ>::Value bedeuten bei einer Suche: interessiert nicht
	 * Die Modul-Spec wird insbesondere zum Suchen von Default-Server/Publishern in
	 * Konfigurations-eintraegen verwendet. Diesen Eintraege muessen dann natuerlich
	 * vollstaendig sein, damit fehlende Eintraege ergaenzt werden koennen.
	 */
	class ModuleSpec : public Serializable {
	public:
		/// CTor fuer Arrays oder std::container
		ModuleSpec()
		: moduleId_(InvalidModule), fileName_(No<PvString>::Value) {}
		/// Std-CTor der Ideealfall
		ModuleSpec(Modules mId, PvString fName)
		: moduleId_(mId), fileName_(fName) {}

		ModuleSpec(ModuleSpec const& rhs) : moduleId_(rhs.moduleId_), fileName_(rhs.fileName_) {}

		ModuleSpec(MessageBuffer const& buffer);

		virtual void serialize	(MessageBuffer &buffer ) const;

		virtual void deserialize(MessageBuffer const& buffer );

		void swap(ModuleSpec & rhs);

	public:
		// die normale Accessoren
		void setModuleId(Modules mId) { moduleId_ = mId; }
		Modules	moduleId() const { return moduleId_; }
		//uInt appId() const { return moduleId_; } // depricated
		void setFileName(PvString const& fName) { fileName_ = fName; }
		PvString const& fileName() const { return fileName_; }
		friend std::ostream& operator << (std::ostream&os, ModuleSpec const&s) {
			if (	(s.moduleId_==InvalidModule)
				 || (s.moduleId_>=NumApplications) ) {
				os << "Invalid Module";
			} else {
				os << "Mod: <"	<< ModuleName[s.moduleId_];
//				if (!s.fileName_.empty())
//				{ os << " @ " << s.fileName_; }
				os  << "> ";
			}
				return os;
		}
	private:
		/// Enum des Module
		Modules moduleId_;
		/// konkreter ModulName incl.Pfad (sonst defaultPfad)
		PvString fileName_;
	}; // ModuleSpec

	/// Wie wird ein Modul gestartet
	enum ModuleType { InvalidType=-1, ThreadModule, ProcessModule, LocalModule };

	/**
	 * Wenn die ModuleSpec ein Modul aus Anwendersicht (nach aussen) repraesentiert
	 * so gib die Moduleinfo die Sicht des Registrars wieder. Die ProzessId ist eindeutig und
	 * vom 'System' vergeben. Die Extension gibt an, wenn ein Modul mehrfach geladen ist. Etwa
	 * Analyzer0, Analyzer1.
	 */
	class ModuleInfo
	{
	public:
		ModuleInfo() : moduleHandle_(-1), extension_(-1), type_(InvalidType) {}
		ModuleInfo(int mId, int ext, ModuleType t) : moduleHandle_(mId), extension_(ext), type_(t) {}
		int moduleHandle() const { return moduleHandle_; }
		ModuleType type() const { return type_; }
		bool isThread() const { return type_==ThreadModule; }
		int extension() const { return extension_; }
		void setProtocol(SmpProtocol & p) { protocol_ = p;}
		SmpProtocol protocol() const { return protocol_; }
		friend std::ostream& operator << (std::ostream&os, ModuleInfo const&i) {
			os << "Info: <" << i.moduleHandle_  << "." << i.extension_ << i.protocol_->protocolType() << ">: "; return os;
		}
	private:
		/// processId oder (spaeter) threadId
		int moduleHandle_;
		/// ein Modul kann vllt einmal mehrfach geladen sein (?? Versionen, high availiability????, Win/QNX)
		int extension_;
		/// definiert, was processId_ letztlich ist.
		ModuleType type_;
		/// hiermit kommuniziert der MM mit dem (immer vorhandenen) Modul-Interface des Moduls
		SmpProtocol protocol_;
	}; // ModuleInfo


	typedef interface::Server<MsgHandler>* 		PMHandler;
	typedef interface::Server<MsgProxy>* 	 		PMProxy;
	typedef interface::Server<EventHandler>* 	PEHandler;
	typedef interface::Server<EventProxy>* 	 	PEProxy;

	/**
	 * Listeneintraege fuer die Module
	 * Hier steht fuer jedes Unterstuetzte Innterface der Handler
	 * und (optional) appId und Pfad des gewuenschten Gegenueber
	 */
	class EventServerEntry
	{
	public:
		EventServerEntry() {}
		EventServerEntry(PEHandler h, int id, PvString p)
		: handler(h), appId(id), path(p) {}
	public:
		PEHandler 	handler;
		int 				appId;
		PvString			path;
	};
	class AnalyzerEntry
	{
	public:
		AnalyzerEntry() {}
		AnalyzerEntry(PMHandler h, int id, PvString p)
		: handler(h), appId(id), path(p) {}
	public:
		PMHandler 	handler;
		int 				appId;
		PvString			path;
	};

	// jeder Handler wird mit seinem Interface in die Map eingetragen
	typedef std::map<int, AnalyzerEntry> MessageHandlerList;
	typedef std::map<int, EventServerEntry> 	EventHandlerList;
	typedef MessageHandlerList::iterator MHandlerListIter;
	typedef EventHandlerList::iterator EHandlerListIter;

	/**
	 * zu jedem Modul (Server oder Client) gibt es ein Proxy, ueber den remote
	 * Server/Clients gestartet und gestopt werden koennen
	 * Zu jedem Modul gibt es eine eigene Verbindung (protocol)
	 * Normalerweise identifiziert die appId ein Modul, aber es kann vorkommen, dass
	 * mehrere module der gleichen App existieren (Zukunft) daher noch der Pfad
	 */
	class ProxyEntry
	{
	public:
		ProxyEntry() {}
		ProxyEntry(PMProxy mp, Modules id, PvString p) : mProxy_(mp), eProxy_(NULL), modId(id), path(p) {}
		ProxyEntry(PEProxy ep, Modules id, PvString p) : mProxy_(NULL), eProxy_(ep), modId(id), path(p) {}
		void setProtocolMP(SmpProtocolInfo &p) { mProxy_->activate(p);	}
		void setProtocolEP(SmpProtocolInfo &p) { eProxy_->activate(p); }
		int numMessages() const { return mProxy_ ? mProxy_->numMessages() : eProxy_->numMessages();	}
		interface::Server<MsgProxy> &msgProxy() const { return *mProxy_; }
		// existiert vermutlich nur wg debug  \todo rausschmeissen und durch msgProxy ersetzen
		interface::Server<MsgProxy> *msgProxyX() const { return mProxy_; }
		interface::Server<EventProxy> &eventProxy() const { return *eProxy_; }

        bool isEventProxy() const
        {
            return eProxy_ != nullptr;
        }
        bool isMessageProxy() const
        {
            return mProxy_ != nullptr;
        }

		//ProxyEntry(int id, PvString p) : appId(id), path(p) {}
	public:
		/// \todo m/e-proxy Eintraege sind alternativ!!! union ausprobieren!!!
		PMProxy	mProxy_;
		PEProxy	eProxy_;
		/// hierueber kann e.g. auf den Modul-Name (globaler ModuleName-Array) zugegriffen werden
		Modules	modId;
		/// wichtig, wenn ein bestimmtes Modul gestartet werden soll
		PvString	path;
	};
	// gemapt wird auf die InterfaceNummer ???? ist map wirklich gut hier????
	typedef std::map<int, ProxyEntry> ProxyList;




	//-------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------
	//-------------------------------------------------------------------------------------------
	//--------- definitions

	inline ModuleSpec::ModuleSpec(MessageBuffer const& buffer)
	: moduleId_(Modules(Serializable::deMarshal<int>(buffer))),
		fileName_(Serializable::deMarshal<PvString>(buffer))
	{
	}

	inline void ModuleSpec::serialize	(MessageBuffer &buffer ) const {
		marshal(buffer, int(moduleId_));
		marshal(buffer, fileName_);
	}

	inline void ModuleSpec::deserialize(MessageBuffer const& buffer ) {
		ModuleSpec tmp(buffer);
		swap(tmp);
	}

	inline void ModuleSpec::swap(ModuleSpec & rhs)	{
		Modules tmp(rhs.moduleId_); moduleId_=rhs.moduleId_; rhs.moduleId_=tmp;
		std::swap(fileName_, rhs.fileName_);
	}

} // namespace interface
} // namespace precitec

#endif /*MODULE_STRUCTS_H_*/
