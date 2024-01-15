#ifndef TEST_SERVER_HANDLER_H_
#define TEST_SERVER_HANDLER_H_

#include <iostream>
#include <string> // std::string

#include "server/handler.h"
#include "message/testServer.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{


	/**
	 * TTestServer<MsgHandler> erhaelt ueber die Basisklasse Server<MsgHandler>
	 * die Message in den Passenden Handler gelegt. Dazu muessen die Handler
	 * als Callbacks (Member)-Funktionspointer) mit der Messagenummer in einer
	 * Callback-Tabelle abgelegt werden. Dies erledigt die init-Routine
	 * die einzelnen Callback, die sinnvollerweise so heissen, wie ihre server implementierungen
	 *
	 */
	template <>
	class TTestServer<MsgHandler> : public Server<MsgHandler> {
	public:
		MSG_HANDLER(TTestServer);
//	public: \
//		/** Std Kontruktor ohne Protokoll */
//		TTestServer(TTestServer<AbstractInterface>* server)
//		: Server<MsgHandler>(TTestServer<Messages>().info),
//			server_(server), dispatcher_(numMessages()) {
//			dispatcher_.clearCallbackList(numMessages(), Callback(&TTestServer<MsgHandler>::defaultHandler));
//		}
//		/** Kontruktor mit hartkodiertem Protokoll (nicht MM gemanaged */
//		TTestServer(TTestServer<AbstractInterface>* server, SmpProtocolInfo &protInfo)
//		: Server<MsgHandler>(TTestServer<Messages>().info), server_(server),
//		dispatcher_(numMessages()) {
//			dispatcher_.clearCallbackList(numMessages(), Callback(&TTestServer<MsgHandler>::defaultHandler));
//			activate(protInfo); /* ab jetzt laeuft der Server, wenn p!=NULL */
//		}
//		~TTestServer() { TTestServer<MsgHandler>::dispose(); }
//	protected:
//		/** handler leitet nur noch an den dispacher weiter */
//		virtual void handle(Receiver & receiver, int i) { dispatcher_.handle(receiver, this, i); }
//		/** Typdefinition der Callbacks, waere obsolet mit richtigen Delegates */
//		typedef void (TTestServer<MsgHandler>::*Callback) (Receiver &);
//	private:
//		/** Typdefinition der Callbacks, wandert in Server mit richtigen Delegates */
//		void defaultHandler(Receiver&) { std::cout << "testLocal TTestServer <MsgHandler>::defaultHandler" << std::endl;	}
//		/** wir brauchen natuerlich einen Localcall-Server, der die Arbeit erledigt */
//		TTestServer<AbstractInterface> 			*server_;
//		/** erledigt das Callbackmanagement */
//		Dispatcher<TTestServer<MsgHandler> >  dispatcher_;




		// damit die MessageLoop in der Basisklasse weiss was sie tun muss, wird hier fuer
		// jede Message ein Callback registriert, der weiter unten definiert wird
		void registerCallbacks() {
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER2(TTestServer, add, int, int );
			REGISTER4(TTestServer, compare, int, int, float, float);
			REGISTER0(TTestServer, output);
			REGISTER1(TTestServer, copy, PvString);
			REGISTER2(TTestServer, cat, PvString, PvString);
			REGISTER2(TTestServer, multiplyField, Field, int);
			REGISTER1(TTestServer, getField, int);
			REGISTER1(TTestServer, sendField, Field);
		}

		// Die Callbacks haben eine andere Signatur (keine Argumente) als das Interface
		// sie greifen au
 		void CALLBACK2(add, int, int)(Receiver &receiver) {
			//std::cout << "RHandler::add(";
			// das Demarshalling erfogt auf dieser Ebene, da wir hier die Argumentvariablen brauchen
			// um sie and die Implementation-Funktion (server_.add) weitergeben zu koennen
			// mit einer Delegate-Klasse, die Member-Funktions-Pointer zu verschiedenen Klassen
			// halten kann, kann dieses Interface etwas vereinfacht werden

			// die Reihenfolge der Aufrufe ist wichtig!!!!! arg0, arg1, arg2, ...
			// die Namen der Argumente sind beliebig
			int a; receiver.deMarshal(a);
			int b; receiver.deMarshal(b);
			//std::cout << a << ", " << b << ")" << std::endl;
			// die eigntliche Server-Funktion (add) wird aufgerufen; der Returnwert verpackt
			receiver.marshal(server_->add(a, b));

			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		void CALLBACK4(compare, int, int, float, float)(Receiver &receiver) {
			//std::cout << "RHandler::compare(" << std::endl;
			//TestServer_Messages::MESSAGE_NAME4(compare, int, int, float, float) msg(sendBuffer, replyBuffer);
			int   arg0; receiver.deMarshal(arg0);
			int   arg1; receiver.deMarshal(arg1);
			float arg2; receiver.deMarshal(arg2);
			float arg3; receiver.deMarshal(arg3);
			//std::cout << arg0 << ", " << arg1 << ", " << arg2 << ", " << arg3 << ")" << std::endl;
			//int ret = server_->compare(arg0, arg1, arg2, arg3);
			//receiver.marshal(ret);
			receiver.marshal(server_->compare(arg0, arg1, arg2, arg3));
			receiver.reply();
		}

		void CALLBACK0(output)(Receiver &receiver) {
			//std::cout << "RHandler::output()" << std::endl;
			// hier koennen wir uns das Marshaln und Demarshaln sparen
			server_->output();
			receiver.reply();
		}


		void CALLBACK1(copy, PvString)(Receiver &receiver) {
			//std::cout << "RHandler::copy(";
			PvString arg0; receiver.deMarshal(arg0);
			//std::cout << arg0 << ")" << std::endl;
			// hier koennen wir us das deMarshaln sparen, muessen aber die Argument entmarshalen
			PvString ret = server_->copy(arg0);
			receiver.marshal(ret);
			receiver.reply();
		}

		void CALLBACK2(cat, PvString, PvString)(Receiver &receiver) {
			//std::cout << "RHandler:::cat(";
			PvString arg0; receiver.deMarshal(arg0);
			PvString arg1; receiver.deMarshal(arg1);
			//std::cout << arg0 << ", " << arg1 << ")" << std::endl;
			// hier koennen wir us das deMarshaln sparen
			//TestServer_Messages::MESSAGE_NAME0(output) *msg(sendBuffer, replyBuffer);
			// das marshal koennten wir us sparen, aber der Befehl, kommt mit void-argumenten zurecht

			//std::cout << "handleRemote:cat" << arg0 << ":" << arg1 << std::endl;
			PvString ret = server_->cat(arg0, arg1);
			receiver.marshal(ret);
			receiver.reply();
		}

		void CALLBACK2(multiplyField, Field, int)(Receiver &receiver)	{
			Field f; receiver.deMarshal(f);
			int		factor; receiver.deMarshal(factor);
			Field ret(server_->multiplyField(f, factor));
			receiver.marshal(ret);
			receiver.reply();
		}

		void CALLBACK1(getField, int)(Receiver &receiver)	{
			int size; receiver.deMarshal(size);
			Field ret(server_->getField(size));
			receiver.marshal(ret);
			receiver.reply();
		}

		void CALLBACK1(sendField, Field)(Receiver &receiver)	{
			//std::cout << "testServer[MsgHandler]::sendField( " << ")" << std::endl;
			Field f; receiver.deMarshal(f);
			server_->sendField(f);
			receiver.reply();
			//std::cout << "testServer[MsgHandler]::sendField() ok" << std::endl;
		}

	}; // class TTestServer<MsgHandler>


} // interface
} // precitec


#endif /*TEST_SERVER_HANDLER_H_*/
