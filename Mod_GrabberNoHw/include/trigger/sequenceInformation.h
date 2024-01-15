
/**
*
* @defgroup Framegrabber Framegrabber
*
* \section sec Image Loading
*
*
* @file
* @brief  File Helper to browse previously recorded images and/or Samples corresponding to product, seamseries and seam
* @copyright    Precitec GmbH & Co. KG
* @author GG
* @date   16.08.17
*
*
*/


#pragma once

#include "Poco/ListMap.h"
#include "Poco/HashMap.h"
#include "vdrFileInfo.h"
#if ! defined (WM_RUNNER_DOMAIN)
#include "module/moduleLogger.h"
#endif

#include <deque>
#include <filesystem>
#include <optional>
#include <vector>


namespace precitec
{
using Poco::ListMap;
using Poco::HashMap;

enum VdrFileType
{
    UnknownVdrFileType,
    ImageVdrFileType,
    SampleVdrFileType
};

class SequenceInformation
{
public:
#if !defined(__QNX__)
    using VdrFolderMap_t = HashMap<std::string, VdrFileInfo>;
    using ProductFolderMap_t = ListMap<std::filesystem::path, std::pair<Poco::UUID, uint32_t>>;
#else
    typedef ListMap<std::string, VdrFileInfo> VdrFolderMap_t;
    typedef ListMap<std::string, uint32_t> ProductFolderMap_t;
#endif

    enum class OperationMode {
        SerialNumber,
        ProductInstance
    };

    void setOperationMode(OperationMode mode)
    {
        if (m_mode == mode)
        {
            return;
        }
        m_mode = mode;
    }

    void reset();
    void setBasepath(std::string const & _path);
    void scanFolders();

    void makeVdrFileKey(uint32_t _productNumber, uint32_t _seamseriesNumber, uint32_t _seamNumber, uint32_t _imageNumber, std::string & _strKey) const;
    std::string makeVdrFileKey(const Poco::UUID &productInstance, uint32_t _seamseriesNumber, uint32_t _seamNumber, uint32_t _imageNumber) const;
    
    bool findProductFolders(const std::filesystem::path & folderPath, ProductFolderMap_t & _productFolderMap);
    
    VdrFolderMap_t & ImageFolderMap();
    VdrFolderMap_t & SampleFolderMap();
    std::deque<VdrFileInfo> & ImageFolderVector();
    std::deque<VdrFileInfo> & SampleFolderVector();

    /**
        * A combination of images and samples containing for each image number either a sample or an image.
        **/
    const std::deque<VdrFileInfo> &imageAndSampleVector() const
    {
        return m_imageAndSampleFolderVector;
    }

    void setInstanceId(const Poco::UUID &id)
    {
        m_instanceId = id;
    }
    Poco::UUID instanceId() const
    {
        return m_instanceId;
    }

private:
    bool insertVdrFile(const std::filesystem::path & _imageName, const Poco::UUID &productInstance, uint32_t _productNumber, uint32_t _seamseriesNumber, uint32_t _seamNumber, const std::optional<uint32_t> &sequenceInfoSeamNumber, uint32_t vdrNumber, VdrFileType vdrFileType);

    std::optional<uint32_t> seamNumberFromSequenceInfo(const std::filesystem::path &seamDirectory);
    VdrFolderMap_t m_ImageFolderMap;
    VdrFolderMap_t m_SampleFolderMap;
    std::deque<VdrFileInfo> m_ImageFolderVector;
    std::deque<VdrFileInfo> m_SampleFolderVector;
    std::deque<VdrFileInfo> m_imageAndSampleFolderVector;
    std::string m_Basepath;
    OperationMode m_mode = OperationMode::SerialNumber;
    Poco::UUID m_instanceId;
};
}
