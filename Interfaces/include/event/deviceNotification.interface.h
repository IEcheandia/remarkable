#pragma once

#include  "message/device.h"
#include  "module/interfaces.h"
#include  "server/interface.h"

namespace precitec
{
namespace interface
{

template <int CallType>
class TDeviceNotification;


/**
 * Protocol to inform about changes in KeyValue.
 **/
template <>
class TDeviceNotification<AbstractInterface>
{
public:
    TDeviceNotification() {}
    virtual ~TDeviceNotification() {}

    /**
     * The @p keyValue in the Device identified by @p deviceId changed.
     **/
    virtual void keyValueChanged(const Poco::UUID& deviceId, SmpKeyValue keyValue) = 0;
};

struct TDeviceNotificationMessageDefinition
{
    EVENT_MESSAGE(KeyValueChanged, Poco::UUID, SmpKeyValue);
    MESSAGE_LIST(
        KeyValueChanged
    );
};

template <>
class TDeviceNotification<Messages> : public Server<Messages>, public TDeviceNotificationMessageDefinition
{
public:
    TDeviceNotification<Messages>() : info(system::module::DeviceNotification, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;
private:
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };

};

}
}
