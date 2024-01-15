#pragma once

#include "module/baseModule.h"
#include "event/inspectionCmd.proxy.h"
#include "event/recorder.interface.h"
#include "event/results.interface.h"
#include "event/systemStatus.interface.h"
#include "event/recorderPoll.proxy.h"
#include "event/querySystemStatus.proxy.h"
#include "common/systemConfiguration.h"
#include "message/calibrationCoordinatesRequest.proxy.h"
#include "event/schedulerEvents.proxy.h"
#include "event/storageUpdate.proxy.h"

#include <QObject>

class QTimer;
class QMutex;

namespace precitec
{

typedef std::shared_ptr<precitec::interface::TInspectionCmd<precitec::interface::EventProxy>> InspectionCmdProxy;
typedef std::shared_ptr<precitec::interface::TCalibrationCoordinatesRequest<precitec::interface::AbstractInterface>> CalibrationCoordinatesRequestProxy;
typedef std::shared_ptr<precitec::interface::TQuerySystemStatus<precitec::interface::EventProxy>> QuerySystemStatusProxy;
typedef std::shared_ptr<precitec::interface::TSchedulerEvents<precitec::interface::EventProxy>> SchedulerEventsProxy;

namespace storage
{

class ProductModel;
class ResultsServer;

}

namespace gui
{

class RecorderServer;
class SystemStatusServer;

namespace components
{
namespace logging
{

class LogNotificationServer;
typedef std::shared_ptr<LogNotificationServer> NotificationServer;

}
}

class AbstractModule : public QObject, public precitec::framework::module::ModuleManagerConnector
{
    Q_OBJECT

    /**
     * The name of the station.
     * For the HardwareModule this is the environment variable WM_STATION_NAME.
     * For the SimulationModule this is "SIMULATION"
     **/
    Q_PROPERTY(QByteArray station READ station WRITE setStation NOTIFY stationChanged)

    /**
     * Whether the Module is ready, that is all interfaces are connected.
     **/
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)

    Q_PROPERTY(precitec::InspectionCmdProxy inspectionCmdProxy READ inspectionCmdProxy CONSTANT)

    Q_PROPERTY(precitec::gui::RecorderServer* recorder READ recorder CONSTANT)

    Q_PROPERTY(precitec::storage::ResultsServer* results READ results CONSTANT)

    Q_PROPERTY(precitec::gui::SystemStatusServer* systemStatus READ systemStatus CONSTANT)

    Q_PROPERTY(precitec::CalibrationCoordinatesRequestProxy calibrationCoordinatesRequestProxy READ calibrationCoordinatesRequestProxy CONSTANT)

    Q_PROPERTY(precitec::storage::ProductModel* productModel READ productModel CONSTANT)

    Q_PROPERTY(precitec::SchedulerEventsProxy schedulerEventsProxy READ schedulerEventsProxy CONSTANT)

    /**
     * SystemConfiguration keys
     * Read from SystemConfig.xml
     **/

    /**
     * Whether the system has a sensor grabber (e.g. framegrabber).
     * Controlled by SystemConfiguration key "HardwareCameraEnabled".
     **/
    Q_PROPERTY(bool sensorGrabberEnabled READ sensorGrabberEnabled CONSTANT)

    Q_PROPERTY(bool idmEnabled READ idmEnabled CONSTANT)

    Q_PROPERTY(bool coaxCameraEnabled READ coaxCameraEnabled CONSTANT)

    Q_PROPERTY(bool scheimpflugCameraEnabled READ scheimpflugCameraEnabled CONSTANT)

    Q_PROPERTY(bool ledCameraEnabled READ ledCameraEnabled CONSTANT)

    Q_PROPERTY(bool lineLaser1Enabled READ lineLaser1Enabled CONSTANT)

    Q_PROPERTY(bool lineLaser2Enabled READ lineLaser2Enabled CONSTANT)

    Q_PROPERTY(bool lineLaser3Enabled READ lineLaser3Enabled CONSTANT)

    Q_PROPERTY(bool ledEnabled READ ledEnabled CONSTANT)

    Q_PROPERTY(bool laserControlEnabled READ laserControlEnabled CONSTANT)

    Q_PROPERTY(bool laserControlChannel2Enabled READ laserControlChannel2Enabled CONSTANT)

    Q_PROPERTY(bool scanTrackerEnabled READ scanTrackerEnabled CONSTANT)

    Q_PROPERTY(bool imageTriggerViaEncoderEnabled READ imageTriggerViaEncoderEnabled CONSTANT)

    /**
     * @c 0 framegrabber
     * @c 1 GigE
     **/
    Q_PROPERTY(int cameraInterfaceType READ cameraInterfaceType CONSTANT)

    Q_PROPERTY(bool encoderInput1Enabled READ encoderInput1Enabled CONSTANT)

    Q_PROPERTY(bool encoderInput2Enabled READ encoderInput2Enabled CONSTANT)

    Q_PROPERTY(bool zCollimatorEnabled READ zCollimatorEnabled CONSTANT)

    Q_PROPERTY(bool lwmEnabled READ lwmEnabled CONSTANT)

    Q_PROPERTY(bool externalLwmEnabled READ externalLwmEnabled CONSTANT)

    Q_PROPERTY(bool souvisPreInspectionEnabled READ souvisPreInspectionEnabled CONSTANT)

    Q_PROPERTY(bool newsonScannerEnabled READ newsonScannerEnabled CONSTANT)

    Q_PROPERTY(bool scanlabScannerEnabled READ scanlabScannerEnabled CONSTANT)

    Q_PROPERTY(bool souvisApplication READ souvisApplication CONSTANT)

    Q_PROPERTY(bool headMonitorEnabled READ headMonitorEnabled CONSTANT)

    Q_PROPERTY(bool octWithReferenceArms READ octWithReferenceArms CONSTANT)

    Q_PROPERTY(precitec::gui::AbstractModule::ScannerGeneralMode scannerGeneralMode READ scannerGeneralMode CONSTANT)

    Q_PROPERTY(bool liquidLensEnabled READ liquidLensEnabled CONSTANT)


public:
    enum class ScannerGeneralMode
    {
        ScanMasterMode = int(interface::ScannerGeneralMode::eScanMasterMode),
        ScanTracker2DMode = int(interface::ScannerGeneralMode::eScantracker2DMode),
    };
    Q_ENUM(ScannerGeneralMode)
    ~AbstractModule() override;

    precitec::InspectionCmdProxy inspectionCmdProxy() const
    {
        return m_inspectionCmdProxy;
    }

    RecorderServer* recorder() const
    {
        return m_recorderServer;
    }

    precitec::storage::ResultsServer* results() const
    {
        return m_resultsServer;
    }

    SystemStatusServer* systemStatus() const
    {
        return m_systemStatusServer;
    }

    CalibrationCoordinatesRequestProxy calibrationCoordinatesRequestProxy() const
    {
        return m_calibrationCoordinatesRequestProxy;
    }

    QByteArray station() const
    {
        return m_stationName;
    }
    void setStation(const QByteArray& station);

    precitec::storage::ProductModel* productModel() const
    {
        return m_products.get();
    }

    precitec::SchedulerEventsProxy schedulerEventsProxy() const
    {
        return m_schedulerEventsProxy;
    }

    Q_INVOKABLE void quitSystemFault();
    Q_INVOKABLE bool prepareRestore();

    bool isReady() const;

    bool sensorGrabberEnabled() const
    {
        return m_sensorGrabberEnabled;
    }   

    bool idmEnabled() const
    {
        return m_idmEnabled;
    }   
      bool coaxCameraEnabled() const
    {
        return m_coaxCameraEnabled;
    }

    bool scheimpflugCameraEnabled() const
    {
        return m_scheimpflugCameraEnabled;
    }

    bool ledCameraEnabled() const
    {
        return m_ledCameraEnabled;
    }

    bool lineLaser1Enabled() const
    {
        return m_lineLaser1Enabled;
    }

    bool lineLaser2Enabled() const
    {
        return m_lineLaser2Enabled;
    }

    bool lineLaser3Enabled() const
    {
        return m_lineLaser3Enabled;
    }

    bool ledEnabled() const
    {
        return m_ledEnabled;
    }

    bool laserControlEnabled() const
    {
        return m_laserControlEnabled;
    }

    bool laserControlChannel2Enabled() const
    {
        return m_laserControlChannel2Enabled;
    }

    bool scanTrackerEnabled() const
    {
        return m_scanTrackerEnabled;
    }

    bool imageTriggerViaEncoderEnabled() const
    {
        return m_imageTriggerViaEncoderEnabled;
    }

    int cameraInterfaceType() const
    {
        return m_cameraInterfaceType;
    }
    bool encoderInput1Enabled() const
    {
        return m_encoderInput1Enabled;
    }
    bool encoderInput2Enabled() const
    {
        return m_encoderInput2Enabled;
    }

    bool zCollimatorEnabled() const
    {
        return m_zCollimatorEnabled;
    }

    bool lwmEnabled() const
    {
        return m_lwmEnabled;
    }

    bool externalLwmEnabled() const
    {
        return m_externalLwmEnabled;
    }
    bool souvisPreInspectionEnabled() const
    {
        return m_souvisPreInspection;
    }
    bool newsonScannerEnabled() const
    {
        return m_newsonScannerEnabled;
    }

    bool scanlabScannerEnabled() const
    {
        return m_scanlabScannerEnabled;
    }

    bool souvisApplication() const
    {
        return m_souvisApplication;
    }
    
    bool headMonitorEnabled() const
    {
        return m_headMonitorEnabled;
    }

    bool octWithReferenceArms() const
    {
        return m_octWithReferenceArms;
    }

    ScannerGeneralMode scannerGeneralMode() const
    {
        return m_scannerGeneralMode;
    }

    bool liquidLensEnabled() const
    {
        return m_liquidLensEnabled;
    }





    Q_INVOKABLE void logDebug(const QString &message);
    Q_INVOKABLE void logInfo(const QString &message);
    Q_INVOKABLE void logWarning(const QString &message);
    Q_INVOKABLE void logError(const QString &message);

Q_SIGNALS:
    void stationChanged();
    void readyChanged();
    void interfacesConnected();

protected:
    AbstractModule(const QByteArray& station, QObject* parent = nullptr);

    precitec::interface::ModuleSpec getMyModuleSpec() override;
    virtual void init();
    virtual void registerPublications();
    virtual void markDevicesAsConnected();

    QuerySystemStatusProxy querySystemStatusProxy() const
    {
        return m_querySystemStatusProxy;
    }

private:
    QByteArray m_stationName;

    precitec::storage::ResultsServer* m_resultsServer;
    std::unique_ptr<precitec::interface::TResults<precitec::interface::EventHandler>> m_resultsHandler;

    RecorderServer* m_recorderServer;
    std::unique_ptr<precitec::interface::TRecorder<precitec::interface::EventHandler>> m_recorderHandler;

    SystemStatusServer* m_systemStatusServer;
    std::unique_ptr<precitec::interface::TSystemStatus<precitec::interface::EventHandler>> m_systemStatusHandler;

    InspectionCmdProxy m_inspectionCmdProxy;
    std::shared_ptr<precitec::interface::TCalibrationCoordinatesRequest<precitec::interface::MsgProxy>> m_calibrationCoordinatesRequestProxy;

    QuerySystemStatusProxy m_querySystemStatusProxy;
    std::shared_ptr<precitec::interface::TRecorderPoll<precitec::interface::EventProxy>> m_recorderPollProxy;
    std::shared_ptr<precitec::storage::ProductModel> m_products;

    SchedulerEventsProxy m_schedulerEventsProxy;
    
    std::shared_ptr<precitec::interface::TStorageUpdate<precitec::interface::EventProxy>> m_storageUpdateProxy;

    std::unique_ptr<QMutex> m_readyMutex;
    
    bool m_ready = false;

    bool m_sensorGrabberEnabled = false;
    
    bool m_idmEnabled;
    
    bool m_coaxCameraEnabled;

    bool m_scheimpflugCameraEnabled;

    bool m_ledCameraEnabled;

    bool m_lineLaser1Enabled;

    bool m_lineLaser2Enabled;

    bool m_lineLaser3Enabled;

    bool m_ledEnabled;

    bool m_laserControlEnabled;

    bool m_laserControlChannel2Enabled;

    bool m_imageTriggerViaEncoderEnabled;

    int m_cameraInterfaceType;

    bool m_scanTrackerEnabled;

    bool m_encoderInput1Enabled;

    bool m_encoderInput2Enabled;

    bool m_zCollimatorEnabled;

    bool m_lwmEnabled;

    bool m_externalLwmEnabled;

    bool m_souvisPreInspection;

    bool m_newsonScannerEnabled;

    bool m_scanlabScannerEnabled;

    bool m_souvisApplication;

    bool m_headMonitorEnabled;

    bool m_octWithReferenceArms;

    ScannerGeneralMode m_scannerGeneralMode;

    bool m_liquidLensEnabled;


    QTimer *m_querySystemStatusTimer;
};

}
}

