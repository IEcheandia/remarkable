#ifndef MESSAGERECEIVER_H_
#define MESSAGERECEIVER_H_


#include "system/types.h"
#include "Poco/SharedPtr.h"
#include "protocol/protocol.h"
#include "message/message.h"
#include "message/messageBuffer.h"
#include "message/derivedMessageBuffer.h"
#include "message/serializer.h"

#include <iostream>
#include <sys/types.h> // wg fifo
#include <sys/stat.h>  // wg fifo

namespace precitec
{
namespace system
{
namespace message
{
	//class ReceiverHolder;
	class Receiver {
	public:
		friend class ReceiverHolder;
		/// wird fuer debug-Zwecke benoetigt
		Receiver() : protocolManager_(NULL) { /*std::cout << "Receiver::CTor() nur für DEBUG" << std::endl;*/ }
#if defined __QNX__ || defined __linux__
		/// nur fuer Qnx-Event-Receiver wg Qnx-Pulse
		Receiver(std::string p_oName, int maxElementSize, SmpProtocol & protocol, SharedMem &shMem)
		: protocolManager_(protocol),
//			sendBuffer_(new SharedMessageBuff(shMem, maxElementSize)),
			replyBuffer_(0),
			sharedMem_(&shMem) {
			//std::cout << "Receiver CTor0: " << int(sharedMem_->begin()) << std::endl;
			sendBuffer_= new SharedMessageBuff(p_oName, shMem, maxElementSize);
		}
#endif // __QNX__||__linux__
		/// CTor ohne Protokol, das mit setProtocol nachgeliefert wird
		Receiver(int sBuffSize, int rBufSize)
		: protocolManager_(NULL),
			sendBuffer_(new StaticMessageBuffer(sBuffSize)),
			replyBuffer_(new StaticMessageBuffer(rBufSize)),
			sharedMem_(NULL) {
			//std::cout << "Receiver::CTor1(" << sBuffSize << ", " << rBufSize << ")" << std::endl;
		}
		/// Das Protokoll muss uebergeben werden, da jedes Protokoll anders initialisiert wird
		Receiver(int sBuffSize, int rBufSize, SmpProtocol & protocol)
		: protocolManager_(protocol),
			sendBuffer_(new StaticMessageBuffer(sBuffSize, protocol->protocolType()==Qnx)),
			replyBuffer_(new StaticMessageBuffer(rBufSize, protocol->protocolType()==Qnx)),
			sharedMem_(NULL) {
			//std::cout << "Receiver::CTor(" << sBuffSize << ", " << rBufSize << ")" << std::endl;
			if (!protocolManager_.isNull()) { protocolManager_->initReceiver(); }
		}
		~Receiver() {
			if (sendBuffer_) 	delete sendBuffer_;
			if (replyBuffer_) delete replyBuffer_;
			std::cout<<"DTor Receiver..."<<std::endl;
			//if (sharedMem_) 	delete sharedMem_;
		}

	public:
/*
		// gibt zusaetzlich noch die replyId wg Reply und Pulse-Bestimmung
	 	int getMessageHeader() {
	 		// da binaer geschrieben wird ist der Header ok und muss nicht entpackt werden
	 		protocolManager().getMessageHeader(sendBuffer.header());
	 		return header_.messageNum;
	 	}
*/
		MessageBuffer * sendBuffer() { return sendBuffer_; }
		MessageBuffer * replyBuffer() { return replyBuffer_;  }
		void dumpSendBuffer() { if (sendBuffer_) sendBuffer_->dump(); }
		void dumpReplyBuffer() { if (replyBuffer_) replyBuffer_->dump();  }
	 	int getMessage() {
			sendBuffer_->clear();
			int num = protocolManager_->getMessage(*sendBuffer_);
			//if (num>=0)
				//std::cout << "receiver::getMessage" << " @ " << std::hex << int(sendBuffer_->rawData()) << std::dec << std::endl;
			// dies erlaubt beim Sender eine minimale Empfangskontrolle
			//replyBuffer_.header().messageNum = num;
			return  num;
		}
	 	int getEventMessage() {
			//std::cout << "receiver::getEventMessage" << std::endl;
			sendBuffer_->clear();
			return  protocolManager_->getPulse(*sendBuffer_);
		}
	 	/// fuer QNX-Events wird der allokierte ShMem-Puffer wieder freigegeben
	 	void messageHandled() {
	 		//std::cout << "receiver::messageHandled" << std::endl;
			if (sendBuffer_!=NULL)	sendBuffer_->freeBuffer();
	 	}

		/**
		 * in marshal/deMarshal wird zur Typinformation des zu serialisierenden Wertes der
		 * Transfertyp (IntraProcess, InterProcess, Global) mitgeliefert. Dieser Typ ist aus der
		 * ProtocolInfo zu erlesen (spaeter einmal eigene Variable z.Zt: Qnx=InterProcess, Rest=Global)
		 * @param value
		 */
		template <class T>
	 	void deMarshal(T & value) {
			Serializer<T, FindSerializer<T, Single>::Value> ::deserialize( *sendBuffer_, value );
		}

		template <class T>
	 	void deMarshal(T & value, int length) {
			Serializer<T, FindSerializer<T, Vector>::Value> ::deserialize( *sendBuffer_, value, length );
		}

		template <class T>
	 	void marshal(T const& value) {
			Serializer<T, FindSerializer<T, Single>::Value> ::serialize( *replyBuffer_, value );
		}

		template <class T>
	 	void marshal(T const& value, int length) {
		 	Serializer<T, FindSerializer<T, Vector>::Value> ::serialize( *replyBuffer_, value, length);
		}

	 	void reply() {
	 		if (!replyBuffer_) return;
	 		replyBuffer_->setMsgSize();
			protocolManager_->reply(*replyBuffer_);
			replyBuffer_->clear();
			//std::cout << "receiver::reply reply sent" << std::endl;
		}

	 	void nullReply() {
			//std::cout << "Receiver nullReply:" << std::endl;
	 		protocolManager_->nullReply(*replyBuffer_);
 		}

		/// bisher nur fuer die ShutdownMessage benoertigt ???? vllt gibt's 'ne bessere Loesung
		SmpProtocol 	protocol() const { return protocolManager_; }
		/// Ausgabe wg Nice-Class
	  friend  std::ostream &operator <<(std::ostream &os, Receiver const& r);

	  private:
		//Protocol 	&protocolManager() { return *protocolManager_; }
	private:
		void setProtocol(SmpProtocol & protocol) {
			if (protocol.isNull()) {
				std::cout << "Receiver setProtocol is Null" << std::endl;
				throw;
			}
			/// ggf Flag setzen, dass ShMemPtr anders bahandelt wrden sollen
			dynamic_cast<StaticMessageBuffer*>(sendBuffer_)->setShMemAccess(protocol->protocolType()==Qnx);
			dynamic_cast<StaticMessageBuffer*>(replyBuffer_)->setShMemAccess(protocol->protocolType()==Qnx);
			//std::cout << "Sender protocol init" << std::endl;
			// altes Protokoll wird ggf. vernichtet
//			if (protocolManager_) {
//				// wir implizieren hier den Willen des Programmierers, auf jeden Fall das alte
//				// Protokoll abzumelden, egal ob ein legales neues geliefert wird
//				delete protocolManager_;
//				protocolManager_ = NULL;
//			}
			protocolManager_ = protocol;
			protocolManager_->initReceiver();
		}
//		void setProtocol(Protocol *protocol) {
//			//std::cout << "Sender protocol init" << std::endl;
//			protocolManager_=protocol; // altes Protokoll wird ggf. von SharedPtr vernichtet
//			if (!protocolManager_.isNull()) protocolManager_->initReceiver();
//		}
	private:
		/// Receiver kopiert Info aus Smp und killt Protokoll wenn noetig
		SmpProtocol		protocolManager_;
		//Protocol		 *protocolManager_; // cache
		MessageBuffer *sendBuffer_;
		MessageBuffer	*replyBuffer_;
		SharedMem 		*sharedMem_;
	}; // Receiver

  inline std::ostream &operator <<(std::ostream &os, Receiver const& r) {
		os << "Receiver: ";
		if (!r.protocolManager_.isNull()) {
			os << r.protocolManager_->protocolInfo() << "[->" << r.sendBuffer_ << ":<-" << r.replyBuffer_ << "]";
		} else {
			os << " NULL ";
		}
		return os;
	}

	/**
	 * Diese Klasse erlaubt es transparent im Event-Handler-Callback nach Aufruf
	 * der Server-Funktion eine Aufraeum-Aktion zu starten. Insbesondre ist dies
	 * fuer das QNX-Protokoll noetig, da hier der SharedMem-Block freigegeben werden
	 * muss.
//	 */
//	class ReceiverHolder {
//	public:
//		ReceiverHolder(Receiver &r) : r_(r) {}
//		~ReceiverHolder() {
//			//std::cout << "ReceiverHolder: releasing Buffer " << r_.sendBuffer_ << std::endl;
//			/*r_.messageHandled(); */r_.sendBuffer_->releaseAccess();}
//		operator Receiver() { r_.sendBuffer_->getAccess(); return r_; }
//		Receiver *operator ->() { return &r_; }
//	private:
//		Receiver &r_;
//	};









#if defined __QNX__ || defined __linux__

  /**
   * (neue) noch nicht verwendeter Ersatz der Receiverklasse fuer Events(Pulse)
   */
	class SigObserver {
	public:
		/// wird fuer debug-Zwecke benoetigt
		SigObserver() : protocolManager_(NULL) { std::cout << "SigObserver::CTor() nur fuer DEBUG" << std::endl; }

		/// nur fuer Qnx-Event-SigObserver wg Qnx-Pulse
		SigObserver(std::string p_oName, int bufSize, SmpProtocol & protocol, SharedMem &shMem)
		: protocolManager_(protocol),
			sendBuffer_(new SharedMessageBuff(p_oName, shMem, bufSize)),
			sharedMem_(&shMem)/*,
			maxMessageSize_(bufSize) */{
		}
		~SigObserver() {
			if (sendBuffer_) 	delete sendBuffer_;
			if (sharedMem_) 	delete sharedMem_;
		}

	public:
		void dumpSendBuffer() { if (sendBuffer_) sendBuffer_->dump(); }
	 	int getMessage() {
			//std::cout << "SigObserver::getMessage" << std::endl;
			sendBuffer_->clear();
			int num = protocolManager_->getPulse(*sendBuffer_);
			return  num;
		}

		/**
		 * in marshal/deMarshal wird zur Typinformation des zu serialisierenden Wertes der
		 * Transfertyp (IntraProcess, InterProcess, Global) mitgeliefert. Dieser Typ ist aus der
		 * ProtocolInfo zu erlesen (spaeter einmal eigene Variable z.Zt: Qnx=InterProcess, Rest=Global)
		 * @param value
		 */
		template <class T>
	 	void deMarshal(T & value) {
			Serializer<T, FindSerializer<T, Single>::Value> ::deserialize( *sendBuffer_, value );
		}

		template <class T>
	 	void deMarshal(T & value, int length) {
			Serializer<T, FindSerializer<T, Vector>::Value> ::deserialize( *sendBuffer_, value, length );
		}

		/// bisher nur fuer die ShutdownMessage benoertigt ???? vllt gibt's 'ne bessere Loesung
		SmpProtocol 	protocol() const { return protocolManager_; }
		/// Ausgabe wg Nice-Class
	  friend  std::ostream &operator <<(std::ostream &os, SigObserver const& r);

	  private:
		//Protocol 	&protocolManager() { return *protocolManager_; }
	private:
		/// SigObserver kopiert Info aus Smp und killt Protokoll wenn noetig
		SmpProtocol		protocolManager_;
		//Protocol		 *protocolManager_; // cache
		MessageBuffer *sendBuffer_;
		SharedMem 		*sharedMem_;
		//int 					 maxMessageSize_;
	}; // SigObserver

  inline std::ostream &operator <<(std::ostream &os, SigObserver const& r) {
		os << "SigObserver: ";
		if (!r.protocolManager_.isNull()) {
			os << r.protocolManager_->protocolInfo() << "[->" << r.sendBuffer_ << "]";
		} else {
			os << " NULL ";
		}
		return os;
	}
#endif // __QNX__||__linux__

} // namespace message
} // namespace system
} // namespace precitec

#endif /*MESSAGERECEIVER_H_*/
