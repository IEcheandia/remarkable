#include "math/CalibrationParamMap.h"

#include <cassert>
#include "math/mathUtils.h"

#include "Poco/AutoPtr.h"
#include "Poco/Util/XMLConfiguration.h"

#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/NodeList.h"

#include "Poco/XML/XMLWriter.h"

#include "system/tools.h"
#include "math/calibrationStructures.h"

#include "util/CalibrationScanMasterData.h"
#include "common/defines.h"

using namespace Poco;
using namespace Poco::XML;
using precitec::math::SensorModel;

namespace precitec
{
namespace math
{
	CalibrationParamMap::CalibrationParamMap()
	{
        initializeCalibrationKeyValues();
	}

	CalibrationParamMap::CalibrationParamMap(const std::string oConfigFilename):
			m_oConfigFilename(oConfigFilename)
	{
		initializeCalibrationKeyValues();
		readParamMapFromFile(m_oConfigFilename);
		if ( m_oConfigFilename.length() == 0 )
		{
			wmLog(eWarning, "CalibrationParamMap: file %s not valid\n", oConfigFilename.c_str());
		}
	}
	
	void CalibrationParamMap::serialize ( system::message::MessageBuffer &buffer ) const
	{
        // ProtectedParamKey and graphparam key are hardcoded
        assert(buffer.hasSpace(sizeof(m_oParameters)));

        marshal(buffer, int(m_oParameters.size()));
        marshal(buffer, m_oParameters);
        
    }
    void CalibrationParamMap::deserialize( system::message::MessageBuffer const&buffer )
    {
        //define keys, define ProtectedParamKey and graphparam key
        initializeCalibrationKeyValues();

        int oNumParams;
        interface::Configuration oTmpConf;
        deMarshal(buffer, oNumParams);
        
        deMarshal(buffer, oTmpConf);
        assert( int(oTmpConf.size()) == oNumParams);
        
        m_oParameters.reserve(oNumParams);
        for (auto & curParameter: oTmpConf)
        {
            const auto & curKey = curParameter->key();
            if (hasKey(curKey))
            {
                //as in readfromfile, ignore unknow keys
                m_oParameters.push_back(curParameter);
                m_oParamMap[curKey] = curParameter;
            }
        }
        
#ifndef NDEBUG
        //check that the hard coded parameter list agree with the actual content
        for (auto & oKey : m_oProcedureParameterKeys )
        {
            assert(hasKey(oKey));
        }
        
        for (auto & oKey : m_oProtectedParameterKeys)
        {
            assert(hasKey(oKey));
        }
#endif

        setModifiedFlag(false);
    }
    


	// -----------------------------  calibration filter graph parameters  --------------------------------

	void CalibrationParamMap::addCalibrationProcedureParameters()
	{
		m_oProcedureParametersModified = false;
		m_oProcedureParameterKeys.clear();

		auto completeParameterDefinition = [this] ()
		{
			const auto curParameter = this->m_oParameters.back();
			const auto curKey = curParameter->key();
			m_oParamMap[curKey] = curParameter;
			m_oProcedureParameterKeys.push_back(curKey);
		};


		//these parameters are only visualized on the device page
        //now they are computed by CalibrationData::getParameters()
        
		auto completeProtectedParameterDefinition = [this] ()
		{
            this->m_oParameters.back()->setReadOnly(true);
			const auto curParameter = this->m_oParameters.back();
			const auto curKey = curParameter->key();
			m_oParamMap[curKey] = curParameter;
			m_oProcedureParameterKeys.push_back(curKey);
			m_oProtectedParameterKeys.push_back(curKey);
			
		};
        
		for ( int intLaserLine = 0; intLaserLine < (int) filter::LaserLine::NumberLaserLines; intLaserLine++ )
		{
			auto suffix = CoaxCalibrationData::ParameterKeySuffix(filter::LaserLine(intLaserLine));
			m_oParameters.push_back(new interface::TKeyValue<double>("triangulationAngle" + suffix, 90.0, -90.0, 90.0, 90.0)); // Coax triangulation angle in Degree
			completeProtectedParameterDefinition();
            m_oParameters.back()->setPrecision(3);
		}
		

		// check consistency with default graph!!

		// calibration workpiece layer detection filter
		m_oParameters.push_back(new interface::TKeyValue<int>("layerExtend", 40, 1, 100, 40));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("intensityAvgThreshold", 0, -1, 250, 0));
		completeParameterDefinition();

		// Parallel Max filter: pixel intensity thresholds
		m_oParameters.push_back(new interface::TKeyValue<int>("thresholdTop", 90, 5, 250, 90));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("thresholdBot", 70, 5, 250, 70)); // probably not as well illuminated as top layer, thus lower default threshold
		completeParameterDefinition();

		// calibration result filter: number of adjacent pixels per layer necessary for successful calibration
		m_oParameters.push_back(new interface::TKeyValue<int>("pixPerLayer", 25, 5, 100, 25));
		completeParameterDefinition();

		// Parameter fuer die Wahl der ROIs fuer die Kalibrierung der 3 Linien:
		m_oParameters.push_back(new interface::TKeyValue<int>("XLeftEdge0", 40, 1, 1000, 40));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("XRightEdge0", 100, 1, 1000, 100));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("YTopEdge0", 40, 1, 1000, 40));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("YBottomEdge0", 100, 1, 1000, 100));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("XLeftEdgeTCP", 40, 1, 1000, 40));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("XRightEdgeTCP", 100, 1, 1000, 100));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("YTopEdgeTCP", 40, 1, 1000, 40));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("YBottomEdgeTCP", 100, 1, 1000, 100));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("XLeftEdge2", 40, 1, 1000, 40));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("XRightEdge2", 100, 1, 1000, 100));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("YTopEdge2", 40, 1, 1000, 40));
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("YBottomEdge2", 100, 1, 1000, 100));
		completeParameterDefinition();

		//these parameters are used only in the calibration procedure, but they are not strictly graph parameters
		m_oParameters.push_back(new interface::TKeyValue<double>("GrooveDepth", 2.0, 0, 10, 2.0)); // calibration piece groove depth in mm
		completeParameterDefinition();
        m_oParameters.back()->setPrecision(3);

		m_oParameters.push_back(new interface::TKeyValue<double>("GrooveWidth", 3.0, 0, 10, 3.0)); // calibration piece groove width in mm
		completeParameterDefinition();
        m_oParameters.back()->setPrecision(3);

       
        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel1ReferenceBrightness", 125, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel2ReferenceBrightness", 125, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel3ReferenceBrightness", 125, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel4ReferenceBrightness", 125, 0, 255, 125));
        completeParameterDefinition();
        
         m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel5ReferenceBrightness", 125, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel6ReferenceBrightness", 125, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel7ReferenceBrightness", 125, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel8ReferenceBrightness", 125, 0, 255, 125));
        completeParameterDefinition(); 

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel1MeasuredBrightness", 0, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel2MeasuredBrightness", 0, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel3MeasuredBrightness", 0, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel4MeasuredBrightness", 0, 0, 255, 125));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel5MeasuredBrightness", 0, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel6MeasuredBrightness", 0, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel7MeasuredBrightness", 0, 0, 255, 125));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("ledChannel8MeasuredBrightness", 0, 0, 255, 125));
        completeParameterDefinition(); 

        m_oParameters.push_back(new interface::TKeyValue<int>("ledCalibrationChannel", 0, 0, 4, 0));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("CameraBlackLevelShiftForLEDcalib", -100, -400, 400, -100));
        completeParameterDefinition();
        
        double sm_min = -1000;
        double sm_max =  1000;
        
        m_oParameters.push_back(new interface::TKeyValue<double>("SM_X_min", -50.0, sm_min, sm_max, -50.0, 2));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<double>("SM_X_max", 50.0,  sm_min, sm_max, 50.0, 2));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<double>("SM_Y_min", -50.0,  sm_min, sm_max, -50.0, 2));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<double>("SM_Y_max", 50.0,  sm_min, sm_max, 50.0, 2));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("SM_deltaX", 25.0,  sm_min, sm_max, 25.0, 2));
        completeParameterDefinition();
    
        m_oParameters.push_back(new interface::TKeyValue<double>("SM_deltaY", 25.0,  sm_min, sm_max, 25.0, 2));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("SM_IDMdeltaX", 5.0,  sm_min, sm_max, 5.0, 2));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("SM_IDMdeltaY", 5.0,  sm_min, sm_max, 5.0, 2));
        completeParameterDefinition();
         
        //parameters used for the scan field calibration graph
        
        m_oParameters.push_back(new interface::TKeyValue<int>("SM_searchROI_X", 100, 0, MAX_CAMERA_WIDTH, 100));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("SM_searchROI_Y", 100, 0, MAX_CAMERA_HEIGHT, 100));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("SM_searchROI_W", 800, 0, MAX_CAMERA_WIDTH, 800));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("SM_searchROI_H", 800, 0, MAX_CAMERA_HEIGHT, 800));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<int>("SM_searchGridBorder", 100, 0, 1000, 100)); 
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<int>("SM_searchGridSide", 80, 0, 100, 80)); 
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("SM_CircleRadius_pix", 150, 10, 400, 150));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("SM_EnclosingSquareSize_mm", 10.0,  1.0, 100.0, 10.0, 2));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<double>("SM_CircleRadius_mm", 3.5, 0.1, 100.0, 3.5, 2));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<bool>("SM_UseGridRecognition", true, false, true, true));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<bool>("SM_CalibrateAngle", true, false, true, true));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<int>("SM_CalibRoutineRepetitions", 5, 1, 100, 5)); 
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<bool>("SM_CalibRoutineMinimizeJump", false, false, true, false));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<int>("SM_CalibRoutineTrigger_ms", 30, 1, 1000, 30));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<int>("SM_DetectionTolerance_pix", 8, 0, 50, 8));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<bool>("IDM_AdaptiveExposureMode", true, false, true, true));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<int>("IDM_AdaptiveExposureBasicValue", 50, 0, 100, 50));
        completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<int>("ScheimpflugCameraModel", 0, 0, 3, 3));
        completeParameterDefinition();
        m_oParameters.push_back(new interface::TKeyValue<int>("ScheimpflugCalib_Threshold", -1, -1, 255, -1));
        completeParameterDefinition();
        m_oParameters.push_back(new interface::TKeyValue<double>("SM_ZCollDrivingRelative", 0, -50., 50., 0));
        completeParameterDefinition();
        m_oParameters.back()->setPrecision(3);
        m_oParameters.push_back(new interface::TKeyValue<int>("SM_LaserPowerForCalibration", 15, 0, 1000, 15));
        completeParameterDefinition();
        m_oParameters.push_back(new interface::TKeyValue<int>("SM_WeldingDurationForCalibration", 50, 1, 1000, 50));
        completeParameterDefinition();
        m_oParameters.push_back(new interface::TKeyValue<int>("SM_JumpSpeedForCalibration", 250, 1, 2000, 250));
        completeParameterDefinition();
	}

    bool CalibrationParamMap::isProtectedParamKey(const std::string & p_rKey) const
    {
        return std::find( m_oProtectedParameterKeys.begin(), m_oProtectedParameterKeys.end(), p_rKey) != m_oProtectedParameterKeys.end();
    }
    
    bool CalibrationParamMap::isProcedureParamKey (const std::string & p_rKey) const
    {
        return std::find( m_oProcedureParameterKeys.begin(), m_oProcedureParameterKeys.end(), p_rKey) != m_oProcedureParameterKeys.end();
    }
    
	bool CalibrationParamMap::hasModifiedParameters() const
	{

		return m_oProcedureParametersModified || m_oCalibrationParametersModified;
	}


	bool CalibrationParamMap::hasModifiedCalibrationParameter() const
	{

		return m_oCalibrationParametersModified;
	}

	size_t CalibrationParamMap::size() const
	{
		return m_oParamMap.size();
	}

	void CalibrationParamMap::updateModifiedFlag(const std::string &p_rKey)
	{

		if ( m_oProcedureParametersModified && m_oCalibrationParametersModified  )
		{
			//everything is already modified, I dont' need to check further (only saveToFile can reset the modified flag)
			return;
		}
		if (isProtectedParamKey(p_rKey))
        {
            return;
        }

		//Tests whether Key is one of the calibration graph paremeters
		if ( isProcedureParamKey (p_rKey))
		{
			m_oProcedureParametersModified = true;
		}
		else
		{	//this flag means that they have not been written to file
			m_oCalibrationParametersModified = true;
		}
	}

	void CalibrationParamMap::setModifiedFlag(bool val)
	{
		m_oProcedureParametersModified = val;
		m_oCalibrationParametersModified = val;


	}

	void CalibrationParamMap::initializeCalibrationKeyValues()
	{

		//see also ReloadKalibKeys in wmMain (CalibrationSetupViewModel.cs) and CoaxCalibrationData 
		m_oParameters.clear();
		m_oParamMap.clear();
        m_oProtectedParameterKeys.clear();
		setModifiedFlag(false);

		auto completeParameterDefinition = [this] ()
		{
			const auto curParameter = this->m_oParameters.back();
			const auto curKey = curParameter->key();
			m_oParamMap[curKey] = curParameter;
		};


		auto completeProtectedParameterDefinition = [this] ()
		{
            m_oParameters.back()->setReadOnly(true);
			const auto curParameter = this->m_oParameters.back();
			const auto curKey = curParameter->key();
			m_oParamMap[curKey] = curParameter;
			m_oProtectedParameterKeys.push_back(curKey);
		};


		m_oParameters.push_back(new interface::TKeyValue<double>("beta0", 0.5, 0.0, 10.0, 1.0)); // Abbildungsmassstab x-Richtung
		completeParameterDefinition();
        m_oParameters.back()->setPrecision(12);

		m_oParameters.push_back(new interface::TKeyValue<double>("betaZ", 0.5, 0.0, 10.0, 1.0)); // Abbildungsmassstab z-Richtung (Tiefe)
		completeParameterDefinition();
        m_oParameters.back()->setPrecision(12);

		m_oParameters.push_back(new interface::TKeyValue<double>("betaZ_2", 0.5, 0.0, 10.0, 1.0)); // Abbildungsmassstab2 (z.B. 2. LaserLinie) z-Richtung (Tiefe)
		completeParameterDefinition();
        m_oParameters.back()->setPrecision(12);

		m_oParameters.push_back(new interface::TKeyValue<double>("betaZ_TCP", 0.5, 0.0, 10.0, 1.0)); // Abbildungsmassstab2 (z.B. 2. LaserLinie) z-Richtung (Tiefe)
		completeParameterDefinition();
        m_oParameters.back()->setPrecision(12);

		m_oParameters.push_back(new interface::TKeyValue<int>("HighPlaneOnImageTop", 1, 0, 1, 1)); // Direction laser line. It triang angle is < 90 , higher Z are at the top of the image
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("HighPlaneOnImageTop_2", 1, 0, 1, 1)); // Direction laser line 2
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("HighPlaneOnImageTop_TCP", 1, 0, 1, 1)); // Direction laser line TCP
		completeParameterDefinition();

        for (std::string suffix : {"", "_2", "_TCP"})
        {
            double precision = 12;
            m_oParameters.push_back(new interface::TKeyValue<double>("laserLine_a"+suffix, 0.0, -10000.0, 10000.0, 0.0, precision));
            completeParameterDefinition();
            m_oParameters.push_back(new interface::TKeyValue<double>("laserLine_b"+suffix, 1.0, -10000.0, 10000.0, 1.0, precision));
            completeParameterDefinition();
            m_oParameters.push_back(new interface::TKeyValue<double>("laserLine_c"+suffix, 0.0, -10000.0, 10000.0, 0.0, precision));
            completeParameterDefinition();
        }


		m_oParameters.push_back(new interface::TKeyValue<double>("xtcp", 512, 0.0, 100000.0, 512)); // in pixel
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<double>("ytcp", 512, 0.0, 100000.0, 512)); // in pixel
		completeParameterDefinition();
        m_oParameters.push_back(new interface::TKeyValue<double>("ytcp_2", 512, 0.0, 100000.0, 512)); // reference y position on the second laser line
		completeParameterDefinition();
		m_oParameters.push_back(new interface::TKeyValue<double>("ytcp_tcp", 512, 0.0, 100000.0, 512)); // reference y position on the laser line going through the tcp
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<double>("xcc", 512, -1.0, 100000.0, -1)); // -1 -> autoset to sensorWidth/2. Origin
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<double>("ycc", 512, -1.0, 100000.0, -1)); // -1 -> autoset to sensorHeight/2. Origin
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<double>("DpixX", 10.6e-3, -100000.0, 100000.0, 10.6e-3));
		completeParameterDefinition();
        m_oParameters.back()->setPrecision(6);

		m_oParameters.push_back(new interface::TKeyValue<double>("DpixY", 10.6e-3, -100000.0, 100000.0, 10.6e-3));
		completeParameterDefinition();
        m_oParameters.back()->setPrecision(6);

		m_oParameters.push_back(new interface::TKeyValue<int>("sensorWidth", 1024, 0, 100000, 1024)); // in pixel
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<int>("sensorHeight", 1024, 0, 100000, 1024)); // in pixel
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<bool>("InvertX", false, false, true, false)); 
		completeParameterDefinition();
        
		m_oParameters.push_back(new interface::TKeyValue<bool>("InvertY", false, false, true, false)); 
		completeParameterDefinition();
        
		m_oParameters.push_back(new interface::TKeyValue<double>("axisCorrectionFactorY", 1.0, 0.0, 100.0, 1.0)); // Abbildungsmassstab z-Richtung (Tiefe)
		completeParameterDefinition();

		m_oParameters.push_back(new interface::TKeyValue<double>("scheimTriangAngle", 0.0, -180.0, 180.0, 0.0)); // Scheimpflug triangulation angle  todo: sensorID
		completeProtectedParameterDefinition();
        m_oParameters.back()->setPrecision(3);

        m_oParameters.push_back(new interface::TKeyValue<double>("scheimOrientationAngle", 0.0, -180.0, 180.0, 0.0)); //  angle for actual laser plane inclination (to be added to scheimTriangAngle)
		completeParameterDefinition();
        m_oParameters.back()->setPrecision(3);
        
		m_oParameters.push_back(new interface::TKeyValue<double>("distanceTcpToFrontLineInMm", 0.0, 0, 100000, 0.0)); // distance from tcp to front laser line in mm
		completeProtectedParameterDefinition();
        m_oParameters.back()->setPrecision(3);

		m_oParameters.push_back(new interface::TKeyValue<double>("distanceTcpToBehindLineInMm", 0.0, 0, 100000, 0.0)); // distance from tcp to behind laser line in mm
		completeProtectedParameterDefinition();
        m_oParameters.back()->setPrecision(3);
        
        m_oParameters.push_back(new interface::TKeyValue<int>("SensorModel", int(SensorModel::eUndefined), int(SensorModel::eSensorModelMin), int(SensorModel::eSensorModelMax), int(SensorModel::eUndefined))); 
		completeProtectedParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<int>("Debug", 0, 0, 1, 0)); // Additional debug messages
		completeParameterDefinition();
        
        m_oParameters.push_back(new interface::TKeyValue<bool>("SensorParametersChanged", false, false, true, false));
		m_oParameters.back()->setReadOnly(true);
		completeParameterDefinition();
        
        //parameters used to build the scanfield image
        {
            calibration::ScanMasterCalibrationData oDefaultScanMasterCalibration;
            for (auto kv : oDefaultScanMasterCalibration.makeConfiguration())
            {
                m_oParameters.push_back(kv);
                completeParameterDefinition();
            }            
        }        

        m_oParameters.push_back(new interface::TKeyValue<bool>("ScanfieldDistortionCorrectionEnable", false, false, true, false));
        completeParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kax1", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kax2", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kbx1", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kbx2", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kcx", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kdx", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kex", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kfx", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kay1", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kay2", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kby1", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kby2", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kcy", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kdy", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_key", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("ScanfieldDistortion_kfy", 0, -1000.0, 1000.0, 0.0, 15));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("IDM_k1", 0, -1000.0, 1000.0, 0.0, 6));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("IDM_k2", 0, -1000.0, 1000.0, 0.0, 6));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("IDM_k3", 0, -1000.0, 1000.0, 0.0, 6));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("IDM_k4", 0, -1000.0, 1000.0, 0.0, 6));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("IDM_l01", 0, -1000000.0, 1000000.0, 0.0, 3));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("IDM_l02", 0, -1000000.0, 1000000.0, 0.0, 3));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("IDM_l03", 0, -1000000.0, 1000000.0, 0.0, 3));
        completeProtectedParameterDefinition();

        m_oParameters.push_back(new interface::TKeyValue<double>("IDM_l04", 0, -1000000.0, 1000000.0, 0.0, 3));
        completeProtectedParameterDefinition();

		addCalibrationProcedureParameters();
		
	}

	bool CalibrationParamMap::exportToFile(const std::string oConfigFilename) const
	{
		bool success = false;
		wmLog(eDebug, "Export to file %s\n", oConfigFilename.c_str());
		try
		{
			File oConfigFile(oConfigFilename);
			interface::Configuration oTmpConf;

            oTmpConf.insert(oTmpConf.end(), m_oParameters.begin(), m_oParameters.end());
			
			interface::writeToFile(oConfigFilename, oTmpConf);
			success =  true;
		} // try
		catch ( ... )
		{
			system::logExcpetion(__FUNCTION__, std::current_exception());
		} // catch
		return success;
	} // save

	void CalibrationParamMap::setDouble(std::string p_oName, double p_oValue)
	{
		setValue<double>(p_oName, p_oValue);

	} // setDouble


	void CalibrationParamMap::print() const
	{
		std::ostringstream oMsg;
		for ( auto it(std::begin(m_oParamMap)); it != std::end(m_oParamMap); ++it )
		{
			oMsg.str("");
			oMsg << it->second->toString() << "\n";
			wmLog(eDebug, oMsg.str());
		}
	} // print

	interface::SmpKeyValue CalibrationParamMap::get(interface::Key p_oKey) const
	{
		const auto oFound(m_oParamMap.find(p_oKey));
		const auto oItMapEnd(m_oParamMap.end());

		if ( oFound != oItMapEnd )
		{
			//std::ostringstream oMsg;
			//oMsg << __FUNCTION__ << ":Key value get: '" << oFound->second->toString() << "'.\n";
			//wmLog(eDebug, oMsg.str());
            auto oResult = oFound->second->clone();
            return oResult;
		}
		else
		{
			std::ostringstream oMsg;
			oMsg << "ERROR" << __FUNCTION__ << ":Key '" << p_oKey << "' NOT found.";
			wmLog(eDebug, oMsg.str());
			wmLogTr(eWarning, "QnxMsg.Calib.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
			return new interface::KeyValue;
		} // else

	} // get

	/*
	interface::Configuration  CalibrationParamMap::get() const
	{
		return m_oParameters;
	} // get
*/
	
	bool CalibrationParamMap::hasKey(const std::string& key) const
	{
		return m_oParamMap.find(key) != m_oParamMap.end() ;
    }
    
    const std::vector<std::string> & CalibrationParamMap::getProcedureParameterKeys() const
    {
        return m_oProcedureParameterKeys;
    }
    
    std::vector<std::string> CalibrationParamMap::getScanMasterKeys(bool withCalibrationProcedureParameters) const
    {
        static const std::string scanmaster_prefix = "SM_";
        static const unsigned int prefix_length = scanmaster_prefix.length();
        std::vector<std::string> oKeys;
        
        const std::vector<std::string> oKeysToExclude = [&] ()
            {
                std::vector<std::string> result;
                if (!withCalibrationProcedureParameters)
                {            
                    for (auto & rKey: getProcedureParameterKeys() )
                    {
                        if  (rKey.compare (0, prefix_length, scanmaster_prefix)==0)
                        {
                            result.push_back(rKey);
                        }                
                    }
                }
                return result;
            }();
        
        const auto itEndKeysToExclude = oKeysToExclude.end();
        
        for (auto & rEntry: m_oParamMap )
        {
            auto rKey = rEntry.first;
            if  (rKey.compare (0, prefix_length, scanmaster_prefix)==0)
            {
                auto itFound = std::find(oKeysToExclude.begin(), itEndKeysToExclude, rKey);
                if (itFound == itEndKeysToExclude)
                {
                    oKeys.push_back(rKey);
                }
            }                
        }
        return oKeys;        
    }

	bool CalibrationParamMap::getBool(std::string p_oName) const
	{
        //assert(p_oName != "hasPrecomputedCoords");
        //assert(p_oName != "SensorModel");
        
		return getValue<bool>(p_oName);
	} // getInt

	int CalibrationParamMap::getInt(std::string p_oName) const
	{
		return getValue<int>(p_oName);
	} // getInt


	void CalibrationParamMap::setInt(std::string p_oName, int p_oValue)
	{
		setValue<int>(p_oName, p_oValue);
	}
	
	
    interface::SmpKeyValue CalibrationParamMap::prepareKeyValueForModification ( std::string p_oName, bool overrideProtected)
    {

        if ( !overrideProtected && isProtectedParamKey ( p_oName ) ) {
            std::ostringstream oMsg;
            oMsg << __FUNCTION__ << ": Could not set protected Key '" << p_oName << "\n";
            wmLog ( eInfo, oMsg.str() );
            return {};
        }

        const auto oFound ( m_oParamMap.find ( p_oName ) );
        const auto oItMapEnd ( m_oParamMap.end() );
        if ( oFound == oItMapEnd ) {
            std::ostringstream oMsg;
            oMsg << __FUNCTION__ << ":Key '" << p_oName << "' NOT found." << "\n";
            wmLog ( eInfo, oMsg.str() );
            return {};
        }
        updateModifiedFlag ( p_oName );
        return oFound->second;
    }
	
    template <> void CalibrationParamMap::setValue<std::string>(std::string p_oName, std::string p_oValue, bool overrideProtected) 
    {
        
        auto oSmpKey = prepareKeyValueForModification(p_oName, overrideProtected);
        if (oSmpKey.isNull())
        {
            return;
        }

        const Types oType(oSmpKey->type());
        
        switch ( oType )
        {
            case TString: 
                    oSmpKey->setValue<std::string>(p_oValue);
                break;
            default:
                std::ostringstream oMsg;
                oMsg << __FUNCTION__ << ": invalid value type: " << oType << "\n";
                wmLogTr(eWarning, "QnxMsg.Calib.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
                break;
        } // switch 
    }

    
    

	template <typename tKeyType>
	tKeyType CalibrationParamMap::getValue(std::string p_oName) const
	{

		const auto oFound(m_oParamMap.find(p_oName));
		const auto oItMapEnd(m_oParamMap.end());

		if ( oFound != oItMapEnd )
		{
			return get(p_oName)->value<tKeyType>();
		}
		else
		{
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << ":Key '" << p_oName << "' NOT found.";
			wmLog(eDebug, oMsg.str());
			wmLogTr(eWarning, "QnxMsg.Calib.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());

		} // else

		return tKeyType(0);
	}
	
	
	double CalibrationParamMap::getDouble(std::string p_oName) const
	{
		return getValue<double>(p_oName);
	} // getDouble

	template<typename T> // any nullable type...
	bool CalibrationParamMap::getTagValue(T &p_rValue, Poco::XML::AutoPtr<Poco::XML::Document> &p_rDoc, std::string p_oTag) const
	{
		p_rValue = 0;
		Poco::XML::AutoPtr<Poco::XML::NodeList> pNodeAxisList = p_rDoc->getElementsByTagName(p_oTag);
		if ( pNodeAxisList->length() == 0 )
		{
			wmLogTr(precitec::eWarning, "QnxMsg.Calib.NoGridEl", "Calibration grid data %s not found!", p_oTag.c_str());
			return false;
		}
		Poco::XML::AutoPtr<Poco::XML::Element> pElement = (Poco::XML::Element*)pNodeAxisList->item(0);
		if ( !from_string<T>(p_rValue, pElement->nodeValue(), std::dec) )
		{
			wmLogTr(precitec::eWarning, "QnxMsg.Calib.BadGridVal", "Invalid value for grid data %s!", p_oTag.c_str());
			return false;
		}
		return true;
	}


	bool CalibrationParamMap::setFile(const std::string oConfigFilename)
	{
		if ( m_oConfigFilename.length() > 0 )
		{
			if ( m_oConfigFilename == oConfigFilename )
			{
				wmLog(eWarning, "The file was already set");
			}
			else
			{
				wmLog(eWarning, "A different file was assigned!");
			}
		}
		m_oConfigFilename = oConfigFilename;
		return saveToFile();
	}
	const std::string & CalibrationParamMap::getConfigFilename() const
	{
        return m_oConfigFilename;
    }


	bool CalibrationParamMap::readParamMapFromFile(const std::string oConfigFilename)
	{
        //initialized with default values, they will be updated from the file
        assert(size() > 0);
        
		wmLog(eInfo, "Reading calibration parameters from file "+ oConfigFilename + "\n");
		m_oConfigFilename = oConfigFilename;
		bool res = true;
		AutoPtr<Util::XMLConfiguration> pConfIn;
		try
		{// poco syntax exception might be thrown or sax parse excpetion
			pConfIn = new Util::XMLConfiguration(oConfigFilename);
		}
		catch ( const Exception &p_rException )
		{
			res = false;
			std::cout << "XML configuration kann nicht erstellt werden... " << std::endl;
			wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
			wmLog(eDebug, "Could not read parameters from file:\n");
			wmLog(eDebug, oConfigFilename + "\n" );
			wmLog(eWarning,"An error occurred in the procedure '%s'. Message: '%s'. Calibration file %s could not be read.\n", __FUNCTION__, p_rException.message().c_str(), oConfigFilename);
			m_oConfigFilename.clear();
			return res;
		} // catch


		for ( auto it(std::begin(m_oParamMap)); it != std::end(m_oParamMap); ++it )
		{
			//std::cout<< "Parameter " << it->second->key().c_str()<<std::endl;

			const Types oType(it->second->type());
			const std::string oKey(it->second->key());
			try
			{
				switch ( oType )
				{
					case TInt:
						it->second->setValue(pConfIn->getInt(oKey, it->second->value<int>())); // assign value
						//std::cout<<"it TInt: "<<it->second->value<int>()<<std::endl;
						break;
					case TDouble:
						it->second->setValue(pConfIn->getDouble(oKey, it->second->value<double>())); // assign value
						break;
                    case TBool:
                        it->second->setValue(pConfIn->getBool(oKey, it->second->value<bool>())); // assign value
                        break;
                    case TString:
                        it->second->setValue(pConfIn->getString(oKey, it->second->value<std::string>()));
                        break;
					default:
						std::ostringstream oMsg;
						oMsg << "Invalid value type: '" << oType << " for key " << oKey << "'\n";
						wmLogTr(eWarning, "QnxMsg.Calib.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
						break;
				} // switch
			} // try
			catch ( const Exception &p_rException )
			{
				wmLog(eDebug, "%s - '%s': %s\n", __FUNCTION__, p_rException.name(), p_rException.message().c_str());
				std::ostringstream oMsg;
				oMsg << "Parameter '" << oKey.c_str() << "' of type '" << oType << "' could not be converted. Reset to default value.\n";
				wmLog(eDebug, oMsg.str());
				wmLogTr(eWarning, "QnxMsg.Calib.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
				res = false;
			} // catch
		} // for
		return res;
	}



	bool CalibrationParamMap::saveToFile()
	{
		if ( m_oConfigFilename.length() == 0 )
		{
			wmLog(eWarning, "Parameter map was not initialized with a valid file");
			return false;
		}

		std::cout << "Saving to " << m_oConfigFilename << std::endl;
		bool res = exportToFile(m_oConfigFilename);
		if ( res )
		{
			setModifiedFlag(false);
		}
		return res;


	} // save

	bool CalibrationParamMap::syncXMLContent(std::string & rXMLContent)
	{
		if ( !saveToFile() )
		{
			return false;
		}
		return system::readXMLConfigToString(rXMLContent, m_oConfigFilename);
	}

	interface::KeyHandle CalibrationParamMap::set(interface::SmpKeyValue p_oSmpKeyValue, bool overrideProtected)
	{

		const auto &rKey(p_oSmpKeyValue->key());

        
        const auto oFound(m_oParamMap.find(rKey));
		const auto oItMapEnd(m_oParamMap.end());

		if ( oFound != oItMapEnd )
		{
            if (!overrideProtected && isProtectedParamKey(rKey))
            {
                wmLog(eWarning, "The protected parameter %s cannot be set\n", rKey.c_str());
                return oFound->second->handle();
            }
            
			//the wmMain tcp page sends invalid key-value objects, therefore the following code was commented out ...
			//if (p_oSmpKeyValue->isValueValid() == false)
			//{
			//	return -1; // value invalid, ignore it.
			//}
			const Types oType(oFound->second->type());
			updateModifiedFlag(p_oSmpKeyValue->key()); //also in case of error the key will be overwritten

			switch ( oType )
			{
				case TInt: {
					// assign value
					oFound->second->setValue(p_oSmpKeyValue->value<int>());
					break;
				} // case
				case TDouble:
					// assign value
					oFound->second->setValue(p_oSmpKeyValue->value<double>());
					break;
				case TBool:
					// assign value
					oFound->second->setValue(p_oSmpKeyValue->value<bool>());
					break;
				case TString:
					// assign value
					oFound->second->setValue(p_oSmpKeyValue->value<std::string>());
					break;
				default:
					std::ostringstream oMsg;
					oMsg << __FUNCTION__ << ": invalid value type: " << oType;
					wmLog(eDebug, oMsg.str()); std::cerr << oMsg.str();
					wmLogTr(eWarning, "QnxMsg.Calib.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
					break;
			} // switch


			// key was set successfully
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << ": Key value set: '" << p_oSmpKeyValue->toString() << "'.\n";
			wmLog(eDebug, oMsg.str());
			// save to file

			return oFound->second->handle(); // return handle

		} // if
		else
		{
			std::ostringstream oMsg;
			oMsg << __FUNCTION__ << ":Key '" << p_oSmpKeyValue->key() << "' NOT found.";
			wmLogTr(eWarning, "QnxMsg.Calib.ProcNonCritical", "A non-critcal  error occurred in the procedure '%s'. Message: '%s'. See log file.\n", __FUNCTION__, oMsg.str().c_str());
			return -1;
		} // else


	} // set


	
	

std::string CalibrationParamMap::getString ( std::string p_oName ) const
{
    return getValue<std::string> ( p_oName );
}


void CalibrationParamMap::setString ( std::string p_oName, std::string p_oValue )
{
    setValue<std::string> ( p_oName, p_oValue );
}



}
}
