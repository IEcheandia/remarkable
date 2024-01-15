#pragma once

#include "liveModeController.h"

#include <QAbstractItemModel>
#include <QPointer>

#include "message/device.h"

namespace precitec
{

namespace gui
{

class DeviceProxyWrapper;
class DeviceNotificationServer;
class ScanTrackerFrequencyModel;

class ScanTrackerController : public LiveModeController
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *weldHeadDevice READ weldHeadDevice WRITE setWeldHeadDevice NOTIFY weldHeadDeviceChanged)
    Q_PROPERTY(QAbstractItemModel *frequencyModel READ frequencyModel CONSTANT)
    Q_PROPERTY(int frequencyIndex READ frequencyIndex NOTIFY frequencyIndexChanged)
    Q_PROPERTY(bool driverEnabled READ isDriverEnabled NOTIFY driverEnabledChanged)
    Q_PROPERTY(bool widthOutOfGap READ widthOutOfGap NOTIFY widthOutOfGapChanged)
    Q_PROPERTY(bool posOutOfGap READ posOutOfGap NOTIFY posOutOfGapChanged)
    /**
     * NotificationServer to get events when the data changed
     **/
    Q_PROPERTY(precitec::gui::DeviceNotificationServer *deviceNotificationServer READ notificationServer WRITE setNotificationServer NOTIFY notificationServerChanged)
public:
    ScanTrackerController(QObject *parent = nullptr);
    ~ScanTrackerController() override;

    DeviceProxyWrapper *weldHeadDevice() const
    {
        return m_weldHeadDevice;
    }
    void setWeldHeadDevice(DeviceProxyWrapper *device);

    Q_INVOKABLE void queryStatus();
    Q_INVOKABLE void queryVersion();
    Q_INVOKABLE void querySerialNumber();

    Q_INVOKABLE void setFrequencyIndex(int index);
    Q_INVOKABLE void toggleDriverEnabled();
    Q_INVOKABLE void togglePosOutOfGap();
    Q_INVOKABLE void toggleWidthOutOfGap();
    Q_INVOKABLE void setExpertMode(bool enable);
    Q_INVOKABLE void setScanPosFixed(int pos);
    Q_INVOKABLE void setScanWidthFixed(int width);

    QAbstractItemModel *frequencyModel() const;

    int frequencyIndex() const
    {
        return m_frequencyIndex;
    }

    bool isDriverEnabled() const
    {
        return m_driverEnabled;
    }

    bool posOutOfGap() const
    {
        return m_posOutOfGap;
    }

    bool widthOutOfGap() const
    {
        return m_widthOutOfGap;
    }

    DeviceNotificationServer *notificationServer() const
    {
        return m_notificationServer.data();
    }
    void setNotificationServer(DeviceNotificationServer *server);

Q_SIGNALS:
    void weldHeadDeviceChanged();
    void frequencyIndexChanged();
    void driverEnabledChanged();
    void posOutOfGapChanged();
    void widthOutOfGapChanged();
    void notificationServerChanged();

private:
    void query(const std::string &key);
    void updateKeyValue(const interface::SmpKeyValue &keyValue);
    DeviceProxyWrapper *m_weldHeadDevice = nullptr;
    QMetaObject::Connection m_weldHeadDeviceDestroyed;
    QMutex m_weldHeadDeviceMutex;
    ScanTrackerFrequencyModel *m_frequencyModel;
    int m_frequencyIndex = -1;
    bool m_driverEnabled = false;
    bool m_posOutOfGap = false;
    bool m_widthOutOfGap = false;
    QPointer<DeviceNotificationServer> m_notificationServer;
    QMetaObject::Connection m_notificationConnection;
};

}
}
