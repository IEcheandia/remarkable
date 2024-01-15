#ifndef MSGDISPATCHER_H_
#define MSGDISPATCHER_H_

#include "message/messageReceiver.h"

namespace precitec
{
	using system::message::Receiver;
namespace interface
{
	/**
	 * Der Dispatcher uebernimmt die Verwaltung der Callbacks
	 * und den eigentlichen Aufruf derselben.
	 *
	 * Das Problem ist. Die Ablauflogik ist hier immer die gleiche. Ergo
	 * sollte der Code in den Handler-Basisklasen sein. Aber da die Callbacks
	 * Funktionen der Abgeleiteten Userklassen sind, die die basisklasse nicht kennt.
	 * Muss dieser Kode in die abgeleiteten Userklassen.
	 * Um die Komplexitaet gering zu halten, wird der ganze Code in eine Template-Klasse (Policy)
	 * gesteckt.
	 * TODO moeglicherweise kann mit einer Pimpl-Implementierung weiterer Kode in die Serverklasse
	 * gesteckt werden.
	 */
	template < class Handler>
	class Dispatcher
	{
		typedef void (Handler::*Callback) (Receiver& r);

	public:
		/// Typdefinition der Callbacks, waere obsolet mit richtigen Delegates
		//typedef void (Handler::*Callback) ();

	public:
		Dispatcher(int numMessages)	: callbackList_(numMessages)	{}
		~Dispatcher() {}
	public:
		/// die Server-Klasse ruft hiermit typrichtig die Callbacks, mit richtigen Delegates waere das obsolet
		void handle(Receiver& receiver, Handler* sClass, int i) { (sClass->*callbackList_[i])(receiver);	}

		/// fuer alle Messages DefaultHandler in Liste eintragen
		void clearCallbackList(int numMessages, Callback defaultHandler) {
			for(int msg=0; msg<numMessages; msg++ ) {
				callbackList_[msg] = defaultHandler;
			}
		}

		/// add callBack to List, used by Register-Macros in init()
		void addCallback(int msgNum, Callback callback) {
			//std::cout << "Server<MsgHandler>::addCallback" << msgNum << std::endl;
			callbackList_[msgNum] = callback;
		}
	private:
		/// hier werden vom abgeleiteten Server die Callbacks eingetragen
		std::vector<Callback> callbackList_;
	};

	//using system::message::ReceiverHolder;

	template < class EventHandler>
	class EventDispatcher {
		// ??? Achtung!!! Redundant in MessageMacros schon mal definiert
		typedef void (EventHandler::*EventCallback) (Receiver &r);

	public:
		/// Typdefinition der Callbacks, waere obsolet mit richtigen Delegates
		//typedef void (Handler::*Callback) ();

	public:
		EventDispatcher(int numMessages)	: callbackList_(numMessages)	{}
		~EventDispatcher() {}
	public:
		/// die Server-Klasse ruft hiermit typrichtig die Callbacks, mit richtigen Delegates waere das obsolet
		void handle(Receiver& receiver, EventHandler* sClass, int i) { (sClass->*callbackList_[i])(receiver);	}

		/// fuer alle Messages DefaultHandler in Liste eintragen
		void clearCallbackList(int numMessages, EventCallback defaultHandler) {
			for(int event=0; event<numMessages; event++ ) {
				callbackList_[event] = defaultHandler;
			}
		}

		/// add callBack to List, used by Register-Macros in init()
		void addCallback(int eventNum, EventCallback callback) { callbackList_[eventNum] = callback; }
	private:
		/// hier werden vom abgeleiteten Server die Callbacks eingetragen
		std::vector<EventCallback> callbackList_;
	};


/*
	template < class EventHandler>
	class DispatcherEvent
	{
		typedef void (EventHandler::*Callback) (Receiver& r);

	public:
		/// Typdefinition der Callbacks, waere obsolet mit richtigen Delegates
		//typedef void (Handler::*Callback) ();

	public:
		DispatcherEvent(int numMessages)	: callbackList_(numMessages)	{}
	public:
		/// die Server-Klasse ruft hiermit typrichtig die Callbacks, mit richtigen Delegates waere das obsolet
		void handle(Receiver& receiver, EventHandler* sClass, int i) { (sClass->*callbackList_[i])(receiver);	}

		/// fuer alle Messages DefaultHandler in Liste eintragen
		void clearCallbackList(int numMessages, Callback defaultHandler) {
			for(int event=0; event<numMessages; event++ ) {
				callbackList_[event] = defaultHandler;
			}
		}

		/// add callBack to List, used by Register-Macros in init()
		void addCallback(int eventNum, Callback callback) {
			//std::cout << "Server<MsgHandler>::addCallback" << eventNum << std::endl;
			callbackList_[eventNum] = callback;
		}
	private:
		/// hier werden vom abgeleiteten Server die Callbacks eingetragen
		std::vector<Callback> callbackList_;
	};
*/


} // namespace interface
} // namespace precitec

#endif /*MSGDISPATCHER_H_*/
