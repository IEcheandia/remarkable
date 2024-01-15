#ifndef UDP_PROTOCOL_H_
#define UDP_PROTOCOL_H_

#include <string>
#include <iostream>

#include "message/message.h"
#include "protocol/protocol.h"
#include "protocol/infinitePollableDatagramSocket.h"
#include "message/messageReceiver.h"

#include "Poco/Thread.h"
#include "Poco/Net/Net.h"
#include "Poco/Net/DatagramSocket.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/NetException.h"
#include <time.h>

#include <unistd.h>

namespace precitec
{
namespace system
{
namespace message
{
	using std::min;
	using std::max;
	using Poco::Net::DatagramSocket; // UDP-Socket
	using Poco::Net::SocketAddress;
	using Poco::Net::IPAddress;

	/**
	 *  Diese Klasse stellt einer Verbindung zu einem Messagereceiver her. Das
	 *  geschieht ueber den Namen. Es genuegt wenn Sender und Receiver den selben
	 *  Namen tragen. Weiter muss sichergestellt sein, dass der Receiver (Server)
	 *  schon erzeugt wurde.
	 *  Die Prozesse des Senders und Receivers muessen auf dem gleichen Rechner sein.
	 */
	class UdpProtocol : public Protocol {
	 typedef SocketInfo const* PSI;

	public:
		/// Dieser CTor wird normalerweise nur ueber die Fabrikfkt create aufgerufen
		UdpProtocol();
		UdpProtocol(SocketInfo const& info);
		UdpProtocol(SmpProtocolInfo const& info);
		~UdpProtocol() override;

		/// Die ChannelId wird als Kriterium herangezogen
		virtual bool isValid();
		/// gibt die ProtocolInfo zurueck (die erst in der abgeleiteten Klasse definiert ist)
		ProtocolInfo const& protocolInfo() const override;

		static int ThreadID()
		{
			Poco::Thread* pThrd = Poco::Thread::current();
			if(pThrd)
				return pThrd->id();
			else
				return 0;
		}

	public:

		void initSender() override;

		void initReceiver() override;

		/**
		 * receive wartet auf ein Datagramm und lest dann die Daten ein
		 * receive wirft (direkt oder ueber RawRead) Poco::Exception& (timeout)
		 * receive ruft rawRead fuer das eigentliche (ggf. mehrphasige) Lesen
		 * \param MessageBuffer buffer hierhin werden die Daten kopiert und der Header gesetzt
		 * \return Nummer der Message (fuer die Callback-Tabelle)
		 */
		int getMessage(MessageBuffer &buffer) override;

		void reply(MessageBuffer &replyBuffer) override;

		void send(MessageBuffer &sendBuffer, MessageBuffer &replyBuffer) override;

		void sendPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId=module::Client) override;

        void sendQuitPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId=module::Client) override;

	private:
		/**
		 * rawSend erledigt das senden ggf in kleinen (ca. 64K) Bloecken wenn die
		 * Puffer gross sind. Alle Info ist im SendPuffer enthalten
		 * \param  sendBufffer Puffer mit allen Daten, im Header ist die Groesse enthalten
		 */
		void rawSend(MessageBuffer &buffer);

		//
		void rawReply(MessageBuffer &buffer, const SocketAddress & sender);

		/**
		 * rawReceive liest von einem Socket Daten
		 * rawReceive wird aufgerufen, wenn poll() erfolgreich war, es ist also irgentetwas da
		 * Wenn die Message (header ist aufden ersten Byts untergebracht) mehr Daten hat als
		 * UDP in einem Rutsch uebertragen kann (ca. 64K), wird solange nachgelesen bis alle Daten
		 * da sind. Wenn dabei ein Timeout auftritt, (Poco::Exception&) wird dieser nicht aufgefangen,
		 * sondern weitergeleitet.
		 * Im Normalfall wird die MsgNummer (aus Messageheader) zurueckgegeben
		 * \param MessageBuffer &buffer von Aussen vorgegebener Puffer
		 * \result Messagenummer
		 */
		int rawReceive(MessageBuffer &buffer);

		int rawReceiveFrom(MessageBuffer &buffer, SocketAddress & sender);


	private:
		InfinitePollableDatagramSocket 	socket_;
		SocketAddress 	sender_; // entspricht replyId_????

		/// hiermit wird eine Verbindung aufbgebaut = url + port/service
		SocketInfo 			socketInfo_;
		/// Timeout fuer Verbindungsaufnahme
		Poco::Timespan 	timeout_;
		bool						initialized_;
		enum { UDPSizeLimit = 1500 }; // Protokollabhaengiges Maximum fuer Bytes pro Transfer
	}; // UdpProtocol

	template<>
	inline SmpProtocol createTProtocol<Udp>(SmpProtocolInfo & info) {
		return SmpProtocol(new UdpProtocol(info));
	}


} // namespace message
} // namespace system
} // precitec

#endif /* UDP_PROTOCOL_H_*/
