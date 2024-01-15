/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB), Andreas Beschorner (BA)
 * 	@date		2011, 2013-2014
 * 	@brief		Manages and executes calibration procedures.
 */


#include <sstream>
#include "Poco/File.h"


// WM includes
#include <message/calibration.interface.h>
#include <calibration/calibrationManager.h>
#include <calibration/homing.h>
#include <calibration/calibrate.h>
#include <calibration/CalibrateIbAxis.h>

#include <common/product.h>
#include <event/sensor.h>
#include <module/moduleLogger.h>
#include <system/tools.h>
// Poco includes
#include <Poco/Thread.h>
#include "math/mathCommon.h"

#include "common/systemConfiguration.h"
#include "common/calibrationConfiguration.h"

#include "calibration/CalibrationOCTData.h"
#include "calibration/CalibrationOCTLine.h"
#include "calibration/CalibrationOCT_TCP.h"
#include "calibration/CalibrationOCTMeasurementModel.h"
#include "calibration/calibrateLEDIllumination.h"
#include "calibration/calibrateScanField.h"
#include <calibration/ScanFieldImageCalculator.h>           // needed only in getScannerPositionFromScanFieldImage
#include <calibration/chessboardRecognition.h>
#include <calibration/chessboardWeldPointGenerator.h>

#include "coordinates/fieldDistortionMapping.h"


//#define __testCalib ;
#ifndef NDEBUG

#define _logSensorServer

#endif


namespace precitec {
using namespace geo2d;
namespace calibration {
using math::SensorModelDescription;
using math::SensorModel;

#ifndef NDEBUG
const bool DBG_ALLOW_FALLBACK = true; //used only in assertions
#endif

const unsigned int  CalibrationManager::m_oPosDelta = 10;


CalibrationManager::CalibrationManager(
		TTriggerCmd<AbstractInterface>& p_rTriggerCmdProxy,
		TRecorder<AbstractInterface>& p_rRecorderProxy,
		TDevice<AbstractInterface>& p_rCameraDeviceProxy,
		TWeldHeadMsg<AbstractInterface>& p_rWeldHeadMsgProxy,
		TCalibDataMsg<MsgProxy>& p_rCalibDataMsgProxy,
		TDevice<AbstractInterface>& p_rIDMDeviceProxy,
		TDevice<AbstractInterface>& p_rWeldHeadDeviceProxy,
        OCTApplicationType p_oOCTTRackApplication
	) :
	m_rTriggerCmdProxy( p_rTriggerCmdProxy ),
	m_rRecorderProxy( p_rRecorderProxy ),
	m_rCameraDeviceProxy ( p_rCameraDeviceProxy ),
	m_rWeldHeadMsgProxy( p_rWeldHeadMsgProxy ),
	m_rCalibDataMsgProxy( p_rCalibDataMsgProxy ),
	m_rIDMDeviceProxy(p_rIDMDeviceProxy),
	m_rWeldHeadDeviceProxy(p_rWeldHeadDeviceProxy),
	m_isSimulation(true),
	m_oRunning( false ),
	m_oImageNumber( 1 ),
	m_oImageSema(0, 10000),
	m_oSampleSema(0, 100000),
	m_pCanvas( new OverlayCanvas( 512, 512 ) ),
	m_oCalData({{ math::eSensorId0, math::eSensorId1, math::eSensorId2 }}),
	m_oOCTApplicationType(p_oOCTTRackApplication),
    m_oRenderImageImmediately(false),
    m_oOCTScanWide(true),
    m_laserPowerInPct(15.),
    m_weldingDurationInMs(50.),
    m_jumpSpeedInMmPerSec(250.)
{
#if !defined(NDEBUG)
	std::cout << "CalibrationManager::CTOR " << std::endl;
#endif
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            m_oHasCamera = SystemConfiguration::instance().getBool("HasCamera", true);
        }
        else
        {
            m_oHasCamera = false;
        }
    }
    else
    {
        m_oHasCamera = false;
    }

	m_oCalibOpticalSystem = new CalibrateIbOpticalSystem(*this);
    m_calibrateLEDIllumination = new CalibrateLEDIllumination(*this);
	m_oCalibAxis = new CalibrateIbAxis(*this);
	assert(!getCalibrationData(0).hasData());
    assert(m_pCalibrationOCTData == nullptr);
    assert(m_pCalibrationOCT_TCP == nullptr);
}


CalibrationManager::~CalibrationManager()
{
	if (m_oCalibOpticalSystem != nullptr)
	{
		delete m_oCalibOpticalSystem;
	}
	if (m_oCalibAxis != nullptr)
	{
		delete m_oCalibAxis;
	}
	if (m_pCalibrationOCTData != nullptr)
    {
        delete m_pCalibrationOCTData;
    }
    if (m_pCalibrationOCT_TCP != nullptr)
    {
        delete m_pCalibrationOCT_TCP;
    }
    if (m_calibrateLEDIllumination != nullptr)
    {
        delete m_calibrateLEDIllumination;
    }
    if (m_pCalibrateScanField != nullptr)
    {
        delete m_pCalibrateScanField;
    }
}


bool CalibrationManager::calibrate( int oMethod )
{
#ifndef NDEBUG
		std::cout << "CalibrationManager::calibrate( " << oMethod << " )" << std::endl;
#endif

	//std::cout<<"CAL calibrat startet "<<std::endl;
	wmLog( eInfo, "Calibrate called, Method: %d\n", oMethod );
    
    if (m_isSimulation && oMethod != eInitCalibrationData)
    {
        wmLog(eWarning, "Calibration method %d not supported in simulation\n", oMethod);
        assert(false && "Called wrong calibration method in simulation");
        return false;
    }

	// init variables
	m_oRunning = true;
	m_oImageNumber = 1;

	Poco::Thread::sleep(200);


    auto oRequestFlashEnableOff = (oMethod == eCalibrateOsIbLine0
                                || oMethod == eCalibrateOsIbLineTCP
                                || oMethod == eCalibrateOsIbLine2
                                || oMethod == eCalibGridAngle);

    auto led1Enabled = false;
    auto led2Enabled = false;
    auto led3Enabled = false;
    auto led4Enabled = false;
    auto led5Enabled = false;
    auto led6Enabled = false;
    auto led7Enabled = false;
    auto led8Enabled = false;

    if (oRequestFlashEnableOff && !m_isSimulation)
    {
        led1Enabled = m_rWeldHeadMsgProxy.getLEDEnable(LED_PANEL_1);
        led2Enabled = m_rWeldHeadMsgProxy.getLEDEnable(LED_PANEL_2);
        led3Enabled = m_rWeldHeadMsgProxy.getLEDEnable(LED_PANEL_3);
        led4Enabled = m_rWeldHeadMsgProxy.getLEDEnable(LED_PANEL_4);
        led5Enabled = m_rWeldHeadMsgProxy.getLEDEnable(LED_PANEL_5);
        led6Enabled = m_rWeldHeadMsgProxy.getLEDEnable(LED_PANEL_6);
        led7Enabled = m_rWeldHeadMsgProxy.getLEDEnable(LED_PANEL_7);
        led8Enabled = m_rWeldHeadMsgProxy.getLEDEnable(LED_PANEL_8);

        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_1, false))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_1, false) failed, could not communicate with VI_WeldHead.");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_2, false))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_2, false) failed, could not communicate with VI_WeldHead.");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_3, false))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_3, false) failed, could not communicate with VI_WeldHead.");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_4, false))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_4, false) failed, could not communicate with VI_WeldHead.");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_5, false))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_5, false) failed, could not communicate with VI_WeldHead.");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_6, false))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_6, false) failed, could not communicate with VI_WeldHead.");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_7, false))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_7, false) failed, could not communicate with VI_WeldHead.");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_8, false))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_8, false) failed, could not communicate with VI_WeldHead.");
        }
        m_rWeldHeadDeviceProxy.set(SmpKeyValue{new TKeyValue<bool>{std::string{"LEDSendData"}, true}});
    }

    if (!m_isSimulation)
    {
        switch (oMethod)
        {
            case eCalibrateScanFieldIDM_Z:
            case eVerifyScanFieldIDM_Z:
            {
                m_rWeldHeadDeviceProxy.set(SmpKeyValue{new TKeyValue<int>{std::string{"CorrectionFileMode"}, static_cast<int>(CorrectionFileMode::HeightMeasurement)}});
                break;
            }
            case eAcquireScanFieldImage:
            case eCalibrateScanFieldTarget:
            default:
            {
                m_rWeldHeadDeviceProxy.set(SmpKeyValue{new TKeyValue<int>{std::string{"CorrectionFileMode"}, static_cast<int>(CorrectionFileMode::Welding)}});
            }
        }
    }

	//
	// execute the selected calibration procedure
	//
	bool useOrientedLine = SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::Scanner2DEnable); //TODO use calib parameter
	bool oResult = false;
	try
	{
		switch(oMethod)
		{

			case eCalibrateAxisIb:
			{
				oResult = m_oCalibAxis->calibrate(); // todo: sensorID
				break;
			}

			// standard coax optical calibration to determine beta0 and betaZ
			case eCalibrateOsIbLine0:
			{
				std::cout<<"CAL start calibrating optical system "<<std::endl;
				oResult = m_oCalibOpticalSystem->calibrateLine(filter::LaserLine::FrontLaserLine, useOrientedLine, readCameraRelatedParameters()); // todo: sensorID
				std::cout<<"calibrationManager: eCalibrateIsIbLine0 finished with: "<<oResult<<std::endl;
				break;
			}

			case eBlackLevelOffset:
			{
				wmLog( eError, "CalibrationManager::calibrate(eBlackLevelOffset) not implemented!\n" );
				break;
			}

			case eHomingY:
			{
				wmLog( eInfo, "Homing of axis Y started ...\n" );
				Homing oHoming(*this);
				oResult = oHoming.calibrate(eHomingY);
				break;
			}

			case eHomingZ:
			{
				wmLog( eInfo, "Homing of axis Z started ...\n" );
				Homing oHoming(*this);
				oResult = oHoming.calibrate(eHomingZ);
				break;
			}

			case eHomingZColl:
			{
				wmLog( eInfo, "Homing of Z-Collimator started ...\n" );
				Homing oHoming(*this);
				oResult = oHoming.calibrate(eHomingZColl);
				break;
			}

			// called from Mod_Workflow/src/init.cpp at startup of system, or when calibration is updated in the simulation station
			case eInitCalibrationData:
			{   //todo: loop over all sensors 
                math::SensorId oSensorId = math::eSensorId0;
				wmLog( eInfo, "Initialize calibration data...\n");
                auto& rCalibData = getCalibrationData(oSensorId);
                const bool canInitialize( !rCalibData.isInitialized() || m_isSimulation);
                if (canInitialize)
                {                    
                    if (rCalibData.isInitialized())
                    {
                        //needed by simulation when the calibration config has changed
                        wmLog(eInfo, "Resetting - calibration will be initialized again\n");
                        rCalibData.resetConfig();
                    }
                    oResult = getOSCalibrationDataFromHW(oSensorId);
                    assert((oResult || checkCamera(oSensorId,1).empty()) && "error in eInitCalibrationData");
                }
                else
                {
                   //we could be here when called from the fieldbus
                   wmLog(eError, "Skipping call to InitCalibrationData: calibration is already initialized\n");
                }
                if (isOCTTrackApplication() && !m_isSimulation)
                {
                    wmLog(eInfo, "IDM enabled, updating calibration data\n");
                    if (m_pCalibrationOCTData == nullptr)
                    {
                        m_pCalibrationOCTData = new CalibrationOCTData();
                    }
                    m_pCalibrationOCTData->load(rCalibData.getHomeDir());
                    m_pCalibrationOCTData->initializeSystem(*this);
                    //now the file CalibrationData0.xml has been modified, the simulation will update by itself
                }
                break;
			}

			// determine Scheimpflug triangulation angle
			case eCalibGridAngle:
			{
				wmLog(eInfo, "Determining Scheimpflug calibration triangulation angle...\n");
				oResult = determineTriangulationAngle(0, readCameraRelatedParameters()); // todo: sensor id as parameter! Must be send from windows...
				break;
			}

			case eCalibrateOsIbLineTCP:
			{
				std::cout << "start calibrating optical system" << std::endl;
				oResult = m_oCalibOpticalSystem->calibrateLine(filter::LaserLine::CenterLaserLine, useOrientedLine, readCameraRelatedParameters()); // todo: sensorID
				std::cout << "calibrationManager: eCalibrateIsIbLine_TCPfinished with: " << oResult << std::endl;

				break;
			}

			case eCalibrateOsIbLine2:
			{
				std::cout << "start calibrating optical system" << std::endl;
				oResult = m_oCalibOpticalSystem->calibrateLine(filter::LaserLine::BehindLaserLine, useOrientedLine, readCameraRelatedParameters()); // todo: sensorID
				std::cout << "calibrationManager: eCalibrateIsIbLine_2 finished with: " << oResult << std::endl;
				break;
			}
            case eOCT_LineCalibration:
                if (isOCTTrackApplication())
                { 
                    CalibrationOCTLine  oCalibrateOCTLine(*this);
                    oResult = oCalibrateOCTLine.calibrate();
                }
                else
                {
                    wmLog(eWarning, "Cannot calibrate, OCT not enabled \n");
                    oResult = false;
                }
                break;
            case eOCT_TCPCalibration:
                if (isOCTTrackApplication())
                { 
                    if (m_pCalibrationOCT_TCP != nullptr)
                    {
                        delete m_pCalibrationOCT_TCP;
                    }
                    m_pCalibrationOCT_TCP = new CalibrationOCT_TCP(*this);
                    m_pCalibrationOCT_TCP->m_oScanWideArea = m_oOCTScanWide;
                    m_oRenderImageImmediately = true;
                    oResult = m_pCalibrationOCT_TCP->calibrate();
                    m_oRenderImageImmediately = false;
                }
                else
                {
                    wmLog(eWarning, "Cannot calibrate, OCT not enabled \n");
                    oResult = false;
                }
                break;
            case eCalibrateLED:
                wmLog( eInfo, "LED Calibration started! \n" );
                std::cout<<"CAL start calibrating illumination system "<<std::endl;
                oResult = m_calibrateLEDIllumination->calibrate();
                std::cout<<"calibrationManager: eCalibrateIsIbLine0 finished with: "<<oResult<<std::endl;
                break;
            
            case eAcquireScanFieldImage:
            case eCalibrateScanFieldTarget:
            case eCalibrateScanFieldIDM_Z:
            case eVerifyScanFieldIDM_Z:
            case eScannerCalibrationMeasure:
            case eScanmasterCameraCalibration:
                {
                    if (m_pCalibrateScanField == nullptr)
                    {
                        m_pCalibrateScanField = new CalibrateScanField(*this);
                    }
                    bool ok = m_pCalibrateScanField->setCalibrationType(oMethod);
                    (void)ok;
                    assert(ok);
                    oResult =  m_pCalibrateScanField->calibrate();
                }
                break;
            case eIDMDarkReference:
                if (isOCTEnabled())
                {
                    setIDMKeyValue(interface::SmpKeyValue{new interface::TKeyValue<bool> ("Perform Dark Reference", true )});
                    oResult = true;
                    wmLog(eInfo, "Dark Reference calibration done\n");
                }
                else
                {
                    wmLog(eWarning, "IDM Dark Reference requested, but IDM not enabled\n");
                    oResult = false;
                }
                break;

            case eCalibGridChessboard:
            {
                int oSensorId = 0;
                auto& rCalibParameters = getCalibrationData(oSensorId).getParameters();
                auto threshold = rCalibParameters.getInt("ScheimpflugCalib_Threshold");
                auto cameraModel = static_cast<CamGridData::ScheimpflugCameraModel>(rCalibParameters.getInt("ScheimpflugCameraModel"));
                oResult = calibrateLaserPlaneWithChessboardRecognition(threshold, cameraModel);
            }
                break;
            case eScannerCalibration:
            {
                oResult = weldForScannerCalibration();
            }
                break;
			default:
			{
				wmLog( eError, "Calibration-method %i not implemented!\n", oMethod );
				m_oRunning = false;
				break;
			}
		} // switch
	}
	catch( ... )
	{
		// something has happened, calibration was not successful...
		oResult = false;
	}

    if (oRequestFlashEnableOff && !m_isSimulation)
    {
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_1, led1Enabled))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_1, led1Enabled = %s) failed, could not communicate with VI_WeldHead.", led1Enabled ? "true" : "false");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_2, led2Enabled))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_2, led2Enabled = %s) failed, could not communicate with VI_WeldHead.", led2Enabled ? "true" : "false");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_3, led3Enabled))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_3, led3Enabled = %s) failed, could not communicate with VI_WeldHead.", led3Enabled ? "true" : "false");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_4, led4Enabled))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_4, led4Enabled = %s) failed, could not communicate with VI_WeldHead.", led4Enabled ? "true" : "false");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_5, led5Enabled))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_5, led5Enabled = %s) failed, could not communicate with VI_WeldHead.", led5Enabled ? "true" : "false");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_6, led6Enabled))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_6, led6Enabled = %s) failed, could not communicate with VI_WeldHead.", led6Enabled ? "true" : "false");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_7, led7Enabled))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_7, led7Enabled = %s) failed, could not communicate with VI_WeldHead.", led7Enabled ? "true" : "false");
        }
        if (!m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_8, led8Enabled))
        {
            wmLog(eWarning, "m_rWeldHeadMsgProxy.setLEDEnable(LED_PANEL_8, led8Enabled = %s) failed, could not communicate with VI_WeldHead.", led8Enabled ? "true" : "false");
        }
        m_rWeldHeadDeviceProxy.set(SmpKeyValue{new TKeyValue<bool>{std::string{"LEDSendData"}, true}});
    }

    if (!m_isSimulation && (oMethod == eCalibrateScanFieldIDM_Z || oMethod == eVerifyScanFieldIDM_Z))
    {
        m_rWeldHeadDeviceProxy.set(SmpKeyValue{new TKeyValue<int>{std::string{"CorrectionFileMode"}, static_cast<int>(CorrectionFileMode::Welding)}});
    }

	// exit the calibration again
	m_oRunning = false;
	m_rTriggerCmdProxy.cancel(std::vector<int>(1, 0));

	return oResult;
}

// ----------------------------------------------------------------------------
// Sensor Interface - Grabber, Encoder, etc...
// ----------------------------------------------------------------------------


BImage CalibrationManager::getImage() // todo: sensorID
{
    assert(!m_isSimulation);
#if !defined(NDEBUG)
	std::cout << "CalibrationManager::getImage()" << std::endl;
	wmLog(eInfo, "CalibrationManager::getImage()\n" );
#endif

	while(m_oImageSema.tryWait(1));
	m_rTriggerCmdProxy.single(std::vector<int>(1, 0), TriggerContext(true) ); Poco::Thread::sleep( 200 );
	if( !m_oImageSema.tryWait( 5000 ) )
	{
		wmLogTr(eError, "QnxMsg.Calib.NoImage", "Calibration failed, no image available -- check hardware.\n");
		throw std::exception();
	}

	return m_oImage;
};

BImage CalibrationManager::getCurrentImage()
{
    assert(!m_isSimulation);
	return m_oImage;
}



Sample CalibrationManager::getSample(int p_oSensorId )
{
    assert(!m_isSimulation);
#ifdef _logSensorServer
	std::cout << "CalibrationManager::getSample()" << std::endl;
#endif

	m_oSampleSensorId = p_oSensorId;

    while(m_oSampleSema.tryWait(1));                                     
    //TriggerContext vor Umstellung mit 0,0 initialisiert
    m_rTriggerCmdProxy.single(std::vector<int>(1, p_oSensorId), TriggerContext(true) ); 
    Poco::Thread::sleep(5);

	if( !m_oSampleSema.tryWait(5000) )
	{
		wmLogTr(eError, "QnxMsg.Calib.NoSample", "Calibration failed, no sample available -- check hardware.\n");
		throw std::exception();
	}

    cancelTrigger({p_oSensorId});
	return m_oSample;

};

std::vector<Sample> CalibrationManager::getSamples(int p_oSensorId, int numSamples, unsigned int trigger_ms, unsigned int timeout_ms )
{
    assert(!m_isSimulation);
#ifdef _logSensorServer
	std::cout << "CalibrationManager::getSamples(" << p_oSensorId <<", " << numSamples <<") "<<  std::endl;
#endif

    if (numSamples == 0)
    {
         return {};
    }
    
	m_oSampleSensorId = p_oSensorId;

    //decrement the semaphore value until 0
    while(m_oSampleSema.tryWait(1)); 
    
    if (numSamples == 1)
    {
        return {getSample(p_oSensorId)};
    }
    
    std::vector<int> oSensorsIds(1, p_oSensorId);
    bool isSingleShot = false;
    unsigned int oTriggerDistance_ns = trigger_ms * 1000000;
    int oTriggerSource = -1;
    
    startStoringFrames(numSamples);
    assert(m_oSamples.size() == 0);
    
    m_rTriggerCmdProxy.burst(oSensorsIds, TriggerContext(isSingleShot), oTriggerSource, TriggerInterval(oTriggerDistance_ns, numSamples));

    Poco::Thread::sleep(5);
    
    if( !m_oSampleSema.tryWait(timeout_ms) )
    {
        wmLogTr(eWarning, "QnxMsg.Calib.NoSample", "Calibration failed, no sample available -- check hardware.\n");
        std::ostringstream oMsg;
        oMsg << "Calibration: not enough samples available ( " << m_oSamples.size() << " / " << numSamples << " timeout = " << timeout_ms << " ms" ;
        std::cout << oMsg.str() << std::endl;
        throw precitec::system::TimeoutException(oMsg.str());
    }
    
    cancelTrigger(oSensorsIds);
    return m_oSamples;
};


TriggerContext CalibrationManager::getTriggerContext()
{
    assert(!m_isSimulation);
#ifdef _logSensorServer
	std::cout << "CalibrationManager::getTriggerContext()" << std::endl;
#endif

	return m_oTriggerContext;
}


void CalibrationManager::setIndexForNextImageFromDisk(int index)
{
    if (m_oHasCamera)
    { 
        return;
    }
    m_rCameraDeviceProxy.set(SmpKeyValue{new TKeyValue<bool>("ReloadImagesFromDisk",true)});
    for (int i = 0; i < index; i++)
    {
        getImage();
    }
}

//here p_oSensorID doesnt' have the same meaning as in the calibration (here it's 1, in the calibration is 0), but it's not used anyway
void CalibrationManager::image(int p_oSensorId, TriggerContext const& p_rTriggerContext, BImage const& p_rData)
{
    assert(!m_isSimulation);
	if ( !m_oRunning )
		return;

	m_oImageNumber++;

#ifdef _logSensorServer
	std::cout << "CalibrationManager::inspect::image [sensorid=" << p_oSensorId << "] " << m_oImageNumber << std::endl;
#endif

	Poco::FastMutex::ScopedLock oLock( m_oInspectMutex );

	if (p_rData.height() != m_pCanvas->height() || p_rData.width() != m_pCanvas->width() )
	{
		m_pCanvas.reset( new OverlayCanvas( p_rData.width(), p_rData.height() ) );
	}

	m_oTriggerContext = p_rTriggerContext;
	m_oTriggerContext.setImageNumber( m_oImageNumber );
	m_oImage = p_rData;
    if (m_oStoreFrames)
    {
        m_oImages.push_back(BImage (p_rData.size()));
        auto & rCopyImage(m_oImages.back());
        for (int row = 0; row < p_rData.height(); ++row)
        {
            memcpy(rCopyImage.rowBegin(row), p_rData.rowBegin(row), p_rData.width());
        }
        m_oContexts.push_back(p_rTriggerContext);
        if (m_oImages.size() >= m_oFramesToStore)
        {
            stopStoringFrames();
            m_oImageSema.set();
        }
    }
    else
    {
        m_oImageSema.set();
    }
    
    if (m_oRenderImageImmediately)
    {
        renderImage(m_oImage);
    }
};



void CalibrationManager::sample(int p_oSensorId, TriggerContext const& p_rTriggerContext, Sample const& p_rData)
{
    assert(!m_isSimulation);
	if ( !m_oRunning )
		return;

#ifdef _logSensorServer
	std::cout << "CalibrationManager::inspect::Sample[" << p_oSensorId << "]" << p_rTriggerContext.imageNumber() << std::endl;
#endif

	Poco::FastMutex::ScopedLock oLock( m_oInspectMutex );

	m_oTriggerContext = p_rTriggerContext;
    bool debug = getCalibrationData(0).getParameters().getInt("Debug") > 0;

	if ( p_oSensorId == m_oSampleSensorId )
	{
		m_oSample = p_rData;
        if (debug)
        {
            wmLog(eDebug, "Sample %d/%d from sensor %d received: %d \n",
                  int( m_oStoreFrames ? m_oSamples.size() :1 ), int( m_oStoreFrames ? m_oFramesToStore :1 ),
                  p_oSensorId,
                  int(p_rData.numElements() > 0 ? p_rData[0]: -1 ));
        }
        if (m_oStoreFrames)
        {
            m_oSamples.push_back(p_rData);
            if (m_oSamples.size() >= m_oFramesToStore)
            {
                stopStoringFrames();
                m_oSampleSema.set();
            }
        }
        else
        {
            m_oSampleSema.set();
        }
	}
};


void CalibrationManager::cancelTrigger ( const std::vector< int >& p_rSensorIds )
{
    m_rTriggerCmdProxy.cancel ( p_rSensorIds );
}

// ----------------------------------------------------------------------------
// WeldHead Interface
// ----------------------------------------------------------------------------

void CalibrationManager::setHeadPos( int p_oYpos, HeadAxisID p_oAxis )
{
    assert(!m_isSimulation);
#if !defined(NDEBUG)
	std::cout << "CalibrationManager::setWeldHeadPos: " << p_oYpos << std::endl;
#endif


	if( !m_rWeldHeadMsgProxy.setHeadPos( p_oAxis, p_oYpos ) )
	{
		std::cout << "CalibrationManager::setWeldHeadPos() failed, could not communicate with VI_WeldHead." << std::endl;
		throw std::exception();
	}
	//todo: timeout hinzufuegen
	while (std::abs(p_oYpos - getHeadPos(p_oAxis)) > int(CalibrationManager::m_oPosDelta)) {} // wait until target position has been taken (approx.)

} // setHeadPos



int CalibrationManager::getHeadPos( HeadAxisID p_oAxis )
{
    assert(!m_isSimulation);
#if !defined(NDEBUG)
	std::cout << "CalibrationManager::getHeadPosition" << std::endl;
#endif

	int oPosition = m_rWeldHeadMsgProxy.getHeadPosition( p_oAxis );
	return oPosition;

} // getHeadPos



void CalibrationManager::setHeadMode( HeadAxisID p_oAxis, MotionMode p_oMode, bool p_oHome )
{
    assert(!m_isSimulation);
#if !defined(NDEBUG)
	std::cout << "CalibrationManager::setHeadMode" << std::endl;
#endif

	if( !m_rWeldHeadMsgProxy.setHeadMode( p_oAxis, p_oMode, p_oHome ) )
	{
		std::cout << "CalibrationManager::setHeadMode() failed, could not communicate with VI_WeldHead." << std::endl;
		throw std::exception();
	}

} // setHeadMode

void CalibrationManager::doZCollHoming( void )
{
    assert(!m_isSimulation);
#if !defined(NDEBUG)
	std::cout << "CalibrationManager::doZCollHoming" << std::endl;
#endif

	if( !m_rWeldHeadMsgProxy.doZCollHoming() )
	{
		std::cout << "CalibrationManager::doZCollHoming() failed, could not communicate with VI_WeldHead." << std::endl;
		throw std::exception();
	}

} // doZCollHoming

void CalibrationManager::doZCollDrivingRelative(int value)
{
#if !defined(NDEBUG)
	std::cout << "CalibrationManager::doZCollDrivingRelative" << std::endl;
#endif

	if (!m_rWeldHeadMsgProxy.doZCollDrivingRelative(value))
	{
		std::cout << "CalibrationManager::doZCollDrivingRelative() failed, could not communicate with VI_WeldHead." << std::endl;
		throw std::exception();
	}
}

// ----------------------------------------------------------------------------
// Canvas Interface
// ----------------------------------------------------------------------------

void CalibrationManager::drawLine( int p_oX0, int p_oY0, int p_oX1, int p_oY1, Color p_oColor)
{
	OverlayLayer &rLayer = m_pCanvas->getLayerLine();
	rLayer.add<OverlayLine>( p_oX0, p_oY0, p_oX1, p_oY1, p_oColor );
}


void CalibrationManager::drawCross( int p_oX, int p_oY, int p_oSize, Color p_oColor)
{
	int oX0, oY0, oX1, oY1;
	oX0 = p_oX - p_oSize;
	if( oX0<0 ) oX0=0;

	oY0 = p_oY - p_oSize;
	if( oY0<0 ) oY0=0;

	oX1 = p_oX + p_oSize;
	oY1 = p_oY + p_oSize;
	drawLine(  oX0,  p_oY,  oX1,  p_oY,  p_oColor);
	drawLine(  p_oX,  oY0,  p_oX,  oY1,  p_oColor);
};



void CalibrationManager::drawPixel( int p_oX, int p_oY, Color p_oColor)
{
	OverlayLayer &rLayer = m_pCanvas->getLayerContour();
	rLayer.add<OverlayPoint>( p_oX, p_oY, p_oColor );
};



void CalibrationManager::drawRect( int p_oX, int p_oY, int p_oW, int p_oH, Color p_oColor)
{
	OverlayLayer &rLayer = m_pCanvas->getLayerLine();
	geo2d::Rect oBox( Point( p_oX, p_oY ), Size( p_oW, p_oH) );
	rLayer.add<OverlayRectangle>( oBox, p_oColor );
};

void CalibrationManager::drawText( int p_oX, int p_oY, std::string p_oText, Color p_oColor)
{
	OverlayLayer &rLayer = m_pCanvas->getLayerText();
	rLayer.add<OverlayText>(p_oText, Font(11), Rect(p_oX, p_oY, 400, 50), p_oColor);
}

void CalibrationManager::drawImage( int p_oX, int p_oY, BImage oImage, std::string p_oText, Color p_oColor)
{
	OverlayLayer &rLayer = m_pCanvas->getLayerImage();
	const auto		oPosition	=	Point(p_oX, p_oY);
	const auto		oTitle		=	OverlayText(p_oText, Font(), Rect(150, 18), p_oColor);

	rLayer.add<OverlayImage>(Point(p_oX, p_oY),  oImage, oTitle);
}

void CalibrationManager::renderCanvas()
{
    renderImage(getCurrentImage());
}

void CalibrationManager::renderImage(const BImage& p_rImage)
{
	ImageContext oImageContext( m_oTriggerContext );
	oImageContext.setImageNumber( m_oTriggerContext.imageNumber() );
	TaskContext oTaskContext;
	oImageContext.setTaskContext(oTaskContext);
	m_rRecorderProxy.data(0, oImageContext, p_rImage, *m_pCanvas );

} // renderImage

void CalibrationManager::renderImage(const BImage& p_rImage, ImageContext ctx)
{
    if (p_rImage.height() != m_pCanvas->height() || p_rImage.width() != m_pCanvas->width() )
	{
		m_pCanvas.reset( new OverlayCanvas( p_rImage.width(), p_rImage.height() ) );
	}

	m_rRecorderProxy.data(0, ctx, p_rImage, *m_pCanvas );

} // renderImage


void CalibrationManager::clearCanvas()
{
	m_pCanvas->clearShapes();

} // clearCanvas



std::shared_ptr<OverlayCanvas> CalibrationManager::getCanvas()
{
	return m_pCanvas;

} // getCanvas



math::CalibrationData& CalibrationManager::getCalibrationData(unsigned int p_oSensorId)
{
    assert(p_oSensorId == 0 && "Multiple sensors not supported");
    //TODO: handle invalid p_oSensorId
    return m_oCalData[math::SensorId(p_oSensorId)];
} // getCalibrationData


const math::CalibrationData& CalibrationManager::getCalibrationData(unsigned int p_oSensorId) const
{
    assert(p_oSensorId == 0 && "Multiple sensors not supported");
    //TODO: handle invalid p_oSensorId
    return m_oCalData[math::SensorId(p_oSensorId)];
} // getCalibrationData



// ----------------------------------------------------------------------------
// Device Interface - Set and get camera parameters
// ----------------------------------------------------------------------------

int CalibrationManager::getTestimageEnabled(int p_oDevice)
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getTestimageEnabled(): calling deviceProxy..." << std::endl;
#endif

	SmpKeyValue pKeyValue = m_rCameraDeviceProxy.get("FTTestImage.Enable", p_oDevice);
	int oValue = 0;

	if( pKeyValue->isHandleValid() )
	{
		oValue = pKeyValue->value<int>();

#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getTestimageEnabled(): " << oValue << std::endl;
#endif

	} else {

		std::cout<< "CalibrationManager::getTestimageEnabled(): value not valid, is the camera connected and working?" << std::endl;
		throw std::exception();
	}

	return oValue;
}


bool CalibrationManager::getHWRoi(int &p_rX0, int &p_rY0, int &p_rWidth, int &p_rHeight, const int p_oDevice)
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getHWRoi(...)\n";
#endif

	p_rX0 = 0; p_rY0 = 0; p_rWidth = 0; p_rHeight = 0;

	int oFailure = 0;
	SmpKeyValue pKeyValueX0 = m_rCameraDeviceProxy.get("Window.X", p_oDevice);
	if ( pKeyValueX0->isHandleValid() )
	{
		p_rX0 = pKeyValueX0->value<int>();
	} else
	{
		oFailure += 1;
	}
	Poco::Thread::sleep(200);

	SmpKeyValue pKeyValueY0 = m_rCameraDeviceProxy.get("Window.Y", p_oDevice);
	if ( pKeyValueY0->isHandleValid() )
	{
		p_rY0 = pKeyValueY0->value<int>();
	} else
	{
		oFailure += 2;
	}
	Poco::Thread::sleep(200);

	SmpKeyValue pKeyValueWidth = m_rCameraDeviceProxy.get("Window.W", p_oDevice);
	if ( pKeyValueWidth->isHandleValid() )
	{
		p_rWidth = pKeyValueWidth->value<int>();
	} else
	{
		oFailure += 4;
	}
	Poco::Thread::sleep(200);

	SmpKeyValue pKeyValueHeight = m_rCameraDeviceProxy.get("Window.H", p_oDevice);
	if ( pKeyValueHeight->isHandleValid() )
	{
		p_rHeight = pKeyValueHeight->value<int>();
	} else
	{
		oFailure += 8;
	}
	Poco::Thread::sleep(200);

	if (oFailure > 0)
	{
		std::cout<< "CalibrationManager::getHWRoi(...): Error in parameters, errorFlag is " << oFailure << ".\n ";
		return false;
	}

	return true;
}

void CalibrationManager::setHWRoi(const int p_rX0, const int p_rY0, const int p_rWidth, const int p_rHeight, const int p_oDevice)
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::setHWRoi(" << p_rX0 << ", " << p_rY0 << ", " << p_rWidth << ", " << p_rHeight << ")\n ";
#endif

	SmpKeyValue pKeyValueX0 = new TKeyValue<int>("Window.X", p_rX0);
	m_rCameraDeviceProxy.set( pKeyValueX0, p_oDevice);
	Poco::Thread::sleep(200);

	SmpKeyValue pKeyValueY0 = new TKeyValue<int>("Window.Y", p_rY0);
	m_rCameraDeviceProxy.set( pKeyValueY0, p_oDevice);
	Poco::Thread::sleep(200);

	SmpKeyValue pKeyValueWidth = new TKeyValue<int>("Window.W", p_rWidth);
	m_rCameraDeviceProxy.set( pKeyValueWidth, p_oDevice);
	Poco::Thread::sleep(200);

	SmpKeyValue pKeyValueHeight = new TKeyValue<int>("Window.H", p_rHeight);
	m_rCameraDeviceProxy.set( pKeyValueHeight, p_oDevice);
	Poco::Thread::sleep(200);
}

void CalibrationManager::setTestimageEnabled(int p_oEnable, int p_oDevice)
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::setTestimageEnabled(): " << p_oEnable << std::endl;
#endif

	SmpKeyValue pKeyValue = new TKeyValue<int>("FTTestImage.Enable", p_oEnable);

	m_rCameraDeviceProxy.set( pKeyValue, p_oDevice);
}

float CalibrationManager::getExposureTime( int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getExposureTime(): calling deviceProxy..." << std::endl;
#endif

	SmpKeyValue pKeyValue = m_rCameraDeviceProxy.get("ExposureTime", p_oDevice);
	float oValue = 0.0f;

	if( pKeyValue->isHandleValid() )
	{
		oValue = pKeyValue->value<float>();

#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getExposureTime(): " << oValue << std::endl;
#endif

	} else {

		std::cout<< "CalibrationManager::getExposureTime(): value not valid, is the camera connected and working?" << std::endl;
		throw std::exception();
	}

	return oValue;
};

void CalibrationManager::setExposureTime( float p_oExposureTime, int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::setExposureTime(): " << p_oExposureTime << std::endl;
#endif

	SmpKeyValue pKeyValue = new TKeyValue<float>("ExposureTime", p_oExposureTime );

	m_rCameraDeviceProxy.set( pKeyValue, p_oDevice);
};



int CalibrationManager::getBlackLevelOffset( int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getBlackLevelOffset(): calling deviceProxy..." << std::endl;
#endif

	SmpKeyValue pKeyValue = m_rCameraDeviceProxy.get("Voltages.BlackLevelOffset", p_oDevice);
	int oValue = 0;

	if( pKeyValue ->isHandleValid() )
	{
		oValue = pKeyValue->value<int>();

#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getBlackLevelOffset(): " << oValue << std::endl;
#endif

	} else {

		std::cout<< "CalibrationManager::getBlackLevelOffset(): value not valid, is the camera connected and working?" << std::endl;

		throw std::exception();
	}

	return oValue;
}



void CalibrationManager::setBlackLevelOffset( int p_oBlackLevelOffset, int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::setBlackLevelOffset(): " << p_oBlackLevelOffset << std::endl;
#endif

	SmpKeyValue pKeyValue = new TKeyValue<int>("Voltages.BlackLevelOffset", p_oBlackLevelOffset );
	m_rCameraDeviceProxy.set( pKeyValue, p_oDevice);
}
//*******************************************************************************
void CalibrationManager::setROI_dX( int ivalue, int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::setROI_X0(): " << p_oDevice << std::endl;
#endif
#ifdef __testCalib
		return;
#endif

	SmpKeyValue pKeyValue = new TKeyValue<int>("Window.W", ivalue );

	m_rCameraDeviceProxy.set( pKeyValue, p_oDevice);
};
//*******************************************************************************
void CalibrationManager::setROI_dY( int ivalue, int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::setROI_X0(): " << p_oDevice << std::endl;
#endif
#ifdef __testCalib
		return;
#endif
	SmpKeyValue pKeyValue = new TKeyValue<int>("Window.H", ivalue );

	m_rCameraDeviceProxy.set( pKeyValue, p_oDevice);
};

//*******************************************************************************
void CalibrationManager::setROI_X0( int ivalue, int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::setROI_X0(): " << p_oDevice << std::endl;
#endif
#ifdef __testCalib
		return;
#endif
	SmpKeyValue pKeyValue = new TKeyValue<int>("Window.X", ivalue );

	m_rCameraDeviceProxy.set( pKeyValue, p_oDevice);
};

//*******************************************************************************
int CalibrationManager::getROI_X0( int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getROI_X0(): calling deviceProxy..." << std::endl;
#endif

#ifdef __testCalib
		return 0;
#endif
	SmpKeyValue pKeyValue = m_rCameraDeviceProxy.get("Window.X", p_oDevice);
	int oValue = 0;

	if( pKeyValue ->isHandleValid() )
	{
		oValue = pKeyValue->value<int>();

#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getROI_X0(): " << oValue << std::endl;
#endif

	} else {

		std::cout<< "CalibrationManager::getROI_X0(): value not valid, is the camera connected and working?" << std::endl;

		throw std::exception();
	}

	return oValue;
}

//*******************************************************************************
void CalibrationManager::setROI_Y0( int ivalue, int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::setROI_X0(): " << p_oDevice << std::endl;
#endif
#ifdef __testCalib
		return;
#endif

	SmpKeyValue pKeyValue = new TKeyValue<int>("Window.Y", ivalue );

	m_rCameraDeviceProxy.set( pKeyValue, p_oDevice);
};

//*******************************************************************************
int CalibrationManager::getROI_Y0( int p_oDevice )
{
#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getROI_Y0(): calling deviceProxy..." << std::endl;
#endif
#ifdef __testCalib
		return 0;
#endif
	SmpKeyValue pKeyValue = m_rCameraDeviceProxy.get("Window.Y", p_oDevice);
	int oValue = 0;

	if( pKeyValue ->isHandleValid() )
	{
		oValue = pKeyValue->value<int>();

#if !defined(NDEBUG)
		std::cout<< "CalibrationManager::getROI_Y0(): " << oValue << std::endl;
#endif

	} else {

		std::cout<< "CalibrationManager::getROI_Y0(): value not valid, is the camera connected and working?" << std::endl;

		throw std::exception();
	}

	return oValue;
}

int  CalibrationManager::setROI(int x0,int y0,int dx,int dy, int p_oDevice)
{
	if(x0 < getROI_X0(p_oDevice) )
	{
		setROI_X0( x0,  p_oDevice );
		setROI_dX( dx,  p_oDevice );
	}
	else
	{
		setROI_dX( dx,  p_oDevice );
		setROI_X0( x0,  p_oDevice );
	}

	if(y0 < getROI_Y0(p_oDevice) )
	{
		setROI_Y0( y0,  p_oDevice );
		setROI_dY( dy,  p_oDevice );
	}
	else
	{
		setROI_dY( dy,  p_oDevice );
		setROI_Y0( y0,  p_oDevice );
	}
	return 0;
}

void CalibrationManager::setFullHWROI(int p_oDevice)
{

    SmpKeyValue pKeyValue = m_rCameraDeviceProxy.get("Window.WMax", p_oDevice);
    int oWidth = ( pKeyValue ->isHandleValid() ) ? pKeyValue->value<int>() : 0;

    pKeyValue = m_rCameraDeviceProxy.get("Window.HMax", p_oDevice);
    int oHeight = ( pKeyValue ->isHandleValid() ) ? pKeyValue->value<int>() : 0;

    if (oWidth == 0 || oHeight == 0)
    {
        wmLog(eError, "CalibrationManager.%s not implemented for the current camera model \n", __FUNCTION__);
        return;
    }
    setROI(0,0, oWidth, oHeight);

}



std::string CalibrationManager::checkCamera(int p_oDevice, int maxTrials)
{
    assert(!m_isSimulation);
    if (p_oDevice != 0)
    {
        wmLog(eWarning, "CalibrationManager checkCamera: multiple sensors not supported \n");
    }
    for (int trials = 0;  trials < maxTrials; ++trials)
    {
        auto pKeyValueData = m_rCameraDeviceProxy.get("CameraName", p_oDevice);
        if (pKeyValueData->isHandleValid())
        {
            return pKeyValueData->value<std::string>();
        } 
        wmLog(eInfo, "Invalid handle received from GrabberDevice (attempt %d)\n", trials);
        Poco::Thread::sleep(1000);
        std::cout << "attempt" << trials << std::endl;
    }
    return "";
    
}


std::vector<std::uint8_t> CalibrationManager::readUserFlashData(int start, int num, int p_oDevice)
{
    assert(!m_isSimulation);

    std::cout << "calib mgr readUserFlashData " << std::endl;
    //compare Camera::getUserFlash

    //start requesting UserFlash bytes
    std::vector<std::uint8_t> result;
    result.reserve(num);
    SmpKeyValue pKeyValueAddr = new TKeyValue<int>("UserFlash.Addr", 0 );
    SmpKeyValue pKeyValueData = new TKeyValue<int>("UserFlash.Data", 0 );
    
    for (int i = start, end = start+num; i < end; ++i)
    {
        pKeyValueData = m_rCameraDeviceProxy.get("UserFlash.Data"+std::to_string(i), p_oDevice);
        if (!pKeyValueData ->isHandleValid())
        {
            result.clear();
            break;
        }
        int oValue = pKeyValueData->value<int>();
        if (oValue < 0)
        {
            result.clear();
            break;
        }
        result.push_back(static_cast<std::uint8_t> (oValue));
    }
    return result;
}

bool CalibrationManager::writeUserFlashData(std::vector<std::uint8_t> data)
{
    assert(!m_isSimulation);

    bool writeCameraOK = true;
    for (int addr = 0; addr < (int)data.size() && writeCameraOK; addr++)
    {
        auto kh = m_rCameraDeviceProxy.set( new TKeyValue<int>("UserFlash.Data"+std::to_string(addr), data[addr]));
        if(kh.handle() < 0)        {
            writeCameraOK = false;
            break;
        }
    }
    return writeCameraOK;
}



// ------------------------------------------------------------------------------------------------


void CalibrationManager::sendCalibDataChangedSignal(int p_oSensorID, bool p_oInit)
{
    std::cout << "sendCalibDataChangedSignal " << std::endl;
    //here I assume that Type_of_Sensor hasn't changed (Can't be set in calibration settings)

    // check if calibration coordinates need to be recomputed 
    
    auto & rCalibData = getCalibrationData(p_oSensorID);
    if (rCalibData.getSensorModel() == SensorModel::eUndefined)
    {
        wmLog(eError, "undefined model in sendCalibDataChangedSignal\n");
        assert(false && "undefined model in sendCalibDataChangedSignal");
    }
        
    if (!rCalibData.hasData())
    {
        if (!p_oInit)
        {
            assert(false && "CalibrationCoordinates initialization not requested, but calibration is not ready");
            wmFatal(eInternalError, "QnxMsg.Fatal.InternalError", "CalibrationCoordinates initialization not requested, but calibration is not ready\n" );
            return;
        }
        
        wmLog(eInfo, "CalibrationManager: sendCalibDataChangedSignal called, but before we need to compute coordinates field\n");
        rCalibData.load3DFieldFromParameters();
    }
  
    //force the analyzer to reload the calibration coordinates
    if (!p_oInit)
    {
        wmLog(eDebug, "p_oInit false not supported, setting to true\n");
        p_oInit=true;
    }
    
  
     //update the calibration configuration file
     
    if (!m_isSimulation)
    {
        std::cout << "Calibration manager: save parameters and read xml file back again\n";
        std::string oXmlFile = rCalibData.syncXMLContent();
        //now that the calibration file is saved to disk, CalibrationChangeNotifier is triggered and the simulation will reload the calibration 
        
        if (oXmlFile.empty())
        {
            wmLog(eError, "Error saving calibration data config\n");
            assert(false);
            return;
        }
    }
    else
    {
#ifndef NDEBUG
        std::cout << "Simulation: sendCalibDataChangedSignal but don't save to disk" << std::endl;
#endif
    }

    std::cout << "calib data changed " << std::endl;

    //send message to analyzer
    
    bool calibDataChanged_response =  m_rCalibDataMsgProxy.calibDataChanged(p_oSensorID, p_oInit); // signal analyzer (reloadCalibData and sendImages)
    if (!calibDataChanged_response)
    {
        wmLog(eWarning, "m_rCalibDataMsgProxy.calibDataChanged replied false \n");
    }

    if (p_oInit)
    {
        wmLog(eInfo, "sendCalibDataChangedSignal with init true, preparing call to set3DCoords\n");
        auto r3DCoords = rCalibData.getCalibrationCoords();
        
        assert(r3DCoords.getSensorSize().area() > 0 && "Calling set3DCoords on invalid data ");
     
        bool set3DCoords_response = m_rCalibDataMsgProxy.set3DCoords( p_oSensorID, r3DCoords, rCalibData.getParameters());
        if (!set3DCoords_response)
        {
            wmLog(eWarning, "Error sending 3DCoords to Analyzer \n");
        }
        
        if (rCalibData.hasCameraCorrectionGrid())
        {
            bool response = m_rCalibDataMsgProxy.sendCorrectionGrid( p_oSensorID, rCalibData.getCorrectionGridParameters(), rCalibData.getIDMCorrectionGridParameters());
             std::cout << "sendCorrectionGrid_response " << calibDataChanged_response << std::endl;
            if (!response)
            {
                wmLog(eWarning, "Error sending CorrectionGrid to Analyzer \n");
            }    
        }
    }
    wmLog(eDebug, "exiting sendCalibDataChangedSignal\n");
}


math::SensorModel CalibrationManager::getExpectedSensorModelFromSystemConfig() const
{
    //0:Koax,1:Scheimpflug,2:LED
    //Using -1 as default value just to avoid exception in getInt, will be handled in the switch case
    int oTypeOfSensor = SystemConfiguration::instance().getInt("Type_of_Sensor", -1);

    switch(oTypeOfSensor)
    {
        case 0:
            return SensorModel::eLinearMagnification;
        case 1:
            return SensorModel::eCalibrationGridOnLaserPlane;
        case 2:
            return SensorModel::eLinearMagnification;
        default:
            return SensorModel::eLinearMagnification;
    }
}


///get calibration data for optical system from camera/config file
//called by CalibrationManager::calibrate(eInitCalibrationData)
bool CalibrationManager::getOSCalibrationDataFromHW(const int p_oSensorID) // OS stands for optical system
{
    auto & rCalibData = getCalibrationData(p_oSensorID);
    std::cout << "Enter getOSCalibrationDataFromHW " << std::endl;
    std::cout << "CalibData HomeDir=" << rCalibData.getHomeDir() << std::endl;
	if (!rCalibData.validSensorID(p_oSensorID) )
	{
		wmLog(eWarning, "CalibrationManager::getOSCalibrationDataFromHW invalid sensor id %d \n", p_oSensorID);
		return false;
	}

	bool oRes(false);

    auto oSensorModelFromSystemConfig = getExpectedSensorModelFromSystemConfig();
    
    std::cout << "Get os calibration data: SensorModel " << SensorModelDescription(oSensorModelFromSystemConfig) << "\n";
    
    assert (!rCalibData.isInitialized());
    
    //read configuration (triang angle, etc ) and set sensorType    
    const bool createDefaultFile = !m_isSimulation;
    rCalibData.initConfig(oSensorModelFromSystemConfig, createDefaultFile);
    
    if (!rCalibData.isInitialized())
    {
        //if we are here something unexpected happened during initialization (probably missing tag in xml file),
        //the absence of calibration file it's normally handled by calibrationparammap (writes one with default values)
        wmLog(eError, "Not possible to read calibration data for sensor %d, probably configuration file is corrupted\n", p_oSensorID);
        return false;
    }
    assert (rCalibData.isInitialized());

    if (m_oHasCamera && checkCamera(p_oSensorID).empty())
    {
        wmLog(eError, "No camera found, calibration interrupted\n");
        return false;
    }
    auto cameraParameters = readCameraRelatedParameters(p_oSensorID);

    if (m_oHasCamera && !rCalibData.checkCameraRelatedParameters(cameraParameters))
    {
        const bool recomputeBetaOnSensorParameterChanged = false;
        rCalibData.changeCameraRelatedParameters(cameraParameters, recomputeBetaOnSensorParameterChanged);
        if (recomputeBetaOnSensorParameterChanged)
        {
            wmLog(eInfo, "Updating calibration parameters - camera parameters have changed \n");
        }
    }
    
	if ( oSensorModelFromSystemConfig == SensorModel::eCalibrationGridOnLaserPlane )
	{
        
        std::cout << "getOSCalibrationDataFromHW: scheimpflug case" << std::endl; 
        
        assert ((!m_isSimulation || !m_oHasCamera) && "HasCamera = True in simulation mode, getCamGridDataFromCamera will try to connect to the camera");
		oRes = m_oCalibOpticalSystem->camToCalibData(p_oSensorID);
        
		wmLog(eDebug, "getOSCalibrationDataFromHW scheimpflug camtocalibdata: %s\n", oRes ? "Y" : "N");
        
        if (oRes)
        {
            assert(rCalibData.hasData());
            assert(rCalibData.getSensorModel() == SensorModel::eCalibrationGridOnLaserPlane); //set in camToCalibData
            std::cout << "Grid computation succeded" << std::endl;
            wmLog(eInfo, "Grid computation succeded\n");
            //m_oCalibrationData.getCalibrationGrid().printGridInfo();
        }
        else
        {
            wmLogTr(eInfo, "QnxMsg.Calib.NoSpData", "No Scheimpflug calibration data available for sensor %d.\n", p_oSensorID);
            assert(DBG_ALLOW_FALLBACK);
        
            //change sensortype to coax, with default initial values if necessary
            rCalibData.setFallbackParameters(!m_isSimulation);
        
            assert (rCalibData.getSensorModel() == SensorModel::eLinearMagnification);
        }
        assert(oRes || DBG_ALLOW_FALLBACK);
    }
    
    //from System.Type_of_Sensor or because of setFallbackParameters
    if ( oSensorModelFromSystemConfig == SensorModel::eLinearMagnification || rCalibData.getSensorModel() == SensorModel::eLinearMagnification  ) 
    {
        //coax Fall ohne grid auf Kamera
        std::cout << "getOSCalibrationDataFromHW: coax case" << std::endl; 
        
        rCalibData.setSensorModel(SensorModel::eLinearMagnification);
        oRes =rCalibData.load3DFieldFromParameters();
        assert(oRes);
        wmLog(eInfo, "Coax calibration loaded\n");
    }
    
    assert(!oRes || rCalibData.checkCalibrationValuesConsistency(rCalibData.CALIB_VALUES_TOL_MIN, eWarning, true) );
    
    
    if (oRes)
    {
        // initialization successful so far,  check if CorrectionGrid is neeeded
        
        bool needsCorrectionGrid = SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::Scanner2DEnable);
        if ( needsCorrectionGrid )
        {
            auto filenameCamera = rCalibData.getFilenamesConfiguration().getCameraCorrectionGridFilename();
            rCalibData.setCalibrationCorrectionContainer(coordinates::CalibrationCameraCorrectionContainer::load( filenameCamera ));
            auto filenameIDM = rCalibData.getFilenamesConfiguration().getIDMCorrectionGridFilename();
            rCalibData.setCalibrationIDMCorrectionContainer(coordinates::CalibrationIDMCorrectionContainer::load(filenameIDM));
        }
                
        wmLog(eDebug, "getOSCalibrationDataFromHW sendCalibDataChangedSignal \n");
        //signal that the coordinates have been computed and send them to the analyzer
        sendCalibDataChangedSignal(p_oSensorID, /*p_oInit*/ true); 
    }
    else
    {
        wmLog(eError, "getOSCalibrationDataFromHW not successful, calibration not sent to analyzer\n ");
    }
    std::cout << "getOSCalibrationDataFromHW return value = " << oRes << "\n";
	return oRes;
}

// determine triangulation angle AFTER grid has been evaluated
bool CalibrationManager::determineTriangulationAngle(const int p_oSensorID, const math::CameraRelatedParameters & rCameraParameters)
{
	bool oRes = false;
	std::cout<<"+++++++++++++++++berechne den triangulationswinkel++++++++++++++++++"<<std::endl;

    auto & rCalibData= getCalibrationData(p_oSensorID);

	if (!rCalibData.validSensorID(p_oSensorID) )
	{
        return oRes;
    }

    std::cout<<"-------determineTriangulationAngle calibrationManager sensor valid------"<<std::endl;
    if ( rCalibData.hasData() && rCalibData.getSensorModel() == SensorModel::eCalibrationGridOnLaserPlane)
    {
        // welche laserlinie bzw. betaz
        std::cout<<"-------determineTriangulationAngle calibrationManager------"<<std::endl;
        bool isCoax = false;
        oRes = m_oCalibOpticalSystem->determineTriangulationAngle(p_oSensorID, isCoax, rCameraParameters);
        
    } else
    {
        wmLogTr(eWarning, "QnxMsg.Calib.NoScheimSys", "Sensor %d does not reference a Scheimpflug system!\n", p_oSensorID);
    }


	return oRes ;
}


bool CalibrationManager::get3DCoordinates(math::Vec3D & rCoords, unsigned int pX, unsigned int pY, math::SensorId p_oSensorID, filter::LaserLine p_LaserLine)
{
    auto & rCalibData= getCalibrationData(p_oSensorID);
    if (!rCalibData.validSensorID(p_oSensorID) )
    {
        return false;
    }
    float x,y,z;
   
    bool validConversion = rCalibData.getCalibrationCoords().convertScreenTo3D(x,y,z, pX, pY, p_LaserLine);
    rCoords = math::Vec3D(x,y,z);
    return validConversion;

}


geo2d::DPoint CalibrationManager::getCoordinatesFromGrayScaleImage(double xScreen, double yScreen, const interface::ScannerContextInfo & rContextInfo, math::SensorId p_oSensorID)
{
    auto & rCalibData= getCalibrationData(p_oSensorID);
    if (!rCalibData.validSensorID(p_oSensorID) )
    {
        return {-1000,-1000};
    }

    std::vector<double> K;

    if (rCalibData.scanfieldDistortionCorrectionFactor(K))
    {
        const auto distortionCoefficient = scannerPositionToDistortionCoefficient(rContextInfo.m_x, rContextInfo.m_y, K);
        auto tcp = getTCPPosition(p_oSensorID, rContextInfo, LaserLine::FrontLaserLine); //FIXME (see also CalibrationCoordinatesRequestServer.getTCPPosition)
        const auto point = pixelToWorld(xScreen - tcp.x, yScreen - tcp.y, distortionCoefficient);

        return {point.first, point.second};
    }

    float x_mm,y_mm;
   
    bool validConversion = rCalibData.getCalibrationCoords().convertScreenToHorizontalPlane(x_mm,y_mm, std::round(xScreen), std::round(yScreen));
    if (!validConversion)
    {
        return {-1000,-1000};
    }

    return geo2d::DPoint(x_mm, y_mm);
}

geo2d::DPoint CalibrationManager::getTCPPosition(math::SensorId p_oSensorId, const interface::ScannerContextInfo & rContextInfo, filter::LaserLine p_LaserLine)
{
    auto & rCalibData = getCalibrationData(p_oSensorId);
    return rCalibData.getTCPCoordinate(rContextInfo, p_LaserLine);   // calibData applies the tcp offset, while CalibrationParamMap shows the base value
}

geo2d::DPoint CalibrationManager::getScreenPointZ0(LaserLine laserLine)
{
    auto & rCalibData = getCalibrationData(0);
    assert(rCalibData.getCalibrationCoords().usesOrientedLineCalibration());

    auto laserLineXY = rCalibData.getZLineForOrientedLaserLine(laserLine, 0.0);
    assert(laserLineXY.isValid());
    auto itPreferredPoint = m_screenPointZ0.find(laserLine);
    auto preferredPoint = itPreferredPoint != m_screenPointZ0.end() ? itPreferredPoint->second : geo2d::DPoint{0,0};
    if (laserLineXY.distance(preferredPoint.x, preferredPoint.y) < 1e-6)
    {
        return preferredPoint;
    }
    m_screenPointZ0[laserLine] = rCalibData.getZPointForOrientedLaserLine(laserLine);
    return m_screenPointZ0[laserLine];

}
void CalibrationManager::setScreenPointZ0(LaserLine laserLine, geo2d::DPoint point)
{

    auto & rCalibData = getCalibrationData(0);
    assert(rCalibData.getCalibrationCoords().usesOrientedLineCalibration());
    rCalibData.adjustZPointForOrientedLaserLine(point, laserLine);
    m_screenPointZ0[laserLine] = point;

}

void CalibrationManager::configureAsSimulationStation(bool set)
{
    if (set != m_isSimulation)
    {
        std::cout << " set simulation from " << m_isSimulation << " to " << set << " reset calibration data instances "<< std::endl;
        m_isSimulation = set;
        for (auto && rCalibData : m_oCalData)
        {
            rCalibData.resetConfig(/*canwritetodisk*/!m_isSimulation, rCalibData.getHomeDir());
        }
    }
    wmLog(eInfo, "Calibration manager simulation = %s\n", set? "T":"F");
}

interface::SmpKeyValue CalibrationManager::getIDMKeyValue(interface::Key p_key, int p_subDevice) 
{ 
    return  m_rIDMDeviceProxy.get(p_key, p_subDevice);
}

interface::KeyHandle CalibrationManager::setIDMKeyValue ( interface::SmpKeyValue p_keyValue, int p_subDevice ) 
{
    return m_rIDMDeviceProxy.set(p_keyValue, p_subDevice);
}

std::vector<image::Sample> CalibrationManager::getTCPSearchSamples()
{
    cancelTrigger ( {eIDMTrackingLine} ); //make sure to reload newson parameters
    
    unsigned int numLines = getIDMKeyValue ( "TCP Search Loop Count" )->value<int>();
    if (numLines == 0)
    {
        wmLog(eWarning,"TCP search with 0 lines requested\n");
        return {};
    }
    
    m_oSampleSensorId = eIDMTrackingLine;
    startStoringFrames(numLines+2);
  
    
    interface::SmpKeyValue pKeyValue = new interface::TKeyValue<bool> ( "TCP Search", true );
    auto h = setIDMKeyValue ( pKeyValue );
    
    stopStoringFrames();
    
    if (h.handle() != 2)
    {
        wmLog(eWarning, "Unexpected return value from setKeyValue TCP Search \n");
        return {};
    }
    if (m_oSamples.size() >0 && m_oSamples.back().numElements() != 5)
    {
        wmLog(eWarning, "Incomplete samples received during TCP search %d last sample elements =%d  \n", m_oSamples.size()-1, m_oSamples.back().numElements());
        return {};
    }
    
    if (m_oSamples.size() < numLines || m_oSamples.back().numElements() != 5)
    {
        wmLog(eWarning, "Incomplete samples received during TCP search numSamples =%d  \n", m_oSamples.size());
        return {};
    }
    m_oSamples.pop_back();
    return m_oSamples;
}

bool CalibrationManager::updateOCTCalibrationData(CalibrationOCTMeasurementModel p_OCTMeasurementModel)
{
    if (m_pCalibrationOCTData == nullptr)
    {
        return false;
    }
    m_pCalibrationOCTData->updateMeasurementModel(p_OCTMeasurementModel);
    m_pCalibrationOCTData->write(getCalibrationData(0).getHomeDir());
    return m_pCalibrationOCTData->initializeSystem(*this);
}

bool CalibrationManager::updateOCTCalibrationData(CalibrationOCTMeasurementModel p_OCTMeasurementModel, interface::Configuration p_oKeyValues)
{ 
    if (m_pCalibrationOCTData == nullptr)
    {
        wmLog(eError, "Calibration OCT Data never initialized\n");
        return false;
    }
    
    bool valid = false;
    for (const auto & rKv: p_oKeyValues)
    {
        valid |= (setCalibrationOCTKeyValue(rKv, false).handle() != -1); //update keyValue but not initialize system (it will be done when updating p_OCTMeasurementModel
    }
    if (!valid)
    {
        return false;
    }
    return updateOCTCalibrationData(p_OCTMeasurementModel);
}

interface::Configuration CalibrationManager::getCalibrationOCTConfiguration() const
{
    if (m_pCalibrationOCTData == nullptr)
    {
        wmLog(eError, "Calibration OCT Data never initialized\n");
        return {};
    }
    return m_pCalibrationOCTData->makeConfiguration(true);
}

interface::SmpKeyValue CalibrationManager::getCalibrationOCTKeyValue(std::string p_key) const
{
    if (m_pCalibrationOCTData == nullptr)
    {
        wmLog(eError, "Calibration OCT Data never initialized\n");
        return {};
    }
    return m_pCalibrationOCTData->getKeyValue(p_key);
}

interface::KeyHandle CalibrationManager::setCalibrationOCTKeyValue(interface::SmpKeyValue p_keyValue, bool p_updateSystem)
{
    if (m_pCalibrationOCTData == nullptr)
    {
        wmLog(eError, "Calibration OCT Data never initialized\n");
        return {};
    }
    auto oldValue = m_pCalibrationOCTData->getKeyValue(p_keyValue->key());
    if (oldValue.isNull() ||  !(oldValue->isHandleValid()))
    {
        return {-1}; // invalid  handle
    }
    auto kh =  m_pCalibrationOCTData->setKeyValue(p_keyValue);
    if (p_updateSystem)
    {
        m_pCalibrationOCTData->write(getCalibrationData(0).getHomeDir());
        if (m_pCalibrationOCTData->equivalentCoaxCalibModified())
        {
            m_pCalibrationOCTData->initializeSystem(*this);
        }
    }
    return kh;
}



const CalibrationOCTData * CalibrationManager::getCalibrationOCTData() const
{
    return m_pCalibrationOCTData;
}

void CalibrationManager::setZCollDrivingRelative(double value)
{
    m_ZCollDrivingRelative = value;
}

void CalibrationManager::setLaserPowerInPctForCalibration(double value)
{
    m_laserPowerInPct = value;
}

void CalibrationManager::setWeldingDurationInMsForCalibration(double value)
{
    m_weldingDurationInMs = value;
}

void CalibrationManager::setJumpSpeedInMmPerSecForCalibration(double value)
{
    m_jumpSpeedInMmPerSec = value;
}

double CalibrationManager::getZCollDrivingRelative() const
{
    return m_ZCollDrivingRelative.value();
}

double CalibrationManager::getLaserPowerInPctForCalibration() const
{
    return m_laserPowerInPct;
}

double CalibrationManager::getWeldingDurationInMsForCalibration() const
{
    return m_weldingDurationInMs;
}

double CalibrationManager::getJumpSpeedInMmPerSecForCalibration() const
{
    return m_jumpSpeedInMmPerSec;
}

bool CalibrationManager::availableOCTCoordinates ( interface::OCT_Mode mode )
{
    switch(mode)
    {
        case interface::OCT_Mode::eScan1D:
            //not implemented
            return false;
        case interface::OCT_Mode::eScan2D:
            return (m_pCalibrationOCT_TCP != nullptr);
        default:
            return false;
    }
}

geo2d::DPoint CalibrationManager::getNewsonPosition ( double xScreen, double yScreen, interface::OCT_Mode mode )
{
    if (mode == interface::OCT_Mode::eScan2D && (m_pCalibrationOCT_TCP == nullptr))
    {
        wmLog(eWarning, "OCT 2D Scan not performed yet\n");
        mode = interface::OCT_Mode::eScan1D;
    }
        
    if (mode == interface::OCT_Mode::eScan2D)
    {
        return m_pCalibrationOCT_TCP->pixelScan2DtoNewson({xScreen, yScreen});
    }
    
    if (m_pCalibrationOCTData == nullptr)
    {
        wmLog(eError, "Calibration OCT Data never initialized\n");
        return {0,0};
    }
    
    //TODO
    wmLog(eWarning, "getNewsonPosition on scan line not implemented \n");
    return {0.0, 0.0};
    
}
 

geo2d::DPoint CalibrationManager::getScreenPositionFromNewson ( double xNewson, double yNewson, interface::OCT_Mode mode )
{    
    if (mode == interface::OCT_Mode::eScan2D && (m_pCalibrationOCT_TCP == nullptr))
    {
        wmLog(eWarning, "OCT 2D Scan not performed yet\n");
        mode = interface::OCT_Mode::eScan1D;
    }
        
    if (mode == interface::OCT_Mode::eScan2D)
    {
        auto ret = m_pCalibrationOCT_TCP->newsonToPixelScan2D({xNewson, yNewson});
        return ret;
    }
    
    if (m_pCalibrationOCTData == nullptr)
    {
        wmLog(eError, "Calibration OCT Data never initialized\n");
        return {0,0};
    }
    

    //TODO
    wmLog(eWarning, "getScreenPositionFromNewson on scan line not implemented \n");
    return {0.0, 0.0};
    
}

void CalibrationManager::startStoringFrames ( int numberOfFrames )
{

    m_oFramesToStore = numberOfFrames;
    
    m_oSamples.resize ( 0 );
    m_oSamples.reserve (numberOfFrames);

    m_oImages.resize ( 0 );
    m_oImages.reserve (numberOfFrames);

    m_oContexts.resize ( 0 );
    m_oContexts.reserve (numberOfFrames);

    m_oStoreFrames = true;
}


void CalibrationManager::stopStoringFrames()
{
    m_oStoreFrames = false;
}


void CalibrationManager::setScannerPosition(double x, double y)
{
    assert(!m_isSimulation && "Weldhead device proxy not activated");
    
    wmLog(eInfo, "Set scanner position %f , %f \n", x, y);

    SmpKeyValue  smpKeyValX = new TKeyValue<double>("Scanner_New_X_Position",x);
    KeyHandle kh = m_rWeldHeadDeviceProxy.set(smpKeyValX,0);
    SmpKeyValue  smpKeyValY = new TKeyValue<double>("Scanner_New_Y_Position",y);
    kh = m_rWeldHeadDeviceProxy.set(smpKeyValY,0);
    SmpKeyValue  smpKeyValPos = new TKeyValue<bool>("Scanner_DriveToPosition",true);
    kh = m_rWeldHeadDeviceProxy.set(smpKeyValPos,0);
}

double CalibrationManager::getJumpSpeed() const
{
    assert(!m_isSimulation && "Weldhead device proxy not activated");
    auto kv = m_rWeldHeadDeviceProxy.get("Scanner_Jump_Speed");
    if (!kv)
    {
        return 0.0;
    }
    return kv->value<double>();

}
void CalibrationManager::setJumpSpeed(double speed)
{
    assert(!m_isSimulation && "Weldhead device proxy not activated");
    SmpKeyValue  smpKeyValX = new TKeyValue<double>("Scanner_Jump_Speed", speed);
    m_rWeldHeadDeviceProxy.set(smpKeyValX,0);
}


geo2d::DPoint CalibrationManager::getScannerPositionFromScanFieldImage(double x_pixel, double y_pixel, interface::Configuration scanfieldImageConfiguration) const
{
    int oSensorID = 0;
 
    auto oScanFieldImageParameters = ScanFieldImageParameters::loadConfiguration(scanfieldImageConfiguration);
    if (scanfieldImageConfiguration.size() == 0)
    {
        wmLog(eInfo,  "No scanfieldImageConfiguration found, use current calibration parameters \n");
        oScanFieldImageParameters = ScanFieldImageParameters::computeParameters( getCalibrationData(oSensorID).getParameters());
    }
    return oScanFieldImageParameters.getScannerPositionFromScanFieldImage(x_pixel, y_pixel);
}


geo2d::DPoint CalibrationManager::getScanFieldImagePositionFromScannerPosition(double x_mm, double y_mm, interface::Configuration scanfieldImageConfiguration) const
{
 
    auto oScanFieldImageParameters = ScanFieldImageParameters::loadConfiguration(scanfieldImageConfiguration);
    if (scanfieldImageConfiguration.size() == 0)
    {
        int oSensorID = 0;
        wmLog(eWarning,  "No scanfieldImageConfiguration found, use current calibration parameters \n");
        oScanFieldImageParameters = ScanFieldImageParameters::computeParameters( getCalibrationData(oSensorID).getParameters());
    }
    return oScanFieldImageParameters.getCenterInScanFieldImage( x_mm, y_mm);
}

void CalibrationManager::setOCTReferenceArm(int n)
{
    wmLog(eInfo, "Set OCT reference arm position %f", n);

    SmpKeyValue referenceArm{new TKeyValue<int>("OCT_Reference_Arm", n)};
    m_rWeldHeadDeviceProxy.set(referenceArm, 0);
    SmpKeyValue enable{new TKeyValue<bool>("Scanner_SetOCTReference", true)};
    m_rWeldHeadDeviceProxy.set(enable, 0);
}

void CalibrationManager::weldHeadReloadFiberSwitchCalibration()
{
    m_rWeldHeadMsgProxy.reloadFiberSwitchCalibration();
}

std::string CalibrationManager::getScanFieldPath() const
{
    return m_oScanFieldPath;
}


void CalibrationManager::setScanFieldPath(std::string path)
{
    m_oScanFieldPath = path;
}


int CalibrationManager::chessboardRecognitionThreshold(const BImage & chessboardImage, int threshold)
{
    using precitec::calibration_algorithm::ChessboardRecognitionAlgorithm;

    ChessboardRecognitionAlgorithm oChessboardRecognition(chessboardImage, threshold, ChessboardRecognitionAlgorithm::PreviewType::AfterBinarization);
    int actualThreshold = oChessboardRecognition.getThreshold();
    drawImage(0,0, oChessboardRecognition.getPreviewImage(), "Threshold " + std::to_string(actualThreshold), Color::Green());
    return actualThreshold;


}


bool CalibrationManager::calibrateLaserPlaneWithChessboardRecognition(int threshold, CamGridData::ScheimpflugCameraModel cameraModel)
{
    using precitec::calibration_algorithm::ChessboardRecognitionAlgorithm;
    using coordinates::CalibrationConfiguration;

    clearCanvas();

    if (getExpectedSensorModelFromSystemConfig() != math::SensorModel::eCalibrationGridOnLaserPlane)
    {
        wmLog(eWarning, "Performing Scheimpflug calibration, but the system is configured as coax \n");
    }

    geo2d::Point initialHWROIStart;
    geo2d::Size initialHWROISize;
    getHWRoi(initialHWROIStart.x, initialHWROIStart.y, initialHWROISize.width, initialHWROISize.height);

    setFullHWROI();
    auto oChessboardImage = getImage();
    setHWRoi(initialHWROIStart.x, initialHWROIStart.y, initialHWROISize.width, initialHWROISize.height);

    auto actualThreshold = chessboardRecognitionThreshold(oChessboardImage, threshold);
    renderImage(oChessboardImage);

    ChessboardRecognitionAlgorithm oChessboardRecognition(oChessboardImage, actualThreshold);
    for (auto & corner : oChessboardRecognition.getRecognizedCorners())
    {
        drawPixel(corner.x, corner.y, Color::Red());
    }
    renderImage(oChessboardImage);
    if (!oChessboardRecognition.isValid())
    {
        return false;
    }
    precitec::math::CalibrationCornerGrid cornerGrid = oChessboardRecognition.getCornerGrid();

    for (const auto & segment : cornerGrid.getAllSegmentsScreen())
    {
        const precitec::geo2d::coord2DScreen & rPointA = segment[0];
        const precitec::geo2d::coord2DScreen & rPointB = segment[1];
        drawLine(rPointA.ScreenX, rPointA.ScreenY, rPointB.ScreenX, rPointB.ScreenY, Color::Yellow());
    }
    renderImage(oChessboardImage);

    CamGridData oCamGridData;
    oCamGridData.setGridDelta(CamGridData::scheimpflugCameraPatternSize(cameraModel));
    oCamGridData.setTriangulationAngle_rad(math::degreesToRadians(CamGridData::scheimpflugCameraAngles(cameraModel)[0] + CamGridData::scheimpflugCameraAngles(cameraModel)[1]));
    oCamGridData.setSensorWidth(oChessboardImage.width());
    oCamGridData.setSensorHeight(oChessboardImage.height());
    oCamGridData.setGridMap(cornerGrid.getGrid2D());
    //TODO camera serial number


    //reinitialize the calibration data
    int p_oSensorID = 0;
    auto& rCalibrationData = getCalibrationData(p_oSensorID);
    if (getExpectedSensorModelFromSystemConfig() != math::SensorModel::eCalibrationGridOnLaserPlane
        || rCalibrationData.getSensorModel() != math::SensorModel::eCalibrationGridOnLaserPlane )
    {
        wmLog(eWarning, "Updated Scheimpflug calibration, but the system is configured as coax - change System config before reboot\n");
        //at the time of the first calibration the system is probably using the fallback coax config, so we need to force the scheimpflug config
        rCalibrationData.setSensorModel(math::SensorModel::eCalibrationGridOnLaserPlane);
    }

    rCalibrationData.setKeyValue("scheimOrientationAngle", 0.0);
    //Note: dpixX, dpixY not used for this calibration
    bool ok = rCalibrationData.load3DFieldFromCamGridData(oCamGridData);
    if (!ok)
    {
        wmLog(eWarning, "Error in loading calibration grid");
        return false;
    }
    rCalibrationData.setKeyValue ("SensorParametersChanged",false);
    //signal that the coordinates have been computed and send them to the analyzer
    sendCalibDataChangedSignal(p_oSensorID, /*p_oInit*/ true);


    //export the calibration grid
    const auto & rFilenames = rCalibrationData.getFilenamesConfiguration();
    const std::string oCalibBinCacheFile = rFilenames.getCamGridDataBinaryFilename();
    const std::string oCalibFallbackFile = rFilenames.getCSVFallbackFilename();
    const std::string oCalibCopyFile = rFilenames.getCopyCSVFilename(); //human readable copy of the calibration data, if empty no copy is saved

    oCamGridData.saveToBytes(oCalibBinCacheFile);
    oCamGridData.saveToCSV(oCalibFallbackFile);

    std::vector<std::uint8_t> oBytes;
    oCamGridData.saveToBytes(oBytes);
    writeUserFlashData(oBytes);

    return true;
}

bool CalibrationManager::weldForScannerCalibration()
{
    const auto sensorID = 0;
    const auto calibrationParameters = getCalibrationData(sensorID).getParameters();
    const auto xMin = calibrationParameters.getDouble("SM_X_min");
    const auto xMax = calibrationParameters.getDouble("SM_X_max");
    const auto yMin = calibrationParameters.getDouble("SM_Y_min");
    const auto yMax = calibrationParameters.getDouble("SM_Y_max");
    const auto weldingPoints = calibration_algorithm::ChessboardWeldPointGenerator(xMin, yMin, xMax, yMax).getWeldingPoints();

    if(weldingPoints.size() != 0)
    {
        if (!m_ZCollDrivingRelative.has_value())
        {
            m_ZCollDrivingRelative = m_rWeldHeadDeviceProxy.get(std::string{"Z_Collimator_SystemOffset"})->value<double>();
        }
        const auto zCollDrivingRelativeInUm = m_ZCollDrivingRelative.value() * 1000; // converts from mm to um
        doZCollDrivingRelative(static_cast<int>(zCollDrivingRelativeInUm));
        m_rWeldHeadMsgProxy.weldForScannerCalibration(weldingPoints, m_laserPowerInPct, m_weldingDurationInMs, m_jumpSpeedInMmPerSec);
        m_ZCollDrivingRelative.reset();
    }
    return true;
}

math::CameraRelatedParameters CalibrationManager::readCameraRelatedParameters(int oSensorID) const
{
    const auto& calibData = getCalibrationData(oSensorID);
    if (!calibData.isInitialized())
    {
        wmLog(eError, "Invalid default values in readCameraRelatedParameters, it must be called after loading the calibration data\n");
    }
    const auto& calibDataParameters = calibData.getParameters();
    auto getIntParameterFromCamera = [this, &oSensorID, &calibDataParameters](std::string cameraKey, std::string calibrationKey)
    {
        int parameterValue = calibDataParameters.getInt(calibrationKey); //use current calibration value as default
        auto pKeyValueData = m_rCameraDeviceProxy.get(cameraKey, oSensorID);
        if (pKeyValueData->isHandleValid())
        {
            parameterValue = pKeyValueData->value<int>();
        }
        else
        {
            wmLog(eWarning, "Error reading camera parameter %s. Using default value %d from calibration parameter %s\n",
                cameraKey.c_str(), parameterValue, calibrationKey.c_str());
        }
        return parameterValue;
    };

    math::CameraRelatedParameters cameraParameters;
    cameraParameters.m_oWidth = getIntParameterFromCamera("Window.WMax", "sensorWidth");
    cameraParameters.m_oHeight = getIntParameterFromCamera("Window.HMax", "sensorHeight");

    //TODO read pixel size from grabber
    cameraParameters.m_oDpixX = calibDataParameters.getDouble("DpixX");
    cameraParameters.m_oDpixY = calibDataParameters.getDouble("DpixY");
    return cameraParameters;
}



} // namespace calibration
} // namespace precitec
