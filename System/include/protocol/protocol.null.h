#ifndef NULL_PROTOCOL_H_
#define NULL_PROTOCOL_H_

#include <string>
#include <iostream>

#include "message/message.h"
#include "message/messageBuffer.h"
#include "protocol/protocol.h"
#include "message/messageReceiver.h"
#include "module/interfaces.h"

#include <unistd.h>

namespace precitec
{
namespace system
{
namespace message
{

	/**
	 *  Diese Klasse stellt einer Verbindung zu einem Messagereceiver her. Das
	 *  geschieht ueber den Namen. Es genuegt wenn Sender und Receiver den selben
	 *  Namen tragen. Weiter muss sichergestellt sein, dass der Receiver (Server)
	 *  schon erzeugt wurde.
	 *  Die Prozesse des Senders und Receivers muessen auf dem gleichen Rechner sein.
	 */
	class NullProtocol : public Protocol {

	public:
		/// Dieser CTor wird normalerweise nur ueber die Fabrikfkt create aufgerufen
		NullProtocol()  // timeout in Mikrosekunden
			: Protocol(NullPCol), proxySync_(), serverSync_() { proxySync().wait(); serverSync().wait(); }
		NullProtocol(NullInfo const& info)  // timeout in Mikrosekunden
			: Protocol(NullPCol), proxySync_(info.proxySync_), serverSync_(info.serverSync_) {
			//std::cout << "NullProtocol::CTor(NullInfo): ok" << std::endl;
		}
		/// zweites Protokoll fuer Server/Client (typischerweise der Client), kopiert die Mutexe aus Info
		NullProtocol(SmpProtocolInfo const& info)
			: Protocol(NullPCol),
			proxySync_(dynamic_cast<NullInfo::PNI>(&*info)->proxySync_),
			serverSync_(dynamic_cast<NullInfo::PNI>(&*info)->serverSync_)	{
			//std::cout << "NullProtocol::CTor(NullInfo) ok :" << std::endl;
		}

		~NullProtocol() { }
	public:
		/// Die ChannelId wird als Kriterium herangezogen
		virtual bool isValid(){ return true; }

		void close() { proxySync().set(); serverSync().set(); }
		/// Der ProcessInfo-Def-CTor errzeugt ungueltige Info -> ungueltiges Protokoll
		//virtual Protocol inValid() { return NullProtocol(SocketInfo()); }

		/** Protokolle wrden geschickterweise ueber SmartPtr verwaltet, da sie
		 * an einer Stelle einmal (keine Redundanz) erzeugt werden sollten und dann
		 * allerdings an verschiedenster Stelle in den Servern gespeicher werden
		 * da dort die Protokollart nicht bekannt ist, ist ein Copy schwierig -> SmartPtr
		 */
		static Poco::SharedPtr<Protocol> create(const NullInfo& info)	{
			return Poco::SharedPtr<Protocol> (new NullProtocol(info));
		}

	public:
		virtual void initSender()	{}

		virtual void initReceiver() {}

		/**
		 * receive wartet auf ein Datagramm und lest dann die Daten ein
		 * receive wirft (direkt oder ueber RawRead) Poco::Exception& (timeout)
		 * receive ruft rawRead fuer das eigentliche (ggf. mehrphasige) Lesen
		 * \param MessageBuffer buffer hierhin werden die Daten kopiert und der Header gesetzt
		 * \return Nummer der Message (fuer die Callback-Tabelle)
		 */
		int getMessage(MessageBuffer &buffer)
		{
			serverSync().wait();
			buffer.copyFrom(buffer_);
			//std::cout << "... getMessage: got "; buffer.dump<int>(std::cout); std::cout << " ---> Msg: " << buffer.header().messageNum << std::endl;
			return buffer.header().messageNum;
	  	}

		void reply(MessageBuffer &replyBuffer)
		{
			//std::cout << "reply: "; replyBuffer.dump<int>(std::cout); std::cout << std::endl;
			buffer_.copyFrom(replyBuffer);
			proxySync().set(); // gibt Puffer frei
		}

		void send(MessageBuffer &sendBuffer, MessageBuffer &replyBuffer)
		{ /// Abfrage auf ReturnTyp = PulseType fehlt noch
			//std::cout << "send: "; sendBuffer.dump<int>(std::cout); std::cout << std::endl;
			buffer_.copyFrom(sendBuffer);std::cout << std::endl;
			serverSync().set(); // gibt Puffer frei
			proxySync().wait(); 	// wartet auf Antwort
			replyBuffer.copyFrom(buffer_);
			//std::cout << "... send: replying "; replyBuffer.dump<int>(std::cout); std::cout << std::endl;
		}

		virtual void sendPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId=module::Client)
		{
			//std::cout << "... send: pulse" << std::endl;
			buffer_.copyFrom(sendBuffer);
			serverSync().set(); // gibt Puffer frei
		}
		/// gibt die ProtocolInfo zurueck (die erst in der abgeleiteten Klasse definiert ist)
		virtual ProtocolInfo const& protocolInfo()  const { return nullInfo_; }

	private:
		Poco::Semaphore	&proxySync() { return *proxySync_; }
		Poco::Semaphore	&serverSync() { return *serverSync_; }

		Poco::SharedPtr<Poco::Semaphore> proxySync_;
		Poco::SharedPtr<Poco::Semaphore> serverSync_;
	private:
		/// hier steht nix drin aber sonst kann protocolInfo() nicht definiert werden
		NullInfo	nullInfo_;
		static StaticMessageBuffer	buffer_;
	}; // NullProtocol

	template<>
	inline SmpProtocol createTProtocol<NullPCol>(SmpProtocolInfo &info) {
		//std::cout << "createTProtocol<NullPCol> " << std::endl;
		return SmpProtocol(new NullProtocol(info) );
	}

} // namespace message
} // namespace system
} // precitec

#endif /* NULL_PROTOCOL_H_*/
