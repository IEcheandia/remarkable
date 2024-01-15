#pragma once

#include "event/querySystemStatus.interface.h"
#include "server/eventHandler.h"

namespace precitec
{
namespace interface
{

template <>
class TQuerySystemStatus<EventHandler> : public Server<EventHandler>, public TQuerySystemStatusMessageDefinition
{
public:
    EVENT_HANDLER( TQuerySystemStatus );
public:
    void registerCallbacks()
    {
        REGISTER_EVENT(RequestOperationState, requestOperationState);
    }

    void requestOperationState(Receiver &receiver)
    {
        server_->requestOperationState();
    }
};

}
}
