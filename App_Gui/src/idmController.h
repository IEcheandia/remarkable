#pragma once

#include "liveModeController.h"

#include <QObject>
#include <QPointer>

#include <functional>

class QTimer;

namespace precitec
{

namespace gui
{

class DeviceProxyWrapper;
class DeviceNotificationServer;

/**
 * The IdmController is a small controller class to set the IDM.
 **/
class IdmController : public LiveModeController
{
    Q_OBJECT

    /**
     * The device proxy to the IDM, needed for updating.
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *idmDeviceProxy READ idmDeviceProxy WRITE setIdmDeviceProxy NOTIFY idmDeviceProxyChanged)

    /**
     * Indicates whether this IdmController is fully initialized, that is the current values are read from IDM
     **/
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)

    /**
     * Indicates whether this IdmController can perform a calibration
     **/
    Q_PROPERTY(bool canCalibrate READ canCalibrate NOTIFY inspectionCmdProxyChanged)

    /**
     * Indicates whether the IdmController is currently calibrating
     **/
    Q_PROPERTY(bool calibrating READ isCalibrating NOTIFY calibratingChanged)

    /**
     * The current sample frequency (SHZ)
     **/
    Q_PROPERTY(int sampleFrequency READ sampleFrequency NOTIFY sampleFrequencyChanged)

    /**
     * The current lamp intensity (LAI)
     **/
    Q_PROPERTY(int lampIntensity READ lampIntensity NOTIFY lampIntensityChanged)

    /**
     * The current left side of the detection window (DWD)
     **/
    Q_PROPERTY(int detectionWindowLeft READ detectionWindowLeft NOTIFY detectionWindowLeftChanged)

    /**
     * The current left side of the detection window (DWD) as percentage from scale
     **/
    Q_PROPERTY(float leftLimit READ leftLimit WRITE setLeftLimit NOTIFY leftLimitChanged)

    /**
     * The current right side of the detection window (DWD)
     **/
    Q_PROPERTY(int detectionWindowRight READ detectionWindowRight NOTIFY detectionWindowRightChanged)

    /**
     * The current right side of the detection window (DWD) as percentage from scale
     **/
    Q_PROPERTY(float rightLimit READ rightLimit WRITE setRightLimit NOTIFY rightLimitChanged)

    /**
     * The current quality threshold (QTH)
     **/
    Q_PROPERTY(int qualityThreshold READ qualityThreshold NOTIFY qualityThresholdChanged)

    /**
     * The current data averaging (AVD)
     **/
    Q_PROPERTY(int dataAveraging READ dataAveraging NOTIFY dataAveragingChanged)

    /**
     * The current spectral averaging (AVS)
     **/
    Q_PROPERTY(int spectralAveraging READ spectralAveraging NOTIFY spectralAveragingChanged)

    /**
     * The current scale (SCA)
     **/
    Q_PROPERTY(int scale READ scale NOTIFY scaleChanged)

    /**
     * NotificationServer to get events when the data changed
     **/
    Q_PROPERTY(precitec::gui::DeviceNotificationServer *deviceNotificationServer READ notificationServer WRITE setNotificationServer NOTIFY notificationServerChanged)

    /**
     * Whether the controller is currently updating the hardware.
     **/
    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)
    
    /**
     * The current system offset
     **/
    Q_PROPERTY(int depthSystemOffset READ depthSystemOffset NOTIFY depthSystemOffsetChanged)

public:
    explicit IdmController(QObject *parent = nullptr);
    ~IdmController() override;

    DeviceProxyWrapper *idmDeviceProxy() const
    {
        return m_idmDeviceProxy;
    }
    void setIdmDeviceProxy(DeviceProxyWrapper *device);

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

    int sampleFrequency() const
    {
        return m_sampleFrequency;
    }

    int lampIntensity() const
    {
        return m_lampIntensity;
    }

    int detectionWindowLeft() const
    {
        return m_detectionWindowLeft;
    }

    float leftLimit() const
    {
        return m_leftLimit;
    }
    void setLeftLimit(float leftLimit);

    int detectionWindowRight() const
    {
        return m_detectionWindowRight;
    }

    float rightLimit() const
    {
        return m_rightLimit;
    }
    void setRightLimit(float rightLimit);

    int qualityThreshold() const
    {
        return m_qualityThreshold;
    }

    int dataAveraging() const
    {
        return m_dataAveraging;
    }

    int spectralAveraging() const
    {
        return m_spectralAveraging;
    }

    int scale() const
    {
        return m_scale;
    }
    
    int depthSystemOffset() const
    {
        return m_depthSystemOffset;
    }

    DeviceNotificationServer *notificationServer() const
    {
        return m_notificationServer.data();
    }
    void setNotificationServer(DeviceNotificationServer *server);

    bool isUpdating() const
    {
        return m_updating;
    }

    /**
     * Updates the sample frequency of the IDM to @p sampleFrequency Hz.
     **/
    Q_INVOKABLE void updateSampleFrequency(int sampleFrequency);

    /**
     * Updates the lamp intensity of the IDM to @p lampIntensity percent.
     **/
    Q_INVOKABLE void updateLampIntensity(int lampIntensity);

    /**
     * Updates the left side of the detection window of the IDM to @p detectionWindowLeft micrometers.
     **/
    Q_INVOKABLE void updateDetectionWindowLeft(int detectionWindowLeft);

    /**
     * Updates the right side of the detection window of the IDM to @p detectionWindowRight micrometers.
     **/
    Q_INVOKABLE void updateDetectionWindowRight(int detectionWindowRight);

    /**
     * Updates the quality threshold of the IDM to @p qualityThreshold.
     **/
    Q_INVOKABLE void updateQualityThreshold(int qualityThreshold);

    /**
     * Updates the data averaging of the IDM to @p dataAveraging samples.
     **/
    Q_INVOKABLE void updateDataAveraging(int dataAveraging);

    /**
     * Updates the spectral averaging of the IDM to @p spectralAveraging samples.
     **/
    Q_INVOKABLE void updateSpectralAveraging(int spectralAveraging);
    
    /**
     * Updates the system offset of the IDM Welding Depth signal to @p depthSystemOffset
     **/
    Q_INVOKABLE void updateDepthSystemOffset(int depthSystemOffset);

    Q_INVOKABLE void performDarkReference();

    Q_INVOKABLE void endCalibration();

Q_SIGNALS:
    void idmDeviceProxyChanged();
    void readyChanged();
    void calibratingChanged();
    void notificationServerChanged();
    void updatingChanged();
    void sampleFrequencyChanged();
    void lampIntensityChanged();
    void detectionWindowLeftChanged();
    void leftLimitChanged();
    void detectionWindowRightChanged();
    void rightLimitChanged();
    void qualityThresholdChanged();
    void dataAveragingChanged();
    void spectralAveragingChanged();
    void scaleChanged();
    void depthSystemOffsetChanged();

private:
    bool hasPermission();
    void setReady(bool set);
    void setUpdating(bool set);
    void setCalibrating(bool set);
    void updateIDM(std::function<void()> updateFunction);

    DeviceProxyWrapper *m_idmDeviceProxy = nullptr;
    QMetaObject::Connection  m_idmDeviceDestroyConnection;
    QPointer<DeviceNotificationServer> m_notificationServer;
    QMetaObject::Connection m_notificationConnection;

    bool m_ready = false;
    bool m_calibrating = false;
    bool m_updating = false;
    int m_sampleFrequency = 70000;
    int m_lampIntensity = 8;
    int m_detectionWindowLeft = 400;
    int m_detectionWindowRight = 9600;
    int m_qualityThreshold = 25;
    int m_dataAveraging = 1;
    int m_spectralAveraging = 1;
    int m_scale = 512;
    float m_leftLimit = 0.0f;
    float m_rightLimit = 100.0f;
    int m_depthSystemOffset = 0;

    QTimer* m_calibrationTimeout;
};

}
}

