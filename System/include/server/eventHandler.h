/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			KIR, WOR
 *  @date			2009
 *  @brief			Base class for event handlers.	
 */

#ifndef EVENT_HANDLER_H_
#define EVENT_HANDLER_H_

#include <iostream>

#include "SystemManifest.h"

#include "Poco/Mutex.h"
#include "Poco/SharedPtr.h"
#include "Poco/Thread.h"
#include "message/derivedMessageBuffer.h"
#include "message/messageReceiver.h"
#include "server/msgDispatcher.h"
#include "protocol/protocol.h"
#include "protocol/process.info.h"
#include "message/messageMacros.h"
#include "server/interface.h"
#include "server/protocolHandler.h"
#include "system/realTimeSupport.h"

namespace precitec
{
namespace system
{
namespace message
{
	// Vorwaertsdeklarationen muessen im richtigen Namespace erfolgen
	class Receiver;
	struct MessageBase;

} // message
} // system


namespace interface
{
	/**
	 * Die Basisklasse fuer alle Remote-Hanlder. Man beachte das der Remote-Handler
	 * lokal vorliegt, lediglich die Messages des Remote-Client akzeptiert.
	 * Im Moment ist die Klasse leider ziemlich ausgeduennt worden, da es Schwierigkeiten gibt,
	 * die jeweils richtigen  Routinen der konkreten Kindklasse aufzurufen.
	 */
	template <>
	class SYSTEM_API Server<EventHandler>	{
	public:
		/// legt die verschiedenen Handler an
		Server(MessageInfo const& info);
		virtual ~Server();

	public:
		/// wieso wird hier nichts getan??? etwa return activateProtocol(p)??
		virtual system::message::SmpProtocolInfo activate(system::message::SmpProtocolInfo & p) { return system::message::SmpProtocolInfo(); }
		/// aktiviert Server fuer bestimmten Protocoltyp (Tcp, Messaging, ...)
		system::message::SmpProtocolInfo activateProtocol(system::message::SmpProtocolInfo &protInfo);
		/// startet die Messageloop
		void start(system::message::ProtocolType t);
		/// beendet die Message-Loop, wartet ggf. 100 ms darauf
		void stop(system::message::ProtocolType t);
		/// beendet die Message-Loop fuer alle Protokolle, wartet ggf. 100 ms darauf
		void stop() { for (int p=0; p<system::message::NumActiveProtocols; ++p) { stop(system::message::ProtocolType(p)); } }
	//private:
		int numMessages() const { return info_.numMessages; }
		/// die abgeleitete Klasse wartet hiermit auf das Ende des Server und leitet DTor ein
		void waitForServerEnd();
		/// der abgleitete Server uebernimmt diese Funktion, da nur er typsicher aufloesen kann.
		virtual void handle(Receiver &, int ) { std::cout << " wrong handler" << std::endl;}

        void setRealTimePriority(system::Priority priority)
        {
            m_priority = priority;
        }
	public:
		/// ueber diese Referenz kann auf Message-Konstanten zugegriffen werden
		MessageInfo info_;
	protected:
		/// diese Routine wird aufgerugen falls eine ungueltige Message aufgerufen wird, sollte eigentlich selten vorkommen
		void defaultHandler(system::message::Receiver &)	{	std::cout << "default Handler" << std::endl; }
		/// beim Start des Handlers wird die Callbackliste (des dispatchers der abgeleiteten Klasse) initialisiert
		virtual void registerCallbacks() = 0;

		Poco::FastMutex *serverMutex_;

	private:
		/// jedes Protokol (UDP/TCP/QNX) hat seine eigenen spezifischen Handler
		ProtocolHandler	*protocolHandler_[system::message::NumActiveProtocols];
        system::Priority m_priority = system::Priority::None;
	};

	/**
	 * Diese Klasse erlaubt es transparent im Event-Handler nach Aufruf
	 * der Server-Funktion eine Aufraeum-Aktion zu starten. Insbesondre ist dies
	 * fuer das QNX-Protokoll noetig, da hier der SharedMem-Block freigegeben werden
	 * muss.
	 */
//	template <class Server> class Holder {
//	public:
//		Holder(Server *s, MessageBuffer *) : s_(s), buffer_(NULL) {}
//		Holder(Server *s, SharedMessageBuff *b) : s_(s), buffer_(b) {}
//		~Holder() { if (buffer_) buffer_->freeBuffer(); }
//		Server &operator *() { return *s_; }
//		Server *operator->() { return s_; }
//	private:
//		Server *s_;
//		SharedMessageBuff *buffer_;
//	};

} // namespace interface
} // namespace precitec

#endif /*EVENT_HANDLER_H_*/
