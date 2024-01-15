/**
 *  @file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB)
 *  @date		2013
 *  @brief 		Calibration device server - used to parameterize the calibration, but also to report results back to the user / gui side.
 */

// clib includes
#include <iostream>
#include <cstdio>
#include <memory>
#include <string>
// project includes
#include <calibration/deviceServer.h>
#include <math/calibrationData.h>
#include <common/defines.h>

namespace precitec {
namespace calibration {

using math::SensorModel;

DeviceServer::DeviceServer( CalibrationManager& p_rCalibrationManager ) : 
m_rCalibrationManager( p_rCalibrationManager )
{

} // CTor.



DeviceServer::~DeviceServer()
{

} // DTor.



void DeviceServer::uninitialize()
{
	wmLog( eError, "DeviceServer::uninitialize - Not supported by Calibration Device Server!\n" );

} // uninitialize



void DeviceServer::reinitialize()
{
	wmLog( eError, "DeviceServer::reinitialize - Not supported by Calibration Device Server!\n" );

} // reinitialize



int DeviceServer::initialize(Configuration const& config, int subDevice)
{
	Poco::Thread::sleep(100);
    return 1;

} // initialize


//subdevice is SensorID
KeyHandle DeviceServer::set(SmpKeyValue p_oSmpKeyValue, int subDevice)
{
    std::cout <<  "calibration device server set " << p_oSmpKeyValue->key() << "\n";

    if (p_oSmpKeyValue->key() == "SM_ZCollDrivingRelative")
    {
        m_rCalibrationManager.setZCollDrivingRelative(p_oSmpKeyValue->value<double>());
        return 0;
    }

    if (p_oSmpKeyValue->key() == "SM_LaserPowerForCalibration")
    {
        m_rCalibrationManager.setLaserPowerInPctForCalibration(p_oSmpKeyValue->value<double>());
        return 0;
    }

    if (p_oSmpKeyValue->key() == "SM_WeldingDurationForCalibration")
    {
        m_rCalibrationManager.setWeldingDurationInMsForCalibration(p_oSmpKeyValue->value<double>());
        return 0;
    }

    if (p_oSmpKeyValue->key() == "SM_JumpSpeedForCalibration")
    {
        m_rCalibrationManager.setJumpSpeedInMmPerSecForCalibration(p_oSmpKeyValue->value<double>());
        return 0;
    }

    if (m_rCalibrationManager.isOCTTrackApplication())
    {
        if (p_oSmpKeyValue->key() == "OCT Wide Scan")
        {
            bool wideArea = p_oSmpKeyValue->value<bool>();
            m_rCalibrationManager.setOCTScanWide(wideArea);
            return {1};  
        }

         auto kh = m_rCalibrationManager.setCalibrationOCTKeyValue(p_oSmpKeyValue, true);
         if (kh.handle() != -1) //valid keyhandle
         {
             return kh;
        }
        // if kh is invalid, we need to continue searching the keyvalue
    }
    
    if (p_oSmpKeyValue->key() == "SM_ScanFieldPath" )
    {
        m_rCalibrationManager.setScanFieldPath(p_oSmpKeyValue->value<std::string>());
        return 0;
    }

    auto & rCalibrationData = m_rCalibrationManager.getCalibrationData(subDevice);
    auto oSensorModel = rCalibrationData.getSensorModel();

    assert(rCalibrationData.isInitialized() && rCalibrationData.getSensorId() == subDevice);
  

    if (rCalibrationData.getCalibrationCoords().usesOrientedLineCalibration())
    {
        for (int laserLine = 0; laserLine < 3; laserLine ++)
        {
            std::string suffix = math::CoaxCalibrationData::ParameterKeySuffix(static_cast<filter::LaserLine>(laserLine));
            auto oWhichLine = static_cast<LaserLine>(laserLine);
            for (char coord : {'X' , 'Y'})
            {
                char buffer[50];
                sprintf(buffer, "Screen Point %c at Z=0 line%s", coord, suffix.c_str() );
                if (p_oSmpKeyValue->key() == std::string(buffer) )
                {
                    auto point = m_rCalibrationManager.getScreenPointZ0(oWhichLine);
                    switch(coord)
                    {
                        case 'X':
                            point.x = p_oSmpKeyValue->value<double>();
                            break;
                        case 'Y':
                            point.y = p_oSmpKeyValue->value<double>();
                            break;
                    }
                    m_rCalibrationManager.setScreenPointZ0(oWhichLine, point);
                }
            }
        }
    }

    //update key in calibrationData and updateRelatedValuesInCalibrationGrid if necessary
	KeyHandle oRetHdl = rCalibrationData.setKeyValue(p_oSmpKeyValue);  

    
    assert(oSensorModel == rCalibrationData.getSensorModel() && "sendCalibDataChangedSignal doenst support change of sensortype");

    bool needsCoordInitialization = !rCalibrationData.hasData(); //initialize 3d coords only if necessary
    //should also check if it's  a graph parameter, or just a triangulation angle

    if (oSensorModel == SensorModel::eUndefined)
    {
         wmLog(eWarning, "Updating  %s, but sensor model not initialized ", p_oSmpKeyValue->key().c_str() );
    }
    
    // transfer xml file and data to win and host workflow, update 3DField if needed
    m_rCalibrationManager.sendCalibDataChangedSignal(subDevice, needsCoordInitialization);

	return oRetHdl;
} // set



void DeviceServer::set( Configuration p_oConfig, int p_oSubDevice )
{
	for (auto & rKv: p_oConfig)
    {
        set(rKv, p_oSubDevice);
    }

} // set



SmpKeyValue DeviceServer::get(Key p_oKey, int subDevice)
{
    assert(m_rCalibrationManager.getCalibrationData(subDevice).getSensorId() == subDevice);

    if (p_oKey == "SM_ZCollDrivingRelative")
    {
        return SmpKeyValue(new interface::TKeyValue<double>("SM_ZCollDrivingRelative", m_rCalibrationManager.getZCollDrivingRelative(), -50, 50., 0., 3));
    }
    if (p_oKey == "SM_LaserPowerForCalibration")
    {
        return SmpKeyValue(new interface::TKeyValue<double>("SM_LaserPowerForCalibration", m_rCalibrationManager.getLaserPowerInPctForCalibration(), 0., 100., 15., 0));
    }

    if (p_oKey == "SM_WeldingDurationForCalibration")
    {
        return SmpKeyValue(new interface::TKeyValue<double>("SM_WeldingDurationForCalibration", m_rCalibrationManager.getWeldingDurationInMsForCalibration(), 1., 1000., 50., 0));
    }

    if (p_oKey == "SM_JumpSpeedForCalibration")
    {
        return SmpKeyValue(new interface::TKeyValue<double>("SM_JumpSpeedForCalibration", m_rCalibrationManager.getJumpSpeedInMmPerSecForCalibration(), 1., 2000., 250., 0));
    }

    if (m_rCalibrationManager.isOCTTrackApplication())
    {
        if (p_oKey == "OCT Wide Scan")
        {
            return SmpKeyValue(new interface::TKeyValue<bool>( "OCT Wide Scan", m_rCalibrationManager.getOCTScanWide(), false, true, true));
        }
        auto oRes = m_rCalibrationManager.getCalibrationOCTKeyValue(p_oKey);
        //if the result is valid then return, otherwise check further if it's a coax parameter
        if (!oRes.isNull() && oRes->isHandleValid())
        {
            return oRes;
        }
    }
    
    if (p_oKey == "SM_ScanFieldPath" )
    {
        return new interface::TKeyValue<std::string>("SM_ScanFieldPath", m_rCalibrationManager.getScanFieldPath());
    }
    
    if (m_rCalibrationManager.getCalibrationData(subDevice).getCalibrationCoords().usesOrientedLineCalibration())
    {
        for (int laserLine = 0; laserLine < 3; laserLine ++)
        {
            auto oWhichLine = static_cast<LaserLine>(laserLine);
            std::string suffix = math::CoaxCalibrationData::ParameterKeySuffix(oWhichLine);
            for (char coord : {'X' , 'Y'})
            {
                char buffer[50];
                sprintf(buffer, "Screen Point %c at Z=0 line%s", coord, suffix.c_str() );
                if (p_oKey == std::string(buffer) )
                {
                    auto point = m_rCalibrationManager.getScreenPointZ0(oWhichLine);
                    switch(coord)
                    {
                        case 'X': return new interface::TKeyValue<double>(std::string(buffer), point.x, 0.0, 1280.0, point.x);
                        case 'Y': return new interface::TKeyValue<double>(std::string(buffer), point.y, 0.0, 1024.0, point.x);
                    }
                }
            }
        }
    }

    auto oRes =  m_rCalibrationManager.getCalibrationData(subDevice).getParameters().get( p_oKey );
    if (!oRes->isHandleValid() )
    {
        std::cout << "invalid key \n";
    }
    return oRes;

} // get



SmpKeyValue DeviceServer::get( KeyHandle handle, int subDevice )
{
	wmLog( eError, "DeviceServer::get( KeyHandle ) - Not supported by Calibration Device Server!\n" );

	return new KeyValue();

} // get



Configuration DeviceServer::get( int subDevice )
{
    auto & rCalibrationData = m_rCalibrationManager.getCalibrationData(subDevice);
    if (!rCalibrationData.isInitialized())
    {
        std::cout << "Device server get, but configuraiton not initialized\n";
        Configuration oEmptyConfig; 
        return oEmptyConfig;
    }
    assert(rCalibrationData.getSensorId() == subDevice);

    bool forceOnlyFirstLine = false;
    bool showProcedureParameters = true;

    if (m_rCalibrationManager.isOCTTrackApplication())
    {
        forceOnlyFirstLine = true;
        showProcedureParameters = false;
        auto config = m_rCalibrationManager.getCalibrationOCTConfiguration();
        config.push_back( get( "OCT Wide Scan", subDevice)  );
        for (auto & rEntry :  rCalibrationData.makeConfiguration(forceOnlyFirstLine, showProcedureParameters))
        {
            config.push_back(rEntry);
        }
        return config;
    }
    else
    {
        auto config = rCalibrationData.makeConfiguration(forceOnlyFirstLine, showProcedureParameters);
        config.push_back(new interface::TKeyValue<std::string>("SM_ScanFieldPath", m_rCalibrationManager.getScanFieldPath()));

        if (m_rCalibrationManager.getCalibrationData(subDevice).getCalibrationCoords().usesOrientedLineCalibration())
        {
            for (int laserLine = 0; laserLine < 3; laserLine ++)
            {
                auto oWhichLine = static_cast<LaserLine>(laserLine);
                std::string suffix = math::CoaxCalibrationData::ParameterKeySuffix(oWhichLine);
                for (char coord : {'X' , 'Y'})
                {
                    char buffer[50];
                    sprintf(buffer, "Screen Point %c at Z=0 line%s", coord, suffix.c_str() );
                    config.push_back(get(std::string(buffer)));
                }
            }
        }
        return config;
    }
} // get



} // namespace calibration
} // namespace precitec
