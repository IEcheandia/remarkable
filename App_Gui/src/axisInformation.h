#pragma once

#include <QObject>
#include <QFuture>

#include <Poco/SharedPtr.h>

#include "event/viWeldHeadSubscribe.interface.h"

class QTimer;

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TviWeldHeadSubscribe<precitec::interface::AbstractInterface>> WeldHeadSubscribeProxy;

namespace interface
{
class KeyValue;
typedef Poco::SharedPtr<KeyValue> SmpKeyValue;
}

namespace gui
{

class DeviceNotificationServer;
class DeviceProxyWrapper;
class WeldHeadServer;

/**
 * AxisInformation provides status information about an attached axis.
 * It needs the properties:
 * @li weldHeadDevice
 * @li deviceNotificationServer
 * @li weldHeadSubscribeProxy
 * @li weldHeadServer
 * set, to retrieve information updates.
 *
 * The AxisInformation also allows to modify values of the axis. The
 * functionality is not exposed to Qml, instead use a more specialized component
 * such as AxisController.
 **/
class AxisInformation : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::DeviceProxyWrapper *weldHeadDevice READ weldHeadDevice WRITE setWeldHeadDevice NOTIFY weldHeadDeviceChanged)
    Q_PROPERTY(precitec::gui::DeviceNotificationServer *deviceNotificationServer READ deviceNotificationServer WRITE setDeviceNotificationServer NOTIFY deviceNotificationServerChanged)
    Q_PROPERTY(precitec::WeldHeadSubscribeProxy weldHeadSubscribeProxy READ weldHeadSubscribeProxy WRITE setWeldHeadSubscribeProxy NOTIFY weldHeadSubscribeProxyChanged)
    Q_PROPERTY(precitec::gui::WeldHeadServer *weldHeadServer READ weldHeadServer WRITE setWeldHeadServer NOTIFY weldHeadServerChanged)
    /**
     * Whether the system has the specified axis
     **/
    Q_PROPERTY(bool axisEnabled READ isAxisEnabled NOTIFY axisEnabledChanged)
    /**
     * Whether software limits for the axis are enabled
     **/
    Q_PROPERTY(bool softwareLimitsEnabled READ areSoftLimitsEnabled NOTIFY softLimitsEnabledChanged)
    /**
     * The lower software limit.
     **/
    Q_PROPERTY(int lowerLimit READ lowerLimit NOTIFY lowerLimitChanged)
    /**
     * The upper software limit.
     **/
    Q_PROPERTY(int upperLimit READ upperLimit NOTIFY upperLimitChanged)
    /**
     * The absolute position of the axis in um
     **/
    Q_PROPERTY(int position READ position NOTIFY positionChanged)
    /**
     * The minimum position of the axis in um
     **/
    Q_PROPERTY(int minimumPosition READ minimumPosition NOTIFY minimumPositionChanged)
    /**
     * The maximum position of the axis in um
     **/
    Q_PROPERTY(int maximumPosition READ maximumPosition NOTIFY maximumPositionChanged)

    /**
     * When set to @c true the head info gets polled every 100 msec.
     **/
    Q_PROPERTY(bool pollHeadInfo READ isPollingHeadInfo WRITE setPollHeadInfo NOTIFY pollHeadInfoChanged)

    /**
     * The current status of the axis as hex word.
     **/
    Q_PROPERTY(quint32 statusWord READ status NOTIFY statusChanged)
    Q_PROPERTY(quint32 errorCode READ errorCode NOTIFY errorCodeChanged)
    Q_PROPERTY(int positionUserUnit READ positionUserUnit NOTIFY positionUserUnitChanged)
    Q_PROPERTY(int actVelocity READ actVelocity NOTIFY actVelocityChanged)
    Q_PROPERTY(int actTorque READ actTorque NOTIFY actTorqueChanged)
    Q_PROPERTY(short modeOfOperation READ modeOfOperation NOTIFY modeOfOperationChanged)
    Q_PROPERTY(bool initiallyPolled READ isInitiallyPolled NOTIFY initiallyPolled)
    Q_PROPERTY(bool homingDirectionPositive READ isHomingDirectionPositive NOTIFY homingDirectionChanged)
    Q_PROPERTY(bool requiresHoming READ requiresHoming NOTIFY requiresHomingChanged)

public:
    explicit AxisInformation(QObject *parent = nullptr);
    ~AxisInformation() override;

    enum class Mode {
        Pending = -1,
        Offline = 0,
        Position = 1,
        Position_Relative = 11,
        Position_Absolute = 12 ,
        Velocity = 3,
        Home = 6
    };
    Q_ENUM(Mode)

    DeviceProxyWrapper *weldHeadDevice() const
    {
        return m_weldHeadDevice;
    }

    void setWeldHeadDevice(DeviceProxyWrapper *proxy);

    DeviceNotificationServer *deviceNotificationServer() const
    {
        return m_deviceNotificationServer;
    }
    void setDeviceNotificationServer(DeviceNotificationServer *dns);

    bool isAxisEnabled() const
    {
        return m_axisEnabled;
    }

    bool areSoftLimitsEnabled() const
    {
        return m_softLimitsEnabled;
    }

    int upperLimit() const
    {
        return m_upperLimit;
    }

    int lowerLimit() const
    {
        return m_lowerLimit;
    }

    int position() const
    {
        return m_position;
    }

    int minimumPosition() const
    {
        return m_minimumPosition;
    }
    int maximumPosition() const
    {
        return m_maximumPosition;
    }

    quint32 status() const
    {
        return m_statusWord;
    }

    quint32 errorCode() const
    {
        return m_errorCode;
    }

    int positionUserUnit() const
    {
        return m_positionUserUnit;
    }

    int actVelocity() const
    {
        return m_actVelocity;
    }

    int actTorque() const
    {
        return m_actTorque;
    }

    short modeOfOperation() const
    {
        return m_modeOfOperation;
    }

    WeldHeadSubscribeProxy weldHeadSubscribeProxy() const
    {
        return m_weldHeadSubscribeProxy;
    }
    void setWeldHeadSubscribeProxy(const WeldHeadSubscribeProxy &proxy);

    bool isPollingHeadInfo() const;
    void setPollHeadInfo(bool set);

    WeldHeadServer *weldHeadServer() const
    {
        return m_weldHeadServer;
    }
    void setWeldHeadServer(WeldHeadServer *server);

    /**
     * Requests a move of the axis to @p targetPos.
     * The request is performed asynchronously and thus a future is returned.
     **/
    QFuture<void> moveAxis(int targetPos);

    /**
     * Requests to set the lower software @p limit.
     * The request is performed asynchronously and thus a future is returned.
     **/
    QFuture<void> setLowerLimit(int limit);

    /**
     * Requests to set the upper software @p limit.
     * The request is performed asynchronously and thus a future is returned.
     **/
    QFuture<void> setUpperLimit(int limit);

    /**
     * Requests to @p enable software limits.
     * The request is performed asynchronously and thus a future is returned.
     **/
    QFuture<void> enableSoftwareLimits(bool enable);

    QFuture<void> toggleHomingDirectionPositive(bool set);

    bool isInitiallyPolled() const
    {
        return m_initiallyPolled;
    }

    bool isHomingDirectionPositive() const
    {
        return m_homingDirectionPositive;
    }

    bool requiresHoming() const
    {
        return m_requiresHoming;
    }

Q_SIGNALS:
    void weldHeadDeviceChanged();
    void deviceNotificationServerChanged();
    void axisEnabledChanged();
    void softLimitsEnabledChanged();
    void lowerLimitChanged();
    void upperLimitChanged();
    void positionChanged();
    void minimumPositionChanged();
    void maximumPositionChanged();
    void weldHeadSubscribeProxyChanged();
    void pollHeadInfoChanged();
    void weldHeadServerChanged();
    void statusChanged();
    void errorCodeChanged();
    void positionUserUnitChanged();
    void actVelocityChanged();
    void actTorqueChanged();
    void modeOfOperationChanged();
    void initiallyPolled();
    void homingDirectionChanged();
    void requiresHomingChanged();

private:
    void checkAxisEnabled();
    void initAxisDevice();
    void updateSoftLimitsEnabled(const precitec::interface::SmpKeyValue &kv);
    void updateLowerLimit(const precitec::interface::SmpKeyValue &kv);
    void updateUpperLimit(const precitec::interface::SmpKeyValue &kv);
    void updatePosition(const precitec::interface::SmpKeyValue &kv);
    void keyValueChanged(const QUuid &deviceId, precitec::interface::SmpKeyValue kv);
    void updateStatusWord(const precitec::interface::HeadInfo &info);
    void updateErrorCode(const precitec::interface::HeadInfo &info);
    void updatePositionUserUnit(const precitec::interface::HeadInfo &info);
    void updateActVelocity(const precitec::interface::HeadInfo &info);
    void updateActTorque(const precitec::interface::HeadInfo &info);
    void updateModeOfOperation(const precitec::interface::HeadInfo &info);
    void updateHomingDirection(const precitec::interface::HeadInfo &info);
    QFuture<void> updateKeyValue(const precitec::interface::SmpKeyValue &kv);
    DeviceProxyWrapper *m_weldHeadDevice = nullptr;
    QMetaObject::Connection m_weldHeadDestroyed;
    DeviceNotificationServer *m_deviceNotificationServer = nullptr;
    QMetaObject::Connection m_deviceNotificationDestroyed;
    bool m_axisEnabled = false;
    bool m_softLimitsEnabled = false;
    int m_lowerLimit = 0;
    int m_upperLimit = 0;
    int m_position = 0;
    int m_minimumPosition = 0;
    int m_maximumPosition = 50000;
    WeldHeadSubscribeProxy m_weldHeadSubscribeProxy;
    QTimer *m_pollHeadInfo;
    WeldHeadServer *m_weldHeadServer = nullptr;
    QMetaObject::Connection m_weldHeadServerDestroyed;
    QMetaObject::Connection m_yAxisChanged;
    quint32 m_statusWord = 0;
    quint32 m_errorCode = 0;
    int m_positionUserUnit = 0;
    int m_actVelocity = 0;
    int m_actTorque = 0;
    short m_modeOfOperation = 0;
    bool m_homingDirectionPositive = false;
    bool m_initiallyPolled = false;
    bool m_requiresHoming = true;
    bool m_homingReached = false;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::AxisInformation*)
