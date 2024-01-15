#pragma once

#include  "message/device.h"
#include  "module/interfaces.h"
#include  "server/interface.h"

namespace precitec
{
namespace interface
{

template <int CallType>
class TQuerySystemStatus;


/**
 * Interface as counterpart to TSystemStatus. While TSystemStatus is a push interface, this is
 * a poll interface. If a component needs to know the current state it can call the methods and
 * the server side will provide the reply over the TSystemStatus interface.
 **/
template <>
class TQuerySystemStatus<AbstractInterface>
{
public:
    TQuerySystemStatus() {}
    virtual ~TQuerySystemStatus() {}

    /**
     * Requests TSystemStatus::operationState
     **/
    virtual void requestOperationState() = 0;
};

struct TQuerySystemStatusMessageDefinition
{
    EVENT_MESSAGE(RequestOperationState, void);
    MESSAGE_LIST(
        RequestOperationState
    );
};

template <>
class TQuerySystemStatus<Messages> : public Server<Messages>, public TQuerySystemStatusMessageDefinition
{
public:
    TQuerySystemStatus<Messages>() : info(system::module::QuerySystemStatus, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;
private:
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };

};

}
}
