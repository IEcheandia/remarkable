#pragma once

#include "server/eventProxy.h"
#include "event/recorderPoll.interface.h"

namespace precitec
{

namespace interface
{
template <>
class TRecorderPoll<EventProxy> : public Server<EventProxy>, public TRecorderPoll<AbstractInterface>, public TRecorderPollMessageDefinition
{
public:
    TRecorderPoll() : EVENT_PROXY_CTOR(TRecorderPoll), TRecorderPoll<AbstractInterface>()
    {
    }

    virtual ~TRecorderPoll() {}

public:
    void signalReadyForFrame() override
    {
        INIT_EVENT(SignalReadyForFrame);
        signaler().send();
    }
};

}
}
