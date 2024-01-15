#ifndef TRIGGER_PROXY_H_
#define TRIGGER_PROXY_H_

#include <iostream>
#include "server/eventProxy.h"
#include "event/trigger.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TTrigger<EventProxy> : public Server<EventProxy>, public TTrigger<AbstractInterface>, public TTriggerMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TTrigger() : EVENT_PROXY_CTOR(TTrigger), TTrigger<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TTrigger() {}

	public:
		virtual void trigger(TriggerContext const& context)
		{
			INIT_EVENT(Trigger);
			signaler().marshal(context);
			signaler().send();
		}

		virtual void trigger(TriggerContext const& context, TriggerInterval const& interval)
		{
			INIT_EVENT(TriggerWithInterval);
			signaler().marshal(context);
			signaler().marshal(interval);
			signaler().send();
		}

		virtual void triggerMode(int mode)
		{
			INIT_EVENT(TriggerMode);
			signaler().marshal(mode);
			signaler().send();
		}

		virtual void triggerStop(int flag)
		{
			INIT_EVENT(TriggerStop);
			signaler().marshal(flag);
			signaler().send();
		}
	};

} // interface
} // precitec

#endif /*TRIGGER_PROXY_H_*/
