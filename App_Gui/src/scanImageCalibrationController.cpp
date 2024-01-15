#include "scanImageCalibrationController.h"
#include "seamSeries.h"

#include "message/calibration.interface.h"
#include "math/CalibrationParamMap.h"
#include "calibrationChangeEntry.h"

#include <precitec/userManagement.h>
#include <precitec/userLog.h>

#include <QFutureWatcher>
#include <QtConcurrentRun>
#include <QTimer>
#include <QDebug>

using precitec::storage::SeamSeries;
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

ScanImageCalibrationController::ScanImageCalibrationController(QObject *parent)
    : QObject(parent)
    , m_calibrationTimeout(new QTimer(this))
{
    m_calibrationTimeout->setSingleShot(true);
    m_calibrationTimeout->setInterval(std::chrono::seconds{180});

    connect(m_calibrationTimeout, &QTimer::timeout, this, &ScanImageCalibrationController::endCalibration);
    connect(this, &ScanImageCalibrationController::calibrationDeviceChanged, this,
            [this]
            {
                setCalibrationReady(false);
                if (!m_calibrationDevice)
                {
                    return;
                }
                if (!UserManagement::instance()->hasPermission(int(m_calibrationDevice->readPermission())))
                {
                    return;
                }
                setCalibrationReady(true);
            }
    );
}

ScanImageCalibrationController::~ScanImageCalibrationController() = default;

void ScanImageCalibrationController::setSeamSeries(SeamSeries *seamSeries)
{
    if (m_seamSeries == seamSeries)
    {
        return;
    }
    m_seamSeries = seamSeries;
    disconnect(m_seamSeriesDestroyed);
    if (m_seamSeries)
    {
        m_seamSeriesDestroyed = connect(m_seamSeries, &SeamSeries::destroyed, this, std::bind(&ScanImageCalibrationController::setSeamSeries, this, nullptr));
    } else
    {
        m_seamSeriesDestroyed = QMetaObject::Connection{};
    }
    emit seamSeriesChanged();
}

void ScanImageCalibrationController::setInspectionCmdProxy(const InspectionCmdProxy &proxy)
{
    if (m_inspectionCmdProxy == proxy)
    {
        return;
    }
    m_inspectionCmdProxy = proxy;
    emit inspectionCmdProxyChanged();
}

void ScanImageCalibrationController::setCalibrationDevice(DeviceProxyWrapper *device)
{
    if (m_calibrationDevice == device)
    {
        return;
    }
    disconnect(m_calibrationDeviceDestroyed);
    m_calibrationDevice = device;
    if (m_calibrationDevice)
    {
        m_calibrationDeviceDestroyed = connect(device, &DeviceProxyWrapper::destroyed, this, std::bind(&ScanImageCalibrationController::setCalibrationDevice, this, nullptr));
    } else
    {
        m_calibrationDeviceDestroyed = {};
    }
    emit calibrationDeviceChanged();
}

void ScanImageCalibrationController::startCalibration()
{
    if (!m_seamSeries || !m_enabled || !hasCalibrationPermission() || !m_calibrationDevice || !isCalibrationReady() || isCalibrationUpdating() || !inspectionCmdProxy())
    {
        return;
    }
    setCalibrationUpdating(true);

    QFutureWatcher<void> *updateWatcher = new QFutureWatcher<void>(this);
    connect(updateWatcher, &QFutureWatcher<void>::finished, this,
            [this, updateWatcher]
            {
                updateWatcher->deleteLater();
                setCalibrationUpdating(false);

                setCalibrating(true);
                QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eAcquireScanFieldImage) );
                m_calibrationTimeout->start();
                UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::ScanfieldImage));
            }
    );

    const auto imagePath = m_seamSeries->uuid().toString(QUuid::WithoutBraces).toStdString();
    const auto calibrationDevice = m_calibrationDevice;
    updateWatcher->setFuture(QtConcurrent::run([calibrationDevice, imagePath]
    {
        calibrationDevice->setKeyValue(SmpKeyValue(new TKeyValue<std::string>("SM_ScanFieldPath", imagePath)));
    }));

    QFutureWatcher<void> *calibrationWatcher = new QFutureWatcher<void>(this);
    connect(calibrationWatcher, &QFutureWatcher<void>::finished, this,
            [this, calibrationWatcher]
            {
                calibrationWatcher->deleteLater();
                setCalibrationUpdating(false);
            }
    );
}

void ScanImageCalibrationController::endCalibration()
{
    if (!isCalibrating())
    {
        return;
    }
    m_calibrationTimeout->stop();
    setCalibrating(false);
}

void ScanImageCalibrationController::setCalibrating(bool set)
{
    if (m_calibrating == set)
    {
        return;
    }
    m_calibrating = set;
    emit calibratingChanged();
}

void ScanImageCalibrationController::setCalibrationReady(bool set)
{
    if (m_calibrationReady == set)
    {
        return;
    }
    m_calibrationReady = set;
    emit calibrationReadyChanged();
}

void ScanImageCalibrationController::setCalibrationUpdating(bool set)
{
    if (m_calibrationUpdating == set)
    {
        return;
    }
    m_calibrationUpdating = set;
    emit updatingChanged();
}

bool ScanImageCalibrationController::hasCalibrationPermission()
{
    if (!UserManagement::instance()->hasPermission(int(m_calibrationDevice->writePermission())))
    {
        return false;
    }
    if (!UserManagement::instance()->hasPermission(int(Permission::EditCalibrationDeviceConfig)))
    {
        return false;
    }
    return true;
}

void ScanImageCalibrationController::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
    {
        return;
    }
    m_enabled = enabled;
    emit enabledChanged();
}

}
}


