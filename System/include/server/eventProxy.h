/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			wor, kir
 *  @date			2009
 *  @brief			Defines the base class for all remote event callers.
 */


#ifndef EVENT_PROXY_H
#define EVENT_PROXY_H

#include <iostream>
#include <string>

#include "Poco/SharedPtr.h"
#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "message/eventSignaler.h"
#include "protocol/protocol.h"
#include "server/interface.h"

namespace precitec
{
namespace system
{

namespace message
{
	class Sender;
	struct MessageBase;
}} // system::message


namespace interface
{


	/**
	 * Die Basisklasse fuer alle Remote-Caller.
	 * Die Details der Message-Verarbeitung liegt im Sender.
	 */
	template <>
	class Server<EventProxy> {
	public:
		Server(MessageInfo const& info)
//		: info_(info), signaler_(info_.numMessages, info_.sendBufLen, info_.numHandlers),
		: info_(info), signaler_(info_),
		senderMutex_(new Poco::FastMutex()) {
//			senderMutex_(new Poco::FastMutex()) {
			//std::cout << "remote CTor::Server<Proxy> " << std::endl;
		}
		virtual ~Server()
        {
            delete senderMutex_;
        }
	public:
		void initMessage(std::size_t i) { signaler().initMessage(i); }
		template<class T>
		void marshal(T const&t) { signaler().marshal(t); }
		void send() { try { signaler().send(); } catch(...) {
		}; }


		/// ein einzelnes Event einer Schnittstelle wird aktiviert
		//void activate(SmpProtocolInfo &p, int eventNum) { signaler().addSubscriber(eventNum, p); }
		/// ein komplette Schnittstelle wird aktiviert
		void activate(system::message::SmpProtocolInfo &p) {
			// erst den neuen Subscriber an sich einbauen
			signaler().addSubscriber(info_.interfaceName(), p);
			// dann werden die einzelnen events in die richtigen Subscriberlisten eingetragen
			for (int event=0; event<info_.numMessages; ++event)	{
				//std::cout << "eventProxy::activate " << " " << info_.sendBufLen << std::endl;
				signaler().addSubscriber(event, p);
			}
			//std::cout << "eventProxy::all Signals activated " << " " << info_.sendBufLen << std::endl;
		}
		/// ein einzelnes Event einer Schnittstelle wird deaktiviert
		void deActivate(system::message::SmpProtocolInfo &p, int eventNum) { signaler().removeSubscriber(eventNum, p); }
		/// ein komplette Schnittstelle wird deaktiviert
		void deActivate(system::message::SmpProtocolInfo &p) {
			for (int event=0; event<info_.numMessages;++event)	{
				deActivate(p, event);
			}
		}
		int numMessages() const { return info_.numMessages;	}
		// Dies ist die Process-Manager Schnittstelle
		system::message::EventSignaler &signaler() { return signaler_;}
		friend std::ostream &operator <<(std::ostream &os, Server<EventProxy> const& s);


		MessageInfo info_;
	private:

		system::message::EventSignaler signaler_;
#ifndef NDEBUG
		std::string		name_;	///<<< name of current Message
#endif
	protected:
		Poco::FastMutex *senderMutex_;
	};

	inline std::ostream &operator <<(std::ostream &os, Server<EventProxy> const& s) {
		os << "Server<EventProxy>: " << system::module::InterfaceName[s.info_.interfaceId] << ": " << s.signaler_; return os;
	}
} // namespace interface
} // namespace precitec


#endif // EVENT_PROXY_H
