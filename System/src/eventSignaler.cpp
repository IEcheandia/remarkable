/**
 * eventSignaler.cpp
 *
 *  Created on: 11.05.2010
 *      Author: admin
 */
#include "message/eventSignaler.h"
#include "server/interface.h"
#include "protocol/process.info.h"
#include "module/interfaces.h"
#include "message/derivedMessageBuffer.h"
#include "module/moduleLogger.h"

namespace precitec
{
namespace system
{
namespace message
{
	/**
	 * kill buffers,
	 * delete lists??? do we need to do this? I don't think so
	 */
	EventSignaler::~EventSignaler() {
		for(int i=0; i<NumActiveProtocols; i++)	{
			if (sendBuffer_[i]) delete sendBuffer_[i];
		}
		allSubscribers_.clear();
		subscriberLists_.clear();
	}

	/**
	 * cTor is now obsolete
	 * CTor initialisiert nur die Puffer
	 * Alle Listen sind erstmal (als Listen) leer.
	 * @param numEvents Anzahl der Events des Interfaces
	 * @param sendSize Puffergroesse des Sendpuffers
	 */
	EventSignaler::EventSignaler(int numEvents, int maxMessageSize, int numBuffers)
	: subscriberLists_(numEvents),
	  allSubscribers_(),
		sharedMem_(),
		sendBufferSize_((maxMessageSize+ShMemRingAllocator::headerSize())*numBuffers),
		maxMessageSize_(maxMessageSize)	{
		//std::cout << "EventSignaler cTor: ( " << maxMessageSize << ", " << numBuffers << ")" << std::endl;
		for(int i=0; i<NumActiveProtocols; i++) {	sendBuffer_[i] = NULL; }
	}

	/**
	 * new cTor, MessageInfo contains all necessary information plus debugInfo
	 * maxMessageSize is now obsolete
	 */
	EventSignaler::EventSignaler(MessageInfo const& info)
	: subscriberLists_(info.numMessages),
	  allSubscribers_(),
		sharedMem_(),
		sendBufferSize_((info.sendBufLen+ShMemRingAllocator::headerSize())*info.numHandlers),
		maxMessageSize_(info.sendBufLen), info_(info) {
		//std::cout << "EventSignaler cTor: ( " << maxMessageSize << ", " << numBuffers << ")" << std::endl;
		for(int i=0; i<NumActiveProtocols; i++) {	sendBuffer_[i] = NULL; }
	}

	 	/**
		 * Shortcut-Implementierung fuer Events mit nur einem Subscriber
		 * Das erste was der Proxy aufruft bevor er die Argumente marshallt
		 * Der/die MessagePuffer werden initialisiert
		 * @param eventNum identifiziert das Event -> Memberfunktion
		 * @param serverName fuer Debug-Ausgabe Name des Moduls
		 * @param messageName fuer Debug-Ausgabe Sigantur der Memberfunktion zum Event
		 */
		void EventSignaler::initSingleEvent(int eventNum, Protocol &subscriber)	{

			MessageBuffer &currBuffer(sendBuffer(subscriber.protocolType()));
			// aus Grossem Puffer fuer N-Messages neuen einzelnen Puffer holen
			currBuffer.reset(maxMessageSize_); // sendBufferSize_
			// Puffer loeschen ??? ist das noch noetig?
			currBuffer.clear();
			// Header initialisieren
			currBuffer.setMessageNum(eventNum);
		}
		/// fuer internen Gebrauch etwa umm ShutdownMsg zu verschicken
		void EventSignaler::sendEventToProtocol(int eventNum, Protocol &protocol) {
			module::Interfaces interfaceId = module::Interfaces(info_.interfaceId);
			//std::cout << "EventSignaler::sendEventToProtocol(" << eventNum << ")" << std::endl;
			initSingleEvent(eventNum, protocol);
			ProtocolType currPColType = protocol.protocolType();
			singleBuffer_ = sendBuffer_[currPColType];
			sendBuffer(currPColType).setMsgSize();
			protocol.sendPulse(*singleBuffer_, interfaceId);
		}

	/**
	 * Das erste was der Proxy aufruft bevor er die Argumente marshalt
	 * Der/die MessagePuffer wreden initialisiert
	 */
	void EventSignaler::initMessage(int eventNum)	{

		currList_ = &subscriberLists_[eventNum];
		SubscriberIter subscriber = currList_->begin();

		// fuer einen einzelnen Subscriber machen wir etwas weniger Aufwand
		if (currList_->size()==1) {
			initSingleEvent(eventNum, **subscriber);
			// fuer schnelles Weierverarbeiten richtigen Einzel-Puffer zwischenspeicern
			singleBuffer_ = sendBuffer_[(*subscriber)->protocolType()];
			return;
		}

		// mehrere Subscriber, aber manche Verbindungen werden den gleichen Puffer verwenden koennen
		// (gleiches Protokoll); der soll natuerlich nur einmal angelegt werden.
		for (; subscriber!=currList_->end(); ++subscriber)	{
			ProtocolType currPColType = (*subscriber)->protocolType();
			bufferProcessed_[currPColType] = false;
		}
		// die bufferProcessed-Optimierung ist hier nicht sinnvoll, da pro Subscriber wenig getan wird
		// wir gehen durch die Subscriberliste
		for (subscriber = currList_->begin(); subscriber!=currList_->end(); ++subscriber) {
			// fuer jeden Subscriber
			ProtocolType currPColType = (*subscriber)->protocolType();
			if (!bufferProcessed_[currPColType]) {
				initSingleEvent(eventNum, **subscriber);
				bufferProcessed_[currPColType] = true;
			}
		}
	}


	/// TODO Achtung!!! Observer ist objekt nicht subjekt also send-Member zu notify-Nichtmember-Funktion machen
	void EventSignaler::send() {
		module::Interfaces interfaceId = module::Interfaces(info_.interfaceId);

		// da binaer geschrieben wird ist der Header ok und muss nicht entpackt werden
		SubscriberIter subscriber = currList_->begin();
		for (; subscriber!=currList_->end(); ++subscriber) {
			if ((interfaceId==module::InspectionCmd)||(interfaceId==module::InspectionOut)) {
				//std::stringstream oSt; oSt << "EventSignaler::send() sending to observer " << ++i << " "  << " to " << (*subscriber)->protocolInfo().destination() << std::endl;
				//wmLog( eDebug, oSt.str() );
			}
			ProtocolType currPColType = (*subscriber)->protocolType();
			try {
		 		// da binaer geschrieben wird, ist der Header ok und muss nicht entpackt werden
				sendBuffer(currPColType).setMsgSize();
				(*subscriber)->sendPulse(sendBuffer(currPColType), interfaceId);

			} catch (...) {
				// Hier landen wir, wenn die Komunikation fehlgeschlagen ist
				// dies passiert, wenn
				//  - Netzwerk-Stecker gezogen werden
				//  - manchmal, wenn das System herunterfaehrt (bisher im Wesentlichen unter Windows?!?)
				// \todo Module sollte diese Exception abhandlen (unter QNX zumindest), aber wie??
				// Loesung: Resend N mal mit SysErrorLog, dann rethrow
				std::stringstream oSt; oSt << "eventSignaler: sendPulse failed: " << sendBuffer(currPColType) << " - " << currPColType << std::endl;
				wmLog( eError, oSt.str() );
				wmLog(eDebug, "interfaceId: %d, msgNum: %d\n", interfaceId, sendBuffer(currPColType).messageNum());
			}
		}
	}

	/**
	 * Neuer Observer = neues Sender-Protokoll wird fuer jedes Event in eine eigene
	 */
	void EventSignaler::addSubscriber( std::string p_oName, SmpProtocolInfo & protInfo) {
        std::lock_guard lock{m_sendMutex};
		//std::cout << "EventSignaler<" << info_.name() << ">::addSubscriber("  << *protInfo << ") allSubs, num reexisting subscribers: " << allSubscribers_.size() << std::endl;
		// protinfo muss fuer Client/Sender legal sein
		if (!protInfo->isValid()) {
			wmLog( eError, "Receiver setProtocol is invalid!\n" );
			throw MessageException("eventSignaler: adSubscriber failed, Protocol is invalid");
		}

		try {
			// ein Protokoll kann bereits angemeldet worden sein, also erst mal suchen
			ProtocolType currPColType = protInfo->type();
			SubscriberIter subIter(findSubscriber(currPColType));

			if (subIter==allSubscribers_.end()) {
				// neuer subscriber
//				if ((info_.interfaceId==module::InspectionCmd)||(info_.interfaceId==module::InspectionOut)) {
//					std::cout << "addSubscriber (new Protocol->new Buffer)" << " - " << currPColType << std::endl;
//				}
				if (currPColType==Qnx) {
#if defined __QNX__ || defined __linux__
					if (sendBuffer_[currPColType] == NULL) {
						// Events mit Qnx-Protokoll verwenden Pulse mit Daten  die in ein ShMem serialisiert werden
						// dyn_cast ist in diesem Fall immer ok
						ProcessInfo &pInfo = *dynamic_cast<ProcessInfo*>(&*protInfo);
						int totalSharedMemSize(sendBufferSize_);
						//std::cout << "EventSignaler::addSubscriber: totalSharedMemSize " << totalSharedMemSize << " to " << pInfo.destination() << std::endl;
						// der MM hat das fuer diese Kommunikation wesentliche ShMem als Server geoeffnet
						sharedMem_.set(pInfo.shMemName(), SharedMem::StdClient, totalSharedMemSize);
						// mit dem ShMem wird nun der Ring-Message-Puffer initialisiert
						sendBuffer_[currPColType] =	new SharedMessageBuff(p_oName,sharedMem_, maxMessageSize_);
						//std::cout << "EventSignaler::addSubscriber: shMem " << sharedMem_ << " to " << pInfo.destination() << std::endl;
					}
#endif // __QNX__||__linux__
				} else {
					if (sendBuffer_[currPColType] == NULL) {
						// der einfache Fall mit statischem Puffer
						//std::cout << "addSubscriber (new Buffer) allocating: " << sendBufferSize_ << " for " << *protInfo << std::endl;
						sendBuffer_[currPColType] = new StaticMessageBuffer(sendBufferSize_);
					}
				}
			}

			SubscriberIter subIter1(findSubscriber(protInfo));
			if (subIter1==allSubscribers_.end()) {
				// nur wenn sie noch nicht gelistet ist, wird eine neues Protokoll angelegt
				//std::cout << "addSubscriber (new Subscriber)" << " - " << *protInfo <<  std::endl;
				SmpSubscriber subscriber(createProtocol(protInfo)); // wirft bei Speicherknappheit!!!
				//std::cout << "addSubscriber::created protocol" << std::endl;
				subscriber->initSender();
				// alle Subscriber stehen in dieser Liste, dies erlaubt ein schnelles Aufraeuen im DTor
				allSubscribers_.push_front(subscriber);
				subIter = allSubscribers_.begin();
				//std::cout << "addSubscriber ok0" << std::endl;
			}
			if (currPColType==Qnx) {
				// immer wird die Anzahl der Subscriber pro Puffer/Protokoll mitgezaehlt
				// dies gilt nur fuer SharedMemBuff (Lock-Verwaltung)
				sendBuffer_[Qnx]->addMultiUse();
				if ((info_.interfaceId==module::InspectionCmd)||(info_.interfaceId==module::InspectionOut)) {
					//std::cout << "EventSignaler::addSubscriber: multiUse: " << sendBuffer_[currPColType]->multiUse() << std::endl;
				}
				//std::cout << "addSubscriber ok1" << std::endl;
			}
		} catch (std::exception const& e) {
			// Speichermangel bei Protokoll-Allokation -> rethrow() - oder ????
			wmLog( eError, "addSubscriber caught exception %s\n", e.what() );
			throw;
		}

	}

	/**
	 * Neuer Observer = neues Sender-Protokoll wird fuer jedes Event in eine eigene
	 * Sende-Liste eingetragen
	 * @param eventNum Nummer des Events
	 * @param protInfo Das Protokoll wird (lazy) erst hier generiert und initialisiert.
	 */
	void EventSignaler::addSubscriber(int eventNum, SmpProtocolInfo & protInfo) {
        std::lock_guard lock{m_sendMutex};
		//std::cout << "addSubscriber " << eventNum << " - " << protInfo->type() << std::endl;
		// protinfo ist Client/Sender legal, in addSubscriber(P) bereits abgefangen

		SubscriberIter subIter(findSubscriber(protInfo->type()));
		// subIter zeigt nun auf jeden Fall auf gueltiges Protokoll
		// Doppeltaneldungen sollen nichts bewirken (die Listen nicht durch doppeleintraege verhundsen)
		// also schauen wir nach, ob observer schon eingtragen ist
		// die Liste vergleicht direkt die Smps
		SubscriberIter sub(findSubscriber(eventNum, *subIter));
		if (sub==subscriberLists_[eventNum].end()) {
			// nicht gefunden -> neuer Subscriber fuer dieses Event
			//std::cout << "addSubscriber new: "  <<  std::endl;
			// fuer dieses Signal wird der Subscriber eingegtragen
			subscriberLists_[eventNum].push_front(*subIter);
		}
		//std::cout << "addSubscriber("<< eventNum << ") ok" << std::endl;
	}

	void EventSignaler::removeSubscriber(int eventNum, SmpProtocolInfo const& protInfo) {
        std::lock_guard lock{m_sendMutex};
		/// protinfo muss fuer Client/Sender legal sein
		//assert(protInfo->isValid());
		if (!protInfo->isValid()) {
			wmLog( eError, "Receiver setProtocol is invalid.\n" );
			throw;
		}

		//std::cout << "addObserver " << msgNum << " - " << int(observer->type) << std::endl;
		/// ein Protokoll muss bereist angemeldet worden sein
		SubscriberIter subscriber(findSubscriber(protInfo));
		if (subscriber!=allSubscribers_.end()) {
			// subscriber zeigt nun auf jeden Fall auf gueltiges Protokoll
			SubscriberIter sub(findSubscriber(eventNum, *subscriber));
			if (sub!=subscriberLists_[eventNum].end()) {
				subscriberLists_[eventNum].erase(sub);
				(*subscriber)->stop();
			}
		}
		ProtocolType currPColType = protInfo->type();
		if (currPColType==Qnx) {
			// immer wird die Anzahl der Subscriber pro Puffer/Protokoll mitgezaehlt
			// dies gilt nur fuer SharedMemBuff (Lock-Verwaltung)
			sendBuffer_[Qnx]->removeMultiUse();
			//std::cout << "EventSignaler::removeSubscriber: multiUse: " << sendBuffer_[currPColType]->multiUse() << std::endl;
		}

		// falls wir nix finden, sind wir mal nicht so und tun nix, sonst waere ein throw angebracht
	}

	/**
	 * findet Subscriber in Subscriberliste fuer Event
	 * @param eventNum
	 * @param observer Protokoll fuer Verbindung mit Subscriber
	 * @return
	 */
	EventSignaler::SubscriberIter EventSignaler::findSubscriber(int eventNum, SmpSubscriber const& subscriber)	{
		SubscriberIter sub = subscriberLists_[eventNum].begin();
		for (/*int i=0*/; sub!=subscriberLists_[eventNum].end(); ++sub) {
			//std::cout << "findSubscriber(" <<eventNum<< ") end" << ++i << std::endl;
			if (*sub == subscriber) break;
		}
		//std::cout << "findSubscriber(" <<eventNum<< ") end" << std::endl;
		return sub;
	}

	/**
	 * In allSubscriber werden alle erzeugten Protocolle eingetragen
	 * Unterschieden werden sie anhand ihrer ProtocolInfo-Struktur
	 * @param protInfo Die gesuchte Protokollinfo
	 * @return Iterator auf gefundenen SmpProtocol (ggf allSubscriber.end())
	 */
	EventSignaler::SubscriberIter EventSignaler::findSubscriber(SmpProtocolInfo const& protInfo)	{
		SubscriberList::iterator sub = allSubscribers_.begin();
		for (; sub!=allSubscribers_.end(); ++sub) {
			if ( (*sub)->protocolInfo() == *protInfo) break;
		}
		return sub;
	}

	EventSignaler::SubscriberIter EventSignaler::findSubscriber(ProtocolType  protType)	{
	SubscriberList::iterator sub = allSubscribers_.begin();
	for (; sub!=allSubscribers_.end(); ++sub) {
		if ( (*sub)->protocolInfo().type() == protType) break;
	}
	return sub;
}


} // namespace message
} // namespace system
} // namespace precitec
