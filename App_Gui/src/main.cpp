#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QAbstractItemModel>
#include <QSettings>
#include <QtQuickControls2/QQuickStyle>
#include "assemblyImageInspectionModel.h"
#include "assemblyImagesModel.h"
#include "attribute.h"
#include "attributeFileInformation.h"
#include "attributeModel.h"
#include "axisController.h"
#include "axisInformation.h"
#include "axisStatusModel.h"
#include "checkedFilterModel.h"
#include "configurationTabModel.h"
#include "deleteProductInstanceController.h"
#include "detectionController.h"
#include "deviceKeyValueChangeEntry.h"
#include "deviceKeyModel.h"
#include "deviceKeySortFilterModel.h"
#include "deviceNotificationServer.h"
#include "deviceProxyWrapper.h"
#include "devicesTabModel.h"
#include "divideBy16Validator.h"
#include "emergencyConfigurationTabModel.h"
#include "errorSettingModel.h"
#include "errorTemplateModel.h"
#include "fieldIlluminationModel.h"
#include "filterAttributeModel.h"
#include "filterAttributeSortFilterModel.h"
#include "filterGroupsModel.h"
#include "filterInstanceModel.h"
#include "filterInstanceGroupFilterModel.h"
#include "filterParameterOnSeamConfigurationController.h"
#include "guiConfiguration.h"
#include "hardwareRoiController.h"
#include "hardwareRoiGigEController.h"
#include "hardwareRoiFlushedChangeEntry.h"
#include "hardwareParameterOverriddenModel.h"
#include "hardwareParameterOverridesModel.h"
#include "hardwareParameterFilterModel.h"
#include "hardwareParameterController.h"
#include "hardwareParameterSeamModel.h"
#include "hardwareParameterSeriesModel.h"
#include "hardwareParameterProductModel.h"
#include "hardwareParametersOverviewModel.h"
#include "hardwareParametersOverviewSortFilterModel.h"
#include "abstractGraphModel.h"
#include "graphModel.h"
#include "latestResultsModel.h"
#include "lineLaserModel.h"
#include "resultSettingModel.h"
#include "resultSettingFilterModel.h"
#include "errorTemplateFilterModel.h"
#include "resultSetting.h"
#include "hardwareModule.h"
#include "simulationModule.h"
#include "nioSettingModel.h"
#include "openVpnController.h"
#include "parameterSet.h"
#include "parameterSetsDeltaModel.h"
#include "parameterSetToGraphDeltaModel.h"
#include "permissions.h"
#include "product.h"
#include "productAddedChangeEntry.h"
#include "productDeletedChangeEntry.h"
#include "productController.h"
#include "productFilterModel.h"
#include "productInstanceModel.h"
#include "productInstancesTransferController.h"
#include "productInstancesCacheController.h"
#include "productInstanceTableModel.h"
#include "productInstanceSeriesModel.h"
#include "productInstanceSortModel.h"
#include "productInstanceSeamModel.h"
#include "productInstanceSeamSortModel.h"
#include "productSeamModel.h"
#include "productModel.h"
#include "productModifiedChangeEntry.h"
#include "productTypeValidator.h"
#include "productWizardFilterModel.h"
#include "parametersExporter.h"
#include "quitSystemFaultChangeEntry.h"
#include "recorderServer.h"
#include "referenceRunYAxisChangeEntry.h"
#include "remoteDesktopController.h"
#include "resultChangeEntry.h"
#include "resultsLoader.h"
#include "resultsDataSetModel.h"
#include "resultsExporter.h"
#include "resultsServer.h"
#include "resultsStatisticsController.h"
#include "resultsStatisticsSeamSeriesModel.h"
#include "resultsStatisticsSeamSeriesErrorModel.h"
#include "resultsStatisticsSeamModel.h"
#include "resultsStatisticsSeamErrorModel.h"
#include "resultsStatisticsSeamsModel.h"
#include "resultsStatisticsSeamsErrorModel.h"
#include "scanTrackerController.h"
#include "scanTrackerInformation.h"
#include "screenConfigurationController.h"
#include "seam.h"
#include "seamsOnAssemblyImageFilterModel.h"
#include "seamsOnAssemblyImageModel.h"
#include "seamInterval.h"
#include "seamSeries.h"
#include "seamWizardFilterModel.h"
#include "seamSeriesWizardFilterModel.h"
#include "simulationController.h"
#include "simulationImageFilterModel.h"
#include "simulationImageModel.h"
#include "serviceToGuiServer.h"
#include "subGraphModel.h"
#include "subGraphCategoryFilterModel.h"
#include "subGraphCheckedFilterModel.h"
#include "subGraphAlternativesModel.h"
#include "seamError.h"
#include "intervalError.h"
#include "simpleErrorModel.h"
#include "seamErrorModel.h"
#include "intervalErrorFilterModel.h"
#include "intervalErrorModel.h"
#include "seamErrorValueFilterModel.h"
#include "errorNioFilterModel.h"
#include "systemStatusServer.h"
#include "securityFilterModel.h"
#include "toolCenterPointController.h"
#include "verificationController.h"
#include "videoDataLoader.h"
#include "weldHeadServer.h"
#include "wizardFilterModel.h"
#include "wizardModel.h"
#include <precitec/permission.h>
#include <precitec/user.h>
#include <precitec/userLog.h>
#include <precitec/userManagement.h>
#include "onlineHelpFiles.h"
#include "idmController.h"
#include "sampleItem.h"
#include "idmCalibrationController.h"
#include "cameraCalibrationModel.h"
#include "ledChannel.h"
#include "ledCalibrationController.h"
#include "videoImportProductModel.h"
#include "videoImportProductInstanceModel.h"
#include "seamIntervalsVisualizeController.h"
#include "referenceCurvesController.h"
#include "referenceResultTypeFilterModel.h"
#include "abstractLaserControlModel.h"
#include "laserControlModel.h"
#include "laserControlMeasureModel.h"
#include "laserControlProductModel.h"
#include "laserControlPreset.h"
#include "laserControlPresetFilterModel.h"
#include "measureTaskNumberValidator.h"
#include "infoBoxModel.h"
#include "infoBoxFilterModel.h"
#include "infoTabModel.h"
#include "overlay/overlayPrimitive.h"
#include "fliplib/graphContainer.h"
#include "latestProductErrorsModel.h"
#include "historyModel.h"
#include "errorsDataModel.h"
#include "scanImageCalibrationController.h"
#include "resultsModel.h"
#include "resultsStatisticsModel.h"
#include "resultsFilterModel.h"
#include "focusPositionController.h"
#include "latestInstanceModel.h"
#include "weldmasterPaths.h"
#include "errorGroupModel.h"
#include "laserControlDelayController.h"
#include "errorGroupFilterModel.h"
#include "colorMapModel.h"
#include "abstractMeasureTask.h"
#include "overlyingErrorModel.h"
#include "seamSeriesErrorModel.h"
#include "seamSeriesError.h"
#include "productErrorModel.h"
#include "productError.h"
#include "intervalErrorConfigModel.h"
#include "intervalErrorSimpleConfigModel.h"
#include "staticErrorConfigController.h"
#include "referenceErrorConfigController.h"
#include "sensorSettingsModel.h"
#include "seamSelectionModel.h"
#include "seamPropertyModel.h"
#include "seamPropertyFilterModel.h"
#include "scanfieldCalibrationController.h"
#include "referenceImageController.h"
#include "qualityNormModel.h"
#include "qualityNorm.h"
#include "qualityNormResult.h"
#include "scrollBarSynchController.h"
#include "resultsSeriesLoader.h"
#include "seamSeriesResultsModel.h"
#include "laserControlPresetModel.h"
#include "plotterFilterModel.h"
#include "abstractPlotterDataModel.h"
#include "abstractSingleSeamDataModel.h"
#include "abstractMultiSeamDataModel.h"
#include "linkedSeamWizardFilterModel.h"
#include "referenceCurve.h"
#include "referenceCurvesModel.h"
#include "lwmController.h"
#include "hardwareParameters.h"
#include "scanLabController.h"
#include "topBarButtonModel.h"
#include "topBarButtonFilterModel.h"
#include "lwmResultFilterModel.h"
#include "scanfieldSeamModel.h"
#include "scanfieldSeamFilterModel.h"
#include "scanfieldSeamController.h"
#include "scanfieldModule.h"
#include "hardwareParametersModule.h"
#include "instanceResultModel.h"
#include "instanceResultSortModel.h"
#include "referenceCurveConstructor.h"
#include "extendedProductInfoHelper.h"
#include "figureSimulationPilotLaserController.h"
#include "assemblyImageFromProductInstanceTableModel.h"
#include "basicFigureSelectionModel.h"
#include "scanTracker2DHardwareParameterController.h"
#include "resultTemplateFilterModel.h"
#include "resultTemplateModel.h"
#include "scanfieldImageToJpgConverter.h"
#include "SettingNameFilterModel.h"
#include "postProcessingModule.h"

#include "module/moduleLogger.h"
#include "module/interfaces.h"

#include "common/connectionConfiguration.h"

#include <QLibraryInfo>
#include <QQmlContext>
#include <QSocketNotifier>
#include <QTranslator>
#include <QtConcurrentRun>
#include <QWindow>

#include <precitec/notificationSystem.h>

#include <sys/signalfd.h>
#include <signal.h>
#include <unistd.h>

#include <weldmasterVersion.h>
#include <changesetId.h>

std::unique_ptr<precitec::ModuleLogger> g_pLogger = nullptr;

Q_DECLARE_METATYPE(precitec::InspectionCmdProxy)
Q_DECLARE_METATYPE(precitec::SimulationCmdProxy)
Q_DECLARE_METATYPE(precitec::interface::SmpKeyValue)
Q_DECLARE_METATYPE(precitec::ServiceFromGuiProxy)
Q_DECLARE_METATYPE(precitec::WeldHeadSubscribeProxy)
Q_DECLARE_METATYPE(precitec::StorageUpdateProxy)
Q_DECLARE_METATYPE(precitec::VideoRecorderProxy)
Q_DECLARE_METATYPE(precitec::interface::ImageContext)
Q_DECLARE_METATYPE(precitec::SchedulerEventsProxy)
Q_DECLARE_METATYPE(precitec::gui::DbProxy)
Q_DECLARE_METATYPE(precitec::gui::ResultsProxy)
Q_DECLARE_METATYPE(precitec::gui::RecorderProxy)    
Q_DECLARE_METATYPE(precitec::gui::SystemStatusProxy)
Q_DECLARE_METATYPE(precitec::gui::VideoRecorderProxy)
Q_DECLARE_METATYPE(precitec::gui::InspectionOutProxy)
Q_DECLARE_METATYPE(precitec::gui::CalibrationProxy)
Q_DECLARE_METATYPE(precitec::gui::WeldHeadMsgProxy)
Q_DECLARE_METATYPE(precitec::gui::TriggerCmdProxy)



using precitec::gui::components::user::UserManagement;
using precitec::interface::ConnectionConfiguration;
using precitec::gui::components::userLog::UserLog;
using precitec::gui::GuiConfiguration;
using precitec::gui::WeldmasterPaths;

namespace {
void notificationsMessageHandler(QtMsgType type, const QMessageLogContext &, const QString &message)
{
    switch (type)
    {
    case QtDebugMsg:
    case QtInfoMsg:
        precitec::gui::components::notifications::NotificationSystem::instance()->information(message);
        break;
    case QtWarningMsg:
        precitec::gui::components::notifications::NotificationSystem::instance()->warning(message);
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        precitec::gui::components::notifications::NotificationSystem::instance()->error(message);
        break;
    }
}

}

int main(int argc, char *argv[])
{
    // setup signal handler
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    sigprocmask(SIG_BLOCK, &mask, nullptr);
    int signalFd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);

    // set cpu affinity to cpu 1 - 0 is processing thread, 3 is ethercat master
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(1, &cpuset);
    CPU_SET(2, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);

    QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);
    QGuiApplication::setApplicationName(QStringLiteral("weldmaster"));
    QGuiApplication::setApplicationVersion(QStringLiteral(WELDMASTER_VERSION_STRING));
    QGuiApplication app(argc, argv);
    QGuiApplication::setFont(QStringLiteral("Noto Sans"));

    for (int i = 0; i < argc -1; i++)
    {
        if (qstrcmp(argv[i], "-pipePath") == 0)
        {
            int fd = open(argv[i+1], O_WRONLY);
            unlink(argv[i+1]);
            write(fd, "1", 1);
            close(fd);
            break;
        }
    }

    ConnectionConfiguration::instance().setInt(pidKeys[GUI_KEY_INDEX], getpid());

    g_pLogger = std::unique_ptr<precitec::ModuleLogger>(new precitec::ModuleLogger( precitec::system::module::ModuleName[precitec::system::module::Modules::UserInterfaceModul] ) );

    if (signalFd != -1)
    {
        QSocketNotifier *notifier = new QSocketNotifier(signalFd, QSocketNotifier::Read, &app);
        QObject::connect(notifier, &QSocketNotifier::activated, &app, &QCoreApplication::quit);
    }

    QIcon::setThemeName(QStringLiteral("precitec"));

    qRegisterMetaType<precitec::interface::ResultArgs>();
    qRegisterMetaType<precitec::gui::DeviceProxyWrapper*>();
    qRegisterMetaType<precitec::gui::DeviceNotificationServer*>();
    qRegisterMetaType<precitec::gui::ServiceToGuiServer*>();
    qRegisterMetaType<precitec::InspectionCmdProxy>();
    qRegisterMetaType<precitec::SimulationCmdProxy>();
    qRegisterMetaType<precitec::ServiceFromGuiProxy>();
    qRegisterMetaType<precitec::WeldHeadSubscribeProxy>();
    qRegisterMetaType<precitec::StorageUpdateProxy>();
    qRegisterMetaType<precitec::VideoRecorderProxy>();
    qRegisterMetaType<precitec::gui::RecorderServer*>();
    qRegisterMetaType<precitec::interface::SmpKeyValue>();
    qRegisterMetaType<precitec::storage::ResultsServer*>();
    qRegisterMetaType<precitec::gui::SimulationImageModel*>();
    qRegisterMetaType<precitec::gui::SystemStatusServer*>();
    qRegisterMetaType<precitec::gui::WeldHeadServer*>();
    qRegisterMetaType<precitec::storage::Attribute*>();
    qRegisterMetaType<precitec::image::Sample>();
    qRegisterMetaType<precitec::interface::ImageContext>();
    qRegisterMetaType<precitec::gui::WeldHeadServer*>();
    qRegisterMetaType<precitec::SchedulerEventsProxy>();
    qmlRegisterUncreatableType<precitec::storage::Product>("Precitec.AppGui", 1, 0, "Product", QString());
    qRegisterMetaType<precitec::storage::ParameterSet*>();
    qRegisterMetaType<precitec::storage::ProductModel*>();
    qRegisterMetaType<precitec::storage::ProductInstanceSortModel*>();
    qRegisterMetaType<precitec::storage::AbstractMeasureTask*>();
    qRegisterMetaType<precitec::storage::Seam*>();
    qRegisterMetaType<precitec::storage::SeamInterval*>();
    qRegisterMetaType<precitec::storage::SeamSeries*>();
    qRegisterMetaType<precitec::storage::ResultSettingModel*>();
    qRegisterMetaType<precitec::storage::ErrorSettingModel*>();
    qRegisterMetaType<precitec::storage::NioSettingModel*>();
    qRegisterMetaType<precitec::storage::QualityNorm*>();
    qRegisterMetaType<precitec::storage::QualityNormResult*>();
    qRegisterMetaType<precitec::gui::AbstractHardwareParameterModel*>();
    qRegisterMetaType<precitec::gui::AbstractLaserControlModel*>();
    qRegisterMetaType<precitec::gui::AbstractPlotterDataModel*>();
    qRegisterMetaType<precitec::gui::AbstractSingleSeamDataModel*>();
    qRegisterMetaType<precitec::gui::AbstractMultiSeamDataModel*>();
    qRegisterMetaType<QAbstractItemModel*>();
    qRegisterMetaType<precitec::gui::DbProxy>();
    qRegisterMetaType<precitec::gui::ResultsProxy>();
    qRegisterMetaType<precitec::gui::RecorderProxy>();
    qRegisterMetaType<precitec::gui::SystemStatusProxy>();
    qRegisterMetaType<precitec::gui::VideoRecorderProxy>();
    qRegisterMetaType<precitec::gui::SimulationCmdProxy>();
    qRegisterMetaType<precitec::gui::InspectionOutProxy>();
    qRegisterMetaType<precitec::gui::CalibrationProxy>();
    qRegisterMetaType<precitec::gui::WeldHeadMsgProxy>();
    qRegisterMetaType<precitec::gui::TriggerCmdProxy>();
    qRegisterMetaType<precitec::gui::InspectionOutProxy>();

    qmlRegisterType<precitec::gui::AssemblyImageInspectionModel>("Precitec.AppGui", 1, 0, "AssemblyImageInspectionModel");
    qmlRegisterType<precitec::gui::AssemblyImagesModel>("Precitec.AppGui", 1, 0, "AssemblyImagesModel");
    qmlRegisterType<precitec::gui::AxisController>("Precitec.AppGui", 1, 0, "AxisController");
    qmlRegisterType<precitec::gui::AxisInformation>("Precitec.AppGui", 1, 0, "AxisInformation");
    qmlRegisterType<precitec::gui::AxisStatusModel>("Precitec.AppGui", 1, 0, "AxisStatusModel");
    qmlRegisterType<precitec::gui::CheckedFilterModel>("Precitec.AppGui", 1, 0, "CheckedFilterModel");
    qmlRegisterType<precitec::gui::SimulationController>("Precitec.AppGui", 1, 0, "SimulationController");
    qmlRegisterType<precitec::gui::SimulationImageFilterModel>("Precitec.AppGui", 1, 0, "SimulationImageFilterModel");
    qmlRegisterType<precitec::gui::EmergencyConfigurationTabModel>("Precitec.AppGui", 1, 0, "EmergencyConfigurationTabModel");
    qmlRegisterType<precitec::gui::ConfigurationTabModel>("Precitec.AppGui", 1, 0, "ConfigurationTabModel");
    qmlRegisterType<precitec::gui::DevicesTabModel>("Precitec.AppGui", 1, 0, "DevicesTabModel");
    qmlRegisterType<precitec::gui::SecurityFilterModel>("Precitec.AppGui", 1, 0, "SecurityFilterModel");
    qmlRegisterType<precitec::gui::LatestResultsModel>("Precitec.AppGui", 1, 0, "LatestResultsModel");
    qmlRegisterType<precitec::gui::LatestInstanceModel>("Precitec.AppGui", 1, 0, "LatestInstanceModel");
    qmlRegisterType<precitec::gui::LatestProductErrorsModel>("Precitec.AppGui", 1, 0, "LatestProductErrorsModel");
    qmlRegisterType<precitec::gui::HistoryModel>("Precitec.AppGui", 1, 0, "HistoryModel");
    qmlRegisterType<precitec::gui::ErrorsDataModel>("Precitec.AppGui", 1, 0, "ErrorsDataModel");
    qmlRegisterType<precitec::gui::ResultsDataSetModel>("Precitec.AppGui", 1, 0, "ResultsDataSetModel");
    qmlRegisterType<precitec::gui::SeamSeriesResultsModel>("Precitec.AppGui", 1, 0, "SeamSeriesResultsModel");
    qmlRegisterType<precitec::gui::DeleteProductInstanceController>("Precitec.AppGui", 1, 0, "DeleteProductInstanceController");
    qmlRegisterType<precitec::gui::DeviceKeyModel>("Precitec.AppGui", 1, 0, "DeviceKeyModel");
    qmlRegisterType<precitec::gui::DeviceKeySortFilterModel>("Precitec.AppGui", 1, 0, "DeviceKeySortFilterModel");
    qmlRegisterType<precitec::gui::HardwareRoiController>("Precitec.AppGui", 1, 0, "HardwareRoiController");
    qmlRegisterType<precitec::gui::HardwareRoiGigEController>("Precitec.AppGui", 1, 0, "HardwareRoiGigEController");
    qmlRegisterType<precitec::gui::IdmController>("Precitec.AppGui", 1, 0, "IdmController");
    qmlRegisterType<precitec::gui::InfoBoxModel>("Precitec.AppGui", 1, 0, "InfoBoxModel");
    qmlRegisterType<precitec::gui::InfoBoxFilterModel>("Precitec.AppGui", 1, 0, "InfoBoxFilterModel");
    qmlRegisterType<precitec::gui::InfoTabModel>("Precitec.AppGui", 1, 0, "InfoTabModel");
    qmlRegisterType<precitec::gui::IDMCalibrationController>("Precitec.AppGui", 1, 0, "IDMCalibrationController");
    qmlRegisterType<precitec::gui::CameraCalibrationModel>("Precitec.AppGui", 1, 0, "CameraCalibrationModel");
    qmlRegisterType<precitec::gui::LEDChannel>("Precitec.AppGui", 1, 0, "LEDChannel");
    qmlRegisterType<precitec::gui::LEDCalibrationController>("Precitec.AppGui", 1, 0, "LEDCalibrationController");
    qmlRegisterType<precitec::gui::SampleItem>("Precitec.AppGui", 1, 0, "SampleItem");
    qmlRegisterType<precitec::gui::ProductFilterModel>("Precitec.AppGui", 1, 0, "ProductFilterModel");
    qmlRegisterType<precitec::gui::DetectionController>("Precitec.AppGui", 1, 0, "DetectionController");
    qmlRegisterType<precitec::gui::SimpleErrorModel>("Precitec.AppGui", 1, 0, "SimpleErrorModel");
    qmlRegisterType<precitec::gui::SeamErrorModel>("Precitec.AppGui", 1, 0, "SeamErrorModel");
    qmlRegisterType<precitec::gui::IntervalErrorFilterModel>("Precitec.AppGui", 1, 0, "IntervalErrorFilterModel");
    qmlRegisterType<precitec::gui::IntervalErrorModel>("Precitec.AppGui", 1, 0, "IntervalErrorModel");
    qmlRegisterType<precitec::gui::IntervalErrorConfigModel>("Precitec.AppGui", 1, 0, "IntervalErrorConfigModel");
    qmlRegisterType<precitec::gui::IntervalErrorSimpleConfigModel>("Precitec.AppGui", 1, 0, "IntervalErrorSimpleConfigModel");
    qmlRegisterType<precitec::gui::StaticErrorConfigController>("Precitec.AppGui", 1, 0, "StaticErrorConfigController");
    qmlRegisterType<precitec::gui::ReferenceErrorConfigController>("Precitec.AppGui", 1, 0, "ReferenceErrorConfigController");
    qmlRegisterType<precitec::gui::OverlyingErrorModel>("Precitec.AppGui", 1, 0, "OverlyingErrorModel");
    qmlRegisterType<precitec::gui::SeamSeriesErrorModel>("Precitec.AppGui", 1, 0, "SeamSeriesErrorModel");
    qmlRegisterType<precitec::gui::ProductErrorModel>("Precitec.AppGui", 1, 0, "ProductErrorModel");
    qmlRegisterType<precitec::gui::ReferenceCurvesController>("Precitec.AppGui", 1, 0, "ReferenceCurvesController");
    qmlRegisterType<precitec::gui::ReferenceCurvesModel>("Precitec.AppGui", 1, 0, "ReferenceCurvesModel");
    qmlRegisterType<precitec::gui::ReferenceResultTypeFilterModel>("Precitec.AppGui", 1, 0, "ReferenceResultTypeFilterModel");
    qmlRegisterType<precitec::gui::SeamErrorValueFilterModel>("Precitec.AppGui", 1, 0, "SeamErrorValueFilterModel");
    qmlRegisterType<precitec::gui::ErrorNioFilterModel>("Precitec.AppGui", 1, 0, "ErrorNioFilterModel");
    qmlRegisterType<precitec::gui::SeamIntervalsVisualizeController>("Precitec.AppGui", 1, 0, "SeamIntervalsVisualizeController");
    qmlRegisterType<precitec::gui::FilterAttributeSortFilterModel>("Precitec.AppGui", 1, 0, "FilterAttributeSortFilterModel");
    qmlRegisterType<precitec::gui::FilterInstanceGroupFilterModel>("Precitec.AppGui", 1, 0, "FilterInstanceGroupFilterModel");
    qmlRegisterType<precitec::gui::FilterParameterOnSeamConfigurationController>("Precitec.AppGui", 1, 0, "FilterParameterOnSeamConfigurationController");
    qmlRegisterType<precitec::gui::LineLaserModel>("Precitec.AppGui", 1, 0, "LineLaserModel");
    qmlRegisterType<precitec::gui::LaserControlModel>("Precitec.AppGui", 1, 0, "LaserControlModel");
    qmlRegisterType<precitec::gui::LaserControlMeasureModel>("Precitec.AppGui", 1, 0, "LaserControlMeasureModel");
    qmlRegisterType<precitec::gui::LaserControlProductModel>("Precitec.AppGui", 1, 0, "LaserControlProductModel");
    qmlRegisterType<precitec::gui::FieldIlluminationModel>("Precitec.AppGui", 1, 0, "FieldIlluminationModel");
    qmlRegisterType<precitec::gui::ProductController>("Precitec.AppGui", 1, 0, "ProductController");
    qmlRegisterType<precitec::gui::HardwareParameterFilterModel>("Precitec.AppGui", 1, 0, "HardwareParameterFilterModel");
    qmlRegisterType<precitec::gui::HardwareParameterSeamModel>("Precitec.AppGui", 1, 0, "HardwareParameterSeamModel");
    qmlRegisterType<precitec::gui::HardwareParameterSeriesModel>("Precitec.AppGui", 1, 0, "HardwareParameterSeriesModel");
    qmlRegisterType<precitec::gui::HardwareParameterController>("Precitec.AppGui", 1, 0, "HardwareParameterController");
    qmlRegisterType<precitec::gui::HardwareParameterProductModel>("Precitec.AppGui", 1, 0, "HardwareParameterProductModel");
    qmlRegisterType<precitec::gui::HardwareParametersOverviewModel>("Precitec.AppGui", 1, 0, "HardwareParametersOverviewModel");
    qmlRegisterType<precitec::gui::HardwareParametersOverviewSortFilterModel>("Precitec.AppGui", 1, 0, "HardwareParametersOverviewSortFilterModel");
    qmlRegisterType<precitec::gui::SeamsOnAssemblyImageModel>("Precitec.AppGui", 1, 0, "SeamsOnAssemblyImageModel");
    qmlRegisterType<precitec::gui::SeamsOnAssemblyImageFilterModel>("Precitec.AppGui", 1, 0, "SeamsOnAssemblyImageFilterModel");
    qmlRegisterType<precitec::gui::ToolCenterPointController>("Precitec.AppGui", 1, 0, "ToolCenterPointController");
    qmlRegisterType<precitec::gui::ResultSettingFilterModel>("Precitec.AppGui", 1, 0, "ResultSettingFilterModel");
    qmlRegisterType<precitec::gui::ErrorTemplateFilterModel>("Precitec.AppGui", 1, 0, "ErrorTemplateFilterModel");
    qmlRegisterType<precitec::gui::PlotterFilterModel>("Precitec.AppGui", 1, 0, "PlotterFilterModel");
    qmlRegisterType<precitec::gui::VerificationController>("Precitec.AppGui", 1, 0, "VerificationController");
    qmlRegisterType<precitec::gui::VideoImportProductModel>("Precitec.AppGui", 1, 0, "VideoImportProductModel");
    qmlRegisterType<precitec::gui::VideoImportProductInstanceModel>("Precitec.AppGui", 1, 0, "VideoImportProductInstanceModel");
    qmlRegisterType<precitec::gui::ProductWizardFilterModel>("Precitec.AppGui", 1, 0, "ProductWizardFilterModel");
    qmlRegisterType<precitec::gui::LinkedSeamWizardFilterModel>("Precitec.AppGui", 1, 0, "LinkedSeamWizardFilterModel");
    qmlRegisterType<precitec::gui::SeamWizardFilterModel>("Precitec.AppGui", 1, 0, "SeamWizardFilterModel");
    qmlRegisterType<precitec::gui::SeamSeriesWizardFilterModel>("Precitec.AppGui", 1, 0, "SeamSeriesWizardFilterModel");
    qmlRegisterType<precitec::gui::WizardFilterModel>("Precitec.AppGui", 1, 0, "WizardFilterModel");
    qmlRegisterType<precitec::gui::WizardModel>("Precitec.AppGui", 1, 0, "WizardModel");
    qmlRegisterType<precitec::gui::ResultsFilterModel>("Precitec.AppGui", 1, 0, "ResultsFilterModel");
    qmlRegisterType<precitec::gui::ResultsModel>("Precitec.AppGui", 1, 0, "ResultsModel");
    qmlRegisterType<precitec::gui::ResultsStatisticsModel>("Precitec.AppGui", 1, 0, "ResultsStatisticsModel");
    qmlRegisterType<precitec::gui::ProductTypeValidator>("Precitec.AppGui", 1, 0, "ProductTypeValidator");
    qmlRegisterType<precitec::gui::MeasureTaskNumberValidator>("Precitec.AppGui", 1, 0, "MeasureTaskNumberValidator");
    qmlRegisterType<precitec::gui::RemoteDesktopController>("Precitec.AppGui", 1, 0, "RemoteDesktopController");
    qmlRegisterType<precitec::gui::ScanTrackerController>("Precitec.AppGui", 1, 0, "ScanTrackerController");
    qmlRegisterType<precitec::gui::ScanTrackerInformation>("Precitec.AppGui", 1, 0, "ScanTrackerInformation");
    qmlRegisterType<precitec::gui::ScreenConfigurationController>("Precitec.AppGui", 1, 0, "ScreenConfigurationController");
    qmlRegisterType<precitec::gui::OpenVpnController>("Precitec.AppGui", 1, 0, "OpenVpnController");
    qmlRegisterType<precitec::gui::ScanImageCalibrationController>("Precitec.AppGui", 1, 0, "ScanImageCalibrationController");
    qmlRegisterType<precitec::gui::LaserControlDelayController>("Precitec.AppGui", 1, 0, "LaserControlDelayController");
    qmlRegisterType<precitec::gui::FocusPositionController>("Precitec.AppGui", 1, 0, "FocusPositionController");
    qmlRegisterType<precitec::gui::ErrorGroupModel>("Precitec.AppGui", 1, 0, "ErrorGroupModel");
    qmlRegisterType<precitec::gui::ErrorGroupFilterModel>("Precitec.AppGui", 1, 0, "ErrorGroupFilterModel");
    qmlRegisterType<precitec::gui::ColorMapModel>("Precitec.AppGui", 1, 0, "ColorMapModel");
    qmlRegisterType<precitec::gui::DivideBy16Validator>("Precitec.AppGui", 1, 0, "DivideBy16Validator");
    qmlRegisterType<precitec::gui::ErrorTemplateModel>("Precitec.AppGui", 1, 0, "ErrorTemplateModel");
    qmlRegisterType<precitec::gui::SeamSelectionModel>("Precitec.AppGui", 1, 0, "SeamSelectionModel");
    qmlRegisterType<precitec::gui::SeamPropertyModel>("Precitec.AppGui", 1, 0, "SeamPropertyModel");
    qmlRegisterType<precitec::gui::SeamPropertyFilterModel>("Precitec.AppGui", 1, 0, "SeamPropertyFilterModel");
    qmlRegisterType<precitec::gui::ScanfieldCalibrationController>("Precitec.AppGui", 1, 0, "ScanfieldCalibrationController");
    qmlRegisterType<precitec::gui::ReferenceImageController>("Precitec.AppGui", 1, 0, "ReferenceImageController");
    qmlRegisterType<precitec::gui::ScrollBarSynchController>("Precitec.AppGui", 1, 0, "ScrollBarSynchController");
    qmlRegisterType<precitec::gui::LaserControlPresetModel>("Precitec.AppGui", 1, 0, "LaserControlPresetModel");
    qmlRegisterType<precitec::gui::LaserControlPresetFilterModel>("Precitec.AppGui", 1, 0, "LaserControlPresetFilterModel");
    qmlRegisterType<precitec::gui::HardwareParameterOverriddenModel>("Precitec.AppGui", 1, 0, "HardwareParameterOverriddenModel");
    qmlRegisterType<precitec::gui::HardwareParameterOverridesModel>("Precitec.AppGui", 1, 0, "HardwareParameterOverridesModel");
    qmlRegisterType<precitec::gui::ParameterSetsDeltaModel>("Precitec.AppGui", 1, 0, "ParameterSetsDeltaModel");
    qmlRegisterType<precitec::gui::ParameterSetToGraphDeltaModel>("Precitec.AppGui", 1, 0, "ParameterSetToGraphDeltaModel");
    qmlRegisterType<precitec::gui::LwmController>("Precitec.AppGui", 1, 0, "LwmController");
    qmlRegisterType<precitec::gui::ScanLabController>("Precitec.AppGui", 1, 0, "ScanLabController");
    qmlRegisterType<precitec::gui::TopBarButtonModel>("Precitec.AppGui", 1, 0, "TopBarButtonModel");
    qmlRegisterType<precitec::gui::LwmResultFilterModel>("Precitec.AppGui", 1, 0, "LwmResultFilterModel");
    qmlRegisterType<precitec::gui::TopBarButtonFilterModel>("Precitec.AppGui", 1, 0, "TopBarButtonFilterModel");
    qmlRegisterType<precitec::gui::ReferenceCurveConstructor>("Precitec.AppGui", 1, 0, "ReferenceCurveConstructor");
    qmlRegisterType<precitec::gui::SubGraphAlternativesModel>("Precitec.AppGui", 1, 0, "SubGraphAlternativesModel");
    qmlRegisterType<precitec::gui::ParametersExporter>("Precitec.AppGui", 1, 0, "ParametersExporter");
    qmlRegisterType<precitec::gui::ScanfieldSeamModel>("Precitec.AppGui", 1, 0, "ScanfieldSeamModel");
    qmlRegisterType<precitec::gui::ScanfieldSeamFilterModel>("Precitec.AppGui", 1, 0, "ScanfieldSeamFilterModel");
    qmlRegisterType<precitec::gui::ScanfieldSeamController>("Precitec.AppGui", 1, 0, "ScanfieldSeamController");
    qmlRegisterType<precitec::gui::ScanfieldModule>("Precitec.AppGui", 1, 0, "ScanfieldModule");
    qmlRegisterType<precitec::gui::HardwareParametersModule>("Precitec.AppGui", 1, 0, "HardwareParametersModule");
    qmlRegisterType<precitec::gui::ProductInstancesTransferController>("Precitec.AppGui", 1, 0, "ProductInstancesTransferController");
    qmlRegisterType<precitec::gui::ProductInstancesCacheController>("Precitec.AppGui", 1, 0, "ProductInstancesCacheController");
    qmlRegisterType<precitec::gui::InstanceResultModel>("Precitec.AppGui", 1, 0, "InstanceResultModel");
    qmlRegisterType<precitec::gui::InstanceResultSortModel>("Precitec.AppGui", 1, 0, "InstanceResultSortModel");
    qmlRegisterType<precitec::gui::FigureSimulationPilotLaserController>("Precitec.AppGui", 1, 0, "FigureSimulationPilotLaserController");
    qmlRegisterType<precitec::gui::AssemblyImageFromProductInstanceTableModel>("Precitec.AppGui", 1, 0, "AssemblyImageFromProductInstanceTableModel");
    qmlRegisterType<precitec::gui::BasicFigureSelectionModel>("Precitec.AppGui", 1, 0, "BasicFigureSelectionModel");
    qmlRegisterType<precitec::gui::ScanTracker2DHardwareParameterController>("Precitec.AppGui", 1, 0, "ScanTracker2DHardwareParameterController");
    qmlRegisterType<precitec::storage::AttributeModel>("Precitec.AppGui", 1, 0, "AttributeModel");
    qmlRegisterType<precitec::storage::FilterAttributeModel>("Precitec.AppGui", 1, 0, "FilterAttributeModel");
    qmlRegisterType<precitec::storage::FilterGroupsModel>("Precitec.AppGui", 1, 0, "FilterGroupsModel");
    qmlRegisterType<precitec::storage::FilterInstanceModel>("Precitec.AppGui", 1, 0, "FilterInstanceModel");
    qmlRegisterType<precitec::storage::GraphModel>("Precitec.AppGui", 1, 0, "GraphModel");
    qmlRegisterType<precitec::storage::AbstractGraphModel>("Precitec.AppGui", 1, 0, "AbstractGraphModel");
    qmlRegisterType<precitec::storage::ProductInstanceModel>("Precitec.AppGui", 1, 0, "ProductInstanceModel");
    qmlRegisterType<precitec::storage::ProductInstanceTableModel>("Precitec.AppGui", 1, 0, "ProductInstanceTableModel");
    qmlRegisterType<precitec::storage::ProductInstanceSeriesModel>("Precitec.AppGui", 1, 0, "ProductInstanceSeriesModel");
    qmlRegisterType<precitec::storage::ProductInstanceSeamSortModel>("Precitec.AppGui", 1, 0, "ProductInstanceSeamSortModel");
    qmlRegisterType<precitec::storage::ProductInstanceSeamModel>("Precitec.AppGui", 1, 0, "ProductInstanceSeamModel");
    qmlRegisterType<precitec::storage::ProductSeamModel>("Precitec.AppGui", 1, 0, "ProductSeamModel");
    qmlRegisterType<precitec::storage::ResultsExporter>("Precitec.AppGui", 1, 0, "ResultsExporter");
    qmlRegisterType<precitec::storage::ResultsLoader>("Precitec.AppGui", 1, 0, "ResultsLoader");
    qmlRegisterType<precitec::storage::ResultsSeriesLoader>("Precitec.AppGui", 1, 0, "ResultsSeriesLoader");
    qmlRegisterType<precitec::storage::ResultsStatisticsController>("Precitec.AppGui", 1, 0, "ResultsStatisticsController");
    qmlRegisterType<precitec::storage::ResultsStatisticsSeamSeriesModel>("Precitec.AppGui", 1, 0, "ResultsStatisticsSeamSeriesModel");
    qmlRegisterType<precitec::storage::ResultsStatisticsSeamSeriesErrorModel>("Precitec.AppGui", 1, 0, "ResultsStatisticsSeamSeriesErrorModel");
    qmlRegisterType<precitec::storage::ResultsStatisticsSeamModel>("Precitec.AppGui", 1, 0, "ResultsStatisticsSeamModel");
    qmlRegisterType<precitec::storage::ResultsStatisticsSeamErrorModel>("Precitec.AppGui", 1, 0, "ResultsStatisticsSeamErrorModel");
    qmlRegisterType<precitec::storage::ResultsStatisticsSeamsModel>("Precitec.AppGui", 1, 0, "ResultsStatisticsSeamsModel");
    qmlRegisterType<precitec::storage::ResultsStatisticsSeamsErrorModel>("Precitec.AppGui", 1, 0, "ResultsStatisticsSeamsErrorModel");
    qmlRegisterType<precitec::storage::ResultSettingModel>("Precitec.AppGui", 1, 0, "ResultSettingModel");
    qmlRegisterType<precitec::storage::ErrorSettingModel>("Precitec.AppGui", 1, 0, "ErrorSettingModel");
    qmlRegisterType<precitec::storage::SensorSettingsModel>("Precitec.AppGui", 1, 0, "SensorSettingsModel");
    qmlRegisterType<precitec::storage::NioSettingModel>("Precitec.AppGui", 1, 0, "NioSettingModel");
    qmlRegisterType<precitec::storage::ProductInstanceSortModel>("Precitec.AppGui", 1, 0, "ProductInstanceSortModel");
    qmlRegisterType<precitec::storage::ProductError>("Precitec.AppGui", 1, 0, "ProductError");
    qmlRegisterType<precitec::storage::SeamSeriesError>("Precitec.AppGui", 1, 0, "SeamSeriesError");
    qmlRegisterType<precitec::storage::SeamError>("Precitec.AppGui", 1, 0, "SeamError");
    qmlRegisterType<precitec::storage::IntervalError>("Precitec.AppGui", 1, 0, "IntervalError");
    qmlRegisterType<precitec::storage::ReferenceCurve>("Precitec.AppGui", 1, 0, "ReferenceCurve");
    qmlRegisterType<precitec::storage::SubGraphModel>("Precitec.AppGui", 1, 0, "SubGraphModel");
    qmlRegisterType<precitec::storage::LaserControlPreset>("Precitec.AppGui", 1, 0, "LaserControlPreset");
    qmlRegisterType<precitec::storage::SubGraphCategoryFilterModel>("Precitec.AppGui", 1, 0, "SubGraphCategoryFilterModel");
    qmlRegisterType<precitec::storage::SubGraphCheckedFilterModel>("Precitec.AppGui", 1, 0, "SubGraphCheckedFilterModel");
    qmlRegisterType<precitec::storage::ResultSetting>("Precitec.AppGui", 1, 0, "ResultSetting");
    qmlRegisterType<precitec::storage::QualityNormModel>("Precitec.AppGui", 1, 0, "QualityNormModel");
    qmlRegisterType<precitec::storage::VideoDataLoader>("Precitec.AppGui", 1, 0, "VideoDataLoader");
    qmlRegisterAnonymousType<precitec::storage::ExtendedProductInfoHelper>("Precitec.AppGui", 1);
    qmlRegisterUncreatableType<precitec::storage::Parameter>("Precitec.AppGui", 1, 0, "Parameter", QString());
    qmlRegisterUncreatableType<precitec::gui::SystemStatusServer>("Precitec.AppGui", 1, 0, "SystemStatusServer", QString());
    qmlRegisterUncreatableMetaObject(precitec::gui::staticMetaObject, "precitec.gui", 1, 0, "App", QString());
    qmlRegisterUncreatableMetaObject(precitec::gui::onlineHelp::staticMetaObject, "precitec.gui", 1, 0, "OnlineHelp", QString());
    qmlRegisterType<precitec::gui::ResultTemplateFilterModel>("Precitec.AppGui", 1, 0, "ResultTemplateFilterModel");
    qmlRegisterType<precitec::gui::ResultTemplateModel>("Precitec.AppGui", 1, 0, "ResultTemplateModel");
    qmlRegisterType<precitec::gui::SettingNameFilterModel>("Precitec.AppGui", 1, 0, "SettingNameFilterModel");

    qRegisterMetaType<precitec::gui::ClickAction>();
    qRegisterMetaType<precitec::gui::ScreenshotAction>();
    qRegisterMetaType<precitec::gui::TimerAction>();
    qRegisterMetaType<precitec::gui::WaitForChangeAction>();
    qRegisterMetaType<QPointer<precitec::storage::Product>>();
    qRegisterMetaType<QPointer<precitec::storage::Seam>>();
    qRegisterMetaType<precitec::storage::ResultSetting::Type>();
    qRegisterMetaType<precitec::storage::AttributeFileInformation>();

    qRegisterMetaType<std::vector< precitec::image::OverlayText >>();
    qRegisterMetaType<std::vector< fliplib::InstanceFilter >>();

    qmlRegisterSingletonType<precitec::gui::HardwareModule>("Precitec.AppGui", 1, 0, "HardwareModule",
        [](QQmlEngine *, QJSEngine *) -> QObject *
        {
            auto hardwareModule = precitec::gui::HardwareModule::instance();
            QQmlEngine::setObjectOwnership(hardwareModule, QQmlEngine::CppOwnership);
            return hardwareModule;
        });

    qmlRegisterSingletonType<precitec::gui::SimulationModule>("Precitec.AppGui", 1, 0, "SimulationModule",
        [](QQmlEngine *, QJSEngine *) -> QObject *
        {
            auto simulationModule = precitec::gui::SimulationModule::instance();
            std::cout << "SimulationModule instance created" << std::endl;
            QQmlEngine::setObjectOwnership(simulationModule, QQmlEngine::CppOwnership);
            return simulationModule;
        });

    qmlRegisterSingletonType<precitec::gui::HardwareParameters>("Precitec.AppGui", 1, 0, "HardwareParameters",
        [](QQmlEngine *, QJSEngine *) -> QObject *
        {
            auto hardwareParametes = precitec::gui::HardwareParameters::instance();

            QQmlEngine::setObjectOwnership(hardwareParametes, QQmlEngine::CppOwnership);
            return hardwareParametes;
        });
    std::cout << "Registering PostProcessingModule" << std::endl;
    
        qmlRegisterSingletonType<precitec::gui::PostProcessingModule>("Precitec.AppGui", 1, 0, "PostProcessingModule",
        [](QQmlEngine * engine, QJSEngine * scriptEngine) -> QObject *
        {          
            auto postProcessingModule = precitec::gui::PostProcessingModule::instance();
            std::cout << "PostProcessingModule instance created" << std::endl;
            QQmlEngine::setObjectOwnership(postProcessingModule, QQmlEngine::CppOwnership);
            return postProcessingModule;
        });
   
    std::cout << "Registering PostProcessingModule done" << std::endl;
    

    QSettings userSettings(WeldmasterPaths::instance()->configurationDir() + QStringLiteral("defaultUsers"), QSettings::IniFormat);
    QSettings roleSettings(WeldmasterPaths::instance()->configurationDir() + QStringLiteral("roles"), QSettings::IniFormat);

    // load language settings
    GuiConfiguration::instance()->setDefaultConfigFilePath(WeldmasterPaths::instance()->configurationDir() + QStringLiteral("defaultUiSettings"));
    GuiConfiguration::instance()->setConfigFilePath(WeldmasterPaths::instance()->configurationDir() + QStringLiteral("uiSettings"));
    GuiConfiguration::instance()->setupTranslators();

    UserManagement::instance()->installPermissions({
        {int(precitec::gui::Permission::ResetSystemStatus), QObject::tr("Reset system status")},
        {int(precitec::gui::Permission::ShutdownSystem), QObject::tr("Shutdown or reboot the system")},
        {int(precitec::gui::Permission::StopAllProcesses), QObject::tr("Terminate all running processes without physical system shutdown")},
        {int(precitec::gui::Permission::ViewGrabberDeviceConfig), QObject::tr("View grabber device configuration")},
        {int(precitec::gui::Permission::ViewCalibrationDeviceConfig), QObject::tr("View calibration device configuration")},
        {int(precitec::gui::Permission::ViewVideoRecorderDeviceConfig), QObject::tr("View video recorder device configuration")},
        {int(precitec::gui::Permission::ViewWeldHeadDeviceConfig), QObject::tr("View weld head device configuration")},
        {int(precitec::gui::Permission::ViewServiceDeviceConfig), QObject::tr("View service device configuration")},
        {int(precitec::gui::Permission::ViewInspectionDeviceConfig), QObject::tr("View inspection device configuration")},
        {int(precitec::gui::Permission::ViewWorkflowDeviceConfig), QObject::tr("View workflow device configuration")},
        {int(precitec::gui::Permission::ViewStorageDeviceConfig), QObject::tr("View storage device configuration")},
        {int(precitec::gui::Permission::ViewIDMDeviceConfig), QObject::tr("View OCT device configuration")},
        {int(precitec::gui::Permission::ViewGuiDeviceConfig), QObject::tr("View Gui device configuration")},
        {int(precitec::gui::Permission::EditGrabberDeviceConfig), QObject::tr("Edit grabber device configuration")},
        {int(precitec::gui::Permission::EditCalibrationDeviceConfig), QObject::tr("Edit calibration device configuration")},
        {int(precitec::gui::Permission::EditVideoRecorderDeviceConfig), QObject::tr("Edit video recorder device configuration")},
        {int(precitec::gui::Permission::EditWeldHeadDeviceConfig), QObject::tr("Edit weld head device configuration")},
        {int(precitec::gui::Permission::EditServiceDeviceConfig), QObject::tr("Edit service device configuration")},
        {int(precitec::gui::Permission::EditInspectionDeviceConfig), QObject::tr("Edit inspection device configuration")},
        {int(precitec::gui::Permission::EditWorkflowDeviceConfig), QObject::tr("Edit workflow device configuration")},
        {int(precitec::gui::Permission::EditStorageDeviceConfig), QObject::tr("Edit storage device configuration")},
        {int(precitec::gui::Permission::EditIDMDeviceConfig), QObject::tr("Edit OCT device configuration")},
        {int(precitec::gui::Permission::EditResultsConfig), QObject::tr("Edit results configuration")},
        {int(precitec::gui::Permission::EditGuiDeviceConfig), QObject::tr("Edit gui configuration")},
        {int(precitec::gui::Permission::RunHardwareAndProductWizard), QObject::tr("Run hardware and product wizard")},
        {int(precitec::gui::Permission::MountPortableDevices), QObject::tr("Mount portable devices")},
        {int(precitec::gui::Permission::PerformBackup), QObject::tr("Perform backup")},
        {int(precitec::gui::Permission::PerformRestore), QObject::tr("Perform restore")},
        {int(precitec::gui::Permission::PerformUpdate), QObject::tr("Perform updates")},
        {int(precitec::gui::Permission::SetToolCenterPoint), QObject::tr("Set or update Tool Center Point")},
        {int(precitec::gui::Permission::DownloadVideo), QObject::tr("Download video")},
        {int(precitec::gui::Permission::UploadVideo), QObject::tr("Upload video")},
        {int(precitec::gui::Permission::ViewEthercat), QObject::tr("View EtherCAT slaves")},
        {int(precitec::gui::Permission::ModifyEthercat), QObject::tr("Modify EtherCAT slaves")},
        {int(precitec::gui::Permission::ViewDebugLogMessages), QObject::tr("View debug log messages")},
        {int(precitec::gui::Permission::ClearLogMessages), QObject::tr("Clear log messages")},
        {int(precitec::gui::Permission::ViewUserLog), QObject::tr("View user log")},
        {int(precitec::gui::Permission::ViewFilterParameterAdmin), QObject::tr("View all filter parameters (super user)")},
        {int(precitec::gui::Permission::ViewFilterParameterSuperUser), QObject::tr("View filter parameters of level 1..3 (admin)")},
        {int(precitec::gui::Permission::ViewFilterParameterGroupLeader), QObject::tr("View filter parameters of level 2..3 (group leader)")},
        {int(precitec::gui::Permission::ViewFilterParameterOperator), QObject::tr("View filter parameters of level 3 (operator)")},
        {int(precitec::gui::Permission::AxisTriggerHardwareLimits), QObject::tr("Trigger hardware limits of y-Axis from user interface")},
        {int(precitec::gui::Permission::ViewSpsSimulation), QObject::tr("SpsSimulation page (development only)")},
        {int(precitec::gui::Permission::Localization), QObject::tr("Configure localization settings")},
        {int(precitec::gui::Permission::ConfigureNetworkDevices), QObject::tr("Configure network devices")},
        {int(precitec::gui::Permission::ConfigureUps), QObject::tr("Configure uninterruptible power supply")},
        {int(precitec::gui::Permission::BackupRestoreHardwareConfiguration), QObject::tr("Create and restore hardware configuration backups")},
        {int(precitec::gui::Permission::ExportGraph), QObject::tr("Export filter graph")},
        {int(precitec::gui::Permission::ExportResults), QObject::tr("Export result data to xlsx")},
        {int(precitec::gui::Permission::ViewSSHConfiguration), QObject::tr("View SSH configuration")},
        {int(precitec::gui::Permission::ImportSSHAuthorizedKey), QObject::tr("Import SSH public key to authorized keys")},
        {int(precitec::gui::Permission::RemoveSSHAuthorizedKey), QObject::tr("Remove SSH public key from authorized keys")},
        {int(precitec::gui::Permission::RemoteDesktop), QObject::tr("Start/stop remote desktop connectivity")},
        {int(precitec::gui::Permission::ConfigureScreen), QObject::tr("Start external screen configuration tool")},
        {int(precitec::gui::Permission::UploadResults), QObject::tr("Upload results")},
        {int(precitec::gui::Permission::DeleteVideo), QObject::tr("Delete video (product instance)")},
        {int(precitec::gui::Permission::ImportProducts), QObject::tr("Import products for Weldmaster (XML and Json)")},
        {int(precitec::gui::Permission::EditReferenceImage), QObject::tr("Edit reference image")},
        {int(precitec::gui::Permission::EditGraphsWithGrapheditor), QObject::tr("Edit graphs with graph editor")},
        {int(precitec::gui::Permission::TaskScheduler), QObject::tr("View task scheduler and add/edit/delete tasks")},
        {int(precitec::gui::Permission::SaveTemplate), QObject::tr("Create and save template from image")},
        {int(precitec::gui::Permission::OpenSystemNetworkConfiguration), QObject::tr("Open system network configuration tool")},
    });
    UserManagement::instance()->loadRoles(&roleSettings);
    UserManagement::instance()->loadUsers(&userSettings);

    UserLog::instance()->setLogDirectory(WeldmasterPaths::instance()->logfilesDir());
    UserLog::instance()->registerChange(precitec::gui::DeviceKeyValueChangeEntry::staticMetaObject);
    UserLog::instance()->registerChange(precitec::gui::HardwareRoiFlushedChangeEntry::staticMetaObject);
    UserLog::instance()->registerChange(precitec::gui::QuitSystemFaultChangeEntry::staticMetaObject);
    UserLog::instance()->registerChange(precitec::gui::ReferenceRunYAxisChangeEntry::staticMetaObject);
    UserLog::instance()->registerChange(precitec::storage::ResultChangeEntry::staticMetaObject);
    UserLog::instance()->registerChange(precitec::gui::ProductAddedChangeEntry::staticMetaObject);
    UserLog::instance()->registerChange(precitec::gui::ProductDeletedChangeEntry::staticMetaObject);
    UserLog::instance()->registerChange(precitec::gui::ProductModifiedChangeEntry::staticMetaObject);

    QQmlApplicationEngine engine;
    QObject::connect(GuiConfiguration::instance(), &GuiConfiguration::translatorsChanged, &engine, &QQmlApplicationEngine::retranslate);
    engine.addImportPath(WeldmasterPaths::instance()->baseDirectory() + QStringLiteral("/lib/plugins/qml"));
    engine.addImportPath(QStringLiteral("./plugins/qml"));
    QStringList paths = engine.importPathList();
    // append path to default library
    // we need to append it as with prepand it would be used for things like QtQuickControls and pick up system installed Qt
    paths << QStringLiteral("/usr/lib/x86_64-linux-gnu/qt5/qml");
    engine.setImportPathList(paths);

    engine.rootContext()->setContextProperty(QStringLiteral("weldmasterChangesetId"), QStringLiteral(WELDMASTER_CHANGESET_ID));
    engine.rootContext()->setContextProperty(QStringLiteral("weldmasterBuildTimestamp"), QStringLiteral(WELDMASTER_BUILD_TIMESTAMP));

    QQuickStyle::setStyle(QLatin1String("/usr/lib/x86_64-linux-gnu/qt5/qml/precitecqtquickstyle/"));
    QQuickStyle::setFallbackStyle(QString());
    if (qEnvironmentVariableIsSet("WM_RESTARTED"))
    {
        engine.load(QUrl(QStringLiteral("qrc:/resources/qml/emergency_main.qml")));
    } else
    {
        bool restarted = false;
        QtMessageHandler originalHandler = nullptr;
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
            [&restarted, &engine, originalHandler] (QObject *object)
            {
                if (object)
                {
                    qInstallMessageHandler(originalHandler);
                    if (qEnvironmentVariableIsSet("QT_WAYLAND_SHELL_INTEGRATION"))
                    {
                        qobject_cast<QWindow*>(object)->showFullScreen();
                    }
                    else
                    {
                        qobject_cast<QWindow*>(object)->showMaximized();
                    }

                    // convert existing scanfield images from bmp to jpg
                    QtConcurrent::run(
                        []
                        {
                            const auto& scanfieldDirs{QDir{WeldmasterPaths::instance()->scanFieldDir()}.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)};
                            for (const auto& scanfieldDir : scanfieldDirs)
                            {
                                precitec::gui::ScanfieldImageToJpgConverter converter{QDir{scanfieldDir.absoluteFilePath()}};
                                converter();
                            }
                        });
                    return;
                }
                if (restarted)
                {
                    // TODO: QWidget error?
                    return;
                }
                restarted = true;
                engine.load(QUrl(QStringLiteral("qrc:/resources/qml/emergency_main.qml")));
            });
        qInstallMessageHandler(notificationsMessageHandler);
        engine.load(QUrl(QStringLiteral("qrc:/resources/qml/main.qml")));
    }

    return app.exec();
}

#include "moc_permissions.cpp"
#include "moc_onlineHelpFiles.cpp"
