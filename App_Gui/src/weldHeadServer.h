#pragma once

#include <QObject>

#include "event/viWeldHeadPublish.interface.h"

class QMutex;

namespace precitec
{
namespace gui
{

class WeldHeadServer : public QObject, public precitec::interface::TviWeldHeadPublish<precitec::interface::AbstractInterface>
{
    Q_OBJECT
public:
    explicit WeldHeadServer(QObject *parent = nullptr);
    ~WeldHeadServer() override;

    void headError(precitec::interface::HeadAxisID axis, precitec::interface::ErrorCode errorCode, int value) override;
    void headInfo(precitec::interface::HeadAxisID axis, precitec::interface::HeadInfo info) override;
    void headIsReady(precitec::interface::HeadAxisID axis, precitec::interface::MotionMode currentMode) override;
    void headValueReached(precitec::interface::HeadAxisID axis, precitec::interface::MotionMode currentMode, int currentValue) override;
    void trackerInfo(precitec::interface::TrackerInfo p_oTrackerInfo) override;

    /**
     * The current HeadInfo for the y axis.
     * @see yAxisChanged
     **/
    precitec::interface::HeadInfo yAxisInfo() const;

    /**
     * The current HeadInfo for the ScanTracker.
     * @see scanTrackerChanged
     **/
    precitec::interface::HeadInfo scanTracker() const;

Q_SIGNALS:
    /**
     * Emitted whenever the HeadInfo for the yAxis changes.
     **/
    void yAxisChanged();
    /**
     * Emitted whenever the HeadInfo for the ScanTracker changes.
     **/
    void scanTrackerChanged();

private:
    precitec::interface::HeadInfo m_yAxisInfo;
    precitec::interface::HeadInfo m_scanTrackerInfo;
    std::unique_ptr<QMutex> m_mutex;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::WeldHeadServer*)
