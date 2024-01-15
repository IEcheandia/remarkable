/**
*
* @defgroup Framegrabber Framegrabber
*
* \section sec Image Loading
*
*
* @file
* @brief  Load previously recorded images and/or Samples corresponding to product, seamseries and seam
* @copyright    Precitec GmbH & Co. KG
* @author GG
* @date   09.05.17
*
*
*/

#include <iostream>
#include <vector>
#include "Poco/Delegate.h"
#include "Poco/Environment.h"
#include "Poco/Path.h"
#include "Poco/File.h"
#include "Poco/Exception.h"
#include "common/connectionConfiguration.h"
#include "system/types.h"
#include "module/moduleLogger.h"
#include "trigger/sequenceProvider.h"

namespace precitec
{
SequenceProvider::SequenceProvider() :
    m_IsInitialized(false)
{
}

SequenceProvider::~SequenceProvider()
{
}

void SequenceProvider::init(grabber::SharedMemoryImageProvider *memory)
{
    if( m_strBasepath.empty() )
    {
        Poco::Path pathBase;
        const std::string wmBase = "WM_BASE_DIR";
        Poco::Path pathSub("video");
        pathSub.append("testsequences");
        if(Poco::Environment::has(wmBase))
        {
            std::string strWmDir = Poco::Environment::get(wmBase);
            pathBase = Poco::Path(strWmDir);
            pathBase.append(pathSub);
        }
        else
        {
            std::string strFallback = Poco::Environment::get("HOME");
            Poco::Path pathFallback(strFallback);
            pathFallback.append(pathSub);
            pathBase = Poco::Path(pathFallback);
        }

        Poco::File fileCheck(pathBase.toString());
        if( ! fileCheck.exists() )
        {
            try
            {
                fileCheck.createDirectories();
            }
            catch(Poco::Exception const & _ex)
            {
                wmLog( eDebug, " %s Can't create folder %s %s\n", __FUNCTION__, pathBase.toString().c_str(), _ex.name() );
                return;
            }
        }

        m_strBasepath = pathBase.toString();
    }
    m_SequenceLoader.setBasepath(m_strBasepath);
    m_SequenceLoader.close();
    m_SequenceLoader.init(memory);
    m_IsInitialized = true;
}

void SequenceProvider::setBasepath(std::string const & _path)
{
    m_strBasepath = _path;
    m_SequenceLoader.setBasepath(m_strBasepath);
}

void SequenceProvider::setTestProductInstance(const Poco::UUID &productInstance)
{
    m_SequenceLoader.setTestProductInstance(productInstance);
}

void SequenceProvider::setTestImagesProductInstanceMode(bool set)
{
    m_SequenceLoader.setTestImagesProductInstanceMode(set);
}

void SequenceProvider::reload(uint32_t productNumber)
{
    m_SequenceLoader.reload(productNumber);
}

void SequenceProvider::close(void)
{
    m_SequenceLoader.close();
}

bool SequenceProvider::getImage(TriggerContext & _triggerContext, image::BImage & _image)
{
    uint32_t productNumber;
    uint32_t seamseriesNumber;
    uint32_t seamNumber;
    uint32_t imageNumber;

    productNumber = _triggerContext.getProductNumber();
    seamseriesNumber = _triggerContext.getSeamSeriesNumber();
    seamNumber = _triggerContext.getSeamNumber();
    imageNumber = _triggerContext.imageNumber();

    ImageDataHolder imageDataHolder;
    bool isOK = m_SequenceLoader.getImage( _triggerContext.productInstance(), productNumber, seamseriesNumber, seamNumber, imageNumber, imageDataHolder );

    if(isOK)
    {
        _image = imageDataHolder.getByteImage();
        if(imageDataHolder.getIsExtraDataValid())
        {
            _triggerContext.HW_ROI_x0 = imageDataHolder.getHardwareRoiOffsetX();
            _triggerContext.HW_ROI_y0 = imageDataHolder.getHardwareRoiOffsetY();
            _triggerContext.HW_ROI_dx0 = _image.width();
            _triggerContext.HW_ROI_dy0 = _image.height();
            uint32_t imageNumberExt = imageDataHolder.getImageNumber();
            _triggerContext.setImageNumber(imageNumberExt);
            if( imageNumber != imageNumberExt )
            {
                wmLog( eDebug, " %s Internal Imagenumber differs to External Imagenumber %d != %d, proceeding with External.  \n", __FUNCTION__, imageNumber, imageNumberExt );
            }
        }
    }

    return isOK;
}

bool SequenceProvider::getSamples(TriggerContext & _triggerContext, fileio::SampleDataHolder & _sampleDataHolder)
{
    uint32_t productNumber;
    uint32_t seamseriesNumber;
    uint32_t seamNumber;
    uint32_t imageNumber;

    productNumber = _triggerContext.getProductNumber();
    seamseriesNumber = _triggerContext.getSeamSeriesNumber();
    seamNumber = _triggerContext.getSeamNumber();
    imageNumber = _triggerContext.imageNumber();

    bool isOK = m_SequenceLoader.getSamples( _triggerContext.productInstance(), productNumber, seamseriesNumber, seamNumber, imageNumber, _sampleDataHolder );
    return isOK;
}

} // namespace precitec
