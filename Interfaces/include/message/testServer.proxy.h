#ifndef TEST_SERVER_PROXY_H_
#define TEST_SERVER_PROXY_H_

#include "server/proxy.h"
#include "message/testServer.interface.h"
#include "message/testServer.proxy.h"

namespace precitec
{
namespace interface
{

	/**
	 *  die Remote-Call-Spezialisierung ist recht simpel
	 * Fuer jede Member-Funktion des Basis-Servers gibt es eine
	 * Funktion mit gleicher Signatur.
	 * Nach initMessage wird jeder Parameter gemarshalled (in die Message verpackt)
	 * und dann mit send abgeschickt. Fuer nicht-void-funktionen wird der Returnparameter
	 * demarshalled (aus der Message entpackt) unr zurueckgegeben.
	 * Dass dies alles von Hand passiert hat den Vorteil, dass dieser Code
	 * ggf. Optimierungen bzw. Spezialcode fuer In-Out-Parameter enthalten kann.
	 * Dies den Kompiler automatisch machen zu lassen, ist vermutlich moeglich, wuerde aber
	 * sehr viel undurchsichtigen Template-Macro-Metaprogrammier-Code enthalten.
	 */
	template <>
	class TTestServer<MsgProxy> : public Server<MsgProxy>, public TTestServer<AbstractInterface>	{
	public:
		/// beide Basisklassen muessen geeignet initialisiert werden
		TTestServer() : PROXY_CTOR(TTestServer), TTestServer<AbstractInterface>()
		{
			//std::cout << "remote CTor::Server<Proxy> ohne Protokoll" << std::endl;
		}
		/// normalerweise wird das Protokoll gleich mitgeliefert
		TTestServer(SmpProtocolInfo &p) : PROXY_CTOR1(TTestServer,  p), TTestServer<AbstractInterface>()
		{
			//std::cout << "remote CTor::Module<Proxy> " << std::endl;
		}
		/// der DTor muss virtuell sein
		virtual ~TTestServer() {}

	public:

		/// addiert zwei Zahlen
		int add(int a, int b)
		{
			// Von der Message brauchen wir nur den Typ, damitwir auf den Index
			// eine statische Membervariable zugriefen koennen
			INIT_MESSAGE2(TTestServer, add, int, int);

			//std::cout << "RCaller::add(" << a << ", " << b << ")" << std::endl;
			// von der Message brauchen wir nur den index
			sender().marshal(a);
			sender().marshal(b);
			sender().send();
			int ret;	sender().deMarshal(ret);
			return ret;
		}

		/// erzeugt einen bool
		bool compare(int a, int b, float c, float d)
		{
			//std::cout << "RCaller::compare( " << a << ", " << b << ", " << c << ", " << d << ") " << std::endl;
			INIT_MESSAGE4(TTestServer, compare, int, int, float, float);
			sender().marshal(a);
			sender().marshal(b);
			sender().marshal(c);
			sender().marshal(d);
			sender().send();
			bool ret; sender().deMarshal(ret);
			return ret;
		}

		/// gibt lokal einen String aus
		void output(void)
		{
			//std::cout << "RCaller::output () " << std::endl;
			INIT_MESSAGE0(TTestServer, output);
			sender().send();
		}

		/// gibt lokal einen String aus
		PvString cat(PvString const& s, PvString const& t)
		{
			//std::cout << "remoteCall:cat(" << s << ", " << t << ")" << std::endl;
			INIT_MESSAGE2(TTestServer, cat, PvString, PvString);
			sender().marshal(s);
			sender().marshal(t);
			sender().send();
			PvString ret;	sender().deMarshal(ret);
			return ret;
		}
		/// gibt lokal einen PvString aus
		PvString copy(PvString const& s)
		{
			//std::cout << "remoteCall:copy( " << s << ")" << std::endl;
			INIT_MESSAGE1(TTestServer, copy, PvString);
			sender().marshal(s);
			sender().send();
			PvString ret;	sender().deMarshal(ret);
			return ret;
		}
		virtual Field multiplyField(Field const& f, int factor)
		{
			//std::cout << "remoteCall:multiplyField( "  << ")" << std::endl;
			INIT_MESSAGE2(TTestServer, multiplyField, Field, int);
			sender().marshal(f);
			sender().marshal(factor);
			sender().send();
			Field ret;	sender().deMarshal(ret);
			//std::cout << "remoteCall:multiplyField demarshalled" << std::endl;
			return ret;
		}
		virtual Field getField(int size) {
			//std::cout << "remoteCall:getField( "  << ")" << std::endl;
			INIT_MESSAGE1(TTestServer, getField, int);
			sender().marshal(size);
			sender().send();
			Field ret;	sender().deMarshal(ret);
			//std::cout << "remoteCall:getField( "  << ") ok" << std::endl;
			return ret;
		}
		virtual void sendField(Field const& f) {
			//std::cout << "testServer[MsgProxy]::sendField( "  << ")" << std::endl;
			INIT_MESSAGE1(TTestServer, sendField, Field);
			sender().marshal(f);
			sender().send();
			//std::cout << "testServer[MsgProxy]::sendField( "  << ") ok" << std::endl;
		}

	};

} // interface
} // precitec


#endif /*TEST_SERVER_PROXY_H_*/
