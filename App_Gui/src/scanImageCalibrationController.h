#pragma once

#include "event/inspectionCmd.proxy.h"

#include <QObject>
#include "deviceProxyWrapper.h"

#include <functional>

class QTimer;

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TInspectionCmd<precitec::interface::EventProxy>> InspectionCmdProxy;

namespace storage
{

class SeamSeries;

}
namespace gui
{

class ScanImageCalibrationController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::storage::SeamSeries *seamSeries READ seamSeries WRITE setSeamSeries NOTIFY seamSeriesChanged)

    Q_PROPERTY(precitec::InspectionCmdProxy inspectionCmdProxy READ inspectionCmdProxy WRITE setInspectionCmdProxy NOTIFY inspectionCmdProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *calibrationDevice READ calibrationDevice WRITE setCalibrationDevice NOTIFY calibrationDeviceChanged)

    Q_PROPERTY(bool canCalibrate READ canCalibrate NOTIFY inspectionCmdProxyChanged)

    Q_PROPERTY(bool calibrating READ isCalibrating NOTIFY calibratingChanged)

    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged)

public:
    explicit ScanImageCalibrationController(QObject *parent = nullptr);
    ~ScanImageCalibrationController() override;

    precitec::storage::SeamSeries *seamSeries() const
    {
        return m_seamSeries;
    }
    void setSeamSeries(precitec::storage::SeamSeries *seamSeries);

    InspectionCmdProxy inspectionCmdProxy() const
    {
        return m_inspectionCmdProxy;
    }
    void setInspectionCmdProxy(const InspectionCmdProxy &proxy);

    DeviceProxyWrapper *calibrationDevice() const
    {
        return m_calibrationDevice;
    }
    void setCalibrationDevice(DeviceProxyWrapper *device);

    bool canCalibrate() const
    {
        return inspectionCmdProxy().get();
    }

    bool isCalibrating() const
    {
        return m_calibrating;
    }

    bool isCalibrationReady() const
    {
        return m_calibrationReady;
    }

    bool isCalibrationUpdating() const
    {
        return m_calibrationUpdating;
    }

    bool isEnabled() const
    {
        return m_enabled;
    }
    void setEnabled(bool enabled);

    Q_INVOKABLE void startCalibration();

    Q_INVOKABLE void endCalibration();

Q_SIGNALS:
    void seamSeriesChanged();
    void inspectionCmdProxyChanged();
    void calibrationDeviceChanged();
    void calibratingChanged();
    void calibrationReadyChanged();
    void updatingChanged();
    void enabledChanged();

private:
    precitec::storage::SeamSeries *m_seamSeries = nullptr;
    QMetaObject::Connection m_seamSeriesDestroyed;
    bool hasCalibrationPermission();
    void setCalibrationReady(bool set);
    void setCalibrationUpdating(bool set);
    void setCalibrating(bool set);

    bool m_calibrationReady = false;
    bool m_calibrating = false;
    bool m_calibrationUpdating = false;
    bool m_enabled = false;

    QTimer *m_calibrationTimeout;

    InspectionCmdProxy m_inspectionCmdProxy;

    DeviceProxyWrapper *m_calibrationDevice = nullptr;
    QMetaObject::Connection m_calibrationDeviceDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ScanImageCalibrationController*)



