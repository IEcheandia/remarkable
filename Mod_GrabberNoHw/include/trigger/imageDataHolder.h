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

#pragma once

#include <string>
#if defined(__QNX__)
	#include "image/image.h"
	#include "common/bitmap.h"
#else
	#include "../Analyzer_Interface/include/image/image.h"
	#include "../Interfaces/include/common/bitmap.h"
#endif

namespace precitec
{

class ImageDataHolder
{
public:

    ImageDataHolder();
    ~ImageDataHolder();

    void setPath(std::string const & _path);
    std::string getPath(void) const;
    void setKey(std::string const & _key);
    std::string getKey(void) const;
    void setImageNumber(int _newValue);
    int getImageNumber(void) const;
    void setByteImage(image::BImage const & _newValue);
    image::BImage getByteImage(void) const;
    void setHardwareRoiOffsetX(int _newValue);
    int getHardwareRoiOffsetX(void) const;
    void setHardwareRoiOffsetY(int _newValue);
    int getHardwareRoiOffsetY(void) const;
    void setIsExtraDataValid(bool _newValue);
    bool getIsExtraDataValid(void) const;


private:
    bool m_isExtraDataValid;
    int m_ImageNumber;
    int m_HardwareRoiOffsetX;
    int m_HardwareRoiOffsetY;
    std::string m_strPath;
    std::string m_strKey;
    image::BImage m_BImage;
};
} // namespace precitec
