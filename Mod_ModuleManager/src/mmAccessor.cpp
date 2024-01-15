#include  "mmAccessor.h"
#include  "moduleEntry.h"
#include  "paired.h"
#include  "publication.h"
#include  "subscription.h"
#include  "message/module.proxy.h"
#include  "module/interfaces.h"
#include  "registrar.server.h"
#include  "message/registrar.handler.h"

#include  "protocol/protocol.info.h"
#include  "protocol/process.info.h"

//#include  "poco/Mutex.h"
//#include  "poco/ScopedLock.h"
//#include  "message/module.h"
//#include 	"registrar.server.h"
//#include 	"message/registrar.handler.h"


namespace precitec
{
namespace interface
{
	using system::module::Interfaces;
	using system::module::AllEvents;
	using system::module::isMessageInterface;

	/**
	 * Das Paar wird erzeugt, und gleich aktiviert
	 * Achtung, da wirmit dummen Pointern arbeiten, duerfen die Elemente nicht
	 * in irgend einen Std-Container gelegt werdne, da hier CToren und DToren wie wild
	 * aufgerufen werden.  Wenn std-Container, dann ShaedPtr.
	 *//*
	RegHandlerPair::RegHandlerPair(MMAccessor &ca, SmpProtocolInfo & protInfo, int mHandle)
	: server(new TRegistrar<MsgServer>(ca)),
		handler(new TRegistrar<MsgHandler>(server)),
		moduleHandle(mHandle) {
		// RAII (oder?)
		handler->activate(protInfo);
	}

	RegHandlerPair::~RegHandlerPair() {
		// erst der Handler, da er vom Server abhaengt
		if (handler) delete handler;
		if (server) delete server;
	}
	*/


	MMAccessor::~MMAccessor() {
		// sollte automatisch geschehen
		//for (int i=0;i<numModules_; ++i) { delete registrars_[i]; }
	}

	MMAccessor::MMAccessor()
	: 	moduleList_(0), numModules_(0),
			catalogue_(0), subscriptions_(0), activationPairs_(), waitingPublishers_(),
			protocolCursor_(0) {
		//std::cout << "MMAccessor CTor: " << std::endl;
		// der Registrar generiert Protokolle, die sich aus diesen 'Basisprotokollen' ableiten
		baseProtocols_[Udp] = createBaseProtocolInfo<Udp>();
		baseProtocols_[Qnx] = createBaseProtocolInfo<Qnx>();
		baseProtocols_[Tcp] = SmpProtocolInfo();
		baseProtocols_[NullPCol] = createBaseProtocolInfo<NullPCol>();
	}

	/**
	 * fuegt ein neues Modul in die Modulliste
	 * Beide Verbindungen (Server/Proxy) werden fuer das Modul angelegt
	 * \return Iterator auf das neue Element
	 */
	void MMAccessor::addModule(ModuleSpec spec, int moduleHandle, ModuleType typ,
									 SmpProtocolInfo & proxyPInfo, SmpProtocolInfo & handlerPInfo) {
		//std::cout << "MMAccessor::addModule: " << moduleList_.size() << std::endl;
		Poco::ScopedLock<Poco::FastMutex> lock(commonMutex_); // Accessor sperren

		ModuleIter mod = findModule(moduleHandle);
		if (mod==moduleList_.end() ) {
			// der Normalfall (genau ein Modul mit einer appId)
			//std::cout << "module is new: creating entry with proxy" << std::endl;
			int	extension = 0;
			moduleList_.push_back(ModuleEntry(spec, ModuleInfo(moduleHandle, extension, typ),
																				*this, proxyPInfo, handlerPInfo) );
 		}	else {
			// n-tes Modul mit gleichem Namen (Spezialfall fuer spaeter)
			// wenn ein Modul mehrfach geladen wird, unterscheiden sich die
			//   Instanzen durch ihre extension.
			// Diese Implementierung geht davon aus, dass die Extensions mit _fallender_ Extensionnummer
			//   einsortiert werden,
			//				Die letzte Extension (mit der hoechsten Nummer) findet sich wie oben (als erstes),
			//				Die erste ist <extension> Elemente dahinter.
			int	extension = 1 + mod->extension();
			//std::cout<< "module is already known: creating entry with extention: " << extension << " with proxy" << std::endl;
			// Element _vor_ gefundenem einsetzen, dann passt die Sortierung
			moduleList_.insert(mod, ModuleEntry(spec, ModuleInfo(moduleHandle, extension, typ),
																					*this, proxyPInfo, handlerPInfo) );
		}
		//std::cout<< "MMAccessor::addModule ok" << std::endl;
	}

	/// exit eines Moduls: Modul aus
	void MMAccessor::removeModule(int moduleHandle) {
		ModuleIter mod = findModule(moduleHandle);
		if (mod!=moduleList_.end() ) {
			moduleList_.erase(mod);
		} else {
			//std::cout<< "MMAccessor::removeModule failed to find " << moduleHandle << std::endl;
		}
	}

	/**
	 * findet Modul in ModuleListe_
	 * \param eindeutiige ModulHandle (=ProzessId oder ThreadId)
	 * \return Iterator auf Modul, ggf EndIterator
	 */
	ModuleCIter MMAccessor::findModule(int moduleHandle) const {
		for (ModuleCIter mod = moduleList_.begin(); mod!=moduleList_.end();	++mod) {
			// //std::cout<< "findModule(" << moduleHandle << "): mod->handle(): " << mod->handle() << std::endl;

			if (mod->handle()==moduleHandle) return mod;
		}
		return moduleList_.end();
	}

	ModuleIter MMAccessor::findModule(int moduleHandle) {
		for (ModuleIter mod = moduleList_.begin(); mod!=moduleList_.end();	++mod) {
			// //std::cout<< "findModule(" << moduleHandle << "): mod->handle(): " << mod->handle() << std::endl;

			if (mod->handle()==moduleHandle) return mod;
		}
		return moduleList_.end();
	}
	/**
	 * gibt alle Module in ModuleListe aus, mit allen Interfaces
	 */
	void MMAccessor::listModules(std::ostream &os) {
		ModuleIter mod;
		for (mod = moduleList_.begin(); mod!=moduleList_.end(); ++mod) {
			// we use the handle (PID) to identify subs/pubs belonging to mod
			int moduleHandle(mod->handle());
			os << "\t" << ModuleName[mod->moduleId()] << std::endl;
			SubscriptionCIter sub;
			bool firstInterface = true;
			for (sub=subscriptions_.begin(); sub!=subscriptions_.end(); ++sub) {
				if (firstInterface) { os << "\t + "; firstInterface=false; }
				if (sub->handle()==moduleHandle) {
					os << /*<<*/ (isEventInterface(sub->interfaceId()) ? "E-" : "M-") << InterfaceName[sub->interfaceId()] << "   ";
				}
			}
			// neue Zeile nur wenn server gelistet wurden
			if (!firstInterface) os << std::endl;
			PublicationCIter pub;
			firstInterface = true;
			for (pub=catalogue_.begin(); pub!=catalogue_.end(); ++pub) {
				if (firstInterface) { os << "\t - ";  firstInterface=false; }
				if (pub->handle()==moduleHandle) {
					os << (isEventInterface(pub->interfaceId()) ? "E-" : "M-") << InterfaceName[pub->interfaceId()] << "   ";
				}
			}
			// neue Zeile wenn clients gelistet wurden
			if (!firstInterface) os << std::endl;
		}
	}



	/**
	 * gibt alle Publications in catalogue_aus
	 */
	void MMAccessor::listCatalogue(std::ostream &os) const {
		PublicationCIter pub;
		for (pub=catalogue_.begin(); pub!=catalogue_.end(); ++pub) {
			os << "\t" << *pub << " ";
			PairedCIter paired = findPaired(pub);
			if (paired!=activationPairs_.end()) {
				os << "<-> Sub-Mod: " << ModuleName[paired->sub()->appId()];
			}
			os << std::endl;
		}
	}

	/**
	 * gibt alle Subscriptoions in SubscriptionListe aus
	 */
	void MMAccessor::listSubscriptions(std::ostream &os) const {
		SubscriptionCIter sub;
		for (sub=subscriptions_.begin(); sub!=subscriptions_.end(); ++sub) {
			os << "\t" << *sub << std::endl;
		}
	}

	/// alle ActivationPairs auflisten
	void MMAccessor::listActivations(std::ostream &os) const {
		PairedCIter pair;
		for (pair=activationPairs_.begin(); pair!=activationPairs_.end(); ++pair) {
			Publication &pub(*pair->pub());
			Subscription &sub(*pair->sub());

			os << "\t<" << (isEventInterface(sub.interfaceId()) ? "E-" : "M-") << module::InterfaceName[sub.interfaceId()]  << "> ";
			os << module::ModuleName[findModule(pub.handle())->moduleId()] << " --> ";
			os << module::ModuleName[sub.appId()];
			if (isEventInterface(sub.interfaceId())) {
				ProcessInfo &pi = *dynamic_cast<ProcessInfo*>(&*sub.protocolInfo());
				os << "  @" << pi.shMemName();
			}
		  os << std::endl;
		}
	}

	/// alle verbindbaren Publisher auflisten
	void MMAccessor::listWaiting(std::ostream &os) const {
		WaitingPubCIter pubs;
		for (pubs=waitingPublishers_.begin(); pubs!=waitingPublishers_.end(); ++pubs) {
			Publication &pub(*pubs->pub);

			os << "\t" << module::ModuleName[findModule(pub.handle())->moduleId()] << " <";
			os << 				module::InterfaceName[pub.interfaceId()] << ">" << std::endl;
			//os << "\t" << *pair << std::endl;
		}
	}
	/**
	 * hiermit kann ein externes Modul den MM stoppen
	 */
	void MMAccessor::kill() {
		// ????? alle wartenden Publisher freigeben
		//std::map<int, SmpSemaphore>::iterator pubs = waitingPublishers_.begin();
		//while (pubs!= waitingPublishers_.end()) { pubs->second->set(); }

		WaitingPubIter pubs = waitingPublishers_.begin();
		while (pubs!= waitingPublishers_.end()) { pubs->sem->set(); }
	} //

/*
	void MMAccessor::addRegistrar(RegHandlerPair *rPair) {
		Poco::ScopedLock<Poco::FastMutex> lock(commonMutex_);
		registrars_[numModules_++] = rPair;
	}
*/
	/**
	 * Remove Registrar ist etwas subtiler als addRegsitrar
	 * Wir wollen keine Luecken hinterlassen, also wird beim Loeschen eines
	 * Eintrags der letzte Eintrag in die Luecke kopiert.
	 * Wesentlich ist, das der zu verschiebende Eintrag nicht deleted wird
	 * -> delete List[i]; list[i]¨= list[N-1]; list[N-1]=empty-Element;
	 *//*
	void MMAccessor::removeRegistrar(int mHandle) {
		Poco::ScopedLock<Poco::FastMutex> lock(commonMutex_);
		int entry;
		for (entry=0; entry<numModules_; ++entry) {
			if (registrars_[numModules_].moduleHandle==mHandle) {
				// server und handle werden gekillt
				delete registrars_[numModules_];
				registrars_[entry] = registrars_[--numModules_];
				registrars_[numModules_] = Registrars;
				return;
			}
		}
		// hier duerfen wir eigentlich nicht hinkommen.
		// wir dulden doppelloeschungen als IdemPotende Funktionen mit Fehlermeldung
		//std::cout<< "Loeschen von Registrar-Eintrag mit moduleHandle: " << mHandle << " Eintrag nicht gefunden!" << std::endl;
	}
		*/


	/**
	 *  Publisher (allg. Modul) gibt sein Interface (alle verwendeten Signale) bekannt
	 */
	PublicationIter MMAccessor::addPublication(int moduleHandle, Interfaces interfaceId,
																							int appId, Publication &publication, int numEvents, int subAppId, const std::string &path) {
		////std::cout<< "TRegistrar<MsgServer>::addPublication " << InterfaceName[interfaceId] << std::endl;
		Poco::ScopedLock<Poco::FastMutex> lock(commonMutex_);
		// jede EventInterfaceListe wird einfach ans Ende angehaengt
		// 	mehrere apps duerfen die gleiche EventInterfaceListe haben

		// find publication in catalogue_
        auto pub = std::find_if(catalogue_.begin(), catalogue_.end(),
            [=] (const auto publication)
            {
                return publication.interfaceId() == interfaceId && publication.handle() == moduleHandle && publication.numEvents() == numEvents && publication.subAppId() == subAppId && publication.path() == path;
            });
		////std::cout<< "post find" << interfaceId << std::endl;
		while (pub!=catalogue_.end())	{
			// App mit Interface gefunden
			if (pub->handle() == moduleHandle) {
				// jede App darf das interface nur einmal publishen,
				// --> Zweit-Anmeldung einfach ignorieren -> Funktion ist idempotent
				////std::cout<< "MMAccessor::addPublication - found existing publisher with interface " << InterfaceName[interfaceId] << ", doing nothing" << std::endl;
				return pub; // es soll auf jeden fall mit dem Richtigen Eintrag gearbeite werden
			}
			// wir suchen in der restichen Liste weiter
			////std::cout<< "adding publisher to pubList" << interfaceId << std::endl;
			pub = std::find(++pub, catalogue_.end(), publication);
		}
		// neue App wird als publisher in die Publicationliste eingetragen
		pub = catalogue_.insert(catalogue_.end(), Publication(interfaceId, moduleHandle,
								numEvents, subAppId, path/*, moduleEntry(moduleHandle)*/));
		return pub; // after pushback, pub is the last element in list
	}

/*
	PublicationIter TRegistrar<MsgServer>::startDefaultPublisher(int interfaceId)
	{
//		PublisherIter pubName = defaultPublishers_.find(interfaceId)->second();
//		moduleList.push_back(ModuleEntry( ,load(pubName));
//		int appId = load(pubName);
//		addPublication(appId, Publication(interfaceId), int numSignals, int signalId[]);
	}
*/
	SubscriptionIter MMAccessor::addSubscription(Subscription &subscription)	{
		std::cout << "TRegistrar<MsgServer>::addSubscription " << InterfaceName[subscription.interfaceId()] << std::endl;
		Poco::ScopedLock<Poco::FastMutex> lock(commonMutex_);
		// jede EventInterfaceListe wird einfach ans Ende angehaengt
		// 	mehrere apps duerfen die gleiche EventInterfaceListe haben
		//std::cout << "Cataloge: "; listCatalogue(std::cout);

		// find subscription in list
		for (SubscriptionIter sub = subscriptions_.begin(); sub!=subscriptions_.end(); ++sub) {
			if ( 		(sub->appId() == subscription.appId())
					 && (sub->interfaceId()==subscription.interfaceId())
                     && (sub->path() == subscription.path())) {
				// bereits abboniert
				std::cout<< "addSubscription: bereits vorhanden  appID: " << sub->appId() << " interface: " << sub->interfaceId()  << std::endl;
				// --> einfach ignorieren -> Funktion ist idempotent
				return sub;
			}

		}
		// nichts gefunden, also einfuegen
		std::cout << "TRegistrar<MsgServer>::addSubscription end" << InterfaceName[subscription.interfaceId()] << std::endl;
		//return subscriptions_.insert(subscriptions_.end(), subscription);
		subscriptions_.push_front(subscription);

//		std::cout << "TRegistrar<MsgServer>::addSubscription " << InterfaceName[subscription.interfaceId()] << " ok " << std::endl;
	//	std::cout << "moduleList: " << std::endl; listModules(std::cout);
//		std::cout << "connList: " << std::endl; listActivations(std::cout);
//		std::cout << "waitList: " << std::endl; listWaiting(std::cout);
//		std::cout << std::endl << std::endl << std::endl;
		return subscriptions_.begin();
	}

	/**
	 *
	 */
	void  MMAccessor::activateMatchingPublications(SubscriptionIter sub)	{
		std::cout << "TRegistrar<MsgServer>::activateMatchingPublications for " << InterfaceName[sub->interfaceId()] << std::endl;
		bool found = false;
		Poco::ScopedLock<Poco::FastMutex> lock(commonMutex_);
		PublicationIter pub = catalogue_.begin();
		// es werden alle! passenden Publisher aktiviert
		for (; pub!=catalogue_.end(); ++pub) {
			//std::cout << "checking " << ModuleName[pub->handle()] << " for subscriber " << InterfaceName[sub->interfaceId()] << std::endl;
			if (match(*pub, *sub)) {
				found = true;
				std::cout << "found publisher: activating for " << InterfaceName[sub->interfaceId()]<< std::endl;
				activate(pub, sub, sub->event());
//				std::cout << "TRegistrar<MsgServer>::activateMatchingPublications " << InterfaceName[sub->interfaceId()] << " ok " << std::endl;
//				std::cout << "moduleList: " << std::endl; listModules(std::cout);
//				std::cout << "connList: " << std::endl; listActivations(std::cout);
//				std::cout << "waitList: " << std::endl; listWaiting(std::cout);
//				std::cout << std::endl << std::endl << std::endl;

				std::cout << " waiting publisher activated" << std::endl;
				//if (isMessageInterface(Interfaces(pub->interfaceId())) ) {
					// nur fuer Message-Clients wird gesucht ob sie haengen und auf einen Server warten
					std::cout << "TRegistrar<MsgServer>::activateMatchingPublications looking for waiting publisher" << std::endl;
					for (WaitingPubIter found(waitingPublishers_.begin()); found!=waitingPublishers_.end();) {
						if (found->pub==pub) {
							found->sem->set(); // publisher  thread wird freigegeben
							std::cout << "freed publisher" << std::endl;
							found = waitingPublishers_.erase(found); // wir loeschen, weil wir den Iterator eh gerade zur Hand haben
						} else {
							found++;
						} // if
					} // for found
				//} // if isMessageInterface
			} // if match
		} // for pub
		std::cout<< "TRegistrar<MsgServer>::activateMatchingPublications " << InterfaceName[sub->interfaceId()] << (found ? " ok " : " no publisher yet") << std::endl;
	}

	/**
	 * called by addPublication
	 * activation of a message-publication must not fail!!, so wait until a fitting sub is ready
	 * \return true if match succeeded, false if interrupted by shutdown
	 */
	bool  MMAccessor::activateMatchingSubscriptions(PublicationIter pub)	{
		//std::cout << "TRegistrar<MsgServer>::activateMatchingSubscriptions for " << InterfaceName[pub->interfaceId()]<< std::endl;
		bool matched = false;

		{ // Block wg ScopedLock NICHT entfernen
			Poco::ScopedLock<Poco::FastMutex> lock(commonMutex_);
			SubscriptionIter sub = subscriptions_.begin();
			// alle! passenden Subscriber werden aktiviert
			for (; sub!=subscriptions_.end(); ++sub) {
				if (match(*pub, *sub)) {
					std::cout << "found existing subscriber " << ModuleName[sub->appId()] << " for " << InterfaceName[pub->interfaceId()] << std::endl;
					activate(pub, sub, sub->event());

//					std::cout << "TRegistrar<MsgServer>::activateMatchingSubscriptions <" << InterfaceName[pub->interfaceId()] << "> ok " << std::endl;
//					std::cout << "moduleList: " << std::endl; listModules(std::cout);
//					std::cout << "connList: " << std::endl; listActivations(std::cout);
//					std::cout << "waitList: " << std::endl; listWaiting(std::cout);
//					std::cout << std::endl << std::endl << std::endl;
					matched = true;
				}
			}
		}
		if (!matched) {
			// is this a message subscription
			if (isMessageInterface( Interfaces(pub->interfaceId()))) {
				std::cout << "TRegistrar<MsgServer>::activateMatchingSubscriptions waiting for subscriber " <<  InterfaceName[pub->interfaceId()] <<  std::endl;
				// da die Semaphore nur existiert, waehrend die routine hier darauf wartet,
				// koennte! man sie auch direkt auf dem Stack anlegen!!!!  Erschreckend, aber es sollte klappen?!?
				// die SharePtr-Implementierung sollte auch Semaphore sauber loeschen, wenn bei Programmende noch welche uebrig sind??!!?????
				// Die subscriber-thread einfach bei Programmende freizugeben ist keine Loesung, da sie sofort crashen wuerden
				// es muss wohl eine Exception fuer diesen Fall her !!!???? -> spaetere Version

				// create a semaphore, put it into a map, and wait for it to be set
				int initValue = 0; // we will go into wait immediately
				int maxValue  = 255; // we currently!!! match on first interface, but that may change
				SmpSemaphore sem(new Poco::Semaphore(initValue, maxValue));
				waitingPublishers_.push_back(WaitingPub(pub, sem));
				std::cout<< " starting to wait for subscriber for " << InterfaceName[pub->interfaceId()] << std::endl;
				sem->wait(); // we block until someone else sets the semaphore
				//std::cout<< "MMAccessor::activateMatchingSubscriptions woke up for " << InterfaceName[pub->interfaceId()] <<std::endl;

//				std::cout << "TRegistrar<MsgServer>::activateMatchingSubscriptions <" << InterfaceName[pub->interfaceId()] << "> ok " << std::endl;
//				std::cout << "moduleList: " << std::endl; listModules(std::cout);
//				std::cout << "connList: " << std::endl; listActivations(std::cout);
//				std::cout << "waitList: " << std::endl; listWaiting(std::cout);
//				std::cout << std::endl << std::endl << std::endl;
				// so hier hat uns jemand befreit, die Semaphore brauchen wir also nicht mehr
				// der andere Thread hat sie aus der Map geloescht (wir brauchen hier also nicht mehr zugreifen)
				// das sem automatische Varaible ist, wird die Semaphore auch automatisch geloescht, sobald sie auch aus der Liste entfernt wurde

				// wenn kein Handler (via activate) gesetzt wurde, sind wir am Aussteigen -> false
				return isPaired(pub);
			}
		}
		// kein Blocking -> true
		std::cout << "TRegistrar<MsgServer>::activateMatchingSubscriptions for " << InterfaceName[pub->interfaceId()] << " ok" << std::endl;
		return true;
	}



	/**
	 * Erste Implementierung eines gefundenen Interfaces wird aktiviert
	 */
	void MMAccessor::activate(PublicationIter pub, SubscriptionIter sub, int eventId)	{
//		std::cout << "TRegistrar<MsgServer>::activate events for: "
//							<< (eventId==AllEvents ? "all Events" : toString(eventId)) << " "
//							<< InterfaceName[sub->interfaceId()] << "  " << *sub << " :: " << *pub << std::endl;
		// wenn der Subscriber noch kein Protokol hat, muss dieses erst bestimmt werden
		// der Publisher braucht dann immer!! ein neues Protokol

		// Beim Publisher/Client ist die Sache verzwickter. Pro Paarung/Subscription hat er ein eigenes
		// Protokol (aus dem Server generiert). Das Client-Protokol ist daher nicht beim
		// Client/Publisher, sondern in der Subscription gespeichert.
		// Wenn der Publisher noch kein Protokol hat, gibt es diese Paarung noch nicht -> neue Subscription
		// Dies ist natuerlich immer der Fall, wen es den Subscriber noch nicht gab


		// der Server wird aus zwei Gruenden vor dem Client gestartet
		// 1. das Client-Protokol haengt je nach Protokol-Typ vom Server-Protokol ab
		// 2. der Client legt mit dem Aktivieren sofort los, braucht also einen laufenden Server


		SmpProtocolInfo clientProtocol;
		if (sub->protocolInfo().isNull()) {
//			if (sub->interfaceId()==InspectionCmd) {
//				std::cout << "MMAccessor::first activation of subscriber: " << std::endl;
//			}
			// ... create new Protocol (with QNX: this is a 2phase process: serverProtocol -> activateServer -> clientProtocol)
			ProtocolType protocolType(findFittingProtocolType(
																			sub->interfaceId(),
																			findModule(pub->handle())->moduleId(),
																			findModule(sub->handle())->moduleId()
																	));
			// der Server brauht nur das Protokoll, er hoert immer auf alle Events/Messages
//			if (sub->interfaceId()==InspectionCmd) {
//				std::cout << "MMAccessor::activate event: generating derived Interface from " << *baseProtocols_[protocolType] << " : " << protocolType << std::endl;
//			}
			sub->setProtocolInfo(baseProtocols_[protocolType]->generateDerived(++protocolCursor_));

//			if (sub->interfaceId()==InspectionCmd) {
//				std::cout << "TRegistrar<MsgServer>::activate activating server: " << *sub->protocolInfo() << std::endl;
//			}

			// fuer QNX-Pulse (Events) ist ein gemeinsames SharedMemory zwischen Publisher/Subscriber noetig
			if ((protocolType == Qnx) && (isEventInterface(sub->interfaceId()) )) {
				ProcessInfo &subpInfo = *reinterpret_cast<ProcessInfo*>(&*sub->protocolInfo());
				// alle QNX-sup-pub-Paare eines Interfaces teilen das gleiche XFer Shared Mem
				// also suchen wir nach allen sub mit gleichem pub und QNX-Protokoll
				PvString shMemName;
				for (PairedIter	paired = activationPairs_.begin(); paired!=activationPairs_.end(); ++paired) {
					if ((paired->pub()==pub) && (paired->sub()->protocolInfo()->type()==Qnx))	 {
						// wir uebernehmen das shMem aus dieser Verbindung
						ProcessInfo &oldpInfo = *reinterpret_cast<ProcessInfo*>(&*paired->sub()->protocolInfo());
						shMemName = oldpInfo.shMemName();
//						if (sub->interfaceId()==InspectionCmd) {
//							std::cout << "TRegistrar<MsgServer>::activate old shMemName: " << shMemName << std::endl;
//						}
						break;
					}
				}
				if (shMemName.empty()) {
					// keine existierende QNX-aktivierung fuer dieses Interface gefunden
					// also neues shMem generieren:
					shMemName = PvString("xFerMem")+std::string(getenv("WM_STATION_NAME"))+std::to_string(protocolCursor_);
//					if (sub->interfaceId()==InspectionCmd) {
//						std::cout << "TRegistrar<MsgServer>::activate new shMemName: " << shMemName << std::endl;
//					}
				}
				subpInfo.setSharedMem(shMemName);
			}
			clientProtocol = findModule(sub->handle())->activateServer(sub->interfaceId(),
																																	sub->protocolInfo(), sub->path());
//			if (sub->interfaceId()==InspectionCmd) {
//				std::cout << "clientProtocol: " << *clientProtocol << std::endl;
//			}

			// das Protokol muss fuer den Client gespeichert werden
			sub->setProtocolInfo(clientProtocol);
//			if (sub->interfaceId()==InspectionCmd) {
//				std::cout << "MMAccessor::activate new publisher: " << *clientProtocol << std::endl;
//			}

			// neue subscription
			activationPairs_.push_front(ActivationPair(pub, sub, clientProtocol));
		} else {
			// std::cout << "MMAccessor::activate existing subscriber: " << std::endl;
			// erst muessen wir die Subscription (so vorhanden) in der Sub.Liste finden
			// finde in Liste der bereits gepaarten Subscriptions/Publications
			bool 				found = false;
			PairedIter	paired;
			for (paired = activationPairs_.begin(); paired!=activationPairs_.end(); ++paired ) {
				if ((paired->sub()==sub) && (paired->pub()==pub))	 {
						found = true; break;
				}
			}
				//std::cout << "MMAccessor::activate subscription " << (found ? "found":"not found") << std::endl;

			// es gibt noch eine kleine Komplikation
			// ... der QNX-Protokoll-Server kann mehrere Verbindungen managen
			// ... ein TCP/UDP-Server baut jeweils Punkt zu Punkt-Verbindungen auf
			//     (fuer TCP gilt das in diesem Protokol-Kontext)
			ProtocolType protocolType;
			if (!found) {
				// ... create new Protocol (with QNX: this is a 2-phase process: serverProtocol -> activateServer -> clientProtocol)
				protocolType = findFittingProtocolType(
														sub->interfaceId(),
														findModule(pub->handle())->moduleId(),
														findModule(sub->handle())->moduleId()
												);
			} else {
				protocolType = paired->clientProtocol()->type();
			}

			//	std::cout << "MMAccessor::activate existing subscriber: " << protocolType << std::endl;
			if (found) {
				//	std::cout << "MMAccessor::activate trying to reActivateServer for existing client" << std::endl;
				// der Client wird auf das jeweilige (oder die jeweiligen) EventId(s) aktiviert
				// der Client/Publisher hat pro Subscription/Protokoll genau einen Server/Subscriber
				// daher gibt es hier nur ein activate (kein reactivate)
//					std::cout << "MMAccessor::reActivate "<< (found?"old":"new") << " publisher on"
//										<< (protocolType==Qnx ? " ":"non ") << "QNX protocol: " << std::endl;
				// der Server wird reAktiviert mit dem alten Client-Protokoll (wird wg Identifizierung mitgeliefert)
				clientProtocol = paired->clientProtocol();
				findModule(sub->handle()) ->reActivateServer(sub->interfaceId(), clientProtocol);
					//std::cout<< "using protocol: " << *clientProtocol << std::endl;
			} else {
				if (protocolType == Qnx) {
					//	std::cout << "MMAccessor::activate trying to reActivateServer for Qnx client" << std::endl;
					// der Client wird auf das jeweilige (oder die jeweiligen) EventId(s) aktiviert
					// der Client/Publisher hat pro Subscription/Protokoll genau einen Server/Subscriber
					// daher gibt es hier nur ein activate (kein reactivate)
//						std::cout << "MMAccessor::reActivate "<< (found?"old":"new") << " publisher on"
//											<< (protocolType==Qnx ? " ":"non ") << "QNX protocol: " << std::endl;
					// der Server wird reAktiviert; unter QNx verwenden mehrere Clients den gleichen Channel (ProtocolInfo)
					findModule(sub->handle()) ->reActivateServer(sub->interfaceId(),
																											 sub->protocolInfo() );
					clientProtocol = sub->protocolInfo();
					//	std::cout << "using protocol: " << *clientProtocol << std::endl;
				} else {
					//	std::cout << "MMAccessor::activate trying to activateServer for new publisher: " << std::endl;
					// der Server wird aktiviert und damit erhalten wir das Client-Protokoll
					clientProtocol = findModule(sub->handle()) ->activateServer(sub->interfaceId(),
																																			sub->protocolInfo(), sub->path()
																																			);
					//	std::cout << "MMAccessor::activate new publisher: " << *clientProtocol << std::endl;
				}
				// neue subscription
				//std::cout << "MMAccessor::activate adding to activation list: " << std::endl;
				activationPairs_.push_front(ActivationPair(pub, sub, clientProtocol));
				paired = activationPairs_.begin();
			}
		}
		/// der Client wird auf das jeweilige (oder die jeweiligen) EventId(s) aktiviert
//		if (sub->interfaceId()==InspectionCmd) {
//			char 	buff[20];
//			std::cout << "MMAccessor::activate: activating client"
//							 << InterfaceName[pub->interfaceId()] << " " << clientProtocol
//							 << " " << (eventId==module::AllEvents? "all events" : PvString("event: ") + itoa(eventId, buff, 10)) << std::endl;
//		}
		findModule(pub->handle())->activateClient(pub->interfaceId(), clientProtocol, eventId, pub->subAppId(), pub->path());

		//	 std::cout << "MMAccessor::activate: ok " << *pub << " :: " << *sub << std::endl;
	}





	/**
	 * Vergleicht Publication und Subscription (z.Zt. nur nach id)
	 */
	bool MMAccessor::match(Publication &pub, Subscription& sub) {
//		std::cout << "match " << InterfaceName[pub.interfaceId()] << " : "
//													<< InterfaceName[sub.interfaceId()]
//													<< (pub.interfaceId() ==	sub.interfaceId() ? " match": " no match ")<< std::endl;
		return (pub.interfaceId() ==	sub.interfaceId() ) &&
                (pub.subAppId() == AnyModule || pub.subAppId() == sub.appId()) &&
                (pub.path() == sub.path());
//					|| (pub.interfaceId()== AllEvents) || (sub.interfaceId()== AllEvents) ;
	}

	PairedCIter	MMAccessor::findPaired(PublicationCIter pub, SubscriptionCIter sub) const {
		PairedCIter	paired;
		for (paired = activationPairs_.begin(); paired!=activationPairs_.end(); ++paired ) {
			if ((paired->sub()==sub) && (paired->pub()==pub))	 {
					break;
			}
		}
		return paired;
	}

	PairedCIter	MMAccessor::findPaired(PublicationCIter pub) const {
		PairedCIter	paired;
		for (paired = activationPairs_.begin(); paired!=activationPairs_.end(); ++paired ) {
			if (paired->pub()==pub) {
					break;
			}
		}
		return paired;
	}

	/**
	 * Search ModuleList for specific app implementing specific interface, return its
	 */
	ModuleEntry& MMAccessor::moduleEntry(int moduleHandle)	{
		ModuleIter mod = moduleList_.begin();
		for (; (mod!= moduleList_.end())
				&& (mod->handle()!=moduleHandle);
					++mod) {}
		return *mod;
	}

	// MM-Interface
	ProtocolType MMAccessor::findFittingProtocolType(int interfaceId, int moduleId0, int moduleId1) {
		//if (interfaceId==module::InspectionCmd)  return Udp;
		//else
			return Qnx;
	}

	TModule<MsgProxy> &MMAccessor::proxyFromHandle(int moduleHandle) {
		for (ModuleIter mod=moduleList_.begin(); mod!= moduleList_.end(); ++mod) {
			if (mod->handle()==moduleHandle) return mod->proxy();
		}
		throw 1/* ???*/;
	}

	/// exit eines Moduls: alle Aktivierungen einer Publication (Interface) deaktivieren
	void MMAccessor::deactivateAll(PublicationIter p) {
		for (PairedIter	paired=activationPairs_.begin(); paired!=activationPairs_.end();) {
			if (paired->pub() == p) {
				paired = activationPairs_.erase(paired);
			} else {
				++paired;
			}
		}
	}

	/// exit eines Moduls: alle Aktivierungen einer Publication (Interface) deaktivieren
	void MMAccessor::deactivateAll(SubscriptionIter s) {
		for (PairedIter	paired=activationPairs_.begin(); paired!=activationPairs_.end();) {
			if (paired->sub() == s) {
				paired = activationPairs_.erase(paired);
			} else {
				++paired;
			}
		}
	}

	/// exit eines Moduls: alle Interfaces einer Publication deaktivieren
	void MMAccessor::unpublishAll(int moduleHandle) {
		// for each publication of module: unpublish all activations
		for (PublicationIter pub = catalogue_.begin(); (pub!= catalogue_.end());) {
			ModuleIter module(findModule(pub->handle()));
			if (module->handle()==moduleHandle) {
				deactivateAll(pub);
				moduleList_.erase(module);
				pub = catalogue_.erase(pub); // mod zeigt nun auf naechstes Element
			} else {
				++pub;	// die regulaere Inkrementierung
			}
		}
	}


	/// exit eines Moduls: alle Interfaces einer Subscription deaktivieren
	void MMAccessor::unsubscribeAll(int moduleHandle) {
		// for each publication of module: unpublish all activations
		for (SubscriptionIter sub=subscriptions_.begin(); sub!=subscriptions_.end(); ) {
			ModuleIter module(findModule(sub->handle()));
			if (module->handle()==moduleHandle) {
				deactivateAll(sub);
				moduleList_.erase(module);
				sub = subscriptions_.erase(sub); // mod zeigt nun auf naechstes Element
			} else {
				++sub;	// die regulaere Inkrementierung
			}
		}
	}


} // namespace interface
} // namespace precitec
