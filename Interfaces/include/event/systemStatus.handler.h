/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			AL, SB
 *  @date			2010
 *  @brief			Transmit system status			
 */

#ifndef SYSTEM_STATUS_HANDLER_H
#define SYSTEM_STATUS_HANDLER_H

#include "event/systemStatus.interface.h"
#include "server/eventHandler.h"


#include <string>
#include <vector>
#include <utility> // for pair


namespace precitec
{
namespace interface
{

	template <>
	class TSystemStatus<EventHandler> : public Server<EventHandler>, public TSystemStatusMessageDefinition
	{
	public:
		EVENT_HANDLER(TSystemStatus);

		typedef Poco::UUID	  PocoUUID;     ///< for msg macros

		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!
			REGISTER_EVENT(SignalSystemError, signalSystemError);
			REGISTER_EVENT(SignalHardwareError, signalHardwareError);
			REGISTER_EVENT(AcknowledgeError, acknowledgeError);
			REGISTER_EVENT(SignalState, signalState);
			REGISTER_EVENT(Mark, mark);
			REGISTER_EVENT(OperationStateMsg, operationState);
			REGISTER_EVENT(UpsStateMsg, upsState);
			REGISTER_EVENT(WorkingStateMsg, workingState);
			REGISTER_EVENT(SignalProductInfo, signalProductInfo);
			REGISTER_EVENT(ProductUpdated, productUpdated);
            REGISTER_EVENT(FilterParameterUpdated, filterParameterUpdated);
		}

		void signalSystemError(Receiver &receiver)
		{
			int errorState; receiver.deMarshal(errorState);
			getServer()->signalSystemError(ErrorState(errorState));
		}

		void signalHardwareError(Receiver &receiver)
		{
			int hardware; receiver.deMarshal(hardware);
			getServer()->signalHardwareError(Hardware(hardware));
		}

		void acknowledgeError(Receiver &receiver)
		{
			int errorState; receiver.deMarshal(errorState);
			getServer()->acknowledgeError(ErrorState(errorState));
		}

		void signalState(Receiver &receiver)
		{
			int state; receiver.deMarshal(state);
			getServer()->signalState(ReadyState(state));
		}

		void mark(Receiver &receiver)
		{
			int errorState; receiver.deMarshal(errorState);
			int position; receiver.deMarshal(position);
			getServer()->mark(ErrorType(errorState), position);
		}

		void operationState(Receiver &receiver)
		{
			int state; receiver.deMarshal(state);
			getServer()->operationState(OperationState(state));
		}

		void upsState(Receiver &receiver)
		{
			UpsState state; receiver.deMarshal(state);
			getServer()->upsState(state);
		}

		void workingState(Receiver &receiver)
		{
			int state; receiver.deMarshal(state);
			getServer()->workingState(WorkingState(state));
		}

		void signalProductInfo(Receiver &receiver)
		{
			ProductInfo productInfo; receiver.deMarshal(productInfo);
			getServer()->signalProductInfo(productInfo);
		}

        void productUpdated(Receiver &receiver)
        {
            Poco::UUID productId;
            receiver.deMarshal(productId);
            getServer()->productUpdated(productId);
        }

        void filterParameterUpdated(Receiver &receiver)
        {
            Poco::UUID measureTaskId;
            receiver.deMarshal(measureTaskId);
            Poco::UUID instanceFilterId;
            receiver.deMarshal(instanceFilterId);
            getServer()->filterParameterUpdated(measureTaskId, instanceFilterId);
        }

	private:
		TSystemStatus<AbstractInterface> * getServer()
		{
			return server_;
		}

	};

} // namespace interface
} // namespace precitec

#endif /*SYSTEM_STATUS_HANDLER_H*/
