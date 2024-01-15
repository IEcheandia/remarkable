#include <calibration/ScanFieldImageCalculator.h>
#include <common/bitmap.h>
#include <filesystem>

using precitec::image::ImageFillMode;

namespace precitec{
namespace calibration{    


ScanFieldImageCalculator::ScanFieldImageCalculator(ScanFieldImageParameters p_parameters)
  : m_parameters(std::move(p_parameters))
{   
#ifndef NDEBUG
    std::ostringstream oMsg;
    oMsg <<  "ScanFieldImageCalculator CTOR " << p_parameters << "\n";
    wmLog(eDebug,  oMsg.str());
#endif
    m_Mask.assign(m_parameters.m_scanfieldimageSize.area(),0);
    m_Sum.assign(m_parameters.m_scanfieldimageSize.area(),0);
}


std::tuple<image::BImage,std::string, std::string> ScanFieldImageCalculator::computeAndWriteScanFieldImage(const std::string outputFolder) const
{
    using namespace precitec;
    using namespace precitec::image;
    
    BImage oOutput(m_parameters.m_scanfieldimageSize);
    std::string imageFilename = "";
    std::string configurationFilename = "";
    
    auto itCount = m_Mask.cbegin();
    auto fComputePixelValue = [&itCount] (const int & pixelSum)
    {
        assert((*itCount) > 0 || pixelSum == 0);
        byte pixel = pixelSum;
        if ((*itCount)>1)
        {
            pixel = std::round(pixelSum/double(*itCount));
            
        }   
        assert(pixel >= 0 && pixel <= 255);                                        
        itCount++;
        return pixel;
    };
    
    assert(oOutput.isValid() && oOutput.isContiguos());
    std::transform(m_Sum.cbegin(),m_Sum.cend(), oOutput.begin(), fComputePixelValue);
    
    assert(itCount == m_Mask.cend());
    
    switch(m_parameters.m_ScanMasterData.getImageFillMode())
    {
        case image::ImageFillMode::Direct:
            //nothing to do
            break;
        case image::ImageFillMode::FlippedHorizontal:
            //oOutput = oOutput.copyHorizontallyFlipped();
            oOutput = image::fillBImage<ImageFillMode::FlippedHorizontal>(oOutput.width(), oOutput.height(), oOutput.begin());            
            break;
        case image::ImageFillMode::FlippedVertical:
            //oOutput = oOutput.copyVerticallyFlipped();
            oOutput = image::fillBImage<ImageFillMode::FlippedVertical>(oOutput.width(), oOutput.height(), oOutput.begin());            
            break;
        case image::ImageFillMode::Reverse:
            oOutput = image::fillBImage<ImageFillMode::Reverse>(oOutput.width(), oOutput.height(), oOutput.begin());
            break;        
    }
        
    if (!outputFolder.empty())
    {
        assert(outputFolder.back() ==  '/');
        const std::filesystem::path directory{outputFolder};
        for (const auto& entry : std::filesystem::directory_iterator{directory})
        {
            if (!entry.is_regular_file())
            {
                continue;
            }
            const std::string compareString{"ScanFieldImage_"};
            if (entry.path().filename().string().substr(0, compareString.length()) == compareString)
            {
                std::filesystem::remove(entry);
            }
        }

        imageFilename = outputFolder + "ScanFieldImage.bmp";
        fileio::Bitmap oBitmap(imageFilename, oOutput.width(), oOutput.height(),false);
        bool saved = false;
        if (oBitmap.isValid()) 
        {
            oBitmap.save(oOutput.data());
            
            configurationFilename = outputFolder + "ScanFieldImage.ini";
            saved = m_parameters.saveToIni(configurationFilename);

        }
        if (!saved)
        {
            imageFilename = "";
            configurationFilename = "";
        }
    }
    return {oOutput,  outputFolder,  configurationFilename};
} 


//no subpixel interpolation
bool ScanFieldImageCalculator::pasteImage(const precitec::image::BImage & rImage, int xTopLeft_pix, int yTopLeft_pix)
{
    if ( xTopLeft_pix < 0 || (xTopLeft_pix + rImage.width()) > m_parameters.m_scanfieldimageSize.width
        || yTopLeft_pix < 0 || (yTopLeft_pix + rImage.height()) > m_parameters.m_scanfieldimageSize.height)
    {
        return false;
    }
    
    
    for (int sourceY = 0, lastSourceY = rImage.height(), destinationY = yTopLeft_pix; sourceY< lastSourceY; sourceY++, destinationY++)
    {
        auto * pPixel = rImage.rowBegin(sourceY);
        auto * pRowEnd = rImage.rowEnd(sourceY);
        
        auto index = destinationY * m_parameters.m_scanfieldimageSize.width + xTopLeft_pix;
        auto itMask = m_Mask.begin() + index;
        auto itSum = m_Sum.begin() + index;
        for (; pPixel != pRowEnd; ++pPixel, ++itMask, ++itSum)
        {
            (*itSum) += (*pPixel);
            (*itMask)++;
        }
        
    }
    return true;
}
    


const ScanFieldImageParameters& ScanFieldImageCalculator::getParameters() const 
{
    return m_parameters;
}



}
}
