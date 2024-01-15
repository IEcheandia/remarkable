#include "hardwareRoiController.h"
#include "deviceProxyWrapper.h"
#include "deviceNotificationServer.h"
#include "hardwareRoiFlushedChangeEntry.h"
#include "product.h"
#include "productModel.h"
#include "../../App_Storage/src/compatibility.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

#include <precitec/userLog.h>

using precitec::storage::ProductModel;
using namespace precitec::interface;

namespace precitec
{
namespace gui
{

using components::userLog::UserLog;

static const QRect s_fullFrame{0, 0, 1024, 1024};

namespace
{

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

HardwareRoiController::HardwareRoiController(QObject *parent)
    : LiveModeController(parent)
{
    connect(this, &HardwareRoiController::grabberDeviceProxyChanged, this,
        [this]
        {
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                setReady(false);
                return;
            }
            QFutureWatcher<Configuration> *watcher = new QFutureWatcher<Configuration>(this);
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                [this, watcher]
                {
                    watcher->deleteLater();
                    auto configuration = watcher->result();
                    m_roi = QRect(getValue(configuration, std::string("Window.X"), 0),
                                  getValue(configuration, std::string("Window.Y"), 0),
                                  getValue(configuration, std::string("Window.W"), 512),
                                  getValue(configuration, std::string("Window.H"), 512));
                    emit roiChanged();
                    m_exposureTime = getValue(configuration, std::string("ExposureTime"), 2.0f);
                    emit exposureTimeChanged();
                    m_brightness = getValue(configuration, std::string("Voltages.BlackLevelOffset"), 14);
                    emit brightnessChanged();
                    m_linLogMode = getValue(configuration, std::string("LinLog.Mode"), 1);
                    emit linLogModeChanged();
                    m_linLogValue1 = getValue(configuration, std::string("LinLog.Value1"), 10);
                    emit linLogValue1Changed();
                    m_linLogValue2 = getValue(configuration, std::string("LinLog.Value2"), 5);
                    emit linLogValue2Changed();
                    m_linLogTime1 = getValue(configuration, std::string("LinLog.Time1"), 900);
                    emit linLogTime1Changed();
                    m_linLogTime2 = getValue(configuration, std::string("LinLog.Time2"), 1000);
                    emit linLogTime2Changed();
                    setReady(true);
                }
            );
            watcher->setFuture(QtConcurrent::run(
                [grabberDeviceProxy, this]
                {
                    QMutexLocker lock{grabberMutex()};
                    if (!grabberDeviceProxy)
                    {
                        return Configuration{};
                    }
                    return grabberDeviceProxy->deviceProxy()->get();
                }));
        }
    );
    connect(this, &HardwareRoiController::systemStateNormal, this,
        [this]
        {
            if (m_exitUpdatingOnNextReady)
            {
                setUpdating(false);
                setReadyToPersist(true);
                m_exitUpdatingOnNextReady = false;
            }
            if (m_onReadyFunction)
            {
                m_onReadyFunction();
                m_onReadyFunction = std::function<void()>{};
            }
        }
    );
}

HardwareRoiController::~HardwareRoiController() = default;

void HardwareRoiController::setReady(bool set)
{
    if (m_ready == set)
    {
        return;
    }
    m_ready = set;
    emit readyChanged();
}

void HardwareRoiController::setUpdating(bool set)
{
    if (m_updating == set)
    {
        return;
    }
    m_updating = set;
    emit updatingChanged();
}

bool HardwareRoiController::isRectValid(const QRect &rect) const
{
    return s_fullFrame.contains(rect);
}

void HardwareRoiController::updateToFullFrame()
{
    updateCameraGeometry(s_fullFrame);
}

void HardwareRoiController::updateCameraGeometry(const QRect &rect)
{
    // TODO: add security check
    if (liveMode())
    {
        requireActivateAfterReturnFromNotReady();
    }
    updateCamera([this, rect]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                return;
            }
            m_exitUpdatingOnNextReady = true;
            grabberDeviceProxy->setKeyValues({
                SmpKeyValue(new TKeyValue<int>(std::string("Window.X"), rect.x())),
                SmpKeyValue(new TKeyValue<int>(std::string("Window.Y"), rect.y())),
                SmpKeyValue(new TKeyValue<int>(std::string("Window.W"), rect.width())),
                SmpKeyValue(new TKeyValue<int>(std::string("Window.H"), rect.height()))
            });
        });
}

void HardwareRoiController::updateExposureTime(qreal time)
{
    // TODO: add security check
    if (qFuzzyCompare(m_exposureTime, time))
    {
        return;
    }
    m_exposureTime = time;
    emit exposureTimeChanged();
    updateCamera([this, time]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                setUpdating(false);
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<float>(std::string("ExposureTime"), time)));
            setUpdating(false);
            setReadyToPersist(true);
        });
}

void HardwareRoiController::updateBrightness(int brightness)
{
    // TODO: add security check
    if (m_brightness == brightness)
    {
        return;
    }
    m_brightness = brightness;
    emit brightnessChanged();
    updateCamera([this, brightness]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                setUpdating(false);
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("Voltages.BlackLevelOffset"), brightness)));
            setUpdating(false);
            setReadyToPersist(true);
        });
}

void HardwareRoiController::updateLinLogMode(int linLogMode)
{
    // TODO: add security check
    if (m_linLogMode == linLogMode)
    {
        return;
    }
    m_linLogMode = linLogMode;
    emit linLogModeChanged();
    updateCamera([this, linLogMode]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                setUpdating(false);
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LinLog.Mode"), linLogMode)));
            setUpdating(false);
            setReadyToPersist(true);
        });
}

void HardwareRoiController::updateLinLogValue1(int linLogValue1)
{
    // TODO: add security check
    if (m_linLogValue1 == linLogValue1)
    {
        return;
    }
    m_linLogValue1 = linLogValue1;
    emit linLogValue1Changed();
    updateCamera([this, linLogValue1]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                setUpdating(false);
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LinLog.Value1"), linLogValue1)));
            setUpdating(false);
            setReadyToPersist(true);
        });
}

void HardwareRoiController::updateLinLogValue2(int linLogValue2)
{
    // TODO: add security check
    if (m_linLogValue2 == linLogValue2)
    {
        return;
    }
    m_linLogValue2 = linLogValue2;
    emit linLogValue2Changed();
    updateCamera([this, linLogValue2]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                setUpdating(false);
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LinLog.Value2"), linLogValue2)));
            setUpdating(false);
            setReadyToPersist(true);
        });
}

void HardwareRoiController::updateLinLogTime1(int linLogTime1)
{
    // TODO: add security check
    if (m_linLogTime1 == linLogTime1)
    {
        return;
    }
    m_linLogTime1 = linLogTime1;
    emit linLogTime1Changed();
    updateCamera([this, linLogTime1]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                setUpdating(false);
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LinLog.Time1"), linLogTime1)));
            setUpdating(false);
            setReadyToPersist(true);
        });
}

void HardwareRoiController::updateLinLogTime2(int linLogTime2)
{
    // TODO: add security check
    if (m_linLogTime2 == linLogTime2)
    {
        return;
    }
    m_linLogTime2 = linLogTime2;
    emit linLogTime2Changed();
    updateCamera([this, linLogTime2]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                setUpdating(false);
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LinLog.Time2"), linLogTime2)));
            setUpdating(false);
            setReadyToPersist(true);
        });
}

void HardwareRoiController::updateCamera(std::function<void()> updateFunction)
{
    if (!this->grabberDeviceProxy())
    {
        return;
    }
    if (isUpdating())
    {
        return;
    }
    setUpdating(true);
    auto update = [this, updateFunction]
    {
        QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
        connect(watcher, &QFutureWatcher<void>::finished, this,
            [this, watcher]
            {
                watcher->deleteLater();
            }
        );
        watcher->setFuture(QtConcurrent::run(updateFunction));
    };

    m_onReadyFunction = update;
    stopLiveMode();
}

void HardwareRoiController::setNotificationServer(DeviceNotificationServer *server)
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
                auto grabberDeviceProxy = this->grabberDeviceProxy();
                if (!grabberDeviceProxy || id != grabberDeviceProxy->uuid())
                {
                    return;
                }

                if (kv->key() == "Window.X")
                {
                    m_roi = QRect(kv->value<int>(), m_roi.y(), m_roi.width(), m_roi.height());
                    emit roiChanged();
                } else if (kv->key() == "Window.Y")
                {
                    m_roi = QRect(m_roi.x(), kv->value<int>(), m_roi.width(), m_roi.height());
                    emit roiChanged();
                } else if (kv->key() == "Window.W")
                {
                    m_roi.setWidth(kv->value<int>());
                    emit roiChanged();
                } else if (kv->key() == "Window.H")
                {
                    m_roi.setHeight(kv->value<int>());
                    emit roiChanged();
                } else if (kv->key() == "ExposureTime")
                {
                    if (m_exposureTime != kv->value<float>())
                    {
                        m_exposureTime = kv->value<float>();
                        emit exposureTimeChanged();
                    }
                } else if (kv->key() == "Voltages.BlackLevelOffset")
                {
                    if (m_brightness != kv->value<int>())
                    {
                        m_brightness = kv->value<int>();
                        emit brightnessChanged();
                    }
                } else if (kv->key() == "LinLog.Value1")
                {
                    if (m_linLogValue1 != kv->value<int>())
                    {
                        m_linLogValue1 = kv->value<int>();
                        emit linLogValue1Changed();
                    }
                } else if (kv->key() == "LinLog.Value2")
                {
                    if (m_linLogValue2 != kv->value<int>())
                    {
                        m_linLogValue2 = kv->value<int>();
                        emit linLogValue2Changed();
                    }
                } else if (kv->key() == "LinLog.Time1")
                {
                    if (m_linLogTime1 != kv->value<int>())
                    {
                        m_linLogTime1 = kv->value<int>();
                        emit linLogTime1Changed();
                    }
                } else if (kv->key() == "LinLog.Time2")
                {
                    if (m_linLogTime2 != kv->value<int>())
                    {
                        m_linLogTime2 = kv->value<int>();
                        emit linLogTime2Changed();
                    }
                }
            }, Qt::QueuedConnection
        );
    }
    emit notificationServerChanged();
}

void HardwareRoiController::persistToCamera()
{
    QMutexLocker lock{grabberMutex()};
    setReadyToPersist(false);
    lock.unlock();
    // TODO: add security check
    UserLog::instance()->addChange(new HardwareRoiFlushedChangeEntry);
    updateCamera([this]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                setUpdating(false);
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("StoreDefaults"), 1)));
            setUpdating(false);
        });
}

void HardwareRoiController::setReadyToPersist(bool set)
{
    if (m_readyToPersist == set)
    {
        return;
    }
    m_readyToPersist = set;
    emit readyToPersistChanged();
}

}
}
