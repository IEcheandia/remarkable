#ifndef MESSAGE_PROXY_H
#define MESSAGE_PROXY_H

#include <iostream>

#include "Poco/SharedPtr.h"
#include "Poco/Thread.h"
#include "Poco/Mutex.h"
#include "message/messageSender.h"
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
}} // message

	using system::message::Sender;
	using system::message::MessageBase;
	using system::message::Protocol;
	using system::message::SmpProtocolInfo;

namespace interface
{


	/**
	 * Die Basisklasse fuer alle Remote-Caller.
	 * Die Details der Message-Verarbeitung liegt im Sender.
	 */
	template <>
	class Server<MsgProxy> {
	public:
		void initMessage(std::size_t i ) { sender().initMessage(i); }
		template<class T> void marshal(T const&t) { sender().marshal(t); }
		/// Exception-fangende Variante
		void send() {
			try {sender().send();}
			catch(...) {};
		}

		template<class T> T sendWithReturn(T defaultVal) {
			try {
				sender().send();
				//return sender().deMarshal(Sender::TypeHolder<T>());
				T ret; sender().deMarshal(ret);
				return ret;
			} catch (...) {
				return defaultVal;
			}
		}
		template<class T> T sendWithReturn(T const& defaultVal, int size) {
			try {
				sender().send();
				//return sender().deMarshal(Sender::TypeHolder<T>(), size);
				T ret; sender().deMarshal(ret, size);
				return ret;
			} catch(...) {
				return defaultVal;
			}
		}
	public:

		Server(MessageInfo const& info)
		: info_(info), sender_(), senderMutex_(new Poco::FastMutex())	{
			//std::cout << "remote CTor::Server<Proxy> " << InterfaceName[info.interfaceId] << std::endl;
		}
		Server(MessageInfo const& info, SmpProtocolInfo &protInfo)
		: info_(info), sender_(), senderMutex_(new Poco::FastMutex())	{
			sender_.setProtocol(protInfo, info_.sendBufLen, info_.replyBufLen);
			//std::cout << "remote CTor::Server<Proxy>wp " << InterfaceName[info.interfaceId] << std::endl;
		}
		virtual ~Server() { if (senderMutex_) delete senderMutex_; }

		void activate(SmpProtocolInfo &protInfo) {
			//std::cout << "Server<MsgProxy>::activate" << std::endl;
			if (protInfo.isNull()) { std::cout << "Server<MsgProxy>::activate: empty protocol" << std::endl; throw; }
			//std::cout << *protInfo << std::endl;
			sender().setProtocol(protInfo, info_.sendBufLen, info_.replyBufLen);
			}
		void deActivate() {
			sender().stop();
			}
		int numMessages() const { return info_.numMessages;	}
		/// Dies ist die Process-Manager Schnittstelle
		Sender &sender() { return sender_;}
		/// alles Wesentliche ueber die Schnittstelle
		MessageInfo info_;
		/// Ausgabe wg Nice-Class
		friend  std::ostream &operator <<(std::ostream &os, Server<MsgProxy> const& s);
	private:
		Sender sender_;
	protected:
		Poco::FastMutex *senderMutex_;
	};

	typedef Poco::SharedPtr<Server<MsgProxy> > SmpMsgProxy;

	inline std::ostream &operator <<(std::ostream &os, Server<MsgProxy> const& s) {
		os << "Server<MsgProxy>: " << system::module::InterfaceName[s.info_.interfaceId] << ": " << s.sender_; return os;
	}
} // namespace interface
} // namespace precitec


#endif // MESSAGE_PROXY_H
