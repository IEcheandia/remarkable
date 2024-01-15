#include "axisController.h"
#include "axisInformation.h"
#include "deviceNotificationServer.h"
#include "deviceProxyWrapper.h"
#include "message/calibration.interface.h"
#include "referenceRunYAxisChangeEntry.h"

#include <precitec/userLog.h>

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

using components::userLog::UserLog;

AxisController::AxisController(QObject* parent)
    : LiveModeController(parent)
    , m_referenceRunTimeout(new QTimer(this))
{
    m_referenceRunTimeout->setSingleShot(true);
    m_referenceRunTimeout->setInterval(std::chrono::seconds{30});
    connect(m_referenceRunTimeout, &QTimer::timeout, this, &AxisController::endReferenceRun);
}

AxisController::~AxisController() = default;

void AxisController::setAxisInformation(AxisInformation *information)
{
    if (m_axisInformation == information)
    {
        return;
    }
    m_axisInformation = information;
    emit axisInformationChanged();
}

void AxisController::referenceRun()
{
    if (!inspectionCmdProxy())
    {
        return;
    }
    setCalibrating(true);
    auto watcher = new QFutureWatcher<void>{this};
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            QtConcurrent::run(inspectionCmdProxy().get(), &precitec::interface::TInspectionCmd<precitec::interface::EventProxy>::requestCalibration, int(precitec::interface::eHomingY));
            m_referenceRunTimeout->start();
            UserLog::instance()->addChange(new ReferenceRunYAxisChangeEntry);
        }
    );
    watcher->setFuture(stopLiveMode());
}

void AxisController::endReferenceRun()
{
    if (!isCalibrating())
    {
        return;
    }
    m_referenceRunTimeout->stop();
    if (liveMode())
    {
        startLiveMode();
    }
    setCalibrating(false);
}

void AxisController::setCalibrating(bool set)
{
    if (m_calibrating == set)
    {
        return;
    }
    m_calibrating = set;
    emit calibratingChanged();
}

void AxisController::moveAxis(int targetPos)
{
    performAction(std::bind(&AxisInformation::moveAxis, m_axisInformation, targetPos));
}

void AxisController::setLowerLimit(int limit)
{
    performAction(std::bind(&AxisInformation::setLowerLimit, m_axisInformation, limit));
}

void AxisController::setUpperLimit(int limit)
{
    performAction(std::bind(&AxisInformation::setUpperLimit, m_axisInformation, limit));
}

void AxisController::enableSoftwareLimits(bool enable)
{
    performAction(std::bind(&AxisInformation::enableSoftwareLimits, m_axisInformation, enable));
}

void AxisController::toggleHomingDirectionPositive(bool set)
{
    performAction(std::bind(&AxisInformation::toggleHomingDirectionPositive, m_axisInformation, set));
}

void AxisController::performAction(std::function<QFuture<void>()> f)
{
    if (!m_axisInformation)
    {
        return;
    }
    if (liveMode())
    {
        stopLiveMode();
    }
    setUpdating(true);
    auto watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished,
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
    watcher->setFuture(f());
}

void AxisController::setUpdating(bool set)
{
    if (m_updating == set)
    {
        return;
    }
    m_updating = set;
    emit updatingChanged();
}

}
}
