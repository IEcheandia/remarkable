/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			AL, SB
 *  @date			2010
 *  @brief			Transmit system status			
 */

#ifndef SYSTEM_STATUS_PROXY_H
#define SYSTEM_STATUS_PROXY_H

#include <iostream>

#include "event/systemStatus.interface.h"
#include "server/eventProxy.h"
#include <string>

namespace precitec
{
namespace interface
{

	/**
	 * Die Basisklasse fuer alle Remote-Caller.
	 * Die Details der Message-Verarbeitung liegt im Sender.
	 * Wichtig!! public muss wiederholt werden fuer jede Basisklasse
	 */
	template <>
	class TSystemStatus<EventProxy> : public Server<EventProxy>, public TSystemStatus<AbstractInterface>, public TSystemStatusMessageDefinition
	{
	public:
		
		/// beide Basisklassen muessen geeignet initialisiert werden
		TSystemStatus() : EVENT_PROXY_CTOR(TSystemStatus), TSystemStatus<AbstractInterface>()
		{
		}
 		/// der DTor muss virtuell sein
		virtual ~TSystemStatus() {}

		virtual void signalSystemError(ErrorState state)
		{
			// von der Message brauchen wir nur den index
			INIT_EVENT(SignalSystemError);
			//signaler().initMessage(Msg::index);
			signaler().marshal(int(state));
			signaler().send();
		}

		virtual void signalHardwareError(Hardware hardware)
		{
			// von der Message brauchen wir nur den index
			INIT_EVENT(SignalHardwareError);
			//signaler().initMessage(Msg::index);
			signaler().marshal(int(hardware));
			signaler().send();
		}

		virtual void acknowledgeError(ErrorState errorState)
		{
			// von der Message brauchen wir nur den index
			INIT_EVENT(AcknowledgeError);
			//signaler().initMessage(Msg::index);
			signaler().marshal(int(errorState));
			signaler().send();
		}

		virtual void signalState(ReadyState state)
		{
			// von der Message brauchen wir nur den index
			INIT_EVENT(SignalState);
			//signaler().initMessage(Msg::index);
			signaler().marshal(int(state));
			signaler().send();
		}
		virtual void mark(ErrorType errorType, int position)
		{
			// von der Message brauchen wir nur den index
			INIT_EVENT(Mark);
			//signaler().initMessage(Msg::index);
			signaler().marshal(int(errorType));
			signaler().marshal(position);
			signaler().send();
		}
		virtual void operationState(OperationState state)
		{
			// std::cout << "RCaller::signalState" << state<< std::endl;
			// von der Message brauchen wir nur den index
			INIT_EVENT(OperationStateMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(int(state));
			signaler().send();
		}
		virtual void upsState(UpsState state)
		{
			INIT_EVENT(UpsStateMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(state);
			signaler().send();
		}
		virtual void workingState(WorkingState state)
		{
			INIT_EVENT(WorkingStateMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(int(state));
			signaler().send();
		}
		virtual void signalProductInfo(ProductInfo productInfo)
		{
			INIT_EVENT(SignalProductInfo);
			//signaler().initMessage(Msg::index);
			signaler().marshal(productInfo);
			signaler().send();
		}

        void productUpdated(Poco::UUID productId) override
        {
            INIT_EVENT(ProductUpdated);
            signaler().marshal(productId);
            signaler().send();
        }

        void filterParameterUpdated(Poco::UUID measureTaskId, Poco::UUID instanceFilterId) override
        {
            INIT_EVENT(FilterParameterUpdated);
            signaler().marshal(measureTaskId);
            signaler().marshal(instanceFilterId);
            signaler().send();
        }
	};

} // namespace interface
} // namespace precitec


#endif // SYSTEM_STATUS_PROXY
