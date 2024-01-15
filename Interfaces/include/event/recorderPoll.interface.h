#pragma once

#include "server/interface.h"
#include "module/interfaces.h" // wg appId

namespace precitec
{
namespace interface
{

template <int CallType>
class TRecorderPoll;

/**
 * This interface allows to better control the TRecorder interface by signaling when
 * the RecorderSever is prepared to accept forther frames.
 **/
template <>
class TRecorderPoll<AbstractInterface>
{
public:
    TRecorderPoll(){}
    virtual ~TRecorderPoll() {}
public:
    virtual void signalReadyForFrame() = 0;
};


struct TRecorderPollMessageDefinition
{
    EVENT_MESSAGE(SignalReadyForFrame, void);
    MESSAGE_LIST(
        SignalReadyForFrame
    );
};

template <>
class TRecorderPoll<Messages> : public TRecorderPollMessageDefinition
{
public:
    TRecorderPoll<Messages>() : info(system::module::RecorderPoll, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;
private:
    // Werte werden so festgelegt, dass alle Parameter und Ergebnisse Platz finden
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 2*MBytes, replyBufLen = 1500*KBytes, NumBuffers=8 };
};

}
}
