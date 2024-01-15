#pragma once
#include "deviceNotificationServer.h"
#include "message/device.interface.h"

#include <QAbstractListModel>
#include <QPointer>
#include <QMutex>

namespace precitec
{
namespace gui
{
class DeviceProxyWrapper;
class DeviceNotificationServer;

/**
 * An abstract list model which supports a DeviceProxyWrapper and a DeviceNotificationServer.
 *
 * Does not provide anything from the QAbstractListModel API, is just intended to share code.
 **/
class AbstractDeviceKeyModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The proxy to the Device server providing the key value pairs
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *deviceProxy READ deviceProxy WRITE setDeviceProxy NOTIFY deviceProxyChanged)
    /**
     * NotificationServer to get events when the data changed
     **/
    Q_PROPERTY(precitec::gui::DeviceNotificationServer *notificationServer READ notificationServer WRITE setNotificationServer NOTIFY notificationServerChanged)

    /**
     * Whether changes on the device proxy require a system reboot
     **/
    Q_PROPERTY(bool changesRequireRestart READ changesRequireRestart NOTIFY deviceProxyChanged)

public:
    explicit AbstractDeviceKeyModel(QObject *parent);
    ~AbstractDeviceKeyModel() override;

    DeviceProxyWrapper *deviceProxy() const
    {
        return m_deviceProxy;
    }
    void setDeviceProxy(DeviceProxyWrapper *proxy);

    DeviceNotificationServer *notificationServer() const
    {
        return m_notificationServer.data();
    }
    void setNotificationServer(DeviceNotificationServer *server);

    bool changesRequireRestart() const;

Q_SIGNALS:
    void deviceProxyChanged();
    void notificationServerChanged();

protected:
    QMutex *proxyMutex()
    {
        return &m_proxyMutex;
    }

private:
    QMutex m_proxyMutex;
    DeviceProxyWrapper *m_deviceProxy = nullptr;
    QMetaObject::Connection m_destroyConnection;
    QPointer<DeviceNotificationServer> m_notificationServer;
};

}
}
