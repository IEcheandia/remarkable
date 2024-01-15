#pragma once

#include "message/device.interface.h"

#include <QPointer>

namespace precitec
{

namespace gui
{
class DeviceNotificationServer;

/**
 * DeviceServer for Gui configuration. Used only app internally, not exposed to interfaces.
 **/
class DeviceServer : public interface::TDevice<interface::AbstractInterface>
{
public:
    explicit DeviceServer();
    ~DeviceServer() override;

    int initialize(interface::Configuration const& config, int subDevice=0) override;
    void uninitialize() override;
    void reinitialize() override;

    interface::KeyHandle set(interface::SmpKeyValue keyValue, int subDevice=0) override;

    void set(interface::Configuration config, int subDevice=0) override;
    interface::SmpKeyValue get(interface::Key key, int subDevice=0) override;
    interface::SmpKeyValue get(interface::KeyHandle handle, int subDevice=0) override;
    interface::Configuration get(int subDevice=0) override;

    void setNotificationServer(DeviceNotificationServer *server);

private:
    interface::SmpKeyValue value(const std::string &name) const;
    QPointer<DeviceNotificationServer> m_notificationServer = nullptr;

};

}
}
