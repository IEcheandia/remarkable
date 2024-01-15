#ifndef EVENT_TEST_PROXY_H
#define EVENT_TEST_PROXY_H

#include "server/eventProxy.h"
#include "event/eventTest.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TEventTest<EventProxy> : public Server<EventProxy>, public TEventTest<AbstractInterface>
	{

	public:
		/// beide Basisklassen muessen geeignet initialisiert werden
//		TEventTest() : EVENT_PROXY_CTOR(TEventTest), TEventTest<AbstractInterface>()
//		{
//			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
//		}
		TEventTest() : Server<EventProxy>(TEventTest<Messages>().info), TEventTest<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

	public:
		virtual void add(int a, int b, int sum)
		{
			//std::cout << "TEventTest<EventProxy>::add(" << a << ", " << b << " = " << sum << ")" << std::endl;
			INIT_EVENT3(TEventTest, add, int, int, int);
			//std::cout << "TEventTest<EventProxy>::add(" << a << ", " << b << " = " << sum << ") msg inited" << std::endl;
			//signaler().initMessage(Msg::index);
			signaler().marshal(a);
			signaler().marshal(b);
			signaler().marshal(sum);
			//std::cout << "TEventTest<EventProxy>::add(" << a << ", " << b << " = " << sum << ") marshalled" << std::endl;
			signaler().send();
			//std::cout << "TEventTest<EventProxy>::add(" << a << ", " << b << " = " << sum << ") sent" << std::endl;
		}

		virtual void trigger()
		{
			//std::cout << "TEventTest<EventProxy>::trigger()" << std::endl;
			INIT_EVENT0(TEventTest, trigger);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}

		virtual void trigger(int a)
		{
			//std::cout << "TEventTest<EventProxy>::trigger(" << a << ")" << std::endl;
			INIT_EVENT1(TEventTest, trigger, int);
			//std::cout << "TEventTest<EventProxy>::trigger()" << a << " msg inited" << std::endl;
			//signaler().initMessage(Msg::index);
			//std::cout << "TEventTest<EventProxy>::trigger() marshalling" << std::endl;
			signaler().marshal(a);
			//std::cout << "TEventTest<EventProxy>::trigger()" << a << " marshalled" << std::endl;
			signaler().send();
			//std::cout << "TEventTest<EventProxy>::trigger() sent" << std::endl;
		}

		virtual void iota(Field const& f, int factor)
		{
			std::cout << "TEventTest<EventProxy>::iota(" << "f" << ")" << std::endl;
			INIT_EVENT2(TEventTest, iota, Field, int);
			//signaler().initMessage(Msg::index);
			signaler().marshal(f);
			signaler().marshal(factor);
			signaler().send();
		}

	}; // class TEventTest<EventProxy>

} // namespace interface
} // namespace precitec

#endif /*EVENT_TEST_PROXY_H*/
