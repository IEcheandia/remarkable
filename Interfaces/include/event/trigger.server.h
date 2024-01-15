#ifndef TRIGGER_SERVER_H_
#define TRIGGER_SERVER_H_


#include "event/trigger.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TTrigger<EventServer> : public TTrigger<AbstractInterface>
	{
	public:
		TTrigger(){}
		virtual ~TTrigger() {}
	public:
		// triggert ein Bild/Messung
		virtual void trigger(TriggerContext const& context) {}
		// triggert N Bilder/Messsungen ( Bildnummer)
		virtual void trigger(TriggerContext const& context, TriggerInterval const& interval) {}

		// setzt tiggerMode
		virtual void triggerMode(int triggerMode) {}

		// setzt tiggerMode
		virtual void triggerStop(int flag) {}

	};


} // interface
} // precitec


#endif /*TRIGGER_SERVER_H_*/
