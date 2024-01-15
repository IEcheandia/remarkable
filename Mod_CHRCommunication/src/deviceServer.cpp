/**
 * @file
 * @brief   DeviceServer von App_CHRCommunication
 *
 * @author  EA
 * @date    23.10.2017
 * @version 1.0
 */

#include <iostream>
#include <unistd.h>
#include <string>

#include "CHRCommunication/deviceServer.h"
#include "CHRCommunication/OCTDeviceConfiguration.h"

#define DEBUG_DEVICESERVER 0

namespace precitec
{

namespace grabber
{

/**
 * @brief CTOR
 *
 */
DeviceServer::DeviceServer(CHRCommunication& p_rCHRCommunication)
    : m_rCHRCommunication(p_rCHRCommunication)
{
}

/**
 * @brief DTOR
 *
 */
DeviceServer::~DeviceServer()
{
}

int DeviceServer::initialize(Configuration const& config, int subDevice)
{
    return 0;
}

void DeviceServer::uninitialize()
{
}

void DeviceServer::reinitialize()
{
}

KeyHandle DeviceServer::set(SmpKeyValue keyValue, int subDevice)
{

    auto& rKeyList = OCTDeviceConfiguration::s_KeyList;
    typedef OCTDeviceConfiguration::keyId KeyId;

    ValueTypes keyType = keyValue->type();

    switch (keyType)
    {
    case TBool:
        if (keyValue->key() == "Results On/Off")
        {
            m_rCHRCommunication.SetResultsOnOff(keyValue->value<bool>());
        }
        if (keyValue->key() == "Ask Version String")
        {
            m_rCHRCommunication.AskVersionString();
        }
        if (keyValue->key() == "Download FFT Spectrum")
        {
            m_rCHRCommunication.DownloadFFTSpectrum();
        }
        if (keyValue->key() == "Perform Dark Reference")
        {
            m_rCHRCommunication.PerformDarkReference();
        }
        if (keyValue->key() == "SLD Dimmer On/Off")
        {
            m_rCHRCommunication.SetSLDDimmerOnOff(keyValue->value<bool>());
        }
        if (keyValue->key() == "Adaptive Exposure Mode On/Off")
        {
            m_rCHRCommunication.SetAdaptiveExposureOnOff(keyValue->value<bool>());
        }
        if (keyValue->key() == "Debug Info CHR Response")
        {
            m_rCHRCommunication.SetDebugInfoCHRResponseOnOff(keyValue->value<bool>());
        }
        if (keyValue->key() == "TestFunction 2")
        {
            m_rCHRCommunication.TestFunction2();
        }
        if (keyValue->key() == "TestFunction 3")
        {
            m_rCHRCommunication.TestFunction3();
        }
        if (keyValue->key() == "TestFunction 4")
        {
            m_rCHRCommunication.TestFunction4();
        }
        if (keyValue->key() == "TestFunction 5")
        {
            m_rCHRCommunication.TestFunction5();
        }
        if (keyValue->key() == "TestFunction 6")
        {
            m_rCHRCommunication.TestFunction6();
        }

#if DEBUG_DEVICESERVER
        printf("set %s: %d\n", keyValue->key().c_str(), keyValue->value<bool>());
#endif
        break;

    case TInt:

        if (keyValue->key() == rKeyList[KeyId::eSampleFrequency])
        {
            m_rCHRCommunication.SetSampleFrequency(keyValue->value<int>());
        }
        if (keyValue->key() == rKeyList[KeyId::eLampIntensity])
        {
            m_rCHRCommunication.SetLampIntensity(keyValue->value<int>());
        }
        if (keyValue->key() == rKeyList[KeyId::eDetectionWindowLeft])
        {
            m_rCHRCommunication.SetDetectionWindow(eDWD_Window_Left, keyValue->value<int>());
        }
        if (keyValue->key() == rKeyList[KeyId::eDetectionWindowRight])
        {
            m_rCHRCommunication.SetDetectionWindow(eDWD_Window_Right, keyValue->value<int>());
        }
        if (keyValue->key() == rKeyList[KeyId::eQualityThreshold])
        {
            m_rCHRCommunication.SetQualityThreshold(keyValue->value<int>());
        }
        if (keyValue->key() == rKeyList[KeyId::eDataAveraging])
        {
            m_rCHRCommunication.SetDataAveraging(keyValue->value<int>());
        }
        if (keyValue->key() == rKeyList[KeyId::eSpectralAveraging])
        {
            m_rCHRCommunication.SetSpectralAveraging(keyValue->value<int>());
        }
        if (keyValue->key() == rKeyList[KeyId::eNumProfilesToSend])
        {
            m_rCHRCommunication.SetMaxNumProfilesToSend(keyValue->value<int>());
        }
        if (keyValue->key() == rKeyList[KeyId::eNumBufferLines])
        {
            m_rCHRCommunication.SetMaxBufferLines(keyValue->value<int>());
        }
        if (keyValue->key() == "Rescale CHR Results")
        {
            m_rCHRCommunication.SetRescaleFactorCHRResults(keyValue->value<int>());
        }
        if (keyValue->key() == rKeyList[KeyId::eWeldingDepthSystemOffset])
        {
            m_rCHRCommunication.SetWeldingDepthSystemOffset(keyValue->value<int>());
        }
        if (keyValue->key() == "Adaptive Exposure Basic Value")
        {
            m_rCHRCommunication.SetAdaptiveExposureBasicValue(keyValue->value<int>());
        }
#if DEBUG_DEVICESERVER
        printf("set %s: %d\n", keyValue->key().c_str(), keyValue->value<int>());
#endif
        break;

    case TDouble:

#if DEBUG_DEVICESERVER
        printf("set %s: %f\n", keyValue->key().c_str(), keyValue->value<double>());
#endif

    case TString:
        if (keyValue->key() == "Direct CHR command")
        {
            m_rCHRCommunication.SetDirectCHRCommand(keyValue->value<PvString>());
        }

#if DEBUG_DEVICESERVER
        printf("set %s: %s\n", keyValue->key().c_str(), keyValue->value<PvString>());
#endif
        break;

    default:
        break;
    }

    // CHRCommunication knows wheter to save or not (m_oConfigurationModified)
    m_rCHRCommunication.saveConfigurationToFile();

    return KeyHandle();
}

void DeviceServer::set(Configuration config, int subDevice)
{
}

SmpKeyValue DeviceServer::get(Key key, int subDevice)
{
#if DEBUG_DEVICESERVER
    printf("get %s\n", key.c_str());
#endif

    // fist search the parameters managed by OCTDeviceConfiguration

    auto res = m_rCHRCommunication.getKeyValue(key);
    if (!res.isNull())
    {
        return res;
    }

    //if the key was not found, it's a command
    if (key == "Results On/Off")
    {
        return SmpKeyValue(new TKeyValue<bool>("Results On/Off", m_rCHRCommunication.GetResultsOnOff(), false, true, false));
    }
    if (key == "Ask Version String")
    {
        return SmpKeyValue(new TKeyValue<bool>("Ask Version String", false, false, true, false));
    }
    if (key == "Download FFT Spectrum")
    {
        return SmpKeyValue(new TKeyValue<bool>("Download FFT Spectrum", false, false, true, false));
    }
    if (key == "Perform Dark Reference")
    {
        return SmpKeyValue(new TKeyValue<bool>("Perform Dark Reference", false, false, true, false));
    }
    if (key == "SLD Dimmer On/Off")
    {
        return SmpKeyValue(new TKeyValue<bool>("SLD Dimmer On/Off", m_rCHRCommunication.GetSLDDimmerOnOff(), false, true, false));
    }
    if (key == "Adaptive Exposure Mode On/Off")
    {
        return SmpKeyValue(new TKeyValue<bool>("Adaptive Exposure Mode On/Off", m_rCHRCommunication.GetAdaptiveExposureOnOff(), false, true, false));
    }
    if (key == "Direct CHR command")
    {
        return SmpKeyValue(new TKeyValue<PvString>("Direct CHR command", "", "", "", ""));
    }
    if (key == "Debug Info CHR Response")
    {
        return SmpKeyValue(new TKeyValue<bool>("Debug Info CHR Response", m_rCHRCommunication.GetDebugInfoCHRResponseOnOff(), false, true, false));
    }
    if (key == "TestFunction 2")
    {
        return SmpKeyValue(new TKeyValue<bool>("TestFunction 2", false, false, true, false));
    }
    if (key == "TestFunction 3")
    {
        return SmpKeyValue(new TKeyValue<bool>("TestFunction 3", false, false, true, false));
    }
    if (key == "TestFunction 4")
    {
        return SmpKeyValue(new TKeyValue<bool>("TestFunction 4", false, false, true, false));
    }
    if (key == "TestFunction 5")
    {
        return SmpKeyValue(new TKeyValue<bool>("TestFunction 5", false, false, true, false));
    }
    if (key == "TestFunction 6")
    {
        return SmpKeyValue(new TKeyValue<bool>("TestFunction 6", false, false, true, false));
    }

    if ( key == "Scale (SCA)")
    {
        return SmpKeyValue(new TKeyValue<int>( "Scale (SCA)", m_rCHRCommunication.getScale(), 1, 32768, 512 ) );
    }
    if (key == "Rescale CHR Results")
    {
        return SmpKeyValue(new TKeyValue<int>("Rescale CHR Results", m_rCHRCommunication.GetRescaleFactorCHRResults(), 1, 50, 10));
    }
    if (key == "Adaptive Exposure Basic Value")
    {
        return SmpKeyValue(new TKeyValue<int>("Adaptive Exposure Basic Value", m_rCHRCommunication.GetAdaptiveExposureBasicValue(), 0, 100, 50));
    }

    return nullptr;
}

SmpKeyValue DeviceServer::get(KeyHandle handle, int subDevice)
{
    return nullptr;
}

Configuration DeviceServer::get(int subDevice)
{

    //parameters
    Configuration config = m_rCHRCommunication.makeConfiguration();

    //commands
    config.push_back(SmpKeyValue(new TKeyValue<bool>("Results On/Off", m_rCHRCommunication.GetResultsOnOff(), false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("Ask Version String", false, false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("Download FFT Spectrum", false, false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("Perform Dark Reference", false, false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("SLD Dimmer On/Off", m_rCHRCommunication.GetSLDDimmerOnOff(), false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("Adaptive Exposure Mode On/Off", m_rCHRCommunication.GetAdaptiveExposureOnOff(), false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<int>("Adaptive Exposure Basic Value", m_rCHRCommunication.GetAdaptiveExposureBasicValue(), 0, 100, 50)));
    config.push_back( SmpKeyValue( new TKeyValue<int>( "Scale (SCA)", m_rCHRCommunication.getScale(), 1, 32768, 512 ) ) );

    config.push_back(SmpKeyValue(new TKeyValue<PvString>("Direct CHR command", "", "", "", "")));

    config.push_back(SmpKeyValue(new TKeyValue<bool>("Debug Info CHR Response", m_rCHRCommunication.GetDebugInfoCHRResponseOnOff(), false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("TestFunction 2", false, false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("TestFunction 3", false, false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("TestFunction 4", false, false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("TestFunction 5", false, false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<bool>("TestFunction 6", false, false, true, false)));
    config.push_back(SmpKeyValue(new TKeyValue<int>("Rescale CHR Results", m_rCHRCommunication.GetRescaleFactorCHRResults(), 1, 50, 10)));

    std::cout << config << std::endl;

    return config;
}

} // namespace grabber

} // namespace precitec
