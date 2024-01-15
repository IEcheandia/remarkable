#include "deviceProxyWrapper.h"
#include "deviceKeyValueChangeEntry.h"

#include <precitec/changeSet.h>
#include <precitec/userLog.h>

namespace precitec
{
namespace gui
{

using components::userLog::UserLog;

DeviceProxyWrapper::~DeviceProxyWrapper() = default;

DeviceProxyWrapper::DeviceProxyWrapper(const std::shared_ptr<precitec::interface::TDevice<precitec::interface::AbstractInterface>> &proxy, Permission readPermission, Permission writePermission, const QUuid &uuid, QObject *parent)
    : QObject(parent)
    , m_readPermission(readPermission)
    , m_writePermission(writePermission)
    , m_deviceProxy(proxy)
    , m_uuid(uuid)
{
}

void DeviceProxyWrapper::setKeyValue(const interface::SmpKeyValue &keyValue)
{
    if (!m_deviceProxy)
    {
        return;
    }
    UserLog::instance()->addChange(new DeviceKeyValueChangeEntry{m_uuid, keyValue});
    m_deviceProxy->set(keyValue);
}

void DeviceProxyWrapper::setKeyValues(const std::vector<precitec::interface::SmpKeyValue> &keyValues)
{
    if (!m_deviceProxy)
    {
        return;
    }
    auto changeSet = UserLog::instance()->createChangeSet();
    std::for_each(keyValues.begin(), keyValues.end(), [&changeSet, this] (const auto &kv) { changeSet->addChange(new DeviceKeyValueChangeEntry{m_uuid, kv}); });
    m_deviceProxy->set(keyValues);
    changeSet->finalize();
}

void DeviceProxyWrapper::markAsConnected()
{
    m_connected = true;
    emit connected();
}

}
}
