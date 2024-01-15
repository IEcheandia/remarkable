/**
*
* @defgroup Framegrabber Framegrabber
*
* \section sec Image Loading
*
*
* @file
* @brief  Dataholder Class of Data of previously recorded images corresponding to product, seamseries and seam
* @copyright    Precitec GmbH & Co. KG
* @author GG
* @date   23.05.17
*
*/

#include "../include/trigger/imageDataHolder.h"

namespace precitec
{
ImageDataHolder::ImageDataHolder() :
    m_isExtraDataValid(false),
    m_ImageNumber(0),
    m_HardwareRoiOffsetX(0),
    m_HardwareRoiOffsetY(0)
{
}

ImageDataHolder::~ImageDataHolder()
{
}

void ImageDataHolder::setPath(std::string const & _path)
{
    m_strPath = _path;
}

std::string ImageDataHolder::getPath(void) const
{
    return std::string(m_strPath);
}

void ImageDataHolder::setKey(std::string const & _key)
{
    m_strKey = _key;
}

std::string ImageDataHolder::getKey(void) const
{
    return std::string(m_strKey);
}

void ImageDataHolder::setImageNumber(int _newValue)
{
    m_ImageNumber=_newValue;
}

int ImageDataHolder::getImageNumber(void) const
{
    return m_ImageNumber;
}

void ImageDataHolder::setHardwareRoiOffsetX(int _newValue)
{
    m_HardwareRoiOffsetX=_newValue;
}

int ImageDataHolder::getHardwareRoiOffsetX(void) const
{
    return m_HardwareRoiOffsetX;
}

void ImageDataHolder::setHardwareRoiOffsetY(int _newValue)
{
    m_HardwareRoiOffsetY=_newValue;
}

int ImageDataHolder::getHardwareRoiOffsetY(void) const
{
    return m_HardwareRoiOffsetY;
}

void ImageDataHolder::setIsExtraDataValid(bool _newValue)
{
    m_isExtraDataValid=_newValue;
}

bool ImageDataHolder::getIsExtraDataValid(void) const
{
    return m_isExtraDataValid;
}

void ImageDataHolder::setByteImage(image::BImage const & _newValue)
{
    m_BImage=_newValue;
}

image::BImage ImageDataHolder::getByteImage(void) const
{
    return m_BImage;
}

} // namespace precitec
