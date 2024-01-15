/**
*
* @defgroup Framegrabber Framegrabber
*
* \section sec VDR File Loading
*
*
* @file
* @brief  Dataholder Class of Information about previously recorded images and/or Samples corresponding to product, seamseries and seam
* @copyright    Precitec GmbH & Co. KG
* @author GG
* @date   18.05.17
*
*
*/

#include "trigger/vdrFileInfo.h"

namespace precitec
{

    void VdrFileInfo::setPath(std::string const & _path)
    {
        m_strPath = _path;
    }

    std::string VdrFileInfo::getPath(void) const
    {
        return std::string(m_strPath);
    }

    void VdrFileInfo::setKey(std::string const & _key)
    {
        m_strKey = _key;
    }

    std::string VdrFileInfo::getKey(void) const
    {
        return std::string(m_strKey);
    }

    uint32_t VdrFileInfo::product() const
    {
        return m_productNumber;
    }

    uint32_t VdrFileInfo::seamSeries() const
    {
        return m_seamSeriesNumber;
    }

    uint32_t VdrFileInfo::seam() const
    {
        return m_seamNumber;
    }

    uint32_t VdrFileInfo::image() const
    {
        return m_imageNumber;
    }

    void VdrFileInfo::setProduct(uint32_t number)
    {
        m_productNumber = number;
    }

    void VdrFileInfo::setSeamSeries(uint32_t number)
    {
        m_seamSeriesNumber = number;
    }

    void VdrFileInfo::setSeam(uint32_t number)
    {
        m_seamNumber = number;
    }

    void VdrFileInfo::setImage(uint32_t number)
    {
        m_imageNumber = number;
    }

    const Poco::UUID & VdrFileInfo::productInstance() const
    {
        return m_productInstance;
    }

    void VdrFileInfo::setProductInstance(const Poco::UUID &id)
    {
        m_productInstance = id;
    }

    const std::optional<uint32_t> &VdrFileInfo::sequenceInfoSeamNumber() const
    {
        return m_sequenceInfoSeamNumber;
    }

    void VdrFileInfo::setSequenceInfoSeamNumber(const std::optional<uint32_t> &number)
    {
        m_sequenceInfoSeamNumber = number;
    }

} // namespace precitec
