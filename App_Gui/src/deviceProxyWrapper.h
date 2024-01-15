#pragma once
#include "permissions.h"

#include "message/device.interface.h"

#include <QObject>
#include <QUuid>

namespace precitec
{
namespace gui
{

/**
 * Small wrapper class to hold a @c TDevice<MsgProxy> and the permissions required to read and edit.
 **/
class DeviceProxyWrapper : public QObject
{
    Q_OBJECT
public:
    explicit DeviceProxyWrapper(const std::shared_ptr<precitec::interface::TDevice<precitec::interface::AbstractInterface>> &proxy, Permission readPermission, Permission writePermission, const QUuid &deviceId, QObject *parent = nullptr);
    ~DeviceProxyWrapper() override;

    Permission readPermission() const
    {
        return m_readPermission;
    }

    Permission writePermission() const
    {
        return m_writePermission;
    }

    const std::shared_ptr<precitec::interface::TDevice<precitec::interface::AbstractInterface>> &deviceProxy() const
    {
        return m_deviceProxy;
    }

    void setKeyValue(const precitec::interface::SmpKeyValue &keyValue);
    void setKeyValues(const std::vector<precitec::interface::SmpKeyValue> &keyValues);

    const QUuid &uuid() const
    {
        return m_uuid;
    }

    bool changesRequireRestart() const
    {
        return m_changesRequireRestart;
    }
    void setChangesRequireRestart(bool restartRequired)
    {
        m_changesRequireRestart = restartRequired;
    }

    bool isConnected() const
    {
        return m_connected;
    }

    void markAsConnected();

Q_SIGNALS:
    void connected();

private:
    Permission m_readPermission;
    Permission m_writePermission;
    std::shared_ptr<precitec::interface::TDevice<precitec::interface::AbstractInterface>> m_deviceProxy;
    QUuid m_uuid;
    bool m_changesRequireRestart = false;
    bool m_connected = false;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::DeviceProxyWrapper*)
