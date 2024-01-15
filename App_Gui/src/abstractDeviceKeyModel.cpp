#include "abstractDeviceKeyModel.h"
#include "deviceProxyWrapper.h"

namespace precitec
{
namespace gui
{

AbstractDeviceKeyModel::AbstractDeviceKeyModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

AbstractDeviceKeyModel::~AbstractDeviceKeyModel() = default;

void AbstractDeviceKeyModel::setDeviceProxy(DeviceProxyWrapper *proxy)
{
    if (m_deviceProxy == proxy)
    {
        return;
    }
    m_deviceProxy = proxy;
    disconnect(m_destroyConnection);
    m_destroyConnection = QMetaObject::Connection{};
    if (m_deviceProxy)
    {
        m_destroyConnection = connect(m_deviceProxy, &QObject::destroyed, this, std::bind(&AbstractDeviceKeyModel::setDeviceProxy, this, nullptr));
    }
    emit deviceProxyChanged();
}

void AbstractDeviceKeyModel::setNotificationServer(DeviceNotificationServer *server)
{
    if (m_notificationServer.data() == server)
    {
        return;
    }
    m_notificationServer = server;
    emit notificationServerChanged();
}

bool AbstractDeviceKeyModel::changesRequireRestart() const
{
    if (!m_deviceProxy)
    {
        return false;
    }
    return m_deviceProxy->changesRequireRestart();
}

}
}
