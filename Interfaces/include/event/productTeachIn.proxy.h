#ifndef PRODUCTTEACHIN_PROXY_H_
#define PRODUCTTEACHIN_PROXY_H_


#include "server/eventProxy.h"
#include "event/productTeachIn.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TProductTeachIn<EventProxy> : public Server<EventProxy>, public TProductTeachIn<AbstractInterface>, public TProductTeachInMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TProductTeachIn() : EVENT_PROXY_CTOR(TProductTeachIn), TProductTeachIn<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TProductTeachIn() {}

	public:

		virtual void start(int seamSeries, int seam)
		{
			INIT_EVENT(Start);
			//signaler().initMessage(Msg::index);
			signaler().marshal(seamSeries);
			signaler().marshal(seam);
			signaler().send();
		}

		virtual void end(system::Timer::Time duration)
		{
			INIT_EVENT(End);
			//signaler().initMessage(Msg::index);
			signaler().marshal(duration);
			signaler().send();
		}

		virtual void startAutomatic(int code)
		{
			INIT_EVENT(StartAutomatic);
			//signaler().initMessage(Msg::index);
			signaler().marshal(code);
			signaler().send();
		}

		virtual void stopAutomatic()
		{
			INIT_EVENT(StopAutomatic);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}
	};

} // interface
} // precitec


#endif /*PRODUCTTEACHIN_PROXY_H_*/
