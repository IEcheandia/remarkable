#ifndef PROTOCOL_QNX_MSG_H_
#define PROTOCOL_QNX_MSG_H_

#if defined __QNX__ || defined __linux__
	#if defined __QNX__
		#include <sys/neutrino.h>
		#include <sys/netmgr.h> // ND_LOCAL_NODE
	#endif
#include <sched.h> // getprio()

#include "message/message.h"
#include "message/derivedMessageBuffer.h"
#include "protocol/protocol.h"
#include "protocol/process.info.h"
#include "message/messageReceiver.h"
#include "message/messageException.h"
#include <sched.h>
#include "Poco/Process.h"
#include "Poco/Thread.h" // pid()
#include <errno.h>
//#include "module/moduleLogger.h" // debug

#if defined __linux__
    // IPC via SIMPL
    #include "Poco/UUIDGenerator.h"
    #include "Poco/UUID.h"
    #include "simplipc/simpl.h"
    #include "simplipc/simplProto.h"
    #define  MSG_LENGTH_SIMPL_PULSE    20
#endif

namespace precitec
{
namespace system
{
namespace message
{
#if defined __linux__
	#define _PULSE_CODE_MINAVAIL 0
	#define _PULSE_CODE_MAXAVAIL 127
	struct _pulse {
		uint16_t		type;
		uint16_t		subtype;
		int8_t			code;
		uint8_t			zero[3];
		union sigval	value;
		int32_t			scoid;
	};
#endif

	/**
	 * SystemEvents (analog TimouOutMessage) (es gibt bisher keine)
	 * werden auf  -MaxReservedUserEvents.. -1 gemappt
	 */
	enum { MaxReservedUserEvents = 5 };

	/**
	 *  Diese Klasse stellt einer Verbindung zu einem Messagereceiver her. Das
	 *  geschieht ueber den Namen. Es genuegt wenn Sender und Receiver den selben
	 *  Namen tragen. Weiter muss sichergestellt sein, dass der Receiver (Server)
	 *  schon erzeugt wurde.
	 *  Die Prozesse des Senders und Receivers muessen auf dem gleichen Rechner sein.
	 */
	class QnxProtocol : public Protocol {

	public:
		/// Dieser CTor wird normalerweise nur ueber die Fabrikfkt create aufgerufen
		QnxProtocol( ProcessInfo& info);
		/// mehr an Aufraeumen sollte nicht noetig sien
		~QnxProtocol() {
#if defined __QNX__
			ConnectDetach(conId_);
#endif
#if  defined __linux__
            // IPC via SIMPL
			if (name_detach(_simpl_myStuff) == -1)
			{
				printf("QnxProtocol::DTor: cannot detach name: %s\n", whatsMyError());
				//exit(-1);
			}
#endif
		}

	public:
		/// Die node-info wird als Kriterium herangezogen
		virtual bool isValid() { return processInfo_.isValid(); }
		/// ueber diese Funktion kann via Basis-Ptr auf die Info-Strukturen der abgeleiteten Klassen  zugegriffen werden
		virtual ProtocolInfo const& protocolInfo()  const { return processInfo_; }
		/// gibt an, ob Komm. global ist: false -> ShMem kann vrrwendet werden;  ShMemPtr werden als POD behandelt
		//bool isGlobal() { return false; }

	public:
		// Sender-Interface
		virtual void initSender();
		void send(MessageBuffer &sendBuffer, MessageBuffer &replyBuffer);
		void sendPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId=module::Client);
        void sendQuitPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId=module::Client) override;

	public:
		// Receiver-Interface
		/**
		 * fuer das Qnx-Protokoll findet die initialisierung im CTor statt, da
		 * Info ueber die ProzessInfo an den Client weitergegeben wird
		 */
		virtual void initReceiver() {}
		/// liest Message-Daten in Puffer
		int getMessage(MessageBuffer &sendBuffer);
		/// nur fuer Messages
		void reply(MessageBuffer &replyBuffer);
		/// Event-Interface
		int getPulse(MessageBuffer &rcvBuffer/*, PChar shMemBase*/);
	private:
#if defined __QNX__
		_msg_info info_; 		///< Puffer um erweiterte MsgInfos aufzunehmen, wird nur lokal verwendet
#endif
		int				replyId_;	///< Rueckadresse beim Messages (nur Receiver)
		int				conId_;		///< die Verbindung, ueber die gesendet wird (nur Sender)
		/// wenn true, ist das Protokoll definiert und kann eine Verbindung versuchen
		bool						initialized_;	/// Verbindung zu Gegenstelle ist gegeben
#if defined __linux__
		// IPC via SIMPL
		int m_oReceiverFd;
		char *m_pSender;
		WHO_AM_I _simpl_myStuff;
		int64_t _simpl_sender_shmid;
#endif
	private:
		/// hiermit (ConnectionId, ProcessId) wird eine Verbindung aufbgebaut
		ProcessInfo			processInfo_; // noetig???? ggf. für reconnect??
	};

	inline QnxProtocol::QnxProtocol( ProcessInfo& info) : Protocol(Qnx), initialized_(false) {
		//std::cout << "QnxProtocol(" << info << ")" << " on proc: " << Poco::Process::id() << std::endl;
#if defined __linux__
		// IPC via SIMPL
		strcpy(_simpl_myStuff.whom, "");
		_simpl_myStuff.pid = -1;
		_simpl_myStuff.fd = -1;
		_simpl_myStuff.y_fd = -1;
		_simpl_myStuff.shmid = -1;
		_simpl_myStuff.shmPtr = NULL;
		_simpl_myStuff.shmSize = 0;
		_simpl_myStuff.myExit = NULL;
#endif

		if (info.isServerInfo()) {
			//std::cout << "QnxProtocol::CTor ... creating Server-Protocol ";
			initialized_ = true;
			// Server-Initialisierung
#if defined __QNX__
			// Flags fuer ChannelCreate_r
				int flags = 	_NTO_CHF_REPLY_LEN		// include replylen in info-struct
										| _NTO_CHF_SENDER_LEN		// include sendlen  in info-struct
										| _NTO_CHF_UNBLOCK;			// verhindert einen speziellen Sync-Fehler
#endif
			// die ProcessInfo wird hier gefuellt und spaeter an der Client weitergegeben
			try {
#if defined __QNX__
				// IPC via QNX IPC
				Poco::UUIDGenerator& oUUIDGenerator = Poco::UUIDGenerator::defaultGenerator();
				Poco::UUID oUUID(oUUIDGenerator.createRandom());
				PvString oUUIDStrg = oUUID.toString();

				info = ProcessInfo(ChannelCreate(flags), Poco::Process::id(), info.node(), oUUIDStrg, info.shMemName());
#endif
#if defined __linux__
				// IPC via SIMPL
				Poco::UUIDGenerator& oUUIDGenerator = Poco::UUIDGenerator::defaultGenerator();
				Poco::UUID oUUID(oUUIDGenerator.createRandom());
				PvString oUUIDStrg = oUUID.toString();

				if (name_attach(oUUIDStrg.c_str(), NULL, _simpl_myStuff) == -1)
				{
					printf("QnxProtocol::CTor: cannot attach name: %s\n", whatsMyError());
					//exit(-1);
				}
				info = ProcessInfo(0, Poco::Process::id(), info.node(), oUUIDStrg, info.shMemName());
#endif
				//std::printf("TID %i - AFTER ChannelCreate to chanid %i returned:\n", Thread::currentTid(), info.channelId());
				//std::cout << "ProcessInfo: " << info << "\n";
			} catch (...) {
				std::cout << "QnxProtocol::CTor Caught unknown exception: rethrowing" << std::endl;
				throw;
			}
			//std::cout << " -> " << info << std::endl;
		} else { // Client verlangt in Info gueltigen Channel von Server
			//std::cout << "QnxProtocol::CTor ... creating Client-Protocol: " << info << std::endl;
			// client-CTor
			//initSender();
			//std::cout << "\t" << "p::ch: " << std::hex << processInfo_.processId() << " :: " << processInfo_.channelId()<< std::endl;
		}
		processInfo_ = info;
	}

	inline void QnxProtocol::initSender()	{
		//std::cout << "QnxProtocol::initSender attempting " << isValid() << " : " << initialized_<< " " <<processInfo_ << std::endl;
		if (isValid() && !initialized_) {
			//std::cout << "QnxProtocol::initSender " << processInfo_ << std::endl;
			//conId_ = ConnectAttach_r(node, pId, channel, _NTO_SIDE_CHANNEL, 0);
#if defined __QNX__
			// IPC via QNX IPC
			conId_ = ConnectAttach_r(processInfo_.node(), processInfo_.processId(), processInfo_.channelId(), _NTO_SIDE_CHANNEL, 0);
			//std::printf("TID %i - AFTER ConnectAttach_r to chanid %i returned conID: %i\n", Thread::currentTid(), processInfo_.channelId(), conId_);
			if (conId_<0) {
				std::cout << "QnxProtocol::initSender: ConnectAttach failed: " << -conId_ << std::endl;
				switch (-conId_) {
				case EAGAIN: throw MessageException("error: all connections in use");
					break;
				case ESRCH : throw MessageException("error: problem with node, pid, channel");
					break;
				case ENXIO : throw MessageException("error: procces no longer exists");
					break;
				default:
					break;
				}
			} else if (conId_==0) {
				std::cout << "QnxProtocol::initSender failed conId = " << std::endl;
			} // if conId
#endif
#if defined __linux__
			// IPC via SIMPL
			Poco::UUIDGenerator& oUUIDGenerator = Poco::UUIDGenerator::defaultGenerator();
			Poco::UUID oUUID(oUUIDGenerator.createRandom());
			PvString oUUIDStrg = oUUID.toString();

			if (name_attach(oUUIDStrg.c_str(), NULL, _simpl_myStuff) == -1)
			{
				printf("QnxProtocol::initSender: cannot attach name: %s\n", whatsMyError());
				//exit(-1);
			}

			PvString oServerUUIDStrg = processInfo_.serverUUIDStrg();

			m_oReceiverFd = name_locate(oServerUUIDStrg.c_str(), _simpl_myStuff);
			if (m_oReceiverFd == -1)
			{
				printf("QnxProtocol::initSender: cannot locate receiver: %s\n", whatsMyError());
				//exit(-1);
			}
#endif
			//std::cout << "QnxProto(sender): attached to " << std::hex << processInfo_.processId() << " : " << processInfo_.channelId() << " to make " << conId_ << std::dec <<std::endl;
		} // if isValid
	}

	inline int QnxProtocol::getMessage(MessageBuffer &rcvBuffer) {
		//std::cout << "QnxProtocol::getMessage " <<  processInfo_  << " "  << isValid() << std::endl;
		if (!isValid()) { return TimeoutMessage; }
		try {
#if defined __QNX__
			// IPC via QNX IPC
			//std::cout << "QnxProtocol::getMessage receiving from " <<  processInfo_  << std::endl;
			replyId_ = MsgReceive_r(processInfo_.channelId(), rcvBuffer.rawData(), rcvBuffer.limMsgSize(), &info_);
			//wmLog(eDebug, "TID %i - AFTER MsgReceive_r on channel id: %i returned receive id: %i. conid %i\n", Thread::currentTid(), processInfo_.channelId(), replyId_, conId_);
			if (replyId_<0) {
				//std::cout << "QnxProtocol::getMessage "  << processInfo_ << " " << -replyId_ << std::endl;
				//std::cout << "QnxProtocol::getMessage: replyId_" << -replyId_ << std::endl;
				throw MessageException("Qnx: getMessage failed");
			} else if (replyId_==0) {
				//std::cout << "getMessage received pulse on channel " << processInfo_.channelId() << std::endl;
				//std::cout << "QnxProtocol::getMessage received Buff: "; rcvBuffer.dump<int>(std::cout); std::cout << std::endl;
				return rcvBuffer.messageNum();
				//return TimeoutMessage;
			}
#endif
#if defined __linux__
			// IPC via SIMPL
			//printf("getMessage: Receive\n");
			int retValue = Receive(&m_pSender, (void *)rcvBuffer.rawData(), rcvBuffer.limMsgSize(), _simpl_sender_shmid, _simpl_myStuff);
			//printf("getMessage: retValue: %d\n", retValue);
			if (retValue < -1)
			{
				// Proxy (Pulse) received
				printf("getMessage: Pulse received, habe aber keinen Pulse erwartet !\n");
				replyId_ = 0;
				//std::cout << "getMessage received pulse on channel " << processInfo_.channelId() << std::endl;
				//std::cout << "QnxProtocol::getMessage received Buff: "; rcvBuffer.dump<int>(std::cout); std::cout << std::endl;
				return rcvBuffer.messageNum();
				//return TimeoutMessage;
			}
			else if (retValue == -1)
			{
				printf("getMessage: Receive error: %s\n", whatsMyError());
				//exit(-1);
				replyId_ = -1;
				//std::cout << "QnxProtocol::getMessage "  << processInfo_ << " " << -replyId_ << std::endl;
				//std::cout << "QnxProtocol::getMessage: replyId_" << -replyId_ << std::endl;
				throw MessageException("Qnx: getMessage failed");
			}
			else
			{
				replyId_ = 1;
			}
#endif

			//std::cout << "rcvBuffer.msgSize(): " << rcvBuffer.msgSize() << std::endl;
			//std::cout << "QnxProtocol::getMessage received Buff: "; rcvBuffer.dump<int>(std::cout); std::cout << std::endl;
			//std::cout << "received " << info_.msglen-sizeof(Header) << " Bytes" << std::endl;

			// stellt den Puffer-Cursor wieder auf den Anfang
			rcvBuffer.rewind();

		} catch (MessageException&) {
			throw;
		} catch (...) {
			std::cout << "QnxProtocol::getMessage caught Exception in getMessage: " << rcvBuffer.header().messageNum
								<< " " << processInfo_ << std::endl;
			Poco::Thread::sleep(1000);
		}
			//rcvBuffer.dump<int>(std::cout);
		return rcvBuffer.messageNum();
   }

	inline int QnxProtocol::getPulse(MessageBuffer &rcvBuffer/*, PChar shMemBase*/) {
//		struct _pulse {
//		    uint16_t                    type;
//		    uint16_t                    subtype;
//		    int8_t                      code;
//		    uint8_t                     zero[3];
//		    union sigval                value;
//		    int32_t                     scoid;
//		};
		typedef struct _pulse Pulse;
		Pulse pulse;
		//std::cout << "QnxProtocol::getPulse " <<  processInfo_  << " "  << isValid() << std::endl;
		if (!isValid()) { return TimeoutMessage; }
		try {

			//std::cout << "receiving from " <<  processInfo_  << std::endl;
#if defined __QNX__
			// IPC via QNX IPC
			replyId_ = MsgReceivePulse_r(processInfo_.channelId(), &pulse, sizeof(pulse), NULL);
			//wmLog(eDebug, "TID %i - AFTER MsgReceivePulse_r on channel id: %i returned receive id: %i. conid %i\n", Thread::currentTid(), processInfo_.channelId(), replyId_, conId_);
			//std::cout << "getPulse::received pulse from " <<  processInfo_
			//					<< " Code:" << int(pulse.code)-(_PULSE_CODE_MINAVAIL)  << std::endl;
#endif
#if defined __linux__
			// IPC via SIMPL
			pulse.code = 0;
			pulse.value.sival_int = 0;
			unsigned char oData[MSG_LENGTH_SIMPL_PULSE];
			//printf("getPulse: Receive\n");
			int retValue = Receive(&m_pSender, &oData, MSG_LENGTH_SIMPL_PULSE, _simpl_sender_shmid, _simpl_myStuff);
			//printf("getPulse: retValue: %d\n", retValue);
			if (retValue < -1)
			{
				// Proxy (Pulse) received
				pulse.code = 0; // wird nicht verwendet !?
				pulse.value.sival_int = returnProxy(retValue);
			}
			else if (retValue == -1)
			{
				printf("getPulse: Receive error: %s\n", whatsMyError());
				//exit(-1);
			}
			else
			{
				printf("getPulse: Message received, habe aber keine Message erwartet !\n");
			}
#endif
			// ist Pulse-Code zu Gross
			if (pulse.code > _PULSE_CODE_MAXAVAIL) { return TimeoutMessage; }
			// MsgNum wird beim senden in Intervall _PULSE_CODE_MAXAVAIL .. _PULSE_CODE_MINAVAIL geschoben
			int msgNum = int(pulse.code) - (_PULSE_CODE_MINAVAIL);
			// ist msg legal
			if (msgNum<0) { return TimeoutMessage; }

			// dann ist pulse.value der Pointer ins SharedMemory ( = Offset von SharedMemory-Base )
			SharedMessageBuff &msgBuff(*dynamic_cast<SharedMessageBuff*>(&rcvBuffer));
			// mit setToRead setzen wird den Start-Pointer des Puffers auf die uebergebene Startadresse
			msgBuff.offsetToBuffer(pulse.value.sival_int);
			//std::cout << "getPulse got shMemBuffer: #" << msgNum << " " << pulse.value.sival_int << " " << int(msgBuff.rawData()) << std::endl;
			// jetzt koennen wir aus rcvPuffer lesen (wie bei andren Protokollen

			//std::cout << "rcvBuffer.msgSize(): " << rcvBuffer.msgSize() << std::endl;
			//std::cout << "receivePulse: "; msgBuff.dump<int>(std::cout); std::cout << std::endl;
			//std::cout << "receivePulse: "; rcvBuffer.dump<int>(std::cout); std::cout << std::endl;
			//std::cout << "received " << info_.msglen-sizeof(Header) << " Bytes" << std::endl;

			// stellt den Puffer-Cursor wieder auf den Anfang
			rcvBuffer.rewind();

		} catch (MessageException&) {
			std::cout << "caught MsgException in getPulse: " << rcvBuffer.header().messageNum
					<< " " << processInfo_ << std::endl;
			throw;
		} catch (...) {
			std::cout << "caught Exception in getPulse: " << rcvBuffer.header().messageNum
								<< " " << processInfo_ << std::endl;
			Poco::Thread::sleep(1000);
		}
			//rcvBuffer.dump<int>(std::cout);
		return rcvBuffer.messageNum();
   }


	inline void QnxProtocol::reply(MessageBuffer &replyBuffer) {
		//std::cout << "qnx-protocol reply:" << replyId_ << std::endl;
		if (replyId_>0) {
			//replyBuffer.setCheckSum();
		// nur auf Messages wird geantwortet (nicht auf Pulse)
			//const int status = 0; // frei belegbar, ergibt den Sende-Returnwert
			//std::cout << "replying: " << replyBuffer.msgSize() << " Bytes " << std::endl;
			//replyBuffer.dump<int>(std::cout);
			//std::cout << "reply: "; replyBuffer.dump<int>(std::cout); std::cout << std::endl;
			//wmLog(eDebug, "TID %i - BEFORE MsgReply_r to receiver Id_: %i on channel %i conid %i\n", Thread::currentTid(), replyId_, processInfo_.channelId(), conId_);
#if defined __QNX__
			// IPC via QNX IPC
			int error = MsgReply_r( replyId_, status, replyBuffer.rawData(), sizeof(Header)+replyBuffer.dataSize());
			if (error<0) {
				std::cout << "QnxProtocol::reply: error" << -error << " " << processInfo_ << std::endl;
				throw MessageException("message-reply failed");
			}
#endif
#if defined __linux__
			// IPC via SIMPL
			int retValue = Reply(m_pSender,
					(void *)replyBuffer.rawData(), sizeof(Header)+replyBuffer.dataSize(),
					_simpl_sender_shmid, _simpl_myStuff);
			if (retValue == -1) {
				std::cout << "QnxProtocol::reply: retValue" << -retValue << " " << processInfo_ << std::endl;
				throw MessageException("message-reply failed");
			}
#endif
		}
	}

	inline void QnxProtocol::send(MessageBuffer &sendBuffer, MessageBuffer &replyBuffer) {
		//std::cout << "QnxProtocol::send attempting " << isValid() << " : " << processInfo_ << std::endl;
		//std::cout << "send: "; sendBuffer.dump<int>(std::cout); std::cout << std::endl;
		if (!isValid()) { return; }
		/// \todo Abfrage auf ReturnTyp = PulseType fehlt noch
		// sendBuffer.header()->multiMsg =false; // wird vllt spaeter relevant
		//std::cout << "QnxProtocol::send sending to " << processInfo_ << " " << sendBuffer.dataSize() << " Bytes" << std::endl;
		//std::cout << "send-ev: "; sendBuffer.dump<int>(std::cout); std::cout << std::endl;
		//wmLog(eDebug, "TID %i - BEFORE MsgSend_r on channel id: %i conid %i\n", Thread::currentTid(), processInfo_.channelId(), conId_);
#if defined __QNX__
		// IPC via QNX IPC
		int status = MsgSend_r(conId_, sendBuffer.rawData(), sizeof(Header)+sendBuffer.dataSize(),
																	 replyBuffer.rawData(), replyBuffer.limMsgSize());
		//std::cout << "send-r: " << status << " " ; replyBuffer.dump<int>(std::cout); std::cout << std::endl;
		if (status<0)	{
			std::cout << "QnxProtocol::send: status " << -status << " " << processInfo_ << std::endl;
			if (-status == 89) {
				std::cout << "Verbindungsfehler: nicht zustande gekommen / unterbrochen" << std::endl;
			}
			throw MessageException("QnxProtocol::send failed");
		}
#endif
#if defined __linux__
		// IPC via SIMPL
		int retValue = Send(m_oReceiverFd,
				(void *)sendBuffer.rawData(), (void *)replyBuffer.rawData(),
				sizeof(Header)+sendBuffer.dataSize(), replyBuffer.limMsgSize(),
				_simpl_myStuff);
		if (retValue == -1)	{
			std::cout << "QnxProtocol::send: retValue " << -retValue << " " << processInfo_ << std::endl;
			//if (-status == 89) {
			//	std::cout << "Verbindungsfehler: nicht zustande gekommen / unterbrochen" << std::endl;
			//}
			throw MessageException("QnxProtocol::send failed");
		}
#endif
		//replyBuffer.dump<int>(std::cout);
		replyBuffer.rewind();
		replyBuffer.setMessageNum(sendBuffer.messageNum());
	}

	inline void QnxProtocol::sendPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId)	{
		//std::cout << "protocol.qnxMsg.h --QnxProtocol::sendPulse attempting " << isValid() << " : " << processInfo_ << std::endl;
		if (!isValid()) { return ; }
		int SizeNull = 100; // ok, so generally 100 != 0, but you never know
		StaticMessageBuffer replyBuffer(SizeNull);
			//std::cout << "sendPulse at shMemBuffer: " << msgBuff.bufferToOffset() << "msgNum: " << sendBuffer.messageNum() << " == " << sendBuffer.messageNum()-_PULSE_CODE_MINAVAIL << std::endl;
			//std::cout << "sendPulse: "; msgBuff.dump<int>(std::cout); std::cout << std::endl;

		//sendBuffer.makePulse();
			//std::cout << "QnxProtocol::sendPulse: " << conId_ << " " << processInfo_ << " " << sendBuffer.dataSize() << " Bytes " << replyBuffer.limMsgSize() << std::endl;
#if defined __QNX__
		// IPC via QNX IPC
		int status = MsgSendPulse_r(conId_, getprio(Poco::Process::id()),
														sendBuffer.messageNum()-_PULSE_CODE_MINAVAIL,  sendBuffer.bufferToOffset());
		if (status<0)	{
			std::cout << "QnxProtocol::sendPulse: status " << -status << std::endl;
			throw MessageException("qnx message sendPulse failed for unknown reason");
		}
#endif
#if defined __linux__
		// IPC via SIMPL
		int retValue = Trigger(m_oReceiverFd, sendBuffer.bufferToOffset(), _simpl_myStuff);
		if (retValue == -1)
		{
			printf("sendPulse: Trigger error: %s\n", whatsMyError());
			//exit(-1);
			std::cout << "QnxProtocol::sendPulse: retValue " << -retValue << std::endl;
			throw MessageException("qnx message sendPulse failed for unknown reason");
		}
#endif
	}

    inline void QnxProtocol::sendQuitPulse(MessageBuffer &sendBuffer, module::Interfaces interfaceId)	{
        if (!isValid()) {
            return ;

        }
#if defined __QNX__
        sendPulse(sendBuffer, interfaceId);
#endif
#if defined __linux__
        // IPC via SIMPL
        const int retValue = Trigger(_simpl_myStuff.fd, sendBuffer.bufferToOffset(), _simpl_myStuff);
        if (retValue == -1)
        {
            printf("sendPulse: Trigger error: %s\n", whatsMyError());
            std::cout << "QnxProtocol::sendPulse: retValue " << -retValue << std::endl;
            throw MessageException("qnx message sendPulse failed for unknown reason");
        }
#endif
    }

	/*
	inline void QnxProtocol::sendPulse(int value)
	{ /// Abfrage auf ReturnTyp = PulseType fehlt noch
		// sendBuffer.header()->multiMsg =false; // wird vllt spaeter relevant
		std::cout << "sending to " << std::hex << conId_ << " " << std::dec << sendBuffer.dataSize() << " Bytes" << std::endl;
		int status = MsgSendPulse_r(conId_, sched_get_priority_min(), _PULSE_CODE_MINAVAIL, value);
		if (status<0)
		{
			throw MessageException("message send failed");
		}
		//replyBuffer.dump<int>(std::cout);
	}
	*/

	template<>
	inline SmpProtocol createTProtocol<Qnx>(SmpProtocolInfo & info) {
		typedef ProcessInfo * PPI;
		//ProcessInfo &pi(*PPI(info.get()));
		//std::cout << "createTProtocol<Qnx>: " << pi << std::endl;
		//return SmpProtocol(new QnxProtocol(pi));
		return SmpProtocol(new QnxProtocol( *PPI(info.get())));
	}

} // namespace message
} // namespace system
} // precitec

#endif // __QNX__||__linux__

#endif /*PROTOCOL_QNX_MSG_H_*/
