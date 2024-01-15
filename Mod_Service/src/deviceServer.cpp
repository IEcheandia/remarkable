/**
 * @file
 * @brief   DeviceServer von VI_Service
 *
 * @author  EA
 * @date    14.02.2014
 * @version 1.0
 */

#include <iostream>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

#include "viService/deviceServer.h"
#include "common/systemConfiguration.h"

#define DEBUG_DEVICESERVER 0

namespace precitec
{

namespace ethercat
{

/**
 * @brief CTOR
 *
 */
DeviceServer::DeviceServer()
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

static const std::vector<SystemConfiguration::StringKey> s_ipAddresses{
    SystemConfiguration::StringKey::ZCollimator_IP_Address,
    SystemConfiguration::StringKey::LiquidLensController_IP_Address,
    SystemConfiguration::StringKey::Scanner2DController_IP_Address,
    SystemConfiguration::StringKey::IDM_Device1_IP_Address,
    SystemConfiguration::StringKey::CLS2_Device1_IP_Address,
    SystemConfiguration::StringKey::SOUVIS6000_IP_Address_MachineControl,
    SystemConfiguration::StringKey::SOUVIS6000_IP_Address_CrossSection_Other,
    SystemConfiguration::StringKey::FieldbusBoard_IP_Address,
    SystemConfiguration::StringKey::FieldbusBoard_2_IP_Address,
    SystemConfiguration::StringKey::FieldbusBoard_Netmask,
    SystemConfiguration::StringKey::FieldbusBoard_2_Netmask,
};


struct IntDefault
{
    IntDefault(int minValue, int maxValue)
        : minValue(minValue)
        , maxValue(maxValue)
    {
    }
    int minValue;
    int maxValue;
};

static const std::map<SystemConfiguration::IntKey, IntDefault> s_intKeyValues{
    {SystemConfiguration::IntKey::Type_of_Sensor, {0, 10}},
    {SystemConfiguration::IntKey::LineLaser_CameraCommandSet, { 0, 1}},
    {SystemConfiguration::IntKey::LED_CONTROLLER_TYPE, {0, 10}},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan1, {0, 8000}},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan2, {0, 8000}},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan3, {0, 8000}},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan4, {0, 8000}},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan5, {0, 8000}},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan6, {0, 8000}},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan7, {0, 8000}},
    {SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan8, {0, 8000}},
    {SystemConfiguration::IntKey::CameraEncoderBurstFactor, {1, 100}},
    {SystemConfiguration::IntKey::ZCollimatorType, {1, 2}},
    {SystemConfiguration::IntKey::ScannerModel, {int(ScannerModel::ScanlabScanner), int(ScannerModel::SmartMoveScanner)}},
    {SystemConfiguration::IntKey::ScannerGeneralMode, {int(ScannerGeneralMode::eScanMasterMode), int(ScannerGeneralMode::eScantracker2DMode)}},
    {SystemConfiguration::IntKey::FieldbusBoard_TypeOfFieldbus, {1, 3}},
    {SystemConfiguration::IntKey::SOUVIS6000_Machine_Type, {0, 2}},
    {SystemConfiguration::IntKey::CameraInterfaceType, {0, 1}},
    {SystemConfiguration::IntKey::ScanlabScanner_Lens_Type, {1, 3}},
    {SystemConfiguration::IntKey::Maximum_Simulated_Laser_Power, {1, 15000}},
    {SystemConfiguration::IntKey::Scanner2DController, {int(ScannerModel::ScanlabScanner), int(ScannerModel::SmartMoveScanner)}},
};

KeyHandle DeviceServer::set(SmpKeyValue keyValue, int subDevice)
{
	ValueTypes keyType = keyValue->type();

	switch( keyType )
	{
		case TBool:
            if (const auto key = SystemConfiguration::stringToBooleanKey(keyValue->key()); key != SystemConfiguration::BooleanKey::KeyCount)
            {
                SystemConfiguration::instance().set(key, keyValue->value<bool>());
            }

#if DEBUG_DEVICESERVER
			printf("set %s: %d\n", keyValue->key().c_str(), keyValue->value<bool>());
#endif
			break;

		case TInt:
            if (const auto key = SystemConfiguration::stringToIntKey(keyValue->key()); key != SystemConfiguration::IntKey::KeyCount)
            {
                SystemConfiguration::instance().setInt(keyValue->key(), keyValue->value<int>());
            }

#if DEBUG_DEVICESERVER
			printf("set %s: %d\n", keyValue->key().c_str(), keyValue->value<int>());
#endif
			break;

        case TString:
            if (const auto key = SystemConfiguration::stringToStringKey(keyValue->key()); std::find(s_ipAddresses.begin(), s_ipAddresses.end(), key) != s_ipAddresses.end())
            {
                uint32_t oDummy;
                int oRetValue = inet_pton(AF_INET, keyValue->value<PvString>().c_str(), &oDummy);
                if (oRetValue != 1)
                {
                    wmLog(eError, "wrong format of <%s>\n", keyValue->key());
                    SystemConfiguration::instance().setString(keyValue->key(), SystemConfiguration::getDefault(key));
                }
                else
                {
                    SystemConfiguration::instance().setString(keyValue->key(), keyValue->value<PvString>());
                }
            }
            if (const auto key = SystemConfiguration::stringToStringKey(keyValue->key()); key != SystemConfiguration::StringKey::KeyCount)
            {
                const auto value = keyValue->value<PvString>();
                if (value.empty())
                {
                    wmLog(eDebug, "empty entry in <%s>\n", keyValue->key());
                }
                SystemConfiguration::instance().setString(keyValue->key(), value);
            }
#if DEBUG_DEVICESERVER
            printf("set %s: %s\n", keyValue->key().c_str(), keyValue->value<PvString>().c_str());
#endif
            break;

		default:
			break;
	}
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

    if (const auto systemKey = SystemConfiguration::stringToBooleanKey(key); systemKey != SystemConfiguration::BooleanKey::KeyCount)
    {
        return SmpKeyValue(new TKeyValue<bool>(key, SystemConfiguration::instance().get(systemKey), false, true, SystemConfiguration::getDefault(systemKey)) );
    }
    if (const auto systemKey = SystemConfiguration::stringToStringKey(key); systemKey != SystemConfiguration::StringKey::KeyCount)
    {
        return SmpKeyValue(new TKeyValue<PvString>(key, SystemConfiguration::instance().get(systemKey), "", "", SystemConfiguration::getDefault(systemKey)));
    }
    if (const auto systemKey = SystemConfiguration::stringToIntKey(key); systemKey != SystemConfiguration::IntKey::KeyCount)
    {
        if (auto it = s_intKeyValues.find(systemKey); it != s_intKeyValues.end())
        {
            return SmpKeyValue(new TKeyValue<int>(key, SystemConfiguration::instance().get(systemKey), it->second.minValue, it->second.maxValue, SystemConfiguration::getDefault(systemKey)) );
        }
    }

	return NULL;
}

SmpKeyValue DeviceServer::get(KeyHandle handle, int subDevice)
{
	return NULL;
}

Configuration DeviceServer::get(int subDevice)
{
	Configuration config;

    auto addBoolean = [&config] (SystemConfiguration::BooleanKey key)
    {
        const auto name = SystemConfiguration::keyToString(key);
        config.emplace_back(new TKeyValue<bool>{name, SystemConfiguration::instance().get(key), false, true, SystemConfiguration::getDefault(key)});
    };
    auto addString = [&config] (SystemConfiguration::StringKey key)
    {
        const auto name = SystemConfiguration::keyToString(key);
        config.emplace_back(new TKeyValue<PvString>{name, SystemConfiguration::instance().get(key), "", "", SystemConfiguration::getDefault(key)});
    };
    auto addInt = [&config] (SystemConfiguration::IntKey key)
    {
        if (auto it = s_intKeyValues.find(key); it != s_intKeyValues.end())
        {
            const auto name = SystemConfiguration::keyToString(key);
            config.emplace_back(new TKeyValue<int>{name, SystemConfiguration::instance().get(key), it->second.minValue, it->second.maxValue, SystemConfiguration::getDefault(key)});
        }
    };

    addInt(SystemConfiguration::IntKey::Type_of_Sensor);
	addBoolean(SystemConfiguration::BooleanKey::ImageMirrorActive);
	addBoolean(SystemConfiguration::BooleanKey::ImageMirrorYActive);
	addBoolean(SystemConfiguration::BooleanKey::HasCamera);
	addBoolean(SystemConfiguration::BooleanKey::LineLaser1Enable);
	addBoolean(SystemConfiguration::BooleanKey::LineLaser1_OutputViaCamera);
	addBoolean(SystemConfiguration::BooleanKey::LineLaser2Enable);
	addBoolean(SystemConfiguration::BooleanKey::LineLaser2_OutputViaCamera);
	addBoolean(SystemConfiguration::BooleanKey::FieldLight1Enable);
	addBoolean(SystemConfiguration::BooleanKey::FieldLight1_OutputViaCamera);
    addInt(SystemConfiguration::IntKey::LineLaser_CameraCommandSet);
	addBoolean(SystemConfiguration::BooleanKey::LED_IlluminationEnable);
    addInt(SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan1);
    addInt(SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan2);
    addInt(SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan3);
    addInt(SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan4);
    addInt(SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan5);
    addInt(SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan6);
    addInt(SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan7);
    addInt(SystemConfiguration::IntKey::LED_MaxCurrent_mA_Chan8);
	addBoolean(SystemConfiguration::BooleanKey::ImageTriggerViaEncoderSignals);
	addBoolean(SystemConfiguration::BooleanKey::ImageTriggerViaCameraEncoder);
    addInt(SystemConfiguration::IntKey::CameraEncoderBurstFactor);
	addBoolean(SystemConfiguration::BooleanKey::ZCollimatorEnable);
    addInt(SystemConfiguration::IntKey::ZCollimatorType);
    addString(SystemConfiguration::StringKey::ZCollimator_IP_Address);
	addBoolean(SystemConfiguration::BooleanKey::ZCollAutoHomingEnable);
    addBoolean(SystemConfiguration::BooleanKey::LiquidLensControllerEnable);
    addString(SystemConfiguration::StringKey::LiquidLensController_IP_Address);
	addBoolean(SystemConfiguration::BooleanKey::ScanTrackerEnable);
	addBoolean(SystemConfiguration::BooleanKey::ScanTrackerSerialViaOnboardPort);
    addString(SystemConfiguration::StringKey::ScanTrackerSerialOnboardPort);
	addBoolean(SystemConfiguration::BooleanKey::LaserControlEnable);
	addBoolean(SystemConfiguration::BooleanKey::LaserControlTwoChannel);
	addBoolean(SystemConfiguration::BooleanKey::Scanner2DEnable);
    addString(SystemConfiguration::StringKey::Scanner2DController_IP_Address );
    addInt(SystemConfiguration::IntKey::ScannerModel);
    addInt(SystemConfiguration::IntKey::Scanner2DController);
    addInt(SystemConfiguration::IntKey::ScannerGeneralMode);
    addInt(SystemConfiguration::IntKey::ScanlabScanner_Lens_Type);
    addString(SystemConfiguration::StringKey::ScanlabScanner_Correction_File);
	addBoolean(SystemConfiguration::BooleanKey::LaserPowerInputEnable);
    addInt(SystemConfiguration::IntKey::Maximum_Simulated_Laser_Power);
	addBoolean(SystemConfiguration::BooleanKey::EncoderInput1Enable);
	addBoolean(SystemConfiguration::BooleanKey::EncoderInput2Enable);
	addBoolean(SystemConfiguration::BooleanKey::RobotTrackSpeedEnable);
	// this key is currently unvisible to the user (AxisX is not implemented)
	//addBoolean(SystemConfiguration::BooleanKey::AxisXEnable);
	addBoolean(SystemConfiguration::BooleanKey::AxisYEnable);
	// this key is currently unvisible to the user (AxisZ is not totally implemented)
	//addBoolean(SystemConfiguration::BooleanKey::AxisZEnable);
	addBoolean(SystemConfiguration::BooleanKey::ForceHomingOfAxis);
	addBoolean(SystemConfiguration::BooleanKey::IDM_Device1Enable);
	addBoolean(SystemConfiguration::BooleanKey::OCT_with_reference_arms);
    addString(SystemConfiguration::StringKey::IDM_Device1_IP_Address);
	addBoolean(SystemConfiguration::BooleanKey::CLS2_Device1Enable);
    addString(SystemConfiguration::StringKey::CLS2_Device1_IP_Address);
	addBoolean(SystemConfiguration::BooleanKey::Newson_Scanner1Enable);
	addBoolean(SystemConfiguration::BooleanKey::HeadMonitorGatewayEnable);
	addBoolean(SystemConfiguration::BooleanKey::HeadMonitorSendsNotReady);
	addBoolean(SystemConfiguration::BooleanKey::LWM40_No1_Enable);
    addBoolean(SystemConfiguration::BooleanKey::Communication_To_LWM_Device_Enable);
    addString(SystemConfiguration::StringKey::LWM_Device_IP_Address);
    addInt(SystemConfiguration::IntKey::LWM_Device_TCP_Port);
	addBoolean(SystemConfiguration::BooleanKey::FastAnalogSignal1Enable);
	addBoolean(SystemConfiguration::BooleanKey::FastAnalogSignal2Enable);
	addBoolean(SystemConfiguration::BooleanKey::FastAnalogSignal3Enable);
	addBoolean(SystemConfiguration::BooleanKey::FastAnalogSignal4Enable);
	addBoolean(SystemConfiguration::BooleanKey::FastAnalogSignal5Enable);
	addBoolean(SystemConfiguration::BooleanKey::FastAnalogSignal6Enable);
	addBoolean(SystemConfiguration::BooleanKey::FastAnalogSignal7Enable);
	addBoolean(SystemConfiguration::BooleanKey::FastAnalogSignal8Enable);
	addBoolean(SystemConfiguration::BooleanKey::EmergencyStopSignalEnable);
	addBoolean(SystemConfiguration::BooleanKey::CabinetTemperatureOkEnable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogIn1Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogIn2Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogIn3Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogIn4Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogIn5Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogIn6Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogIn7Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogIn8Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogOut1Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogOut2Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogOut3Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogOut4Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogOut5Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogOut6Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogOut7Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeAnalogOut8Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeDigOut1Enable);
	addBoolean(SystemConfiguration::BooleanKey::SeamEndDigOut1Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeDigOutMultipleEnable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeDigIn1Enable);
	addBoolean(SystemConfiguration::BooleanKey::GenPurposeDigInMultipleEnable);
	addBoolean(SystemConfiguration::BooleanKey::ProductNumberFromWMEnable);
	addBoolean(SystemConfiguration::BooleanKey::ContinuouslyModeActive);
	addBoolean(SystemConfiguration::BooleanKey::EtherCATMasterIsActive);
	addBoolean(SystemConfiguration::BooleanKey::FieldbusViaSeparateFieldbusBoard);
	addBoolean(SystemConfiguration::BooleanKey::FieldbusTwoSeparateFieldbusBoards);
    addInt(SystemConfiguration::IntKey::FieldbusBoard_TypeOfFieldbus);
    addString(SystemConfiguration::StringKey::FieldbusBoard_IP_Address);
    addString(SystemConfiguration::StringKey::FieldbusBoard_2_IP_Address);
    addString(SystemConfiguration::StringKey::FieldbusBoard_Netmask);
    addString(SystemConfiguration::StringKey::FieldbusBoard_2_Netmask);
	addBoolean(SystemConfiguration::BooleanKey::FieldbusInterfaceStandard);
	addBoolean(SystemConfiguration::BooleanKey::FieldbusInterfaceLight);
	addBoolean(SystemConfiguration::BooleanKey::FieldbusInterfaceFull);
	addBoolean(SystemConfiguration::BooleanKey::FieldbusExtendedProductInfo);
	addBoolean(SystemConfiguration::BooleanKey::ProductTypeOutOfInterfaceFull);
	addBoolean(SystemConfiguration::BooleanKey::ProductNumberOutOfInterfaceFull);
	addBoolean(SystemConfiguration::BooleanKey::QualityErrorFieldOnSeamSeries);
	addBoolean(SystemConfiguration::BooleanKey::NIOResultSwitchedOff);
	addBoolean(SystemConfiguration::BooleanKey::HardwareCameraEnabled);
    addInt(SystemConfiguration::IntKey::CameraInterfaceType);
	addBoolean(SystemConfiguration::BooleanKey::SOUVIS6000_Application);
    addInt(SystemConfiguration::IntKey::SOUVIS6000_Machine_Type);
	addBoolean(SystemConfiguration::BooleanKey::SOUVIS6000_Is_PreInspection);
	addBoolean(SystemConfiguration::BooleanKey::SOUVIS6000_Is_PostInspection_Top);
	addBoolean(SystemConfiguration::BooleanKey::SOUVIS6000_Is_PostInspection_Bottom);
	addBoolean(SystemConfiguration::BooleanKey::SOUVIS6000_TCPIP_Communication_On);
	addBoolean(SystemConfiguration::BooleanKey::SOUVIS6000_Automatic_Seam_No);
	addBoolean(SystemConfiguration::BooleanKey::SOUVIS6000_CrossSectionMeasurementEnable);
	addBoolean(SystemConfiguration::BooleanKey::SOUVIS6000_CrossSection_Leading_System);
    addString(SystemConfiguration::StringKey::SOUVIS6000_IP_Address_MachineControl);
    addString(SystemConfiguration::StringKey::SOUVIS6000_IP_Address_CrossSection_Other);
	addBoolean(SystemConfiguration::BooleanKey::SCANMASTER_Application);
	addBoolean(SystemConfiguration::BooleanKey::SCANMASTER_ThreeStepInterface);
	addBoolean(SystemConfiguration::BooleanKey::SCANMASTER_GeneralApplication);

std::cout << config << std::endl;

	return config;
}

} // namespace ethercat

} // namespace precitec

