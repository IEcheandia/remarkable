#include "hardwareModule.h"

#include "permissions.h"
#include "systemStatusServer.h"
#include "recorderServer.h"
#include "serviceToGuiServer.h"
#include "deviceNotificationServer.h"
#include "weldHeadServer.h"
#include "deviceProxyWrapper.h"
#include "deviceServer.h"
#include "event/viServiceFromGUI.proxy.h"
#include "event/viWeldHeadSubscribe.proxy.h"
#include "message/device.proxy.h"

#include <QtConcurrentRun>

namespace precitec
{
namespace gui
{

static const QUuid s_videoRecorderID{QByteArrayLiteral("96599c45-4e20-4aaa-826d-25463670dd09")};
static const QUuid s_weldHeadID{QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095")};
static const QUuid s_cameraID{QByteArrayLiteral("1f50352e-a92a-4521-b184-e16809345026")};
static const QUuid s_calibID{QByteArrayLiteral("c3a01597-53db-4262-a091-69b0345f083d")};
static const QUuid s_serviceID{QByteArrayLiteral("a97a5a4c-dcd0-4a77-b933-9d1e20dbe73c")};
static const QUuid s_inspectionID{QByteArrayLiteral("F42DDE6B-C8FF-4CE5-86DE-1A5CB51D633A")};
static const QUuid s_workflowID{QByteArrayLiteral("A60D345E-16CA-4710-AB4D-CAD65CC42959")};
static const QUuid s_storageID{QByteArrayLiteral("1f58fcea-58bc-4c1a-945b-9b24a9e09963")};
static const QUuid s_idmID{QByteArrayLiteral("4fc80872-b8ad-11e7-abc4-cec278b6b50a")};
static const QUuid s_guiID{QByteArrayLiteral("7b27d004-47c9-4f9a-b807-cae94806ae67")};

HardwareModule::HardwareModule()
    : AbstractModule(qgetenv("WM_STATION_NAME"))
    , m_serviceFromGuiProxy(std::make_shared<TviServiceFromGUI<EventProxy>>())
    , m_serviceServer(new ServiceToGuiServer(m_serviceFromGuiProxy, this))
    , m_weldHeadSubscribeProxy()
    , m_deviceNotificationServer(new DeviceNotificationServer(this))
    , m_deviceNotificationHandler(std::make_unique<TDeviceNotification<EventHandler>>(m_deviceNotificationServer))
    , m_weldHeadServer(new WeldHeadServer(this))
    , m_weldHeadHandler(std::make_unique<TviWeldHeadPublish<EventHandler>>(m_weldHeadServer))
    , m_videoRecorderProxy(std::make_shared<TVideoRecorder<EventProxy>>())
    , m_videoRecorderDeviceProxy(new DeviceProxyWrapper(std::make_shared<TDevice<MsgProxy>>(), Permission::ViewVideoRecorderDeviceConfig, Permission::EditVideoRecorderDeviceConfig, s_videoRecorderID, this))
    , m_grabberDeviceProxy(new DeviceProxyWrapper(std::make_shared<TDevice<MsgProxy>>(), Permission::ViewGrabberDeviceConfig, Permission::EditGrabberDeviceConfig, s_cameraID, this))
    , m_calibrationDeviceProxy(new DeviceProxyWrapper(std::make_shared<TDevice<MsgProxy>>(), Permission::ViewCalibrationDeviceConfig, Permission::EditCalibrationDeviceConfig, s_calibID, this))
    , m_weldHeadDeviceProxy(new DeviceProxyWrapper(std::make_shared<TDevice<MsgProxy>>(), Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, s_weldHeadID, this))
    , m_serviceDeviceProxy(new DeviceProxyWrapper(std::make_shared<TDevice<MsgProxy>>(), Permission::ViewServiceDeviceConfig, Permission::EditServiceDeviceConfig, s_serviceID, this))
    , m_inspectionDeviceProxy(new DeviceProxyWrapper(std::make_shared<TDevice<MsgProxy>>(), Permission::ViewInspectionDeviceConfig, Permission::EditInspectionDeviceConfig, s_inspectionID, this))
    , m_workflowDeviceProxy(new DeviceProxyWrapper(std::make_shared<TDevice<MsgProxy>>(), Permission::ViewWorkflowDeviceConfig, Permission::EditWorkflowDeviceConfig, s_workflowID, this))
    , m_storageDeviceProxy(new DeviceProxyWrapper(std::make_shared<TDevice<MsgProxy>>(), Permission::ViewStorageDeviceConfig, Permission::EditStorageDeviceConfig, s_storageID, this))
    , m_idmDeviceProxy(new DeviceProxyWrapper(std::make_shared<TDevice<MsgProxy>>(), Permission::ViewIDMDeviceConfig, Permission::EditIDMDeviceConfig, s_idmID, this))
    , m_guiDeviceProxy(new DeviceProxyWrapper(std::make_shared<DeviceServer>(), Permission::ViewGuiDeviceConfig, Permission::EditGuiDeviceConfig, s_guiID, this))
{
    connect(this, &AbstractModule::readyChanged, this, [this]
        {
            m_serviceFromGuiProxy->requestSlaveInfo();
        }, Qt::QueuedConnection
    );

    m_serviceDeviceProxy->setChangesRequireRestart(true);
    m_guiDeviceProxy->markAsConnected();

    if (auto deviceServer = std::dynamic_pointer_cast<DeviceServer>(m_guiDeviceProxy->deviceProxy()))
    {
        deviceServer->setNotificationServer(m_deviceNotificationServer);
    }

    connect(m_videoRecorderDeviceProxy, &DeviceProxyWrapper::connected, this, &HardwareModule::videoRecorderDeviceProxyChanged);
    connect(m_grabberDeviceProxy, &DeviceProxyWrapper::connected, this, &HardwareModule::grabberDeviceProxyChanged);
    connect(m_calibrationDeviceProxy, &DeviceProxyWrapper::connected, this, &HardwareModule::calibrationDeviceProxyChanged);
    connect(m_weldHeadDeviceProxy, &DeviceProxyWrapper::connected, this, &HardwareModule::weldHeadDeviceProxyChanged);
    connect(m_serviceDeviceProxy, &DeviceProxyWrapper::connected, this, &HardwareModule::serviceDeviceProxyChanged);
    connect(m_inspectionDeviceProxy, &DeviceProxyWrapper::connected, this, &HardwareModule::inspectionDeviceProxyChanged);
    connect(m_workflowDeviceProxy, &DeviceProxyWrapper::connected, this, &HardwareModule::workflowDeviceProxyChanged);
    connect(m_storageDeviceProxy, &DeviceProxyWrapper::connected, this, &HardwareModule::storageDeviceProxyChanged);
    connect(m_idmDeviceProxy, &DeviceProxyWrapper::connected, this, &HardwareModule::idmDeviceProxyChanged);

    connect(systemStatus(), &SystemStatusServer::stateChanged, this, [this] {
        recorder()->setThrottled(systemStatus()->state() == SystemStatusServer::OperationState::Automatic);
    });
}

void HardwareModule::initialize()
{
    QtConcurrent::run(this, &HardwareModule::init);
}

HardwareModule* HardwareModule::instance()
{
    static HardwareModule hardwareModule;
    return &hardwareModule;
}

void HardwareModule::registerPublications()
{
    AbstractModule::registerPublications();

    registerPublication(m_serviceFromGuiProxy.get());
    registerSubscription(m_deviceNotificationHandler.get());

    m_serviceToGuiHandler = std::make_unique<TviServiceToGUI<EventHandler>>(m_serviceServer);
    registerSubscription(m_serviceToGuiHandler.get());

    m_weldHeadSubscribeProxy = std::make_shared<TviWeldHeadSubscribe<EventProxy>>();
    registerPublication(m_weldHeadSubscribeProxy.get());

    registerSubscription(m_weldHeadHandler.get());
    registerPublication(m_videoRecorderProxy.get());

    registerPublication(static_cast<TDevice<MsgProxy>*>(m_videoRecorderDeviceProxy->deviceProxy().get()), precitec::system::module::WorkflowModul, std::string("Videorecorder"));
    registerPublication(static_cast<TDevice<MsgProxy>*>(m_grabberDeviceProxy->deviceProxy().get()), precitec::system::module::WorkflowModul, std::string("Grabber"));
    registerPublication(static_cast<TDevice<MsgProxy>*>(m_calibrationDeviceProxy->deviceProxy().get()), precitec::system::module::WorkflowModul, std::string("CalibrationControl"));
    registerPublication(static_cast<TDevice<MsgProxy>*>(m_weldHeadDeviceProxy->deviceProxy().get()), precitec::system::module::WorkflowModul, std::string("WeldHeadControl"));
    registerPublication(static_cast<TDevice<MsgProxy>*>(m_serviceDeviceProxy->deviceProxy().get()), precitec::system::module::WorkflowModul, std::string("Service"));
    registerPublication(static_cast<TDevice<MsgProxy>*>(m_inspectionDeviceProxy->deviceProxy().get()), precitec::system::module::WorkflowModul, std::string("InspectionControl"));
    registerPublication(static_cast<TDevice<MsgProxy>*>(m_workflowDeviceProxy->deviceProxy().get()), precitec::system::module::WorkflowModul, std::string("Workflow"));
    registerPublication(static_cast<TDevice<MsgProxy>*>(m_storageDeviceProxy->deviceProxy().get()), precitec::system::module::WorkflowModul, std::string("Storage"));
    if (idmEnabled())
    {
        registerPublication(static_cast<TDevice<MsgProxy>*>(m_idmDeviceProxy->deviceProxy().get()), precitec::system::module::WorkflowModul, std::string("CHRCommunication"));
    }
}

void HardwareModule::markDevicesAsConnected()
{
    m_videoRecorderDeviceProxy->markAsConnected();
    m_grabberDeviceProxy->markAsConnected();
    m_calibrationDeviceProxy->markAsConnected();
    m_weldHeadDeviceProxy->markAsConnected();
    m_serviceDeviceProxy->markAsConnected();
    m_inspectionDeviceProxy->markAsConnected();
    m_workflowDeviceProxy->markAsConnected();
    m_storageDeviceProxy->markAsConnected();
    if (idmEnabled())
    {
        m_idmDeviceProxy->markAsConnected();
    }

    emit weldHeadSubscribeProxyChanged();
}

void HardwareModule::init()
{
    AbstractModule::init();
}

#define DEVICEPROXY(name) \
DeviceProxyWrapper* HardwareModule::name##DeviceProxy() const \
{ \
    if (!m_##name##DeviceProxy->isConnected()) \
        return nullptr; \
    return m_##name##DeviceProxy; \
}

DEVICEPROXY(videoRecorder)
DEVICEPROXY(grabber)
DEVICEPROXY(calibration)
DEVICEPROXY(weldHead)
DEVICEPROXY(service)
DEVICEPROXY(inspection)
DEVICEPROXY(workflow)
DEVICEPROXY(storage)
DEVICEPROXY(idm)
DEVICEPROXY(gui)
#undef DEVICEPROXY

ServiceFromGuiProxy HardwareModule::serviceFromGuiProxy() const
{
    return m_serviceFromGuiProxy;
}

WeldHeadSubscribeProxy HardwareModule::weldHeadSubscribeProxy() const
{
    return m_weldHeadSubscribeProxy;
}

VideoRecorderProxy HardwareModule::videoRecorderProxy() const
{
    return m_videoRecorderProxy;
}

}
}
