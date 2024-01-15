#ifndef MESSAGESENDER_H_
#define MESSAGESENDER_H_

#include <string.h>
#include <vector>
#include <list>
#include <iostream>

#include "Poco/SharedPtr.h"
#include "protocol/protocol.udp.h"
#include "protocol/protocol.h"
#include "message/message.h"
#include "message/serializer.h"

#include <sys/types.h> // wg fifo
#include <sys/stat.h>  // wg fifo

namespace precitec
{
namespace system
{
namespace message
{

	class SYSTEM_API Sender {
	public:
		/// erlaubt Typzugriff auch fuer POD/void-Parameter
		template <class T> struct TypeHolder {
			typedef T Type;
			TypeHolder() {}
		};

		/// CTor ohne Protokoll, wird durch setProtocol nachgeliefert
		Sender() : protocolManager_(NULL), sendBuffer_(NULL), replyBuffer_(NULL) {}
		/// loescht Puffer
	  ~Sender();

	public:
		/// debug utility
		void dumpSendBuffer() { sendBuffer_->dump(); }
		/// debug utility
		void dumpReplyBuffer() { replyBuffer_->dump(); }
		/// fuer den geordneten Rueckzug; sendet
		void sendQuitMessage();
		/// debug utility
	  MessageBuffer &sendBuffer() const { return *sendBuffer_; }
	  MessageBuffer &replyBuffer() const { return *replyBuffer_; }
		/// check if reply is empty because server wasn't ready
		bool isReplyEmpty() { return replyBuffer_->msgSize()==0; }

		void initMessage(int messageNum) {
			// if not ready (protocolManager_.isNull())
			// -> wait for activation-Mutex
			sendBuffer_->clear(); // cursor zuruecksetzen
			sendBuffer_->setMessageNum( messageNum );
		}

		//-----------------------------------------------------------------
		//-----------------------------------------------------------------
		//-----------------------------------------------------------------
		template <class T>
	 	void marshal(T const& value) {
			Serializer<T, FindSerializer<T, Single>::Value> ::serialize( *sendBuffer_, value );
		}

		template <class T>
	 	void marshal(T const& value, int length) {
			Serializer<T, FindSerializer<T, Vector>::Value> ::serialize( *sendBuffer_, value );
		}

		template <class T>
	 	void deMarshal(T & value) {
            if (replyBuffer_->header().messageNum == byte(TimeoutMessage))
            {
                return;
            }
			Serializer<T, FindSerializer<T, Single>::Value> ::deserialize( *replyBuffer_, value );
		}

		template <class T>
	 	void deMarshal(T & value, int length) {
            if (replyBuffer_->header().messageNum == byte(TimeoutMessage))
            {
                return;
            }
			Serializer<T, FindSerializer<T, Vector>::Value> ::deserialize( *replyBuffer_, value );
		}
/*
		template <class T>
	 	T deMarshal(TypeHolder<T>) {
			T value;
	 		Serializer<T, FindSerializer<T>::Value> ::deserialize( replyBuffer_, value );
			return value;
		}

		template <class T>
	 	T deMarshal(TypeHolder<T>, int length) {
			T value;
	 		Serializer<T, FindSerializer<T, true>::Value> ::deserialize( replyBuffer_, value, length );
	 		return value;
		}
*/
		/// Message in Puffer abschicken
	 	int send();
	 	/// Kommunikations-Partner zuweisen
		void setProtocol(SmpProtocolInfo & protInfo, int sendSize, int replySize);

		bool setProtocol(SmpProtocol & protocol, int sendSize, int replySize);

		/// Kommunikation anhalten
		void stop()	{	if (!protocolManager_.isNull())	protocolManager_->stop();	}

		/// Ausgabe wg Nice-Class
	 friend  std::ostream &operator <<(std::ostream &os, Sender const& s);

	private:

		SmpProtocol  	protocolManager_; // das Protokoll muss aus Performance-Gruenden gecacht werden !!!! ????
		MessageBuffer *sendBuffer_;
		MessageBuffer	*replyBuffer_;
	}; // Sender

  inline std::ostream &operator <<(std::ostream &os, Sender const& s) {
		os << "Sender: ";
		if (!s.protocolManager_.isNull()) {
			os << s.protocolManager_->protocolInfo() << "[->" << s.sendBuffer_ << ":<-" << s.replyBuffer_ << "]";
		} else {
			os << " NULL ";
		}
		return os;
	}







  inline Sender::~Sender() {
		if (sendBuffer_) delete sendBuffer_;
		if (replyBuffer_) delete replyBuffer_;
		//if (protocolManager_) delete protocolManager_;
		//std::cout << "Sender::DTor" << std::endl;
	}

  inline int Sender::send() {
 		if (protocolManager_.isNull()) {
 			std::cout << "send has no protocol " << std::endl;
 			return 0; // ???? ShutdownMsg??
 		}
		 	//if (protocolManager_->isBaseProtocol()) { std::cout << "send has base protocol" << std::endl; }
	 	//std::cout << "sender send" << std::endl;
 		// da binaer geschrieben wird ist der Header ok und muss nicht entpackt werden
		sendBuffer_->setMsgSize();
		// checkSumme darf erst berechnet werden, wenn MsgSize stimmt
		//sendBuffer_->setCheckSum();
 		protocolManager_->send(*sendBuffer_, *replyBuffer_);
 		//std::cout << "sender::send reply received" << std::endl;
 		//replyBuffer_.dump<int>(std::cout);
 		return replyBuffer_->header().messageNum; // soll ueberhaupt etwas zurueckgegeben werden???? eher nicht
 	}

 	inline void Sender::setProtocol(SmpProtocolInfo & protInfo, int sendSize, int replySize) {
		//std::cout << "sender::setProtocol()" << *protInfo << std::endl;
		if (protInfo.isNull()) {
			std::cout << "Receiver setProtocol is Null" << std::endl;
			throw;
		}

		bool shMemAccess(protInfo->type()==Qnx);
		sendBuffer_  = new StaticMessageBuffer(sendSize, shMemAccess);
		replyBuffer_ = new StaticMessageBuffer(replySize, shMemAccess);

		SmpProtocol protocol = createProtocol(protInfo);
		protocolManager_=protocol; // altes Protokoll wird ggf. von SharedPtr vernichtet
		protocolManager_->initSender();
		//std::cout << "MessageSender::setProtocol: " << protocolManager_->protocolInfo() << std::endl;
		// -> clear activation-Mutex
	}

	/**
	 * wird ausschliesslich von SendQuitMessage aufgerufen, um HandlerThreads des
	 * gleichen Moduls sauber herunterzufahren
	 */
	inline bool Sender::setProtocol(SmpProtocol & protocol, int sendSize, int replySize) {
		//std::cout << "sender::setProtocol()" << *protInfo << std::endl;
		if (protocol.isNull()) {
			std::cout << "Receiver setProtocol2 is Null" << std::endl;
			// Receiver lebt nicht mehr also nix tun
			return false;
		}
		bool shMemAccess(false); // ShMemPtr brauchen wir fuer die ShutdownMessage nicht
		if (sendBuffer_) delete sendBuffer_;
		sendBuffer_  = new StaticMessageBuffer(sendSize, shMemAccess);

		if (replyBuffer_) delete replyBuffer_;
		replyBuffer_ = new StaticMessageBuffer(replySize, shMemAccess);

		protocolManager_=protocol; // altes Protokoll wird ggf. von SharedPtr vernichtet
		protocolManager_->initSender();
		//std::cout << "MessageSender::setProtocol: " << protocolManager_->protocolInfo() << std::endl;
		// -> clear activation-Mutex
		return true;
	}

	inline void Sender::sendQuitMessage() {
		//std::cout << "sender: sendQuitMessage()" << std::endl;
		initMessage(ShutdownMessage);
		sendBuffer_->reset();
		sendBuffer_->setMsgSize();
		protocolManager_->sendQuitPulse(*sendBuffer_);
	}

} // namespace message
} // namespace system
} // namespace precitec

#endif /*MESSAGESENDER_H_*/
