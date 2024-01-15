#include "scanfieldCalibrationController.h"
#include "permissions.h"
#include "message/calibration.interface.h"
#include "calibrationChangeEntry.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QTimer>
#include <QDir>

#include <precitec/userManagement.h>
#include <precitec/userLog.h>
#include <weldmasterPaths.h>

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

ScanfieldCalibrationController::ScanfieldCalibrationController(QObject *parent)
    : QObject(parent)
    , m_calibrationTimeout(new QTimer(this))
{
    m_calibrationTimeout->setSingleShot(true);
    m_calibrationTimeout->setInterval(std::chrono::seconds{300});
    connect(m_calibrationTimeout, &QTimer::timeout, this, &ScanfieldCalibrationController::endCalibration);
    connect(this, &ScanfieldCalibrationController::calibrationDeviceProxyChanged, this, &ScanfieldCalibrationController::initValues);

    connect(this, &ScanfieldCalibrationController::grabberDeviceProxyChanged, this,
        [this]
        {
            if (!m_grabberDeviceProxy)
            {
                return;
            }
            auto grabberDeviceProxy = m_grabberDeviceProxy;
            QFutureWatcher<Configuration>* watcher = new QFutureWatcher<Configuration>(this);
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                [this, watcher]
                {
                    watcher->deleteLater();
                    auto configuration = watcher->result();

                    m_sensorWidth = getValue(configuration, std::string("Window.W"), 1280);
                    m_sensorHeight = getValue(configuration, std::string("Window.H"), 1024);
                    emit sensorWidthChanged();
                    emit sensorHeightChanged();
                }
            );
            watcher->setFuture(QtConcurrent::run(
                [grabberDeviceProxy]
                {
                    if (!grabberDeviceProxy)
                    {
                        return Configuration{};
                    }
                    return grabberDeviceProxy->deviceProxy()->get();
                })
            );
        }
    );
}

ScanfieldCalibrationController::~ScanfieldCalibrationController() = default;

void ScanfieldCalibrationController::initValues()
{
    if (!m_calibrationDeviceProxy)
    {
        setReady(false);
        return;
    }
    auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    if (!UserManagement::instance()->hasPermission(int(calibrationDeviceProxy->readPermission())))
    {
        return;
    }
    auto watcher = new QFutureWatcher<Configuration>{this};
    connect(watcher, &QFutureWatcher<Configuration>::finished, this,
            [this, watcher]
            {
                watcher->deleteLater();
                auto configuration = watcher->result();

                m_minX = getValue(configuration, std::string("SM_X_min"), -50.0);
                emit minXChanged();
                m_maxX = getValue(configuration, std::string("SM_X_max"), 50.0);
                emit maxXChanged();
                m_minY = getValue(configuration, std::string("SM_Y_min"), -50.0);
                emit minYChanged();
                m_maxY = getValue(configuration, std::string("SM_Y_max"), 50.0);
                emit maxYChanged();

                m_deltaX = getValue(configuration, std::string("SM_deltaX"), 25.0);
                emit deltaXChanged();
                m_deltaY = getValue(configuration, std::string("SM_deltaY"), 25.0);
                emit deltaYChanged();

                m_idmDeltaX = getValue(configuration, std::string("SM_IDMdeltaX"), 5.0);
                emit idmDeltaXChanged();
                m_idmDeltaY = getValue(configuration, std::string("SM_IDMdeltaY"), 5.0);
                emit idmDeltaYChanged();

                m_searchROIX = getValue(configuration, std::string("SM_searchROI_X"), 100);
                emit searchROIXChanged();
                m_searchROIY = getValue(configuration, std::string("SM_searchROI_Y"), 100);

                emit searchROIYChanged();
                m_searchROIW = getValue(configuration, std::string("SM_searchROI_W"), 800);
                emit searchROIWChanged();
                m_searchROIH = getValue(configuration, std::string("SM_searchROI_H"), 800);
                emit searchROIHChanged();

                m_routineRepetitions = getValue(configuration, std::string("SM_CalibRoutineRepetitions"), 3);
                emit routineRepetitionsChanged();

                m_flipX = getValue(configuration, std::string("SM_mirrorX"), false);
                emit flipXChanged();
                m_flipY = getValue(configuration, std::string("SM_mirrorY"), false);
                emit flipYChanged();

                m_adaptiveExposureMode = getValue(configuration, std::string("IDM_AdaptiveExposureMode"), true);
                emit adaptiveExposureModeChanged();

                m_adaptiveExposureBasicValue = getValue(configuration, std::string("IDM_AdaptiveExposureBasicValue"), 50);
                emit adaptiveExposureBasicValueChanged();

                // TODO: ScannerWelding

                setReady(true);
            }
    );
    watcher->setFuture(QtConcurrent::run(
        [this, calibrationDeviceProxy]
        {
            setReady(false);

            return calibrationDeviceProxy->deviceProxy()->get();
        }));
}

void ScanfieldCalibrationController::setInspectionCmdProxy(const InspectionCmdProxy& proxy)
{
    if (m_inspectionCmdProxy == proxy)
    {
        return;
    }
    m_inspectionCmdProxy = proxy;
    emit inspectionCmdProxyChanged();
}

void ScanfieldCalibrationController::setCalibrationDeviceProxy(DeviceProxyWrapper *device)
{
    if (m_calibrationDeviceProxy == device)
    {
        return;
    }
    disconnect(m_calibrationDeviceProxyDestroyed);
    m_calibrationDeviceProxy = device;
    if (m_calibrationDeviceProxy)
    {
        m_calibrationDeviceProxyDestroyed = connect(device, &DeviceProxyWrapper::destroyed, this, std::bind(&ScanfieldCalibrationController::setCalibrationDeviceProxy, this, nullptr));
    } else
    {
        m_calibrationDeviceProxyDestroyed = {};
    }
    emit calibrationDeviceProxyChanged();
}

void ScanfieldCalibrationController::endCalibration()
{
    if (!isCalibrating())
    {
        return;
    }
    m_calibrationTimeout->stop();
    setCalibrating(false);
}

void ScanfieldCalibrationController::setCalibrating(bool set)
{
    if (m_calibrating == set)
    {
        return;
    }
    m_calibrating = set;
    emit calibratingChanged();
}

void ScanfieldCalibrationController::setReady(bool set)
{
    if (m_ready == set)
    {
        return;
    }
    m_ready = set;
    emit readyChanged();
}

void ScanfieldCalibrationController::setUpdating(bool set)
{
    if (m_updating == set)
    {
        return;
    }
    m_updating = set;
    emit updatingChanged();
}

void ScanfieldCalibrationController::updateDevice(std::function<void()> updateFunction)
{
    if (!m_calibrationDeviceProxy)
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

bool ScanfieldCalibrationController::hasPermission()
{
    if (!UserManagement::instance()->hasPermission(int(m_calibrationDeviceProxy->writePermission())))
    {
        return false;
    }
    if (!UserManagement::instance()->hasPermission(int(Permission::EditCalibrationDeviceConfig)))
    {
        return false;
    }
    return true;
}

void ScanfieldCalibrationController::setMinX(qreal minX)
{
    if (qFuzzyCompare(m_minX, minX) || !hasPermission() || !isReady())
    {
        return;
    }
    m_minX = minX;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, minX]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_X_min"), minX)));
    });

    emit minXChanged();
}

void ScanfieldCalibrationController::setMinY(qreal minY)
{
    if (qFuzzyCompare(m_minY, minY) || !hasPermission() || !isReady())
    {
        return;
    }
    m_minY = minY;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, minY]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_Y_min"), minY)));
    });

    emit minYChanged();
}

void ScanfieldCalibrationController::setMaxX(qreal maxX)
{
    if (qFuzzyCompare(m_maxX, maxX) || !hasPermission() || !isReady())
    {
        return;
    }
    m_maxX = maxX;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, maxX]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_X_max"), maxX)));
    });

    emit maxXChanged();
}

void ScanfieldCalibrationController::setMaxY(qreal maxY)
{
    if (qFuzzyCompare(m_maxY, maxY) || !hasPermission() || !isReady())
    {
        return;
    }
    m_maxY = maxY;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, maxY]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_Y_max"), maxY)));
    });

    emit maxYChanged();
}

void ScanfieldCalibrationController::setDeltaX(qreal deltaX)
{
    if (qFuzzyCompare(m_deltaX, deltaX) || !hasPermission() || !isReady())
    {
        return;
    }
    m_deltaX = deltaX;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, deltaX]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_deltaX"), deltaX)));
    });

    emit deltaXChanged();
}

void ScanfieldCalibrationController::setDeltaY(qreal deltaY)
{
    if (qFuzzyCompare(m_deltaY, deltaY) || !hasPermission() || !isReady())
    {
        return;
    }
    m_deltaY = deltaY;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, deltaY]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_deltaY"), deltaY)));
    });

    emit deltaYChanged();
}

void ScanfieldCalibrationController::setIdmDeltaX(qreal idmDeltaX)
{
    if (qFuzzyCompare(m_idmDeltaX, idmDeltaX) || !hasPermission() || !isReady())
    {
        return;
    }
    m_idmDeltaX = idmDeltaX;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, idmDeltaX]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_IDMdeltaX"), idmDeltaX)));
    });

    emit idmDeltaXChanged();
}

void ScanfieldCalibrationController::setIdmDeltaY(qreal idmDeltaY)
{
    if (qFuzzyCompare(m_idmDeltaY, idmDeltaY) || !hasPermission() || !isReady())
    {
        return;
    }
    m_idmDeltaY = idmDeltaY;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, idmDeltaY]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_IDMdeltaY"), idmDeltaY)));
    });

    emit idmDeltaYChanged();
}

void ScanfieldCalibrationController::setSearchROIX(int x)
{
    if (m_searchROIX == x || !hasPermission() || !isReady())
    {
        return;
    }
    m_searchROIX = x;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, x]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("SM_searchROI_X"), x)));
    });

    emit searchROIXChanged();
}

void ScanfieldCalibrationController::setSearchROIY(int y)
{
    if (m_searchROIY == y || !hasPermission() || !isReady())
    {
        return;
    }
    m_searchROIY = y;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, y]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("SM_searchROI_Y"), y)));
    });

    emit searchROIYChanged();
}

void ScanfieldCalibrationController::setSearchROIW(int width)
{
    if (m_searchROIW == width || !hasPermission() || !isReady())
    {
        return;
    }
    m_searchROIW = width;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, width]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("SM_searchROI_W"), width)));
    });

    emit searchROIWChanged();
}

void ScanfieldCalibrationController::setSearchROIH(int height)
{
    if (m_searchROIH == height || !hasPermission() || !isReady())
    {
        return;
    }
    m_searchROIH = height;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, height]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("SM_searchROI_H"), height)));
    });

    emit searchROIHChanged();
}

void ScanfieldCalibrationController::setRoutineRepetitions(int repetitions)
{
    if (m_routineRepetitions == repetitions || !hasPermission() || !isReady())
    {
        return;
    }
    m_routineRepetitions = repetitions;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, repetitions]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("SM_CalibRoutineRepetitions"), repetitions)));
    });

    emit routineRepetitionsChanged();
}

void ScanfieldCalibrationController::setFlipX(bool flipX)
{
    if (m_flipX == flipX || !hasPermission() || !isReady())
    {
        return;
    }
    m_flipX = flipX;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, flipX]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("SM_mirrorX"), flipX)));
    });

    emit flipXChanged();
}

void ScanfieldCalibrationController::setFlipY(bool flipY)
{
    if (m_flipY == flipY || !hasPermission() || !isReady())
    {
        return;
    }
    m_flipY = flipY;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, flipY]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("SM_mirrorY"), flipY)));
    });

    emit flipYChanged();
}

void ScanfieldCalibrationController::setAdaptiveExposureMode(bool adaptiveExposureMode)
{
    if (m_adaptiveExposureMode == adaptiveExposureMode || !hasPermission() || !isReady())
    {
        return;
    }
    m_adaptiveExposureMode = adaptiveExposureMode;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, adaptiveExposureMode]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<bool>(std::string("IDM_AdaptiveExposureMode"), adaptiveExposureMode)));
    });

    emit adaptiveExposureModeChanged();
}

void ScanfieldCalibrationController::setAdaptiveExposureBasicValue(int adaptiveExposureBasicValue)
{
    if (m_adaptiveExposureBasicValue == adaptiveExposureBasicValue || !hasPermission() || !isReady())
    {
        return;
    }
    m_adaptiveExposureBasicValue = adaptiveExposureBasicValue;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, adaptiveExposureBasicValue]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("IDM_AdaptiveExposureBasicValue"), adaptiveExposureBasicValue)));
    });

    emit adaptiveExposureBasicValueChanged();
}

void ScanfieldCalibrationController::setZCollDrivingRelative(double value)
{
    if (m_scannerWelding.zCollDrivingRelative == value || !hasPermission() || !isReady())
    {
        return;
    }
    m_scannerWelding.zCollDrivingRelative = value;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, value]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_ZCollDrivingRelative"), value)));
    });

    emit zCollDrivingRelativeForCalibrationChanged();
}

void ScanfieldCalibrationController::setLaserPowerInPctForCalibration(int laserPower)
{
    if (m_scannerWelding.power == laserPower || !hasPermission() || !isReady())
    {
        return;
    }
    m_scannerWelding.power = laserPower;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, laserPower]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_LaserPowerForCalibration"), laserPower)));
    });

    emit laserPowerInPctForCalibrationChanged();
}

void ScanfieldCalibrationController::setWeldingDurationInMsForCalibration(int weldingDuration)
{
    if (m_scannerWelding.duration == weldingDuration || !hasPermission() || !isReady())
    {
        return;
    }
    m_scannerWelding.duration = weldingDuration;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, weldingDuration]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_WeldingDurationForCalibration"), weldingDuration)));
    });

    emit weldingDurationInMsForCalibrationChanged();
}

void ScanfieldCalibrationController::setJumpSpeedInMmPerSecForCalibration(int jumpSpeed)
{
    if (m_scannerWelding.jumpSpeed == jumpSpeed || !hasPermission() || !isReady())
    {
        return;
    }
    m_scannerWelding.jumpSpeed = jumpSpeed;

    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy, jumpSpeed]
    {
        calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<double>(std::string("SM_JumpSpeedForCalibration"), jumpSpeed)));
    });
    emit jumpSpeedInMmPerSecForCalibrationChanged();
}

void ScanfieldCalibrationController::startTargetImageCalibration()
{
    if (!inspectionCmdProxy())
    {
        return;
    }
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eCalibrateScanFieldTarget));
    m_calibrationTimeout->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::ScanfieldTarget));
}


void ScanfieldCalibrationController::startAcquireScanFieldImage()
{
    if (!inspectionCmdProxy())
    {
        return;
    }
    const auto calibrationDeviceProxy = m_calibrationDeviceProxy;
    updateDevice([calibrationDeviceProxy]
        {
            //define a special scanfieldpath, that does not overwrite seamseries images and makes the debug data be saved
            // see CalibrateScanField::prepareExportFolder()
            calibrationDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<std::string>(std::string("SM_ScanFieldPath"), "preview")));
        });
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eAcquireScanFieldImage));
    m_calibrationTimeout->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::ScanfieldImage));
}
void ScanfieldCalibrationController::startIdmZCalibration()
{
    if (!inspectionCmdProxy())
    {
        return;
    }
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eCalibrateScanFieldIDM_Z));
    m_calibrationTimeout->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::ScanfieldDepth));
}

void ScanfieldCalibrationController::startScannerWeldingCalibration()
{
    if (!inspectionCmdProxy())
    {
        return;
    }
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eScannerCalibration));
    m_calibrationTimeout->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::ScannerWelding));
}

void ScanfieldCalibrationController::startScannerCalibrationMeausure()
{
    if (!inspectionCmdProxy())
    {
        return;
    }
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eScannerCalibrationMeasure));
    m_calibrationTimeout->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::ScannerCalibrationMeasure));
}

void ScanfieldCalibrationController::startCameraCalibration()
{
    if (!inspectionCmdProxy())
    {
        return;
    }
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eScanmasterCameraCalibration));
    m_calibrationTimeout->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::CameraCalibration));
}

void ScanfieldCalibrationController::computeDepthImage()
{
    if (!inspectionCmdProxy())
    {
        return;
    }
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eVerifyScanFieldIDM_Z));
    m_calibrationTimeout->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::DepthImage));
}

QFileInfo ScanfieldCalibrationController::scanfieldDataDirInfo() const
{
    return QFileInfo{WeldmasterPaths::instance()->scanfieldDataDir()};
}

void ScanfieldCalibrationController::deleteScanfieldData()
{
    QDir{WeldmasterPaths::instance()->scanfieldDataDir()}.removeRecursively();
}

void ScanfieldCalibrationController::setGrabberDeviceProxy(DeviceProxyWrapper* device)
{
    if (m_grabberDeviceProxy == device)
    {
        return;
    }

    disconnect(m_grabberDeviceDestroyConnection);

    m_grabberDeviceProxy = device;

    if (m_grabberDeviceProxy)
    {
        m_grabberDeviceDestroyConnection = connect(m_grabberDeviceProxy, &QObject::destroyed, this, std::bind(&ScanfieldCalibrationController::setGrabberDeviceProxy, this, nullptr));
    }
    else
    {
        m_grabberDeviceDestroyConnection = {};
    }

    emit grabberDeviceProxyChanged();
}

}
}

