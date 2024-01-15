/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB), Andreas Beschorner (AB)pda
 * 	@brief		Stores and distributes the calibration parameters.
 */

// poco includes
#include "Poco/File.h"

// project includes

#include <cstdio> // for std::remove
#include <cmath>
#include "math/calibrationData.h"
#include <algorithm>
#include <sstream>
#include <string>
#include <fstream>
#include <cstdlib>

#include "system/tools.h" //pwdToStr
#include "common/calibrationConfiguration.h"
#include "geo/coordinate.h"
#include "math/Calibration3DCoordsLoader.h"
#include <common/defines.h>
#include <common/systemConfiguration.h>

using Poco::File;

namespace precitec {


namespace math {

using filter::LaserLine;
using coordinates::CalibrationConfiguration;
using math::CalibrationParamMap;
using math::CoaxCalibrationData;
using coordinates::CalibrationCameraCorrectionContainer;
using coordinates::CalibrationCameraCorrectionState;

const double CalibrationData::CALIB_VALUES_TOL_MIN=1e-5;


CalibrationData::CalibrationData(SensorId p_oSensorID)
:CalibrationData(p_oSensorID, false, system::wmBaseDir())
{}

CalibrationData::CalibrationData(SensorId p_oSensorID, bool pCanWriteToDisk, std::string pHomeDir) 
: m_oSensorID(validSensorID(p_oSensorID) ? p_oSensorID : SensorId::eInvalidSensor)
, m_oFilenamesConfiguration(p_oSensorID, pHomeDir)
, m_pCameraCorrectionState(nullptr)
{
    if ( !validSensorID(p_oSensorID) )
    {
        wmLogTr(eError, "QnxMsg.Calib.InvalidID", "Calibration data addresses invalid camera %d (expected: 0, 1, 2)!\n", p_oSensorID);
    }
    resetConfig(pCanWriteToDisk, pHomeDir);
} // CTor.



void CalibrationData::resetConfig(bool pCanWriteToDisk, std::string pHomeDir)
{
    m_oParameters = math::CalibrationParamMap();
    getCalibrationCoordsReference() = math::Calibration3DCoords();
    m_o3DFieldModified = true;
    m_canWriteToDisk = pCanWriteToDisk;
    m_oSensorModel = SensorModel::eUndefined; // setSensorModel doesn't work well when the parameter list is empty
    setHomeDirectory(pHomeDir);
    m_oInitialized = false;
    resetCalibrationCorrection();
    assert(!isInitialized());
    assert(!hasData());
}

void CalibrationData::resetConfig()
{
    resetConfig(m_canWriteToDisk, getHomeDir());
}


void CalibrationData::setHomeDirectory(std::string pHomeDir)
{
	if (pHomeDir != m_oFilenamesConfiguration.getHomeDir())
	{
		m_oFilenamesConfiguration = CalibrationConfiguration(m_oSensorID, pHomeDir);
		m_oInitialized = false;//remember to reload the parameters if the config file changes
	}
}
	

//load coordinates from camgriddata
bool CalibrationData::load3DFieldFromCamGridData(const system::CamGridData & pCamGridData)
{
    assert(getSensorModel() != SensorModel::eLinearMagnification);
    if (getSensorModel() == SensorModel::eLinearMagnification)
    {
        wmLog(eError, "Inconsistency in sensor type: load3DFieldFromCamGridData called for coax case\n");
        return false;
    }
    
    if (getSensorModel() == SensorModel::eUndefined)
    {
        setSensorModel(SensorModel::eCalibrationGridOnLaserPlane);
    }
    wmLog(eDebug, "CalibrationData:: load3DFieldFromCamGridData\n");   
    int sensorWidth = m_oParameters.getInt("sensorWidth");
    int sensorHeight = m_oParameters.getInt("sensorHeight");
    if (pCamGridData.sensorWidth() !=  sensorWidth ||  pCamGridData.sensorHeight() !=  sensorHeight)
    {
        wmLog(eWarning, "Unexpected sensor size in CamGridData %d x %d (current sensor size %d x %d )\n", 
            pCamGridData.sensorWidth(),  pCamGridData.sensorHeight(), sensorWidth, sensorHeight);
        return false;
    }
    
    auto & r3DCoords = getCalibrationCoordsReference();
    auto res = loadCamGridData(r3DCoords, pCamGridData);
    
    if (res)
    {
        assert(r3DCoords.getSensorSize().area() > 0);
        
        //update the scheimpflug triangulation angle with the one coming from camera
        // (in the device page only scheimOrientationAngle can be set)
        m_oParameters.setValue<double>("scheimTriangAngle", double(pCamGridData.triangulationAngle_deg()), true);
        
        //complete calibration with parameters in the key-value list (handle both scheimTriangAngle and scheimOrientationAngle)
        load3DFieldFromParameters();
        //finally reset the flag
        m_o3DFieldModified = false;
    }
    else
    {
        //"QnxMsg.Calib.CalError"
        wmLog(eWarning,"Error in calibrationData load3DFieldFromCamGridData\n");
        setSensorModel(SensorModel::eUndefined);
    }
    return res;
}

bool CalibrationData::hasData() const
{
	//assert(!m_oParameters.hasModifiedParameters() && "Inconsistent state, changes have not been saved to xml");
    
	return !m_o3DFieldModified && m_o3DCoords.getSensorSize().area()>0;
}


CalibrationData::~CalibrationData()
{
} // DTor.


//change calibrationdata instance according to arguments
bool CalibrationData::reload(math::Calibration3DCoords p_o3DCoords, math::CalibrationParamMap p_pCalibrationParameters)
{
    if (isInitialized())
    {
        wmLog(eInfo, "reloadCalibdata will override current calibration\n");
    }
    
    m_oInitialized = true;
    m_oParameters = p_pCalibrationParameters;
    
    setSensorModel(p_o3DCoords.isScheimpflugCase() ? SensorModel::eCalibrationGridOnLaserPlane : SensorModel::eLinearMagnification);
 
    auto & r3DCoords = getCalibrationCoordsReference();    
    r3DCoords = p_o3DCoords;
    assert( r3DCoords.getSensorSize().area() > 0 ); // == p_oCoordsArray.getSize());
    assert(&r3DCoords == &getCalibrationCoordsReference());
    m_o3DFieldModified = false; //finally I can reset the flag

    //TODO
    //if (!checkCalibrationValuesConsistency(0.0001, eWarning, true))
    {
        //TODO: what happens if calibration parameters (key-values) are not in sync anymore?
        //assert(false && "after calibrationdata reload, calibration parameters not in sync anymore");
    }

    return true;
    
}

bool CalibrationData::checkCameraRelatedParameters(const CameraRelatedParameters & cameraParameters ) const
{
    auto dpixx = m_oParameters.getDouble("DpixX");
	auto dpixy = m_oParameters.getDouble("DpixY");
    auto imageWidth = m_oParameters.getInt("sensorWidth");
    auto imageHeight = m_oParameters.getInt("sensorHeight");

    return (math::isClose(dpixx, cameraParameters.m_oDpixX, 1e-10) &&
            math::isClose(dpixy, cameraParameters.m_oDpixY, 1e-10) &&
            imageWidth == cameraParameters.m_oWidth &&
            imageHeight == cameraParameters.m_oHeight);
}

void CalibrationData::changeCameraRelatedParameters(const CameraRelatedParameters & cameraParameters, bool recomputeBetaOnSensorParameterChanged)
{

    bool correctCameraParameters = checkCameraRelatedParameters( cameraParameters );
    if (correctCameraParameters)
    {
        return;
    }

    if (m_oParameters.getInt("sensorWidth") != cameraParameters.m_oWidth || m_oParameters.getInt("sensorHeight") != cameraParameters.m_oHeight)
    {
        wmLog(eWarning, "Update calibration sensor size from %dx%d to %dx%d\n",
              m_oParameters.getInt("sensorWidth"), m_oParameters.getInt("sensorHeight"),
              cameraParameters.m_oWidth, cameraParameters.m_oHeight);
        m_oParameters.setValue<int>("sensorWidth", cameraParameters.m_oWidth, true);
        m_oParameters.setValue<int>("sensorHeight", cameraParameters.m_oHeight, true);
    }

    const double oldDpixX = m_oParameters.getDouble("DpixX");
    const double oldDpixY = m_oParameters.getDouble("DpixY");

    // a change in the sensor size only changes the size of the internal calibration lookup table and it does not need any other special handling,
    // while a change in the pixel size requires a recomputation of the other calibration parameters
    bool sensorParametersChanged = (!math::isClose(oldDpixX, cameraParameters.m_oDpixX, 1e-10)) || !math::isClose(oldDpixY, cameraParameters.m_oDpixY, 1e-10);

    if (sensorParametersChanged)
    {
        wmLog(eWarning, "The actual sensor parameters %f %f are different from the calibration parameters %f %f\n",
              cameraParameters.m_oDpixX, cameraParameters.m_oDpixY, oldDpixX, oldDpixY);

        if (recomputeBetaOnSensorParameterChanged)
        {
            m_oParameters.setValue<double>("DpixX", cameraParameters.m_oDpixX, true);
            m_oParameters.setValue<double>("DpixY", cameraParameters.m_oDpixY,  true);

            // as in Calibration3DCoordsLoader::loadCoaxModel
            const double oFactorX = oldDpixX / m_oParameters.getDouble("beta0");
            const double oFactorY = oldDpixY / m_oParameters.getDouble("beta0");
            double newBeta0 = cameraParameters.m_oDpixX  / oFactorX;
            if (!math::isClose(oFactorY, m_oParameters.getDouble("DpixY") / newBeta0))
            {
                wmLog(eWarning, "DpixY not compatible with model,  triangulationAngle should be changed \n");
                assert(false);    // TODO
            }
            // beta0 depends on the value of DpixX,  DpixY
            m_oParameters.setValue<double>("beta0", newBeta0,  true);
            m_oParameters.setValue<bool>("SensorParametersChanged", false);
            wmLog(eWarning, "Updating calibration beta0 due to a difference in the sensor parameters\n");
        }
        else
        {
            m_oParameters.setValue<bool>("SensorParametersChanged", true);
        }
    }

    // after changing parameteres,  3dfield needs to be computed
    m_o3DFieldModified = true; 
    load3DFieldFromParameters();
    assert(!m_o3DFieldModified);
    
}




//read configuration, sets sensor model
void CalibrationData::initConfig(const SensorModel p_oSensorModel, bool createDefault)
{
    if (!m_canWriteToDisk && createDefault)
    {
        wmLog(eWarning, "initConfig: setting createDefault option to false (this calibration data instance can't write to disk)\n");
        createDefault = false;
    }

	if (m_oInitialized)
    {
        wmLog(eWarning, "called initConfig but sensor already initalized\n") ;
        return;
    }
    assert(!m_oInitialized);
    
    // the sensor model in config file is ignored, it will be overridden it with p_oSensorModel
    bool useSensorModelFromConfig = false;
    UNUSED bool ok = loadConfig(useSensorModelFromConfig, createDefault);
    assert(ok);    
    
    setSensorModel(p_oSensorModel);
    
    assert(m_o3DFieldModified);
}

//used by filtertest, compare CalibrationManager::getOSCalibrationDataFromHW
bool CalibrationData::loadFromConfigFolder(bool recomputeFromCamGridData, bool useOnlyBinaryCache)
{
	std::cout << "loadFromConfigFolder: Initializing sensor  " << m_oSensorID  <<  std::endl;
    
    if (!recomputeFromCamGridData  && useOnlyBinaryCache)
    {
        wmLog(eWarning, "useOnlyBinaryCache parameter will be ignored \n");
    }
    
	if (m_oInitialized)
	{
		wmLog(eWarning, "Sensor already initialized\n");
	}

	loadConfig(/*useSensorModelFromConfig*/true, /*createDefault*/ false);
	
	//read sensor model from calibrationdata.xml, coax by default
	SensorModel oSensorModelFromConfig = SensorModel::eUndefined;
	if ( m_oParameters.hasKey("SensorModel") )
	{
        int parameterSensorModel = m_oParameters.getInt("SensorModel");
        if (parameterSensorModel < int(SensorModel::eSensorModelMin) || parameterSensorModel > int(SensorModel::eSensorModelMax))
        {
            oSensorModelFromConfig = SensorModel::eUndefined;
        }
        else
        {            
            oSensorModelFromConfig = static_cast<SensorModel>(parameterSensorModel);
        }
		
	}
	if ( oSensorModelFromConfig == SensorModel::eUndefined)
	{
		wmLog(eWarning, "CalibrationData: config folder doesn't specifiy sensor model, settign to coax\n");
		oSensorModelFromConfig = SensorModel::eLinearMagnification;
	}
	setSensorModel(oSensorModelFromConfig);
	assert(getSensorModel() != SensorModel::eUndefined);
	
	bool ok = false;
	if ( oSensorModelFromConfig == SensorModel::eLinearMagnification )
	{
		ok = load3DFieldFromParameters();
	}
	else
	{
        if (recomputeFromCamGridData)
        {
            system::CamGridData oCamGridData;            
            //compare CalibrateIbOpticalSystem::getCamGridDataFromCamera
            bool validCamGridData(false);
            //try to read camgriddata from binary file
            {
                std::uint32_t oCamGridDataCacheChecksum;
                validCamGridData = oCamGridData.loadFromBytes(m_oFilenamesConfiguration.getCamGridDataBinaryFilename(), oCamGridDataCacheChecksum, true);
            }
            if (!validCamGridData && !useOnlyBinaryCache)
            {
                std::string oCalibCSVFile = m_oFilenamesConfiguration.getCopyCSVFilename();
                std::string oCamGridDataError  = oCamGridData.loadFromCSV(oCalibCSVFile);
                validCamGridData = oCamGridDataError.empty();
            }
            if (!validCamGridData && !useOnlyBinaryCache)
            {
                std::string oCalibCSVFile = m_oFilenamesConfiguration.getCSVFallbackFilename();
                std::string oCamGridDataError  = oCamGridData.loadFromCSV(oCalibCSVFile);
                validCamGridData = oCamGridDataError.empty();
            }
            if (validCamGridData)
            {
                ok = load3DFieldFromCamGridData(oCamGridData);            
            }
        }
	}
	return ok;
}


SensorId CalibrationData::getSensorId() const
{
	return m_oSensorID;
};

SensorModel CalibrationData::getSensorModel() const
{
    return m_oSensorModel;
};


bool CalibrationData::setSensorModel(SensorModel pModel)
{
    wmLog(eDebug, "Setting sensor model from %s to %d (%s) \n",  SensorModelDescription(m_oSensorModel).c_str(), int(pModel), SensorModelDescription(pModel).c_str());
    
    if (pModel == SensorModel::eUndefined || m_oSensorModel != pModel)
    {
        //3dfield needs to be computed
        m_o3DFieldModified = true; 
    }

    m_oSensorModel = pModel;
    
	assert(getSensorModel() == pModel);
    m_oParameters.setValue<int>("SensorModel", int(m_oSensorModel), true);
    return true ;
}


bool CalibrationData::isInitialized() const
{
	return m_oInitialized;
};


bool CalibrationData::checkCalibrationValuesConsistency() const
{
	//if debug is enabled, print the checkCalibrationValuesConsistency messages also in the gui, otherwise only in the logs
	LogType logLevel = m_oParameters.getInt("Debug") ? eInfo: eDebug;
	return checkCalibrationValuesConsistency(0.001, logLevel, false); //usually I don't check for the coordinates, only the angles
}


bool CalibrationData::checkCalibrationValuesConsistency(double tol, LogType logLevel, bool checkCoordinates) const
{
    // FIXME
	bool valid = true;
    
	if ( m_oParameters.hasModifiedParameters() )
	{
		wmLog(logLevel, "checkCalibrationValuesConsistency: Updated parameters not yet saved to file\n");
	}
	else
	{
		wmLog(eDebug, "checkCalibrationValuesConsistency: compare with parameters on disk\n");
		const std::string oConfigFile = getConfigFilename();
		CalibrationParamMap oParametersFromDisk(oConfigFile);
		bool okLoad = !oParametersFromDisk.getConfigFilename().empty(); 
		if (okLoad)
		{
			CoaxCalibrationData oDataFromDisk(oParametersFromDisk);
			if ( ! (oDataFromDisk == getCoaxCalibrationData()) )
			{
				wmLog(logLevel, "checkCalibrationValuesConsistency: calibration data inconsistent with configuration file " + oConfigFile + "\n");
				//assert(false);
				//valid = false;
			}
		}
		else
		{
			wmLog(logLevel, "checkCalibrationValuesConsistency: error reading configuration file " + oConfigFile + "\n");            
		}
		

	}

    const auto & r3DCoords(getCalibrationCoords());
	

    bool isModelScheimpflug = m_o3DCoords.isScheimpflugCase();
    if (m_oSensorModel == SensorModel::eUndefined)
    {
        if (!m_o3DFieldModified)
        {
            wmLog(logLevel, "checkCalibrationValuesConsistency: Undefined sensor model, but grid initialized\n");
            valid = false;
        }
        if (checkCoordinates)
        {
            wmLog(logLevel, "checkCalibrationValuesConsistency: Undefined sensor model, but checkCoordinates requested\n");
            valid = false;
        }
    }
    else
    {
        if ( (m_oSensorModel == SensorModel::eCalibrationGridOnLaserPlane) != isModelScheimpflug )
        {
            wmLog(logLevel, "checkCalibrationValuesConsistency: Inconsistency with sensor model: m_oSensorModel %d in coords isScheimpflug= %d\n", int(m_oSensorModel), isModelScheimpflug);
            //valid =false;
        }
    }
    
	if ( r3DCoords.getSensorSize().area() == 0 )
    {
        if (checkCoordinates)
        {
            wmLog(logLevel, "checkCalibrationValuesConsistency: 3d coords empty \n");
            valid = false;
        }
        else
        {
            wmLog(eDebug, "checkCalibrationValuesConsistency: 3d coords empty \n");
        }
    }

	const CoaxCalibrationData oCoaxCalibData = getCoaxCalibrationData();
	
	if (m_oSensorModel == SensorModel::eLinearMagnification)
	{
		double beta0 = oCoaxCalibData.m_oBeta0;
		if ( beta0 < math::eps )
		{
			wmLog(eError, "checkCalibrationValuesConsistency: beta0 can't be null\n");
			valid = false;
		}


		for ( LaserLine oLaserLine : { LaserLine::FrontLaserLine, LaserLine::CenterLaserLine, LaserLine::BehindLaserLine} )
		{
			std::string suffix = oCoaxCalibData.ParameterKeySuffix(oLaserLine);
			std::string triangAngleStr = "triangulationAngle" + suffix;


			double betaZ;
			bool isHighPlaneOnImageTop;
			math::LineEquation lineXY;
			oCoaxCalibData.getLineDependentParameters(betaZ, isHighPlaneOnImageTop, lineXY, oLaserLine);

			double triangAngle_deg = m_o3DCoords.getTriangulationAngle(angleUnit::eDegrees,  oLaserLine);
                        double triangAngle = m_o3DCoords.getTriangulationAngle(angleUnit::eRadians, oLaserLine);

			if ( triangAngle_deg > 90 )
			{
				wmLog(logLevel, "%s  %f should be between -90 and 90 \n", triangAngleStr.c_str(), triangAngle_deg);
				triangAngle_deg = 180 - triangAngle_deg;
			}

			if ( !(triangAngle_deg > -90 && triangAngle_deg < 90) )
			{
				wmLog(logLevel, "%s  %f should be between -90 and 90 \n", triangAngleStr.c_str(), triangAngle_deg);
				valid = false;
			}

			if ( betaZ < math::eps )
			{
				wmLog(logLevel, "betaZ%s  can't be null\n", suffix.c_str());
				valid = false;
			}

			double expectedBetaZ = beta0 * tan(triangAngle);

			if ( std::abs(std::abs(expectedBetaZ) - betaZ) > tol )
			{
				double expectedAngle = atan2(betaZ, beta0) * 180.0 / math::pi;
				wmLog(logLevel, " %s  %f is not consistent with beta0=%f and betaz=%f  (should be %f, otherwise betaz would be %f ) \n",
					triangAngleStr.c_str(), triangAngle, beta0, betaZ, expectedAngle, expectedBetaZ);
				valid = false;
			}

			//same convention as computeLineDirectionFromCalibrationAngle
			if ( !(isHighPlaneOnImageTop ? triangAngle_deg > 0: triangAngle_deg < 0) )
			{
				wmLog(logLevel, "checkCalibrationValuesConsistency:  %s  %f inconsistent with laser direction (HighPlaneOnImageTop=%d) \n",
					triangAngleStr.c_str(), triangAngle_deg, int(isHighPlaneOnImageTop));
				valid = false;
			}


		}//end for laserline
	}  
	else //Scheimpflug case 
	{
		const std::string triangAngleStr = "scheimTriangAngle";
		const std::string orientationAngleStr = "scheimOrientationAngle";

        const double triangAngleParam = m_oParameters.getDouble(triangAngleStr);
        const double orientationAngleParam = m_oParameters.getDouble(orientationAngleStr)  ;

		const LaserLine oLaserLine = LaserLine::FrontLaserLine;
		
		//similar to the coax case
		const double triangAngle_deg = m_o3DCoords.getTriangulationAngle(angleUnit::eDegrees, oLaserLine);
		if ( std::abs(triangAngleParam + orientationAngleParam - triangAngle_deg) > tol )
		{
			wmLog(logLevel, "checkCalibrationValuesConsistency:  %s  %f inconsistent with saved value (%f  + %f  diff=%f) \n", 
                  triangAngleStr.c_str(), triangAngle_deg,  m_oParameters.getDouble(triangAngleStr), m_oParameters.getDouble(orientationAngleStr) , 
                  std::abs(triangAngleParam - triangAngle_deg));
			valid = false;
		}

	}
#ifndef NDEBUG
    if (!valid)
    {
        wmLog(eError, "Invalid calib values, see log\n");
    }
    assert(valid && "Check calibration Values consistency");
#endif
	return valid;
}


const math::CalibrationParamMap & CalibrationData::getParameters() const
{
    return m_oParameters;
}

//replaces CalibrationParamMap::get() 
//return a copy of the keyvalues, relevant for the current sensors
interface::Configuration CalibrationData::makeConfiguration(bool onlyFirstLine, bool withProcedureParameters ) const
{    
    interface::Configuration  oResult;
    
    auto oKV_SensorModel = m_oParameters.get("SensorModel");    
    oResult.push_back(oKV_SensorModel);
    assert(oKV_SensorModel->value<int>() == int(m_oSensorModel));

    if (m_oSensorModel == SensorModel::eCalibrationGridOnLaserPlane)
    {
        onlyFirstLine = true; 
    }
        
    const std::vector<std::string> suffixesToExclude = onlyFirstLine ?  std::vector<std::string>{"2", "TCP"}: std::vector<std::string>{};

    if (m_oSensorModel == SensorModel::eLinearMagnification)
    {
        //TODO: compare with getCoaxCalibrationData

        std::vector<std::string> coax_keys {"beta0"};
        static const std::vector<std::string> coax_keys_laserline_dependent {"betaZ", "HighPlaneOnImageTop", "laserLine_a", "laserLine_b", "laserLine_c"};
        for ( std::string suffix : { CoaxCalibrationData::ParameterKeySuffix(LaserLine::FrontLaserLine),
                                    CoaxCalibrationData::ParameterKeySuffix(LaserLine::CenterLaserLine),
                                    CoaxCalibrationData::ParameterKeySuffix(LaserLine::BehindLaserLine)} )
        {
            std::transform(coax_keys_laserline_dependent.begin(), coax_keys_laserline_dependent.end(),
                           std::back_inserter(coax_keys), [&suffix](std::string root){return root+suffix;});
            if (onlyFirstLine)
            {
                break;
            }
        }

        for (auto & key: coax_keys)
        {
            assert(m_oParameters.hasKey(key)); //check typos in the list
            oResult.push_back(m_oParameters.get(key) );
        }
                        
		for ( LaserLine oLaserLine : { LaserLine::FrontLaserLine, LaserLine::CenterLaserLine, LaserLine::BehindLaserLine} )
		{
            double oAngle = getCalibrationCoords().getTriangulationAngle(angleUnit::eDegrees, oLaserLine);
            //use the triangulation angles if calibrationcoords has been already initialized
            if ( ! (std::isnan(oAngle)) )
            {
                std::string triangAngleStr = "triangulationAngle" + CoaxCalibrationData::ParameterKeySuffix(oLaserLine);
                oResult.push_back(new interface::TKeyValue<double>(triangAngleStr, oAngle, -90.0, 90.0, oAngle));
                oResult.back()->setReadOnly(true);
                oResult.back()->setPrecision(3);
#ifndef NDEBUG
                if (m_oParameters.hasKey(triangAngleStr))
                {
                    if (std::abs( m_oParameters.getDouble(triangAngleStr) - oAngle ) > CALIB_VALUES_TOL_MIN )
                    {
                        wmLog(eError, "Inconsistency in CalibrationData instance value of %s = %d, should be %d, was it perhaps read from configuration file?\n", 
                            triangAngleStr,  m_oParameters.getDouble(triangAngleStr) , oAngle );
                        assert( false && "Inconsistency in computed triangulation angle");
                    }
                    
                }
#endif
            }
            if (onlyFirstLine)
            {
                break;
            }
        }
        
    }

    
    if (m_oSensorModel == SensorModel::eCalibrationGridOnLaserPlane)
    {
        oResult.push_back(m_oParameters.get("scheimTriangAngle"));
        oResult.push_back(m_oParameters.get("scheimOrientationAngle"));
    }
    
    //add ScanMaster keys, without the parameters for the calibration procedure (handled later)
    for (auto & key: m_oParameters.getScanMasterKeys(false)) 
    {
        oResult.push_back(m_oParameters.get(key));
    }
    
    if ( withProcedureParameters )
    {
        //copy the graph parameters that belong only to the first laser line
        //compare to addCalibrationProcedureParameters
        
        for (auto & key: m_oParameters.getProcedureParameterKeys() )
        {
            if (m_oParameters.isProtectedParamKey(key))
            {
                continue;
            }
            //check if it's a value for the other laser lines
            bool isRequestedLine = true;
            if ( key.length() >  4) 
            {
                for (auto & suffix: suffixesToExclude)
                {
                    if  (key.compare (key.length() - suffix.length(), suffix.length(), suffix)==0)
                    {
                        isRequestedLine=false;
                    }
                }
            }
            if (isRequestedLine)
            {
                assert(m_oParameters.hasKey(key)); 
                oResult.push_back(m_oParameters.get(key));
            }
        }
    }
    //copy all the other key values
    for (auto & key: {"xtcp", "ytcp", "ytcp_2", "ytcp_tcp", "xcc", "ycc", "DpixX", "DpixY", "sensorWidth", "sensorHeight", "InvertX", "InvertY",
        "axisCorrectionFactorY", "distanceTcpToFrontLineInMm", "distanceTcpToBehindLineInMm","Debug", "SensorParametersChanged"})
    {
        assert(m_oParameters.hasKey(key)); //check typos in the list
        oResult.push_back(m_oParameters.get(key) );
    }
    
    oResult.emplace_back(new interface::TKeyValue<bool>{"hasCameraCorrectionGrid", hasCameraCorrectionGrid(), 0, 1, 0});
    oResult.back()->setReadOnly(true);
    
    oResult.emplace_back(new interface::TKeyValue<bool>{"hasIDMCorrectionGrid", hasIDMCorrectionGrid(), 0, 1, 0});
    oResult.back()->setReadOnly(true);

    oResult.push_back(m_oParameters.get("ScanfieldDistortionCorrectionEnable"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kax1"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kax2"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kbx1"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kbx2"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kcx"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kdx"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kex"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kfx"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kay1"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kay2"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kby1"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kby2"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kcy"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kdy"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_key"));
    oResult.push_back(m_oParameters.get("ScanfieldDistortion_kfy"));

    oResult.push_back(m_oParameters.get("IDM_k1"));
    oResult.push_back(m_oParameters.get("IDM_k2"));
    oResult.push_back(m_oParameters.get("IDM_k3"));
    oResult.push_back(m_oParameters.get("IDM_k4"));
    oResult.push_back(m_oParameters.get("IDM_l01"));
    oResult.push_back(m_oParameters.get("IDM_l02"));
    oResult.push_back(m_oParameters.get("IDM_l03"));
    oResult.push_back(m_oParameters.get("IDM_l04"));
	return oResult;

}

std::string CalibrationData::syncXMLContent()
{
    if (!m_canWriteToDisk)
    {
        wmLog(eWarning, "syncXMLContent called, but this calibration data instance can't write to disk)\n");
        assert(false);
        return("");
    }
	std::string oFileContent;
	bool ok = m_oParameters.syncXMLContent(oFileContent);
	if ( !ok )
	{
		oFileContent = "";
	}
	return oFileContent;
}


void CalibrationData::print3DCoords(const geo2d::Point & pRoiOrigin, const geo2d::Size & pRoiSize) const
{

    std::cout << m_oSensorID << std::endl;
    wmLog(eInfo, "GridInfo: Sensor %d \n", m_oSensorID);
    if ( hasData() )
    {
        //assert(m_oSensorType == oSensorModel);

        wmLog(eInfo, "Triangulation Angle line front : %f grad\n", m_o3DCoords.getTriangulationAngle(angleUnit::eDegrees, filter::LaserLine::FrontLaserLine));
        wmLog(eInfo, "Triangulation Angle line back : %f grad\n", m_o3DCoords.getTriangulationAngle(angleUnit::eDegrees, filter::LaserLine::BehindLaserLine));
        wmLog(eInfo, "Triangulation Angle line TCP : %f grad\n", m_o3DCoords.getTriangulationAngle(angleUnit::eDegrees, filter::LaserLine::CenterLaserLine));

        std::ostringstream fileSuffix;
        fileSuffix << "SensorId" << m_oSensorID;
        switch ( m_oSensorModel )
        {
            case SensorModel::eLinearMagnification:
                fileSuffix << "coax";
                break;
            case SensorModel::eCalibrationGridOnLaserPlane:
                fileSuffix << "scheimpflug";

                //m_oCamGridData.at(static_cast<ScheimpflugSensorId> (oSensorId)).show();
                //wmLog(eInfo, "Line boundaries: %d elements \n", m_oLineBoundaries.size());
                break;
            default:
                assert(false);
                break;
        }

        m_o3DCoords.coordsToTable(fileSuffix.str(), pRoiOrigin, pRoiSize);

    }

}

// ----------------------------------------------------------


void CalibrationData::showData(std::ostream & out) const
{
    // FIXME
    double angle = 0;
	out << "Sensor " << m_oSensorID;
    if (!hasData())
    {
        out << "coordinates not computed\n";
        return;
    }
    
	if ( getSensorModel() == SensorModel::eCalibrationGridOnLaserPlane )
    {
        angle =  m_oParameters.getDouble("scheimTriangAngle") + m_oParameters.getDouble("scheimOrientationAngle");
		out << " ---> Scheimpflug calibration " ;
			
	}
	else
    {
        assert(getSensorModel() == SensorModel::eLinearMagnification);
        angle = getCoaxCalibrationData().computeTriangulationAngle(LaserLine::FrontLaserLine);
        
		out << " ---> coax values: " 
			<< m_oParameters.getDouble("beta0") << ", " << m_oParameters.getDouble("betaZ") << "," ;
	}
	out << " magnification at center = " <<
			m_o3DCoords.factorHorizontal() << " , " << m_o3DCoords.factorVertical(100, 512, 512, LaserLine::FrontLaserLine)
			<< " angle=" << angle << " degrees\n";
}

const std::string & CalibrationData::getConfigFilename() const
{
	return m_oFilenamesConfiguration.getConfigFilename();
}

const std::string & CalibrationData::getHomeDir() const
{
    return m_oFilenamesConfiguration.getHomeDir();
}


//reads calibration xml file, sets sensor but doesn'r compute coords yer
//called by loadCalibDataAfterSignal to update TCP key values
bool CalibrationData::loadConfig(bool useSensorModelFromConfig, bool createDefault)
{
    if (!m_canWriteToDisk && createDefault)
    {
        wmLog(eWarning, "loadConfig: setting createDefault option to false (this calibration data instance can't write to disk)\n");
        createDefault = false;
    }
    wmLog(eInfo, "CalibrationData loadConfig (useSensorModelFromConfig %s)\n", useSensorModelFromConfig ?  "Y" : "N");
	assert(validSensorID(m_oSensorID) && "Wrong sensor in CalibrationData::load");

    /////////////////////////////////////////////////
    //  Read parameters from CalibrationData0.xml if it exists, or create it with default values
    ////////////////////////////////////////////////

	const std::string oConfigFile = m_oFilenamesConfiguration.getConfigFilename();
	File oConfigFileInstance( oConfigFile );

	bool ok = true;
	if (oConfigFileInstance.exists())
	{
        
		ok &= m_oParameters.readParamMapFromFile(oConfigFile);

	}
	else
	{
        if (createDefault)
        {
            std::ostringstream oMsg;
            oMsg << ": Configuration file '" << oConfigFile << "' not found.\n";
            std::cout << oMsg.str();
            wmLog(eDebug, oMsg.str().c_str());

            oMsg.str("");

            oMsg << "Using default parameters. File will be created.\n";
            wmLog(eWarning, oMsg.str().c_str());
            
            ok &= m_oParameters.setFile(oConfigFile); //sets file and reads from it
            ok &= m_oParameters.saveToFile(); 
            if (!ok)
            {
                m_oInitialized = false;
                wmLog(eError, "Error writing to file %s\n", oConfigFile);
            }
        }
        else
        {
            wmLog(eError, "Inexistent calibfile %s \n", oConfigFile);
        }
	}

	if (!ok || m_oParameters.size()==0)
	{
		wmLog(eError, "Error in reading/saving the calibration confugration file\n");
        assert(false && "Error in CalibrationData load");
		m_oInitialized = false;
		return false;
	}
	
	
    if (m_oParameters.hasModifiedParameters())
	{
		wmLog(eError, "CalibrationData::load: error in file sync\n");
	}
	
	

    //the sensor model will be actually set when load3DFieldFromParameters or load3DFieldFromPrecomputeCoordsOnDisk is called
    auto oSensorModelFromConfig = m_oParameters.getInt("SensorModel");
    setSensorModel(SensorModel::eUndefined); 
    if ( useSensorModelFromConfig )
    {
        //I rewrite the sensor model to the configuration because of loadFromConfigFolder
        m_oParameters.setValue("SensorModel", oSensorModelFromConfig, true);
    }
    
    /*
     * Set status: the configuration has been read (m_oInitialized is true), 
     * but coordinates are not computed yet (m_o3DFieldModified has not been set to false yet)
    */
    m_oInitialized = true;
    assert(m_o3DFieldModified);
	return ok;

} // load

template <typename tKeyType>
void CalibrationData::setKeyValue(const interface::Key p_oKey, const tKeyType p_oValue, bool overwriteProtected) // updates CalibrationParamMap and CalinbrationGrid
{
	//CalibrateIbOpticalSystem wants to set "triangulationAngle"  + m_oLineNamesInResultValues, but these are not key-values anymore
	if ( (p_oKey == "triangulationAngle" || p_oKey == "triangulationAngle_TCP" || p_oKey == "triangulationAngle_2") )
	{
        assert(false && "trying to set directly triangulatonAngle");
		return;
	}
	
	m_oParameters.setValue<tKeyType>(p_oKey, p_oValue, overwriteProtected);
    
    //TODO: should TCP be handled differently?
	if (m_oParameters.hasModifiedCalibrationParameter())
    {
        m_o3DFieldModified = true;
    }
};

//explicit template instantiation
template void CalibrationData::setKeyValue<double>(const interface::Key p_oKey, const double p_oValue, bool overwriteProtected);
template void CalibrationData::setKeyValue<int>(const interface::Key p_oKey, const int p_oValue, bool overwriteProtected);
template void CalibrationData::setKeyValue<bool>(const interface::Key p_oKey, const bool p_oValue, bool overwriteProtected);


//needed by calibration::DeviceServer
interface::KeyHandle CalibrationData::setKeyValue(interface::SmpKeyValue p_oSmpKeyValue)
{
	auto res = m_oParameters.set(p_oSmpKeyValue);
    
	if (m_oParameters.hasModifiedCalibrationParameter())
    {
        m_o3DFieldModified = true;
    }
    return res;
}


CoaxCalibrationData CalibrationData::getCoaxCalibrationData()  const
{
	CoaxCalibrationData oData (m_oParameters);
	return oData;
}

geo2d::DPoint CalibrationData::getTCPCoordinate(int p_oIdxWorker, filter::LaserLine p_laserLine) const
{
    std::string key_x = "xtcp";
    std::string key_y = "ytcp";
    switch (p_laserLine)
    {
        default:
        case filter::LaserLine::FrontLaserLine:
            key_y    = "ytcp";
            break;
        case filter::LaserLine::BehindLaserLine:
            key_y    = "ytcp_2";
            break;
        case filter::LaserLine::CenterLaserLine:
            key_y   = "ytcp_tcp";
            break;
    }
    //std::string key_y = "ytcp" + oCoaxCalibData.ParameterKeySuffix()[p_laserLine];
    
    
    auto oTCPSensorCoordinates =  geo2d::DPoint{ m_oParameters.getDouble(key_x),  m_oParameters.getDouble(key_y)};
    if (m_pCameraCorrectionState)
    {
        m_pCameraCorrectionState->getCurrentCorrection(p_oIdxWorker).apply(oTCPSensorCoordinates.x, oTCPSensorCoordinates.y); 
    }
	return oTCPSensorCoordinates ;
}

geo2d::DPoint CalibrationData::getTCPCoordinate(const interface::ScannerContextInfo & rScannerContext, filter::LaserLine p_laserLine) const
{
    std::string key_x = "xtcp";
    std::string key_y = "ytcp";
    switch (p_laserLine)
    {
        default:
        case filter::LaserLine::FrontLaserLine:
            key_y    = "ytcp";
            break;
        case filter::LaserLine::BehindLaserLine:
            key_y    = "ytcp_2";
            break;
        case filter::LaserLine::CenterLaserLine:
            key_y   = "ytcp_tcp";
            break;
    }
    //std::string key_y = "ytcp" + oCoaxCalibData.ParameterKeySuffix()[p_laserLine];
    auto oTCPSensorCoordinates =  geo2d::DPoint{ m_oParameters.getDouble(key_x),  m_oParameters.getDouble(key_y)};
    if (m_pCameraCorrectionState)
    {
        auto  oCorrection  = m_pCameraCorrectionState->computeCorrection(rScannerContext.m_x, rScannerContext.m_y);
        oCorrection.apply(oTCPSensorCoordinates.x, oTCPSensorCoordinates.y);
    }
	return oTCPSensorCoordinates ;
}

interface::ScannerContextInfo CalibrationData::getCurrentScannerInfo(int p_oIdxWorker) const
{
    if (!m_pCameraCorrectionState)
    {
        return interface::ScannerContextInfo{false, 0.0, 0.0};
    }
    auto pos = m_pCameraCorrectionState->getCurrentPosition(p_oIdxWorker);
    return interface::ScannerContextInfo{true, pos.x, pos.y};
}

bool CalibrationData::scanfieldDistortionCorrectionFactor(std::vector<double>& k) const
{
    k.resize(16);
    k[0] = m_oParameters.getDouble("ScanfieldDistortion_kax1");
    k[1] = m_oParameters.getDouble("ScanfieldDistortion_kax2");
    k[2] = m_oParameters.getDouble("ScanfieldDistortion_kbx1");
    k[3] = m_oParameters.getDouble("ScanfieldDistortion_kbx2");
    k[4] = m_oParameters.getDouble("ScanfieldDistortion_kcx");
    k[5] = m_oParameters.getDouble("ScanfieldDistortion_kdx");
    k[6] = m_oParameters.getDouble("ScanfieldDistortion_kex");
    k[7] = m_oParameters.getDouble("ScanfieldDistortion_kfx");
    k[8] = m_oParameters.getDouble("ScanfieldDistortion_kay1");
    k[9] = m_oParameters.getDouble("ScanfieldDistortion_kay2");
    k[10] = m_oParameters.getDouble("ScanfieldDistortion_kby1");
    k[11] = m_oParameters.getDouble("ScanfieldDistortion_kby2");
    k[12] = m_oParameters.getDouble("ScanfieldDistortion_kcy");
    k[13] = m_oParameters.getDouble("ScanfieldDistortion_kdy");
    k[14] = m_oParameters.getDouble("ScanfieldDistortion_key");
    k[15] = m_oParameters.getDouble("ScanfieldDistortion_kfy");
    return m_oParameters.getBool("ScanfieldDistortionCorrectionEnable");
}

std::pair<std::vector<double>, std::vector<double>> CalibrationData::idmModel() const
{
    std::pair<std::vector<double>, std::vector<double>> model;
    auto& k = model.second;
    auto& l0 = model.first;
    k.emplace_back(m_oParameters.getDouble("IDM_k1"));
    k.emplace_back(m_oParameters.getDouble("IDM_k2"));
    k.emplace_back(m_oParameters.getDouble("IDM_k3"));
    k.emplace_back(m_oParameters.getDouble("IDM_k4"));
    l0.emplace_back(m_oParameters.getDouble("IDM_l01"));
    l0.emplace_back(m_oParameters.getDouble("IDM_l02"));
    l0.emplace_back(m_oParameters.getDouble("IDM_l03"));
    l0.emplace_back(m_oParameters.getDouble("IDM_l04"));

    return model;
}

const Calibration3DCoords & CalibrationData::getCalibrationCoords() const
{
    return m_o3DCoords;
}

Calibration3DCoords & CalibrationData::getCalibrationCoordsReference()
{
    return m_o3DCoords;
}

LEDCalibrationData CalibrationData::getLEDCalibrationData() const
{
    LEDCalibrationData data (m_oParameters);
    return data;
}

void CalibrationData::setFallbackParameters(bool syncConfigFile)
{
    setSensorModel(SensorModel::eLinearMagnification);

	assert(m_oInitialized);
	CoaxCalibrationData oCoaxData = getCoaxCalibrationData();
	
	double oBeta0 = (oCoaxData.m_oBeta0 < math::eps) ? 0.5 : oCoaxData.m_oBeta0;
	double oBetaZ = (oCoaxData.m_oBetaZ < math::eps) ? 0.5 : oCoaxData.m_oBetaZ;
	setKeyValue("beta0", oBeta0);
	setKeyValue("betaZ", oBetaZ);
	if (syncConfigFile)
	{
		if (m_canWriteToDisk)
		{
			m_oParameters.saveToFile();
		}
		else
		{
			wmLog(eWarning, "Fallback Parameters not saved to file (this calibration data instance can't write to disk)\n");
		}
		assert(!m_oParameters.hasModifiedParameters() && "Inconsistent state, changes have not been saved to xml");
	}
	wmLog(eInfo, "SetFallback sensor %d \n", m_oSensorID);
}


bool CalibrationData::load3DFieldFromParameters()
{
    assert(m_oInitialized);

    if (m_oSensorModel == SensorModel::eUndefined)
    {
        return false;
    }
    auto & rCoords = getCalibrationCoordsReference();
    bool oGridOK = false;
    if (getSensorModel() == SensorModel::eCalibrationGridOnLaserPlane)
    {
        //in the scheimpflug case, the parameters provide only the angle
        rCoords.completeInitialization(SensorModel::eCalibrationGridOnLaserPlane);
    
        float oAngleValue = float(m_oParameters.getDouble( "scheimTriangAngle") + m_oParameters.getDouble( "scheimOrientationAngle"));
        rCoords.setAllTriangulationAngles( oAngleValue, angleUnit::eDegrees);
        //finally reset the flag
        m_o3DFieldModified = false;
        oGridOK = true;
    }
    else
    {
#ifndef NDEBUG    
        const CoaxCalibrationData oCoaxData = getCoaxCalibrationData();    
        
        assert(oCoaxData.m_oBeta0 > math::eps );
        assert(oCoaxData.m_oBetaZ > math::eps );
#endif        
        //compute the 3dcoords from the updated coax data
        

        bool useOrientedLine = interface::SystemConfiguration::instance().get(interface::SystemConfiguration::BooleanKey::Scanner2DEnable); //TODO use calib parameter
        oGridOK = loadCoaxModel(rCoords, getCoaxCalibrationData(), useOrientedLine);
        if (oGridOK)
        {
            m_o3DFieldModified = false;
        }
        assert( int(oCoaxData.m_oWidth* oCoaxData.m_oHeight) == rCoords.getSensorSize().area());
        
        for ( LaserLine oLaserLine : { LaserLine::FrontLaserLine, LaserLine::CenterLaserLine, LaserLine::BehindLaserLine} )
        {
            double oAngle = getCalibrationCoords().getTriangulationAngle(angleUnit::eDegrees, oLaserLine);
            std::string triangAngleStr = "triangulationAngle" + CoaxCalibrationData::ParameterKeySuffix(oLaserLine);
            m_oParameters.setValue<double>(triangAngleStr, oAngle, true);
        }

    }
    return oGridOK;
}


bool CalibrationData::validSensorID(const int p_oSensorID) 
{
    //gives false in case of eSensorIdWildcard or 	eInvalidSensor
	return (p_oSensorID < eNumSupportedSensor ) && (p_oSensorID>=0);
}

geo2d::Size CalibrationData::getSensorSize() const
{
    if (!isInitialized() || m_oSensorModel == SensorModel::eUndefined)
    {
        return geo2d::Size(0,0);
    }
    return getCalibrationCoords().getSensorSize();
  
}

void CalibrationData::resetCalibrationCorrection()
{
    m_pCameraCorrectionState.reset();
}

void CalibrationData::setCalibrationCorrectionContainer ( CalibrationCameraCorrectionContainer correction_initialization_arguments)
{
    m_pCameraCorrectionState = std::make_shared<CalibrationCameraCorrectionState>(correction_initialization_arguments);
    // m_o3DFieldModified is not updated intentionally: the correction is applied for last, it does not modify the coordinates
}

bool CalibrationData::updateCameraCorrection(double scanner_x,  double scanner_y, int p_oIdxWorker)
{
    bool changed = false;
    if (m_pCameraCorrectionState)
    {
        changed = m_pCameraCorrectionState->updateCorrectionPosition(scanner_x, scanner_y, p_oIdxWorker);
        assert(m_pCameraCorrectionState->getCurrentPosition(p_oIdxWorker).x ==  scanner_x);
        assert(m_pCameraCorrectionState->getCurrentPosition(p_oIdxWorker).y ==  scanner_y);
    }
    else
    {
        wmLog(eWarning, "%s called, but corrections not defined \n",  __FUNCTION__);
    }
    return changed;
}

bool CalibrationData::hasCameraCorrectionGrid() const
{
    return m_pCameraCorrectionState !=  nullptr;
}


CalibrationCameraCorrectionContainer CalibrationData::getCorrectionGridParameters() const
{
    if (!hasCameraCorrectionGrid())
    {
        return {};
    }
    return m_pCameraCorrectionState->getCorrectionContainer();
}

bool CalibrationData::hasIDMCorrectionGrid() const
{
    return !m_oIDMCorrectionContainer.empty();
}

int CalibrationData::getCalibratedIDMWeldingDepth(double scanner_x,  double scanner_y, int raw_idmvalue) const
{
    if ( hasIDMCorrectionGrid())
    {
        return m_oIDMCorrectionContainer.get(scanner_x,scanner_y, raw_idmvalue);
    }
    return raw_idmvalue;
}

    
void CalibrationData::setCalibrationIDMCorrectionContainer(coordinates::CalibrationIDMCorrectionContainer idmcorrection)
{
    m_oIDMCorrectionContainer = std::move(idmcorrection);
}



const coordinates::CalibrationConfiguration& CalibrationData::getFilenamesConfiguration() const
{
    return m_oFilenamesConfiguration;
}


coordinates::CalibrationIDMCorrectionContainer CalibrationData::getIDMCorrectionGridParameters() const
{
    if (m_oIDMCorrectionContainer.empty())
    {
        return {};
    }
    return m_oIDMCorrectionContainer;
}



math::LineEquation CalibrationData::getLaserLineAtZCollimatorHeight(double z_mm, const interface::ScannerContextInfo & rScannerContext, filter::LaserLine p_laserline)
{
    const auto & r3DCoords = getCalibrationCoords();
    if (r3DCoords.isScheimpflugCase())
    {
        if (z_mm == 0.0)
        {
            auto TCPCoords_pix = getTCPCoordinate(rScannerContext, p_laserline);
            return LineEquation{0,1.0, TCPCoords_pix.y};
        }
        wmLog(eError, "LaserLine at Z not implemented in Scheimpflug case \n");
        return {};
    }

    if (r3DCoords.usesOrientedLineCalibration())
    {
        if (rScannerContext.m_hasPosition && math::isClose(rScannerContext.m_x, 0.0) && math::isClose(rScannerContext.m_y, 0.0) )
        {
            try
            {
                auto & model = r3DCoords.getOrientedLineCalibration(p_laserline);
                return model.getLaserLineAtZCollimatorHeight(z_mm);

            }
            catch(...)
            {
                return {};
            }
        }
        else
        {
            wmLog(eError, "LaserLine at Z implemented only at scanner position 0,0");
            return {};
        }
    }

    auto TCPCoords_pix = getTCPCoordinate(rScannerContext, p_laserline);
    // line horizontal
    std::vector<geo2d::DPoint> distancesFromTCP { {-1.0, z_mm}, {1.0, z_mm} };
    auto points_pix =  r3DCoords.distanceTCPmmToSensorCoordCoax(distancesFromTCP, TCPCoords_pix.x, TCPCoords_pix.y);
    if (points_pix.empty())
    {
        return {};
    }
    std::vector<double> x_pix{points_pix[0].x, points_pix[1].x};
    std::vector<double> y_pix{points_pix[0].y, points_pix[1].y};
    return LineEquation{x_pix, y_pix};
}


math::LineEquation CalibrationData::getZLineForOrientedLaserLine (filter::LaserLine p_laserline, double z_mm) const
{
    using namespace filter;
    assert(m_o3DCoords.usesOrientedLineCalibration());
    return m_o3DCoords.getOrientedLineCalibration(p_laserline).getLaserLineAtZCollimatorHeight(z_mm);
}

geo2d::DPoint CalibrationData::getZPointForOrientedLaserLine (filter::LaserLine p_laserline) const
{

    auto laserLineXY = getZLineForOrientedLaserLine(p_laserline, 0.0);
    //get a point on the laser line , more or less at the center of the screen
    geo2d::DPoint point;
    laserLineXY.project(point.x, point.y, getSensorSize().width/2.0, getSensorSize().height/2.0);
    return point;
}

void CalibrationData::adjustZPointForOrientedLaserLine (geo2d::DPoint point,filter::LaserLine p_laserline)
{
    using namespace filter;
    assert(m_o3DCoords.usesOrientedLineCalibration());
    m_o3DCoords.adjustZPointForOrientedLaserLine(p_laserline, point);
}




} // math
} // precitec
