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

#pragma once

#include <optional>
#include <string>
#include <Poco/UUID.h>

namespace precitec
{

class VdrFileInfo
{
public:
    void setPath(std::string const & _path);
    std::string getPath(void) const;
    void setKey(std::string const & _key);
    std::string getKey(void) const;

    uint32_t product() const;
    uint32_t seamSeries() const;
    uint32_t seam() const;
    uint32_t image() const;
    /**
     * The seam number from sequence_info.xml
     * May differ from @link{seam} in case of a linked seam
     * and points to the actual processed seam.
     **/
    const std::optional<uint32_t> &sequenceInfoSeamNumber() const;

    const Poco::UUID &productInstance() const;

    void setProduct(uint32_t number);
    void setSeamSeries(uint32_t number);
    void setSeam(uint32_t number);
    void setImage(uint32_t number);
    void setProductInstance(const Poco::UUID &id);
    void setSequenceInfoSeamNumber(const std::optional<uint32_t> &number);

private:
    std::string m_strPath;
    std::string m_strKey;
    uint32_t m_productNumber = 0;
    uint32_t m_seamSeriesNumber = 0;
    uint32_t m_seamNumber = 0;
    uint32_t m_imageNumber = 0;
    std::optional<uint32_t> m_sequenceInfoSeamNumber;
    Poco::UUID m_productInstance;
};
} // namespace precitec
