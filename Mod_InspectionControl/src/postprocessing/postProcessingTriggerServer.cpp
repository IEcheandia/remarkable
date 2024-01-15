#include "../include/viInspectionControl/postProcessingTriggerServer.h"
#include <optional>
#include <common/bitmap.h>
#include <iostream>
#include <event/imageShMem.h>


namespace precitec
{

namespace viInspectionControl{   
    
    
PostProcessingTriggerServer::PostProcessingTriggerServer(PostProcessingController* controller, TDb<MsgProxy>& dbProxy):
m_ptrDataReady(dbProxy),
m_dbProxy(dbProxy),
m_processsingController(controller)
/*m_dbProxy(),
m_triggerCmdProxy(),
m_resultsProxy(),
m_recorderProxy(),
m_systemStatusProxy(),
m_calibrationProxy(),
m_weldHeadMsgProxy(),
m_videoRecorderProxy(),
m_inspectionOutProxy(),
m_centralDeviceManager(),
m_processsingController(
    &m_dbProxy,
    &m_weldHeadMsgProxy,
    &m_triggerCmdProxy,
    &m_resultsProxy,
    &m_recorderProxy,
    &m_systemStatusProxy,
    &m_videoRecorderProxy,
    &m_inspectionOutProxy,
    &m_centralDeviceManager,
    true)*/
{
  //  m_imageAndSampleVector = m_ptrDataReady->getImageAndSampleVector();
    
}
    

void PostProcessingTriggerServer::testHandler()
{
    int state = UserEnterData;
    uint32_t productNum = 1;
    uint32_t seamInput;
    
    std::cout << "Calling to TestHandler" << std::endl;
    
    
    m_ptrDataReady.init();
    
    while(true)
    {
        switch (state)
        {
            case UserEnterData:
            {   
                std::cout << "Enter a product number: " << std::endl;
                std::cin >> productNum;
                std::cout << "Enter a Seam number: " << std::endl;
                std::cin >> seamInput;
                state = CallTrigger;
                break;
            };
            case CallTrigger:
            {
             //   std::cout << "CallTRigger state" << std::endl;
                triggerSeam(productNum, 0, 0, seamInput);
           //     std::cout << "Seam processing..." << std::endl;
                sleep(10); //10secs
                state = UserEnterData;
            };
        }
    }  
    
    
    
}



void PostProcessingTriggerServer::triggerProduct(uint32_t p_oProductType)
{
}


void PostProcessingTriggerServer::triggerSeam(uint32_t p_oProductType, uint32_t instance, uint32_t seamSeries, uint32_t seamNumber)
{
    std::cout << "Calling to triggerSeam in PostProcessingTriggerServer block..." << std::endl;
 //   sleep(3);
    
    auto productID = findProductUUIDfromType(p_oProductType);
    auto instanceUUID = instanceNumberToUUIDConversion(productID, instance);
    
    if(m_previousSeam != seamNumber)
    {
      //  std::cout << "New seam" << std::endl;
      //  sleep(3);
        //I need to restart the inspection
        m_processsingController->stopInspect();
    //    sleep(3);
        for(auto& product : *m_ptrDataReady.getProductList())
        {
            m_processsingController->changeProduct(product);
        }
        m_processsingController->activate(productID, p_oProductType, "no info");
    //    sleep(3);
        m_processsingController->activateSeamSeries(seamSeries);
   //     sleep(3);
        auto label = std::to_string(seamNumber);
        m_processsingController->activateSeam(seamSeries, label);
        sleep(3);
        m_processsingController->startInspect(seamSeries, seamNumber, label);       
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

void PostProcessingTriggerServer::loadImages()
{
    std::cout << "load images" << std::endl;
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
    std::cout << "Starting preparation of vdr file..." << std::endl;
    auto ptrTriggerContext = new TriggerContext{vdrInfo.productInstance(), static_cast<int>(vdrInfo.product()), static_cast<int>(vdrInfo.seamSeries()), static_cast<int>(vdrInfo.seam()), static_cast<int>(vdrInfo.image())};
          
    ptrTriggerContext->setProductInstance(vdrInfo.productInstance());
    ptrTriggerContext->sync(m_contextTimer.us());
    
    image::TLineImage<byte> byteImage;
    fileio::SampleDataHolder sampleDataHolder;
    
    const bool hasImage = false; //m_sequenceProvider.getImage(*ptrTriggerContext, byteImage);
    const bool hasSample = false; //m_sequenceProvider.getSamples(*ptrTriggerContext, sampleDataHolder);
     
    if(hasImage)
    {
        std::cout << "Sending image to controller..." << std::endl;
        m_processsingController->data(CamGreyImage, ptrTriggerContext, byteImage);
    }
    else if(hasSample)
    {
        std::cout << "Sending sample to controller..." << std::endl;
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
                m_processsingController->data(sensorID, ptrTriggerContext, sample);
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
        std::cout << "no image no sample" << std::endl;
        ImageDataHolder imageDataHolder = m_imageDataArray[m_idx];
        image::BImage byteImage = imageDataHolder.getByteImage();
        if(imageDataHolder.getIsExtraDataValid())
        {
            ptrTriggerContext->HW_ROI_x0 = imageDataHolder.getHardwareRoiOffsetX();
            ptrTriggerContext->HW_ROI_y0 = imageDataHolder.getHardwareRoiOffsetY();
            ptrTriggerContext->HW_ROI_dx0 = byteImage.width();
            ptrTriggerContext->HW_ROI_dy0 = byteImage.height();
        }

        m_processsingController->data(CamGreyImage, ptrTriggerContext, byteImage);
        
    }
    
    
}



Poco::UUID PostProcessingTriggerServer::findProductUUIDfromType(uint32_t o_ProductType)
{
  //  std::cout << "findproductfromUUID" << std::endl;
    sleep(3);
    auto productList = m_ptrDataReady.getProductList();
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

