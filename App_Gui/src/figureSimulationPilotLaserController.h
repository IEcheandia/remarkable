#pragma once
#include <QObject>

namespace precitec
{
namespace gui
{

class DeviceProxyWrapper;

/**
 * Controller class for simulating a welding figure with pilot laser.
 **/
class FigureSimulationPilotLaserController : public QObject
{
    /**
     * Whether the input is valid and the simulation can be started.
     **/
    Q_PROPERTY(bool valid READ isValid NOTIFY validChanged)
    /**
     * Whether the simulation is running.
     **/
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    /**
     * Whether a wobble figure should be used to overlay the seam figure.
     * @see wobbleId
     **/
    Q_PROPERTY(bool wobble READ wobble WRITE setWobble NOTIFY wobbleChanged)
    /**
     * The id of the seam figure to simulate. By default an invalid @c -1 is set.
     **/
    Q_PROPERTY(int seamId READ seamId WRITE setSeamId NOTIFY seamIdChanged)
    /**
     * The id of the wobble figure to overlay the seam simulate. By default an invalid @c -1 is set.
     * Only relevant if wobble is @c true.
     * @see wobble
     **/
    Q_PROPERTY(int wobbleId READ wobbleId WRITE setWobbleId NOTIFY wobbleIdChanged)
    /**
     * The velocity in m/s
     **/
    Q_PROPERTY(double velocity READ velocity WRITE setVelocity NOTIFY velocityChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* weldheadDeviceProxy READ weldheadDeviceProxy WRITE setWeldheadDeviceProxy NOTIFY weldheadDeviceProxyChanged)
    Q_OBJECT
public:
    FigureSimulationPilotLaserController(QObject *parent = nullptr);
    ~FigureSimulationPilotLaserController() override;

    /**
     * Starts the simulation. Emits @link{runningChanged} if started.
     **/
    Q_INVOKABLE void start();

    /**
     * Stops the simulation. Emits @link{runningChanged} if stopped.
     **/
    Q_INVOKABLE void stop();

    bool isValid() const
    {
        return m_valid;
    }
    bool isRunning() const
    {
        return m_running;
    }

    bool wobble() const
    {
        return m_wobble;
    }
    void setWobble(bool wobble);

    int seamId() const
    {
        return m_seamId;
    }
    void setSeamId(int seamId);

    int wobbleId() const
    {
        return m_wobbleId;
    }
    void setWobbleId(int wobbleId);

    double velocity() const
    {
        return m_velocity;
    }
    void setVelocity(double velocity);

    DeviceProxyWrapper* weldheadDeviceProxy() const
    {
        return m_weldheadDeviceProxy;
    }
    void setWeldheadDeviceProxy(DeviceProxyWrapper* device);

Q_SIGNALS:
    void validChanged();
    void runningChanged();
    void wobbleChanged();
    void seamIdChanged();
    void wobbleIdChanged();
    void velocityChanged();
    void weldheadDeviceProxyChanged();

private:
    void updateValid();

    bool m_valid{false};
    bool m_running{false};
    bool m_wobble{false};
    int m_seamId{-1};
    int m_wobbleId{-1};
    double m_velocity{1.0};

    DeviceProxyWrapper* m_weldheadDeviceProxy = nullptr;
    QMetaObject::Connection  m_weldheadDeviceDestroyConnection;
};

}
}
