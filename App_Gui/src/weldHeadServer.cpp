#include "weldHeadServer.h"

#include <QMutex>

using namespace precitec::interface;

namespace precitec
{
namespace gui
{

WeldHeadServer::WeldHeadServer(QObject *parent)
    : QObject(parent)
    , m_mutex(std::make_unique<QMutex>())
{
}

WeldHeadServer::~WeldHeadServer() = default;

void WeldHeadServer::headError(HeadAxisID axis, ErrorCode errorCode, int value)
{
    Q_UNUSED(axis)
    Q_UNUSED(errorCode)
    Q_UNUSED(value)
}

void WeldHeadServer::headInfo(HeadAxisID axis, HeadInfo info)
{
    if (axis == eAxisY)
    {
        {
            QMutexLocker lock(m_mutex.get());
            m_yAxisInfo = info;
        }
        emit yAxisChanged();
    }
    if (axis == eAxisTracker)
    {
        {
            QMutexLocker lock(m_mutex.get());
            m_scanTrackerInfo = info;
        }
        emit scanTrackerChanged();
    }
}

void WeldHeadServer::headIsReady(HeadAxisID axis, MotionMode currentMode)
{
    Q_UNUSED(axis)
    Q_UNUSED(currentMode)
}

void WeldHeadServer::headValueReached(HeadAxisID axis, MotionMode currentMode, int currentValue)
{
    Q_UNUSED(axis)
    Q_UNUSED(currentMode)
    Q_UNUSED(currentValue)
}

void WeldHeadServer::trackerInfo(TrackerInfo p_oTrackerInfo)
{
    Q_UNUSED(p_oTrackerInfo)
}

HeadInfo WeldHeadServer::yAxisInfo() const
{
    QMutexLocker lock(m_mutex.get());
    return m_yAxisInfo;
}

HeadInfo WeldHeadServer::scanTracker() const
{
    QMutexLocker lock(m_mutex.get());
    return m_scanTrackerInfo;
}

}
}
