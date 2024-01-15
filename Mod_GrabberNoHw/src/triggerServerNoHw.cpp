#include "../include/trigger/triggerServerNoHw.h"
#include <Poco/DirectoryIterator.h>
#include <common/bitmap.h>
#include <common/connectionConfiguration.h>

#include <module/moduleLogger.h>
#include <common/systemConfiguration.h>
#include <event/imageShMem.h>
#include "../Mod_VideoRecorder/include/videoRecorder/types.h"



namespace precitec
{
namespace grabber
{

using fileio::Bitmap; 
using image::BImage; 
using image::Size2d; 
using image::genModuloPattern; 
using image::TLineImage; 


TriggerServerNoHw::TriggerServerNoHw(TSensor<EventProxy>& sensorProxy):
    m_sensorProxy(sensorProxy)
    , m_serverAccess(new Poco::FastMutex())
    , m_idx(0)
    , m_imgLoaded(0)
    , m_isSimulation(false)
{
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) != 0)
        {
            m_isSimulation = true;
        }
    }
    m_hilfsTimer.start();
}

void TriggerServerNoHw::trigger(TriggerContext const& context)
{
    Poco::ScopedLock<Poco::FastMutex> lock(*m_serverAccess);
    trigger(std::vector<int>{}, context);

}

void TriggerServerNoHw::trigger(const std::vector<int>& sensorIds, TriggerContext const& context)
{
    TriggerContext hilfsContext;
    hilfsContext = context;
    hilfsContext.setProductInstance(context.productInstance());
    m_hilfsTimer.elapsed();
    hilfsContext.sync(m_hilfsTimer.us());
    m_hilfsTimer.restart();

    BImage byteImage;
    fileio::SampleDataHolder sampleDataHolder;
    const bool hasImage = m_sequenceProvider.getImage(hilfsContext, byteImage);
    const bool hasSample = m_sequenceProvider.getSamples(hilfsContext, sampleDataHolder);

    if (!sensorIds.empty())
    {
        bool allValid = true;
        for (int sensorId : sensorIds)
        {
            if (sensorId <= interface::eImageSensorMax)
            {
                if (!hasImage)
                {
                    allValid = false;
                    break;
                }
                continue;
            }
            if (std::none_of(sampleDataHolder.allData.begin(), sampleDataHolder.allData.end(), [sensorId] (const auto &sample) { return sample.sensorID == sensorId; }))
            {
                allValid = false;
                break;
            }
        }
        if (!allValid)
        {
            m_sensorProxy.simulationDataMissing(hilfsContext);
            return;
        }
    }

    if (hasImage)
    {
        wmLog(eDebug, "Has image");
        m_sensorProxy.data(CamGreyImage, hilfsContext, byteImage);
    }
    if (!hasImage && !hasSample)
    {
      //  wmLog(eDebug, "no image no sample");
        ImageDataHolder imageDataHolder = m_imageDataArray[m_idx];
        image::BImage byteImage = imageDataHolder.getByteImage();
        if(imageDataHolder.getIsExtraDataValid())
        {
            hilfsContext.HW_ROI_x0 = imageDataHolder.getHardwareRoiOffsetX();
            hilfsContext.HW_ROI_y0 = imageDataHolder.getHardwareRoiOffsetY();
            hilfsContext.HW_ROI_dx0 = byteImage.width();
            hilfsContext.HW_ROI_dy0 = byteImage.height();
        }

        m_sensorProxy.data(CamGreyImage, hilfsContext, byteImage);
        m_idx = ((m_idx + 1) % m_imgLoaded);
    }

    if (hasSample)
    {
        wmLog(eDebug, "has sample!");
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
                m_sensorProxy.data(sensorID, hilfsContext, sample);
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
}


void TriggerServerNoHw::trigger(TriggerContext const& context, TriggerInterval const& interval)
{
}

void TriggerServerNoHw::triggerMode(int mode)
{
}

void TriggerServerNoHw::triggerStop(int flag)
{
}

void TriggerServerNoHw::loadImages()
{
    try
    {
        std::string homeDir(getenv("WM_BASE_DIR"));
        homeDir += "/images/";
        loadImages(Poco::Path(homeDir));
    }
    catch(...)
    {
        wmLog(eError, "ERROR loadImages: unhandled exception\n");
        throw;
    }

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
            m_imgLoaded++;
    }
    }
    m_idx = 0;
}


void TriggerServerNoHw::loadImages(Poco::Path const& path)
{

    m_noGrabberSharedMemory.init(false, true);
    m_sequenceProvider.init(&m_noGrabberSharedMemory);

    if(!path.isDirectory())
    {
        wmLog( eWarning, "WARNING: TriggerServer -- loadImages: Invalid path %s\n", path.toString().c_str() );
        return;
    }

    std::vector<std::string> oFileList;
    for (Poco::DirectoryIterator it = Poco::DirectoryIterator(path); it != Poco::DirectoryIterator(); ++it)
    {
        Poco::Path p(it->path());
        if (p.isFile() == true && p.getExtension() == "bmp")
        {
            wmLog(eDebug, "TriggerServer -- loadImages: Loading from path %s...\n", it->path().c_str() );
            oFileList.push_back(it->path());
        }
    }
    std::sort(oFileList.begin(), oFileList.end());

    if (oFileList.size() > MAXIMAGE)
    {
        wmLog(eWarning,"loadImages: folder %s has %d files, but %d will be loaded \n", path.toString().c_str(), oFileList.size() , MAXIMAGE );
    }
    m_idx = std::min((size_t)MAXIMAGE, oFileList.size());
    m_imgLoaded = m_idx;

    for (int i=0; i < m_idx; ++i)
    {
        Bitmap bmp{oFileList[i]};
        auto sharedMem = m_noGrabberSharedMemory.nextImagePointer(bmp.fileSize());
        image::Size2d size {bmp.width(), bmp.height()};
        TLineImage<byte> image{sharedMem, size};
        std::vector<unsigned char> oAdditionalData;
        if(!bmp.load(image.begin(), oAdditionalData))
        {
            std::ostringstream oMsg;
            oMsg  << __FUNCTION__ << " could not load image file " << oFileList[i] << "\n";
            wmLog(eDebug, oMsg.str());
            ImageDataHolder imageDataHolder;
            image = genModuloPattern(Size2d(greyImage::ImageWidth, greyImage::ImageHeight), i);
            imageDataHolder.setPath(oFileList[i]);
            imageDataHolder.setByteImage(image);
            imageDataHolder.setIsExtraDataValid(false);
            m_imageDataArray[i] = imageDataHolder;
        }
        else
        {
        ImageDataHolder imageDataHolder;
        imageDataHolder.setPath(oFileList[i]);
        imageDataHolder.setByteImage(image);
        if(oAdditionalData.size()>=vdr::add_data_indices::eNbBytes)
        {
            unsigned short sHwRoiX, sHwRoiY, sImgNb;
            sHwRoiX = *(reinterpret_cast<unsigned short*>(&oAdditionalData[vdr::add_data_indices::eHwRoiX]));
            imageDataHolder.setHardwareRoiOffsetX(sHwRoiX);
            sHwRoiY = *(reinterpret_cast<unsigned short*>(&oAdditionalData[vdr::add_data_indices::eHwRoiY]));
            imageDataHolder.setHardwareRoiOffsetY(sHwRoiY);
            sImgNb = *(reinterpret_cast<unsigned short*>(&oAdditionalData[vdr::add_data_indices::eImgNb]));
            imageDataHolder.setImageNumber(sImgNb);
            imageDataHolder.setIsExtraDataValid(true);
            std::cout << imageDataHolder.getPath() << " N=" << sImgNb << " HwRoiX=" << sHwRoiX << " HwRoiY=" << sHwRoiY << std::endl;
        }
        else
        {
            imageDataHolder.setIsExtraDataValid(false);
        }
        m_imageDataArray[i] = imageDataHolder;
        }
    }
}

ImageFromDiskParametersNoHw TriggerServerNoHw::getImageFromDiskHWROI(const interface::TriggerContext& rContext)
{
    auto type = SystemConfiguration::instance().getInt("CameraInterfaceType", 0) ;
    
    auto str_type = std::to_string(type); 
    wmLog(eDebug, str_type); 
    auto maxCameraSize = [&]() ->geo2d::Size
    {
        switch(SystemConfiguration::instance().getInt("CameraInterfaceType", 0) )
        {
            default:
            case 0: return {1024,1024}; //framegrabber
            case 1: return {MAX_CAMERA_WIDTH, MAX_CAMERA_HEIGHT}; //GigE
        }
    }();
    
    interface::TriggerContext hilfsContext;
    hilfsContext = rContext;
    BImage byteImage;
    const bool hasImage = m_sequenceProvider.getImage(hilfsContext, byteImage);
    if (!hasImage)
    {
        const ImageDataHolder & rImageDataHolder = m_imageDataArray[0];
        auto imageSize = rImageDataHolder.getByteImage().size();
        if (!rImageDataHolder.getIsExtraDataValid())
        {
            return {0,0, imageSize.width, imageSize.height, maxCameraSize.width, maxCameraSize.height};
        }
        return 
        {
            rImageDataHolder.getHardwareRoiOffsetX(),
            rImageDataHolder.getHardwareRoiOffsetY(), 
            imageSize.width, 
            imageSize.height,
            maxCameraSize.width,
            maxCameraSize.height
        };
    }
    return 
    {
        hilfsContext.HW_ROI_x0,
        hilfsContext.HW_ROI_y0,
        hilfsContext.HW_ROI_dx0,
        hilfsContext.HW_ROI_dy0,
        maxCameraSize.width,
        maxCameraSize.height
    };
}


void TriggerServerNoHw::uninit()
{
    m_sequenceProvider.close();
}

bool TriggerServerNoHw::isSimulationStation() const
{
    return m_isSimulation;
}

void TriggerServerNoHw::setSimulation(bool onoff)
{
}

void TriggerServerNoHw::resetImageFromDiskNumber()
{
    wmLog(eWarning,  "resetImageFromDiskNumber called, rescan wm_inst/images and start from first image \n");
    m_idx = 0;
    loadImages();
}

void TriggerServerNoHw::setTestImagesPath(std::string const& testImagesPath)
{
    m_sequenceProvider.setBasepath(testImagesPath);
    loadImages();
}

void TriggerServerNoHw::setTestProductInstance(const Poco::UUID& productInstance)
{
    m_sequenceProvider.setTestProductInstance(productInstance);
}

void TriggerServerNoHw::prepareActivity(uint32_t productNumber)
{
    m_sequenceProvider.reload(productNumber);
}

void TriggerServerNoHw::setTestImagesProductInstanceMode(bool set)
{
    m_sequenceProvider.setTestImagesProductInstanceMode(set);
}




}
}
