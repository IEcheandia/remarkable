#pragma once

#include "event/inspectionCmd.proxy.h"

#include <QObject>
#include <QFileInfo>

#include "deviceProxyWrapper.h"

#include <functional>

class QTimer;

namespace precitec
{
typedef std::shared_ptr<precitec::interface::TInspectionCmd<precitec::interface::EventProxy>> InspectionCmdProxy;

namespace gui
{

class ScanfieldCalibrationController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::InspectionCmdProxy inspectionCmdProxy READ inspectionCmdProxy WRITE setInspectionCmdProxy NOTIFY inspectionCmdProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *calibrationDeviceProxy READ calibrationDeviceProxy WRITE setCalibrationDeviceProxy NOTIFY calibrationDeviceProxyChanged)

    Q_PROPERTY(bool canCalibrate READ canCalibrate NOTIFY inspectionCmdProxyChanged)

    Q_PROPERTY(bool calibrating READ isCalibrating NOTIFY calibratingChanged)

    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)

    Q_PROPERTY(qreal minX READ minX WRITE setMinX NOTIFY minXChanged)

    Q_PROPERTY(qreal minY READ minY WRITE setMinY NOTIFY minYChanged)

    Q_PROPERTY(qreal maxX READ maxX WRITE setMaxX NOTIFY maxXChanged)

    Q_PROPERTY(qreal maxY READ maxY WRITE setMaxY NOTIFY maxYChanged)

    Q_PROPERTY(qreal deltaX READ deltaX WRITE setDeltaX NOTIFY deltaXChanged)

    Q_PROPERTY(qreal deltaY READ deltaY WRITE setDeltaY NOTIFY deltaYChanged)

    Q_PROPERTY(qreal idmDeltaX READ idmDeltaX WRITE setIdmDeltaX NOTIFY idmDeltaXChanged)

    Q_PROPERTY(qreal idmDeltaY READ idmDeltaY WRITE setIdmDeltaY NOTIFY idmDeltaYChanged)

    Q_PROPERTY(int searchROIX READ searchROIX WRITE setSearchROIX NOTIFY searchROIXChanged)

    Q_PROPERTY(int searchROIY READ searchROIY WRITE setSearchROIY NOTIFY searchROIYChanged)

    Q_PROPERTY(int searchROIW READ searchROIW WRITE setSearchROIW NOTIFY searchROIWChanged)

    Q_PROPERTY(int searchROIH READ searchROIH WRITE setSearchROIH NOTIFY searchROIHChanged)

    Q_PROPERTY(int sensorWidth READ sensorWidth NOTIFY sensorWidthChanged)

    Q_PROPERTY(int sensorHeight READ sensorHeight NOTIFY sensorHeightChanged)

    Q_PROPERTY(int routineRepetitions READ routineRepetitions WRITE setRoutineRepetitions NOTIFY routineRepetitionsChanged)

    Q_PROPERTY(bool flipX READ flipX WRITE setFlipX NOTIFY flipXChanged)

    Q_PROPERTY(bool flipY READ flipY WRITE setFlipY NOTIFY flipYChanged)

    Q_PROPERTY(bool adaptiveExposureMode READ adaptiveExposureMode WRITE setAdaptiveExposureMode NOTIFY adaptiveExposureModeChanged)

    Q_PROPERTY(int adaptiveExposureBasicValue READ adaptiveExposureBasicValue WRITE setAdaptiveExposureBasicValue NOTIFY adaptiveExposureBasicValueChanged)

    Q_PROPERTY(double zCollDrivingRelative READ zCollDrivingRelative WRITE setZCollDrivingRelative NOTIFY zCollDrivingRelativeForCalibrationChanged)

    Q_PROPERTY(int laserPowerInPctForCalibration READ laserPowerInPctForCalibration WRITE setLaserPowerInPctForCalibration NOTIFY laserPowerInPctForCalibrationChanged)

    Q_PROPERTY(int weldingDurationInMsForCalibration READ weldingDurationInMsForCalibration WRITE setWeldingDurationInMsForCalibration NOTIFY weldingDurationInMsForCalibrationChanged)

    Q_PROPERTY(int jumpSpeedInMmPerSecForCalibration READ jumpSpeedInMmPerSecForCalibration WRITE setJumpSpeedInMmPerSecForCalibration NOTIFY jumpSpeedInMmPerSecForCalibrationChanged)

    Q_PROPERTY(QFileInfo scanfieldDataDirInfo READ scanfieldDataDirInfo CONSTANT)

    /**
     * Proxy to the grabber
     * Needed to acquire the actual sensor size
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* grabberDeviceProxy READ grabberDeviceProxy WRITE setGrabberDeviceProxy NOTIFY grabberDeviceProxyChanged)

public:
    explicit ScanfieldCalibrationController(QObject* parent = nullptr);
    ~ScanfieldCalibrationController() override;

    InspectionCmdProxy inspectionCmdProxy() const
    {
        return m_inspectionCmdProxy;
    }
    void setInspectionCmdProxy(const InspectionCmdProxy& proxy);

    DeviceProxyWrapper* calibrationDeviceProxy() const
    {
        return m_calibrationDeviceProxy;
    }
    void setCalibrationDeviceProxy(DeviceProxyWrapper* device);

    bool canCalibrate() const
    {
        return inspectionCmdProxy().get();
    }

    bool isCalibrating() const
    {
        return m_calibrating;
    }

    bool isReady() const
    {
        return m_ready;
    }

    bool isUpdating() const
    {
        return m_updating;
    }

    qreal minX() const
    {
        return m_minX;
    }
    void setMinX(qreal minX);

    qreal minY() const
    {
        return m_minY;
    }
    void setMinY(qreal minY);

    qreal maxX() const
    {
        return m_maxX;
    }
    void setMaxX(qreal maxX);

    qreal maxY() const
    {
        return m_maxY;
    }
    void setMaxY(qreal maxY);

    qreal deltaX() const
    {
        return m_deltaX;
    }
    void setDeltaX(qreal deltaX);

    qreal deltaY() const
    {
        return m_deltaY;
    }
    void setDeltaY(qreal deltaY);

    qreal idmDeltaX() const
    {
        return m_idmDeltaX;
    }
    void setIdmDeltaX(qreal idmDeltaX);

    qreal idmDeltaY() const
    {
        return m_idmDeltaY;
    }
    void setIdmDeltaY(qreal idmDeltaY);

    int searchROIX() const
    {
        return m_searchROIX;
    }
    void setSearchROIX(int x);

    int searchROIY() const
    {
        return m_searchROIY;
    }
    void setSearchROIY(int y);

    int searchROIW() const
    {
        return m_searchROIW;
    }
    void setSearchROIW(int width);

    int searchROIH() const
    {
        return m_searchROIH;
    }
    void setSearchROIH(int height);

    int sensorWidth() const
    {
        return m_sensorWidth;
    }

    int sensorHeight() const
    {
        return m_sensorHeight;
    }

    int routineRepetitions() const
    {
        return m_routineRepetitions;
    }
    void setRoutineRepetitions(int repetitions);

    bool flipX() const
    {
        return m_flipX;
    }
    void setFlipX(bool flipX);

    bool flipY() const
    {
        return m_flipY;
    }
    void setFlipY(bool flipY);

    bool adaptiveExposureMode() const
    {
        return m_adaptiveExposureMode;
    }
    void setAdaptiveExposureMode(bool adaptiveExposureMode);

    int adaptiveExposureBasicValue() const
    {
        return m_adaptiveExposureBasicValue;
    }
    void setAdaptiveExposureBasicValue(int adaptiveExposureBasicValue);

    void setZCollDrivingRelative(double value);

    double zCollDrivingRelative() const
    {
        return m_scannerWelding.zCollDrivingRelative;
    }

    int laserPowerInPctForCalibration() const
    {
        return m_scannerWelding.power;
    }

    void setLaserPowerInPctForCalibration(int laserPower);

    int weldingDurationInMsForCalibration() const
    {
        return m_scannerWelding.duration;
    }

    void setWeldingDurationInMsForCalibration(int weldingDuration);

    void setJumpSpeedInMmPerSecForCalibration(int jumpSpeed);

    int jumpSpeedInMmPerSecForCalibration() const
    {
        return m_scannerWelding.jumpSpeed;
    }

    QFileInfo scanfieldDataDirInfo() const;

    DeviceProxyWrapper* grabberDeviceProxy() const
    {
        return m_grabberDeviceProxy;
    }
    void setGrabberDeviceProxy(DeviceProxyWrapper* device);

    Q_INVOKABLE void initValues();

    Q_INVOKABLE void startTargetImageCalibration();

    Q_INVOKABLE void startAcquireScanFieldImage();

    Q_INVOKABLE void startIdmZCalibration();

    Q_INVOKABLE void computeDepthImage();

    Q_INVOKABLE void endCalibration();

    Q_INVOKABLE void deleteScanfieldData();

    Q_INVOKABLE void startScannerWeldingCalibration();

    Q_INVOKABLE void startScannerCalibrationMeausure();

    Q_INVOKABLE void startCameraCalibration();

Q_SIGNALS:
    void inspectionCmdProxyChanged();
    void calibrationDeviceProxyChanged();
    void calibratingChanged();
    void readyChanged();
    void updatingChanged();
    void minXChanged();
    void minYChanged();
    void maxXChanged();
    void maxYChanged();
    void deltaXChanged();
    void deltaYChanged();
    void idmDeltaXChanged();
    void idmDeltaYChanged();
    void searchROIXChanged();
    void searchROIYChanged();
    void searchROIWChanged();
    void searchROIHChanged();
    void sensorWidthChanged();
    void sensorHeightChanged();
    void routineRepetitionsChanged();
    void flipXChanged();
    void flipYChanged();
    void adaptiveExposureModeChanged();
    void adaptiveExposureBasicValueChanged();
    void zCollDrivingRelativeForCalibrationChanged();
    void laserPowerInPctForCalibrationChanged();
    void weldingDurationInMsForCalibrationChanged();
    void grabberDeviceProxyChanged();
    void jumpSpeedInMmPerSecForCalibrationChanged();

private:
    bool hasPermission();
    void setReady(bool set);
    void setUpdating(bool set);
    void setCalibrating(bool set);
    void updateDevice(std::function<void()> updateFunction);

    bool m_ready = false;
    bool m_calibrating = false;
    bool m_updating = false;

    qreal m_minX = -50.0;
    qreal m_maxX = 50.0;
    qreal m_minY = -50.0;
    qreal m_maxY = 50.0;

    qreal m_deltaX = 25.0;
    qreal m_deltaY = 25.0;

    qreal m_idmDeltaX = 5.0;
    qreal m_idmDeltaY = 5.0;

    int m_searchROIX = 100;
    int m_searchROIY = 100;
    int m_searchROIW = 800;
    int m_searchROIH = 800;
    int m_sensorWidth =  1280;
    int m_sensorHeight =  1024;

    int m_routineRepetitions = 3;

    bool m_flipX = false;
    bool m_flipY = false;

    bool m_adaptiveExposureMode = true;
    int m_adaptiveExposureBasicValue = 50;

    QTimer *m_calibrationTimeout;

    struct ScannerWeldingCalibration {
        double zCollDrivingRelative = 0.;
        int power = 15;
        int duration = 50;
        int jumpSpeed = 250;
    };
    ScannerWeldingCalibration m_scannerWelding;

    InspectionCmdProxy m_inspectionCmdProxy;

    DeviceProxyWrapper *m_calibrationDeviceProxy = nullptr;
    QMetaObject::Connection m_calibrationDeviceProxyDestroyed;

    DeviceProxyWrapper* m_grabberDeviceProxy = nullptr;
    QMetaObject::Connection m_grabberDeviceDestroyConnection;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ScanfieldCalibrationController*)
