/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			wor, kir
 *  @date			2009
 *  @brief			Defines messaging related stuff.
 */

#ifndef INTERFACE_H_
#define INTERFACE_H_

#include <list>
#include <iostream>



// standard-header
#include "system/types.h"
#include "system/typeTraits.h"
#include "system/templates.h"
#include "module/interfaces.h" // wg InterfaceName

// header fuer das Mesaage/Event-Interface
#include "protocol/protocol.h"
#include "message/message.h"
#include "message/messageMacros.h"
#include "Poco/TypeList.h"
#include "module/interfaces.h" // wg appId

namespace precitec
{
namespace interface
{
	/// die verschienenen Klassen die zu einem Server+RPC gehoeren
	enum { AbstractInterface=0, Messages=1, MsgServer=2, MsgProxy=3,
					MsgHandler=4, EventServer=5, EventProxy=6, EventHandler=7  };

	/// Server/Client mit Messages und Subscriber/Publisher (Events) verwenden das gleiche Konzept
	enum { IsMessage, IsEvent };

	/// somit ist Server mit seinen Instanzen (Server<Messages>Server<Proxy> ...) ansprechbar
	template <int CallType>
	class Server {};

	/// eine Convenience Struktur, die Konstanten der abgeleiteten Implementierung den Basis-Klassen zur Verfuegung stellt
	struct MessageInfo	{
		MessageInfo() {}
		MessageInfo(system::module::Interfaces iId, int i, int j, int k, int h=2)
		: interfaceId(iId), sendBufLen(i), replyBufLen(j), numMessages(k), numHandlers(h) {}
		MessageInfo(MessageInfo const& rhs)
		: interfaceId(rhs.interfaceId),
			sendBufLen(rhs.sendBufLen),
			replyBufLen(rhs.replyBufLen),
			numMessages(rhs.numMessages),
			numHandlers(rhs.numHandlers)
			{}
		MessageInfo& operator = (MessageInfo const& rhs) {
			MessageInfo tmp(rhs);
			std::swap(tmp, *this);
			return *this;
		}
		std::string interfaceName() const
		{
			std::stringstream oSt;
			oSt << system::module::InterfaceName[interfaceId];
			return oSt.str();
		}
		system::module::Interfaces interfaceId;
		int sendBufLen;
		int replyBufLen; ///< ggf mal spaeter: bei Events replyBufLen = BuffLen auf ShMem??
		int numMessages;
		int numHandlers;
		stdString name() const { return system::module::InterfaceName[interfaceId]; }
		friend std::ostream& operator << (std::ostream&os, MessageInfo const&i) {
			os << "MsgInfo<" << system::module::InterfaceName[i.interfaceId] << " #" << i.numMessages << ">: " << i.sendBufLen << ":" << i.replyBufLen << " (" << i.numHandlers << "x)"; return os;
		}
	};
	/**
	 * die eigentlichen Messages werden ausschliesslich
	 * in den abgeleiteten Klassen definiert
	 */
	template <>
	class Server<Messages>
	{
	public:
	};

	/**
	 * Resource wird in in der Messagelist der abgeleiteten
	 * Concrete-Server<Messages> im Hintergrund erstellt
	 * und erstelle eine Liste der Messages
	 */
	class Resource {
	public:
		typedef std::list<system::message::MessageBase> MsgList;
		typedef MsgList::const_iterator MsgCIter;

		Resource(system::module::Interfaces interfaceId) : interfaceId_(interfaceId) {}
		void push(system::message::MessageBase msg) { msgList_.push_back(msg); }
		friend bool operator == (Resource const& lhs, Resource const& rhs)  {
			return (lhs <= rhs) && (rhs <= lhs);
			bool equal = true;
			for (MsgCIter m(rhs.msgList_.begin()); m != rhs.msgList_.end(); ++m) {
				equal &= lhs.find(*m);
			}
			return equal;
		}
	private:
		bool operator <=(Resource const& rhs) const {
			bool equal = true;
			for (MsgCIter m(msgList_.begin()); m != msgList_.end(); ++m) {
				equal &= rhs.find(*m);
			}
			return equal;
		}
		bool find(system::message::MessageBase const& msg) const {
			for (MsgCIter m(msgList_.begin()); m != msgList_.end(); ++m) { if (*m == msg) return true;}
			return false;
		}
	private:
		MsgList msgList_;
		system::module::Interfaces interfaceId_;
	};

} // namespace interface
} // namespace precitec


#endif /*INTERFACE_H_*/
