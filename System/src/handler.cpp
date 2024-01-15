#include <iostream>

#include "server/handler.h"
#include "message/messageSender.h"
#include "message/messageReceiver.h"
#include "message/messageException.h"
#include "protocol/protocol.info.h"
#include "system/tools.h"

#include <algorithm>
#include <sys/prctl.h>

namespace precitec
{

	using system::message::Receiver;
	using system::message::Sender;
	using system::message::MessageBase;
	using system::message::SmpProtocol;
	using system::message::SmpProtocolInfo;
	using system::message::ShutdownMessage;
	using system::message::TimeoutMessage;

namespace interface
{

	/**
	 * std-CTor
	 */
	SYSTEM_API Server<MsgHandler>::Server(MessageInfo const& info)
	: info_(info),
		threadStorage_(),
		activeHandlers_(0, 1), // die Semaphore gibt die Zahl einsatzbereiter Threads an
		systemRunning_(false),
		messageLoopActive_(0, 1)
	{
		//std::cout << "Server<MsgHandler>::CTor(p) " << info_ << std::endl;
	}

	/**
	 * CTor fuer Verbindungen mit hartkodiertem Protokoll, etwa die Receptor-Verbindung
	 * Rueckgabewert von activate kann getrost ignoriert werden, da Protocol nicht QNX-Protokoll ist
	 */
	SYSTEM_API Server<MsgHandler>::Server(MessageInfo const& info, SmpProtocolInfo & protInfo)
	: info_(info),
		threadStorage_(),
		activeHandlers_(0, 1), // die Semaphore gibt die Zahl einsatzbereiter Threads an
		systemRunning_(false),
		messageLoopActive_(0, 1)
	{

		activate(protInfo);
		//std::cout << "Server<MsgHandler>::Server() ok" <<  std::endl;
	}

	SmpProtocolInfo SYSTEM_API Server<MsgHandler>::activate(SmpProtocolInfo & protInfo) {
		if (protInfo.isNull()) return protInfo;

		//std::cout << "Server<MsgHandler>::activate()" << (protInfo->isValid() ? "protocol is valid" : "protocol is invalid") <<  std::endl;
		if (!protInfo.isNull()) {
			//std::cout << "Server<MsgHandler>::activate(p): " << *protInfo << std::endl;
			// Achtung *protInfo wird hier beim QNX-Protocol veraendert
			SmpProtocol p = createProtocol(protInfo);
			if (p.isNull()) {
				std::cout << "SmpProtocol nullPointer created:: aborting" <<  std::endl;
				throw;
				return protInfo;
			}
			initThreadStorage(info_.sendBufLen, info_.replyBufLen, p);
			// jetzt sind alle Infos da, umdie Callbacks zu registrieren
			registerCallbacks();
			// sobald das Protokoll eingebundenist, gibt es keinen Grund mehr zu warten
			start();
			//std::cout << "Server<MsgHandler>::activated(p): " << *protInfo << std::endl;
		}
		return protInfo; // mit diesem meist gleichen Protoko wird der Client gestartet
	}

	/// setzt die konstanten Daten fuer die Handler-Threads
	void SYSTEM_API Server<MsgHandler>::initThreadStorage(int sendBufLen, int replyBufLen, SmpProtocol & p ) {
			threadStorage_.mainThread 	 	= this; // hierueber greift der thread auf gemeinsame Variable zu
			threadStorage_.receiver 			= new Receiver(sendBufLen, replyBufLen, p);
	}

	/// startet die Messageloop
	void SYSTEM_API Server<MsgHandler>::start() {
		// callback-Tabelle wird angelegt
		//std::cout << "Server<MsgHandler>::start() " << std::endl;
		// wenn er schon laeuft, muss man nicht stoppen \todo access-counter !!!!
		if (!isStopped()) {
			//std::cout << "Server<MsgHandler>::start(): already running " << std::endl;
			return;
		}
		// sobald dieses Flag false ist, beenden sich alle Threads
		//std::cout << "Server<MsgHandler>::setting system to running" << std::endl;
		systemRunning(true);

		try {
			// erst locken wir alle Message-Mutexe
				//std::cout << "start: locking handler " << i << std::endl;
			threadStorage_.handlerThread.start(&handlerThread, &threadStorage_);
			// hier warten wir, bis der erste Thread bereit ist
			//std::cout << "Server<MsgHandler>::waiting for 1st thread() " << std::endl;
			activeHandlers_.wait();
			// die Semaphore wird sofort wieder freigegeben; damit ist sie als Threadzaehler weiter gueltig
			activeHandlers_.set();

			messageLoopThread_.start(&messageLoop, this);

		#if defined __QNX__ || defined __linux__
			std::ostringstream threadName;
			threadName << "HandMessageLoop_" << messageLoopThread_.tid();
			pthread_setname_np(messageLoopThread_.tid(),threadName.str().c_str());
		#endif

			// wir warten,  bis die Loop laeuft
			//std::cout << "Server<MsgHandler>::waiting for messageloop() " << std::endl;
			messageLoopActive_.wait();
			messageLoopActive_.set();
		}	catch (...)	{
			std::cout << "start msgHandler msgLoopThread failed" << std::endl;
			throw;
		}
		//std::cout << "Server<MsgHandler>::start(): ok" << std::endl;
	}

	/// beendet die Message-Loop, wartet ggf. 100 ms darauf
	void SYSTEM_API Server<MsgHandler>::stop() {
		//std::cout << "MsgHandler::stop" << std::endl;

		if (isStopped()) return; // is schon tot, oder am sterben, falg wird im Handlerthread gesetzt
		systemRunning(false);
		//std::cout << "MsgHandler::stop stopping" << std::endl;
		int buffSize = 100;
		SmpProtocol p(threadStorage_.receiver->protocol());
		Sender sender;
		if (sender.setProtocol(p, buffSize, buffSize)) {
			// nur wen Receiver noch existiert wird Quit-Message gesendet
			// eine einzige Quit-Message reicht,
			// ... da alle anderen Threads nach Aktivierung erst den Status abfragen
			try	{
				sender.sendQuitMessage(); // damit kommen die Handler aus irgendwelhen Wartezustaenden zurueck
			}	catch(...) {
				// niemand mehr zu hause
			}
		}
		p->stop();
		//std::cout << "MsgHandler::stop  sentQuitMessage" << std::endl;
		waitForServerEnd();
	}

	/// die abgeleitete Klasse wartet hiermit auf das Ende des Server und leitet DTor ein
	void SYSTEM_API Server<MsgHandler>::waitForServerEnd() {
		// wir waarten auf das Ende aller Threads
		// die Reihenfolge ist egal, da sie alle zuende sein muessen
		//std::cout << "Server<MsgHandler>::waitForServerEnd " << std::endl;
		// die Loop wartet ihrerseits auf die Handler
		messageLoopThread_.join();
		//std::cout << "Server<MsgHandler>::waitForServerEnd ended" << std::endl;
	}

	void SYSTEM_API Server<MsgHandler>::messageLoop(void * data) {
		//std::cout << "msgLoop: setting up msgLoop" << std::endl;
		// mit diesem Pointer greift der Code auf die ServerKlasse im anderen Thread zu
		// dies ist legal, da alle Werte satisch sind, bis auf das durch ein Mutex gesichertes Flag
		// es sollten also keine Synchronisationsprobleme auftauchen
		Server<MsgHandler> &remoteHandler = *(Server<MsgHandler>*)data;

        if (remoteHandler.m_priority != system::Priority::None)
        {
            system::makeThreadRealTime(remoteHandler.m_priority);
        }

		int numMessages = remoteHandler.info_.numMessages; // nur Optimierung! die Konstante wird lokal gespeichert
		auto interfaceName = remoteHandler.info_.interfaceName();
		//std::cout << "local::Server<MsgHandler> " << Poco::Thread::current()->id() << "activating:" << std::endl;
		remoteHandler.messageLoopActive_.set(); // wir "melden" den Thread aktiv

		while(!remoteHandler.isStopped()) {
			//std::cout << "msgLoop: in loop" << std::endl;

			//std::cout << "msgLoop: still in msgLoop" << std::endl;
			// lass den Receiver ueber das Protocol auf die erste Nachricht warten
			try {
				//std::cout << "msgLoop: finding next Handler : " << currentHandler  << std::endl;
				remoteHandler.threadStorage_.handlerFree.wait();
				//std::cout << "msgLoop: next Handler : " << currentHandler  << std::endl;
				ThreadStorage &ts(remoteHandler.threadStorage_);
				// Message abholen
				//std::cout << "msgLoop: waiting for message: " << std::hex << int(ts.receiver) << std::endl;
				// aus Solidaritaet mit Events (wg Pulse) sind Messges auf << 255 beschraenkt
				try {
					ts.msgNum = std::min(std::max(ts.receiver->getMessage(), std::numeric_limits<int>::min()), 255);
				} catch (MessageException &ex) {
					std::cout << interfaceName << " msgLoop: ignoring corrupt message: " << ex.what() << std::endl;
				}
				//std::cout << "msgLoop: ... got message " << ts.msgNum << std::endl;
				// Handler starten
				//std::cout << "msgLoop: ... unlocking handler " << currentHandler << std::endl;
				if ( (ts.msgNum>=0) && (ts.msgNum<numMessages) )
				{ // die virtuelle Funktion handle wird on der Abgleiteten Klasse aufgerufen und startet das richtige Callback
					ts.messageReceived.set();

				}	else {
					if (ts.msgNum==ShutdownMessage) {
						// regulaeres Shutdown
						//std::cout << "handler received shutdown: " << ts.threadNum << std::endl;
						remoteHandler.systemRunning(false);	// ab jetzt stoppen die Threads wenn sie nicht blockiert sind
						ts.receiver->nullReply(); // dann kann sendmessage weitermachen
						ts.messageReceived.set();
						//std::cout << "handler before exit: " << localStuff->threadNum << std::endl;
						break; // wir gehen freiwillig raus
					} else if (ts.msgNum==TimeoutMessage) {
						// e.g. sin schickt Messages an alle Prozesse und will eine Antwort
						//std::cerr << "handler received timeout" << ts.msgNum << std::endl;
						ts.handlerFree.set();
						// Handler wurde nicht benutzt, kann fuer die naechste Message verwendet werden.
					} else {
						// e.g. sin schickt Messages an alle Prozesse und will eine Antwort
						std::cerr << interfaceName << " handler received unexpected Message:" << ts.msgNum << std::endl;
						ts.receiver->nullReply();
						ts.handlerFree.set();
					}
				} // if

				//std::cout << "msgLoop: ... unlocked handler " << currentHandler << std::endl;
			} catch (Poco::Exception &e) {
				std::cout << interfaceName << " msgLoop: Poco::exception " << Poco::Thread::current()->id() << std::endl;
				std::cout << e.what();
				// alles Aussteigen!!!
				remoteHandler.systemRunning(false);
				break;
			} catch (...) {
				std::cout << interfaceName << " msgLoop: unknown exception " << Poco::Thread::current()->id() << std::endl;
				// alles Aussteigen!!!
				remoteHandler.systemRunning(false);
				break;
			}
		} // while not stoppped
		//std::cout << "local::Server<MsgHandler> messageLoop deactivating: 0" << std::endl;

		// aktiviere alle HandlerThreads, die vllt. noch auf ihr Mutex warten, wg systemRunning_=false stoppen sie
		//std::cout << "local::Server<MsgHandler> messgeLoop trylock: " << i << std::endl;
		remoteHandler.threadStorage_.messageReceived.tryWait(0);
		//std::cout << "local::Server<MsgHandler> messgeLoop triedlock: " << i << std::endl;
		remoteHandler.threadStorage_.messageReceived.set();
		//std::cout << "local::Server<MsgHandler> messgeLoop unlocked: " << i << std::endl;

		//std::cout << "local::MsgHandler<"<< remoteHandler.info().name() << "> messageLoop deactivating: 1" << std::endl;
		//remoteHandler->activeMsgThreads_.wait(); // wir "melden" den Thread ab, das muss immer klappen
		// wenn nun der Zaehler auf Null ist, dann ist kein Prozess mehr am laufen
	} // MessageLoop

	void SYSTEM_API Server<MsgHandler>::handlerThread(void * data) {
		// mit diesem Pointer greift der Code auf die ServerKlasse im anderen Thread zu
		// dies ist legal, da alle Werte satisch sind, bis auf das durch ein Mutex gesichertes Flag
		// es sollten also keine Synchronisationsprobleme auftauchen
		ThreadStorage *localStuff = (ThreadStorage*)data;
		Server<MsgHandler> *remoteHandler = localStuff->mainThread;
		//std::cout << "handler " << localStuff->threadNum << " started" << std::endl;
        std::string threadName{"H"};
        threadName += remoteHandler->info().interfaceName();
        prctl(PR_SET_NAME, threadName.c_str());


		remoteHandler->activeHandlers_.set(); 	// wir melden uns aktiv

		//std::cout << "handler " << localStuff->threadNum << " semaphore set" << std::endl;
		//localStuff->handlerFree.unlock(); 	// wir melden uns frei, dieMsgLoop (findFree) wartet auf diese Semaphore

		// erste Mutexrunde ausserhalb Whileschleife
		//		so geschieht die systemRunning-Abfrage BEVOR eine Auswertung gestartet wird
		//std::cout << "handler " << localStuff->threadNum << " waiting for first msg" << std::endl;
		localStuff->messageReceived.wait(); 			// wir warten auf Auftrag
		//std::cout << "handler " << localStuff->threadNum << " received first msg" << localStuff->msgNum << std::endl;

		// solange bis ein Handler die
		while(!remoteHandler->isStopped()) {
			//std::cout << "handler " << localStuff->threadNum << " got msg " << localStuff->msgNum << std::endl;
			// optimierte Abfrage!!! unsigned sind die Negativen Werte sehr gross -> nur eine Abfrage!!!
			// eine minimale Konsistenzpruefung
			//if ( uInt(msgNum)<uInt(numMessages) )
			try {
                if (localStuff->receiver != nullptr)
                {
                    remoteHandler->handle(*localStuff->receiver, localStuff->msgNum);
                }
                else
                {
                    std::cerr << __FUNCTION__ << " " << threadName << ": Receiver is a nullPointer";
                    wmLog(eError, "%s %s: Receiver is a nullPointer\n", __FUNCTION__, threadName);
                }
			} // catch
			catch(...) {
                std::string info ( __FUNCTION__ );
                info += " ";
                info += threadName;
				system::logExcpetion(info, std::current_exception());                
			} // catch
			localStuff->handlerFree.set(); 				// wir melden uns frei
			localStuff->messageReceived.wait(); 	// wir warten auf Auftrag
			//std::cout << "handler " << localStuff->threadNum << "handled msg " << localStuff->msgNum << std::endl;
		} // while !stopped
		//localStuff->handlerFree.unlock(); 	// wir melden uns frei (falls irgenjemand auf uns wartet)
		//std::cout << "handler " << localStuff->threadNum << " : going down" << std::endl;
	}
} // namespace interface
} // namespace precitec
