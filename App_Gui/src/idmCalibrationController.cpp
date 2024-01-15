#include "idmCalibrationController.h"

#include "deviceProxyWrapper.h"
#include "message/calibration.interface.h"
#include "calibrationChangeEntry.h"

#include <QtConcurrentRun>
#include <QTimer>

#include <precitec/userLog.h>

namespace precitec
{
namespace gui
{

using components::userLog::UserLog;

IDMCalibrationController::IDMCalibrationController(QObject *parent)
    : QObject(parent)
    , m_octLineCalibrationTimeout(new QTimer(this))
{
    m_octLineCalibrationTimeout->setSingleShot(true);
    m_octLineCalibrationTimeout->setInterval(std::chrono::seconds{30});
    connect(m_octLineCalibrationTimeout, &QTimer::timeout, this, &IDMCalibrationController::endOctLineCalibration);
}

IDMCalibrationController::~IDMCalibrationController() = default;

void IDMCalibrationController::octLineCalibration()
{
    if (!inspectionCmdProxy())
    {
        return;
    }
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, 
                      int(precitec::interface::eOCT_LineCalibration));
    m_octLineCalibrationTimeout->start();
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::IDM_Line));
}

void IDMCalibrationController::setInspectionCmdProxy(const InspectionCmdProxy &proxy)
{
    if (m_inspectionCmdProxy == proxy)
    {
        return;
    }
    m_inspectionCmdProxy = proxy;
    emit inspectionCmdProxyChanged();
}

void IDMCalibrationController::endOctLineCalibration()
{
    if (!isCalibrating())
    {
        return;
    }
    m_octLineCalibrationTimeout->stop();
    setCalibrating(false);
}

void IDMCalibrationController::setCalibrating(bool set)
{
    if (m_calibrating == set)
    {
        return;
    }
    m_calibrating = set;
    emit calibratingChanged();
}

void IDMCalibrationController::octTCPCalibration()
{
    
    if (!inspectionCmdProxy())
    {
        return;
    }
    setCalibrating(true);
    QtConcurrent::run(inspectionCmdProxy().get(), 
                      &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, 
                      int(interface::eOCT_TCPCalibration));
    UserLog::instance()->addChange(new CalibrationChangeEntry(CalibrationType::IDM_TCP));
}

void IDMCalibrationController::endoctTCPCalibration()
{
    if (!isCalibrating())
    {
        return;
    }
    setCalibrating(false);
}




}
}
