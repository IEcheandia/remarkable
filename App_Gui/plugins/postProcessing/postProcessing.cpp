
#include "postProcessing.h"
#include "../../Mod_Storage/src/parameter.h"

#include <QUuid>

#include <Poco/UUID.h>
#include <qvariant.h>
#include <../Mod_Storage/src/seamSeries.h>
#include <../Mod_Storage/src/product.h>
 

namespace precitec
{
namespace gui
{
namespace components
{
namespace postprocessing 
{
    
    
PostProcessing::PostProcessing(QObject *parent )
: QObject(parent)
, m_currentSeam(nullptr)
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
    std::cout << "PostProcessing module CTOR; m_dbProxy: " << m_dbProxy << std::endl;
    //sleep(5);
   /* m_triggerServer = PostProcessingTriggerServer(*m_dbProxy,
        *m_triggerCmdProxy,
        *m_resultsProxy,
        *m_recorderProxy,
        *m_systemStatusProxy,
        *m_calibrationProxy,
        *m_weldHeadMsgProxy,
        *m_videoRecorderProxy,
        *m_inspectionOutProxy);*/
}
    
  

void PostProcessing::init()
{
    std::cout << "PostProcessing module init" << std::endl;
    std::cout << "*dbProxy: " << *m_dbProxy << std::endl;
    m_triggerServer.initDataReady(*m_dbProxy);
    m_triggerServer.initController(*m_dbProxy,
        *m_triggerCmdProxy,
        *m_resultsProxy,
        *m_recorderProxy,
        *m_systemStatusProxy,
        *m_calibrationProxy,
        *m_weldHeadMsgProxy,
        *m_videoRecorderProxy,
        *m_inspectionOutProxy);
        std::cout << "-------------------------" << std::endl;
}
void PostProcessing::setCurrentSeam(precitec::storage::Seam *seam)
{
    std::cout << "PostProcessing module set seam!" << std::endl;
    sleep(2);
        if (m_currentSeam != seam) {
            m_currentSeam = seam;
            if (m_currentSeam)
            {
                    m_destroyConnection = connect(m_currentSeam, &QObject::destroyed, this, std::bind(&PostProcessing::setCurrentSeam, this, nullptr));
            }
            emit currentSeamChanged();
        }
}
    
Poco::UUID PostProcessing::convertQUuidToPoco(const QUuid& quuid)
{
    // Convert QUuid to string
    QString uuidString = quuid.toString(QUuid::WithoutBraces);

    // Convert QString to std::string
    std::string stdUuidString = uuidString.toStdString();

    // Use the string to construct Poco::UUID
    return Poco::UUID(stdUuidString);
}

ParameterList PostProcessing::convertFromDBParamSetToInterfaceParameterList(const std::vector<storage::Parameter*> &parameters)
{
    ParameterList ret;
    ret.reserve(parameters.size());
    for (const auto parameter : parameters)
    {
        auto fp = parameter->toFilterParameter();
        if (fp)
        {
            ret.emplace_back(std::move(fp));
        }
    }
    return ret;
}

void PostProcessing::changeFilterParamAndTrigger(int productType, int seamseriesNumber, precitec::storage::Seam* seam, const QUuid &uuid, const QVariant &value) 
{
  //  auto interfaceFilterParam = storageParam->toFilterParameter();
    
    std::cout << "PostProcessing module change filter! " << std::endl;

    auto paramSetID = seam->graphParamSet();
    auto product = seam->seamSeries()->product();
    auto paramSet = product->filterParameterSet(paramSetID);
    sleep(2);
    
    
    auto it = std::find_if(paramSet->parameters().begin(), paramSet->parameters().end(), [uuid] (auto param) 
    { 
     //   std::cout << "param ID " << param->uuid().toString().toStdString()  << std::endl;
        return param->uuid() == uuid; 
        
    });
 //   std::cout << "paramSetID: " << paramSetID.toString().toStdString() << std::endl;
 //   std::cout << "parmSet captured: " << paramSet->uuid().toString().toStdString()  << std::endl;
 //   std::cout << "quuid: " << uuid.toString().toStdString() << std::endl;
    
    
    
    //sleep(6);
    
    
    if (it != paramSet->parameters().end())
    {
        std::cout << "filter param found" << std::endl;
    //     sleep(3);
        (*it)->setValue(value);
    //    std::cout << "set done" << std::endl;
    //    sleep(3);
        try {         
         
            std::cout << "seam: " << seam->number() << std::endl;
            m_triggerServer.changeFilterAndTrigger(productType, seamseriesNumber,  seam->number());// , paramSet); //, (*it));
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
}
}
