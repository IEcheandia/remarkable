#include <QQmlExtensionPlugin>
#include <QtQml/QtQml>


#include "postProcessingController.h"
#include "postProcessingSequenceDataReady.h"
#include "postProcessing.h"
#include "module/interfaces.h"
#include "../../Interfaces/include/message/db.proxy.h"
#include "../../Interfaces/include/event/triggerCmd.proxy.h"
#include "../../Interfaces/include/event/results.proxy.h"
#include "../../Interfaces/include/event/recorder.proxy.h"
#include "../../Interfaces/include/event/systemStatus.proxy.h"
#include "../../Interfaces/include/message/calibration.proxy.h"
#include "../../Interfaces/include/event/inspectionOut.proxy.h"
#include "../../Interfaces/include/message/weldHead.proxy.h"
#include "../../Interfaces/include/event/videoRecorder.proxy.h"
#include "../../Interfaces/include/event/storageUpdate.proxy.h"

 Q_DECLARE_METATYPE(precitec::gui::DbProxy)
    Q_DECLARE_METATYPE(precitec::gui::TriggerCmdProxy)
    Q_DECLARE_METATYPE(precitec::gui::ResultsProxy)
    Q_DECLARE_METATYPE(precitec::gui::RecorderProxy)
    Q_DECLARE_METATYPE(precitec::gui::SystemStatusProxy)
    Q_DECLARE_METATYPE(precitec::gui::CalibrationProxy)
    Q_DECLARE_METATYPE(precitec::gui::WeldHeadMsgProxy)
    Q_DECLARE_METATYPE(precitec::VideoRecorderProxy)
    Q_DECLARE_METATYPE(precitec::gui::InspectionOutProxy)


class PostProcessingPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)
   


public:
    void registerTypes(const char *uri);
};

void PostProcessingPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(qstrcmp(uri, "precitec.gui.components.postprocessing") == 0);
    qmlRegisterType<precitec::gui::components::postprocessing::PostProcessing>(uri, 1, 0, "PostProcessing");
    
 //   qmlRegisterType<precitec::gui::components::postprocessing::StorageUpdateProxy>(uri, 1, 0, "StorageUpdateProxy");
    qRegisterMetaType<precitec::gui::DbProxy>("DbProxy");
    qRegisterMetaType<precitec::gui::TriggerCmdProxy>("TriggerCmdProxy");
    qRegisterMetaType<precitec::gui::ResultsProxy>("ResultsProxy");
    qRegisterMetaType<precitec::gui::RecorderProxy>("RecorderProxy");
    qRegisterMetaType<precitec::gui::SystemStatusProxy>("SystemStatusProxy");
    qRegisterMetaType<precitec::gui::CalibrationProxy>("CalibrationProxy");
    qRegisterMetaType<precitec::gui::WeldHeadMsgProxy>("WeldHeadMsgProxy");
    qRegisterMetaType<precitec::VideoRecorderProxy>("VideoRecorderProxy");
    qRegisterMetaType<precitec::gui::InspectionOutProxy>("InspectionOutProxy");
    


    /*
    qmlRegisterSingletonType<precitec::gui::components::postprocessing::PostProcessingModule>(uri, 1, 0, "PostProcessingModule",
        [](QQmlEngine * engine, QJSEngine * scriptEngine) -> QObject *
        {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            auto postProcessingModule = precitec::gui::components::postprocessing::PostProcessingModule::instance();
            QQmlEngine::setObjectOwnership(postProcessingModule, QQmlEngine::CppOwnership);
            return postProcessingModule;
        });*/


    
}

#include "main.moc"

