#include "axisInformation.h"
#include "deviceNotificationServer.h"
#include "deviceProxyWrapper.h"
#include "weldHeadServer.h"
#include "message/calibration.interface.h"

#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QTimer>

#include <functional>

using precitec::interface::SmpKeyValue;
using precitec::interface::Configuration;

namespace precitec
{
namespace gui
{

const static std::string s_yAxisEnabled{"Y_Axis_Enabled"};
const static std::string s_yAxisSoftLimitsEnabled{"Y_Axis_SoftLimitsOnOff"};
const static std::string s_yAxisSetUpperLimit{"Y_Axis_SetUpperLimit"};
const static std::string s_yAxisSetLowerLimit{"Y_Axis_SetLowerLimit"};
const static std::string s_yAxisAbsolutePosition{"Y_Axis_AbsolutePosition"};
const static std::string s_yAxisHomingDirectionPositive{"Y_Axis_HomingDirectionPositive"};

AxisInformation::AxisInformation(QObject* parent)
    : QObject(parent)
    , m_pollHeadInfo(new QTimer(this))
{
    connect(this, &AxisInformation::weldHeadDeviceChanged, this, &AxisInformation::checkAxisEnabled);
    connect(this, &AxisInformation::axisEnabledChanged, this, &AxisInformation::initAxisDevice);

    m_pollHeadInfo->setInterval(std::chrono::milliseconds(100));
    connect(m_pollHeadInfo, &QTimer::timeout, this,
        [this]
        {
            if (!m_weldHeadSubscribeProxy)
            {
                return;
            }
            m_weldHeadSubscribeProxy->RequestHeadInfo(precitec::interface::eAxisY);
        }
    );
    connect(this, &AxisInformation::homingDirectionChanged, this,
        [this]
        {
            m_requiresHoming = true;
            emit requiresHomingChanged();
        }
    );
}

AxisInformation::~AxisInformation() = default;

void AxisInformation::setDeviceNotificationServer(precitec::gui::DeviceNotificationServer *dns)
{
    if (m_deviceNotificationServer == dns)
    {
        return;
    }
    if (m_deviceNotificationServer)
    {
        disconnect(m_deviceNotificationServer, &DeviceNotificationServer::changed, this, &AxisInformation::keyValueChanged);
    }
    disconnect(m_deviceNotificationDestroyed);
    m_deviceNotificationServer = dns;
    if (m_deviceNotificationServer)
    {
        m_deviceNotificationDestroyed = connect(m_deviceNotificationServer, &QObject::destroyed, this, std::bind(&AxisInformation::setDeviceNotificationServer, this, nullptr));
        connect(m_deviceNotificationServer, &DeviceNotificationServer::changed, this, &AxisInformation::keyValueChanged, Qt::QueuedConnection);
    } else
    {
        m_deviceNotificationDestroyed = QMetaObject::Connection{};
    }
    emit deviceNotificationServerChanged();
}

void AxisInformation::setWeldHeadDevice(precitec::gui::DeviceProxyWrapper *proxy)
{
    if (m_weldHeadDevice == proxy)
    {
        return;
    }
    disconnect(m_weldHeadDestroyed);
    m_weldHeadDevice = proxy;
    if (m_weldHeadDevice)
    {
        m_weldHeadDestroyed = connect(m_weldHeadDevice, &QObject::destroyed, this, std::bind(&AxisInformation::setWeldHeadDevice, this, nullptr));
    } else
    {
        m_weldHeadDestroyed = QMetaObject::Connection{};
    }
    emit weldHeadDeviceChanged();
}

void AxisInformation::checkAxisEnabled()
{
    if (!m_weldHeadDevice)
    {
        return;
    }
    auto watcher = new QFutureWatcher<SmpKeyValue>(this);
    connect(watcher, &QFutureWatcher<SmpKeyValue>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            auto kv = watcher->result();
            if (m_axisEnabled != kv->value<bool>())
            {
                m_axisEnabled = kv->value<bool>();
                emit axisEnabledChanged();
            }
        }
    );
    watcher->setFuture(QtConcurrent::run([this]
        {
            return m_weldHeadDevice->deviceProxy()->get(s_yAxisEnabled);
        }
    ));
}

void AxisInformation::initAxisDevice()
{
    if (!m_weldHeadDevice || !m_axisEnabled)
    {
        return;
    }

    auto watcher = new QFutureWatcher<Configuration>(this);
    connect(watcher, &QFutureWatcher<Configuration>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            const auto configuration = watcher->result();
            for (auto kv : configuration)
            {
                updateSoftLimitsEnabled(kv);
                updateLowerLimit(kv);
                updateUpperLimit(kv);
                updatePosition(kv);
            }
        }
    );
    watcher->setFuture(QtConcurrent::run([this]
        {
            return m_weldHeadDevice->deviceProxy()->get();
        }
    ));
}

void AxisInformation::updateSoftLimitsEnabled(const precitec::interface::SmpKeyValue &kv)
{
    if (kv->key() != s_yAxisSoftLimitsEnabled)
    {
        return;
    }
    if (m_softLimitsEnabled != kv->value<bool>())
    {
        m_softLimitsEnabled = kv->value<bool>();
        emit softLimitsEnabledChanged();
    }
}

void AxisInformation::updateLowerLimit(const precitec::interface::SmpKeyValue &kv)
{
    if (kv->key() != s_yAxisSetLowerLimit)
    {
        return;
    }
    if (m_lowerLimit != kv->value<int>())
    {
        m_lowerLimit = kv->value<int>();
        emit lowerLimitChanged();
    }
}

void AxisInformation::updateUpperLimit(const precitec::interface::SmpKeyValue &kv)
{
    if (kv->key() != s_yAxisSetUpperLimit)
    {
        return;
    }
    if (m_upperLimit != kv->value<int>())
    {
        m_upperLimit = kv->value<int>();
        emit upperLimitChanged();
    }
}

void AxisInformation::updatePosition(const precitec::interface::SmpKeyValue &kv)
{
    if (kv->key() != s_yAxisAbsolutePosition)
    {
        return;
    }
    if (m_position != kv->value<int>())
    {
        m_position = kv->value<int>();
        emit positionChanged();
    }
}

void AxisInformation::keyValueChanged(const QUuid &deviceId, precitec::interface::SmpKeyValue kv)
{
    if (!m_weldHeadDevice || m_weldHeadDevice->uuid() != deviceId)
    {
        return;
    }
    if (kv.isNull())
    {
        return;
    }
    updateSoftLimitsEnabled(kv);
    updateLowerLimit(kv);
    updateUpperLimit(kv);
    updatePosition(kv);
}

QFuture<void> AxisInformation::updateKeyValue(const precitec::interface::SmpKeyValue &kv)
{
    if (!m_weldHeadDevice)
    {
        return QFuture<void>{};
    }
    return QtConcurrent::run(
        [this, kv]
        {
            m_weldHeadDevice->setKeyValue(kv);
        }
    );
}

QFuture<void> AxisInformation::moveAxis(int targetPos)
{
    return updateKeyValue(SmpKeyValue{new precitec::interface::TKeyValue<int>{s_yAxisAbsolutePosition, targetPos}});
}

QFuture<void> AxisInformation::setLowerLimit(int limit)
{
    return updateKeyValue(SmpKeyValue{new precitec::interface::TKeyValue<int>{s_yAxisSetLowerLimit, limit}});
}

QFuture<void> AxisInformation::setUpperLimit(int limit)
{
    return updateKeyValue(SmpKeyValue{new precitec::interface::TKeyValue<int>{s_yAxisSetUpperLimit, limit}});
}

QFuture<void> AxisInformation::enableSoftwareLimits(bool enable)
{
    return updateKeyValue(SmpKeyValue{new precitec::interface::TKeyValue<bool>{s_yAxisSoftLimitsEnabled, enable}});
}

QFuture<void> AxisInformation::toggleHomingDirectionPositive(bool set)
{
    return updateKeyValue(SmpKeyValue{new precitec::interface::TKeyValue<bool>{s_yAxisHomingDirectionPositive, set}});
}

void AxisInformation::setWeldHeadSubscribeProxy(const WeldHeadSubscribeProxy &proxy)
{
    if (m_weldHeadSubscribeProxy == proxy)
    {
        return;
    }
    m_weldHeadSubscribeProxy = proxy;
    emit weldHeadSubscribeProxyChanged();
}

bool AxisInformation::isPollingHeadInfo() const
{
    return m_pollHeadInfo->isActive();
}

void AxisInformation::setPollHeadInfo(bool set)
{
    if (m_pollHeadInfo->isActive() == set)
    {
        return;
    }
    set ? m_pollHeadInfo->start() : m_pollHeadInfo->stop();
    emit pollHeadInfoChanged();
}

void AxisInformation::setWeldHeadServer(WeldHeadServer *server)
{
    if (m_weldHeadServer == server)
    {
        return;
    }
    disconnect(m_weldHeadServerDestroyed);
    disconnect(m_yAxisChanged);
    m_weldHeadServer = server;
    if (m_weldHeadServer)
    {
        m_weldHeadServerDestroyed = connect(m_weldHeadServer, &QObject::destroyed, this, std::bind(&AxisInformation::setWeldHeadServer, this, nullptr));
        m_yAxisChanged = connect(m_weldHeadServer, &WeldHeadServer::yAxisChanged, this,
            [this]
            {
                if (!m_weldHeadServer)
                {
                    return;
                }
                const auto &headInfo = m_weldHeadServer->yAxisInfo();
                if (!m_initiallyPolled && headInfo.status != interface::Offline)
                {
                    m_initiallyPolled = true;
                    emit initiallyPolled();
                }
                updateStatusWord(headInfo);
                updateErrorCode(headInfo);
                updatePositionUserUnit(headInfo);
                updateActTorque(headInfo);
                updateActVelocity(headInfo);
                updateModeOfOperation(headInfo);
                updateHomingDirection(headInfo);
            }, Qt::QueuedConnection);
    } else
    {
        m_weldHeadServerDestroyed = QMetaObject::Connection{};
        m_yAxisChanged = QMetaObject::Connection{};
    }
    emit weldHeadServerChanged();
}

void AxisInformation::updateStatusWord(const precitec::interface::HeadInfo &info)
{
    if (m_statusWord == info.statusWord)
    {
        return;
    }
    m_statusWord = info.statusWord;
    emit statusChanged();
}

void AxisInformation::updateErrorCode(const precitec::interface::HeadInfo &info)
{
    if (m_errorCode == info.errorCode)
    {
        return;
    }
    m_errorCode = info.errorCode;
    emit errorCodeChanged();
}

void AxisInformation::updatePositionUserUnit(const precitec::interface::HeadInfo &info)
{
    if (m_positionUserUnit == info.positionUserUnit)
    {
        return;
    }
    m_positionUserUnit = info.positionUserUnit;
    emit positionUserUnitChanged();
}

void AxisInformation::updateActVelocity(const precitec::interface::HeadInfo &info)
{
    if (m_actVelocity == info.actVelocity)
    {
        return;
    }
    m_actVelocity = info.actVelocity;
    emit actVelocityChanged();
}

void AxisInformation::updateActTorque(const precitec::interface::HeadInfo &info)
{
    if (m_actTorque == info.actTorque)
    {
        return;
    }
    m_actTorque = info.actTorque;
    emit actTorqueChanged();
}

void AxisInformation::updateModeOfOperation(const precitec::interface::HeadInfo &info)
{
    if (m_modeOfOperation == info.modeOfOperation)
    {
        return;
    }
    m_modeOfOperation = info.modeOfOperation;
    if (m_requiresHoming)
    {
        if (!m_homingReached && (info.modeOfOperation == interface::Home))
        {
            m_homingReached = true;
        } else if (m_homingReached && (info.modeOfOperation != interface::Home))
        {
            m_homingReached = false;
            m_requiresHoming = false;
            emit requiresHomingChanged();
        }
    }
    emit modeOfOperationChanged();
}

void AxisInformation::updateHomingDirection(const precitec::interface::HeadInfo &info)
{
    if (m_homingDirectionPositive == info.m_oHomingDirPos)
    {
        return;
    }
    m_homingDirectionPositive = info.m_oHomingDirPos;
    static const int distance = 50000;
    static const int nullPosition = 0;
    if (m_homingDirectionPositive)
    {
        if (m_minimumPosition != -1 * distance)
        {
            m_minimumPosition = -1 * distance;
            emit minimumPositionChanged();
        }
        if (m_maximumPosition != nullPosition)
        {
            m_maximumPosition = nullPosition;
            emit maximumPositionChanged();
        }
    } else
    {
        if (m_minimumPosition != nullPosition)
        {
            m_minimumPosition = nullPosition;
            emit minimumPositionChanged();
        }
        if (m_maximumPosition != distance)
        {
            m_maximumPosition = distance;
            emit maximumPositionChanged();
        }
    }
    emit homingDirectionChanged();
}

}
}
