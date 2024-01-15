#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include <iostream>

#include "SystemManifest.h"

#include "Poco/SharedPtr.h"
#include "message/message.h"
#include "module/interfaces.h"
#include "protocol/protocol.info.h"
#if defined __QNX__ || defined __linux__
	// das Qnx-Protokoll mit ProcessInfo wird nur unter QNX verwendet
	#include "protocol/process.info.h"
#endif
#include <vector>


namespace precitec
{
namespace system
{
namespace message
{
	class MessageBuffer;

	/**
	 * Protokol ist die abstrakte Basisklasse fuer alle Message-Protokolle.
	 * Hier sind die Funktionen send, reply, sendPulse definiert.
	 */
	class SYSTEM_API Protocol	{
	public:
		Protocol(ProtocolType t) : pType_(t) {}
		virtual ~Protocol() {}
		enum { ExtraDebugTimeOutFactor = 1, Milliseconds = 1000 };
	public:
		// es gibt unterschiedlichen Code fuer Empfaenger und Sender
		virtual void initSender() = 0;
		virtual void initReceiver() = 0;
		/// debug, um Zugriffe auf abstrakte Klasse abzufangen
		/// gibt die ProtocolInfo zurueck (die erst in der abgeleiteten Klasse definiert ist)
		virtual ProtocolInfo const& protocolInfo() const = 0;
		/// gibt an, ob Komm. global ist, d.h. ShMemPtr werden wie normale Ptr behandelt
		//bool isGlobal() { return true; }
	public:
		// fuer Sender und Receiver
		/// Accsessor fuer den Protocoltyp (Achtung!!! geht auch ueber Protocolinfo->type())
		ProtocolType protocolType() const { return pType_; }
	public:
		// nur fuer Sender:
		// sende eine Message und warte auf eine Antwort
		virtual void send(MessageBuffer &sendBuffer, MessageBuffer &replyBuffer){}
		// sende eine Message und warte nicht
		virtual void sendPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId=module::Client){}
		virtual void sendQuitPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId=module::Client)
        {
            sendPulse(sendBuffer, interfaceId);
        }
	public:
		// nur fuer Empfaenger
		/// warte auf eine Message
		virtual int  getMessage(MessageBuffer &sendBuffer) {
			std::cout << "protocol::getMessage virt. base should never be called" << std::endl;
			return -1;
		}
		/// warte auf ein Pulse
		virtual int  getPulse(MessageBuffer &sendBuffer) {
			//std::cout << "protocol::getPulse virt. base should never be called" << std::endl;
			//return -1;
			return getMessage(sendBuffer); // wenn Pulse nicht explizit implementiert ist, Message aufrufen
		}
		/// Antwort auf eine Message, wartet nicht
		virtual void reply(MessageBuffer &replyBuffer) {}
		/// Null-Antwort auf eine unbekannte Message, oder auf einen Qnx-Pulse
		virtual void nullReply(MessageBuffer &replyBuffer) { reply(replyBuffer); }
		//
		virtual void stop() { return; }
	private:
		/// gibt den Typ des Protocols an
		const ProtocolType pType_;
	};

	/// wenn das Protocoll zu Compilezeit bekannt ist, ...
	template <ProtocolType type>
	SYSTEM_API SmpProtocol createTProtocol(SmpProtocolInfo & info);
	/// ... fuer Laufzeitentscheidung bzgl. des Protokolls
	SYSTEM_API SmpProtocol createProtocol(SmpProtocolInfo & info);


} // namespace message
} // namespace system
} // precitec

#endif /*PROTOCOL_H_*/
