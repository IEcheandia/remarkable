#ifndef EVENT_TEST_SERVER_H
#define EVENT_TEST_SERVER_H

#include <string> // std::string

#include  "server/interface.h"
#include  "event/eventTest.interface.h"
#include  "module/interfaces.h" // wg appId


/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
namespace interface
{
	/**
	 * TEventTest ist eine primitive TEventTest-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TEventTest<EventServer> : public TEventTest<AbstractInterface> 
	{
	public:
		TEventTest() {}
		virtual ~TEventTest() {}
	public:
		virtual void trigger() {
			std::cout << "TEventTest<EventServer>::trigger()" << std::endl;
		}

		virtual void trigger(int a) {
			std::cout << "TEventTest<EventServer>::trigger(" << a << ")" << std::endl;
		}

		virtual void add(int a, int b, int sum) {
			std::cout << "TEventTest<EventServer>::add(" << a << ", " << b << " = " << sum << (sum==a+b ? ", ok)" : ", nok") << std::endl;
		}

		virtual void iota(Field const& f, int factor) {
			bool ok = f.testIota(factor);
			std::cout << "TEventTest<EventServer>::iota(" << "f" << ",  " << factor << (ok ? " ok )" : " nok)") << std::endl;
		}
	};

	/**
	 * TEventTest ist eine primitive TEventTest-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
/*
	class AlTEventTestServer : public TEventTest<AbstractInterface> 
	{
	public:
		AlTEventTestServer() {}
		virtual ~AlTEventTestServer() {}
	public:
		virtual void trigger() {
			std::cout << "AltEventTestServer::trigger()" << std::endl;
		}

		virtual void add(int a, int b) {
			std::cout << "AltEventTestServer::add(" << a << ", " << b << ")" << std::endl;
		}

		virtual void print(PvString const& string) {
			std::cout << "AltEventTestServer::print(" << string << ")" << std::endl;
		}
		virtual void blob(XVector const& v) {
			std::cout << "AltEventTest<EventServer>::blob(" << "v" << ") = " << (v.testIota() ? "ok" : "nok" ) << std::endl;
		}
	};
*/
} // namespace system
} // namespace precitec

#endif /*EVENT_TEST_SERVER_H*/
