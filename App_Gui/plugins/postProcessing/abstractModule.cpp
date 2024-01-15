#include "abstractModule.h"
#include "permissions.h"
#include "quitSystemFaultChangeEntry.h"
#include "recorderServer.h"
#include "resultsServer.h"
#include "systemStatusServer.h"
#include "productModel.h"
#include "weldmasterPaths.h"

#include "common/systemConfiguration.h"
#include "common/connectionConfiguration.h"
#include "event/imageShMem.h"
#include "event/recorder.handler.h"
#include "event/results.handler.h"
#include "event/systemStatus.handler.h"
#include "message/calibrationCoordinatesRequest.proxy.h"

#include <precitec/userManagement.h>
#include <precitec/userLog.h>

#include <QMutex>
#include <QTimer>
#include <QCoreApplication>

using precitec::storage::ProductModel;
using precitec::storage::ResultsServer;
using precitec::gui::components::user::UserManagement;
using precitec::gui::components::userLog::UserLog;
using precitec::system::module::UserInterfaceModul;
using precitec::interface::TSchedulerEvents;
using precitec::interface::EventProxy;

namespace precitec
{
namespace gui
{
namespace components
{

namespace postProcessing
{
AbstractModule::AbstractModule(const QByteArray& station, QObject* parent)
    : QObject(parent)
    , ModuleManagerConnector(UserInterfaceModul)
    , m_stationName(station)
    , m_resultsServer(new ResultsServer{this})
    , m_resultsHandler(std::make_unique<TResults<EventHandler>>(m_resultsServer))
    , m_recorderServer(new RecorderServer{this})
    , m_recorderHandler(std::make_unique<TRecorder<EventHandler>>(m_recorderServer))
    , m_systemStatusServer(new SystemStatusServer{this})
    , m_systemStatusHandler(std::make_unique<TSystemStatus<EventHandler>>(m_systemStatusServer))
    , m_inspectionCmdProxy(std::make_shared<TInspectionCmd<EventProxy>>())
    , m_calibrationCoordinatesRequestProxy(std::make_shared<TCalibrationCoordinatesRequest<MsgProxy>>())
    , m_querySystemStatusProxy(std::make_shared<TQuerySystemStatus<EventProxy>>())
    , m_recorderPollProxy(std::make_shared<TRecorderPoll<EventProxy>>())
    , m_products(std::make_shared<ProductModel>())
    , m_schedulerEventsProxy(std::make_shared<TSchedulerEvents<EventProxy>>())
    , m_readyMutex(std::make_unique<QMutex>())
    , m_sensorGrabberEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::HardwareCameraEnabled))
    , m_idmEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::IDM_Device1Enable) || SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::CLS2_Device1Enable)) 
    , m_coaxCameraEnabled(m_sensorGrabberEnabled && SystemConfiguration::instance().get(SystemConfiguration::IntKey::Type_of_Sensor) == TypeOfSensor::eCoax)
    , m_scheimpflugCameraEnabled(m_sensorGrabberEnabled && SystemConfiguration::instance().get(SystemConfiguration::IntKey::Type_of_Sensor) == TypeOfSensor::eScheimpflug)
    , m_ledCameraEnabled(m_sensorGrabberEnabled && SystemConfiguration::instance().get(SystemConfiguration::IntKey::Type_of_Sensor) == TypeOfSensor::eLED)
    , m_lineLaser1Enabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::LineLaser1Enable))
    , m_lineLaser2Enabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::LineLaser2Enable))
    , m_lineLaser3Enabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::FieldLight1Enable))
    , m_ledEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::LED_IlluminationEnable))
    , m_laserControlEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::LaserControlEnable))
    , m_laserControlChannel2Enabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::LaserControlTwoChannel))
    , m_imageTriggerViaEncoderEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::ImageTriggerViaEncoderSignals))
    , m_cameraInterfaceType(SystemConfiguration::instance().get(SystemConfiguration::IntKey::CameraInterfaceType))
    , m_scanTrackerEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::ScanTrackerEnable))
    , m_encoderInput1Enabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::EncoderInput1Enable))
    , m_encoderInput2Enabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::EncoderInput2Enable))
    , m_zCollimatorEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::ZCollimatorEnable))
    , m_lwmEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::LWM40_No1_Enable))
    , m_externalLwmEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::Communication_To_LWM_Device_Enable))
    , m_souvisPreInspection(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::SOUVIS6000_Is_PreInspection))
    , m_newsonScannerEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::Newson_Scanner1Enable))
    , m_scanlabScannerEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::Scanner2DEnable))
    , m_souvisApplication(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::SOUVIS6000_Application))
    , m_headMonitorEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::HeadMonitorGatewayEnable))
    , m_octWithReferenceArms(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::OCT_with_reference_arms))
    , m_scannerGeneralMode(static_cast<ScannerGeneralMode>(SystemConfiguration::instance().get(SystemConfiguration::IntKey::ScannerGeneralMode)))
    , m_liquidLensEnabled(SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::LiquidLensControllerEnable))    
    , m_querySystemStatusTimer(new QTimer{this})
{
    connect(this, &AbstractModule::readyChanged, this, [this]
        {
            m_querySystemStatusTimer->stop();
            markDevicesAsConnected();
            const bool isSimulationStation = qstrcmp(m_stationName, QByteArrayLiteral("SIMULATION")) == 0;
            m_recorderServer->init(m_stationName.toStdString(), greyImage::sharedMemorySize(isSimulationStation));
            m_recorderServer->setRecorderPollProxy(m_recorderPollProxy);
            m_recorderServer->requestNextImage();

        }, Qt::QueuedConnection
    );

    m_recorderServer->setHasFrameGrabber(m_sensorGrabberEnabled);   
    m_products->setReferenceStorageDirectory(WeldmasterPaths::instance()->referenceCruveDir());
    m_products->setScanfieldImageStorageDirectory(WeldmasterPaths::instance()->scanFieldDir());
    m_products->loadProducts(QDir(WeldmasterPaths::instance()->productDir()));
    m_resultsServer->setProducts(m_products);

    m_querySystemStatusTimer->setInterval(std::chrono::seconds{30});
    m_querySystemStatusTimer->setSingleShot(true);
    connect(m_querySystemStatusTimer, &QTimer::timeout, this,
        [this]
        {
            m_querySystemStatusProxy->requestOperationState();
        }
    );
    connect(this, &AbstractModule::interfacesConnected, m_querySystemStatusTimer, [this] { m_querySystemStatusTimer->start(); }, Qt::QueuedConnection );
    connect(m_systemStatusServer, &SystemStatusServer::stateChanged, this,
        [this]
        {
            if (m_ready)
            {
                return;
            }
            m_ready = true;
            emit readyChanged();
        }, Qt::QueuedConnection
    );
}

AbstractModule::~AbstractModule() = default;

void AbstractModule::registerPublications()
{
    registerPublication(m_inspectionCmdProxy.get());
    registerPublication(m_querySystemStatusProxy.get());
    registerPublication(m_recorderPollProxy.get());
    registerSubscription(m_resultsHandler.get());
    registerSubscription(m_recorderHandler.get());
    registerSubscription(m_systemStatusHandler.get());
    registerPublication(m_calibrationCoordinatesRequestProxy.get());
    registerPublication(m_schedulerEventsProxy.get());
}

void AbstractModule::markDevicesAsConnected()
{
}

void AbstractModule::init()
{
    ConnectionConfiguration config(station().toStdString());

    initModuleManager(config);

    registerPublications();
    subscribeAllInterfaces();
    publishAllInterfaces();

    emit interfacesConnected();
}

void AbstractModule::quitSystemFault()
{
    if (!UserManagement::instance()->hasPermission(int(precitec::gui::Permission::ResetSystemStatus)))
    {
        return;
    }
    UserLog::instance()->addChange(new QuitSystemFaultChangeEntry{QString::fromUtf8(m_stationName)});
    m_inspectionCmdProxy->quitSystemFault();
}

bool AbstractModule::prepareRestore()
{
    if (!UserManagement::instance()->hasPermission(int(precitec::gui::Permission::PerformRestore)) && !UserManagement::instance()->hasPermission(int(precitec::gui::Permission::ShutdownSystem)))
    {
        return false;
    }
    m_inspectionCmdProxy->signalNotReady(512);
    return true;
}

void AbstractModule::setStation(const QByteArray& station)
{
    if (station == m_stationName)
    {
        return;
    }
    m_stationName = station;
    emit stationChanged();
}

ModuleSpec AbstractModule::getMyModuleSpec()
{
    return ModuleSpec(getMyAppId(), QCoreApplication::applicationFilePath().toStdString());
}

bool AbstractModule::isReady() const
{
    QMutexLocker lock(m_readyMutex.get());
    return m_ready;
}

void AbstractModule::logDebug(const QString &message)
{
    wmLog(eDebug, message.toStdString());
}

void AbstractModule::logInfo(const QString &message)
{
    wmLog(eInfo, message.toStdString());
}

void AbstractModule::logWarning(const QString &message)
{
    wmLog(eWarning, message.toStdString());
}

void AbstractModule::logError(const QString &message)
{
    wmLog(eError, message.toStdString());
}

}
}
}
}

