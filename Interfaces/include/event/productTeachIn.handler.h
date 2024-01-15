#ifndef PRODUCTTEACHIN_HANDLER_H_
#define PRODUCTTEACHIN_HANDLER_H_


#include "event/productTeachIn.h"
#include "event/productTeachIn.interface.h"
#include "server/eventHandler.h"

namespace precitec
{
namespace interface
{
	using namespace  message;

	template <>
	class TProductTeachIn<EventHandler> : public Server<EventHandler>, public TProductTeachInMessageDefinition
	{
	public:
		EVENT_HANDLER( TProductTeachIn );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(Start, start);
			REGISTER_EVENT(End, end);
			REGISTER_EVENT(StartAutomatic, startAutomatic);
			REGISTER_EVENT(StopAutomatic, stopAutomatic);
		}

		void start(Receiver &receiver)
		{

			int seamSeries; receiver.deMarshal(seamSeries);
			int seam; receiver.deMarshal(seam);
			getServer()->start(seamSeries,seam);
		}

		void end(Receiver &receiver)
		{

			xLong duration; receiver.deMarshal(duration);
			getServer()->end(duration);
		}

		void startAutomatic(Receiver &receiver)
		{

			int code; receiver.deMarshal(code);
			getServer()->startAutomatic(code);
		}

		void stopAutomatic(Receiver &receiver)
		{

			getServer()->stopAutomatic();
		}

	private:
		TProductTeachIn<AbstractInterface> * getServer()
		{
			return server_;
		}

	};

} // namespace interface
} // namespace precitec



#endif /*PRODUCTTEACHIN_HANDLER_H_*/
