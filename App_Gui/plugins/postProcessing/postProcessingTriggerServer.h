#pragma once
#include <QObject>
#include "postProcessingController.h"
#include "postProcessingSequenceDataReady.h"
#include "system/timer.h"
#include "../../../Mod_GrabberNoHw/include/trigger/sequenceProvider.h"
#include "event/results.proxy.h"
#include "event/recorder.proxy.h"
#include "event/systemStatus.proxy.h"
#include "event/inspectionOut.proxy.h"
#include "event/videoRecorder.proxy.h"
#include "event/triggerCmd.proxy.h"
#include "message/calibration.proxy.h"
#include "message/db.proxy.h"
#include "message/weldHead.proxy.h"
#include "../../../Mod_Storage/src/parameter.h"
#include "../../../Mod_Storage/src/parameterSet.h"


namespace precitec
{
namespace gui
{
namespace components
{
namespace postprocessing 
{

enum State
{
        UserEnterData,
        CallTrigger
};   
    
#define MAXIMAGE 200

class PostProcessingTriggerServer
{
   
public:
    PostProcessingTriggerServer(
        TDb<MsgProxy>& dbProxy,
        TTriggerCmd<EventProxy>& triggerCmdProxy,
        TResults<EventProxy>& resultsProxy,
        TRecorder<EventProxy>& recorderProxy,
        TSystemStatus<EventProxy>& systemStatusProxy,
        TCalibration<MsgProxy>& calibrationProxy,
        TWeldHeadMsg<MsgProxy>& weldHeadMsgProxy,
        TVideoRecorder<EventProxy>& videoRecorderProxy,
        TInspectionOut<EventProxy>& inspectionOutProxy   
        
    );
    ~PostProcessingTriggerServer();
    
    void triggerSeam(uint32_t p_oProductType, uint32_t instance, uint32_t seamSeries, uint32_t seamNumber);
    void triggerProduct(uint32_t p_oProductType);
    void changeFilterAndTrigger(uint32_t productType, uint32_t seamseriesNumber, uint32_t seamNumber); //, storage::ParameterSet* storageParamSet); //,   storage::Parameter* storageParam);
    void initDataReady(TDb<MsgProxy>& dbProxy);
        
    // Initialize the controller with proxies as parameters
    void initController(TDb<MsgProxy>& dbProxy,
        TTriggerCmd<EventProxy>& triggerCmdProxy,
        TResults<EventProxy>& resultsProxy,
        TRecorder<EventProxy>& recorderProxy,
        TSystemStatus<EventProxy>& systemStatusProxy,
        TCalibration<MsgProxy>& calibrationProxy,
        TWeldHeadMsg<MsgProxy>& weldHeadMsgProxy,
        TVideoRecorder<EventProxy>& videoRecorderProxy,
        TInspectionOut<EventProxy>& inspectionOutProxy);

public:
  //  Q_INVOKABLE void testHandler();
  
    
    
    
private:
    
    Poco::UUID findProductUUIDfromType(uint32_t o_ProductType);
    std::optional<Poco::UUID> instanceNumberToUUIDConversion(Poco::UUID productID,  uint32_t instance);
    void prepVdrFileAndSend(const VdrFileInfo& vdrInfo);
    void loadImages();
   
    double                      m_frameDuration = 100; // 100us each frame
    Timer                       m_FrameTimer;
    std::deque<VdrFileInfo>     m_imageAndSampleVector;
    
    precitec::system::Timer                     m_contextTimer;
    SequenceProvider m_sequenceProvider;
    int                m_previousSeam = -1;
    
    
    TDb<MsgProxy>&					m_dbProxy;		
	TTriggerCmd<EventProxy>&		m_triggerCmdProxy;		
	TResults<EventProxy>&			m_resultsProxy;		
	TRecorder<EventProxy>&			m_recorderProxy;		
	TSystemStatus<EventProxy>&		m_systemStatusProxy;		
	TCalibration<MsgProxy>&			m_calibrationProxy;    
    TWeldHeadMsg<MsgProxy>&			m_weldHeadMsgProxy;
    TVideoRecorder<EventProxy>&      m_videoRecorderProxy;
    TInspectionOut<EventProxy>&		m_inspectionOutProxy;
    analyzer::CentralDeviceManager m_centralDeviceManager;
    
    PostProcessingSequenceDataReady       m_ptrDataReady;
    PostProcessingController     m_processsingController;    
    ImageDataHolder              m_imageDataArray[MAXIMAGE];
    
    uint32_t m_idx = 0;

};
    

}
    
}
}
}
