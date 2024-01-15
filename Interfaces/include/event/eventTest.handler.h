#ifndef EVENT_TEST_HANDLER_H
#define EVENT_TEST_HANDLER_H

#include <iostream>
#include <string> // std::string

#include "server/eventHandler.h"
#include "event/eventTest.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TEventTest<EventHandler> : public Server<EventHandler> {
	public:
		EVENT_HANDLER( TEventTest );
		/* Std-Kontruktor ohne Protokoll, das eigenstaendig (setProtocol(p) ) gesetzt wird */
/*
		TEventTest(TEventTest<AbstractInterface> *server) : Server<EventHandler>(TEventTest<Messages>().info),
		server_(server), eventDispatcher_(numMessages()) {
			eventDispatcher_.clearCallbackList(numMessages(), EventCallback(&TEventTest<EventHandler>::defaultHandler));
		}
		virtual ~TEventTest() { Server<EventHandler>::stop(); }
		virtual SmpProtocolInfo activate(SmpProtocolInfo &serverInfo) {
			SmpProtocolInfo clientInfo = Server<EventHandler>::activateProtocol(serverInfo);
			registerCallbacks();
			start(serverInfo->type());	///* Messageloop fuer dieses Protokoll starten
			return clientInfo;
		}
	protected:
		///* die private Server-Klasse ruft hiermit typ-richtig die Callbacks, mit richtigen Delegates ist das obsolet
		virtual void handle(Receiver & receiver, int i ) {
			//std::cout << "testLocal TEventTest<EventHandler>::handle event " << i << std::endl;
			eventDispatcher_.handle(receiver, this, i);
		}
	private:
		///* wir brauchen natuerlich einen Implementation-Server, der die Arbeit erledigt
		TEventTest<AbstractInterface> *server_;
		///** Typdefinition der Callbacks, waere obsolet mit richtigen Delegates
		typedef void (TEventTest<EventHandler>::*EventCallback) (Receiver);
		///* Typdefinition der Callbacks, wandert in Server mit richtigen Delegates
		void defaultHandler(Receiver) {
			 std::cout << "testLocal TEventTest<EventHandler>::defaultHandler" << std::endl;
		}
		EventDispatcher<TEventTest<EventHandler> >  eventDispatcher_;
*/

	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT0(TEventTest, trigger );
			REGISTER_EVENT1(TEventTest, trigger, int );
			REGISTER_EVENT3(TEventTest, add, int, int, int);
			REGISTER_EVENT2(TEventTest, iota, Field, int);
		}

		void CALLBACK0(trigger)(Receiver &receiver)
		{
			//std::cout << "TEventTest<EventHandler>::trigger()" << std::endl;
			server_->trigger();
		}

		void CALLBACK1(trigger, int)(Receiver &receiver)
		{
			//std::cout << "TEventTest<EventHandler>::trigger" << std::endl;
			int a; receiver.deMarshal(a);
			//std::cout << "TEventTest<EventHandler>::trigger(" << a << ")" << std::endl;
			server_->trigger(a);
		}

		void CALLBACK3(add, int, int, int)(Receiver &receiver)
		{
			//std::cout << "TEventTest<EventHandler>::add" << std::endl;
			int a; receiver.deMarshal(a);
			int b; receiver.deMarshal(b);
			int s; receiver.deMarshal(s);
			//std::cout << "TEventTest<EventHandler>::add(" << a << ", " << b << " = " << s << ")" << std::endl;
			server_->add(a, b, s);
		}

		void CALLBACK2(iota, Field, int)(Receiver &receiver)
		{
			Field f; receiver.deMarshal(f);
			int factor; receiver.deMarshal(factor);
			//std::cout << "TEventTest<EventHandler>::iota(" << f << ")" << std::endl;
			server_->iota(f, factor);
		}

	};

} // namespace system
} // namespace precitec

#endif /*EVENT_TEST_HANDLER_H*/
