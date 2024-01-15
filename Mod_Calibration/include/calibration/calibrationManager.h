/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB), Andreas Beschorner (AB)
 * 	@date		2013
 * 	@brief		Manages and executes calibration procedures.
 */

#ifndef CALIBRATIONMANAGER_H_
#define CALIBRATIONMANAGER_H_

// Poco includes
#include <Poco/Mutex.h>
#include <Poco/Semaphore.h>
// stl includes
#include <queue>
#include <memory>
// project includes
#include <Mod_Calibration.h>
#include <common/frame.h>
#include "overlay/overlayCanvas.h"
#include <event/triggerCmd.proxy.h>
#include <event/recorder.proxy.h>
#include <message/device.proxy.h>
#include <math/calibrationData.h>
#include <message/weldHead.interface.h>
#include <message/weldHead.proxy.h>
#include <message/db.proxy.h>
#include <message/calibDataMessenger.proxy.h>
#include <message/calibrationCoordinatesRequest.h> // OCT_Mode

#include <optional>

namespace precitec {

/**
 * @brief In this namespace all the calibration related classes are organized - the CalibrationManager is the main class that controls and executes the calibration procedures.
 */
namespace calibration {

class CalibrateIbAxis;
class CalibrateIbOpticalSystem;
class CalibrateLEDIllumination;
class CalibrationOCTData;
class CalibrationOCTMeasurementModel;
class CalibrationOCT_TCP;
class CalibrateScanField;

enum class OCTApplicationType
{
    NoOCT, OCTEnabled, OCTTrackApplication
};

/**
 * @ingroup Calibration
 * @brief Executes and provides a basic framework for the calibration procedures.
 *
 * @details The CalibrationManager is the central instance in the calibration module, controlling the execution of the calibration procedures. In addition, it provides a simple and convenient interface
 * to all the hardware components that might be of interest to the calibration routines.
 * A single calibration manager object is created in App_Calibration.
 */
class MOD_CALIBRATION_API CalibrationManager
{

public:

	/**
	 * @brief CTor.
	 * @param p_rResultProxy Reference to the result proxy - used to control the weld-head.
	 * @param p_rTriggerCmdProxy Reference to the trigger-cmd proxy - used to send single trigger impulses to the grabber.
	 * @param p_rRecorderProxy Reference to the recorder proxy - used to send images to the wmMain.
	 * @param p_rDeviceProxy Reference to the device proxy - used to set and get parameters of the camera.
	 * @param p_rWeldHeadMsgProxy Reference to the weld-head message proxy - used to control the weld-head axis.
	 */
	CalibrationManager(
						interface::TTriggerCmd<interface::AbstractInterface>& p_rTriggerCmdProxy,
						interface::TRecorder<interface::AbstractInterface>& p_rRecorderProxy,
						interface::TDevice<interface::AbstractInterface>& p_rCameraDeviceProxy,
						interface::TWeldHeadMsg<interface::AbstractInterface>& p_rWeldHeadMsgProxy,
						interface::TCalibDataMsg<interface::MsgProxy>& p_rCalibDataMsgProxy,
						interface::TDevice<interface::AbstractInterface>& p_rIDMDeviceProxy,
						interface::TDevice<interface::AbstractInterface>& p_rWeldHeadDeviceProxy,
						OCTApplicationType p_oOCTTRackApplication
					  );

	/**
	 * @brief DTor.
	 */
	virtual ~CalibrationManager();

	/**
	 * @brief Start a specific calibration routine / method.
	 * @param p_oMethod Which calibration procedure should be executed, e.g. eTriangulation.
	 * @return true if the calibration was successful.
	 */
	bool calibrate( int p_oMethod );

	/**
	 * @brief A single image has arrived - is called by the sensor.
	 * @param p_oSensorId Sensor ID of the sensor that sends the data.
	 * @param p_rTriggerContext Reference to triggerContext coming from the sensor.
	 * @param p_rData Reference to BImage object with the actual image.
	 */
	void image(int p_oSensorId, interface::TriggerContext const& p_rTriggerContext, image::BImage const& p_rData);

	/**
	 * @brief Inspect a single analog sample - is called by the sensor.
	 * @param p_oSensorId Sensor ID of the sensor that sends the data.
	 * @param p_rTriggerContext Reference to triggerContext coming from the sensor.
	 * @param p_rData Reference to Sample object with the actual data.
	 */
	void sample(int p_oSensorId, interface::TriggerContext const& p_rTriggerContext, image::Sample const& p_rData);

	/**
	 * @brief Is the calibration still running?
	 * @return true if the calibration is still running.
	 */
	bool isRunning() { return m_oRunning; };
    
    void cancelTrigger(const std::vector<int>& p_rSensorIds);

	bool determineTriangulationAngle(const int p_oSensorID, const math::CameraRelatedParameters & rCameraParameters );
    
    
	/**
	 * @name Utility
	 * Basic utility functions, for example to receive an image from the camera, sample data values, etc.
	 */
	//@{

	/**
	* @brief Get an image - this function will block until an image is received or a certain time has passed. If the routine is not able to get an image, it will throw an exception.
	* @return BImage object of the new image.
	*/
	BImage getImage();

	/**
	* @brief Returns current image, that is the one received from any previous getImage() call. CAVE! Empty images possible...
	* @return BImage object of the new image.
	*/
	image::BImage getCurrentImage();

	/**
	* @brief Get a sample - this function will block until a sample with a certain sensorId is received or a certain time has passed. If the routine is not able to get a sample, it will throw an exception.
	* @param p_oSensorId the sensor ID the routine is looking for. The function will wait until a sample of this sensor is received.
	* @return sample of the sensor.
	*/
	image::Sample getSample( int p_oSensorId);

	/**
	* @brief Get samples - this function will block until numSamples samples with a certain sensorId are received or a certain time has passed. If the routine is not able to get the samples, it will throw an exception.
	* @param p_oSensorId the sensor ID the routine is looking for. The function will wait until a sample of this sensor is received.
	* @return samples vector of the sensor.
	*/
	std::vector<image::Sample> getSamples ( int p_oSensorId, int numSamples, unsigned int trigger_ms, unsigned int timeout_ms);

	/**
	* @brief Get trigger context - this function returns the latest trigger context. Should only be called after a successful call to getImage() or getSample().
	* @return Latest trigger context associated to last call to getImage()/getSample().
	*/
	interface::TriggerContext getTriggerContext();

	void setIndexForNextImageFromDisk(int index = 0);
	
	//@}

	/**
	 * @name Weldhead
	 * Functions to control the weldhead axis.
	 */
	//@{

	/**
	 * @brief Set position of weld-head.
	 * @param p_oYpos New position.
	 * @param p_oAxis axis which position is set
	 */
	void setHeadPos( int p_oYpos, interface::HeadAxisID p_oAxis = interface::eAxisY );
	/**
	 * @brief Get weld-head position.
	 * @param p_oAxis axis which position is requested
	 */
	int getHeadPos( interface::HeadAxisID p_oAxis = interface::eAxisY );
	/**
	 * @brief Set weld-head axis mode.
	 * @param p_oAxis axis which mode is set
	 * @param p_oMode new mode, e.g. Position
	 * @param p_oHome homing
	 */
	void setHeadMode( interface::HeadAxisID p_oAxis, interface::MotionMode p_oMode, bool p_oHome );
	/**
	 * @brief Do homing of Z-Collimator.
	 */
	void doZCollHoming( void );
	/**
	 * @brief Do relative driving of Z-Collimator, relative to homing position
	 * @param value new relative position to middle of operating range in um for focus position, value = 0 is ident to CenterPosition
	 */
	void doZCollDrivingRelative(int value);

	//@}

	/**
	 * @name Canvas
	 * Canvas interface, which can be used to render an image and simple drawing primitives.
	 */
	//@{

	/**
	 * @brief Draw a line.
	 * @param p_oX0 X position of start point.
	 * @param p_oY0 Y position of start point.
	 * @param p_oX1 X position of end point.
	 * @param p_oY1 Y position of end point.
	 * @param p_oColor Color of line (e.g. Color::Red()).
	 */
	void drawLine( int p_oX0, int p_oY0, int p_oX1, int p_oY1, Color p_oColor);

	/**
	 * @brief Draw a cross.
	 * @param p_oX X position of cross.
	 * @param p_oY Y position of cross.
	 * @param p_oSize Size of cross.
	 * @param p_oColor Color of cross (e.g. Color::Red()).
	 */
	void drawCross( int p_oX, int p_oY, int p_oSize, Color p_oColor);
	/**
	 * @brief Draw a point / pixel.
	 * @param p_oX X position of point.
	 * @param p_oY Y position of point.
	 * @param p_oColor Color of pixel (e.g. Color::Red()).
	 */
	void drawPixel( int p_oX, int p_oY, Color p_oColor);
	/**
	 * @brief Draw a rectangle.
	 * @param p_oX X position of rectangle (top-left corner).
	 * @param p_oY Y position of rectangle (top-left corner).
	 * @param p_oW Width of rectangle.
	 * @param p_oH Height of rectangle.
	 * @param p_oColor Color of rectangle (e.g. Color::Red()).
	 */
	void drawRect( int p_oX, int p_oY, int p_oW, int p_oH, Color p_oColor);

	/**
	 * @brief Draw a textstring of fixed size 11.
	 * @param p_oX X position.
	 * @param p_oY Y position.
	 * @param p_oText Text to write.
	 * @param p_oColor Color of text.
	 */
	void drawText( int p_oX, int p_oY, std::string p_oText, Color p_oColor);
    
	/**
	 * @brief Draw a image overlay.
	 * @param p_oX X position.
	 * @param p_oY Y position.
	 * @param p_oImage image.
	 * @param p_oText Text to write.
	 * @param p_oColor Color of text.
	 */
	void drawImage( int p_oX, int p_oY, BImage oImage, std::string p_oText, Color p_oColor);

	/**
	 * @brief Render an image and send it to windows. This function needs to be called after all the drawXXX primitive calls are done and the canvas is ready to be rendered on the GUI.
	 * @param p_rImage Reference to BImage object.
	 */
    void renderCanvas();
	void renderImage(const BImage& p_rImage);
	void renderImage(const BImage & p_rImage, interface::ImageContext ctx);

	/**
	 * @brief Clear the drawing canvas.
	 */
	void clearCanvas();

	/**
	 * @brief Get the canvas object.
	 * @return shared_ptr to the overlay / canvas object.
	 */
	std::shared_ptr<OverlayCanvas> getCanvas();

	//@}

	/**
	 * @name CalibrationParameters
	 * The interface to get and set calibration parameters.
	 */
	//@{

	/**
	 * @brief Get the calibration data object.
	 * @return Reference to the calibration data object.
	 */
	math::CalibrationData& getCalibrationData(unsigned int p_oSensorId);
	const math::CalibrationData& getCalibrationData(unsigned int p_oSensorId) const;

	//@}

	/**
	 * @name Camera
	 * The camera interface, with which one can control camera parameters like the exposure time.
	 */
	//@{

	/**
	 * @brief Get the exposure time.
	 * @param p_iDevice Device number (default: 0) of the camera.
	 * @return float with the exposure time.
	 */
	float getExposureTime( int p_iDevice =0 );
	int getTestimageEnabled(int p_oDevice = 0);

    bool getOSCalibrationDataFromHW(const int p_oSensorID);
    math::SensorModel getExpectedSensorModelFromSystemConfig() const;

	void setTestimageEnabled(int p_oEnable, int p_oDevice = 0);

	/**
	 * @brief Set the exposure time.
	 * @param p_iDevice Device number (default: 0) of the camera.
	 * @param p_oExposureTime float with the exposure time.
	 */
	void setExposureTime( float p_oExposureTime, int p_iDevice =0);

	/**
	 * @brief Get the black-level offset.
	 * @param p_iDevice Device number (default: 0) of the camera.
	 * @return The offset.
	 */
	int getBlackLevelOffset( int p_iDevice =0 );
	/**
	 * @brief Set the black-level offset.
	 * @param p_iDevice Device number (default: 0) of the camera.
	 * @param p_oBlackLevelOffset int containing the new offset.
	 */
	void setBlackLevelOffset( int p_oBlackLevelOffset, int p_iDevice =0);

	/**
	 * @brief Get the ROI x0
	 * @param p_iDevice Device number (default: 0) of the camera.
	 * @return The offset.
	 */
	int getROI_X0( int p_iDevice =0 );

	/**
	 * @brief Get the ROI y0
	 * @param p_iDevice Device number (default: 0) of the camera.
	 * @return The offset.
	 */
	int getROI_Y0( int p_iDevice =0 );

	void setHWRoi(const int p_rX0, const int p_rY0, const int p_rWidth, const int p_rHeight, const int p_oDevice=0);
	bool getHWRoi(int &p_rX0, int &p_rY0, int &p_rWidth, int &p_rHeight, const int p_oDevice=0);
	int setROI(int x0,int y0,int dx,int dy, int p_iDevice =0 );
	void setROI_X0( int ivalue,  int p_iDevice =0  );
	void setROI_Y0( int ivalue,  int p_iDevice =0  );
	void setROI_dX( int ivalue,  int p_iDevice =0  );
	void setROI_dY( int ivalue,  int p_iDevice =0  );
    void setFullHWROI(int p_iDevice = 0);
	
    std::string checkCamera(int p_oDevice, int maxTrials = 3);
    std::vector<std::uint8_t> readUserFlashData(int start, int num, int p_oDevice = 0);
    bool writeUserFlashData(std::vector<std::uint8_t> data);

    math::CameraRelatedParameters readCameraRelatedParameters(int oSensorID = 0) const;
    
	//@}
    

	/**
	 * @name Analyzer interface
	 */
	//@{
    bool isSimulation() const {return m_isSimulation;}
    /**
    * @brief Make sure that the internal calibration coordinates field is updated and send it to the Analyzer
    */
	void sendCalibDataChangedSignal(int p_oSensorID, bool p_oInit);

    void configureAsSimulationStation(bool set);
    //used by interface TCalibrationCoordinatesRequest
    bool get3DCoordinates(math::Vec3D & rCoords, unsigned int pX, unsigned int pY, math::SensorId p_oSensorID, filter::LaserLine p_LaserLine);
    geo2d::DPoint getCoordinatesFromGrayScaleImage(double xScreen, double yScreen, const interface::ScannerContextInfo & rContextInfo, math::SensorId p_oSensorID);
	//@}
    
    /**
	 * @name IDM
	 * Functions to control IDM and Newson
	 */
	//@{

    bool isOCTEnabled() const { return m_oOCTApplicationType == OCTApplicationType::OCTEnabled || m_oOCTApplicationType == OCTApplicationType::OCTTrackApplication ; }
    bool isOCTTrackApplication() const { return m_oOCTApplicationType == OCTApplicationType::OCTTrackApplication; }
    interface::KeyHandle setIDMKeyValue(interface::SmpKeyValue p_keyValue, int p_subDevice=0);
    interface::SmpKeyValue getIDMKeyValue(interface::Key p_key, int p_subDevice=0);
    
	/**
	* @brief Returns samples collected in the TCP Search procedure of the OCT system. If any error occured, the output vector is empty
	*/
	std::vector<image::Sample> getTCPSearchSamples();
    

    bool getOCTScanWide()  { return m_oOCTScanWide; }
    void setOCTScanWide(bool p_oValue) {m_oOCTScanWide = p_oValue; }
    bool updateOCTCalibrationData(CalibrationOCTMeasurementModel p_OCTMeasurementModel);
    bool updateOCTCalibrationData(CalibrationOCTMeasurementModel p_OCTMeasurementModel, interface::Configuration p_oKeyValues);
    interface::Configuration getCalibrationOCTConfiguration() const;
    interface::SmpKeyValue getCalibrationOCTKeyValue(std::string p_key) const;
    interface::KeyHandle setCalibrationOCTKeyValue(interface::SmpKeyValue p_keyValue, bool p_updateSystem);
    const CalibrationOCTData *  getCalibrationOCTData() const;
    void setZCollDrivingRelative(double value);
    double getZCollDrivingRelative() const;
    void setLaserPowerInPctForCalibration(double value);
    double getLaserPowerInPctForCalibration() const;
    void setWeldingDurationInMsForCalibration(double value);
    double getWeldingDurationInMsForCalibration() const;
    void setJumpSpeedInMmPerSecForCalibration(double value);
    double getJumpSpeedInMmPerSecForCalibration() const;
    geo2d::DPoint  getTCPPosition(math::SensorId p_oSensorId, const interface::ScannerContextInfo & rContextInfo, filter::LaserLine p_LaserLine);
    geo2d::DPoint getScreenPointZ0(filter::LaserLine laserLine);
    void setScreenPointZ0(filter::LaserLine laserLine, geo2d::DPoint);
    bool availableOCTCoordinates(interface::OCT_Mode mode);
    geo2d::DPoint  getNewsonPosition(double xScreen, double yScreen, interface::OCT_Mode mode);
    geo2d::DPoint  getScreenPositionFromNewson(double xNewson, double yNewson, interface::OCT_Mode mode);

	//@}

    
    /**
	 * @name ScanMaster
	 */
	//@{
    std::string getScanFieldPath() const;
    void setScanFieldPath(std::string path);
	void setScannerPosition(double x, double y);
    double getJumpSpeed() const;
    void setJumpSpeed(double speed);
    geo2d::DPoint getScannerPositionFromScanFieldImage(double x_pixel, double y_pixel,  interface::  Configuration scanfieldImageConfiguration) const;
    geo2d::DPoint getScanFieldImagePositionFromScannerPosition(double x_mm, double y_mm,  interface::  Configuration scanfieldImageConfiguration) const;
    void setOCTReferenceArm(int n);
    void weldHeadReloadFiberSwitchCalibration();
    
	//@}


    /**
        * @name Scheimpflug Calibration
        */
    //@{
    bool calibrateLaserPlaneWithChessboardRecognition(int threshold, CamGridData::ScheimpflugCameraModel cameraModel);
    int chessboardRecognitionThreshold(const BImage & chessboardImage, int threshold);
    //@}
    bool weldForScannerCalibration();

private:

    void startStoringFrames (int numberOfFrames);
    void stopStoringFrames();
    
	interface::TTriggerCmd<interface::AbstractInterface>& 			m_rTriggerCmdProxy;		///< Proxy of the trigger-cmd interface.
	interface::TRecorder<interface::AbstractInterface>& 			m_rRecorderProxy;		///< Proxy of the recorder interface.
	interface::TDevice<interface::AbstractInterface>& 				m_rCameraDeviceProxy;			///< Proxy of the device interface.
	interface::TWeldHeadMsg<interface::AbstractInterface>& 			m_rWeldHeadMsgProxy;	///< Proxy of the blocking weldhead message interface.
	interface::TCalibDataMsg<interface::MsgProxy>&                  m_rCalibDataMsgProxy;   ///< Proxy for signaling calibration data changes to the workflow/ analyzer
	interface::TDevice<interface::AbstractInterface>& 				m_rIDMDeviceProxy;		///< Proxy of the device interface.
 	interface::TDevice<interface::AbstractInterface>& 				m_rWeldHeadDeviceProxy;	///< Proxy of the weldhead device interface.


	bool 										m_isSimulation;	
	bool 										m_oRunning;				///< Is the calibration currently active?
	int 										m_oImageNumber;			///< Internal image counter (needed as the images have to have a correct, sequential image number on the windows side, but single trigger impulses do always generate images with the number 0).
	image::BImage								m_oImage;				///< Internal, current image.
	image::Sample			 					m_oSample;				///< Internal, current sample.
	Poco::FastMutex 							m_oInspectMutex;		///< Mutex to protect the image.
	Poco::Semaphore 							m_oImageSema;			///< Semaphore to signal that an image is ready to be picked up.
	Poco::Semaphore 							m_oSampleSema;			///< Semaphore to signal that a sample is ready to be picked up.
	std::shared_ptr<OverlayCanvas>				m_pCanvas;				///< Pointer to the overlay canvas object.
	interface::TriggerContext 					m_oTriggerContext;		///< Trigger-context object.
	int											m_oSampleSensorId;			///< SensorID the getSample function is looking for.
	std::array<math::CalibrationData, math::eNumSupportedSensor> m_oCalData;  // equivalent of calibration data singleton
    
	CalibrateIbOpticalSystem                    *m_oCalibOpticalSystem;
    CalibrateLEDIllumination                    *m_calibrateLEDIllumination;
	CalibrateIbAxis                             *m_oCalibAxis;
    CalibrationOCTData * m_pCalibrationOCTData;
    CalibrationOCT_TCP * m_pCalibrationOCT_TCP;
    CalibrateScanField * m_pCalibrateScanField;
	
    
	static const unsigned int                    m_oPosDelta;           ///< Allowed deviation for setHeadPos from target position
	OCTApplicationType m_oOCTApplicationType;
    bool m_oRenderImageImmediately;
    bool m_oOCTScanWide ;
    bool m_oStoreFrames;
    unsigned int m_oFramesToStore;
    std::vector<image::Sample> m_oSamples;
    std::vector<image::BImage> m_oImages;
    std::vector<interface::TriggerContext> m_oContexts;
    std::string m_oScanFieldPath;
    std::map<filter::LaserLine, geo2d::DPoint> m_screenPointZ0; //virtual keyvalue for the z Reference
    bool m_oHasCamera;
    double m_laserPowerInPct;
    double m_weldingDurationInMs;
    double m_jumpSpeedInMmPerSec;
    std::optional<double> m_ZCollDrivingRelative;
};

} // namespace analyzer
} // namespace precitec

#endif /* CALIBRATIONMANAGER_H_ */
