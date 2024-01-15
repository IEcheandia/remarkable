#ifndef MMACCEPTOR_H_
#define MMACCEPTOR_H_

#include <vector>
#include <list>
//#include <map>

#include "Poco/RWLock.h"

// auf Projekt Interfaces
#include "message/module.h"
#include "message/registrar.interface.h"
#include "message/registrar.interface.h"
#include "message/registrar.iterators.h"
#include "message/module.interface.h"

// aus Projekt System
#include "server/interface.h"

// aus Projekt Framework
#include  "paired.h"
#include 	"moduleEntry.h" // wg Module-Liste

namespace precitec
{
	using interface::TRegistrar;
	using interface::MsgServer;
	using interface::MsgHandler;
	using system::module::Interfaces;

namespace interface
{
	typedef Poco::SharedPtr<Poco::Semaphore> SmpSemaphore;

       template<> class TRegistrar<MsgServer>;
       template<> class TRegistrar<MsgHandler>;

	//class MMAccessor;
	/**
	 * Server und Hanlder gehoeren immer zusammen. Die Struktur ist fuer die
	 * Liste der Registrare (je einen pro Modul) gedacht.
	 * Sortiert wird diese Liste nach der ModuleHandle (= eindeutige ProcessId/ThreadId)
	 *//*
	struct RegHandlerPair {
		RegHandlerPair() : server(NULL), handler(NULL), moduleHandle(0) {}
		RegHandlerPair(MMAccessor &ca, SmpProtocolInfo p, int mHandle);
		~RegHandlerPair();

		TRegistrar<MsgServer> 	*server;
		TRegistrar<MsgHandler>	*handler;
		int											 moduleHandle;
	};
	*/
	/// std-container nur mit Smart-Ptr in RegHandlerPair(wg copy/delete), daher hier statischer Array
	//typedef RegHandlerPair Registrars[MaxModules];
	//typedef std::vector<RegHandlerPair*> Registrars;

	/**
	 * Im MMAccessor sind alle Datenstrukturen, die von verschiedenen Threads des MM
	 * verwendet werden sie sind durch ein gemeinsames Mutex geschuetzt.
	 */
	class MMAccessor {
	public:
		enum { MaxModules = 100 }; ///< willkuerlich, bis wir genaueres wissen
		MMAccessor();
		~MMAccessor();
	public:
		// Receptor-interface
		//void addRegistrar(RegHandlerPair *rPair);
		/// fuer jedes angemeldete Modul gibt es einen eigenen Verwaltungs-Server
		void addModule(ModuleSpec spec, int moduleHandle, ModuleType typ,
									 SmpProtocolInfo & proxyPInfo, SmpProtocolInfo & handlerPInfo);

		/// exit eines Moduls
		void removeModule(int moduleHandle);

	public:
		/// sucht moduleHandle in ModuleListe
		ModuleCIter findModule(int moduleHandle) const;
		ModuleIter findModule(int moduleHandle);
		/// aufgerufen von unregister Module
		void unpublishAll(int moduleHandle);
		/// aufgerufen von unregister Module
		void unsubscribeAll(int moduleHandle);


	public:
		// debug Interface
		/// debug->Ausgabe
		void listModules(std::ostream &os);
		/// alle Publications auflisten
		void listCatalogue(std::ostream &os) const;
		/// alle Subscriptions auflisten
		void listSubscriptions(std::ostream &os) const;
		/// alle ActivationPairs auflisten
		void listActivations(std::ostream &os) const;
		/// alle verbindbaren Publisher auflisten
		void listWaiting(std::ostream &os) const;


	public:
		// Regitrar-Interface
		/// Publisher (allg. Modul) gibt sein Interface (alle verwendeten Signale) bekannt
		PublicationIter addPublication(int moduleHandle, Interfaces interfaceId,
										int appId, Publication &publication, int numEvents, int subAppId, const std::string &path);

		/// Subscriber (Event-Server) gibt Interface bekannt, Messages werden autoconnected
		SubscriptionIter addSubscription(Subscription &subscription);

		/// activate any newly formed pairs if auto activate is on the subscriber
		TModule<MsgProxy> &proxyFromHandle(int moduleHandle);



		void activateMatchingPublications(SubscriptionIter sub);
		bool activateMatchingSubscriptions(PublicationIter pub);

		void kill();

	private:
		PairedCIter	findPaired(PublicationCIter pub, SubscriptionCIter sub) const;
		PairedCIter	findPaired(PublicationCIter pub) const;
		void deactivateAll(PublicationIter p);
		void deactivateAll(SubscriptionIter s);

		// diese Klassen sind logisch intern, duerfen daher auf private Funktionen zugreifen
		friend class Publication;
		friend class Subscription;
		//friend class ActivationPair;

	private:
		/// Publisher Event eventId wird an Subscriber weitergeleitet
		void activate(PublicationIter pub, SubscriptionIter sub, int eventId);

		/// Publisher Event eventId wird nicht mehr an Subscriber weitergeleitet
		void deactivate(PublicationIter pub, SubscriptionIter sub, int eventId);

		/// passen Publication und Subscription zusammen ?
		bool match(Publication &pub, Subscription& sub);

		/// einfachere  Accessor zu Module-Liste
		ModuleEntry &moduleEntry(int moduleHandle);

		/// ersteinmal alles Udp, bald noc hQnxMessaging
		ProtocolType findFittingProtocolType(int interfaceId, int moduleId0, int moduleId1);
		/// ist Publication gepaart worden (wg Ausstieg nach Pub-Unblocken)
		bool isPaired(PublicationIter pub) {
			for (PairedIter p=activationPairs_.begin(); p!=activationPairs_.end(); ++p) {
				if (p->pub()==pub) return true;
			}
			return false;
		}

		/// fuer jedes angemeldete Modul gibt es einen eigenen Proxy-Server; add fuer neue Module
		//void addModuleProxy(ModuleEntry const& entry) { /*lock_.writeLock();*/ moduleList_.push_back(entry); }
		/// fuer jedes angemeldete Modul gibt es einen eigenen Proxy-Server; insert fuer Mehrfach-Module
		//void insertModuleProxy(ModuleIter mdd, ModuleEntry const& entry);
	private:
		/// Publisher warten mit der Semaphore auf einen Subscriber
		struct WaitingPub {
			WaitingPub(PublicationIter p, SmpSemaphore	&s) : pub(p), sem(s) {}
			PublicationIter	pub;
			SmpSemaphore		sem;
		};
		typedef std::list<WaitingPub>						WaitingPubList;
		typedef WaitingPubList::iterator 				WaitingPubIter;
		typedef WaitingPubList::const_iterator	WaitingPubCIter;
	private:
		/// mmAccessors Zweck ist der Zugangsschutz zu den gemeinsamen Daten: dies ist die Mutex dazu
		mutable Poco::FastMutex	commonMutex_;
		/// die Liste der Modul-Ids der registrierten Module
		ModuleList 				moduleList_;
		/// zu jedem Modul gibts einen eigenen Registrar-Server (private Verbindung)
		//Registrars				registrars_;
		/// Anzahl registrierter Module
		int								numModules_;
		/// Liste aller zum Veroeffentlichen registrierten EventLists : diese Events werden gesendet
		PublicationList		catalogue_;
		/// Liste aller registrierten EventInterfaces : diese Events koennen konfiguriert werden
		SubscriptionList	subscriptions_;
		/// Liste aller abonnierten EventInterfaces : diese Events werden vermittelt
		PairedList				activationPairs_; // n*m
		/// hier werden alle Message-Subscriber ohne Publicher mit ihrer interfaceId eingestellt
		//std::map<int, SmpSemaphore> waitingPublishers_;
		WaitingPubList waitingPublishers_;
		/// hiervon werden automatisch neue Protokolle generiert
		SmpProtocolInfo		baseProtocols_[NumActiveProtocols];
		int				 			protocolCursor_;
	};


} // namespace interface
} // namespace precitec

#endif /*MMACCEPTOR_H_*/
