#include "laserControlDelayController.h"
#include "permissions.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

#include <precitec/userManagement.h>

using namespace precitec::interface;
using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

LaserControlDelayController::LaserControlDelayController(QObject *parent)
    : QObject(parent)
{
    connect(this, &LaserControlDelayController::weldHeadDeviceChanged, this,
            [this]
        {
            if (!m_weldHeadDevice)
            {
                setReady(false);
                return;
            }
            auto weldHeadDevice = m_weldHeadDevice;
            if (!UserManagement::instance()->hasPermission(int(weldHeadDevice->readPermission())))
            {
                return;
            }
            auto watcher = new QFutureWatcher<Configuration>{this};
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                [this, weldHeadDevice, watcher]
                {
                    watcher->deleteLater();

                    m_delay = weldHeadDevice->deviceProxy()->get(std::string("LC_Parameter_No2"), 0)->value<int>();
                    emit delayChanged();
                    m_frequency = weldHeadDevice->deviceProxy()->get(std::string("ScanTrackerFrequencyContinuously"), 100)->value<int>();
                    emit frequencyChanged();
                    m_amplitude = (weldHeadDevice->deviceProxy()->get(std::string("ScanWidthFixed"), 4000)->value<int>()) * 0.001;
                    emit amplitudeChanged();

                    weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LC_Parameter_No3"), m_power)));
                    weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LC_Parameter_No4"), 0)));
                    weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LC_Parameter_No5"), 0)));
                    weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LC_Parameter_No6"), 0)));
                    weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("LC_Send_Data"), true)));
                    weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("LC_Start_Processing"), true)));

                    setReady(true);
                }
            );
            watcher->setFuture(QtConcurrent::run(
                [weldHeadDevice]
                {
                    return weldHeadDevice->deviceProxy()->get();
                })
            );
        }
    );
    connect(this, &LaserControlDelayController::delayChanged, this, &LaserControlDelayController::updateDelay);
    connect(this, &LaserControlDelayController::frequencyChanged, this, &LaserControlDelayController::updateFrequency);
    connect(this, &LaserControlDelayController::powerChanged, this, &LaserControlDelayController::updatePower);
    connect(this, &LaserControlDelayController::amplitudeChanged, this, &LaserControlDelayController::updateAmplitude);
}

LaserControlDelayController::~LaserControlDelayController() = default;

void LaserControlDelayController::setWeldHeadDevice(DeviceProxyWrapper *device)
{
    if (m_weldHeadDevice == device)
    {
        return;
    }
    disconnect(m_weldHeadDeviceDestroyed);
    m_weldHeadDevice = device;
    if (m_weldHeadDevice)
    {
        m_weldHeadDeviceDestroyed = connect(device, &DeviceProxyWrapper::destroyed, this, std::bind(&LaserControlDelayController::setWeldHeadDevice, this, nullptr));
    } else
    {
        m_weldHeadDeviceDestroyed = {};
    }
    emit weldHeadDeviceChanged();
}

void LaserControlDelayController::setDelay(int delay)
{
    if(m_delay == delay)
    {
        return;
    }
    m_delay = delay;
    emit delayChanged();
}

void LaserControlDelayController::setFrequency(int frequency)
{
    if(m_frequency == frequency)
    {
        return;
    }
    m_frequency = frequency;
    emit frequencyChanged();
}

void LaserControlDelayController::setPower(int power)
{
    if(m_power == power)
    {
        return;
    }
    m_power = power;
    emit powerChanged();
}

void LaserControlDelayController::setAmplitude(int amplitude)
{
    if(m_amplitude == amplitude)
    {
        return;
    }
    m_amplitude = amplitude;
    emit amplitudeChanged();
}

void LaserControlDelayController::setReady(bool set)
{
    if (m_ready == set)
    {
        return;
    }
    m_ready = set;
    emit readyChanged();
}

void LaserControlDelayController::setUpdating(bool set)
{
    if (m_updating == set)
    {
        return;
    }
    m_updating = set;
    emit updatingChanged();
}

void LaserControlDelayController::setVisible(bool set)
{
    if (m_visible == set)
    {
        return;
    }
    m_visible = set;
    emit visibleChanged();
        
    if (m_weldHeadDevice == nullptr)
    {
        return;
    }
    
    if (m_visible == true)
    {
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int> (std::string("ScanWidthFixed"), m_amplitude * 1000.0 )));
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int> (std::string("ScanTrackerFrequencyContinuously"), m_frequency)));        
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int> (std::string("LC_Parameter_No2"), m_delay)));
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int> (std::string("LC_Parameter_No3"), m_power)));
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int> (std::string("LC_Parameter_No4"), 0)));
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int> (std::string("LC_Parameter_No5"), m_power)));
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int> (std::string("LC_Parameter_No6"), 0)));
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("LC_Send_Data"), true)));
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("TrackerDriverOnOff"), true)));
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("LC_Start_Processing"), true)));
    }
    else
    {
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("TrackerDriverOnOff"), false)));        
        m_weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("LC_Start_Processing"), false)));
    }
}

void LaserControlDelayController::updateDelay()
{
    const auto weldHeadDevice = m_weldHeadDevice;
    update([this, weldHeadDevice]
    {
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LC_Parameter_No2"), m_delay)));
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("LC_Send_Data"), true)));
    });
}

void LaserControlDelayController::updateFrequency()
{
    const auto weldHeadDevice = m_weldHeadDevice;
    update([this, weldHeadDevice]
    {
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("ScanTrackerFrequencyContinuously"), m_frequency)));
    });
}

void LaserControlDelayController::updatePower()
{
    const auto weldHeadDevice = m_weldHeadDevice;
    update([this, weldHeadDevice]
    {
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LC_Parameter_No3"), m_power)));
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LC_Parameter_No4"), 0)));
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LC_Parameter_No5"), 0)));
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LC_Parameter_No6"), 0)));
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("LC_Send_Data"), true)));
    });
}

void LaserControlDelayController::updateAmplitude()
{
    const auto weldHeadDevice = m_weldHeadDevice;
    update([this, weldHeadDevice]
    {
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("ScanWidthFixed"), m_amplitude * 1000.0 )));
    });
}

void LaserControlDelayController::update(std::function<void()> updateFunction)
{
    if (!m_weldHeadDevice)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }
    if (!isReady())
    {
        return;
    }
    if (isUpdating())
    {
        return;
    }
    setUpdating(true);

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this,
            [this, watcher]
            {
                watcher->deleteLater();
                setUpdating(false);
            }
    );
    watcher->setFuture(QtConcurrent::run(updateFunction));
}

bool LaserControlDelayController::hasPermission()
{
    if (!UserManagement::instance()->hasPermission(int(m_weldHeadDevice->writePermission())))
    {
        return false;
    }
    if (!UserManagement::instance()->hasPermission(int(Permission::EditWeldHeadDeviceConfig)))
    {
        return false;
    }
    return true;
}

}
}

