#pragma once

#include "event/recorderPoll.interface.h"
#include "server/eventHandler.h"

namespace precitec
{
namespace interface
{


template <>
class TRecorderPoll<EventHandler> : public Server<EventHandler>, public TRecorderPollMessageDefinition
{
public:
    EVENT_HANDLER( TRecorderPoll );

public:
    void registerCallbacks()
    {
        REGISTER_EVENT(SignalReadyForFrame, signalReadyForFrame);
    }

private:
    void signalReadyForFrame()
    {
        server_->signalReadyForFrame();
    }

};

}
}
