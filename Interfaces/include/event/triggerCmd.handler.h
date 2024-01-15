/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, AL, AB, HS
 * 	@date		2009
 * 	@brief		Trigger command interface. Signals sensors to take a single measurement or several measurements over time (burst).
 */

#ifndef TRIGGERCMD_HANDLER_H_
#define TRIGGERCMD_HANDLER_H_

#include "triggerCmd.h"
#include "triggerCmd.interface.h"
#include "server/eventHandler.h"

namespace precitec {
namespace interface {

	template <>
	class TTriggerCmd<EventHandler> : public Server<EventHandler>, public TTriggerCmdMessageDefinition
	{
	public:
		EVENT_HANDLER( TTriggerCmd );

		void registerCallbacks()
		{
			REGISTER_EVENT(Single, single);
			REGISTER_EVENT(Burst, burst);
			REGISTER_EVENT(Cancel, cancel);
		}

		void single(Receiver &receiver)
		{
			std::vector<int> id;		receiver.deMarshal(id);
			TriggerContext context;		receiver.deMarshal(context);

			server_->single(id, context);
		}

		void burst(Receiver &receiver)
		{
			std::vector<int> id; 			receiver.deMarshal(id);
			TriggerContext context;			receiver.deMarshal(context);
			TriggerSource source; 			receiver.deMarshal(source);
			TriggerInterval interval; 		receiver.deMarshal(interval);

			server_->burst(id, context, source, interval);
		}

		void cancel(Receiver &receiver)
		{
			std::vector<int> id; receiver.deMarshal(id);

			server_->cancel(id);
		}
	};

} // namespace interface
} // namespace precitec


#endif /*TRIGGERCMD_HANDLER_H_*/
