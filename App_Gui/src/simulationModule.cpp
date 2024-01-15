#include "simulationModule.h"
#include "productModel.h"

#include <QtConcurrentRun>

namespace precitec
{
namespace gui
{

SimulationModule::SimulationModule()
    : AbstractModule("SIMULATION")
    , m_simulationCmdProxy(std::make_shared<TSimulationCmd<MsgProxy>>())
    , m_storageUpdateProxy(std::make_shared<TStorageUpdate<EventProxy>>())
    , m_dbProxy(std::make_shared<TDb<MsgProxy>>())
    , m_triggerCmdProxy(std::make_shared<TTriggerCmd<EventProxy>>())
    , m_resultsProxy(std::make_shared<TResults<EventProxy>>())
    , m_recorderProxy(std::make_shared<TRecorder<EventProxy>>())
    , m_systemStatusProxy(std::make_shared<TSystemStatus<EventProxy>>())
    , m_calibrationProxy(std::make_shared<TCalibration<MsgProxy>>())
    , m_inspectionOutProxy(std::make_shared<TInspectionOut<EventProxy>>())
    , m_weldHeadMsgProxy(std::make_shared<TWeldHeadMsg<MsgProxy>>())
    , m_videoRecorderProxy(std::make_shared<TVideoRecorder<EventProxy>>())
    , m_triggerServer(
        *m_dbProxy,
        *m_triggerCmdProxy,
        *m_resultsProxy,
        *m_recorderProxy,
        *m_systemStatusProxy,
        *m_calibrationProxy,
        *m_weldHeadMsgProxy,
        *m_videoRecorderProxy,
        *m_inspectionOutProxy)
{
    std::cout << "SimulationModule CTOR" << std::endl;
    std::cout << "m_simulationCmdProxy: " << m_simulationCmdProxy << std::endl;
    std::cout << "m_storageUpdateProxy: " << m_storageUpdateProxy << std::endl;
    std::cout << "m_dbProxy: " << m_dbProxy << std::endl;
    std::cout << "-------------------------" << std::endl;
    productModel()->setCleanupEnabled(false);
}

SimulationModule* SimulationModule::instance()
{
    static SimulationModule simulationModule;
    return &simulationModule;
}

void SimulationModule::init()
{
    AbstractModule::init();
}

void SimulationModule::initialize()
{
    QtConcurrent::run(this, &SimulationModule::init);
}

void SimulationModule::registerPublications()
{
    AbstractModule::registerPublications();

    std::cout << "SimulationModule::registerPublications()" << std::endl;
    registerPublication(m_simulationCmdProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_simulationCmdProxy:" << static_cast<void*>(m_simulationCmdProxy.get()) <<  std::endl;
    registerPublication(m_storageUpdateProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_storageUpdateProxy: " << static_cast<void*>(m_storageUpdateProxy.get()) <<  std::endl;
    registerPublication(m_dbProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_dbProxy:" << static_cast<void*>(m_dbProxy.get()) <<  std::endl;
    registerPublication(m_inspectionOutProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_inspectionOutProxy:" << static_cast<void*>(m_inspectionOutProxy.get()) <<  std::endl;
    registerPublication(m_recorderProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_recorderProxy:" << static_cast<void*>(m_recorderProxy.get()) <<  std::endl;
    registerPublication(m_resultsProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_resultsProxy:" << static_cast<void*>(m_resultsProxy.get()) <<  std::endl;
    registerPublication(m_triggerCmdProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_triggerCmdProxy:" << static_cast<void*>(m_triggerCmdProxy.get()) <<  std::endl;
    registerPublication(m_systemStatusProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_systemStatusProxy:" << static_cast<void*>(m_systemStatusProxy.get()) <<  std::endl;
    registerPublication(m_weldHeadMsgProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_weldHeadMsgProxy:" << static_cast<void*>(m_weldHeadMsgProxy.get()) <<  std::endl;
    registerPublication(m_videoRecorderProxy.get());
    std::cout << "SimulationModule::registerPublications(), m_videoRecorderProxy:" << static_cast<void*>(m_videoRecorderProxy.get()) <<  std::endl;
}

void SimulationModule::setCurrentSeam(precitec::storage::Seam *seam)
{
    std::cout << "SIMULATION_MODULE::SetCurrentSeam(): " << seam->number() << std::endl;
    sleep(2);
        if (m_currentSeam != seam) {
            m_currentSeam = seam;
            if (m_currentSeam)
            {
                    m_destroyConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&SimulationModule::setCurrentSeam, this, nullptr));
            }
            emit currentSeamChanged();
        }
}

void SimulationModule::setSeam(precitec::storage::Seam* seam)
{
    std::cout << "SIMULATION_MODULE::SetSeam(): " << seam->number() << std::endl;
    sleep(2);
    m_currentSeam = seam;
        
}


void SimulationModule::triggerProduct()
{
    std::cout << "SimulationModule::triggerProduct()" << std::endl;
    m_triggerServer.triggerProduct(26);
}

void SimulationModule::changeFilterParamAndTrigger(int productType, int seamseriesNumber, const QUuid &uuid, const QVariant &value) 
{  
    std::cout << "-------------------------------------------------" << std::endl;
    std::cout << "POSTPROCESSING_MODULE::PostProcessing Triggered " << std::endl;
    std::cout << "-------------------------------------------------" << std::endl;

    auto paramSetID = m_currentSeam->graphParamSet();
    auto product = m_currentSeam->seamSeries()->product();
    auto paramSet = product->filterParameterSet(paramSetID);
    sleep(2);
    
    
    auto it = std::find_if(paramSet->parameters().begin(), paramSet->parameters().end(), [uuid] (auto param) 
    {      
        return param->uuid() == uuid;         
    });

    
    
    if (it != paramSet->parameters().end())
    {
        std::cout << "POSTPROCESSING_MODULE::changeFilterParam(); filter parameter found" << std::endl;
        std::cout << "POSTPROCESSING_MODULE::changeFilterParam(); new parameter value: " << value.toInt() << std::endl;
    
        (*it)->setValue(value);
    
        try {         
         
        //    std::cout << "SIMULATION_MODULE::changeFilterParam(); Seam: " << m_currentSeam->number() << std::endl;
            m_triggerServer.changeFilterAndTrigger(productType, seamseriesNumber,  m_currentSeam->number());// , paramSet); //, (*it));
        } catch (const std::exception& e)
        {
            std::cerr << "Exception caught: " << e.what() << std::endl;
        } catch(...)
        {
            std::cerr << "unknown error" << std::endl;
        }
        sleep(2);
    }
    else
    {
        std::cout << "filter param not found, id: " << std::endl; //(*it)->uuid().toString().toStdString() << " name: " << (*it)->name().toStdString() << std::endl;
        sleep(2);
    }
    
    
    
    
     
     
}


}
}

