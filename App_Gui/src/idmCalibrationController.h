#pragma once

#include "event/inspectionCmd.proxy.h"

#include <QObject>

class QTimer;

namespace precitec
{
typedef std::shared_ptr<precitec::interface::TInspectionCmd<precitec::interface::EventProxy>> InspectionCmdProxy;

namespace gui
{

class IDMCalibrationController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::InspectionCmdProxy inspectionCmdProxy READ inspectionCmdProxy WRITE setInspectionCmdProxy NOTIFY inspectionCmdProxyChanged)
    /**
     * Whether it is possible to perform a OCT-Line Calibration
     **/
    Q_PROPERTY(bool canCalibrate READ canCalibrate NOTIFY inspectionCmdProxyChanged)
    /**
     * @c true while performing the OCT-Line Calibration, otherwise @c false
     **/
    Q_PROPERTY(bool calibrating READ isCalibrating NOTIFY calibratingChanged)

public:
    explicit IDMCalibrationController(QObject *parent = nullptr);
    ~IDMCalibrationController() override;

    InspectionCmdProxy inspectionCmdProxy() const
    {
        return m_inspectionCmdProxy;
    }
    void setInspectionCmdProxy(const InspectionCmdProxy &proxy);

    bool canCalibrate() const
    {
        return inspectionCmdProxy().get();
    }

    bool isCalibrating() const
    {
        return m_calibrating;
    }

    /**
     * Starts a OCT_Line calibration process.
     * @see calibrating
     **/
    Q_INVOKABLE void octLineCalibration();

    /**
     * Tells this controller that the calibration has ended.
     **/
    Q_INVOKABLE void endOctLineCalibration();
    
    Q_INVOKABLE void octTCPCalibration();
    Q_INVOKABLE void endoctTCPCalibration();


Q_SIGNALS:
    void inspectionCmdProxyChanged();
    void calibratingChanged();

private:
    InspectionCmdProxy m_inspectionCmdProxy;
    void setCalibrating(bool set);
    bool m_calibrating = false;
    QTimer *m_octLineCalibrationTimeout;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::IDMCalibrationController*)

