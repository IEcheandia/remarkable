#pragma once

#include "server/eventProxy.h"
#include "event/querySystemStatus.interface.h"

namespace precitec
{
namespace interface
{

template <>
class TQuerySystemStatus<EventProxy> : public Server<EventProxy>, public TQuerySystemStatus<AbstractInterface>, public TQuerySystemStatusMessageDefinition
{
public:
    TQuerySystemStatus() : EVENT_PROXY_CTOR(TQuerySystemStatus), TQuerySystemStatus<AbstractInterface>()
    {}

    void requestOperationState() override
    {
        INIT_EVENT(RequestOperationState);
        signaler().send();
    }
};

}
}
