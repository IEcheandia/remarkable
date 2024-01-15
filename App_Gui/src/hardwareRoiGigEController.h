#pragma once

#include "liveModeController.h"

#include <QObject>
#include <QMutex>
#include <QPointer>
#include <QRect>
#include <QSize>

#include "event/inspectionCmd.proxy.h"

#include <functional>

class QProcess;

namespace precitec
{

namespace storage
{
class Attribute;
class AttributeModel;
class Parameter;
class ParameterSet;
class ProductModel;
}

namespace gui
{

class DeviceProxyWrapper;
class DeviceNotificationServer;

/**
 * The HardwareRoiController is a small controller class to set the Hardware ROI of the camera.
 **/
class HardwareRoiGigEController : public LiveModeController
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

    Q_PROPERTY(QSize maxSize READ maxSize NOTIFY maxSizeChanged)

    Q_PROPERTY(QString pingOutput READ pingOutput NOTIFY pingOutputChanged)

    /**
     * Label for liquid lens position
     **/
    Q_PROPERTY(QString liquidLensPositionLabel READ liquidLensPositionLabel CONSTANT)
    /**
     * AttributeModel for hardware parameters (key value attributes).
     **/
    Q_PROPERTY(precitec::storage::AttributeModel* attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)
    /**
     * The device proxy to the weldhead, needed for liquid lense position
     **/
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* weldHeadDeviceProxy READ weldHeadDeviceProxy WRITE setWeldHeadDeviceProxy NOTIFY weldHeadDeviceProxyChanged)

    /**
     * Parameter for liquid lens position
     **/
    Q_PROPERTY(precitec::storage::Parameter* liquidLensPositionParameter READ liquidLensPositionParameter NOTIFY liquidLensPositionParameterChanged)

    /**
     * Attribute for liquid lens position
     **/
    Q_PROPERTY(precitec::storage::Attribute* liquidLensPositionAttribute READ liquidLensPositionAttribute NOTIFY attributeModelChanged)

public:
    explicit HardwareRoiGigEController(QObject *parent = nullptr);
    ~HardwareRoiGigEController() override;

    bool isReady() const
    {
        return m_ready;
    }

    QRect roi() const
    {
        return m_roi;
    }

    QSize maxSize() const
    {
        return m_maxSize;
    }

    qreal exposureTime() const
    {
        return m_exposureTime;
    }

    int brightness() const
    {
        return m_brightness;
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

    QString pingOutput() const
    {
        return m_pingOutput;
    }

    QString liquidLensPositionLabel() const;

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

    Q_INVOKABLE void startPingCamera();
    Q_INVOKABLE void stopPingCamera();

    storage::AttributeModel* attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(storage::AttributeModel* model);

    precitec::gui::DeviceProxyWrapper *weldHeadDeviceProxy() const
    {
        return m_weldHeadDeviceProxy;
    }
    void setWeldHeadDeviceProxy(precitec::gui::DeviceProxyWrapper *weldHeadDeviceProxy);

    storage::Parameter* liquidLensPositionParameter() const;

    storage::Attribute* liquidLensPositionAttribute() const;

    /**
     * Updates liquid lens position to @p value.
     **/
    Q_INVOKABLE void updateLiquidLensPosition(qreal value);

Q_SIGNALS:
    void readyChanged();
    void roiChanged();
    void exposureTimeChanged();
    void brightnessChanged();
    void linLogValue1Changed();
    void linLogValue2Changed();
    void linLogTime1Changed();
    void linLogTime2Changed();
    void notificationServerChanged();
    void maxSizeChanged();
    void pingOutputChanged();
    void attributeModelChanged();
    void weldHeadDeviceProxyChanged();
    void liquidLensPositionParameterChanged();

private:
    void setReady(bool set);
    void updateCamera(std::function<void()> updateFunction);
    void initLiquidLensParameter();
    bool m_ready = false;
    QRect m_roi{0, 0, 512, 512};
    qreal m_exposureTime = 2.0;
    int m_brightness = 3300;
    int m_linLogValue1 = 0;
    int m_linLogValue2 = 30;
    int m_linLogTime1 = 0;
    int m_linLogTime2 = 1000;
    QPointer<DeviceNotificationServer> m_notificationServer;
    QMetaObject::Connection m_notificationConnection;
    QSize m_maxSize{1024, 1024};
    std::function<void()> m_onReadyFunction;
    QProcess *m_pingProcess = nullptr;
    QString m_pingOutput;
    storage::ParameterSet* m_parameterSet;
    storage::AttributeModel* m_attributeModel{nullptr};
    QMetaObject::Connection m_attributeModelDestroyed;
    DeviceProxyWrapper* m_weldHeadDeviceProxy{nullptr};
    QMetaObject::Connection m_weldHeadDestroyedConnection;
};

}
}
