#include "deviceNotificationServer.h"

#include <QUuid>

namespace precitec
{
namespace gui
{

DeviceNotificationServer::DeviceNotificationServer(QObject *parent)
    : QObject(parent)
{
}

DeviceNotificationServer::~DeviceNotificationServer() = default;

void DeviceNotificationServer::keyValueChanged(const Poco::UUID &deviceId, precitec::interface::SmpKeyValue keyValue)
{
    if(keyValue.isNull())
    {
        return;
    }
    emit changed(QUuid(QString::fromStdString(deviceId.toString())), keyValue);
}

}
}
