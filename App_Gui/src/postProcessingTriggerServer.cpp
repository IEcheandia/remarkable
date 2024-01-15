#include "postProcessingTriggerServer.h"
#include <optional>
#include <common/bitmap.h>
#include <iostream>
#include <event/imageShMem.h>

#include "common/systemConfiguration.h"
#include "event/inspectionToS6k.proxy.h"

namespace precitec
{
namespace gui
{

    


    
PostProcessingTriggerServer::PostProcessingTriggerServer(
    TDb<MsgProxy>& dbProxy,
    TTriggerCmd<EventProxy>& triggerCmdProxy,
    TResults<EventProxy>& resultsProxy,
    TRecorder<EventProxy>& recorderProxy,
    TSystemStatus<EventProxy>& systemStatusProxy,
    TCalibration<MsgProxy>& calibrationProxy,
    TWeldHeadMsg<MsgProxy>& weldHeadMsgProxy,
    TVideoRecorder<EventProxy>& videoRecorderProxy,
    TInspectionOut<EventProxy>& inspectionOutProxy 
):
m_dbProxy(dbProxy), 
m_triggerCmdProxy(triggerCmdProxy),
m_resultsProxy(resultsProxy),
m_recorderProxy(recorderProxy),
m_systemStatusProxy(systemStatusProxy),
m_calibrationProxy(calibrationProxy),
m_weldHeadMsgProxy(weldHeadMsgProxy),
m_videoRecorderProxy(videoRecorderProxy),
m_inspectionOutProxy(inspectionOutProxy),
m_centralDeviceManager(),
m_ptrDataReady(PostProcessingSequenceDataReady( m_dbProxy)),
m_processsingController(&m_dbProxy,
                         &m_weldHeadMsgProxy,
                         &m_triggerCmdProxy,
                         &m_resultsProxy,
                         &m_recorderProxy,
                         &m_systemStatusProxy,
                         &m_videoRecorderProxy,
                         &m_inspectionOutProxy,
                         &m_centralDeviceManager,
                         true)
{
    std::cout << "PostProcessingTriggerServer CTOR; m_dbProxy ptr: " << m_dbProxy << std::endl;
   
   // sleep(5);
   
}

void PostProcessingTriggerServer::initDataReady(TDb<MsgProxy>& dbProxy)
{   
    m_ptrDataReady.setDBProxy(dbProxy); 
   
}

void PostProcessingTriggerServer::initController(TDb<MsgProxy>& dbProxy,
    TTriggerCmd<EventProxy>& triggerCmdProxy,
    TResults<EventProxy>& resultsProxy,
    TRecorder<EventProxy>& recorderProxy,
    TSystemStatus<EventProxy>& systemStatusProxy,
    TCalibration<MsgProxy>& calibrationProxy,
    TWeldHeadMsg<MsgProxy>& weldHeadMsgProxy,
    TVideoRecorder<EventProxy>& videoRecorderProxy,
    TInspectionOut<EventProxy>& inspectionOutProxy)
{
    std::cout << "TRIGGER_SERVER::initController(); init controller" << std::endl;
    m_processsingController.setDbProxy(&dbProxy);
    m_processsingController.setTriggerCmdProxy(&triggerCmdProxy);
    m_processsingController.setResultProxy(&resultsProxy);
    m_processsingController.setRecorderProxy(&recorderProxy);
    m_processsingController.setSystemStatusProxy(&systemStatusProxy);    
    m_processsingController.setWeldHeadMsgProxy(&weldHeadMsgProxy);
    m_processsingController.setVideoRecorderProxy(&videoRecorderProxy);
    m_processsingController.setInspectionOutProxy(&inspectionOutProxy);

    std::cout << "TRIGGER_SERVER::initController(); ; controller initialized" << std::endl;
    sleep(3);
}
    
/*
void PostProcessingTriggerServer::testHandler()
{
  //  int state = UserEnterData;
 //   uint32_t seamInput;
    
    std::cout << "calling testHandler" << std::endl;
   
    while(true)
    {
        switch (state)
        {
            case UserEnterData:
            {                
                std::cout << "Enter a Seam number: " << std::endl;
                std::cin >> seamInput;
                state = CallTrigger;
                break;
            };
            case CallTrigger:
            {
                triggerSeam(6, 0, 0, seamInput);
                std::cout << "Seam processing..." << std::endl;
                sleep(10); //10secs
                state = UserEnterData;
            };
        }
    }  
    
    
    
}
*/


void PostProcessingTriggerServer::triggerProduct(uint32_t p_oProductType)
{
    std::cout << "TRIGGER_SERVER::triggerProduct();  trigger product" << std::endl;
    m_ptrDataReady.init();
    sleep(3);
}


void PostProcessingTriggerServer::triggerSeam(uint32_t p_oProductType, uint32_t instance, uint32_t seamSeries, uint32_t seamNumber)
{
    std::cout << "TRIGGER_SERVER::triggerSeam(); Calling to triggerSeam in PostProcessingTriggerServer block..." << std::endl;
 //   sleep(3);
    
    auto productID = findProductUUIDfromType(p_oProductType);
    auto instanceUUID = instanceNumberToUUIDConversion(productID, instance);
    
    if(static_cast<uint32_t>(m_previousSeam) != seamNumber)
    {
      //  std::cout << "New seam" << std::endl;
      //  sleep(3);
        //I need to restart the inspection
        m_processsingController.stopInspect();
    //    sleep(3);
        for(auto& product : *m_ptrDataReady.getProductList())
        {
            m_processsingController.changeProduct(product);
        }
        m_processsingController.activate(productID, p_oProductType, "no info");
    //    sleep(3);
        m_processsingController.activateSeamSeries(seamSeries);
   //     sleep(3);
        auto label = std::to_string(seamNumber);
        m_processsingController.activateSeam(seamSeries, label);
        sleep(3);
        m_processsingController.startInspect(seamSeries, seamNumber, label);       
    }    
    
     m_imageAndSampleVector = m_ptrDataReady.getImageAndSampleVector();
    
     std::cout << "image and sample video cached: " << m_imageAndSampleVector.size() << " files in total" << std::endl;
 //    std::cout << "Product ID: " << productID << ", instance: " << instance << ", seamseries: " << seamSeries << " seam: " << seamNumber << std::endl;
     
   //  loadImages();
   
    while(!m_imageAndSampleVector.empty())
    {
      //  std::cout << "image and samples vector is not empty - start reading the vdr files" << std::endl;
        const auto& vdrFile = m_imageAndSampleVector.front();
      
        wmLog(eDebug, "vdrfile:productID: %d , instance: %s  seamseries: %d seam: %d \n", std::to_string(vdrFile.product()), vdrFile.productInstance().toString(), vdrFile.seamSeries(), vdrFile.seam());
        
        if(vdrFile.productInstance() == instanceUUID && vdrFile.seamSeries() == seamSeries
            && vdrFile.seam() == seamNumber)
        {
        //    std::cout << "vdrfile found" << std::endl;
            prepVdrFileAndSend(vdrFile);
            m_imageAndSampleVector.pop_front();
            m_idx++;
        }
        
        m_imageAndSampleVector.pop_front();
            
    }  
    m_previousSeam = seamNumber;       
    
}

void PostProcessingTriggerServer::changeFilterAndTrigger(uint32_t productType, uint32_t seamseriesNumber, uint32_t seamNumber) //, storage::ParameterSet* storageParamSet) //,  storage::Parameter* storageParam)
{
    
    std::cout << "TRIGGER_SERVER::changeFilterAndTrigger() " << std::endl;
    sleep(3);
    m_ptrDataReady.init();
    auto productID = findProductUUIDfromType(26);
    std::cout << "TRIGGER_SERVER::changeFilterAndTrigger(); productID: " << productID.toString() << std::endl;
   // sleep(2);
    auto instanceUUID = instanceNumberToUUIDConversion(productID, 0);
    if (instanceUUID) 
    { // Check if the optional contains a value
        std::cout << "TRIGGER_SERVER::changeFilterAndTrigger(); instanceUUID: " << instanceUUID.value().toString() << std::endl;
    } else {
        std::cout << "TRIGGER_SERVER::changeFilterAndTrigger(); instanceUUID is empty" << std::endl;
    }

  //  sleep(2);

    std::cout << "TRIGGER_SERVER::changeFilterAndTrigger(); seamseries: " << seamseriesNumber << " seam: " << seamNumber << std::endl;

    m_processsingController.stopInspect();
 //   std::cout << "TRIGGER_SERVER::changeFilterAndTrigger(); inspect stop" << std::endl;
  //  sleep(2);

    for (auto& product : *m_ptrDataReady.getProductList())
    {
        m_processsingController.changeProduct(product);
    }
    m_processsingController.activate(productID, 26, "no info");
    std::cout << "TRIGGER_SERVER::changeFilterAndTrigger(); activate product" << std::endl;
 //   sleep(3);
    m_processsingController.activateSeamSeries(0);
    std::cout << "TRIGGER_SERVER::changeFilterAndTrigger(); activate seamseries" << std::endl;
 //      sleep(3);
     auto label = std::to_string(seamNumber);
     m_processsingController.activateSeam(0, label);
     std::cout << "TRIGGER_SERVER::changeFilterAndTrigger(); activate seam" << std::endl;
 //    sleep(3);

    m_processsingController.startInspect(seamseriesNumber, seamNumber, label);  

sleep(3);

     m_imageAndSampleVector = m_ptrDataReady.getImageAndSampleVector();
    
   //  std::cout << "TRIGGER_SERVER::changeFilterAndTrigger(); image and sample video cached: " << m_imageAndSampleVector.size() << " files in total" << std::endl;
 //    std::cout << "Product ID: " << productID << ", instance: " << instance << ", seamseries: " << seamSeries << " seam: " << seamNumber << std::endl;
     
   //  loadImages();
   
    while(!m_imageAndSampleVector.empty())
    {
      //  std::cout << "image and samples vector is not empty - start reading the vdr files" << std::endl;
        const auto& vdrFile = m_imageAndSampleVector.front();
      
        wmLog(eDebug, "vdrfile:productID: %d , instance: %s  seamseries: %d seam: %d , ImageNumber: %d \n", std::to_string(vdrFile.product()), vdrFile.productInstance().toString(), vdrFile.seamSeries(), vdrFile.seam(), vdrFile.image());
        
        if(vdrFile.productInstance() == instanceUUID && vdrFile.seamSeries() == seamseriesNumber
            && vdrFile.seam() == seamNumber)
        {
        //    std::cout << "vdrfile found" << std::endl;
            prepVdrFileAndSend(vdrFile);
            sleep(2);
            m_imageAndSampleVector.pop_front();
            m_idx++;
        }
        else{
            m_imageAndSampleVector.pop_front();
        }
        
        
    }
     
    // m_processsingController.updateParamSetWihoutDB(storageParamSet, storageParam);
}
    


void PostProcessingTriggerServer::loadImages()
{
    std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); load images" << std::endl;
    try
    {
        std::string homeDir(getenv("WM_BASE_DIR"));
        homeDir += "/images/";
      //  loadImages(Poco::Path(homeDir));
    }
    catch(...)
    {
        wmLog(eError, "ERROR loadImages: unhandled exception\n");
      //  throw;
    }

    m_idx = 0;
    if (m_idx< 1)
    {
        std::srand(time(0));
        int seed = 150;

        for (int i=m_idx; i<MAXIMAGE; ++i)
        {
            ImageDataHolder imageDataHolder;
            imageDataHolder.setByteImage(genModuloPattern(Size2d(greyImage::ImageWidth,greyImage::ImageHeight), seed + std::abs(i - MAXIMAGE/2)));
            imageDataHolder.setIsExtraDataValid(false);
            m_imageDataArray[i] = imageDataHolder;
        }
           
    }
   
}


void PostProcessingTriggerServer::prepVdrFileAndSend(const VdrFileInfo& vdrInfo)
{
    std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); Starting preparation of vdr file..." << std::endl;
    sleep(1);
  //  std::cout << "vdrImage(): " << vdrInfo.image() << std::endl;
    auto ptrTriggerContext = new TriggerContext{vdrInfo.productInstance(), static_cast<int>(vdrInfo.product()), static_cast<int>(vdrInfo.seamSeries()), static_cast<int>(vdrInfo.seam()), static_cast<int>(vdrInfo.image())};
    ptrTriggerContext->setImageNumber(vdrInfo.image());
 //   std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); TriggerContext created with image number(): " << ptrTriggerContext->imageNumber() << std::endl;
    ptrTriggerContext->setProductInstance(vdrInfo.productInstance());
    ptrTriggerContext->sync(m_contextTimer.us());
    
    image::TLineImage<byte> byteImage;
    fileio::SampleDataHolder sampleDataHolder;
    
    const bool hasImage = m_sequenceProvider.getImage(*ptrTriggerContext, byteImage);
    const bool hasSample = m_sequenceProvider.getSamples(*ptrTriggerContext, sampleDataHolder);
     
    if(hasImage)
    {
        std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); Sending image to controller..." << std::endl;
        m_processsingController.data(CamGreyImage, ptrTriggerContext, byteImage);
    }
    else if(hasSample)
    {
        std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); Sending sample to controller..." << std::endl;
        int nSensors = sampleDataHolder.allData.size();
        for( int sensorIdx=0; sensorIdx < nSensors; sensorIdx++ )
        {
            fileio::SampleDataHolder::OneSensorData oneSensorData = sampleDataHolder.allData.at(sensorIdx);
            int nSamples = oneSensorData.dataVector.size();
            int sensorID = oneSensorData.sensorID;
            TSmartArrayPtr<int>::ShArrayPtr* pIntPointer = new TSmartArrayPtr<int>::ShArrayPtr[1];
            pIntPointer[0] = new int[nSamples];

            for(int sampleIdx=0; sampleIdx<nSamples; sampleIdx++)
            {
                int sampleValue = oneSensorData.dataVector.at(sampleIdx);
                pIntPointer[0][sampleIdx] = sampleValue;
            }

            try
            {
                const precitec::image::Sample sample(pIntPointer[0], nSamples);
                m_processsingController.data(sensorID, ptrTriggerContext, sample);
            }
            catch(const std::exception &exception)
            {
                std::ostringstream oMsg;
                oMsg  << __FUNCTION__ << " " << exception.what() << "\n";
                wmLog(eWarning, oMsg.str());
            }
            catch(...)
            {
                std::ostringstream oMsg;
                oMsg  << __FUNCTION__ << " Unknown exception encountered.\n";
                wmLog(eWarning, oMsg.str());
            }

            pIntPointer[0] = 0; // decrement smart pointer reference counter
            delete[] pIntPointer;
            pIntPointer = nullptr;
        }
    }
    else if(!hasImage && !hasSample)
    {
  //      std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); no image no sample" << std::endl;
        ImageDataHolder imageDataHolder = m_imageDataArray[m_idx];
        image::BImage byteImage = imageDataHolder.getByteImage();
        if(imageDataHolder.getIsExtraDataValid())
        {
            ptrTriggerContext->HW_ROI_x0 = imageDataHolder.getHardwareRoiOffsetX();
            ptrTriggerContext->HW_ROI_y0 = imageDataHolder.getHardwareRoiOffsetY();
            ptrTriggerContext->HW_ROI_dx0 = byteImage.width();
            ptrTriggerContext->HW_ROI_dy0 = byteImage.height();
        }
        std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); Image file ready" << std::endl;
        m_processsingController.data(CamGreyImage, ptrTriggerContext, byteImage);
        
    }
    
    
}



Poco::UUID PostProcessingTriggerServer::findProductUUIDfromType(uint32_t o_ProductType)
{
  //  std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); find product uuid from type" << std::endl;
  //  sleep(2);
    auto productList = m_ptrDataReady.getProductList();
 //   std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); product list size: " << productList->size() << std::endl;
  //  sleep(2);
    for (const auto& product : *productList) 
    {
        if (product.productType() == o_ProductType) 
        {
            return product.productID();
        }
    }
    return Poco::UUID::null();
}


std::optional<Poco::UUID> PostProcessingTriggerServer::instanceNumberToUUIDConversion(Poco::UUID productID,  uint32_t instance)
{
    auto instancesList = m_ptrDataReady.getProductInstancesList(productID);
 //   std::cout << "TRIGGER_SERVER::prepVdrFileAndSend(); instances list size: " << instancesList.size() << std::endl;
    uint32_t current_pos = 0;
    for(const auto& [path, pair] : instancesList)
    {
        if(current_pos == instance)
        {
            return pair.first;
        }
        ++current_pos;
    }
    
    return {};
    
    
}


    
    
    
}
    
}

