#include "liveModeController.h"
#include "deviceProxyWrapper.h"
#include "deviceNotificationServer.h"
#include "product.h"
#include "productModel.h"
#include "systemStatusServer.h"
#include "../../App_Storage/src/compatibility.h"
#include "common/systemConfiguration.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QTimer>

using precitec::storage::ProductModel;
using namespace precitec::interface;

namespace precitec
{
namespace gui
{

LiveModeController::LiveModeController(QObject *parent)
    : QObject(parent)
    , m_startDelayedLiveMode(new QTimer(this))
    , m_gigECamera(SystemConfiguration::instance().get(SystemConfiguration::IntKey::CameraInterfaceType) == 1)
{
    connect(this, &LiveModeController::liveModeChanged, this,
        [this]
        {
            if (m_liveMode && !m_handlesStartLiveMode)
            {
                startLiveMode();
            } else if (!m_liveMode)
            {
                if (m_returnToLiveModeTimer)
                {
                    m_returnToLiveModeTimer->stop();
                }
                stopLiveMode();
                QMutexLocker lock(&m_inspectionCmdMutex);
                m_liveModeActive = false;
            }
        }
    );

    m_startDelayedLiveMode->setSingleShot(true);
    m_startDelayedLiveMode->setInterval(std::chrono::seconds{10});
    connect(m_startDelayedLiveMode, &QTimer::timeout, this, &LiveModeController::startLiveMode);
    connect(m_startDelayedLiveMode, &QTimer::timeout, this, &LiveModeController::updatingChanged);
}

LiveModeController::~LiveModeController() = default;

QFuture<void> LiveModeController::startLiveMode()
{
    // TODO: add security check
    if (!m_inspectionCmdProxy || !m_productModel || !liveMode())
    {
        return QFuture<void>{};
    }
    if (m_startDelayedLiveMode->isActive())
    {
        m_startDelayedLiveMode->stop();
        emit updatingChanged();
    }
    auto defaultProduct = m_productModel->defaultProduct();
    if (!defaultProduct)
    {
        return QFuture<void>{};
    }
    const Poco::UUID uuid{precitec::storage::compatibility::toPoco(defaultProduct->uuid())};
    return QtConcurrent::run(
        [this, uuid]
        {
            QMutexLocker lock(&m_inspectionCmdMutex);
            if (m_liveModeActive)
            {
                return;
            }
            lock.unlock();
            QMutexLocker grabberLock{&m_grabberMutex};
            if (m_grabberDevice && m_gigECamera)
            {
                m_grabberDevice->setKeyValue(SmpKeyValue{new TKeyValue<bool>{std::string{"ReuseLastImage"}, false}});
                m_grabberDevice->setKeyValue(SmpKeyValue{new TKeyValue<std::string>{std::string{"AcquisitionMode"}, std::string{"Continuous"}}});
            }
            m_inspectionCmdProxy->startLivemode(uuid, 0, 0);
        }
    );
}

QFuture<void> LiveModeController::stopLiveMode(bool returnOnReady)
{
    // TODO: add security check
    if (!m_inspectionCmdProxy)
    {
        return QFuture<void>{};
    }
    m_returnOnReady = returnOnReady;
    return QtConcurrent::run(
        [this]
        {
            m_inspectionCmdProxy->stopLivemode();
        }
    );
}

void LiveModeController::setInspectionCmdProxy(const InspectionCmdProxy &proxy)
{
    if (m_inspectionCmdProxy == proxy)
    {
        return;
    }
    m_inspectionCmdProxy = proxy;
    emit inspectionCmdProxyChanged();
}

void LiveModeController::setLiveMode(bool liveMode)
{
    if (m_liveMode == liveMode)
    {
        return;
    }
    m_liveMode = liveMode;
    emit liveModeChanged();
}

void LiveModeController::setProductModel(ProductModel *pm)
{
    if (m_productModel == pm)
    {
        return;
    }
    disconnect(m_destroyConnection);
    m_destroyConnection = QMetaObject::Connection{};
    m_productModel = pm;
    if (m_productModel)
    {
        m_destroyConnection = connect(m_productModel, &QObject::destroyed, this, std::bind(&LiveModeController::setProductModel, this, nullptr));
    }
    emit productModelChanged();
}

bool LiveModeController::isUpdating() const
{
    return m_startDelayedLiveMode->isActive();
}

void LiveModeController::startDelayedLiveMode()
{
    const bool needsEmit = !isUpdating();
    m_startDelayedLiveMode->start();
    if (needsEmit)
    {
        emit updatingChanged();
    }
}

void LiveModeController::setSystemStatus(SystemStatusServer *systemStatus)
{
    if (m_systemStatus == systemStatus)
    {
        return;
    }
    m_systemStatus = systemStatus;
    disconnect(m_systemStatusDestroyed);
    disconnect(m_productUpdatedConnection);
    if (m_systemStatus)
    {
        m_systemStatusDestroyed = connect(m_systemStatus, &QObject::destroyed, this, std::bind(&LiveModeController::setSystemStatus, this, nullptr));
        m_productUpdatedConnection = connect(m_systemStatus, &SystemStatusServer::productLoaded, this, &LiveModeController::productUpdated, Qt::QueuedConnection);
        connect(m_systemStatus, &SystemStatusServer::stateChanged, this,
            [this]
            {
                if (m_returnToLiveModeTimer && m_systemStatus->state() != SystemStatusServer::OperationState::Normal)
                {
                    m_returnToLiveModeTimer->stop();
                }
                if (m_liveMode)
                {
                    QMutexLocker lock(&m_inspectionCmdMutex);
                    m_liveModeActive = m_systemStatus->state() == SystemStatusServer::OperationState::Live;
                    m_returnOnReady = true;
                }
            }
        );
        connect(m_systemStatus, &SystemStatusServer::enteredNormalState, this,
            [this]
            {
                if (m_liveMode && !m_activateAfterReturnFromNotReady && m_returnOnReady)
                {
                    if (!m_returnToLiveModeTimer)
                    {
                        m_returnToLiveModeTimer = new QTimer{this};
                        m_returnToLiveModeTimer->setInterval(std::chrono::milliseconds{500});
                        m_returnToLiveModeTimer->setSingleShot(true);
                        connect(m_returnToLiveModeTimer, &QTimer::timeout, this, &LiveModeController::startLiveMode);
                    }
                    m_returnToLiveModeTimer->start();
                }
                emit systemStateNormal();
            }
        );
        connect(m_systemStatus, &SystemStatusServer::enteredNotReadyState, this,
            [this]
            {
                if (m_liveMode && m_activateAfterReturnFromNotReady)
                {
                    m_activateAfterReturnFromNotReady = false;
                }
            }
        );
    } else
    {
        m_systemStatusDestroyed = {};
        m_productUpdatedConnection = {};
    }
    emit systemStatusChanged();
}

void LiveModeController::requireActivateAfterReturnFromNotReady()
{
    if (m_returnToLiveModeTimer)
    {
        m_returnToLiveModeTimer->stop();
    }
    m_activateAfterReturnFromNotReady = true;
}

void LiveModeController::productUpdated(const QUuid &uuid)
{
    if (!m_startDelayedLiveMode->isActive() || !m_productModel)
    {
        return;
    }
    if (m_productModel->defaultProduct()->uuid() == uuid)
    {
        m_startDelayedLiveMode->stop();
        startLiveMode();
        updatingChanged();
    }
}

void LiveModeController::setGrabberDeviceProxy(precitec::gui::DeviceProxyWrapper *grabberDevice)
{
    QMutexLocker lock{&m_grabberMutex};
    if (m_grabberDevice == grabberDevice)
    {
        return;
    }
    m_grabberDevice = grabberDevice;
    lock.unlock();
    disconnect(m_grabberDestroyConnection);
    if (m_grabberDevice)
    {
        m_grabberDestroyConnection = connect(m_grabberDevice, &DeviceProxyWrapper::destroyed, this, std::bind(&LiveModeController::setGrabberDeviceProxy, this, nullptr));
    }
    else
    {
        m_grabberDestroyConnection = {};
    }
    emit grabberDeviceProxyChanged();
}

}
}

