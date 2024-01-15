#pragma once

#include <QObject>
#include "deviceProxyWrapper.h"

#include <functional>

namespace precitec
{

namespace gui
{

class LaserControlDelayController : public QObject
{
    Q_OBJECT

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *weldHeadDevice READ weldHeadDevice WRITE setWeldHeadDevice NOTIFY weldHeadDeviceChanged)

    Q_PROPERTY(int delay READ delay WRITE setDelay NOTIFY delayChanged)
    Q_PROPERTY(int frequency READ frequency WRITE setFrequency NOTIFY frequencyChanged)
    Q_PROPERTY(int power READ power WRITE setPower NOTIFY powerChanged)
    Q_PROPERTY(int amplitude READ amplitude WRITE setAmplitude NOTIFY amplitudeChanged)

    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)

    Q_PROPERTY(bool updating READ isUpdating NOTIFY updatingChanged)

    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)
    
public:
    explicit LaserControlDelayController(QObject *parent = nullptr);
    ~LaserControlDelayController() override;

    DeviceProxyWrapper *weldHeadDevice() const
    {
        return m_weldHeadDevice;
    }
    void setWeldHeadDevice(DeviceProxyWrapper *device);

    int delay() const
    {
        return m_delay;
    }
    void setDelay(int delay);

    int frequency() const
    {
        return m_frequency;
    }
    
    void setFrequency(int frequency);

    int power() const
    {
        return m_power;
    }
    
    void setPower(int power);

    int amplitude() const
    {
        return m_amplitude;
    }
    
    void setAmplitude(int amplitude);
    
    bool isReady() const
    {
        return m_ready;
    }
    bool isUpdating() const
    {
        return m_updating;
    }
    
    bool isVisible() const
    {
        return m_visible;
    }
    
Q_SIGNALS:
    void weldHeadDeviceChanged();
    void readyChanged();
    void delayChanged();
    void frequencyChanged();
    void powerChanged();
    void amplitudeChanged();
    void updatingChanged();
    void visibleChanged();

private:
    bool hasPermission();

    void setReady(bool set);

    void setUpdating(bool set);
    
    void setVisible(bool set);

    void update(std::function<void()> updateFunction);
    void updateDelay();
    void updateFrequency();
    void updatePower();
    void updateAmplitude();

    bool m_ready = false;

    bool m_updating = false;

    bool m_visible = false;
    
    int m_delay = 0;
    int m_frequency = 100;
    int m_power = 50;
    double m_amplitude = 4.0;

    DeviceProxyWrapper *m_weldHeadDevice = nullptr;
    QMetaObject::Connection m_weldHeadDeviceDestroyed;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LaserControlDelayController*)

