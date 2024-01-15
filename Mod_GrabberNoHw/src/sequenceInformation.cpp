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

#include <filesystem>
#include <string>
#include <sstream>
#include <cstdlib>
#undef min
#undef max
#include <limits>
#include <iostream>
#include <vector>
#include "Poco/Exception.h"
#if !defined(__QNX__)
#include "Poco/NumericString.h"
#endif
#include "Poco/UUID.h"
#include "trigger/sequenceInformation.h"

#include <Poco/Util/XMLConfiguration.h>

namespace fs = std::filesystem;

namespace precitec
{

#if defined(WM_RUNNER_DOMAIN)
#define wmLog(unused, str1) std::cout << str1 << std::endl;
#endif

namespace
{
/**
 * @brief Tries to extract a trailning multi-digit number from a string containing the requiredPrefix.
 *
 * @returns Whenever the string toParse has the form <something><requiredPrefix>####, for some trailing digits,
 * #### is returned as an integer. If #### is not parsable as integer, or the <requiredPrefix> does not match the
 * non-numeric characters before it, std::nullopt is returned.
 */
std::optional<uint32_t> parseFolderName(const std::string& toParse, const std::string& requiredPrefix)
{
    try
    {
        const auto lastNonDigit = std::find_if_not(toParse.rbegin(), toParse.rend(), [](unsigned char c)
                                                   { return std::isdigit(c); });
        if (lastNonDigit == toParse.rbegin())
        {
            // There is no digit at the end of the string.
            return std::nullopt;
        }
        auto toParseRestBegin = lastNonDigit;
        for (auto prefixIt = requiredPrefix.rbegin(); prefixIt != requiredPrefix.rend(); ++prefixIt) {
            if (*toParseRestBegin != *prefixIt || toParseRestBegin == toParse.rend())
            {
                return std::nullopt;
            }
            ++toParseRestBegin;
        }
        const std::string digitString{lastNonDigit.base(), toParse.end()};
        return std::stoul(digitString);
    }
    catch (const std::exception & p_rException)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " " << p_rException.what() << "\n";
        wmLog(eWarning, oMsg.str());
    }
    catch (...)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " Unknown exception encountered.\n";
        wmLog(eWarning, oMsg.str());
    }
    return std::nullopt;
}

VdrFileType toVdrFileType(const fs::path& _fileName)
{
    try
    {
        std::string fileExtension = _fileName.extension();
        Poco::toLowerInPlace(fileExtension);
        if (fileExtension == ".bmp")
        {
            return ImageVdrFileType;
        }
        else if (fileExtension == ".smp")
        {
            return SampleVdrFileType;
        }
    }
    catch (Poco::Exception const& _ex)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
        wmLog(eWarning, oMsg.str());
    }
    catch (const std::exception& p_rException)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " " << p_rException.what() << "\n";
        wmLog(eWarning, oMsg.str());
    }
    catch (...)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " Unknown exception encountered.\n";
        wmLog(eWarning, oMsg.str());
    }

    return UnknownVdrFileType;
}

std::optional<uint32_t> parseFileNumberFromName(const fs::path& vdrPath)
{
    try
    {
        return std::stoul(vdrPath.stem());
    }
    catch (...)
    {
        wmLog(eDebug, "Failed to parse image number\n");
        std::cout << "Failed to parse image number" << std::endl;
        return std::nullopt;
    }
}

bool isDirectory(const fs::directory_entry& entry)
{
    return entry.is_directory();
}

bool isRegularFile(const fs::directory_entry& entry)
{
    return entry.is_regular_file();
}

template<class PredicateT>
void makeSortedPathsList(const fs::path& parentDir, std::vector<fs::path> & folders, const PredicateT & isRelevant)
{
    folders.clear();
    for (const auto& seamSeriesFolder : fs::directory_iterator{parentDir})
    {
        if (isRelevant(seamSeriesFolder))
        { // skip all but folders
            folders.emplace_back(seamSeriesFolder.path());
        }
    }
    std::sort(folders.begin(), folders.end());
}
}

void SequenceInformation::reset()
{
    m_ImageFolderMap.clear();
    m_ImageFolderVector.clear();
    m_imageAndSampleFolderVector.clear();
    m_SampleFolderMap.clear();
    m_SampleFolderVector.clear();
}

void SequenceInformation::setBasepath(std::string const & _path)
{
    m_Basepath = _path;
}

SequenceInformation::VdrFolderMap_t & SequenceInformation::ImageFolderMap()
{
    return m_ImageFolderMap;
}

SequenceInformation::VdrFolderMap_t & SequenceInformation::SampleFolderMap()
{
    return m_SampleFolderMap;
}

std::deque<VdrFileInfo> & SequenceInformation::ImageFolderVector()
{
    return m_ImageFolderVector;
}

std::deque<VdrFileInfo> & SequenceInformation::SampleFolderVector()
{
    return m_SampleFolderVector;
}

void SequenceInformation::scanFolders()
{
    try
    {

        std::vector<fs::path> firstLevelFolders;
        std::vector<fs::path> seamFolders;
        std::vector<fs::path> seamSeriesFolders;
        std::vector<fs::path> files;
        makeSortedPathsList(m_Basepath, firstLevelFolders, [](const auto&)
                            { return true; });
        std::cout << "SEQUENCE_INFORMATION::scanFlders(); Number of Products level folders: " << firstLevelFolders.size()  - 1 << std::endl;

        for (const auto & firstLevelFolder : firstLevelFolders)
        { // all product folders in base folder
            
          //  std::cout << "firstLevelFolder: " << firstLevelFolder.c_str() << std::endl;
            wmLog(eDebug, "firstLevelFolder: %s \n", firstLevelFolder.c_str());
            ProductFolderMap_t productFolderMap;
            if (!findProductFolders(firstLevelFolder, productFolderMap))
            {
                continue;
            }

            for (const auto & [instanceFolder, instanceIds] : productFolderMap)
            {
               // std::cout << "instance folder: " << instanceFolder.c_str() << std::endl;
                 wmLog(eDebug, "instancefolder: %s \n", instanceFolder.c_str());
                if (!m_instanceId.isNull() && m_instanceId != instanceIds.first)
                {
                    continue;
                }
                
                makeSortedPathsList(instanceFolder, seamSeriesFolders, isDirectory);
                for (const auto & seamSeriesFolder : seamSeriesFolders)
                { // all seamseries folders in product folder
                   // std::cout << "seamseriesfolder: " << seamSeriesFolder.c_str() << std::endl;
                     wmLog(eDebug, "seamseriesfolder: %s \n", seamSeriesFolder.c_str());
                    const auto seamseriesNumber = parseFolderName(seamSeriesFolder, "seam_series");
                    if (!seamseriesNumber.has_value())
                    {
                        continue;
                    }

                    makeSortedPathsList(seamSeriesFolder, seamFolders, isDirectory);
                    for (const auto& seamFolder :seamFolders)
                    { // all seam folders in seam series folder
                       // std::cout << "seam: " << firstLevelFolder.c_str() << std::endl;
                         wmLog(eDebug, "seamfolder: %s \n", seamFolder.c_str());
                        const auto seamNumber = parseFolderName(seamFolder, "seam");
                        if (!seamNumber.has_value())
                        {
                            continue;
                        }

                        const auto sequenceInfoSeam = seamNumberFromSequenceInfo(seamFolder);
                        makeSortedPathsList(seamFolder, files, isRegularFile);

                        uint32_t vdrCount = 0;
                        for (const auto& fileName : files)
                        { // all files in seam folder
                            const VdrFileType vdrFileType = toVdrFileType(fileName);
                            if(vdrFileType == UnknownVdrFileType) {
                                continue;
                            }
                            const uint32_t vdrNumber = parseFileNumberFromName(fileName).value_or(vdrCount);
                            
                         //   wmLog(eDebug, "Parse number from name: %d y vdrcount: %d\n", vdrNumber, vdrCount);
                            if (insertVdrFile(fileName, instanceIds.first, instanceIds.second, *seamseriesNumber, *seamNumber, sequenceInfoSeam, vdrNumber, vdrFileType))
                            {
                                vdrCount++;
                            }
                        }
                    }
                }
            }
        }
    }
    catch (Poco::Exception const & _ex)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
        wmLog(eWarning, oMsg.str());
    }
    catch (const std::exception &p_rException)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " " << p_rException.what() << "\n";
        wmLog(eWarning, oMsg.str());
    }
    catch (...)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " Unknown exception encountered.\n";
        wmLog(eWarning, oMsg.str());
    }
}

bool SequenceInformation::findProductFolders(const fs::path& folderPath, ProductFolderMap_t& _productFolderMap)
{
    try
    {
        if (!fs::is_directory(folderPath))
        {
            return false; // skip all but folders
        }

        const auto serialNumber = parseFolderName(folderPath, "SN-");
        if (serialNumber.has_value())
        {
            const fs::path idExtension{".id"};
            fs::directory_iterator fileIter{folderPath};
            auto it = std::find_if(fileIter, fs::directory_iterator(),
                                   [&](const fs::directory_entry& file)
                                   {
                                       return file.is_regular_file() && file.path().extension() == idExtension;
                                   });
            Poco::UUID productInstanceId;
            if (it != fs::directory_iterator())
            {
                productInstanceId = Poco::UUID(it->path().stem());
            }
            _productFolderMap.insert(ProductFolderMap_t::ValueType(folderPath, std::make_pair(productInstanceId, *serialNumber)));
            return true;
        }
        else
        {
            bool foundOne = false;
            std::vector<fs::path> candidateDirs;
            makeSortedPathsList(folderPath, candidateDirs, [](const auto&)
                                { return true; });
            for (const auto & candidate : candidateDirs) {
                if (findProductFolders(candidate, _productFolderMap))
                {
                    foundOne = true;
                }            
            }
            return foundOne;
        }
    }
    catch (Poco::Exception const & _ex)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
        wmLog(eWarning, oMsg.str());
    }
    catch (const std::exception &p_rException)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " " << p_rException.what() << "\n";
        wmLog(eWarning, oMsg.str());
    }
    catch (...)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " Unknown exception encountered.\n";
        wmLog(eWarning, oMsg.str());
    }
    return false;
}

bool SequenceInformation::insertVdrFile(const fs::path& _vdrFileName, const Poco::UUID& productInstance, uint32_t _productNumber, uint32_t _seamseriesNumber, uint32_t _seamNumber, const std::optional<uint32_t>& sequenceInfoSeamNumber, uint32_t vdrNumber, VdrFileType vdrFileType)
{
    try
    {
        std::string vdrFileKey;
        switch (m_mode)
        {
        case OperationMode::SerialNumber:
            makeVdrFileKey(_productNumber, _seamseriesNumber, _seamNumber, vdrNumber, vdrFileKey);
            break;
        case OperationMode::ProductInstance:
            vdrFileKey = makeVdrFileKey(productInstance, _seamseriesNumber, _seamNumber, vdrNumber);
            break;
        default:
            // nothing
            break;
        }
        
        wmLog(eDebug, "_productNumber: %d \n", _productNumber);
        VdrFileInfo vdrFileInfo;
        vdrFileInfo.setPath(_vdrFileName);
        vdrFileInfo.setKey(vdrFileKey);
        vdrFileInfo.setProduct(_productNumber);
        vdrFileInfo.setSeamSeries(_seamseriesNumber);
        vdrFileInfo.setSeam(_seamNumber);
        vdrFileInfo.setSequenceInfoSeamNumber(sequenceInfoSeamNumber);
        vdrFileInfo.setImage(vdrNumber);
        vdrFileInfo.setProductInstance(productInstance);

        if (vdrFileType == ImageVdrFileType)
        {
            m_ImageFolderMap.insert(VdrFolderMap_t::ValueType(vdrFileKey, vdrFileInfo));
            m_ImageFolderVector.push_back(vdrFileInfo);
        }
        else if (vdrFileType == SampleVdrFileType)
        {
            m_SampleFolderMap.insert(VdrFolderMap_t::ValueType(vdrFileKey, vdrFileInfo));
            m_SampleFolderVector.push_back(vdrFileInfo);
        } else {
            // This function is always called with a valid VdrFileType, so this should not happen.
            return false;
        }

        
        if (m_imageAndSampleFolderVector.empty())
        {
            m_imageAndSampleFolderVector.push_back(vdrFileInfo);
        } else
        {
            const auto &last = m_imageAndSampleFolderVector.back();
            if (last.product() != _productNumber || last.seamSeries() != _seamseriesNumber || last.seam() != _seamNumber || last.image() != vdrNumber )
            {
                m_imageAndSampleFolderVector.push_back(vdrFileInfo);
            }
            else
            {
                 m_imageAndSampleFolderVector.push_back(vdrFileInfo);
            }
        }

        return true;
    }
    catch (Poco::Exception const & _ex)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " " << _ex.what() << " - " << _ex.message() << "\n";
        wmLog(eWarning, oMsg.str());
    }
    catch (const std::exception &p_rException)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " " << p_rException.what() << "\n";
        wmLog(eWarning, oMsg.str());
    }
    catch (...)
    {
        std::ostringstream oMsg;
        oMsg << __FUNCTION__ << " Unknown exception encountered.\n";
        wmLog(eWarning, oMsg.str());
    }

    return false;
}

void SequenceInformation::makeVdrFileKey(uint32_t _productNumber, uint32_t _seamseriesNumber, uint32_t _seamNumber, uint32_t _imageNumber, std::string & _strKey) const
{
    std::string strUInt;
    std::string strKey;
#if !defined(__QNX__)
    Poco::uIntToStr(_productNumber, 10, strUInt);
#else
    uIntToString(_productNumber, strUInt);
#endif
    strKey.append(strUInt);
#if !defined(__QNX__)
    Poco::uIntToStr(_seamseriesNumber, 10, strUInt);
#else
    uIntToString(_seamseriesNumber, strUInt);
#endif
    strKey.append(".");
    strKey.append(strUInt);
#if !defined(__QNX__)
    Poco::uIntToStr(_seamNumber, 10, strUInt);
#else
    uIntToString(_seamNumber, strUInt);
#endif
    strKey.append(".");
    strKey.append(strUInt);
#if !defined(__QNX__)
    Poco::uIntToStr(_imageNumber, 10, strUInt);
#else
    uIntToString(_imageNumber, strUInt);
#endif
    strKey.append(".");
    strKey.append(strUInt);
    _strKey = strKey;
}

std::string SequenceInformation::makeVdrFileKey(const Poco::UUID &productInstance, uint32_t _seamseriesNumber, uint32_t _seamNumber, uint32_t _imageNumber) const
{
    std::string strUInt;
    std::string strKey;
    strKey.append(productInstance.toString());
#if !defined(__QNX__)
    Poco::uIntToStr(_seamseriesNumber, 10, strUInt);
#else
    uIntToString(_seamseriesNumber, strUInt);
#endif
    strKey.append(".");
    strKey.append(strUInt);
#if !defined(__QNX__)
    Poco::uIntToStr(_seamNumber, 10, strUInt);
#else
    uIntToString(_seamNumber, strUInt);
#endif
    strKey.append(".");
    strKey.append(strUInt);
#if !defined(__QNX__)
    Poco::uIntToStr(_imageNumber, 10, strUInt);
#else
    uIntToString(_imageNumber, strUInt);
#endif
    strKey.append(".");
    strKey.append(strUInt);
    return strKey;
}

std::optional<uint32_t> SequenceInformation::seamNumberFromSequenceInfo(const fs::path& seamDirectory)
{
    try
    {
        const auto sequenceInfoFile = seamDirectory / "sequence_info.xml";
        if (fs::exists(sequenceInfoFile))
        {
            Poco::AutoPtr<Poco::Util::XMLConfiguration> sequenceInfo{new Poco::Util::XMLConfiguration{sequenceInfoFile}};
            return sequenceInfo->getUInt(std::string{"seam"});
        }
    }
    catch (...)
    {
        return std::nullopt;
    }

    return std::nullopt;
}
}
