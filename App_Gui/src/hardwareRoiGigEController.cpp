#include "hardwareRoiGigEController.h"
#include "hardwareParameters.h"
#include "deviceProxyWrapper.h"
#include "deviceNotificationServer.h"
#include "product.h"
#include "productModel.h"
#include "../../App_Storage/src/compatibility.h"
#include "attributeModel.h"
#include "parameterSet.h"
#include "parameter.h"

#include <QFutureWatcher>
#include <QProcess>
#include <QtConcurrentRun>

using precitec::storage::ProductModel;
using namespace precitec::interface;

namespace precitec
{
namespace gui
{

static const std::string s_liquidLensKey{"LiquidLensPosition"};

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

HardwareRoiGigEController::HardwareRoiGigEController(QObject *parent)
    : LiveModeController(parent)
    , m_parameterSet{new storage::ParameterSet{QUuid::createUuid(), this}}
{
    connect(this, &HardwareRoiGigEController::grabberDeviceProxyChanged, this,
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
                    m_maxSize = QSize{getValue(configuration, std::string("Window.WMax"), 1024), getValue(configuration, std::string("Window.HMax"), 1024)};
                    emit maxSizeChanged();
                    m_roi = QRect(getValue(configuration, std::string("Window.X"), 0),
                                  getValue(configuration, std::string("Window.Y"), 0),
                                  getValue(configuration, std::string("Window.W"), 512),
                                  getValue(configuration, std::string("Window.H"), 512));
                    emit roiChanged();
                    m_exposureTime = getValue(configuration, std::string("ExposureTime"), 2.0f);
                    emit exposureTimeChanged();
                    m_brightness = getValue(configuration, std::string("Voltages.BlackLevelOffset"), 14);
                    emit brightnessChanged();
                    m_linLogValue1 = getValue(configuration, std::string("LinLog.Value1"), 25);
                    emit linLogValue1Changed();
                    m_linLogValue2 = getValue(configuration, std::string("LinLog.Value2"), 26);
                    emit linLogValue2Changed();
                    m_linLogTime1 = getValue(configuration, std::string("LinLog.Time1"), 27);
                    emit linLogTime1Changed();
                    m_linLogTime2 = getValue(configuration, std::string("LinLog.Time2"), 28);
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
    connect(this, &HardwareRoiGigEController::systemStateNormal, this,
        [this]
        {
            if (m_onReadyFunction)
            {
                QFutureWatcher<void> *watcher = new QFutureWatcher<void>{this};
                connect(watcher, &QFutureWatcher<void>::finished, this,
                    [this, watcher]
                    {
                        watcher->deleteLater();
                        m_onReadyFunction = std::function<void()>{};
                        startLiveMode();
                    }
                );
                watcher->setFuture(QtConcurrent::run(m_onReadyFunction));
            }
        }
    );

    connect(this, &HardwareRoiGigEController::weldHeadDeviceProxyChanged, this, &HardwareRoiGigEController::initLiquidLensParameter);
}

HardwareRoiGigEController::~HardwareRoiGigEController() = default;

void HardwareRoiGigEController::setReady(bool set)
{
    if (m_ready == set)
    {
        return;
    }
    m_ready = set;
    emit readyChanged();
}

bool HardwareRoiGigEController::isRectValid(const QRect &rect) const
{
    return QRect{{0, 0}, m_maxSize}.contains(rect);
}

void HardwareRoiGigEController::updateToFullFrame()
{
    updateCameraGeometry({{0, 0}, m_maxSize});
}

void HardwareRoiGigEController::updateCameraGeometry(const QRect &rect)
{
    // TODO: add security check
    if (!isRectValid(rect))
    {
        return;
    }
    updateCamera([this, rect]
        {
            QMutexLocker lock{grabberMutex()};
            auto grabberDeviceProxy = this->grabberDeviceProxy();
            if (!grabberDeviceProxy)
            {
                return;
            }
            grabberDeviceProxy->setKeyValues({
                SmpKeyValue(new TKeyValue<int>(std::string("Window.X"), rect.x())),
                SmpKeyValue(new TKeyValue<int>(std::string("Window.Y"), rect.y())),
                SmpKeyValue(new TKeyValue<int>(std::string("Window.W"), rect.width())),
                SmpKeyValue(new TKeyValue<int>(std::string("Window.H"), rect.height()))
            });
        });
}

void HardwareRoiGigEController::updateExposureTime(qreal time)
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
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<float>(std::string("ExposureTime"), time)));
        });
}

void HardwareRoiGigEController::updateBrightness(int brightness)
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
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("Voltages.BlackLevelOffset"), brightness)));
        });
}

void HardwareRoiGigEController::updateLinLogValue1(int linLogValue1)
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
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LinLog.Value1"), linLogValue1)));
        });
}

void HardwareRoiGigEController::updateLinLogValue2(int linLogValue2)
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
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LinLog.Value2"), linLogValue2)));
        });
}

void HardwareRoiGigEController::updateLinLogTime1(int linLogTime1)
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
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LinLog.Time1"), linLogTime1)));
        });
}

void HardwareRoiGigEController::updateLinLogTime2(int linLogTime2)
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
                return;
            }
            grabberDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LinLog.Time2"), linLogTime2)));
        });
}

void HardwareRoiGigEController::updateCamera(std::function<void()> updateFunction)
{
    auto grabberDeviceProxy = this->grabberDeviceProxy();
    if (!grabberDeviceProxy)
    {
        return;
    }
    startDelayedLiveMode();
    stopLiveMode(false);
    m_onReadyFunction = updateFunction;
}

void HardwareRoiGigEController::setNotificationServer(DeviceNotificationServer *server)
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
                if (m_weldHeadDeviceProxy && m_weldHeadDeviceProxy->uuid() == id)
                {
                    if (kv->key() == s_liquidLensKey)
                    {
                        if (auto parameter = liquidLensPositionParameter())
                        {
                            parameter->setValue(kv->value<double>());
                        }
                    }
                    return;
                }
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

void HardwareRoiGigEController::startPingCamera()
{
    if (m_pingProcess)
    {
        return;
    }
    m_pingProcess = new QProcess{this};
    m_pingProcess->setProgram(QStringLiteral("/bin/ping"));
    m_pingProcess->setArguments({QStringLiteral("192.168.255.2")});
    m_pingProcess->setReadChannel(QProcess::StandardOutput);
    connect(m_pingProcess, &QProcess::readyReadStandardOutput, this,
        [this]
        {
            m_pingOutput.append(QString::fromLocal8Bit(m_pingProcess->readAllStandardOutput()));
            emit pingOutputChanged();
        }
    );
    m_pingProcess->start();
}

void HardwareRoiGigEController::stopPingCamera()
{
    if (!m_pingProcess)
    {
        return;
    }
    m_pingProcess->terminate();
    m_pingProcess->waitForFinished();
    delete m_pingProcess;
    m_pingProcess = nullptr;
    m_pingOutput = QString{};
    emit pingOutputChanged();
}

QString HardwareRoiGigEController::liquidLensPositionLabel() const
{
    return tr(HardwareParameters::instance()->properties(HardwareParameters::Key::LiquidLensPosition).name.c_str());
}

void HardwareRoiGigEController::setAttributeModel(storage::AttributeModel* model)
{
    if (m_attributeModel == model)
    {
        return;
    }
    disconnect(m_attributeModelDestroyed);
    m_attributeModel = model;
    if (m_attributeModel)
    {
        m_attributeModelDestroyed = connect(model, &storage::AttributeModel::destroyed, this, std::bind(&HardwareRoiGigEController::setAttributeModel, this, nullptr));
    }
    else
    {
        m_attributeModelDestroyed = {};
    }

    emit attributeModelChanged();
}

void HardwareRoiGigEController::setWeldHeadDeviceProxy(precitec::gui::DeviceProxyWrapper *weldHeadDevice)
{
    QMutexLocker lock{grabberMutex()};
    if (m_weldHeadDeviceProxy == weldHeadDevice)
    {
        return;
    }
    m_weldHeadDeviceProxy = weldHeadDevice;
    lock.unlock();
    disconnect(m_weldHeadDestroyedConnection);
    if (m_weldHeadDeviceProxy)
    {
        m_weldHeadDestroyedConnection = connect(weldHeadDevice, &DeviceProxyWrapper::destroyed, this, std::bind(&HardwareRoiGigEController::setWeldHeadDeviceProxy, this, nullptr));
    }
    else
    {
        m_weldHeadDestroyedConnection = {};
    }
    emit weldHeadDeviceProxyChanged();
}

void HardwareRoiGigEController::initLiquidLensParameter()
{
    m_parameterSet->clear();
    emit liquidLensPositionParameterChanged();
    if (!m_weldHeadDeviceProxy)
    {
        return;
    }
    auto watcher{new QFutureWatcher<SmpKeyValue>{this}};
    connect(watcher, &QFutureWatcher<SmpKeyValue>::finished, this, [this, watcher]
        {
            watcher->deleteLater();
            m_parameterSet->createParameter(watcher->result(), m_weldHeadDeviceProxy->uuid());
            emit liquidLensPositionParameterChanged();
        });
    watcher->setFuture(QtConcurrent::run(
        [this]
        {
            QMutexLocker lock{grabberMutex()};
            return m_weldHeadDeviceProxy->deviceProxy()->get(s_liquidLensKey);
        }));
}

storage::Parameter* HardwareRoiGigEController::liquidLensPositionParameter() const
{
    if (!m_weldHeadDeviceProxy)
    {
        return nullptr;
    }
    return m_parameterSet->findByNameAndTypeId(QString::fromStdString(s_liquidLensKey), m_weldHeadDeviceProxy->uuid());
}

storage::Attribute* HardwareRoiGigEController::liquidLensPositionAttribute() const
{
    if (!m_attributeModel)
    {
        return nullptr;
    }
    return m_attributeModel->findAttributeByName(QString::fromStdString(s_liquidLensKey));
}

void HardwareRoiGigEController::updateLiquidLensPosition(qreal value)
{
    QtConcurrent::run(
        [this, value]
        {
            QMutexLocker lock{grabberMutex()};
            if (!m_weldHeadDeviceProxy)
            {
                return;
            }
            m_weldHeadDeviceProxy->deviceProxy()->set(SmpKeyValue{new TKeyValue<double>{s_liquidLensKey, value}});
        });
}

}
}
