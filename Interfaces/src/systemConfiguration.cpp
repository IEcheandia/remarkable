#include "common/systemConfiguration.h"

#if !defined(CONFIG_LEAN)
#include "module/moduleLogger.h"
#endif

#include <Poco/File.h>
#include <Poco/Util/XMLConfiguration.h>

#include <fstream>
#include <iostream>
#include <array>

namespace precitec
{
namespace interface
{
static const std::string SysConfFileName("SystemConfig.xml");

static const std::array<std::string, std::size_t(SystemConfiguration::BooleanKey::KeyCount)> s_booleanKeys{
    "ImageMirrorActive",
    "ImageMirrorYActive",
    "HasCamera",
    "LineLaser1Enable",
    "LineLaser1_OutputViaCamera",
    "LineLaser2Enable",
    "LineLaser2_OutputViaCamera",
    "FieldLight1Enable",
    "FieldLight1_OutputViaCamera",
    "LED_IlluminationEnable",
    "ImageTriggerViaEncoderSignals",
    "ImageTriggerViaCameraEncoder",
    "ZCollimatorEnable",
    "ZCollAutoHomingEnable",
    "LiquidLensControllerEnable",
    "ScanTrackerEnable",
    "ScanTrackerSerialViaOnboardPort",
    "LaserControlEnable",
    "LaserControlTwoChannel",
    "Scanner2DEnable",
    "LaserPowerInputEnable",
    "EncoderInput1Enable",
    "EncoderInput2Enable",
    "RobotTrackSpeedEnable",
    "AxisXEnable",
    "AxisYEnable",
    "AxisZEnable",
    "ForceHomingOfAxis",
    "IDM_Device1Enable",
    "OCT_with_reference_arms",
    "CLS2_Device1Enable",
    "Newson_Scanner1Enable",
    "HeadMonitorGatewayEnable",
    "HeadMonitorSendsNotReady",
    "LWM40_No1_Enable",
    "Communication_To_LWM_Device_Enable",
    "FastAnalogSignal1Enable",
    "FastAnalogSignal2Enable",
    "FastAnalogSignal3Enable",
    "FastAnalogSignal4Enable",
    "FastAnalogSignal5Enable",
    "FastAnalogSignal6Enable",
    "FastAnalogSignal7Enable",
    "FastAnalogSignal8Enable",
    "EmergencyStopSignalEnable",
    "CabinetTemperatureOkEnable",
    "GenPurposeAnalogIn1Enable",
    "GenPurposeAnalogIn2Enable",
    "GenPurposeAnalogIn3Enable",
    "GenPurposeAnalogIn4Enable",
    "GenPurposeAnalogIn5Enable",
    "GenPurposeAnalogIn6Enable",
    "GenPurposeAnalogIn7Enable",
    "GenPurposeAnalogIn8Enable",
    "GenPurposeAnalogOut1Enable",
    "GenPurposeAnalogOut2Enable",
    "GenPurposeAnalogOut3Enable",
    "GenPurposeAnalogOut4Enable",
    "GenPurposeAnalogOut5Enable",
    "GenPurposeAnalogOut6Enable",
    "GenPurposeAnalogOut7Enable",
    "GenPurposeAnalogOut8Enable",
    "GenPurposeDigOut1Enable",
    "SeamEndDigOut1Enable",
    "GenPurposeDigOutMultipleEnable",
    "GenPurposeDigIn1Enable",
    "GenPurposeDigInMultipleEnable",
    "ProductNumberFromWMEnable",
    "ContinuouslyModeActive",
    "EtherCATMasterIsActive",
    "FieldbusViaSeparateFieldbusBoard",
    "FieldbusTwoSeparateFieldbusBoards",
    "FieldbusInterfaceStandard",
    "FieldbusInterfaceLight",
    "FieldbusInterfaceFull",
    "FieldbusExtendedProductInfo",
    "ProductTypeOutOfInterfaceFull",
    "ProductNumberOutOfInterfaceFull",
    "QualityErrorFieldOnSeamSeries",
    "NIOResultSwitchedOff",
    "HardwareCameraEnabled",
    "SOUVIS6000_Application",
    "SOUVIS6000_Is_PreInspection",
    "SOUVIS6000_Is_PostInspection_Top",
    "SOUVIS6000_Is_PostInspection_Bottom",
    "SOUVIS6000_TCPIP_Communication_On",
    "SOUVIS6000_Automatic_Seam_No",
    "SOUVIS6000_CrossSectionMeasurementEnable",
    "SOUVIS6000_CrossSection_Leading_System",
    "SCANMASTER_Application",
    "SCANMASTER_ThreeStepInterface",
    "SCANMASTER_GeneralApplication",
};

static const std::array<std::string, std::size_t(SystemConfiguration::IntKey::KeyCount)> s_intKeys{
    "Type_of_Sensor",
    "LineLaser_CameraCommandSet",
    "LED_CONTROLLER_TYPE",
    "LED_MaxCurrent_mA_Chan1",
    "LED_MaxCurrent_mA_Chan2",
    "LED_MaxCurrent_mA_Chan3",
    "LED_MaxCurrent_mA_Chan4",
    "LED_MaxCurrent_mA_Chan5",
    "LED_MaxCurrent_mA_Chan6",
    "LED_MaxCurrent_mA_Chan7",
    "LED_MaxCurrent_mA_Chan8",
    "CameraEncoderBurstFactor",
    "ZCollimatorType",
    "ScannerModel",
    "ScannerGeneralMode",
    "FieldbusBoard_TypeOfFieldbus",
    "SOUVIS6000_Machine_Type",
    "CameraInterfaceType",
    "ScanlabScanner_Lens_Type",
    "Maximum_Simulated_Laser_Power",
    "Scanner2DController",
    "LWM_Device_TCP_Port",
};

static const  std::array<std::string, std::size_t(SystemConfiguration::StringKey::KeyCount)> s_stringKeys{
    "ZCollimator_IP_Address",
    "LiquidLensController_IP_Address",
    "Scanner2DController_IP_Address",
    "LWM_Device_IP_Address",
    "IDM_Device1_IP_Address",
    "CLS2_Device1_IP_Address",
    "SOUVIS6000_IP_Address_MachineControl",
    "SOUVIS6000_IP_Address_CrossSection_Other",
    "FieldbusBoard_IP_Address",
    "FieldbusBoard_2_IP_Address",
    "FieldbusBoard_Netmask",
    "FieldbusBoard_2_Netmask",
    "ScanTrackerSerialOnboardPort",
    "ScanlabScanner_Correction_File",
};

static const std::map<SystemConfiguration::BooleanKey, bool> s_booleanDefaults{
    {SystemConfiguration::BooleanKey::ImageMirrorActive, false},
    {SystemConfiguration::BooleanKey::ImageMirrorYActive, false},
    {SystemConfiguration::BooleanKey::HasCamera, true},
    {SystemConfiguration::BooleanKey::LineLaser1Enable, false},
    {SystemConfiguration::BooleanKey::LineLaser1_OutputViaCamera, false},
    {SystemConfiguration::BooleanKey::LineLaser2Enable, false},
    {SystemConfiguration::BooleanKey::LineLaser2_OutputViaCamera, false},
    {SystemConfiguration::BooleanKey::FieldLight1Enable, false},
    {SystemConfiguration::BooleanKey::FieldLight1_OutputViaCamera, false},
    {SystemConfiguration::BooleanKey::LED_IlluminationEnable, false},
    {SystemConfiguration::BooleanKey::ImageTriggerViaEncoderSignals, false},
    {SystemConfiguration::BooleanKey::ImageTriggerViaCameraEncoder, false},
    {SystemConfiguration::BooleanKey::ZCollimatorEnable, false},
    {SystemConfiguration::BooleanKey::ZCollAutoHomingEnable, false},
    {SystemConfiguration::BooleanKey::LiquidLensControllerEnable, false},
    {SystemConfiguration::BooleanKey::ScanTrackerEnable, false},
    {SystemConfiguration::BooleanKey::ScanTrackerSerialViaOnboardPort, false},
    {SystemConfiguration::BooleanKey::LaserControlEnable, false},
    {SystemConfiguration::BooleanKey::LaserControlTwoChannel, false},
    {SystemConfiguration::BooleanKey::Scanner2DEnable, false},
    {SystemConfiguration::BooleanKey::LaserPowerInputEnable, false},
    {SystemConfiguration::BooleanKey::EncoderInput1Enable, false},
    {SystemConfiguration::BooleanKey::EncoderInput2Enable, false},
    {SystemConfiguration::BooleanKey::RobotTrackSpeedEnable, false},
    {SystemConfiguration::BooleanKey::AxisXEnable, false},
    {SystemConfiguration::BooleanKey::AxisYEnable, false},
    {SystemConfiguration::BooleanKey::AxisZEnable, false},
    {SystemConfiguration::BooleanKey::ForceHomingOfAxis, true},
    {SystemConfiguration::BooleanKey::IDM_Device1Enable, false},
    {SystemConfiguration::BooleanKey::OCT_with_reference_arms, false},
    {SystemConfiguration::BooleanKey::CLS2_Device1Enable, false},
    {SystemConfiguration::BooleanKey::Newson_Scanner1Enable, false},
    {SystemConfiguration::BooleanKey::HeadMonitorGatewayEnable, false},
    {SystemConfiguration::BooleanKey::HeadMonitorSendsNotReady, false},
    {SystemConfiguration::BooleanKey::LWM40_No1_Enable, false},
    {SystemConfiguration::BooleanKey::Communication_To_LWM_Device_Enable, false},
    {SystemConfiguration::BooleanKey::FastAnalogSignal1Enable, false},
    {SystemConfiguration::BooleanKey::FastAnalogSignal2Enable, false},
    {SystemConfiguration::BooleanKey::FastAnalogSignal3Enable, false},
    {SystemConfiguration::BooleanKey::FastAnalogSignal4Enable, false},
    {SystemConfiguration::BooleanKey::FastAnalogSignal5Enable, false},
    {SystemConfiguration::BooleanKey::FastAnalogSignal6Enable, false},
    {SystemConfiguration::BooleanKey::FastAnalogSignal7Enable, false},
    {SystemConfiguration::BooleanKey::FastAnalogSignal8Enable, false},
    {SystemConfiguration::BooleanKey::EmergencyStopSignalEnable, false},
    {SystemConfiguration::BooleanKey::CabinetTemperatureOkEnable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogIn1Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogIn2Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogIn3Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogIn4Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogIn5Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogIn6Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogIn7Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogIn8Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogOut1Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogOut2Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogOut3Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogOut4Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogOut5Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogOut6Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogOut7Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeAnalogOut8Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeDigOut1Enable, false},
    {SystemConfiguration::BooleanKey::SeamEndDigOut1Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeDigOutMultipleEnable, false},
    {SystemConfiguration::BooleanKey::GenPurposeDigIn1Enable, false},
    {SystemConfiguration::BooleanKey::GenPurposeDigInMultipleEnable, false},
    {SystemConfiguration::BooleanKey::ProductNumberFromWMEnable, false},
    {SystemConfiguration::BooleanKey::ContinuouslyModeActive, false},
    {SystemConfiguration::BooleanKey::EtherCATMasterIsActive, true},
    {SystemConfiguration::BooleanKey::FieldbusViaSeparateFieldbusBoard, false},
    {SystemConfiguration::BooleanKey::FieldbusTwoSeparateFieldbusBoards, false},
    {SystemConfiguration::BooleanKey::FieldbusInterfaceStandard, false},
    {SystemConfiguration::BooleanKey::FieldbusInterfaceLight, false},
    {SystemConfiguration::BooleanKey::FieldbusInterfaceFull, false},
    {SystemConfiguration::BooleanKey::FieldbusExtendedProductInfo, false},
    {SystemConfiguration::BooleanKey::ProductTypeOutOfInterfaceFull, false},
    {SystemConfiguration::BooleanKey::ProductNumberOutOfInterfaceFull, false},
    {SystemConfiguration::BooleanKey::QualityErrorFieldOnSeamSeries, false},
    {SystemConfiguration::BooleanKey::NIOResultSwitchedOff, false},
    {SystemConfiguration::BooleanKey::HardwareCameraEnabled, true},
    {SystemConfiguration::BooleanKey::SOUVIS6000_Application, false},
    {SystemConfiguration::BooleanKey::SOUVIS6000_Is_PreInspection, false},
    {SystemConfiguration::BooleanKey::SOUVIS6000_Is_PostInspection_Top, false},
    {SystemConfiguration::BooleanKey::SOUVIS6000_Is_PostInspection_Bottom, false},
    {SystemConfiguration::BooleanKey::SOUVIS6000_TCPIP_Communication_On, true},
    {SystemConfiguration::BooleanKey::SOUVIS6000_Automatic_Seam_No, false},
    {SystemConfiguration::BooleanKey::SOUVIS6000_CrossSectionMeasurementEnable, false},
    {SystemConfiguration::BooleanKey::SOUVIS6000_CrossSection_Leading_System, false},
    {SystemConfiguration::BooleanKey::SCANMASTER_Application, false},
    {SystemConfiguration::BooleanKey::SCANMASTER_ThreeStepInterface, false},
    {SystemConfiguration::BooleanKey::SCANMASTER_GeneralApplication, false},
};

static const std::map<SystemConfiguration::IntKey, int> s_intDefaults{
    {SystemConfiguration::IntKey::Type_of_Sensor, 0},
    {SystemConfiguration::IntKey::LineLaser_CameraCommandSet, 0},
    {SystemConfiguration::IntKey::LED_CONTROLLER_TYPE, 0},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan1, 1000},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan2, 1000},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan3, 1000},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan4, 1000},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan5, 1000},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan6, 1000},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan7, 1000},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan8, 1000},
    {SystemConfiguration::IntKey::CameraEncoderBurstFactor, 1},
    {SystemConfiguration::IntKey::ZCollimatorType, 1},
    {SystemConfiguration::IntKey::ScannerModel, int(ScannerModel::ScanlabScanner)},
    {SystemConfiguration::IntKey::ScannerGeneralMode, int(ScannerGeneralMode::eScanMasterMode)},
    {SystemConfiguration::IntKey::FieldbusBoard_TypeOfFieldbus, 1},
    {SystemConfiguration::IntKey::SOUVIS6000_Machine_Type, 0},
    {SystemConfiguration::IntKey::CameraInterfaceType, 0},
    {SystemConfiguration::IntKey::ScanlabScanner_Lens_Type, 1},
    {SystemConfiguration::IntKey::Maximum_Simulated_Laser_Power, 4000},
    {SystemConfiguration::IntKey::Scanner2DController, int(ScannerModel::ScanlabScanner)},
    {SystemConfiguration::IntKey::LWM_Device_TCP_Port, 2400},
};

static const std::map<SystemConfiguration::StringKey, std::string> s_stringDefaults{
    {SystemConfiguration::StringKey::ZCollimator_IP_Address, "192.168.170.3"},
    {SystemConfiguration::StringKey::LiquidLensController_IP_Address, "192.168.170.110"},
    {SystemConfiguration::StringKey::Scanner2DController_IP_Address, "192.168.170.105"},
    {SystemConfiguration::StringKey::LWM_Device_IP_Address, "192.168.170.23"},
    {SystemConfiguration::StringKey::IDM_Device1_IP_Address, "192.168.170.2"},
    {SystemConfiguration::StringKey::CLS2_Device1_IP_Address, "192.168.170.3"},
    {SystemConfiguration::StringKey::SOUVIS6000_IP_Address_MachineControl, "127.0.0.1"},
    {SystemConfiguration::StringKey::SOUVIS6000_IP_Address_CrossSection_Other, "127.0.0.1"},
    {SystemConfiguration::StringKey::FieldbusBoard_IP_Address, "192.168.10.10"},
    {SystemConfiguration::StringKey::FieldbusBoard_2_IP_Address, "192.168.10.11"},
    {SystemConfiguration::StringKey::FieldbusBoard_Netmask, "255.255.255.0"},
    {SystemConfiguration::StringKey::FieldbusBoard_2_Netmask, "255.255.255.0"},
    {SystemConfiguration::StringKey::ScanTrackerSerialOnboardPort, "ttyS2"},
    {SystemConfiguration::StringKey::ScanlabScanner_Correction_File, ""},
};

namespace
{
std::string configDir()
{
    std::string oHomeDir(getenv("WM_BASE_DIR"));
    return oHomeDir + "/config/";
}

std::string loadFilePath()
{
    std::string oSysConfFileName = configDir() + SysConfFileName;
    if (!Poco::File{oSysConfFileName}.exists())
    {
        std::string oDefaultFile = configDir() + "Default" + SysConfFileName;
        if (Poco::File{oDefaultFile}.exists())
        {
            oSysConfFileName = oDefaultFile;
        }
        else
        {
            std::ofstream newfile;
            newfile.open(oSysConfFileName.c_str());
            if (newfile)
            {
                newfile << "<SystemConfig>" << std::endl;
                newfile << "	<File_New_Created>1</File_New_Created>" << std::endl;
                newfile << "	<Type_of_Sensor>1</Type_of_Sensor>" << std::endl;
                newfile << "</SystemConfig>" << std::endl;
                newfile.close();
            }
        }
    }
    std::cout << "Filename: " << oSysConfFileName << std::endl;
    return oSysConfFileName;
}
}

SystemConfiguration::SystemConfiguration()
    : SystemConfiguration(loadFilePath(), configDir() + SysConfFileName)
{
}

SystemConfiguration::SystemConfiguration(const std::string &loadFilePath, const std::string &saveFilePath)
    : m_saveFilePath(saveFilePath.empty() ? loadFilePath : saveFilePath)
{
    try
    {
        m_configuration = new Poco::Util::XMLConfiguration(loadFilePath);
    } catch (...)
    {
        m_configuration = new Poco::Util::XMLConfiguration();
    }
}

SystemConfiguration::~SystemConfiguration() = default;

SystemConfiguration &SystemConfiguration::instance()
{
    static SystemConfiguration s_instance;
    return s_instance;
}

void SystemConfiguration::save()
{
    std::cout << "Filename: " << m_saveFilePath << std::endl;
    m_configuration->save(m_saveFilePath);
}

std::string SystemConfiguration::getString(const std::string& key, const std::string& defaultValue, bool *found)
{
    try
    {
        if (found)
        {
            *found = true;
        }
        return m_configuration->getString(key);
    }
    catch (Poco::NotFoundException& e)
    {
        if (found)
        {
            *found = false;
        }
        return defaultValue;
    }
    catch (Poco::Exception& e)
    {
        std::cerr << "Poco::Exception: " << e.displayText() << std::endl;
        std::cerr << std::endl;
#if !defined(CONFIG_LEAN)
        wmLogTr(eError, "QnxMsg.Misc.SysConfGetErr1", "Cannot get System Configuration switch %s, default value (%s) is set\n", key.c_str(), defaultValue.c_str());
#endif
        if (found)
        {
            *found = false;
        }
        return defaultValue;
    }
}

bool SystemConfiguration::getBool(const std::string& key, bool defaultValue, bool *found)
{
    try
    {
        if (found)
        {
            *found = true;
        }
        return m_configuration->getBool(key);
    }
    catch (Poco::NotFoundException& e)
    {
        if (found)
        {
            *found = false;
        }
        return defaultValue;
    }
    catch (Poco::Exception& e)
    {
        std::cerr << "Poco::Exception: " << e.displayText() << std::endl;
        std::cerr << std::endl;
#if !defined(CONFIG_LEAN)
        wmLogTr(eError, "QnxMsg.Misc.SysConfGetErr2", "Cannot get System Configuration switch %s, default value (%d) is set\n", key.c_str(), defaultValue);
#endif
        if (found)
        {
            *found = false;
        }
        return defaultValue;
    }
}

int SystemConfiguration::getInt(const std::string& key, int defaultValue, bool *found)
{
    try
    {
        if (found)
        {
            *found = true;
        }
        return m_configuration->getInt(key);
    }
    catch (Poco::NotFoundException& e)
    {
        if (found)
        {
            *found = false;
        }
        return defaultValue;
    }
    catch (Poco::Exception& e)
    {
        std::cerr << "Poco::Exception: " << e.displayText() << std::endl;
        std::cerr << std::endl;
#if !defined(CONFIG_LEAN)
        wmLogTr(eError, "QnxMsg.Misc.SysConfGetErr2", "Cannot get System Configuration switch %s, default value (%d) is set\n", key.c_str(), defaultValue);
#endif
        if (found)
        {
            *found = false;
        }
        return defaultValue;
    }
}

double SystemConfiguration::getDouble(const std::string& key, double defaultValue, bool* found)
{
    try
    {
        if (found)
        {
            *found = true;
        }
        return m_configuration->getDouble(key);
    }
    catch (Poco::NotFoundException& e)
    {
        if (found)
        {
            *found = false;
        }
        return defaultValue;
    }
    catch (Poco::Exception& e)
    {
        std::cerr << "Poco::Exception: " << e.displayText() << std::endl;
        std::cerr << std::endl;
#if !defined(CONFIG_LEAN)
        wmLogTr(eError, "QnxMsg.Misc.SysConfGetErr2", "Cannot get System Configuration switch %s, default value (%d) is set\n", key.c_str(), defaultValue);
#endif
        if (found)
        {
            *found = false;
        }
        return defaultValue;
    }
}

void SystemConfiguration::setString(const std::string& key, std::string value)
{
    try
    {
        m_configuration->setString(key, value);
        save();
    }
    catch (Poco::Exception& e)
    {
        std::cerr << "Poco::Exception: " << e.displayText() << std::endl;
        std::cerr << std::endl;
#if !defined(CONFIG_LEAN)
        wmLogTr(eError, "QnxMsg.Misc.SysConfSetErr1", "Cannot set System Configuration switch %s to value (%s)\n", key.c_str(), value.c_str());
#endif
        return;
    }
}

void SystemConfiguration::setInt(const std::string& key, int value)
{
    try
    {
        m_configuration->setInt(key, value);
        save();
    }
    catch (Poco::Exception& e)
    {
        std::cerr << "Poco::Exception: " << e.displayText() << std::endl;
        std::cerr << std::endl;
#if !defined(CONFIG_LEAN)
        wmLogTr(eError, "QnxMsg.Misc.SysConfSetErr2", "Cannot set System Configuration switch %s to value (%d)\n", key.c_str(), value);
#endif
        return;
    }
}

void SystemConfiguration::setBool(const std::string& key, bool value)
{
    try
    {
        m_configuration->setBool(key, value);
        save();
    }
    catch (Poco::Exception& e)
    {
        std::cerr << "Poco::Exception: " << e.displayText() << std::endl;
        std::cerr << std::endl;
#if !defined(CONFIG_LEAN)
        wmLogTr(eError, "QnxMsg.Misc.SysConfSetErr2", "Cannot set System Configuration switch %s to value (%d)\n", key.c_str(), value);
#endif
        return;
    }
}

void SystemConfiguration::setDouble(const std::string& key, double value)
{
    try
    {
        m_configuration->setDouble(key, value);
        save();
    }
    catch (Poco::Exception& e)
    {
        std::cerr << "Poco::Exception: " << e.displayText() << std::endl;
        std::cerr << std::endl;
#if !defined(CONFIG_LEAN)
        wmLogTr(eError, "QnxMsg.Misc.SysConfSetErr2", "Cannot set System Configuration switch %s to value (%d)\n", key.c_str(), value);
#endif
        return;
    }
}

void SystemConfiguration::toStdOut()
{
    m_configuration->save(std::cout);
}

namespace
{
template <typename T>
std::string keyToStringImpl(T key, const std::array<std::string, std::size_t(T::KeyCount)>& keys)
{
    if (key == T::KeyCount)
    {
        return {};
    }
    return keys.at(std::size_t(key));
}

template <typename T>
T stringToKeyImpl(const std::string& key, const std::array<std::string, std::size_t(T::KeyCount)>& keys)
{
    auto it = std::find(keys.begin(), keys.end(), key);
    if (it == keys.end())
    {
        return T::KeyCount;
    }
    return T(std::distance(keys.begin(), it));
}

template <typename T, typename R>
T getDefaultImpl(R key, const std::map<R, T>& defaults)
{
    auto it = defaults.find(key);
    if (it != defaults.end())
    {
        return it->second;
    }
    return {};
}
}

std::string SystemConfiguration::keyToString(BooleanKey key)
{
    return keyToStringImpl(key, s_booleanKeys);
}

std::string SystemConfiguration::keyToString(IntKey key)
{
    return keyToStringImpl(key, s_intKeys);
}

std::string SystemConfiguration::keyToString(StringKey key)
{
    return keyToStringImpl(key, s_stringKeys);
}

SystemConfiguration::BooleanKey SystemConfiguration::stringToBooleanKey(const std::string& key)
{
    return stringToKeyImpl<SystemConfiguration::BooleanKey>(key, s_booleanKeys);
}

SystemConfiguration::IntKey SystemConfiguration::stringToIntKey(const std::string& key)
{
    return stringToKeyImpl<SystemConfiguration::IntKey>(key, s_intKeys);
}

SystemConfiguration::StringKey SystemConfiguration::stringToStringKey(const std::string& key)
{
    return stringToKeyImpl<SystemConfiguration::StringKey>(key, s_stringKeys);
}

bool SystemConfiguration::get(BooleanKey key)
{
    return getBool(keyToString(key), getDefault(key));
}

int SystemConfiguration::get(IntKey key)
{
    return getInt(keyToString(key), getDefault(key));
}

std::string SystemConfiguration::get(StringKey key)
{
    return getString(keyToString(key), getDefault(key));
}

void SystemConfiguration::set(BooleanKey key, bool value)
{
    setBool(keyToString(key), value);
}

void SystemConfiguration::set(IntKey key, int value)
{
    setInt(keyToString(key), value);
}

void SystemConfiguration::set(StringKey key, const std::string& value)
{
    setString(keyToString(key), value);
}

bool SystemConfiguration::getDefault(BooleanKey key)
{
    return getDefaultImpl(key, s_booleanDefaults);
}

int SystemConfiguration::getDefault(IntKey key)
{
    return getDefaultImpl(key, s_intDefaults);
}

std::string SystemConfiguration::getDefault(StringKey key)
{
    return getDefaultImpl(key, s_stringDefaults);
}

}
}
