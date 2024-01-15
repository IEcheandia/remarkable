/**
 * @file
 * @brief   DeviceServer von VI_InspectionControl
 *
 * @author  AL. EA
 * @date    16.8.2018
 * @version 1.0
 */

#include "viInspectionControl/deviceServer.h"

#define DEBUG_DEVICESERVER 0

namespace precitec
{

namespace ethercat
{

/**
 * @brief CTOR
 *
 */
DeviceServer::DeviceServer(VI_InspectionControl& p_rInspectionControl) :
        m_rInspectionControl(p_rInspectionControl)
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
    ValueTypes keyType = keyValue->type();

    switch( keyType )
    {
        case TBool:
            if (keyValue->key() == "LWM_Inspection_Active")
            {
                m_rInspectionControl.setLWMInspectionActive(keyValue->value<bool>());
            }

            if ( keyValue->key() == "DebugInfo_SOURING" )
            {
                m_rInspectionControl.setDebugInfo_SOURING(keyValue->value<bool>());
            }
            if ( keyValue->key() == "DebugInfo_SOURING_Extd" )
            {
                m_rInspectionControl.setDebugInfo_SOURING_Extd(keyValue->value<bool>());
            }
            if ( keyValue->key() == "DebugInfo_SCANMASTER" )
            {
                m_rInspectionControl.setDebugInfo_SCANMASTER(keyValue->value<bool>());
            }

#if DEBUG_DEVICESERVER
            printf("set %s: %d\n", keyValue->key().c_str(), keyValue->value<bool>());
#endif
            break;

        case TInt:
            if (keyValue->key() == "LWM_Program_Number")
            {
                m_rInspectionControl.setLWMProgramNumber(keyValue->value<int>());
            }

#if DEBUG_DEVICESERVER
            printf("set %s: %d\n", keyValue->key().c_str(), keyValue->value<int>());
#endif
            break;

        case TFloat:
            if ( keyValue->key() == "Analog_Trigger_Level" )
            {
                m_rInspectionControl.setAnalogTriggerLevelVolt(keyValue->value<float>());
            }

#if DEBUG_DEVICESERVER
            printf("set %s: %f\n", keyValue->key().c_str(), keyValue->value<float>());
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

    if ( key == "Analog_Trigger_Level")
    {
        return SmpKeyValue(new TKeyValue<float>( "Analog_Trigger_Level", m_rInspectionControl.getAnalogTriggerLevelVolt(), 0.0, 10.0, 5.0 ) );
    }

    if (key == "LWM_Inspection_Active")
    {
        return SmpKeyValue(new TKeyValue<bool>("LWM_Inspection_Active", m_rInspectionControl.getLWMInspectionActive(), false, true, false));
    }
    if (key == "LWM_Program_Number")
    {
        return SmpKeyValue(new TKeyValue<int>("LWM_Program_Number", m_rInspectionControl.getLWMProgramNumber(), 0, 255, 0));
    }

    if ( key == "DebugInfo_SOURING")
    {
        return SmpKeyValue(new TKeyValue<bool>( "DebugInfo_SOURING", m_rInspectionControl.getDebugInfo_SOURING(), false, true, false ) );
    }
    if ( key == "DebugInfo_SOURING_Extd")
    {
        return SmpKeyValue(new TKeyValue<bool>( "DebugInfo_SOURING_Extd", m_rInspectionControl.getDebugInfo_SOURING_Extd(), false, true, false ) );
    }
    if ( key == "DebugInfo_SCANMASTER")
    {
        return SmpKeyValue(new TKeyValue<bool>( "DebugInfo_SCANMASTER", m_rInspectionControl.getDebugInfo_SCANMASTER(), false, true, false ) );
    }

    if ( key == "IsInEmergencyStopState")
    {
        bool isInEmergencyState = m_rInspectionControl.IsInEmergencyStopState();
        return SmpKeyValue(new TKeyValue<bool>("IsInEmergencyStopState", isInEmergencyState, false, true, false) );
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

    if (m_rInspectionControl.m_oMyVIConfigParser.IsStartCycleAnalogInput() ||
        m_rInspectionControl.m_oMyVIConfigParser.IsStartSeamAnalogInput())
    {
        config.push_back( SmpKeyValue( new TKeyValue<float>( "Analog_Trigger_Level", m_rInspectionControl.getAnalogTriggerLevelVolt(), 0.0, 10.0, 5.0 ) ) );
    }

    if (m_rInspectionControl.isCommunicationToLWMDeviceActive())
    {
        config.push_back(SmpKeyValue(new TKeyValue<bool>("LWM_Inspection_Active", m_rInspectionControl.getLWMInspectionActive(), false, true, false)));
        config.push_back(SmpKeyValue(new TKeyValue<int>("LWM_Program_Number", m_rInspectionControl.getLWMProgramNumber(), 0, 255, 0)));
    }

    config.push_back( SmpKeyValue( new TKeyValue<bool>( "DebugInfo_SOURING", m_rInspectionControl.getDebugInfo_SOURING(), false, true, false ) ) );
    config.push_back( SmpKeyValue( new TKeyValue<bool>( "DebugInfo_SOURING_Extd", m_rInspectionControl.getDebugInfo_SOURING_Extd(), false, true, false ) ) );
    config.push_back( SmpKeyValue( new TKeyValue<bool>( "DebugInfo_SCANMASTER", m_rInspectionControl.getDebugInfo_SCANMASTER(), false, true, false ) ) );

std::cout << config << std::endl;

    return config;
}

} // namespace ethercat

} // namespace precitec

