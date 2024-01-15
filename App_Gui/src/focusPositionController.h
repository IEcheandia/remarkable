#pragma once

#include <QObject>
#include "deviceProxyWrapper.h"

#include <functional>

namespace precitec
{

namespace gui
{

class FocusPositionController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *weldHeadDevice READ weldHeadDevice WRITE setWeldHeadDevice NOTIFY weldHeadDeviceChanged)

    Q_PROPERTY(int absolutePosition READ absolutePosition WRITE setAbsolutePosition NOTIFY absolutePositionChanged)

    Q_PROPERTY(double systemOffset READ systemOffset WRITE setSystemOffset NOTIFY systemOffsetChanged)

    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)

    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)

public:
    explicit FocusPositionController(QObject *parent = nullptr);
    ~FocusPositionController() override;

    DeviceProxyWrapper *weldHeadDevice() const
    {
        return m_weldHeadDevice;
    }
    void setWeldHeadDevice(DeviceProxyWrapper *device);

    int absolutePosition() const
    {
        return m_absolutePosition;
    }
    void setAbsolutePosition(int position);

    double systemOffset() const
    {
        return m_systemOffset;
    }
    void setSystemOffset(double offset);
    
    bool isReady() const
    {
        return m_ready;
    }
    bool isUpdating() const
    {
        return m_updating;
    }

    Q_INVOKABLE void performReferenceRun();

Q_SIGNALS:
    void weldHeadDeviceChanged();
    void readyChanged();
    void absolutePositionChanged();
    void updatingChanged();
    void systemOffsetChanged();

private:
    bool hasPermission();

    void setReady(bool set);

    void setUpdating(bool set);

    void update(std::function<void()> updateFunction);
    void updateAbsolutePosition();
    void updateSystemOffset();

    bool m_ready = false;

    bool m_updating = false;

    int m_absolutePosition = 0;

    double m_systemOffset = 0.0;
    
    DeviceProxyWrapper *m_weldHeadDevice = nullptr;
    QMetaObject::Connection m_weldHeadDeviceDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::FocusPositionController*)
