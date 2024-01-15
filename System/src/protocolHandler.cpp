#include "server/protocolHandler.h"
#include "server/eventHandler.h"
#include "message/eventSignaler.h" // wg ShutDown-Event
#include "message/messageSender.h"
#include "module/interfaces.h"
#include "SystemManifest.h"
#include "module/moduleLogger.h"

#include <algorithm>
#include <sys/prctl.h>

namespace precitec
{
using system::message::ShutdownMessage;
using system::message::EventSignaler;
	using system::message::Protocol;
	using system::message::ProtocolInfo;
	using system::message::ProtocolType;
	using system::message::TimeoutMessage;
	using system::message::Sender;

namespace interface
{
	/**
	 * @param info Rererenz auf info_ im EventHandler (Zugriff auf die Daten-des EventTyps)
	 * @param handler Protocolhandler verwaltet die Puffer der Handler deserialiseirt, ruft server
	 */
	ProtocolHandler::ProtocolHandler(MessageInfo const& info, Server<EventHandler> *handler, ProtocolType type)
	:	info_(info), eHandler_(handler),
		systemRunning_(false),
		messageLoopActive_(0, 1),	// Wert 0 (Loop laeuft nicht), Wertebereich [0, 1]
		type_(type)
	{}

	ProtocolHandler::~ProtocolHandler() {
		// ???? stop & wait waren mal weggenommen -> unit-test haengen
		stop();
		waitForServerEnd();
	}

	void ProtocolHandler::initThreadStorage(SmpProtocolInfo & pInfo, PvString const& shMemName) {
		EventThreadStorage &ts(threadStorage_); // nur Abkuerzung fuer die naechsten Zeilen

		if (!shMemName.empty()) {
			int totalSharedMemSize((info().sendBufLen + ShMemRingAllocator::headerSize()) * info().numHandlers );
			ts.sharedMem.set(shMemName, SharedMem::StdClient, totalSharedMemSize);
		}

		// das Protocoll wird erzeugt; *pInfo wird dabei (QNX-Protokoll) veraendert
		ts.protocol		= createProtocol(pInfo);
		ts.mainThread = this->eHandler_; // hierueber greift der thread auf die Handlerfunktion zu
		ts.pHandler		= this; // hierueber greift der thread auf gemeinsame Variable zu
		ts.pid = Poco::Thread::TID();
	}


	void ProtocolHandler::messageLoop(void * data) {
	    // mit diesem Pointer greift der Code auf die ServerKlasse im anderen Thread zu
		// dies ist legal, da alle Werte statisch sind
		// es sollten also keine Synchronisationsprobleme auftauchen
		ProtocolHandler &remoteHandler = *(ProtocolHandler*)data;
		EventThreadStorage &ts(remoteHandler.threadStorage_);

        if (remoteHandler.m_priority != system::Priority::None)
        {
            system::makeThreadRealTime(remoteHandler.m_priority);
        }

		// Werte cachen wg Performance??? Ref sollte reichen, oder gabs da mal Probleme?
		MessageInfo const msgInfo(remoteHandler.info_);

        std::string threadName{"L"};
        threadName += msgInfo.interfaceName();
        prctl(PR_SET_NAME, threadName.c_str());


		Protocol &protocol(*ts.protocol);
		Receiver *pReceiver = NULL;
		if (protocol.protocolType()==Qnx) {
#if defined __QNX__ || defined __linux__

			// Puffer ist wg Pulse in einem XFer-SharedMem (erzeugt von MM) untergebracht
			pReceiver 	= new Receiver(remoteHandler.interfaceName(),msgInfo.sendBufLen, ts.protocol, ts.sharedMem);
#endif
		} else {
			// normaler Ram-Puffer
			pReceiver 	= new Receiver(msgInfo.sendBufLen, msgInfo.replyBufLen, ts.protocol);
		}

		remoteHandler.messageLoopActive_.set(); // wir "melden" den Thread aktiv

		Receiver &receiver(*pReceiver);
		while(!remoteHandler.isStopped())	{
			// lass den Receiver ueber das Protocol auf die erste Nachricht warten
			try {
				// wg Qnx-Pulse sind Events auf << 255 beschraenkt, das casten erhaelt negative Messages
				const int msgNum = std::min(std::max(receiver.getEventMessage(), std::numeric_limits<int>::min()), 255);
				if(remoteHandler.isStopped())
				{
					break;
				}

				if ( (msgNum>=0) && (msgNum<msgInfo.numMessages) )
				{
					// die virtuelle Funktion handle wird on der Abgleiteten Klasse aufgerufen und startet das richtige Callback
					ts.mainThread->handle(receiver, msgNum);
					receiver.messageHandled(); // ggf. Puffer wieder freigeben
				} else
				{
					if (msgNum==ShutdownMessage)
					{
						// regulaeres Shutdown
						remoteHandler.systemRunning(false);	// ab jetzt stoppen die Threads wenn sie nicht blockiert sind
						ts.systemRunning = false;

						receiver.messageHandled(); // ggf. Puffer wieder freigeben
						break; // wir gehen freiwillig raus
					} else if (msgNum==TimeoutMessage) { // auf Timeout reagieren wir gar nicht
						receiver.messageHandled(); // ggf. Puffer wieder freigeben
						//lastEventWasTimeout = true;
					} else { // unbekannte Message, wird ignoriert
						receiver.messageHandled(); // ggf. Puffer wieder freigeben
						wmLog(eWarning, "Procotol handler %s received unexpected Message %d!\n", msgInfo.name().c_str(), msgNum);
					}
				} // if

			} catch (Poco::Exception &e) {
				receiver.messageHandled();
				wmLog(eError, "ProtocolHandler -- msgLoop: Poco::exception %s, thread %d!\n", e.what(), Poco::Thread::current()->id());
				wmLog(eError, "%s\n", e.message().c_str() );
				break;
			} catch (std::exception &e) {
				receiver.messageHandled();
				wmLog(eError, "ProtocolHandler -- msgLoop: std::exception, thread %d!\n", Poco::Thread::current()->id());
				wmLog(eError, "%s\n", e.what() );
				break;
			} catch (...) {
				receiver.messageHandled();
				wmLog(eError, "ProtocolHandler -- msgLoop: unknown exception in thread %d!\n", Poco::Thread::current()->id() );
				break;
			}
		} // while not stopped

		remoteHandler.messageLoopActive_.wait(); // wir "melden" den Thread inaktiv
		protocol.stop();
		if (pReceiver) delete pReceiver;
	} // MessageLoop

	/**
	 */
	void ProtocolHandler::start() {
		if (!isStopped()) {
			return;
		}
		/// sobald dieses Flag false ist, beenden sich alle Threads
		systemRunning(true);
		threadStorage_.systemRunning = true;

		try {
			messageLoopThread_.start(&messageLoop, this);

		#if defined __QNX__ || defined __linux__
			std::ostringstream threadName;
			threadName << "ProtoMessageLoop_" << messageLoopThread_.tid();
			pthread_setname_np(messageLoopThread_.tid(),threadName.str().c_str());
		#endif

			// wir warten, bis Loop laeuft
			messageLoopActive_.wait();
			messageLoopActive_.set();
		} catch (Poco::Exception &e) {
			wmLog(eError, "ProtocolHandler::start -- caught exception in Interface %s.\n ", info().name().c_str() );
			wmLog(eError, " Exception: %s\n", e.displayText().c_str() );
		} catch (...) {
			wmLog(eError, "ProtocolHandler::start failed!\n");
		}
	}


	/// beendet den Message-Loop, wartet ggf. 100 ms darauf
	void ProtocolHandler::stop() {
		systemRunning(false);
		threadStorage_.systemRunning = false;
	}

	void ProtocolHandler::waitForServerEnd() {
		// wir warten auf das Ende aller Threads
		// die Reihenfolge ist egal, da sie alle zuende sein muessen
		try {
			messageLoopThread_.join(50); // Milliseconds
		} catch (...) {
			//std::cout << "EventHandler<"<< info().name()<< ">::waitForServerEnd join timeout" << std::endl;
		}
	}


} // namespace interface
} // namespace precitec

