#include "postProcessingModule.h"
//#include "../../src/productModel.h"

#include <QtConcurrentRun>

namespace precitec
{
namespace gui
{



PostProcessingModule::PostProcessingModule()
    : AbstractModule("SIMULATION")
    //, m_simulationCmdProxy(std::make_shared<TSimulationCmd<MsgProxy>>())
    //, m_storageUpdateProxy(std::make_shared<TStorageUpdate<EventProxy>>())   
    , m_dbProxy(std::make_shared<TDb<MsgProxy>>())
    , m_triggerCmdProxy(std::make_shared<TTriggerCmd<EventProxy>>())
    , m_resultsProxy(std::make_shared<TResults<EventProxy>>())
    , m_recorderProxy(std::make_shared<TRecorder<EventProxy>>())
    , m_systemStatusProxy(std::make_shared<TSystemStatus<EventProxy>>())
    , m_calibrationProxy(std::make_shared<TCalibration<MsgProxy>>())
    , m_inspectionOutProxy(std::make_shared<TInspectionOut<EventProxy>>())
    , m_weldHeadMsgProxy(std::make_shared<TWeldHeadMsg<MsgProxy>>())
    , m_videoRecorderProxy(std::make_shared<TVideoRecorder<EventProxy>>())
    
{
    std::cout << "PostProcessingModule CTOR" << std::endl;
  //  sleep(5);
  //  productModel()->setCleanupEnabled(false);
}

PostProcessingModule* PostProcessingModule::instance()
{
    std::cout << "PostProcessingModule instance" << std::endl;
    static PostProcessingModule postProcessingModule;
   
    return &postProcessingModule;
}

void PostProcessingModule::init()
{
    AbstractModule::init();
}

void PostProcessingModule::initialize()
{
    QtConcurrent::run(this, &PostProcessingModule::init);
}

void PostProcessingModule::registerPublications()
{
    AbstractModule::registerPublications();

    registerPublication(m_simulationCmdProxy.get());
    registerPublication(m_storageUpdateProxy.get());
    registerPublication(m_dbProxy.get());
    registerPublication(m_inspectionOutProxy.get());
    registerPublication(m_recorderProxy.get());
    registerPublication(m_resultsProxy.get());
    registerPublication(m_triggerCmdProxy.get());
    registerPublication(m_systemStatusProxy.get());
    registerPublication(m_weldHeadMsgProxy.get());
    registerPublication(m_videoRecorderProxy.get());
}

}
}


