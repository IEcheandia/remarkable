#include "CHRCommunication/OCTDeviceConfiguration.h"

#include "Poco/AutoPtr.h"
#include "Poco/File.h"
#include <assert.h>
#include <array>

namespace precitec
{
namespace grabber
{
using namespace interface;

const OCTDeviceConfiguration::t_keylist OCTDeviceConfiguration::s_KeyList = OCTDeviceConfiguration::defineKeys();

OCTDeviceConfiguration::t_keylist OCTDeviceConfiguration::defineKeys()
{
    std::array<std::string, keyId::NUMKEYS> oResult;
    oResult[eSampleFrequency] = "SampleFrequency";           // Sample Frequency (SHZ)
    oResult[eLampIntensity] = "LampIntensity";               //"Lamp Intensity (LAI)"
    oResult[eDetectionWindowLeft] = "DetectionWindowLeft";   //"Detection Window left (DWD)"
    oResult[eDetectionWindowRight] = "DetectionWindowRight"; //"Detection Window right (DWD)"
    oResult[eQualityThreshold] = "QualityThreshold";         //Quality Threshold (QTH)
    oResult[eDataAveraging] = "DataAveraging";               //"Data Averaging (AVD)
    oResult[eSpectralAveraging] = "SpectralAveraging";       //"Spectral Averaging (AVS)"
    oResult[eWeldingDepthSystemOffset] = "WeldingDepthSystemOffset";
    oResult[eNumProfilesToSend] = "MaxNumProfilesToSend";
    oResult[eNumBufferLines] = "MaxNumBufferLines";
    assert(std::all_of(oResult.begin(), oResult.end(), [](std::string s)
                       { return !s.empty(); }));
    return oResult;
}

interface::SmpKeyValue OCTDeviceConfiguration::get(const std::string& p_rKey) const
{
    if (p_rKey == s_KeyList[eSampleFrequency])
    {
        return interface::SmpKeyValue(new TKeyValue<int>(p_rKey, m_oSampleFrequency, 32, 70000, 70000));
    }
    if (p_rKey == s_KeyList[eLampIntensity])
    {
        return interface::SmpKeyValue(new TKeyValue<int>(p_rKey, m_oLampIntensity, 0, 100, 8));
    }
    if (p_rKey == s_KeyList[eDetectionWindowLeft])
    {
        return interface::SmpKeyValue(new TKeyValue<int>(p_rKey, m_oDetectionWindow_Left, 1, 10000, 400));
    }
    if (p_rKey == s_KeyList[eDetectionWindowRight])
    {
        return interface::SmpKeyValue(new TKeyValue<int>(p_rKey, m_oDetectionWindow_Right, 1, 10000, 9600));
    }
    if (p_rKey == s_KeyList[eQualityThreshold])
    {
        return interface::SmpKeyValue(new TKeyValue<int>(p_rKey, m_oQualityThreshold, 1, 999, 25));
    }
    if (p_rKey == s_KeyList[eDataAveraging])
    {
        return interface::SmpKeyValue(new TKeyValue<int>(p_rKey, m_oDataAveraging, 1, 9999, 1));
    }
    if (p_rKey == s_KeyList[eSpectralAveraging])
    {
        return interface::SmpKeyValue(new TKeyValue<int>(p_rKey, m_oSpectralAveraging, 1, 256, 1));
    }
    if (p_rKey == s_KeyList[eWeldingDepthSystemOffset])
    {
        return SmpKeyValue(new TKeyValue<int>(p_rKey, m_oWeldingDepthSystemOffset, 0, 10000, 0));
    }
    if (p_rKey == s_KeyList[eNumProfilesToSend])
    {
        return SmpKeyValue(new TKeyValue<int>(p_rKey, m_oNumProfilesToSend, 1, 65536, 1024));
    }
    if (p_rKey == s_KeyList[eNumBufferLines])
    {
        return SmpKeyValue(new TKeyValue<int>(p_rKey, m_oNumBuferLines, 1024, 1048576, 8192));
    }
    return {};
}

int OCTDeviceConfiguration::getInt(Poco::Util::XMLConfiguration* pConfIn, keyId keyId) const
{
    std::string key = s_KeyList[keyId];
    assert(get(key)->type() == TInt);
    int defaultValue = get(key)->defValue<int>();

    if (pConfIn == nullptr)
    {
        return defaultValue;
    }
    return pConfIn->getInt(key, defaultValue);
}

double OCTDeviceConfiguration::getDouble(Poco::Util::XMLConfiguration* pConfIn, keyId keyId) const
{
    std::string key = s_KeyList[keyId];
    assert(get(key)->type() == TDouble);
    double defaultValue = get(key)->defValue<double>();

    if (pConfIn == nullptr)
    {
        return defaultValue;
    }
    return pConfIn->getDouble(key, defaultValue);
}

bool OCTDeviceConfiguration::initFromFile(const std::string& p_rFilePath)
{
    Poco::AutoPtr<Poco::Util::XMLConfiguration> pConfIn;

    if (Poco::File(p_rFilePath).exists())
    {
        try
        { // poco syntax exception might be thrown or sax parse excpetion
            pConfIn = new Poco::Util::XMLConfiguration(p_rFilePath);
        }
        catch (const Poco::Exception& p_rException)
        {
            std::ostringstream oMsg;
            oMsg << "Exception loading XMLConfiguration " << p_rFilePath << "  " << p_rException.displayText() << "\n";
            wmLog(eWarning, oMsg.str());
        } // catch
    }
    else
    {
        std::ostringstream oMsg;
        oMsg << "File " << p_rFilePath << " does not exist "
             << "\n";
        wmLog(eInfo, oMsg.str());
    }

    m_oSampleFrequency = getInt(pConfIn, keyId::eSampleFrequency);
    m_oLampIntensity = getInt(pConfIn, keyId::eLampIntensity);
    m_oDetectionWindow_Left = getInt(pConfIn, keyId::eDetectionWindowLeft);
    m_oDetectionWindow_Right = getInt(pConfIn, keyId::eDetectionWindowRight);
    m_oQualityThreshold = getInt(pConfIn, keyId::eQualityThreshold);
    m_oDataAveraging = getInt(pConfIn, keyId::eDataAveraging);
    m_oSpectralAveraging = getInt(pConfIn, keyId::eSpectralAveraging);
    m_oWeldingDepthSystemOffset = getInt(pConfIn, keyId::eWeldingDepthSystemOffset);
    m_oNumProfilesToSend = getInt(pConfIn, keyId::eNumProfilesToSend);
    m_oNumBuferLines = getInt(pConfIn, keyId::eNumBufferLines);

    return !(pConfIn.isNull());
}

interface::Configuration OCTDeviceConfiguration::makeConfiguration() const
{
    interface::Configuration oConfig;
    oConfig.reserve(s_KeyList.size());
    for (auto& rKey : s_KeyList)
    {
        oConfig.push_back(get(rKey));
    }
    assert(oConfig.size() == s_KeyList.size());
    return oConfig;
}

bool OCTDeviceConfiguration::writeToFile(const std::string& p_rFilePath) const
{
    interface::writeToFile(p_rFilePath, makeConfiguration());
    return true;
}

} //namespace
}
