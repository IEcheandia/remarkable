#pragma once

#include "abstractModule.h"

#include "event/viServiceFromGUI.interface.h"
#include "event/viServiceToGUI.interface.h"
#include "event/viServiceToGUI.handler.h"
#include "event/viWeldHeadSubscribe.interface.h"
#include "event/deviceNotification.interface.h"
#include "event/deviceNotification.handler.h"
#include "event/viWeldHeadPublish.interface.h"
#include "event/viWeldHeadPublish.handler.h"
#include "event/videoRecorder.interface.h"
#include "event/videoRecorder.proxy.h"

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TviServiceFromGUI<precitec::interface::AbstractInterface>> ServiceFromGuiProxy;
typedef std::shared_ptr<precitec::interface::TviWeldHeadSubscribe<precitec::interface::AbstractInterface>> WeldHeadSubscribeProxy;
typedef std::shared_ptr<precitec::interface::TVideoRecorder<precitec::interface::AbstractInterface>> VideoRecorderProxy;

namespace gui
{

class DeviceNotificationServer;
class ServiceToGuiServer;
class WeldHeadServer;
class DeviceProxyWrapper;

class HardwareModule : public AbstractModule
{
    Q_OBJECT

    Q_PROPERTY(precitec::ServiceFromGuiProxy serviceFromGuiProxy READ serviceFromGuiProxy CONSTANT)

    Q_PROPERTY(precitec::gui::ServiceToGuiServer* serviceServer READ serviceServer CONSTANT)

    Q_PROPERTY(precitec::WeldHeadSubscribeProxy weldHeadSubscribeProxy READ weldHeadSubscribeProxy NOTIFY weldHeadSubscribeProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceNotificationServer* deviceNotificationServer READ deviceNotificationServer CONSTANT)

    Q_PROPERTY(precitec::gui::WeldHeadServer* weldHeadServer READ weldHeadServer CONSTANT)

    Q_PROPERTY(precitec::VideoRecorderProxy videoRecorderProxy READ videoRecorderProxy CONSTANT)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* videoRecorderDeviceProxy READ videoRecorderDeviceProxy NOTIFY videoRecorderDeviceProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* grabberDeviceProxy READ grabberDeviceProxy NOTIFY grabberDeviceProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* calibrationDeviceProxy READ calibrationDeviceProxy NOTIFY calibrationDeviceProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* weldHeadDeviceProxy READ weldHeadDeviceProxy NOTIFY weldHeadDeviceProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* serviceDeviceProxy READ serviceDeviceProxy NOTIFY serviceDeviceProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* inspectionDeviceProxy READ inspectionDeviceProxy NOTIFY inspectionDeviceProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* workflowDeviceProxy READ workflowDeviceProxy NOTIFY workflowDeviceProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* storageDeviceProxy READ storageDeviceProxy NOTIFY storageDeviceProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* idmDeviceProxy READ idmDeviceProxy NOTIFY idmDeviceProxyChanged)

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* guiDeviceProxy READ guiDeviceProxy CONSTANT)

public:
    static HardwareModule* instance();

    ServiceFromGuiProxy serviceFromGuiProxy() const;

    ServiceToGuiServer* serviceServer() const
    {
        return m_serviceServer;
    }

    WeldHeadSubscribeProxy weldHeadSubscribeProxy() const;

    DeviceNotificationServer* deviceNotificationServer() const
    {
        return m_deviceNotificationServer;
    }

    WeldHeadServer* weldHeadServer() const
    {
        return m_weldHeadServer;
    }

    precitec::VideoRecorderProxy videoRecorderProxy() const;

    DeviceProxyWrapper* videoRecorderDeviceProxy() const;
    DeviceProxyWrapper* grabberDeviceProxy() const;
    DeviceProxyWrapper* calibrationDeviceProxy() const;
    DeviceProxyWrapper* weldHeadDeviceProxy() const;
    DeviceProxyWrapper* serviceDeviceProxy() const;
    DeviceProxyWrapper* inspectionDeviceProxy() const;
    DeviceProxyWrapper* workflowDeviceProxy() const;
    DeviceProxyWrapper* storageDeviceProxy() const;
    DeviceProxyWrapper* idmDeviceProxy() const;
    DeviceProxyWrapper* guiDeviceProxy() const;

    Q_INVOKABLE void initialize();

Q_SIGNALS:
    void weldHeadSubscribeProxyChanged();
    void videoRecorderDeviceProxyChanged();
    void grabberDeviceProxyChanged();
    void calibrationDeviceProxyChanged();
    void weldHeadDeviceProxyChanged();
    void serviceDeviceProxyChanged();
    void inspectionDeviceProxyChanged();
    void workflowDeviceProxyChanged();
    void storageDeviceProxyChanged();
    void idmDeviceProxyChanged();

private:
    explicit HardwareModule();

    void registerPublications() override;
    void init() override;
    void markDevicesAsConnected() override;

    std::shared_ptr<precitec::interface::TviServiceFromGUI<precitec::interface::EventProxy>> m_serviceFromGuiProxy;

    ServiceToGuiServer* m_serviceServer;
    std::unique_ptr<precitec::interface::TviServiceToGUI<precitec::interface::EventHandler>> m_serviceToGuiHandler;

    std::shared_ptr<precitec::interface::TviWeldHeadSubscribe<precitec::interface::EventProxy>> m_weldHeadSubscribeProxy;

    DeviceNotificationServer* m_deviceNotificationServer;
    std::unique_ptr<precitec::interface::TDeviceNotification<precitec::interface::EventHandler>> m_deviceNotificationHandler;

    WeldHeadServer* m_weldHeadServer;
    std::unique_ptr<precitec::interface::TviWeldHeadPublish<precitec::interface::EventHandler>> m_weldHeadHandler;

    std::shared_ptr<precitec::interface::TVideoRecorder<precitec::interface::EventProxy>> m_videoRecorderProxy;

    DeviceProxyWrapper* m_videoRecorderDeviceProxy;
    DeviceProxyWrapper* m_grabberDeviceProxy;
    DeviceProxyWrapper* m_calibrationDeviceProxy;
    DeviceProxyWrapper* m_weldHeadDeviceProxy;
    DeviceProxyWrapper* m_serviceDeviceProxy;
    DeviceProxyWrapper* m_inspectionDeviceProxy;
    DeviceProxyWrapper* m_workflowDeviceProxy;
    DeviceProxyWrapper* m_storageDeviceProxy;
    DeviceProxyWrapper* m_idmDeviceProxy;
    DeviceProxyWrapper* m_guiDeviceProxy;
};

}
}
