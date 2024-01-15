#pragma once

#include "liveModeController.h"

#include <QObject>
#include <QMutex>
#include <QPointer>
#include <QRect>

#include "event/inspectionCmd.proxy.h"

#include <functional>

namespace precitec
{

namespace storage
{
class ProductModel;
}

namespace gui
{

class DeviceProxyWrapper;
class DeviceNotificationServer;

/**
 * The HardwareRoiController is a small controller class to set the Hardware ROI of the camera.
 **/
class HardwareRoiController : public LiveModeController
{
    Q_OBJECT
    /**
     * Indicates whether this HardwareRoiController is fully initialized, that is the current values are read from grabber
     **/
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
    /**
     * The current roi as reported by the camera/grabber
     **/
    Q_PROPERTY(QRect roi READ roi NOTIFY roiChanged)
    /**
     * The exposure time of the camera.
     **/
    Q_PROPERTY(qreal exposureTime READ exposureTime NOTIFY exposureTimeChanged)
    /**
     * The black level offset of the camera.
     **/
    Q_PROPERTY(qreal brightness READ brightness NOTIFY brightnessChanged)
    /**
     * The lin log mode of the camera.
     **/
    Q_PROPERTY(qreal linLogMode READ linLogMode NOTIFY linLogModeChanged)
    /**
     * The lin log value 1 of the camera.
     **/
    Q_PROPERTY(qreal linLogValue1 READ linLogValue1 NOTIFY linLogValue1Changed)
    /**
     * The lin log value 2 of the camera.
     **/
    Q_PROPERTY(qreal linLogValue2 READ linLogValue2 NOTIFY linLogValue2Changed)
    /**
     * The lin log time 1 of the camera.
     **/
    Q_PROPERTY(qreal linLogTime1 READ linLogTime1 NOTIFY linLogTime1Changed)
    /**
     * The lin log time 2 of the camera.
     **/
    Q_PROPERTY(qreal linLogTime2 READ linLogTime2 NOTIFY linLogTime2Changed)

    /**
     * NotificationServer to get events when the data changed
     **/
    Q_PROPERTY(precitec::gui::DeviceNotificationServer *deviceNotificationServer READ notificationServer WRITE setNotificationServer NOTIFY notificationServerChanged)

    /**
     * Whether ther controller is currently updating the hardware.
     **/
    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)

    /**
     * Whether the system can allow persist to flash memory
     **/
    Q_PROPERTY(bool readyToPersist READ isReadyToPersist NOTIFY readyToPersistChanged)
public:
    explicit HardwareRoiController(QObject *parent = nullptr);
    ~HardwareRoiController() override;

    bool isReady() const
    {
        return m_ready;
    }

    QRect roi() const
    {
        return m_roi;
    }

    qreal exposureTime() const
    {
        return m_exposureTime;
    }

    int brightness() const
    {
        return m_brightness;
    }

    int linLogMode() const
    {
        return m_linLogMode;
    }

    int linLogValue1() const
    {
        return m_linLogValue1;
    }

    int linLogValue2() const
    {
        return m_linLogValue2;
    }

    int linLogTime1() const
    {
        return m_linLogTime1;
    }

    int linLogTime2() const
    {
        return m_linLogTime2;
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

    bool isReadyToPersist() const
    {
        return m_readyToPersist;
    }

    /**
     * @returns @c true if the @p rect is a valid rect for the camera.
     **/
    Q_INVOKABLE bool isRectValid(const QRect &rect) const;

    /**
     * Updates the camera geometry to the maximum geometry.
     **/
    Q_INVOKABLE void updateToFullFrame();
    /**
     * Updates the camera to the given @p rect.
     **/
    Q_INVOKABLE void updateCameraGeometry(const QRect &rect);

    /**
     * Updates the exposure time of the camera to @p time ms.
     **/
    Q_INVOKABLE void updateExposureTime(qreal time);

    /**
     * Updates the black level offset of the camera to @p brightness.
     **/
    Q_INVOKABLE void updateBrightness(int brightness);

    /**
     * Updates the lin log mode of the camera to @p linLogMode.
     **/
    Q_INVOKABLE void updateLinLogMode(int linLogMode);

    /**
     * Updates the lin log value 1 of the camera to @p linLogValue1.
     **/
    Q_INVOKABLE void updateLinLogValue1(int linLogValue1);

    /**
     * Updates the lin log value 2 of the camera to @p linLogValue2.
     **/
    Q_INVOKABLE void updateLinLogValue2(int linLogValue2);

    /**
     * Updates the lin log time 1 of the camera to @p linLogTime1.
     **/
    Q_INVOKABLE void updateLinLogTime1(int linLogTime1);

    /**
     * Updates the lin log time 2 of the camera to @p linLogTime2.
     **/
    Q_INVOKABLE void updateLinLogTime2(int linLogTime2);

    /**
     * Persists the current settings to the camera
     **/
    Q_INVOKABLE void persistToCamera();

Q_SIGNALS:
    void readyChanged();
    void roiChanged();
    void exposureTimeChanged();
    void brightnessChanged();
    void linLogModeChanged();
    void linLogValue1Changed();
    void linLogValue2Changed();
    void linLogTime1Changed();
    void linLogTime2Changed();
    void notificationServerChanged();
    void updatingChanged();
    void readyToPersistChanged();

private:
    void setReady(bool set);
    void setReadyToPersist(bool set);
    void updateCamera(std::function<void()> updateFunction);
    void setUpdating(bool set);
    bool m_ready = false;
    QRect m_roi{0, 0, 512, 512};
    qreal m_exposureTime = 2.0;
    int m_brightness = 3300;
    int m_linLogMode = 1;
    int m_linLogValue1 = 30;
    int m_linLogValue2 = 0;
    int m_linLogTime1 = 0;
    int m_linLogTime2 = 1000;
    QPointer<DeviceNotificationServer> m_notificationServer;
    QMetaObject::Connection m_notificationConnection;
    bool m_updating = false;
    std::function<void()> m_onReadyFunction;
    bool m_exitUpdatingOnNextReady = false;
    bool m_readyToPersist = false;
};

}
}
