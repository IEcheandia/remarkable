#pragma once

#include <QObject>

#include "event/deviceNotification.interface.h"

class QUuid;

namespace precitec
{
namespace gui
{

class DeviceNotificationServer : public QObject, public precitec::interface::TDeviceNotification<precitec::interface::AbstractInterface>
{
    Q_OBJECT
public:
    explicit DeviceNotificationServer(QObject *parent = nullptr);
    ~DeviceNotificationServer() override;

    void keyValueChanged(const Poco::UUID &deviceId, precitec::interface::SmpKeyValue keyValue) override;

Q_SIGNALS:
    void changed(const QUuid &deviceId, precitec::interface::SmpKeyValue keyValue);
};

}
}

Q_DECLARE_METATYPE(precitec::gui::DeviceNotificationServer*)
