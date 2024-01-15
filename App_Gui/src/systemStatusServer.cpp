#include "systemStatusServer.h"
#include <QMutexLocker>
#include <QUuid>

#include <precitec/notification.h>
#include <precitec/notificationSystem.h>

using namespace precitec::interface;

using namespace precitec::gui::components::notifications;

namespace precitec
{
namespace gui
{

SystemStatusServer::SystemStatusServer(QObject *parent) : QObject(parent)
{
    connect(this, &SystemStatusServer::upsStateChanged, this, &SystemStatusServer::handleUpsState, Qt::QueuedConnection);
}

SystemStatusServer::~SystemStatusServer() = default;


void SystemStatusServer::signalSystemError(ErrorState errorState)
{
    Q_UNUSED(errorState)
}

void SystemStatusServer::signalHardwareError(Hardware hardware)
{
    Q_UNUSED(hardware)
}

void SystemStatusServer::acknowledgeError(ErrorState errorState)
{
    Q_UNUSED(errorState)
}

void SystemStatusServer::signalState(ReadyState state)
{
    Q_UNUSED(state)
}

void SystemStatusServer::mark(ErrorType errorType, int position)
{
    Q_UNUSED(errorType)
    Q_UNUSED(position)
}

void SystemStatusServer::operationState(precitec::interface::OperationState state)
{
    const auto s = SystemStatusServer::OperationState(state);
    if (m_state == s)
    {
        return;
    }
    const bool wasCalibration = m_state == SystemStatusServer::OperationState::Calibration;
    m_state = s;
    if (m_state == SystemStatusServer::OperationState::Automatic)
    {
        m_productInfo = precitec::interface::ProductInfo();
        emit productInfoChanged();
    }
    if (m_state == SystemStatusServer::OperationState::Normal)
    {
        emit enteredNormalState();
    } else if (m_state == SystemStatusServer::OperationState::NotReady)
    {
        emit enteredNotReadyState();
    }
    emit stateChanged();
    if (wasCalibration)
    {
        emit returnedFromCalibration();
    }
}

void SystemStatusServer::upsState(precitec::interface::UpsState state)
{
    QMutexLocker lock{&m_upsMutex};
    if (m_ups == state)
    {
        return;
    }
    m_ups = state;
    emit upsStateChanged();
}

void SystemStatusServer::workingState(WorkingState state)
{
    Q_UNUSED(state)
}

void SystemStatusServer::signalProductInfo(precitec::interface::ProductInfo productInfo)
{
    m_productInfo = productInfo;
    m_seamNumber = m_productInfo.m_oSeam;
    try
    {
        if (!m_productInfo.m_oSeamLabel.empty())
        {
            m_seamNumber = std::stoi(m_productInfo.m_oSeamLabel);
        }
    } catch (...)
    {
    }
    emit productInfoChanged();
}

void SystemStatusServer::productUpdated(Poco::UUID productId)
{
    emit productLoaded(QUuid{QByteArray::fromStdString(productId.toString())});
}

void SystemStatusServer::filterParameterUpdated(Poco::UUID measureTaskID, Poco::UUID instanceFilterId)
{
    Q_UNUSED(instanceFilterId)
    emit parameterUpdated(QUuid{QByteArray::fromStdString(measureTaskID.toString())});
}

void SystemStatusServer::handleUpsState()
{
    if (!m_upsNotification.isNull())
    {
        NotificationSystem::instance()->withdraw(m_upsNotification);
    }
    QMutexLocker lock{&m_upsMutex};
    switch (m_ups)
    {
    case interface::LowBattery:
    {
        Notification notification;
        notification.createUuid();
        notification.setLevel(Notification::Level::Warning);
        notification.setIconName(QStringLiteral("dialog-warning"));
        notification.setMessage(tr("UPS battery is low"));
        notification.setPersistent(true);
        m_upsNotification = NotificationSystem::instance()->notify(std::move(notification));
        break;
    }
    case interface::NoCommunication:
    {
        Notification notification;
        notification.createUuid();
        notification.setLevel(Notification::Level::Warning);
        notification.setIconName(QStringLiteral("dialog-warning"));
        notification.setMessage(tr("Communications with UPS lost"));
        notification.setPersistent(true);
        m_upsNotification = NotificationSystem::instance()->notify(std::move(notification));
        break;
    }
    case interface::OnBattery:
        m_upsNotification = NotificationSystem::instance()->error(tr("UPS on battery - going to shutdown in next 3 minutes"));
        break;
    case interface::ReplaceBattery:
        m_upsNotification = NotificationSystem::instance()->error(tr("UPS battery needs to be replaced"));
        break;
    case interface::Online:
        m_upsNotification = NotificationSystem::instance()->information(tr("UPS on line power"));
        break;
    case interface::Shutdown:
        m_upsNotification = NotificationSystem::instance()->information(tr("UPS triggered automatic shutdown"));
        break;
    }
}

} // namespace gui
} // namespace precitec
