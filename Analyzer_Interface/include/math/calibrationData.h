/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB), Andreas Beschorner (BA)
 * 	@date		2013
 * 	@brief		Stores and distributes the calibration parameters.
 */

#ifndef CALIBRATION_DATA_H_
#define CALIBRATION_DATA_H_

// stl includes
#include <string>
#include <vector>
#include <ostream>
#include <stdint.h>

#include <Analyzer_Interface.h>

// project includes
#include <message/device.h> //interface::key

#include "filter/parameterEnums.h" //filter::LaserLine
#include "math/calibrationCommon.h"
#include "math/calibrationStructures.h"
#include "math/CalibrationParamMap.h"
#include "math/calibration3DCoords.h"
#include "util/camGridData.h"
#include "common/calibrationConfiguration.h"
#include "util/CalibrationCorrectionContainer.h"

namespace precitec {
namespace math {


/**
 * @ingroup Analyzer_Interface
 * @brief Stores and distributes the calibration parameters. It is created and manipulated by the calibration module and later accessed by the analyzer.
 */
class ANALYZER_INTERFACE_API CalibrationData
{

public:

	/**
	 * @brief CTor
	 */
    CalibrationData(SensorId p_oSensorID);
	CalibrationData(SensorId p_oSensorID, bool pCanWriteToDisk, std::string pHomeDir);
	/**
	 * @brief DTor.
	 */
	~CalibrationData();

    void resetConfig();
    void resetConfig(bool pCanWriteToDisk, std::string pHomeDir);
    //read configuration, sets sensor model
    void initConfig(const SensorModel p_oSensorModel, bool createDefault);

	//load the sensor if it's not yet initialized, checks that the instance corresponds to the correct sensor 
    //covenience function for Filtertest and InspectManager::loadCalibDataFromDisk (simulation linux)
	bool loadFromConfigFolder(bool recomputeFromCamGridData, bool useOnlyBinaryCache);
    
	//change calibrationdata instance accoridng to arguments
    bool reload(math::Calibration3DCoords p_o3DCoords, math::CalibrationParamMap p_pCalibrationParameters); 
    
    // change sensor and pixel size,  beta0 is updated accordingly
    void changeCameraRelatedParameters(const CameraRelatedParameters & cameraParameters, bool recomputeBetaOnSensorParameterChanged = false);
    bool checkCameraRelatedParameters(const CameraRelatedParameters & cameraParameters) const;

    
	SensorId getSensorId() const;
	SensorModel getSensorModel() const;
	bool isInitialized() const;

    const std::string & getHomeDir() const; 
	const std::string & getConfigFilename() const; 
    const coordinates::CalibrationConfiguration & getFilenamesConfiguration() const;


	void showData(std::ostream & out = std::cout) const;

	/**
	 * @brief Check consistency of calibration parameters (for all line lasers) in CalibrationGrid and ParamMap
	 */
	bool checkCalibrationValuesConsistency(double tol, LogType logLevel, bool checkCoordinates) const;
	bool checkCalibrationValuesConsistency() const;

	template <typename tKeyType>
	void setKeyValue(const interface::Key p_oKey, const tKeyType p_oValue, bool overwriteProtected = false);

    template <typename T>
    bool isValueInRange(const interface::Key& key, const T value)
    {
        const auto keyValue = m_oParameters.get(key);
        return (value <= keyValue->maxima<T>()) && (value >= keyValue->minima<T>());
    }
    
	interface::KeyHandle setKeyValue(interface::SmpKeyValue keyValue);

    void print3DCoords(const geo2d::Point & pRoiOrigin, const geo2d::Size & pRoiSize) const;

	/**
	 * @brief return axis y direction correction factor (if axis is not moving absolutely straight, this returns a value <> 1.0, 1.0 otherwise.
	 */
	double getAxisYFactor() const
	{
		return m_oParameters.getDouble("axisCorrectionFactorY");
	}


	CoaxCalibrationData getCoaxCalibrationData() const;
    geo2d::DPoint getTCPCoordinate(int p_oIdxWorker, filter::LaserLine p_laserLine) const; // for InspectManager and filters
    geo2d::DPoint getTCPCoordinate(const interface::ScannerContextInfo & rScannerContext, filter::LaserLine p_laserline) const; // for CalibrationManager and CalibrationCoordinatesRequest interface
    interface::ScannerContextInfo getCurrentScannerInfo(int p_oIdxWorker) const;

    bool scanfieldDistortionCorrectionFactor(std::vector<double>& k) const;
    std::pair<std::vector<double>, std::vector<double>> idmModel() const;

    LEDCalibrationData getLEDCalibrationData() const;

	/**
	 * @brief One of the main functions for creating the 3D image data for Scheimpflug systems.
	 * @param p_pImgData	The image / cam test image from the cam holding data created by the QT wmCalibration app.
	 * @param p_oSensorID	The sensorID ( 0..2 ) as an int.
	 * @return string       Empty if all was OK, otherwise the string holds the error description (for instance if the image does not have valid calibration data).
	 *
	 * Called from Mod_Calibration/src/calibrate.cpp using an internal reference to the calibrationManager, this functions is responsible
	 * for the process of creating the images holding the 3D data for Scheimpflug systems.
	 */
	bool load3DFieldFromCamGridData(const system::CamGridData & pCamGridData);

    bool load3DFieldFromParameters(); //coax, mod_calibration
    bool updateTriangulationAngles(); 

    math::LineEquation getLaserLineAtZCollimatorHeight(double z_mm, const interface::ScannerContextInfo & rScannerContext, filter::LaserLine p_laserline);


	bool hasData() const; ///< Checks whether there is Scheimpflug 3D data available for sensor p_oSensorID at all

    const math::Calibration3DCoords & getCalibrationCoords() const; ///< Returns Calibration3DCoords for the current sensor (the same as calibrationGridsingleton)
    math::Calibration3DCoords & getCalibrationCoordsReference();

	static bool validSensorID(const int p_oSensorID) ; ///< Checks whether p_oSensorID is in [0, 2]
	geo2d::Size getSensorSize() const;

    //returns a copy of the parameters (+ computed values)
	const math::CalibrationParamMap & getParameters() const;
    //returns a copy of the parameters (+ computed values)
	interface::Configuration makeConfiguration(bool onlyFirstLine, bool withCalibrationProcedureParameters) const;
    
    //compute derived values, save to file and return file content
	std::string syncXMLContent();

    bool setSensorModel(SensorModel pModel); 
    //actually is used only by calibraiton manager
	void setFallbackParameters(bool syncConfigFile); ///< If Scheimpflug calibration data is invalid, Coax fallback is activated in thus function. Used during startup.
    
    void resetCalibrationCorrection();
    void setCalibrationCorrectionContainer(coordinates::CalibrationCameraCorrectionContainer correction_initialization_arguments);
    bool updateCameraCorrection(double scanner_x,  double scanner_y, int p_oIdxWorker);
    bool hasCameraCorrectionGrid() const;
    coordinates::CalibrationCameraCorrectionContainer getCorrectionGridParameters() const;
    
    void setCalibrationIDMCorrectionContainer(coordinates::CalibrationIDMCorrectionContainer idmcorrection);
    bool hasIDMCorrectionGrid() const;
    int getCalibratedIDMWeldingDepth(double scanner_x,  double scanner_y, int raw_idmvalue) const;
    coordinates::CalibrationIDMCorrectionContainer getIDMCorrectionGridParameters() const;

    math::LineEquation getZLineForOrientedLaserLine (filter::LaserLine p_laserline, double z_mm) const;
    geo2d::DPoint getZPointForOrientedLaserLine (filter::LaserLine p_laserline) const;
    void adjustZPointForOrientedLaserLine (geo2d::DPoint,filter::LaserLine p_laserline);

    static const double CALIB_VALUES_TOL_MIN;

protected:


	/**
	* @brief Load a parameter set from the default xml configuration file.
	*/
    bool loadConfig(bool useSensorModelFromConfig, bool createDefault);
    void setHomeDirectory(std::string pHomeDir);
	
	CalibrationParamMap  m_oParameters;
	bool m_o3DFieldModified;  ///< flag = true if m_o3DCoords needs update
	
	const SensorId m_oSensorID;
    SensorModel m_oSensorModel;
	bool m_oInitialized;
    Calibration3DCoords m_o3DCoords;
    
	// init and setup
	coordinates::CalibrationConfiguration m_oFilenamesConfiguration;
	bool m_canWriteToDisk;
	
    //optional field
    //it can't be a unique_ptr because otherwise the copy constructor is deleted and CalibDataSingleton::m_oCalData can't be initialized (seems similar to https://gcc.gnu.org/bugzilla/show_bug.cgi?id=63707 ?)
	std::shared_ptr<coordinates::CalibrationCameraCorrectionState>  m_pCameraCorrectionState; 
    coordinates::CalibrationIDMCorrectionContainer m_oIDMCorrectionContainer;
	
	
};




} // math
} // precitec

#endif
