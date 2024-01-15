#pragma once

#include "server/eventProxy.h"
#include "event/deviceNotification.interface.h"

namespace precitec
{
namespace interface
{

template <>
class TDeviceNotification<EventProxy> : public Server<EventProxy>, public TDeviceNotification<AbstractInterface>, public TDeviceNotificationMessageDefinition
{
public:
    TDeviceNotification() : EVENT_PROXY_CTOR(TDeviceNotification), TDeviceNotification<AbstractInterface>()
    {}

    void keyValueChanged(const Poco::UUID &device, SmpKeyValue keyValue) override
    {
        INIT_EVENT(KeyValueChanged);
        signaler().marshal(device);
        signaler().marshal(keyValue);
        signaler().send();
    }
};

}
}
