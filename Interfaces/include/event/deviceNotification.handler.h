#pragma once

#include "event/deviceNotification.interface.h"
#include "server/eventHandler.h"

namespace precitec
{
namespace interface
{

template <>
class TDeviceNotification<EventHandler> : public Server<EventHandler>, public TDeviceNotificationMessageDefinition
{
public:
    EVENT_HANDLER( TDeviceNotification );
public:
    void registerCallbacks()
    {
        REGISTER_EVENT(KeyValueChanged, keyValueChanged);
    }

    void keyValueChanged(Receiver &receiver)
    {
        Poco::UUID deviceId;
        receiver.deMarshal(deviceId);
        SmpKeyValue keyValue;
        receiver.deMarshal(keyValue);

        server_->keyValueChanged(deviceId, keyValue);
    }
};

}
}

