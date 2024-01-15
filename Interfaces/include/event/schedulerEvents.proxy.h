#pragma once

#include "event/schedulerEvents.interface.h"
#include "server/eventProxy.h"

namespace precitec
{

namespace interface
{

template <>
class TSchedulerEvents<EventProxy> : public Server<EventProxy>, public TSchedulerEvents<AbstractInterface>, public TSchedulerEventsMessageDefinition
{
public:
    TSchedulerEvents() : EVENT_PROXY_CTOR(TSchedulerEvents), TSchedulerEvents<AbstractInterface>()
    {
    }

    virtual ~TSchedulerEvents() {}

    void schedulerEventFunction(SchedulerEvents p_oEvent, const std::map<std::string, std::string>& p_oEventMap) override
    {
        INIT_EVENT(SchedulerEventFunction);
        signaler().marshal(p_oEvent);
        signaler().marshal(p_oEventMap.size());
        for(auto oVar: p_oEventMap)
        {
            signaler().marshal(oVar.first);
            signaler().marshal(oVar.second);
        }
        signaler().send();
    }

};

} // namespace interface
} // namespace precitec
