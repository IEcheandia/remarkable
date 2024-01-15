#ifndef PROTOCOLHANDLER_H_
#define PROTOCOLHANDLER_H_

#include "SystemManifest.h"

#include "Poco/Thread.h" // wg Pid (wg Prio)
#include "server/interface.h"
#include "protocol/protocol.info.h"
#include "message/messageReceiver.h"
#include "system/shMemRingAllocator.h"
#include "system/sharedMem.h"
#include "system/realTimeSupport.h"

namespace precitec
{
	namespace interface
	{
		using system::message::Receiver;
		using system::message::MessageBase;
		using system::message::ShMemRingAllocator;
		using system::message::SmpProtocol;
		using system::message::SmpProtocolInfo;
		using system::message::ProtocolInfo;
		using system::message::ProtocolType;
		using system::message::Qnx;
		using system::SharedMem;


		class ProtocolHandler;
		/**
		 * ThreadStorage haelt Variablen die fuer die protokollspezifischen
		 * EventHandler gedacht sind.
		 * mainthread, pHandler koennen hier vermutlich entfallen !??!?
		 * protocol und sharedMem: Die Strukturen  werden beide im Thread 
		 * nicht veraendert.
		 */
		class EventThreadStorage {
		public:
			EventThreadStorage() :
			  mainThread(NULL),
			  pHandler(NULL),
			  protocol(),
			  sharedMem(),
				  systemRunning(false)
			  {}
			~EventThreadStorage() {
				//std::cout << "EventThreadStorage::dTor: trying to delete protocol" << std::endl;
				try {
					protocol = SmpProtocol(); // wir erzwingen die Destruktion (sofern keine andere Referenz vorliegt)
				} catch (...) {
					std::cout << "EventThreadStorage::dTor: failed: Exception!!!" << std::endl;
				}
				//std::cout << "EventThreadStorage::dTor: ok" << std::endl;
			}
			  /**
			   * da eventLoop eine statische funktione st, braucht sie einen this-Ptr
			   * um auf Member-Variablen zugreifen zu koennen
				 */
			  Server<EventHandler> *mainThread;
			  /// hierueber wird er Handler der abgeleiteten Klassen aufgerufen
			  ProtocolHandler 	*pHandler;
			  /// hiermit ist die Gegenstelle erreichbar
			  SmpProtocol		protocol;
			  /// fuer Qnx-Protokol
			  SharedMem 			sharedMem;

			  ///	pro Handler ein Receiver (Puffer), gemeinsames Protokoll; gesynct
			  //Receiver		 		 *receiver;
			  /// stop-Flag
			  bool							systemRunning;
			  int pid;
		};

		/**
		 * ProtocolHandler handles Events for one specific ProtocolEach protocol has
		 * its own Receivers and thus Buffers.
		 * The main work is done in the messageLoop in a separate thread.
		 */
		class SYSTEM_API ProtocolHandler {
		public:
			/// legt ThreaadStorages und Mutexe an
			ProtocolHandler(MessageInfo const& info, Server<EventHandler> *handler, ProtocolType type);
			/// loescht die Threadstorages
			~ProtocolHandler();
		public:

			/// hier wird auf die Messages gewartet und die Handler gestartet
			static void messageLoop(void * data);

			/// sett die Daten fuer die handler-Threads auf
			//void initThreadStorage(int sendBuf, int ReplyBuf, SmpProtocol & p);
			void initThreadStorage(SmpProtocolInfo & pInfo, PvString const& shMemName);
			void systemRunning(bool isRunning) {
				Poco::ScopedLock<Poco::FastMutex> lock( systemRunningMutex_ );
				systemRunning_ = isRunning;
				//std::cout << "ThreadStatus:" << systemRunning_ << std::endl;
			}
			/// convenience-Zugriff auf Flag
			bool isStopped() {
				Poco::ScopedLock<Poco::FastMutex> lock( systemRunningMutex_ );
				//std::cout << "EventThreadStorage::isStopped(): " << !systemRunning_ << std::endl;
				return !systemRunning_;
			}
			void start();
			/// beendet die Message-Loop, wartet ggf. 100 ms darauf
			void stop();
			/// geordneter Ausstieg
			virtual void waitForServerEnd();
			/// wg Debugausgabe
			ProtocolInfo const& protocolInfo() const { return threadStorage_.protocol->protocolInfo(); }
			std::string interfaceName() const { return info().interfaceName(); }

            void setRealTimePriority(system::Priority priority)
            {
                m_priority = priority;
            }

		private:
			MessageInfo const& info() const { return info_; }
			// hier stehen die wesentlichen Daten zum Interface (das Original steht im eventHandler)
			/// Accessor fuer info wg Debug-Meldungen
			MessageInfo info_; ///< Achtung! Referenz klappt wg MultiThread nicht. wieso ??????
			/// the handler has the callback-table, via this pointer it is referenced
			Server<EventHandler>    *eHandler_;
			/// Thread-Wrapper um Messageloop
			Poco::Thread 						messageLoopThread_;
			/// aktueller Systemzustand
			bool										systemRunning_;
			/// schuetzt gewuenschten Systemzustand, wird nur in Scoped-Locks verwendet
			Poco::FastMutex 				systemRunningMutex_;
			///	laeuft MsgLooop?
			Poco::Semaphore 				messageLoopActive_;
			/// die Verwaltungsinfo fuer jeden einzelnen Thread (koennte TLS werden)
			EventThreadStorage			threadStorage_; // im <userserver>.interface.h definiert, wieviele Handler es gibt
			/// Debug-Info
			ProtocolType						type_;
            system::Priority m_priority = system::Priority::None;
		}; // ProtocolHandler


	} // namespace interface
} // namespace precitec


#endif /*PROTOCOLHANDLER_H_*/
