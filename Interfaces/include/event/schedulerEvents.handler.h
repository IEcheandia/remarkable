#pragma once

#include "event/schedulerEvents.interface.h"
#include "server/eventHandler.h"

namespace precitec
{

namespace interface
{

using namespace message;

template <>
class TSchedulerEvents<EventHandler> : public Server<EventHandler>, public TSchedulerEventsMessageDefinition
{
public:
    EVENT_HANDLER( TSchedulerEvents );
public:
    void registerCallbacks()
    {
        REGISTER_EVENT(SchedulerEventFunction, schedulerEventFunction);
    }

    void schedulerEventFunction(Receiver &receiver)
    {
        SchedulerEvents oEvent;
        receiver.deMarshal(oEvent);
        size_t oSize;
        receiver.deMarshal(oSize);
        std::map<std::string, std::string> oEventMap;
        for (size_t i = 0; i < oSize; i++)
        {
            std::string oStringFirst;
            receiver.deMarshal(oStringFirst);
            std::string oStringSecond;
            receiver.deMarshal(oStringSecond);
            oEventMap.emplace(std::make_pair(oStringFirst, oStringSecond));
        }
        server_->schedulerEventFunction(oEvent, oEventMap);
    }

};

} // namespace interface
} // namespace precitec
