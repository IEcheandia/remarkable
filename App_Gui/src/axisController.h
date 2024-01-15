#pragma once

#include "liveModeController.h"

#include <QObject>

class QTimer;

namespace precitec
{

namespace gui
{

class AxisInformation;

/**
 * The AxisController allows to modify the axis.
 *
 **/
class AxisController : public LiveModeController
{
    Q_OBJECT
    /**
     * The AxisInformation required to update the axis
     **/
    Q_PROPERTY(precitec::gui::AxisInformation *axisInformation READ axisInformation WRITE setAxisInformation NOTIFY axisInformationChanged)
    /**
     * Whether it is possible to perform a reference run
     **/
    Q_PROPERTY(bool canCalibrate READ canCalibrate NOTIFY inspectionCmdProxyChanged)
    /**
     * @c true while performing a reference run, otherwise @c false
     * @see referenceRun
     **/
    Q_PROPERTY(bool calibrating READ isCalibrating NOTIFY calibratingChanged)
    /**
     * Whether ther controller is currently updating the hardware.
     **/
    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)

public:
    explicit AxisController(QObject *parent = nullptr);
    ~AxisController() override;

    AxisInformation *axisInformation() const
    {
        return m_axisInformation;
    }
    void setAxisInformation(AxisInformation *information);

    bool canCalibrate() const
    {
        return inspectionCmdProxy().get();
    }

    bool isCalibrating() const
    {
        return m_calibrating;
    }

    bool isUpdating() const
    {
        return m_updating;
    }

    /**
     * Starts a reference run of the axis, that is calibrates the axis.
     * @see calibrating
     **/
    Q_INVOKABLE void referenceRun();

    /**
     * Tells this controller that the reference run ended.
     **/
    Q_INVOKABLE void endReferenceRun();

    /**
     * Moves the axis to the @p targetPos.
     **/
    Q_INVOKABLE void moveAxis(int targetPos);

    /**
     * Sets the lower software @p limit.
     **/
    Q_INVOKABLE void setLowerLimit(int limit);

    /**
     * Sets the upper software @p limit.
     **/
    Q_INVOKABLE void setUpperLimit(int limit);

    /**
     * Enables or disables software limits.
     **/
    Q_INVOKABLE void enableSoftwareLimits(bool enable);

    Q_INVOKABLE void toggleHomingDirectionPositive(bool set);

Q_SIGNALS:
    void axisInformationChanged();
    void calibratingChanged();
    void updatingChanged();

private:
    void setUpdating(bool set);
    void setCalibrating(bool set);
    void performAction(std::function<QFuture<void>()> f);
    AxisInformation *m_axisInformation = nullptr;
    bool m_calibrating = false;
    bool m_updating = false;
    QTimer *m_referenceRunTimeout;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::AxisController*)
