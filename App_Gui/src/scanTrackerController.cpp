#include "scanTrackerController.h"
#include "deviceNotificationServer.h"
#include "deviceProxyWrapper.h"
#include "scanTrackerFrequencyModel.h"

#include <precitec/userManagement.h>

#include <QtConcurrentRun>
#include <QFutureWatcher>

using namespace precitec::interface;
using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

namespace
{

// TODO: move to device.h and remove from hardwareRoiController.cpp
template <typename T>
T getValue(const Configuration &configuration, const std::string &key, T defaultValue)
{
    auto it = std::find_if(configuration.begin(), configuration.end(), [key] (auto kv) { return kv->key() == key; });
    if (it == configuration.end())
    {
        return defaultValue;
    }
    return (*it)->template value<T>();
}

}

ScanTrackerController::ScanTrackerController(QObject *parent)
    : LiveModeController(parent)
    , m_frequencyModel(new ScanTrackerFrequencyModel{this})
{
    connect(this, &ScanTrackerController::weldHeadDeviceChanged, this,
        [this]
        {
            if (!m_weldHeadDevice)
            {
                return;
            }
            auto weldHeadDevice = m_weldHeadDevice;
            if (!UserManagement::instance()->hasPermission(int(weldHeadDevice->readPermission())))
            {
                return;
            }
            auto watcher = new QFutureWatcher<Configuration>{this};
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                    [this, watcher]
                    {
                        watcher->deleteLater();
                        auto configuration = watcher->result();

                        int index = getValue(configuration, "ScanTrackerFrequency", -1);
                        if (m_frequencyIndex != index)
                        {
                            m_frequencyIndex = index;
                            emit frequencyIndexChanged();
                        }
                        bool driverEnabled = getValue(configuration, "TrackerDriverOnOff", false);
                        if (m_driverEnabled != driverEnabled)
                        {
                            m_driverEnabled = driverEnabled;
                            emit driverEnabledChanged();
                        }
                        bool posOutOfGap = getValue(configuration, "ScanPosOutOfGapPos", false);
                        if (m_posOutOfGap != posOutOfGap)
                        {
                            m_posOutOfGap = posOutOfGap;
                            emit posOutOfGapChanged();
                        }
                        bool widthOutOfGap = getValue(configuration, "ScanWidthOutOfGapWidth", false);
                        if (m_widthOutOfGap != widthOutOfGap)
                        {
                            m_widthOutOfGap = widthOutOfGap;
                            emit widthOutOfGapChanged();
                        }
                    }
            );
            watcher->setFuture(QtConcurrent::run(
                [weldHeadDevice, this]
                {
                    QMutexLocker lock{&m_weldHeadDeviceMutex};
                    if (!weldHeadDevice)
                    {
                        return Configuration{};
                    }
                    return weldHeadDevice->deviceProxy()->get();
                }));
        }
    );
}

ScanTrackerController::~ScanTrackerController() = default;

void ScanTrackerController::setNotificationServer(DeviceNotificationServer *server)
{
    if (m_notificationServer.data() == server)
    {
        return;
    }
    disconnect(m_notificationConnection);
    m_notificationConnection = QMetaObject::Connection{};
    m_notificationServer = server;
    if (server)
    {
        m_notificationConnection = connect(server, &DeviceNotificationServer::changed, this,
            [this] (const QUuid &id, SmpKeyValue kv)
            {
                if (!m_weldHeadDevice || id != m_weldHeadDevice->uuid())
                {
                    return;
                }

                if (kv->key() == "ScanTrackerFrequency")
                {
                    m_frequencyIndex = kv->value<int>();
                    emit frequencyIndexChanged();
                } else if (kv->key() == "TrackerDriverOnOff")
                {
                    m_driverEnabled = kv->value<bool>();
                    emit driverEnabledChanged();
                } else if (kv->key() == "ScanPosOutOfGapPos")
                {
                    m_posOutOfGap = kv->value<bool>();
                    emit posOutOfGapChanged();
                } else if (kv->key() == "ScanWidthOutOfGapWidth")
                {
                    m_widthOutOfGap = kv->value<bool>();
                    emit widthOutOfGapChanged();
                }
            }, Qt::QueuedConnection
        );
    }
    emit notificationServerChanged();
}

void ScanTrackerController::queryStatus()
{
    query({"ScanTrackerAskStatus"});
}

void ScanTrackerController::queryVersion()
{
    query({"ScanTrackerAskRevisions"});
}

void ScanTrackerController::querySerialNumber()
{
    query({"ScanTrackerAskSerialNumbers"});
}

void ScanTrackerController::query(const std::string &key)
{
    updateKeyValue(SmpKeyValue(new TKeyValue<bool>(key, false)));
}

void ScanTrackerController::updateKeyValue(const interface::SmpKeyValue &keyValue)
{
    QtConcurrent::run([this, keyValue] {
        QMutexLocker lock{&m_weldHeadDeviceMutex};
        if (!m_weldHeadDevice)
        {
            return;
        }
        m_weldHeadDevice->setKeyValue(keyValue);
    });
}

void ScanTrackerController::setWeldHeadDevice(DeviceProxyWrapper *device)
{
    QMutexLocker lock{&m_weldHeadDeviceMutex};
    if (m_weldHeadDevice == device)
    {
        return;
    }
    disconnect(m_weldHeadDeviceDestroyed);
    m_weldHeadDevice = device;
    if (m_weldHeadDevice)
    {
        m_weldHeadDeviceDestroyed = connect(device, &DeviceProxyWrapper::destroyed, this, std::bind(&ScanTrackerController::setWeldHeadDevice, this, nullptr));
    } else
    {
        m_weldHeadDeviceDestroyed = {};
    }
    emit weldHeadDeviceChanged();
}

QAbstractItemModel *ScanTrackerController::frequencyModel() const
{
    return m_frequencyModel;
}

void ScanTrackerController::setFrequencyIndex(int index)
{
    updateKeyValue(SmpKeyValue(new TKeyValue<int>({"ScanTrackerFrequency"}, index)));
}

void ScanTrackerController::toggleDriverEnabled()
{
    updateKeyValue(SmpKeyValue(new TKeyValue<bool>({"TrackerDriverOnOff"}, !m_driverEnabled)));
}

void ScanTrackerController::togglePosOutOfGap()
{
    updateKeyValue(SmpKeyValue(new TKeyValue<bool>({"ScanPosOutOfGapPos"}, !m_posOutOfGap)));
}

void ScanTrackerController::toggleWidthOutOfGap()
{
    updateKeyValue(SmpKeyValue(new TKeyValue<bool>({"ScanWidthOutOfGapWidth"}, !m_widthOutOfGap)));
}

void ScanTrackerController::setExpertMode(bool enable)
{
    updateKeyValue(SmpKeyValue(new TKeyValue<bool>({"ScanTrackerExpertMode"}, enable)));
    updateKeyValue(SmpKeyValue(new TKeyValue<bool>({"ScanWidthOutOfGapWidth"}, !enable)));
    updateKeyValue(SmpKeyValue(new TKeyValue<bool>({"ScanPosOutOfGapPos"}, !enable)));
}

void ScanTrackerController::setScanPosFixed(int pos)
{
    updateKeyValue(SmpKeyValue(new TKeyValue<int>({"ScanPosFixed"}, pos)));
}

void ScanTrackerController::setScanWidthFixed(int width)
{
    updateKeyValue(SmpKeyValue(new TKeyValue<int>({"ScanWidthFixed"}, width)));
}

}
}
