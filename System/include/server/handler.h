#ifndef HANDLER_H_
#define HANDLER_H_

#include <iostream>


#include "Poco/SharedPtr.h"
#include "Poco/Thread.h"
#include "Poco/Semaphore.h"
#include "Poco/Mutex.h"
#include "Poco/NamedMutex.h"

#include "protocol/protocol.h"
#include "message/messageMacros.h"
#include "message/messageReceiver.h"
#include "message/messageSender.h"
#include "server/interface.h"
#include "server/msgDispatcher.h"
#include "system/realTimeSupport.h"
#include "message/messageReceiver.h"			// wird von den abgeleiteten Handler benoetigt: (de)Marshal

namespace precitec
{

namespace system
{
namespace message
{
			// Vorwaertsdeklarationen muessen im richtigen Namespace erfolgen
			class Receiver;
			struct MessageBase;
} // message
} // system

	using system::message::Receiver;
	using system::message::MessageBase;
	using system::message::SmpProtocol;
	using system::message::SmpProtocolInfo;

namespace interface
{

	typedef Poco::SharedPtr<Poco::FastMutex> SmpNamedMutex;

	/**
	* Im Threadstoreage gibt es read-only Variablen und gesyncte Variablen
	* Die Synchronisation erfolgt ueber die Mutexe: messageReceived und handlerFree
	* MsgLoop darf schreiben, wenn sie handlerFree geschnappt hat
	* Handler darf schribeen, wenn er messageReceived geschnappt hat
	*/
	struct ThreadStorage {
		ThreadStorage() :
			messageReceived(0, 1),
			handlerFree(1, 1),
			msgNum(0),
			receiver(NULL)
			{}
		/// der mainThread (die Serverklasse + abgeleitete Klassen ueber virtuelle Funktionen) r/o
		Server<MsgHandler> *mainThread;
		/// Thread-Wrapper um HandlerThreads r/o
		Poco::Thread 			handlerThread;

		/// mit dieser Mutex wird die message an die Handler weitergegeben
		Poco::Semaphore 	messageReceived;
		/// mit dieser Mutex werden freie Handler gefunden
		Poco::Semaphore 	handlerFree;
		/// msgNum bestimmt die CallbackFunktion; gesynct
		int								msgNum;
		///	pro Handler ein Receiver (puffer), gemeinsames Protokoll; gesynct
		Receiver		 		 *receiver;
	}; // ThreadStorage


	/**
	* Die Basisklasse fuer alle Remote-Hanlder. Man beachte das der Remote-Handler
	* lokal vorliegt, lediglich die Messages des Remote-Client akzeptiert.
	* Im Moment ist die Klasse leider ziemlich ausgeduennt worden, da es Schwierigkeiten gibt,
	* die jeweils richtigen  Routinen der konkreten Kindklasse aufzurufen.
	*/
	template <>
	class SYSTEM_API Server<MsgHandler> {
	public:
		// std-CTor MM setzt Protokoll spaeter
		Server(MessageInfo const& info);
		// Spezial-Version, fuer Handller die nicht vom MM gemanagt werden
		Server(MessageInfo const& info, SmpProtocolInfo & p);

		virtual ~Server() {
				try	{
                    if (threadStorage_.receiver)
                    {
                        delete threadStorage_.receiver;
                    }
				}	catch (...)	{
					std::cout << "caught exception from killing threadstorage: " << std::endl;
					// nix tun, hier crasht die Thread-Lib ???? muss noch untersucht werden
				}
			}

	public:
		/// erzeugt Receiver aus Protocol und  startet die Messageloop
		SmpProtocolInfo activate(SmpProtocolInfo & p);
		/// == activate mit existierendem Protokoll
		void start();
		/// beendet die Message-Loop, wartet ggf. 100 ms darauf
		void stop();

		int numMessages() const { return info_.numMessages; }
		/// quit wird vom MM aufgerufen, gibt, die Mutex frei, um Ende des Moduls einzuleiten
		//void quit() { /*serverMutex_.unlock();*/ }

        void setRealTimePriority(system::Priority priority)
        {
            m_priority = priority;
        }

	public:
		/// ueber diese Referenz kann auf Message-Konstanten zugegriffen werden -> privat
		MessageInfo 	info_;
		/// accessor fuer info_
		MessageInfo 	const& info() const { return info_; }
		/// sollte eigentlich nie aufgerufen werden,
		virtual void handle(Receiver &, int ) { std::cout << "Server<MsgHandler>::handle() should never be called" << std::endl; }
		/// die abgeleitete Klasse wartet hiermit auf das Ende des Server und leitet DTor ein
		virtual void waitForServerEnd(); // wird u.a. von module.server.cpp::waitForAllServers aufgerufen
	protected:
		/// beim start des Handlers wird die Callbackliste (des dispatchers der abgeleiteten Klasse) initialisiert
		virtual void registerCallbacks() {}
		/// protected Accessor fuer privaten Member
		//virtual Receiver &receiver() const = 0;
		/// stoppt alles, wird vom Destruktor der abgeleiteten Klasse aufgerufen, solange die noch lebt
		void dispose() {	if (!isStopped()) stop(); }
	private:
		/// setzt die Daten fuer die handler-Threads auf
		void initThreadStorage(int sendBuf, int ReplyBuf, SmpProtocol & p);
		/// setze system-Flag
		void systemRunning(bool isRunning) { systemRunning_ = isRunning; }
		/// convenience-Zugriff auf Flag
		bool isStopped() { return !systemRunning_; }

		/// hier wird auf die Messages gewartet und die Handler gestartet
		static void messageLoop(void * data);
		/// die Handler rufen die Callbacks auf
		static void handlerThread(void * data);
		/// Ausgabe wg Nice-Class
		friend  std::ostream &operator <<(std::ostream &os, Server<MsgHandler> const& h);
	private:
		//ccqed

		// das janze Sync-Zeugs

		ThreadStorage			threadStorage_;
		/// ueber diese Seamphore kann man auf die Einsatzereitschaft der Handler warten
		Poco::Semaphore 	activeHandlers_;

		/// Thread-Wrapper um Messageloop
		Poco::Thread 			messageLoopThread_;
		/// gewuenschter Systemzustand
		bool							systemRunning_;
		/// schuetzt gewuenschten Systemzustand
		Poco::FastMutex 	systemRunningMutex_;
		///	laeuft MsgLooop
		Poco::Semaphore 	messageLoopActive_;

        system::Priority m_priority = system::Priority::None;
	}; // Server<MsgHandler>

	inline std::ostream &operator <<(std::ostream &os, Server<MsgHandler> const& h) {
		os << "Server<MsgHandler>: " << h.info_.name();
        os << " : "<< *h.threadStorage_.receiver;
		return os;
	}


} // namespace interface
} // namespace precitec

#endif /*HANDLER_H_*/
