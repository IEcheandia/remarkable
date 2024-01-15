#include "idmController.h"
#include "deviceProxyWrapper.h"
#include "deviceNotificationServer.h"
#include "calibrationChangeEntry.h"

#include <precitec/userLog.h>
#include <precitec/userManagement.h>
#include "message/calibration.interface.h"

#include <QTimer>
#include <QFutureWatcher>
#include <QtConcurrentRun>

using namespace precitec::interface;
using precitec::gui::components::userLog::UserLog;
using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

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

IdmController::IdmController(QObject *parent)
    : LiveModeController(parent)
    , m_calibrationTimeout(new QTimer(this))
{
    connect(this, &IdmController::idmDeviceProxyChanged, this,
        [this]
        {
            if (!m_idmDeviceProxy)
            {
                setReady(false);
                return;
            }
            auto idmDeviceProxy = m_idmDeviceProxy;
            auto watcher = new QFutureWatcher<Configuration>{this};
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                [this, watcher]
                {
                    watcher->deleteLater();
                    auto configuration = watcher->result();
                    m_sampleFrequency = getValue(configuration, std::string("SampleFrequency"), 70000);
                    emit sampleFrequencyChanged();
                    m_lampIntensity = getValue(configuration, std::string("LampIntensity"), 8);
                    emit lampIntensityChanged();
                    m_detectionWindowLeft = getValue(configuration, std::string("DetectionWindowLeft"), 400);
                    emit detectionWindowLeftChanged();
                    m_detectionWindowRight = getValue(configuration, std::string("DetectionWindowRight"), 9600);
                    emit detectionWindowRightChanged();
                    m_qualityThreshold = getValue(configuration, std::string("QualityThreshold"), 25);
                    emit qualityThresholdChanged();
                    m_dataAveraging = getValue(configuration, std::string("DataAveraging"), 1);
                    emit dataAveragingChanged();
                    m_spectralAveraging = getValue(configuration, std::string("SpectralAveraging"), 1);
                    emit spectralAveragingChanged();
                    m_scale = getValue(configuration, std::string("Scale (SCA)"), 512);
                    m_scale = (m_scale / 1000) * 1000;
                    emit scaleChanged();
                    m_leftLimit = float(m_detectionWindowLeft)/m_scale;
                    emit leftLimitChanged();
                    m_rightLimit = float(m_detectionWindowRight)/m_scale;
                    emit rightLimitChanged();
                    m_depthSystemOffset = getValue(configuration, std::string("WeldingDepthSystemOffset"),0);
                    emit depthSystemOffsetChanged();
                    setReady(true);
                }
            );
            watcher->setFuture(QtConcurrent::run(
                [idmDeviceProxy]
                {
                    return idmDeviceProxy->deviceProxy()->get();
                }));
        }
    );

    connect(this, &IdmController::scaleChanged, this, &IdmController::detectionWindowLeftChanged);
    connect(this, &IdmController::scaleChanged, this, &IdmController::detectionWindowRightChanged);

    m_calibrationTimeout->setSingleShot(true);
    m_calibrationTimeout->setInterval(std::chrono::seconds{10});
    connect(m_calibrationTimeout, &QTimer::timeout, this, &IdmController::endCalibration);
}

IdmController::~IdmController() = default;

void IdmController::setReady(bool set)
{
    if (m_ready == set)
    {
        return;
    }
    m_ready = set;
    emit readyChanged();
}

void IdmController::setUpdating(bool set)
{
    if (m_updating == set)
    {
        return;
    }
    m_updating = set;
    emit updatingChanged();
}

void IdmController::setCalibrating(bool set)
{
    if (m_calibrating == set)
    {
        return;
    }
    m_calibrating = set;
    emit calibratingChanged();
}

void IdmController::updateLampIntensity(int lampIntensity)
{
    if (m_lampIntensity == lampIntensity)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }
    m_lampIntensity = lampIntensity; 

    const auto idmDeviceProxy = m_idmDeviceProxy;
    updateIDM([idmDeviceProxy, lampIntensity]
        {
            idmDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LampIntensity"), lampIntensity)));
        });

    emit lampIntensityChanged();
}

void IdmController::updateSampleFrequency(int sampleFrequency)
{
    if (m_sampleFrequency == sampleFrequency)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }
    m_sampleFrequency = sampleFrequency; 

    const auto idmDeviceProxy = m_idmDeviceProxy;
    updateIDM([idmDeviceProxy, sampleFrequency]
        {
            idmDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("SampleFrequency"), sampleFrequency)));
        });

    emit sampleFrequencyChanged();
}

void IdmController::updateDetectionWindowLeft(int detectionWindowLeft)
{
    if (m_detectionWindowLeft == detectionWindowLeft)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }
    m_detectionWindowLeft = detectionWindowLeft; 

    const auto idmDeviceProxy = m_idmDeviceProxy;
    updateIDM([idmDeviceProxy, detectionWindowLeft]
        {
            idmDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("DetectionWindowLeft"), detectionWindowLeft)));
        });

    emit detectionWindowLeftChanged();

    setLeftLimit(float(m_detectionWindowLeft)/m_scale);
}

void IdmController::updateDetectionWindowRight(int detectionWindowRight)
{
    if (m_detectionWindowRight == detectionWindowRight)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }
    m_detectionWindowRight = detectionWindowRight; 

    const auto idmDeviceProxy = m_idmDeviceProxy;
    updateIDM([idmDeviceProxy, detectionWindowRight]
        {
            idmDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("DetectionWindowRight"), detectionWindowRight)));
        });

    emit detectionWindowRightChanged();

    setRightLimit(float(m_detectionWindowRight)/m_scale);
}

void IdmController::updateQualityThreshold(int qualityThreshold)
{
    if (m_qualityThreshold == qualityThreshold)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }
    m_qualityThreshold = qualityThreshold; 

    const auto idmDeviceProxy = m_idmDeviceProxy;
    updateIDM([idmDeviceProxy, qualityThreshold]
        {
            idmDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("QualityThreshold"), qualityThreshold)));
        });

    emit qualityThresholdChanged();
}

void IdmController::updateDataAveraging(int dataAveraging)
{
    if (m_dataAveraging == dataAveraging)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }
    m_dataAveraging = dataAveraging; 

    const auto idmDeviceProxy = m_idmDeviceProxy;
    updateIDM([idmDeviceProxy, dataAveraging]
        {
            idmDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("DataAveraging"), dataAveraging)));
        });

    emit dataAveragingChanged();
}

void IdmController::updateSpectralAveraging(int spectralAveraging)
{
    if (m_spectralAveraging == spectralAveraging)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }
    m_spectralAveraging = spectralAveraging; 

    const auto idmDeviceProxy = m_idmDeviceProxy;
    updateIDM([idmDeviceProxy, spectralAveraging]
        {
            idmDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("SpectralAveraging"), spectralAveraging)));
        });

    emit spectralAveragingChanged();
}

void IdmController::updateDepthSystemOffset(int depthSystemOffset)
{
    if (m_depthSystemOffset == depthSystemOffset)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }
    m_depthSystemOffset = depthSystemOffset; 

    const auto idmDeviceProxy = m_idmDeviceProxy;
    updateIDM([idmDeviceProxy, depthSystemOffset]
        {
            idmDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("WeldingDepthSystemOffset"), depthSystemOffset)));
        });

    emit depthSystemOffsetChanged();
}

void IdmController::performDarkReference()
{
    if (!hasPermission())
    {
        return;
    }

    if (!canCalibrate())
    {
        return;
    }
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eIDMDarkReference));
    m_calibrationTimeout->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::IDM_DarkReference));
}

void IdmController::setLeftLimit(float leftLimit)
{
    if (m_leftLimit == leftLimit)
    {
        return;
    }

    m_leftLimit = leftLimit;

    emit leftLimitChanged();
}

void IdmController::setRightLimit(float rightLimit)
{
    if (m_rightLimit == rightLimit)
    {
        return;
    }

    m_rightLimit = rightLimit;

    emit rightLimitChanged();
}

void IdmController::updateIDM(std::function<void()> updateFunction)
{
    if (!m_idmDeviceProxy)
    {
        return;
    }
    if (isUpdating())
    {
        return;
    }
    setUpdating(true);
    if (liveMode())
    {
        stopLiveMode();
    }

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            if (liveMode())
            {
                startLiveMode();
            }
            setUpdating(false);
        }
    );
    watcher->setFuture(QtConcurrent::run(updateFunction));
}

void IdmController::setIdmDeviceProxy(DeviceProxyWrapper *device)
{
    if (m_idmDeviceProxy == device)
    {
        return;
    }
    m_idmDeviceProxy = device;
    disconnect(m_idmDeviceDestroyConnection);
    m_idmDeviceDestroyConnection = QMetaObject::Connection{};
    if (m_idmDeviceProxy)
    {
        m_idmDeviceDestroyConnection = connect(m_idmDeviceProxy, &QObject::destroyed, this, std::bind(&IdmController::setIdmDeviceProxy, this, nullptr));
    }

    emit idmDeviceProxyChanged();
}

void IdmController::setNotificationServer(DeviceNotificationServer *server)
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
                if (!m_idmDeviceProxy || id != m_idmDeviceProxy->uuid())
                {
                    return;
                }
                {
                    if (kv->key() == "SampleFrequency")
                    {
                        if (m_sampleFrequency != kv->value<int>())
                        {
                            m_sampleFrequency = kv->value<int>();
                            emit sampleFrequencyChanged();
                        }
                    } else if (kv->key() == "LampIntensity")
                    {
                        if (m_lampIntensity != kv->value<int>())
                        {
                            m_lampIntensity = kv->value<int>();
                            emit lampIntensityChanged();
                        }
                    } else if (kv->key() == "DetectionWindowLeft")
                    {
                        if (m_detectionWindowLeft != kv->value<int>())
                        {
                            m_detectionWindowLeft = kv->value<int>();
                            emit detectionWindowLeftChanged();
                            m_leftLimit = float(m_detectionWindowLeft)/m_scale;
                            emit leftLimitChanged();
                        }
                    } else if (kv->key() == "DetectionWindowRight")
                    {
                        if (m_detectionWindowRight != kv->value<int>())
                        {
                            m_detectionWindowRight = kv->value<int>();
                            emit detectionWindowRightChanged();
                            m_rightLimit = float(m_detectionWindowRight)/m_scale;
                            emit rightLimitChanged();
                        }
                    } else if (kv->key() == "QualityThreshold")
                    {
                        if (m_qualityThreshold != kv->value<int>())
                        {
                            m_qualityThreshold = kv->value<int>();
                            emit qualityThresholdChanged();
                        }
                    } else if (kv->key() == "DataAveraging")
                    {
                        if (m_dataAveraging != kv->value<int>())
                        {
                            m_dataAveraging = kv->value<int>();
                            emit dataAveragingChanged();
                        }
                    } else if (kv->key() == "SpectralAveraging")
                    {
                        if (m_spectralAveraging != kv->value<int>())
                        {
                            m_spectralAveraging = kv->value<int>();
                            emit spectralAveragingChanged();
                        }
                    } else if (kv->key() == "Scale (SCA)")
                    {
                        if (m_scale != kv->value<int>())
                        {
                            m_scale = kv->value<int>();
                            m_scale = (m_scale / 1000) * 1000;
                            emit scaleChanged();
                        }
                    } else if (kv->key() == "WeldingDepthSystemOffset")
                    {
                        if (m_depthSystemOffset != kv->value<int>())
                        {
                            m_depthSystemOffset = kv->value<int>();
                            emit depthSystemOffsetChanged();
                        }
                    }
                }
            }, Qt::QueuedConnection
        );
    }
    emit notificationServerChanged();
}

bool IdmController::hasPermission()
{
    if (!UserManagement::instance()->hasPermission(int(m_idmDeviceProxy->writePermission())))
    {
        return false;
    }
    if (!UserManagement::instance()->hasPermission(int(Permission::EditCalibrationDeviceConfig)))
    {
        return false;
    }
    return true;
}

void IdmController::endCalibration()
{
    if (!isCalibrating())
    {
        return;
    }
    m_calibrationTimeout->stop();
    setCalibrating(false);
}

}
}
