#ifndef TRIGGER_HANDLER_H_
#define TRIGGER_HANDLER_H_


#include <iostream>
#include "event/trigger.h"
#include "event/trigger.interface.h"
#include "server/eventHandler.h"

namespace precitec
{
namespace interface
{
	using namespace  message;

	template <>
	class TTrigger<EventHandler> : public Server<EventHandler>, public TTriggerMessageDefinition
	{
	public:
		EVENT_HANDLER( TTrigger );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(Trigger, trigger);
            REGISTER_EVENT(TriggerWithInterval, trigger2);
			REGISTER_EVENT(TriggerMode, triggerMode);
			REGISTER_EVENT(TriggerStop, triggerStop);
		}

		void trigger(Receiver &receiver)
		{
			TriggerContext context; receiver.deMarshal(context);
			server_->trigger(context);
		}

		void trigger2(Receiver &receiver)
		{
#if !defined(NDEBUG)
			wmLog(eDebug, "TTrigger<EventHandler>::trigger2\n");
#endif
			TriggerContext context; receiver.deMarshal(context);
			TriggerInterval	interval; receiver.deMarshal(interval);
			server_->trigger(context, interval);
		}

		void triggerMode(Receiver &receiver)
				{
		#if !defined(NDEBUG)
					wmLog(eDebug, "TTrigger<EventHandler>::triggerMode\n");
		#endif
					int mode; receiver.deMarshal(mode);
					server_->triggerMode(mode);
				}

		void triggerStop(Receiver &receiver)
				{
		#if !defined(NDEBUG)
				wmLog(eDebug, "TTrigger<EventHandler>::triggerStop\n");
		#endif
					int flag; receiver.deMarshal(flag);
					server_->triggerStop(flag);
				}
	};


} // namespace interface
} // namespace precitec


#endif /*TRIGGER_HANDLER_H_*/
