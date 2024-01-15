/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR, WOR
 *  @date			2009
 *  @brief			Base class for event handlers.	
 */

#include <iostream>

#include "SystemManifest.h"
#include "server/eventHandler.h"
#include "message/messageSender.h"
#include "message/messageReceiver.h"

#include "module/moduleLogger.h"

namespace precitec
{
	using namespace system::message;
namespace interface
{
	/**
	 *	Server<EventHandler> is only a holder for the protocol-specific
	 *	ProtocolHandlers. Thus it needs only to store the info-Structure
	 *	(mostly for debug-purposes) and initialize the serverMutex_ (after all
	 *	all events of all protocols are served by the same server.
	 *	\todo This is rather conservative. In future, one might consider
	 *	the option of the server mutexing each serviced function individually, 
	 *	thus allowing the server to serve certain not-interfering in parallel.
	 *	The current implementation does not allow this.
	 */
	SYSTEM_API Server<EventHandler>::Server(MessageInfo const& info)
			: info_(info), serverMutex_(new Poco::FastMutex())  {
		//std::cout << "Server(MessageInfo" << info << ")" << std::endl;
//		for (int protocol=0; protocol<NumActiveProtocols; ++protocol) {
//			protocolHandler_[protocol]	= NULL; //new ProtocolHandler(info, this);
//		}
		protocolHandler_[Tcp]			= new ProtocolHandler(info_, this, Tcp);
		protocolHandler_[Udp]			= new ProtocolHandler(info_, this, Udp);
		protocolHandler_[Qnx]			= new ProtocolHandler(info_, this, Qnx);
		protocolHandler_[NullPCol]	= new ProtocolHandler(info_, this, NullPCol);
	}

	/// Aufraeumen des Handler-Array
	SYSTEM_API Server<EventHandler>::~Server() {
//		for (int p=0; p<NumActiveProtocols; ++p) {
//			if (protocolHandler_[p]) {
//				std::cout << "Server<EventHandler><" << info_.name() << ">::dTor(" << ProtocolType(p) <<  ")>::stopping" << std::endl;
//				protocolHandler_[p]->stop();
//				std::cout << "Server<EventHandler><" << info_.name() << ">::dTor(" << ProtocolType(p) <<  ")>::stopped" << std::endl;
//				try {
//					delete protocolHandler_[p];
//				} catch (std::exception const&ex) {
//					std::cout << "Server<EventHandler><" << info_.name() << ">::dTor(" << ProtocolType(p) <<  ")>::delete failed Exception!!! " << ex.what()<< std::endl;
//				} catch (...) {
//					std::cout << "Server<EventHandler><" << info_.name() << ">::dTor(" << ProtocolType(p) <<  ")>::delete failed Exception!!!" << std::endl;
//				}
//				std::cout << "Server<EventHandler><" << info_.name() << ">::dTor(" << ProtocolType(p) <<  ")>::deleted" << std::endl;
//				protocolHandler_[p] = NULL;
//			}
//		}

		//std::cout << "Handler going down!" << std::endl;
	}

	SmpProtocolInfo Server<EventHandler>::activateProtocol(SmpProtocolInfo &protInfo) {
		// wir schauen uns den Server fuer das gewuenschte Protokoll an
		// Events koennen aktiviert und deaktiviert werden, die Server werden jedesmal neu aufgesetzt
		// fuer ein noch nicht beservtes Protokoll, werden fuer alle Threads die Daten initialisiert
		//std::cout << "EventHandler::activateProtocol" << std::endl;
		try {
			ProtocolType protocolType(protInfo->type());
			//std::cout << "EventHandler<" << info_.name() << "(" << protocolType <<  ")>::activateProtocol" << std::endl;
//			if (protocolHandler_[protocolType]) {
				//std::cout << "Server<EventHandler>::activateProtocol(" << *protInfo << "): protocol already active" << std::endl;
				//std::cout << "Server<EventHandler>::activateProtocol protocol: " << protocolHandler_[protocolType]->protocolInfo() << std::endl;
				//delete protocolHandler_[protocolType];
//			}

		//if (!protocolHandler_[protocolType]) {
			// beim ersten Mal Protokol erzeugen
//			protocolHandler_[protocolType]	= new ProtocolHandler(info_, this, protocolType);
			PvString shMemName("");
			if (protocolType==Qnx) {
				// only the qnx protocol uses ShMems as an optimizing feature
				ProcessInfo &pI(*dynamic_cast<ProcessInfo*>(&*protInfo));
				shMemName = pI.shMemName();
			}
			// ... initialisieren
			// collect data into thread(protocol) specific data-areas
			//std::cout << "EventHandler<" << info_.name() << ">::activateProtocol - pre initThreadStorage" << std::endl;
			protocolHandler_[protocolType]->initThreadStorage(protInfo, shMemName);
		//} else {
		//	std::cout << "Server<EventHandler>::activateProtocol(" << *protInfo << "): protocol already active" << std::endl;
		//}
	} catch (...) {
		std::cout << "EventHandler::activateProtocol: bad something" << std::endl;
	}
		//std::cout << "Server<EventHandler>::activateProtocol: " << *protInfo << std::endl;
		// ab hier registriert der Abgeleitete Server seine Callbacks und startet dann die MessageLoop

		return protInfo; // mit diesem (meist gleichen) Protokol wird der Client gestartet
	}



	/**
	 * acivate startet den EventHandler fuer den angegebenen Protokolltyp
	 * Das Protokoll wird lazy erst jetzt erzeugt
	 * @param protInfo protInfo
	 * @return
	 */
//	SmpProtocolInfo Server<EventHandler>::activate(SmpProtocolInfo &protInfo) {
//		// wir schauen uns den Server fuer das gewuenschte Protokoll an
//		// Events koennen aktiviert und deaktiviert werden, die Server werden jedesmal neu aufgesetzt
//		// fuer ein noch nicht beservtes Protokoll, werden fuer alle Threads die Daten initialisiert
//
//		// protInfo kann von createProtocol veraendert werden, um Info von )
//		// Receiver (wird immer zuerst erzeugt an Sender weiter zu geben
//		ProtocolHandler &pHandler(*protocolHandler_[protInfo->type()]);
//
//		SmpProtocol p = createProtocol(protInfo);
//		pHandler.initThreadStorage(info_.sendBufLen, info_.replyBufLen, p);
//		// ab hier registriert der Abgeleitete Server seine Callbacks und startet dann die MessageLoop
//
//		return protInfo; // mit diesem (meist gleichen) Protokol wird der Client gestartet
//	}

	void SYSTEM_API Server<EventHandler>::start(ProtocolType t) {
//		if (protocolHandler_[t]) {
//			//stop(t); // erst ggf. stoppen -> start == restart
//			protocolHandler_[t]->start();
//		}
        protocolHandler_[t]->setRealTimePriority(m_priority);
		protocolHandler_[t]->start();
	}

	void SYSTEM_API Server<EventHandler>::stop(ProtocolType t) {
//		if (protocolHandler_[t]) {
//			std::cout << "EventHandler<" << info_.name() << ">::stop " << (protocolHandler_[t]->isStopped() ? " isStopped" : "isRunning")<< std::endl;
//			if (!protocolHandler_[t]->isStopped()) {
//				protocolHandler_[t]->stop();
//			}
//		} else {
//			//std::cout << "EventHandler<" << info_.name() << ">::stop - trying to stop nonexisting protocolHandler" << std::endl;
//		}
		wmLog( eDebug, "EventHandler<%s>::stop - trying to stop nonexisting protocolHandler\n", info_.name().c_str() );
		if (!protocolHandler_[t]->isStopped()) protocolHandler_[t]->stop();
	}

	/// die abgeleitete Klasse wartet hiermit auf das Ende des Server und leitet DTor ein
	void SYSTEM_API Server<EventHandler>::waitForServerEnd() {
//		for (int p=0; p<NumActiveProtocols; ++p) {
//			if (protocolHandler_[p]) {
//				protocolHandler_[p]->waitForServerEnd();
//			}
//		}
		for (int p=0; p<NumActiveProtocols; ++p) protocolHandler_[p]->waitForServerEnd();
	}

} // namespace interface
} // namespace precitec
