#pragma once

#include "liveModeController.h"

#include <functional>
#include <QPointer>
#include "deviceNotificationServer.h"

class ScanLabControllerTest;

namespace precitec
{
namespace storage
{

class AttributeModel;

}
namespace gui
{

class DeviceProxyWrapper;

class ScanLabController : public LiveModeController
{
    Q_OBJECT

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* weldheadDeviceProxy READ weldheadDeviceProxy WRITE setWeldheadDeviceProxy NOTIFY weldheadDeviceProxyChanged)

    Q_PROPERTY(precitec::storage::AttributeModel* attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)

    Q_PROPERTY(double scannerXPosition READ scannerXPosition WRITE setScannerXPosition NOTIFY scannerXPositionChanged)

    Q_PROPERTY(double scannerYPosition READ scannerYPosition WRITE setScannerYPosition NOTIFY scannerYPositionChanged)

    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

    Q_PROPERTY(bool updating READ updating NOTIFY updatingChanged)

    Q_PROPERTY(double xMinLimit READ xMinLimit NOTIFY xLimitChanged)

    Q_PROPERTY(double xMaxLimit READ xMaxLimit NOTIFY xLimitChanged)

    Q_PROPERTY(double yMinLimit READ yMinLimit NOTIFY yLimitChanged)

    Q_PROPERTY(double yMaxLimit READ yMaxLimit NOTIFY yLimitChanged)

    Q_PROPERTY(bool canIncrementX READ canIncrementX NOTIFY canReachXLimitChanged)

    Q_PROPERTY(bool canDecrementX READ canDecrementX NOTIFY canReachXLimitChanged)

    Q_PROPERTY(bool canIncrementY READ canIncrementY NOTIFY canReachYLimitChanged)

    Q_PROPERTY(bool canDecrementY READ canDecrementY NOTIFY canReachYLimitChanged)

    Q_PROPERTY(precitec::gui::DeviceNotificationServer *deviceNotificationServer READ notificationServer WRITE setNotificationServer NOTIFY notificationServerChanged)

public:
    explicit ScanLabController(QObject* parent = nullptr);
    ~ScanLabController();

    DeviceProxyWrapper* weldheadDeviceProxy() const
    {
        return m_weldheadDeviceProxy;
    }
    void setWeldheadDeviceProxy(DeviceProxyWrapper* device);

    precitec::storage::AttributeModel* attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(precitec::storage::AttributeModel* model);

    bool ready() const
    {
        return m_ready;
    }

    bool updating() const
    {
        return m_updating;
    }

    double scannerXPosition() const
    {
        return m_scannerXPosition;
    }
    void setScannerXPosition(double x);

    double scannerYPosition() const
    {
        return m_scannerYPosition;
    }
    void setScannerYPosition(double y);

    bool canIncrementX() const;

    bool canDecrementX() const;

    bool canIncrementY() const;

    bool canDecrementY() const;

    double xMinLimit() const
    {
        return m_xMin;
    }

    double xMaxLimit() const
    {
        return m_xMax;
    }

    double yMinLimit() const
    {
        return m_yMin;
    }

    double yMaxLimit() const
    {
        return m_yMax;
    }

    DeviceNotificationServer *notificationServer() const
    {
        return m_notificationServer.data();
    }

    Q_INVOKABLE void incrementXPosition();
    Q_INVOKABLE void decrementXPosition();
    Q_INVOKABLE void incrementYPosition();
    Q_INVOKABLE void decrementYPosition();
    Q_INVOKABLE void resetToZero();
    Q_INVOKABLE void setFiberSwitchPosition(int n);

Q_SIGNALS:
    void weldheadDeviceProxyChanged();
    void attributeModelChanged();
    void readyChanged();
    void updatingChanged();
    void scannerXPositionChanged();
    void scannerYPositionChanged();
    void xLimitChanged();
    void yLimitChanged();
    void canReachXLimitChanged();
    void canReachYLimitChanged();
    void notificationServerChanged();
    void scannerActualXPostionChanged();
    void scannerActualYPostionChanged();

private:
    void setReady(bool set);
    void setUpdating(bool set);
    void updatePosition();
    void update(std::function<void()> updateFunction);
    void setNotificationServer(DeviceNotificationServer *server);
    bool hasPermission();

    bool m_ready = false;
    bool m_updating = false;
    double m_scannerXPosition = 0.0;
    double m_scannerYPosition = 0.0;
    double m_xMin = -500.0;
    double m_xMax = 500.0;
    double m_yMin = -500.0;
    double m_yMax = 500.0;
    const double m_stepSize = 1.0;
    double m_actualXPosition = 0.0;
    double m_actualYPosition = 0.0;

    DeviceProxyWrapper* m_weldheadDeviceProxy = nullptr;
    QMetaObject::Connection  m_weldheadDeviceDestroyConnection;

    precitec::storage::AttributeModel* m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelDestroyedConnection;

    QPointer<DeviceNotificationServer> m_notificationServer = nullptr;
    QMetaObject::Connection m_notificationConnection;

    friend ScanLabControllerTest;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::ScanLabController*)

