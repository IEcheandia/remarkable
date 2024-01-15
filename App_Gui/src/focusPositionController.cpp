#include "focusPositionController.h"
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

FocusPositionController::FocusPositionController(QObject *parent)
    : QObject(parent)
{
    connect(this, &FocusPositionController::weldHeadDeviceChanged, this,
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

                    m_absolutePosition = weldHeadDevice->deviceProxy()->get(std::string("Z_Collimator_PositionAbsolute"), 0)->value<int>();
                    emit absolutePositionChanged();
                    m_systemOffset = weldHeadDevice->deviceProxy()->get(std::string("Z_Collimator_SystemOffset"), 0.0)->value<double>();
                    emit systemOffsetChanged();

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
    connect(this, &FocusPositionController::absolutePositionChanged, this, &FocusPositionController::updateAbsolutePosition);
    connect(this, &FocusPositionController::systemOffsetChanged, this, &FocusPositionController::updateSystemOffset);
}

FocusPositionController::~FocusPositionController() = default;

void FocusPositionController::setWeldHeadDevice(DeviceProxyWrapper *device)
{
    if (m_weldHeadDevice == device)
    {
        return;
    }
    disconnect(m_weldHeadDeviceDestroyed);
    m_weldHeadDevice = device;
    if (m_weldHeadDevice)
    {
        m_weldHeadDeviceDestroyed = connect(device, &DeviceProxyWrapper::destroyed, this, std::bind(&FocusPositionController::setWeldHeadDevice, this, nullptr));
    } else
    {
        m_weldHeadDeviceDestroyed = {};
    }
    emit weldHeadDeviceChanged();
}

void FocusPositionController::setAbsolutePosition(int position)
{
    if(m_absolutePosition == position)
    {
        return;
    }
    m_absolutePosition = position;
    emit absolutePositionChanged();
}

void FocusPositionController::setSystemOffset(double systemOffset)
{
    if(m_systemOffset == systemOffset)
    {
        return;
    }
    m_systemOffset = systemOffset;
    emit systemOffsetChanged();
}

void FocusPositionController::setReady(bool set)
{
    if (m_ready == set)
    {
        return;
    }
    m_ready = set;
    emit readyChanged();
}

void FocusPositionController::setUpdating(bool set)
{
    if (m_updating == set)
    {
        return;
    }
    m_updating = set;
    emit updatingChanged();
}

void FocusPositionController::updateAbsolutePosition()
{
    const auto weldHeadDevice = m_weldHeadDevice;
    update([this, weldHeadDevice]
    {
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("Z_Collimator_PositionAbsolute"), m_absolutePosition)));
    });
}

void FocusPositionController::updateSystemOffset()
{
    const auto weldHeadDevice = m_weldHeadDevice;
    update([this, weldHeadDevice]
    {
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("Z_Collimator_SystemOffset"), m_systemOffset)));
    });
}

void FocusPositionController::performReferenceRun()
{
    const auto weldHeadDevice = m_weldHeadDevice;
    update([weldHeadDevice]
    {
        weldHeadDevice->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("Z_Collimator_Homing"), true)));
    });
}

void FocusPositionController::update(std::function<void()> updateFunction)
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

bool FocusPositionController::hasPermission()
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
