/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		AL, EA, HS
 * 	@date		2010
 * 	@brief		Controls welding head hardware and extern sensors via ethercat bus.
 */

#include <chrono>

#include "viWeldHead/WeldingHeadControl.h"
#include "viWeldHead/outMotionDataServer.h"
#include "module/moduleLogger.h"
#include <sys/prctl.h>

using Poco::XML::SAXParser;
using Poco::XML::XMLReader;
using Poco::FastMutex;
using namespace precitec::interface;

namespace precitec
{

using viWeldHead::OutMotionDataServer;
using namespace interface;

namespace ethercat
{

#define DEBUG_NEW_ZCOLLIMATOR 0
#define EMVTEST_NEW_ZCOLLIMATOR 0

#define SIGNAL_CHANGE_DELTA 10

// folgende CLOCK... verwenden
#define CLOCK_TO_USE CLOCK_MONOTONIC

// folgendes definiert die Anzahl ns pro Sekunde
#define NSEC_PER_SEC    (1000000000)

// Folgendes definiert eine Zykluszeit von 1ms
#define CYCLE_TIME_NS   (1000000)

#define CYCLE_TIMING_VIA_SERIAL_PORT     0

static void *ZCollEndOfDrivingThread(void* p_pArg);

void* SendSensorDataThread(void *p_pArg);
void* ZCPositionDigV1ReadOutThread(void *p_pArg);

///////////////////////////////////////////////////////////
// global variables for debugging purposes
///////////////////////////////////////////////////////////

#if CYCLE_TIMING_VIA_SERIAL_PORT
int g_oDebugSerialFd;
int g_oDTR01_flag;
int g_oRTS02_flag;
#endif

WeldingHeadControl::WeldingHeadControl(OutMotionDataServer& outServer,
										TSensor<AbstractInterface> &sensorProxy,
										TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy):
				m_outMotionDataServer(outServer),
				m_rEthercatOutputsProxy(p_rEthercatOutputsProxy),
				m_pStatesHeadX(NULL),
				m_pStatesHeadY(NULL),
				m_pStatesHeadZ(NULL),
				m_oDebugInfo_AxisController(false),
				m_oLaserPowerSignal(0),
				m_oRobotTrackSpeed(0),
				m_oOversamplingSignal1RingBuffer(20000),
				m_oOversamplingSignal2RingBuffer(20000),
				m_oOversamplingSignal3RingBuffer(20000),
				m_oOversamplingSignal4RingBuffer(20000),
				m_oOversamplingSignal5RingBuffer(20000),
				m_oOversamplingSignal6RingBuffer(20000),
				m_oOversamplingSignal7RingBuffer(20000),
				m_oOversamplingSignal8RingBuffer(20000),
				m_oLWM40_1_PlasmaRingBuffer(20000),
				m_oLWM40_1_TemperatureRingBuffer(20000),
				m_oLWM40_1_BackReflectionRingBuffer(20000),
				m_oLWM40_1_AnalogInputRingBuffer(20000),
				m_oEncoderInput1(0),
				m_oEncoderInput2(0),
				m_oStartBitScannerOK(0),
				m_oStartBitScannerLimits(0),
				m_oEncoderRevolutions(0),
				m_oStartBitZCError(0),
				m_oStartBitZCPosReached(0),
				m_oZCRefTravelIsActive(false),
				m_oZCAutomaticIsActive(false),
				m_oZCError(true),
				m_oZCPosReached(false),
				m_oZCStateVarAutomatic(0),
				m_oZCStateVarRefTravel(0),
				m_oZCNewPositionValue(0),
				m_oZCActPosition(0),
				m_oZCSystemOffset(0.0),
				m_oZCLensToFocusRatio(3.73),
				m_oNextAbsolutePositionV2Um(0.0),
				m_oZCDrivingV2IsActive(false),
				m_oZCHomingV2IsActive(false),
				m_oZCPositionDigV1ReadOutThread_ID(0),
				m_oZCPositionDigV1ReadOut(0),
				m_oStartBitLCErrorSignal(0),
				m_oStartBitLCReadySignal(0),
				m_oStartBitLCLimitWarning(0),
				m_oStartBitLEDTemperatureHigh(0),
				m_oSensorProxy (sensorProxy),
				m_oSendSensorDataThread_ID(0),
				m_interval(0,0),

				m_imageNrGPAI{0},
				m_pValuesGPAI1(nullptr),
				m_pValuesGPAI2(nullptr),
				m_pValuesGPAI3(nullptr),
				m_pValuesGPAI4(nullptr),
				m_pValuesGPAI5(nullptr),
				m_pValuesGPAI6(nullptr),
				m_pValuesGPAI7(nullptr),
				m_pValuesGPAI8(nullptr),
				m_imageNrLP(0),
				m_pValuesLP(NULL),
				m_imageNrRobSpeed(0),
				m_pValuesRobSpeed(NULL),
				m_imageNrENC1(0),
				m_pValuesENC1(NULL),
				m_imageNrENC2(0),
				m_pValuesENC2(NULL),

				m_imageNrOverSmp1(0),
				m_pValuesOverSmp1(NULL),
				m_imageNrOverSmp2(0),
				m_pValuesOverSmp2(NULL),
				m_imageNrOverSmp3(0),
				m_pValuesOverSmp3(NULL),
				m_imageNrOverSmp4(0),
				m_pValuesOverSmp4(NULL),
				m_imageNrOverSmp5(0),
				m_pValuesOverSmp5(NULL),
				m_imageNrOverSmp6(0),
				m_pValuesOverSmp6(NULL),
				m_imageNrOverSmp7(0),
				m_pValuesOverSmp7(NULL),
				m_imageNrOverSmp8(0),
				m_pValuesOverSmp8(NULL),

				m_imageNrScannerXPos(0),
				m_pValuesScannerXPos(nullptr),
				m_imageNrScannerYPos(0),
				m_pValuesScannerYPos(nullptr),
				m_imageNrFiberSwitchPos(0),
				m_pValuesFiberSwitchPos(nullptr),
				m_imageNrZCPositionDigV1(0),
				m_pValuesZCPositionDigV1(nullptr),
				m_imageNrScannerWeldingFinished(0),
				m_pValuesScannerWeldingFinished(nullptr),
				m_imageNrContourPreparedFinished(0),
				m_pValuesContourPreparedFinished(nullptr),

				m_imageNrLWM40_1_Plasma(0),
				m_pValuesLWM40_1_Plasma(nullptr),
				m_imageNrLWM40_1_Temperature(0),
				m_pValuesLWM40_1_Temperature(nullptr),
				m_imageNrLWM40_1_BackReflection(0),
				m_pValuesLWM40_1_BackReflection(nullptr),
				m_imageNrLWM40_1_AnalogInput(0),
				m_pValuesLWM40_1_AnalogInput(nullptr),

				m_oLineLaser1Intensity(0),
				m_oLineLaser1OnOff(false),
				m_oLineLaser2Intensity(0),
				m_oLineLaser2OnOff(false),
				m_oFieldLight1Intensity(0),
				m_oFieldLight1OnOff(false),
				m_oLineLas1IntensToWin(0),
				m_oLineLas1OnOffToWin(false),
				m_oLineLas2IntensToWin(0),
				m_oLineLas2OnOffToWin(false),
				m_oFieldL1IntensToWin(0),
				m_oFieldL1OnOffToWin(false),
				m_oRequiredPositionX(0),
				m_oRequiredPositionY(0),
				m_oRequiredPositionZ(0),
				m_pSerialToTracker(NULL),
				m_pDataToLaserControl(NULL),
				m_oFocalLength(250),		// Standard ist Brennweite 250mm
				m_oTrackerFrequencyStep(eFreq250),
				m_oTrackerFrequencyCont(100),
				m_oTrackerFrequencyBoth(100),
				m_oScannerOK(false),
				m_oScannerLimits(false),
				m_oTrackerDriverOnOff(false),
				m_oTrackerExpertMode(false),
				m_oLCStartSignal(false),
				m_oScanWidthOutOfGapWidth(false),
				m_oScanWidthFixed(0),
				m_oScanWidthFixedWasSet(false),
				m_oScanWidthControlled(0),
				m_oGapWidthToScanWidthOffset(0),
				m_oGapWidthToScanWidthGradient(1),
				m_oScanWidthUMSent(0),
				m_oScanWidthVoltSent(0),
				m_oScanWidthLimitedUM(false),
				m_oScanWidthLimitedVolt(false),
				m_oScanPosOutOfGapPos(true),
				m_oScanPosFixed(0),
				m_oScanPosFixedWasSet(false),
				m_oScanPosControlled(0),
				m_oMaxScanPosUM(6000),		// bei Brennweite 250mm
				m_oMaxScanPosMM(6.0),		// bei Brennweite 250mm
				m_oMinScanPosUM(-6000),		// bei Brennweite 250mm
				m_oMinScanPosMM(-6.0),		// bei Brennweite 250mm
				m_oScanPosUMSent(0),
				m_oScanPosVoltSent(0),
				m_oScanPosLimitedUM(false),
				m_oScanPosLimitedVolt(false),
				m_oTrackerMaxAmplitude(5000),
				m_pScanlab(nullptr),
				m_oLWM40_1_AmpPlasma(1),
				m_oLWM40_1_AmpTemperature(1),
				m_oLWM40_1_AmpBackReflection(1),
				m_oLWM40_1_AmpAnalogInput(1),
				m_oCycleIsOn(false),
				m_oSeamIsOn(false),
				m_bGlasNotPresentCounter(0),
				m_bGlasDirtyCounter(0),
				m_bTempGlasFailCounter(0),
				m_bTempHeadFailCounter(0),
				m_oPrintDebug1(false)
{
    for(int i = eAnalogIn1;i <= eLastAnalogIn;i++)
    {
        m_oGenPurposeAnaInProxyInfo[i].m_oActive = false;
        m_oGenPurposeAnaIn[i].store(0);
    }
	m_oLaserPowerSignalProxyInfo.m_oActive = false;
	m_oRobotTrackSpeedProxyInfo.m_oActive = false;

	m_oOversamplingSignal1ProxyInfo.m_oActive = false;
	m_oOversamplingSignal2ProxyInfo.m_oActive = false;
	m_oOversamplingSignal3ProxyInfo.m_oActive = false;
	m_oOversamplingSignal4ProxyInfo.m_oActive = false;
	m_oOversamplingSignal5ProxyInfo.m_oActive = false;
	m_oOversamplingSignal6ProxyInfo.m_oActive = false;
	m_oOversamplingSignal7ProxyInfo.m_oActive = false;
	m_oOversamplingSignal8ProxyInfo.m_oActive = false;

	m_oLWM40_1_PlasmaProxyInfo.m_oActive = false;
	m_oLWM40_1_TemperatureProxyInfo.m_oActive = false;
	m_oLWM40_1_BackReflectionProxyInfo.m_oActive = false;
	m_oLWM40_1_AnalogInputProxyInfo.m_oActive = false;

	m_oLineLaser1IntensityProxyInfo.m_oActive = false;
	m_oLineLaser2IntensityProxyInfo.m_oActive = false;
	m_oFieldLight1IntensityProxyInfo.m_oActive = false;
    for(int i = eAnalogOut1;i <= eLastAnalogOut;i++)
    {
        m_oGenPurposeAnaOutProxyInfo[i].m_oActive = false;
    }
	m_oTrackerScanWidthProxyInfo.m_oActive = false;
	m_oTrackerScanPosProxyInfo.m_oActive = false;
	m_oZCAnalogInProxyInfo.m_oActive = false;
	m_oLCPowerOffsetProxyInfo.m_oActive = false;

	m_oSTScannerOkProxyInfo.m_oActive = false;
	m_oSTScannerLimitsProxyInfo.m_oActive = false;
	m_oZCErrorProxyInfo.m_oActive = false;
	m_oZCPosReachedProxyInfo.m_oActive = false;
	m_oLCErrorSignalProxyInfo.m_oActive = false;
	m_oLCReadySignalProxyInfo.m_oActive = false;
	m_oLCLimitWarningProxyInfo.m_oActive = false;

	m_oLEDTemperatureHighProxyInfo.m_oActive = false;

	m_oSTEnableDriverProxyInfo.m_oActive = false;
	m_oZCRefTravelProxyInfo.m_oActive = false;
	m_oZCAutomaticProxyInfo.m_oActive = false;
	m_oLCStartSignalProxyInfo.m_oActive = false;

	m_oEncoderInput1ProxyInfo.m_oActive = false;
	m_oEncoderInput2ProxyInfo.m_oActive = false;

	m_oEncoderReset1ProxyInfo.m_oActive = false;
	m_oEncoderReset2ProxyInfo.m_oActive = false;

    m_bGlasNotPresent = false;
	m_bGlasDirty = false;
	m_bTempGlasFail = false;
	m_bTempHeadFail = false;

	m_cbHeadIsReady = new VIMainCallback<WeldingHeadControl> (*this, &WeldingHeadControl::HeadIsReady);
	m_cbHeadValueReached = new VIMainCallback<WeldingHeadControl> (*this, &WeldingHeadControl::HeadValueReached);
	m_cbHeadError = new VIMainCallback<WeldingHeadControl> (*this, &WeldingHeadControl::HeadError);

	SAXParser parser;
	parser.setFeature(XMLReader::FEATURE_NAMESPACES, true);
	parser.setFeature(XMLReader::FEATURE_NAMESPACE_PREFIXES, true);
	parser.setContentHandler(&m_oConfigParser);
	parser.setProperty(XMLReader::PROPERTY_LEXICAL_HANDLER, static_cast<LexicalHandler*> (&m_oConfigParser));

	try
	{
		std::string ConfigFilename(getenv("WM_BASE_DIR"));
		ConfigFilename += "/config/VI_Config.xml";
		Poco::Path configPath( ConfigFilename );

		std::cout << "parsing configuration file: " << configPath.toString() << std::endl;
		wmLogTr(eInfo, "QnxMsg.VI.XMLFileRead", "parsing configuration file: %s\n", configPath.toString().c_str());
		parser.parse( configPath.toString() );
		std::cout << "configuration file successful parsed: " << configPath.toString() << std::endl;
		wmLogTr(eInfo, "QnxMsg.VI.XMLFileReadOK", "configuration file successful parsed: %s\n", configPath.toString().c_str());
	}
	catch (Poco::Exception& e)
	{
		// Fehler beim Parsen der VI_Config.xml
		std::cerr << "error while parsing configuration file: " << e.displayText() << std::endl;
		wmLogTr(eError, "QnxMsg.VI.XMLFileReadErr", "error while parsing configuration file: %s\n", e.displayText().c_str());

		// VI_Config_Standard.xml als Notbehelf in VI_Config.xml kopieren
		std::string oSrcConfigFilename(getenv("WM_BASE_DIR"));
		oSrcConfigFilename += "/config/VI_Config_Standard.xml";
		std::string oDestConfigFilename(getenv("WM_BASE_DIR"));
		oDestConfigFilename += "/config/VI_Config.xml";
		std::string oCommandString = "cp " + oSrcConfigFilename + " " + oDestConfigFilename;
		if (std::system(oCommandString.c_str()) == -1)
		{
			std::cerr << "error while copying Input/Output Configuration file" << std::endl;
			wmLogTr(eError, "QnxMsg.Misc.VIConfCopyErr", "error while copying Input/Output Configuration file\n");
		}

		// Meldung ausgeben, dass VI_Config.xml nicht korrekt ist und System in Nicht-Betriebsbereit-Zustand versetzen
		std::cerr << "Input/Output Configuration file is damaged or missing" << std::endl;
		wmFatal(eDataConsistency, "QnxMsg.Misc.VIConfIsNew1", "Input/Output Configuration file is damaged or missing\n");
		std::cerr << "Input/Output Configuration is lost and must be set again" << std::endl;
		wmFatal(eDataConsistency, "QnxMsg.Misc.VIConfIsNew2", "Input/Output Configuration is lost and must be set again\n");

		// Neue (nicht korrekte) VI_Config.xml parsen, damit Prozess in lauffaehigen Zustand uebergeht
		Poco::Path configPath( oDestConfigFilename );
		parser.parse( configPath.toString() );
	}
	catch (...)
	{
		// Fehler beim Einlesen der VI_Config.xml
		std::string oGenError = "general error";
		std::cerr << "error while parsing configuration file: " << oGenError << std::endl;
		wmLogTr(eError, "QnxMsg.VI.XMLFileReadErr", "error while parsing configuration file: %s\n", oGenError.c_str());

		// VI_Config_Standard.xml als Notbehelf in VI_Config.xml kopieren
		std::string oSrcConfigFilename(getenv("WM_BASE_DIR"));
		oSrcConfigFilename += "/config/VI_Config_Standard.xml";
		std::string oDestConfigFilename(getenv("WM_BASE_DIR"));
		oDestConfigFilename += "/config/VI_Config.xml";
		std::string oCommandString = "cp " + oSrcConfigFilename + " " + oDestConfigFilename;
		if (std::system(oCommandString.c_str()) == -1)
		{
			std::cerr << "error while copying Input/Output Configuration file" << std::endl;
			wmLogTr(eError, "QnxMsg.Misc.VIConfCopyErr", "error while copying Input/Output Configuration file\n");
		}

		// Meldung ausgeben, dass VI_Config.xml nicht korrekt ist und System in Nicht-Betriebsbereit-Zustand versetzen
		std::cerr << "Input/Output Configuration file is damaged or missing" << std::endl;
		wmFatal(eDataConsistency, "QnxMsg.Misc.VIConfIsNew1", "Input/Output Configuration file is damaged or missing\n");
		std::cerr << "Input/Output Configuration is lost and must be set again" << std::endl;
		wmFatal(eDataConsistency, "QnxMsg.Misc.VIConfIsNew2", "Input/Output Configuration is lost and must be set again\n");

		// Neue (nicht korrekte) VI_Config.xml parsen, damit Prozess in lauffaehigen Zustand uebergeht
		Poco::Path configPath( oDestConfigFilename );
		parser.parse( configPath.toString() );
	}

	// check if SystemConfig.xml is newly written
	bool tempBool = SystemConfiguration::instance().getBool("File_New_Created", false);
	if (tempBool)
	{
		wmFatal(eDataConsistency, "QnxMsg.Misc.SysConfIsNew1", "System Configuration file is damaged or missing\n");
		wmFatal(eDataConsistency, "QnxMsg.Misc.SysConfIsNew2", "System Configuration is lost and must be set again\n");
		SystemConfiguration::instance().setBool("File_New_Created", false);
	}

	// SystemConfig Switches for Linelaser/FieldLight
	m_oLineLaser1Enable = SystemConfiguration::instance().getBool("LineLaser1Enable", false);
	wmLog(eDebug, "m_oLineLaser1Enable (bool): %d\n", m_oLineLaser1Enable);
	m_oLineLaser1_OutputViaCamera = SystemConfiguration::instance().getBool("LineLaser1_OutputViaCamera", false);
	wmLog(eDebug, "m_oLineLaser1_OutputViaCamera (bool): %d\n", m_oLineLaser1_OutputViaCamera);
	m_oLineLaser2Enable = SystemConfiguration::instance().getBool("LineLaser2Enable", false);
	wmLog(eDebug, "m_oLineLaser2Enable (bool): %d\n", m_oLineLaser2Enable);
	m_oLineLaser2_OutputViaCamera = SystemConfiguration::instance().getBool("LineLaser2_OutputViaCamera", false);
	wmLog(eDebug, "m_oLineLaser2_OutputViaCamera (bool): %d\n", m_oLineLaser2_OutputViaCamera);
	m_oFieldLight1Enable = SystemConfiguration::instance().getBool("FieldLight1Enable", false);
	wmLog(eDebug, "m_oFieldLight1Enable (bool): %d\n", m_oFieldLight1Enable);
	m_oFieldLight1_OutputViaCamera = SystemConfiguration::instance().getBool("FieldLight1_OutputViaCamera", false);
	wmLog(eDebug, "m_oFieldLight1_OutputViaCamera (bool): %d\n", m_oFieldLight1_OutputViaCamera);

	// SystemConfig Switches for GenPurposeAnalogIn1 to GenPurposeAnalogIn8
	m_oGenPurposeAnaInEnable[eAnalogIn1] = SystemConfiguration::instance().getBool("GenPurposeAnalogIn1Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaInEnable[eAnalogIn1] (bool): %d\n", m_oGenPurposeAnaInEnable[eAnalogIn1]);
	m_oGenPurposeAnaInEnable[eAnalogIn2] = SystemConfiguration::instance().getBool("GenPurposeAnalogIn2Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaInEnable[eAnalogIn2] (bool): %d\n", m_oGenPurposeAnaInEnable[eAnalogIn2]);
	m_oGenPurposeAnaInEnable[eAnalogIn3] = SystemConfiguration::instance().getBool("GenPurposeAnalogIn3Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaInEnable[eAnalogIn3] (bool): %d\n", m_oGenPurposeAnaInEnable[eAnalogIn3]);
	m_oGenPurposeAnaInEnable[eAnalogIn4] = SystemConfiguration::instance().getBool("GenPurposeAnalogIn4Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaInEnable[eAnalogIn4] (bool): %d\n", m_oGenPurposeAnaInEnable[eAnalogIn4]);
	m_oGenPurposeAnaInEnable[eAnalogIn5] = SystemConfiguration::instance().getBool("GenPurposeAnalogIn5Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaInEnable[eAnalogIn5] (bool): %d\n", m_oGenPurposeAnaInEnable[eAnalogIn5]);
	m_oGenPurposeAnaInEnable[eAnalogIn6] = SystemConfiguration::instance().getBool("GenPurposeAnalogIn6Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaInEnable[eAnalogIn6] (bool): %d\n", m_oGenPurposeAnaInEnable[eAnalogIn6]);
	m_oGenPurposeAnaInEnable[eAnalogIn7] = SystemConfiguration::instance().getBool("GenPurposeAnalogIn7Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaInEnable[eAnalogIn7] (bool): %d\n", m_oGenPurposeAnaInEnable[eAnalogIn7]);
	m_oGenPurposeAnaInEnable[eAnalogIn8] = SystemConfiguration::instance().getBool("GenPurposeAnalogIn8Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaInEnable[eAnalogIn8] (bool): %d\n", m_oGenPurposeAnaInEnable[eAnalogIn8]);
	// SystemConfig Switches for GenPurposeAnalogOut1 to GenPurposeAnalogOut8
	m_oGenPurposeAnaOutEnable[eAnalogOut1] = SystemConfiguration::instance().getBool("GenPurposeAnalogOut1Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaOutEnable[eAnalogOut1] (bool): %d\n", m_oGenPurposeAnaOutEnable[eAnalogOut1]);
	m_oGenPurposeAnaOutEnable[eAnalogOut2] = SystemConfiguration::instance().getBool("GenPurposeAnalogOut2Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaOutEnable[eAnalogOut2] (bool): %d\n", m_oGenPurposeAnaOutEnable[eAnalogOut2]);
	m_oGenPurposeAnaOutEnable[eAnalogOut3] = SystemConfiguration::instance().getBool("GenPurposeAnalogOut3Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaOutEnable[eAnalogOut3] (bool): %d\n", m_oGenPurposeAnaOutEnable[eAnalogOut3]);
	m_oGenPurposeAnaOutEnable[eAnalogOut4] = SystemConfiguration::instance().getBool("GenPurposeAnalogOut4Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaOutEnable[eAnalogOut4] (bool): %d\n", m_oGenPurposeAnaOutEnable[eAnalogOut4]);
	m_oGenPurposeAnaOutEnable[eAnalogOut5] = SystemConfiguration::instance().getBool("GenPurposeAnalogOut5Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaOutEnable[eAnalogOut5] (bool): %d\n", m_oGenPurposeAnaOutEnable[eAnalogOut5]);
	m_oGenPurposeAnaOutEnable[eAnalogOut6] = SystemConfiguration::instance().getBool("GenPurposeAnalogOut6Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaOutEnable[eAnalogOut6] (bool): %d\n", m_oGenPurposeAnaOutEnable[eAnalogOut6]);
	m_oGenPurposeAnaOutEnable[eAnalogOut7] = SystemConfiguration::instance().getBool("GenPurposeAnalogOut7Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaOutEnable[eAnalogOut7] (bool): %d\n", m_oGenPurposeAnaOutEnable[eAnalogOut7]);
	m_oGenPurposeAnaOutEnable[eAnalogOut8] = SystemConfiguration::instance().getBool("GenPurposeAnalogOut8Enable", false);
	wmLog(eDebug, "m_oGenPurposeAnaOutEnable[eAnalogOut8] (bool): %d\n", m_oGenPurposeAnaOutEnable[eAnalogOut8]);

	// SystemConfig Switches for LED Illumination
	m_oLED_IlluminationEnable = SystemConfiguration::instance().getBool("LED_IlluminationEnable", false);
	wmLog(eDebug, "m_oLED_IlluminationEnable (bool): %d\n", m_oLED_IlluminationEnable);
	m_oLED_MaxCurrent_mA_Chan1 = SystemConfiguration::instance().getInt("LED_MaxCurrent_mA_Chan1", 1000);
	wmLog(eDebug, "m_oLED_MaxCurrent_mA_Chan1 (int) : %d\n", m_oLED_MaxCurrent_mA_Chan1);
	m_oLED_MaxCurrent_mA_Chan2 = SystemConfiguration::instance().getInt("LED_MaxCurrent_mA_Chan2", 1000);
	wmLog(eDebug, "m_oLED_MaxCurrent_mA_Chan2 (int) : %d\n", m_oLED_MaxCurrent_mA_Chan2);
	m_oLED_MaxCurrent_mA_Chan3 = SystemConfiguration::instance().getInt("LED_MaxCurrent_mA_Chan3", 1000);
	wmLog(eDebug, "m_oLED_MaxCurrent_mA_Chan3 (int) : %d\n", m_oLED_MaxCurrent_mA_Chan3);
	m_oLED_MaxCurrent_mA_Chan4 = SystemConfiguration::instance().getInt("LED_MaxCurrent_mA_Chan4", 1000);
	wmLog(eDebug, "m_oLED_MaxCurrent_mA_Chan4 (int) : %d\n", m_oLED_MaxCurrent_mA_Chan4);
	m_oLED_MaxCurrent_mA_Chan5 = SystemConfiguration::instance().getInt("LED_MaxCurrent_mA_Chan5", 1000);
	wmLog(eDebug, "m_oLED_MaxCurrent_mA_Chan5 (int) : %d\n", m_oLED_MaxCurrent_mA_Chan5);
	m_oLED_MaxCurrent_mA_Chan6 = SystemConfiguration::instance().getInt("LED_MaxCurrent_mA_Chan6", 1000);
	wmLog(eDebug, "m_oLED_MaxCurrent_mA_Chan6 (int) : %d\n", m_oLED_MaxCurrent_mA_Chan6);
	m_oLED_MaxCurrent_mA_Chan7 = SystemConfiguration::instance().getInt("LED_MaxCurrent_mA_Chan7", 1000);
	wmLog(eDebug, "m_oLED_MaxCurrent_mA_Chan7 (int) : %d\n", m_oLED_MaxCurrent_mA_Chan7);
	m_oLED_MaxCurrent_mA_Chan8 = SystemConfiguration::instance().getInt("LED_MaxCurrent_mA_Chan8", 1000);
	wmLog(eDebug, "m_oLED_MaxCurrent_mA_Chan8 (int) : %d\n", m_oLED_MaxCurrent_mA_Chan8);

	// SystemConfig Switches for ScanTracker
	m_oScanTrackerEnable = SystemConfiguration::instance().getBool("ScanTrackerEnable", false);
	wmLog(eDebug, "m_oScanTrackerEnable (bool): %d\n", m_oScanTrackerEnable);

	// SystemConfig Switches for LaserControl
	m_oLaserControlEnable = SystemConfiguration::instance().getBool("LaserControlEnable", false);
	wmLog(eDebug, "m_oLaserControlEnable (bool): %d\n", m_oLaserControlEnable);

	// SystemConfig Switches for ZCollimator
	m_oZCollimatorEnable = SystemConfiguration::instance().getBool("ZCollimatorEnable", false);
	wmLog(eDebug, "m_oZCollimatorEnable (bool): %d\n", m_oZCollimatorEnable);
	m_oZCollimatorType = static_cast<ZCollimatorType>(SystemConfiguration::instance().getInt("ZCollimatorType", 1));
	wmLog(eDebug, "m_oZCollimatorType (int): %d\n", m_oZCollimatorType);

    // SystemConfig Switches for LiquidLensController
    m_liquidLensControllerEnable = SystemConfiguration::instance().getBool("LiquidLensControllerEnable", false);
    wmLog(eDebug, "LiquidLensControllerEnable (bool): %d\n", m_liquidLensControllerEnable);

	// SystemConfig Switches for LaserPowerInput
	m_oLaserPowerInputEnable = SystemConfiguration::instance().getBool("LaserPowerInputEnable", false);
	wmLog(eDebug, "m_oLaserPowerInputEnable (bool): %d\n", m_oLaserPowerInputEnable);

	// SystemConfig Switches for EncoderInput
	m_oEncoderInput1Enable = SystemConfiguration::instance().getBool("EncoderInput1Enable", false);
	wmLog(eDebug, "m_oEncoderInput1Enable (bool): %d\n", m_oEncoderInput1Enable);
	m_oEncoderInput2Enable = SystemConfiguration::instance().getBool("EncoderInput2Enable", false);
	wmLog(eDebug, "m_oEncoderInput2Enable (bool): %d\n", m_oEncoderInput2Enable);

	// SystemConfig Switches for RobotTrackSpeed
	m_oRobotTrackSpeedEnable = SystemConfiguration::instance().getBool("RobotTrackSpeedEnable", false);
	wmLog(eDebug, "m_oRobotTrackSpeedEnable (bool): %d\n", m_oRobotTrackSpeedEnable);

	// SystemConfig Switches for WeldHead Axis
	m_oAxisXEnable = SystemConfiguration::instance().getBool("AxisXEnable", false);
	wmLog(eDebug, "m_oAxisXEnable (bool): %d\n", m_oAxisXEnable);
	m_oAxisYEnable = SystemConfiguration::instance().getBool("AxisYEnable", false);
	wmLog(eDebug, "m_oAxisYEnable (bool): %d\n", m_oAxisYEnable);
	m_oAxisZEnable = SystemConfiguration::instance().getBool("AxisZEnable", false);
	wmLog(eDebug, "m_oAxisZEnable (bool): %d\n", m_oAxisZEnable);

	// SystemConfig Switches for FastAnalogSignals
	m_oFastAnalogSignal1Enable = SystemConfiguration::instance().getBool("FastAnalogSignal1Enable", false);
	wmLog(eDebug, "m_oFastAnalogSignal1Enable (bool): %d\n", m_oFastAnalogSignal1Enable);
	m_oFastAnalogSignal2Enable = SystemConfiguration::instance().getBool("FastAnalogSignal2Enable", false);
	wmLog(eDebug, "m_oFastAnalogSignal2Enable (bool): %d\n", m_oFastAnalogSignal2Enable);
	m_oFastAnalogSignal3Enable = SystemConfiguration::instance().getBool("FastAnalogSignal3Enable", false);
	wmLog(eDebug, "m_oFastAnalogSignal3Enable (bool): %d\n", m_oFastAnalogSignal3Enable);
	m_oFastAnalogSignal4Enable = SystemConfiguration::instance().getBool("FastAnalogSignal4Enable", false);
	wmLog(eDebug, "m_oFastAnalogSignal4Enable (bool): %d\n", m_oFastAnalogSignal4Enable);
	m_oFastAnalogSignal5Enable = SystemConfiguration::instance().getBool("FastAnalogSignal5Enable", false);
	wmLog(eDebug, "m_oFastAnalogSignal5Enable (bool): %d\n", m_oFastAnalogSignal5Enable);
	m_oFastAnalogSignal6Enable = SystemConfiguration::instance().getBool("FastAnalogSignal6Enable", false);
	wmLog(eDebug, "m_oFastAnalogSignal6Enable (bool): %d\n", m_oFastAnalogSignal6Enable);
	m_oFastAnalogSignal7Enable = SystemConfiguration::instance().getBool("FastAnalogSignal7Enable", false);
	wmLog(eDebug, "m_oFastAnalogSignal7Enable (bool): %d\n", m_oFastAnalogSignal7Enable);
	m_oFastAnalogSignal8Enable = SystemConfiguration::instance().getBool("FastAnalogSignal8Enable", false);
	wmLog(eDebug, "m_oFastAnalogSignal8Enable (bool): %d\n", m_oFastAnalogSignal8Enable);

	// SystemConfig Switches for LWM40
	m_oLWM40_No1_Enable = SystemConfiguration::instance().getBool("LWM40_No1_Enable", false);
	wmLog(eDebug, "m_oLWM40_No1_Enable (bool): %d\n", m_oLWM40_No1_Enable);

    // SystemConfig Switches for Scanlab Scanner
    m_oScanlabScannerEnable = SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::Scanner2DEnable);
    wmLog(eDebug, "m_oScanlabScannerEnable (bool): %d\n", m_oScanlabScannerEnable);
    m_scannerGeneralMode = static_cast<ScannerGeneralMode>(SystemConfiguration::instance().getInt("ScannerGeneralMode", 1));
    wmLog(eDebug, "m_scannerGeneralMode (int): %d\n", static_cast<int>(m_scannerGeneralMode));
    m_scannerModel = static_cast<ScannerModel>(SystemConfiguration::instance().getInt("ScannerModel", 0));
    wmLog(eDebug, "Scanner model is: %d\n", static_cast<int>(m_scannerModel));

    // SystemConfig Switch for SOUVIS6000 application
    m_oIsSOUVIS6000_Application = SystemConfiguration::instance().getBool("SOUVIS6000_Application", false);
    wmLog(eDebug, "m_oIsSOUVIS6000_Application (bool): %d\n", m_oIsSOUVIS6000_Application);

	//############################################################################

	WeldHeadDefaults::instance().toStdOut();

	//############################################################################

	// init the LEDI_Parameters
	for(int i = 0;i < ANZ_LEDI_PARAMETERS;i++)
	{
		m_oLEDI_Parameters[i].led_enable = 0;
		m_oLEDI_Parameters[i].led_brightness = 0;
		m_oLEDI_Parameters[i].led_pulse_width = 0;
	}
	m_oLEDControllerType = eLEDTypeNone;
	SystemConfiguration::instance().setInt("LED_CONTROLLER_TYPE", static_cast<int>(m_oLEDControllerType));
	m_oLEDchannelMaxCurrent[0] = m_oLED_MaxCurrent_mA_Chan1;
	m_oLEDchannelMaxCurrent[1] = m_oLED_MaxCurrent_mA_Chan2;
	m_oLEDchannelMaxCurrent[2] = m_oLED_MaxCurrent_mA_Chan3;
	m_oLEDchannelMaxCurrent[3] = m_oLED_MaxCurrent_mA_Chan4;
	m_oLEDchannelMaxCurrent[4] = m_oLED_MaxCurrent_mA_Chan5;
	m_oLEDchannelMaxCurrent[5] = m_oLED_MaxCurrent_mA_Chan6;
	m_oLEDchannelMaxCurrent[6] = m_oLED_MaxCurrent_mA_Chan7;
	m_oLEDchannelMaxCurrent[7] = m_oLED_MaxCurrent_mA_Chan8;
	m_oLED_ParameterChanged = false;

	// Einlesen der gespeicherten LED-Panel-Intensitaet
	m_oLEDI_Parameters[LED_PANEL_1].led_brightness = WeldHeadDefaults::instance().getInt("LEDPanel1Intensity", 20);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_1].led_brightness (int): %d\n", m_oLEDI_Parameters[LED_PANEL_1].led_brightness);
	m_oLEDI_Parameters[LED_PANEL_2].led_brightness = WeldHeadDefaults::instance().getInt("LEDPanel2Intensity", 20);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_2].led_brightness (int): %d\n", m_oLEDI_Parameters[LED_PANEL_2].led_brightness);
	m_oLEDI_Parameters[LED_PANEL_3].led_brightness = WeldHeadDefaults::instance().getInt("LEDPanel3Intensity", 20);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_3].led_brightness (int): %d\n", m_oLEDI_Parameters[LED_PANEL_3].led_brightness);
	m_oLEDI_Parameters[LED_PANEL_4].led_brightness = WeldHeadDefaults::instance().getInt("LEDPanel4Intensity", 20);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_4].led_brightness (int): %d\n", m_oLEDI_Parameters[LED_PANEL_4].led_brightness);
	m_oLEDI_Parameters[LED_PANEL_5].led_brightness = WeldHeadDefaults::instance().getInt("LEDPanel5Intensity", 20);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_5].led_brightness (int): %d\n", m_oLEDI_Parameters[LED_PANEL_5].led_brightness);
	m_oLEDI_Parameters[LED_PANEL_6].led_brightness = WeldHeadDefaults::instance().getInt("LEDPanel6Intensity", 20);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_6].led_brightness (int): %d\n", m_oLEDI_Parameters[LED_PANEL_6].led_brightness);
	m_oLEDI_Parameters[LED_PANEL_7].led_brightness = WeldHeadDefaults::instance().getInt("LEDPanel7Intensity", 20);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_7].led_brightness (int): %d\n", m_oLEDI_Parameters[LED_PANEL_7].led_brightness);
	m_oLEDI_Parameters[LED_PANEL_8].led_brightness = WeldHeadDefaults::instance().getInt("LEDPanel8Intensity", 20);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_8].led_brightness (int): %d\n", m_oLEDI_Parameters[LED_PANEL_8].led_brightness);

	// Einlesen des gespeicherten Ein-Aus-Zustands der LED-Panel
	m_oLEDI_Parameters[LED_PANEL_1].led_enable = (int)WeldHeadDefaults::instance().getBool("LEDPanel1OnOff", false);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_1].led_enable (int): %d\n", m_oLEDI_Parameters[LED_PANEL_1].led_enable);
	m_oLEDI_Parameters[LED_PANEL_2].led_enable = (int)WeldHeadDefaults::instance().getBool("LEDPanel2OnOff", false);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_2].led_enable (int): %d\n", m_oLEDI_Parameters[LED_PANEL_2].led_enable);
	m_oLEDI_Parameters[LED_PANEL_3].led_enable = (int)WeldHeadDefaults::instance().getBool("LEDPanel3OnOff", false);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_3].led_enable (int): %d\n", m_oLEDI_Parameters[LED_PANEL_3].led_enable);
	m_oLEDI_Parameters[LED_PANEL_4].led_enable = (int)WeldHeadDefaults::instance().getBool("LEDPanel4OnOff", false);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_4].led_enable (int): %d\n", m_oLEDI_Parameters[LED_PANEL_4].led_enable);
	m_oLEDI_Parameters[LED_PANEL_5].led_enable = (int)WeldHeadDefaults::instance().getBool("LEDPanel5OnOff", false);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_5].led_enable (int): %d\n", m_oLEDI_Parameters[LED_PANEL_5].led_enable);
	m_oLEDI_Parameters[LED_PANEL_6].led_enable = (int)WeldHeadDefaults::instance().getBool("LEDPanel6OnOff", false);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_6].led_enable (int): %d\n", m_oLEDI_Parameters[LED_PANEL_6].led_enable);
	m_oLEDI_Parameters[LED_PANEL_7].led_enable = (int)WeldHeadDefaults::instance().getBool("LEDPanel7OnOff", false);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_7].led_enable (int): %d\n", m_oLEDI_Parameters[LED_PANEL_7].led_enable);
	m_oLEDI_Parameters[LED_PANEL_8].led_enable = (int)WeldHeadDefaults::instance().getBool("LEDPanel8OnOff", false);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_8].led_enable (int): %d\n", m_oLEDI_Parameters[LED_PANEL_8].led_enable);

	// Einlesen der gespeicherten LED-Panel-Pulsdauer
	m_oLEDI_Parameters[LED_PANEL_1].led_pulse_width = WeldHeadDefaults::instance().getInt("LEDPanel1PulseWidth", 80);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_1].led_pulse_width (int): %d\n", m_oLEDI_Parameters[LED_PANEL_1].led_pulse_width);
	m_oLEDI_Parameters[LED_PANEL_2].led_pulse_width = WeldHeadDefaults::instance().getInt("LEDPanel2PulseWidth", 80);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_2].led_pulse_width (int): %d\n", m_oLEDI_Parameters[LED_PANEL_2].led_pulse_width);
	m_oLEDI_Parameters[LED_PANEL_3].led_pulse_width = WeldHeadDefaults::instance().getInt("LEDPanel3PulseWidth", 80);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_3].led_pulse_width (int): %d\n", m_oLEDI_Parameters[LED_PANEL_3].led_pulse_width);
	m_oLEDI_Parameters[LED_PANEL_4].led_pulse_width = WeldHeadDefaults::instance().getInt("LEDPanel4PulseWidth", 80);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_4].led_pulse_width (int): %d\n", m_oLEDI_Parameters[LED_PANEL_4].led_pulse_width);
	m_oLEDI_Parameters[LED_PANEL_5].led_pulse_width = WeldHeadDefaults::instance().getInt("LEDPanel5PulseWidth", 80);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_5].led_pulse_width (int): %d\n", m_oLEDI_Parameters[LED_PANEL_5].led_pulse_width);
	m_oLEDI_Parameters[LED_PANEL_6].led_pulse_width = WeldHeadDefaults::instance().getInt("LEDPanel6PulseWidth", 80);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_6].led_pulse_width (int): %d\n", m_oLEDI_Parameters[LED_PANEL_6].led_pulse_width);
	m_oLEDI_Parameters[LED_PANEL_7].led_pulse_width = WeldHeadDefaults::instance().getInt("LEDPanel7PulseWidth", 80);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_7].led_pulse_width (int): %d\n", m_oLEDI_Parameters[LED_PANEL_7].led_pulse_width);
	m_oLEDI_Parameters[LED_PANEL_8].led_pulse_width = WeldHeadDefaults::instance().getInt("LEDPanel8PulseWidth", 80);
	wmLog(eDebug, "m_oLEDI_Parameters[LED_PANEL_8].led_pulse_width (int): %d\n", m_oLEDI_Parameters[LED_PANEL_8].led_pulse_width);

	// INIT fuer LED-Schnittstelle
	// -----------------------------------------------

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE
	if ( isLED_IlluminationEnabled() )
	{
		m_oLEDdriverParameter.useDriverInternalTrigger=false;
		m_oLEDdriver.setdebuglevelsource(NULL);

		m_oLED_DriverIsOperational = true;
        m_oLEDdriver.init(&m_oLEDdriverHW_PP420, &m_oLEDcomEthernet1, m_oLEDdriverParameter, m_oLEDchannelMaxCurrent);
        if (m_oLEDdriver.m_oControllerTypeIsWrong)
        {
            printf("WeldingHeadControl: Controller Type is wrong ! (001)\n");
            m_oLEDdriver.init(&m_oLEDdriverHW_PP520, &m_oLEDcomEthernet1, m_oLEDdriverParameter, m_oLEDchannelMaxCurrent);
            if (m_oLEDdriver.m_oControllerTypeIsWrong)
            {
                printf("WeldingHeadControl: Controller Type is wrong ! (002)\n");
                m_oLEDdriver.init(&m_oLEDdriverHW_PP820, &m_oLEDcomEthernet1, m_oLEDdriverParameter, m_oLEDchannelMaxCurrent);
                if (m_oLEDdriver.m_oControllerTypeIsWrong)
                {
                    printf("WeldingHeadControl: Controller Type is wrong ! (003)\n");
                }
                else
                {
                    m_oLEDControllerType = eLEDTypePP820;
                }
            }
            else
            {
                m_oLEDControllerType = eLEDTypePP520;
            }
        }
        else
        {
            m_oLEDControllerType = eLEDTypePP420F;
        }
		SystemConfiguration::instance().setInt("LED_CONTROLLER_TYPE", static_cast<int>(m_oLEDControllerType));
		if (m_oLEDdriver.state != 1)
		{
			m_oLED_DriverIsOperational = false;
		}

		if (m_oLED_DriverIsOperational)
		{
			//m_oLEDdriver.updateMaxCurrent(m_oLEDchannelMaxCurrent);

//			int oIntensityArray[ANZ_LEDI_PARAMETERS];
//			m_oLEDdriver.ExtractIntensityValues(oIntensityArray, m_oLEDdriverHW_PP420.nChannels);
//			for(int i = 0;i < m_oLEDdriverHW_PP420.nChannels;i++)
//			{
//				if (oIntensityArray[i] > 0)
//				{
//					m_oLEDI_Parameters[i].led_enable = 1;
//					m_oLEDI_Parameters[i].led_brightness = oIntensityArray[i];
//					// Schatten-Struktur im Treiber ebenfalls initialisieren
//					// Direkt-Zugriff ist nicht schoen, Struktur war aber schon public
//					m_oLEDdriver.currentparams[i].led_enable = 1;
//					m_oLEDdriver.currentparams[i].led_brightness = oIntensityArray[i];
//				}
//				else
//				{
//					m_oLEDI_Parameters[i].led_enable = 0;
//					m_oLEDI_Parameters[i].led_brightness = 0;
//					// Schatten-Struktur im Treiber ebenfalls initialisieren
//					// Direkt-Zugriff ist nicht schoen, Struktur war aber schon public
//					m_oLEDdriver.currentparams[i].led_enable = 0;
//					m_oLEDdriver.currentparams[i].led_brightness = 0;
//				}
//			}
            if (m_oLEDControllerType == eLEDTypePP820)
            {
                for(int i = 0;i < m_oLEDdriverHW_PP820.nChannels;i++)
                {
                    // Schatten-Struktur im Treiber ebenfalls initialisieren
                    // Direkt-Zugriff ist nicht schoen, Struktur war aber schon public
                    m_oLEDdriver.currentparams[i].led_enable = m_oLEDI_Parameters[i].led_enable;
                    m_oLEDdriver.currentparams[i].led_brightness = 110; // unmoeglicher Wert, erzwingt neu Beschreiben des Controllers
                    m_oLEDdriver.currentparams[i].led_pulse_width = 500; // unmoeglicher Wert, erzwingt neu Beschreiben des Controllers
                }
            }
            else if (m_oLEDControllerType == eLEDTypePP520)
            {
                for(int i = 0;i < m_oLEDdriverHW_PP520.nChannels;i++)
                {
                    // Schatten-Struktur im Treiber ebenfalls initialisieren
                    // Direkt-Zugriff ist nicht schoen, Struktur war aber schon public
                    m_oLEDdriver.currentparams[i].led_enable = m_oLEDI_Parameters[i].led_enable;
                    m_oLEDdriver.currentparams[i].led_brightness = 110; // unmoeglicher Wert, erzwingt neu Beschreiben des Controllers
                    m_oLEDdriver.currentparams[i].led_pulse_width = 500; // unmoeglicher Wert, erzwingt neu Beschreiben des Controllers
                }
            }
            else
            {
                for(int i = 0;i < m_oLEDdriverHW_PP420.nChannels;i++)
                {
                    // Schatten-Struktur im Treiber ebenfalls initialisieren
                    // Direkt-Zugriff ist nicht schoen, Struktur war aber schon public
                    m_oLEDdriver.currentparams[i].led_enable = m_oLEDI_Parameters[i].led_enable;
                    m_oLEDdriver.currentparams[i].led_brightness = 110; // unmoeglicher Wert, erzwingt neu Beschreiben des Controllers
                    m_oLEDdriver.currentparams[i].led_pulse_width = 500; // unmoeglicher Wert, erzwingt neu Beschreiben des Controllers
                }
            }
			LEDI_ParametersT *inpars;
			inpars = const_cast<LEDI_ParametersT *>(m_oLEDI_Parameters);
			m_oLEDdriver.updateall(inpars);
			m_oLED_ParameterChanged = true;
			usleep(100*1000);
			m_oLEDdriver.ReadConfiguration();
		}
	}
#endif

    if (isLiquidLensControllerEnabled())
    {
        std::string liquidLensControllerIPAddress = SystemConfiguration::instance().getString("LiquidLensController_IP_Address", "192.168.170.110");
        wmLog(eDebug, "LiquidLensController_IP_Address: <%s>\n", liquidLensControllerIPAddress.c_str());

        m_lensComEthernet.setLedAddress(liquidLensControllerIPAddress, 30313);
        m_lensComEthernet.setLocalAddress("192.168.170.1", 30312);

        m_lensDriver.setdebuglevelsource(nullptr);
        m_lensDriverIsOperational = true;
        m_lensDriver.init(&m_lensDriverTRCL180, &m_lensComEthernet, m_oLEDdriverParameter, m_oLEDchannelMaxCurrent);
        if (m_lensDriver.m_oControllerTypeIsWrong)
        {
            printf("WeldingHeadControl: Lens Controller Type is wrong ! (001)\n");
        }

        if (m_lensDriver.state != 1)
        {
            m_lensDriverIsOperational = false;
            wmFatal(eExtEquipment, "QnxMsg.VI.NoLiquidLens", "There is no liquid lens controller present\n");
        }

        if (m_lensDriverIsOperational)
        {
            LEDI_ParametersT *inpars;
            inpars = const_cast<LEDI_ParametersT *>(m_oLEDI_Parameters);
            m_lensDriver.updateall(inpars);
            usleep(100*1000);
            m_lensDriver.ReadConfiguration();
        }
    }

	//############################################################################

	if(isScanTrackerEnabled())
	{
		// ScanTracker Calibration File einlesen
		std::string CalibFilename(getenv("WM_BASE_DIR"));
		CalibFilename += "/config/scantrackerStandard.wob";

		std::cout << "parsing configuration file: " << CalibFilename << std::endl;
		wmLogTr(eInfo, "QnxMsg.VI.XMLFileRead", "parsing configuration file: %s\n", CalibFilename.c_str());

		if (parseTrackerCalibrationFile(CalibFilename) == 0)
		{
			std::cout << "configuration file successful parsed: " << CalibFilename << std::endl;
			wmLogTr(eInfo, "QnxMsg.VI.XMLFileReadOK", "configuration file successful parsed: %s\n", CalibFilename.c_str());
		}
		else
		{
			std::cerr << "error while parsing configuration file: " << CalibFilename << std::endl;
			wmLogTr(eError, "QnxMsg.VI.XMLFileReadErr", "error while parsing configuration file: %s\n", CalibFilename.c_str());
		}

		bool oFound = false;
        // gespeicherte Brennweite aus WeldHeadDefaults.xml abholen
	    m_oFocalLength = WeldHeadDefaults::instance().getInt("ScanTrackerFocalLength", 250, &oFound);
	    wmLog(eDebug, "m_oFocalLength (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	    	// gespeicherte Brennweite aus VI_Config.xml abholen
	        m_oFocalLength = m_oConfigParser.getFocalLength();
	        wmLog(eDebug, "m_oFocalLength (VI_Config)(int): %d\n", m_oFocalLength);
	        // Brennweite von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setInt("ScanTrackerFocalLength", m_oFocalLength);
	    }
	    else
        {
	        wmLog(eDebug, "m_oFocalLength (WeldHeadDefaults)(int): %d\n", m_oFocalLength);
        }
		CalculateFocalLengthDependencies(); // anhand der aktuellen Brennweite die neuen Grenzen berechnen

	    // gespeicherte max. zulaessige Amplitude aus WeldHeadDefaults.xml abholen
	    m_oTrackerMaxAmplitude = WeldHeadDefaults::instance().getInt("ScanTrackerMaxAmplitude", 5000, &oFound);
	    wmLog(eDebug, "m_oTrackerMaxAmplitude (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte max. zulaessige Amplitude aus VI_Config.xml abholen
	        m_oTrackerMaxAmplitude = m_oConfigParser.getMaxAmplitude();
	        wmLog(eDebug, "m_oTrackerMaxAmplitude (VI_Config)(int): %d\n", m_oTrackerMaxAmplitude);
	        // zulaessige Amplitude von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setInt("ScanTrackerMaxAmplitude", m_oTrackerMaxAmplitude);
	    }
	    else
        {
	        wmLog(eDebug, "m_oTrackerMaxAmplitude (WeldHeadDefaults)(int): %d\n", m_oTrackerMaxAmplitude);
        }

		// Serielle Kommunikation aufbauen
		wmLogTr( eInfo, "QnxMsg.VI.TrackerSerComType", "ScanTracker: Serial communication type: %d\n", m_oConfigParser.getSerialCommType());
		wmLogTr( eInfo, "QnxMsg.VI.TrackerSerComPort", "ScanTracker: Serial communication port: %d\n", m_oConfigParser.getSerialCommPort());
		m_pSerialToTracker = new SerialToTracker(m_oConfigParser.getSerialCommType(), m_oConfigParser.getSerialCommPort());

		if (m_pSerialToTracker != NULL)
		{
			m_pSerialToTracker->test();
			usleep(20 * 1000);
			m_pSerialToTracker->run();
			usleep(20 * 1000);

			// ScanTracker in definierten Zustand bringen
			m_oTrackerDriverOnOff = false;
			SetTrackerDriverOnOff(m_oTrackerDriverOnOff); // nach Init Off
			SetTrackerFrequencyStep(m_oTrackerFrequencyStep); // nach Init eFreq250
			SetTrackerDriverOnOff(m_oTrackerDriverOnOff); // wegen Bug beim Frequenz Setzen !?

			SetScanWidthOutOfGapWidth(false); // m_oScanWidthOutOfGapWidth auf false setzen, bereits nach Init false
			SetTrackerScanWidthFixed(m_oScanWidthFixed); // nach Init 0

			SetScanPosOutOfGapPos(true); // m_oScanPosOutOfGapPos auf true setzen, bereits nach Init true
			SetTrackerScanPosFixed(m_oScanPosFixed); // nach Init 0
		}

	}

    if (isZCollimatorEnabled() )
    {
        // get saved Z-Collimator SystemOffset out of WeldHeadDefaults.xml
        bool oFound = false;
        m_oZCSystemOffset = WeldHeadDefaults::instance().getDouble("ZCollimator_SystemOffset", 0.0, &oFound);
        wmLog(eDebug, "m_oZCSystemOffset (WeldHeadDefaults) found: %d\n", oFound);
        wmLog(eDebug, "m_oZCSystemOffset (WeldHeadDefaults) (double): %f\n", m_oZCSystemOffset.load());

        // get saved Z-Collimator LensToFocusRatio out of WeldHeadDefaults.xml
        oFound = false;
        m_oZCLensToFocusRatio = WeldHeadDefaults::instance().getDouble("ZCollimator_LensToFocusRatio", 3.73, &oFound);
        wmLog(eDebug, "m_oZCLensToFocusRatio (WeldHeadDefaults) found: %d\n", oFound);
        wmLog(eDebug, "m_oZCLensToFocusRatio (WeldHeadDefaults) (double): %f\n", m_oZCLensToFocusRatio.load());
    }

	if (isLaserControlEnabled() )
	{
		m_pDataToLaserControl = new DataToLaserControl();
		if (m_pDataToLaserControl != NULL)
		{
			wmLog(eDebug, "DataToLaserControl is started\n");
		}
	}

    m_pScanlab = new Scanlab();
    if (m_pScanlab != nullptr)
    {
        ScannerControlStatus status = m_pScanlab->InitScanlabObjects();
        if (status == ScannerControlStatus::Initialized)
        {
            wmLogTr(eInfo, "QnxMsg.VI.ScanlabStartOK", "Startup of Scanlab scanner was successful\n");
            m_pScanlab->SetScannerJumpSpeed(1.0);
            m_pScanlab->SetScannerMarkSpeed(1.0);
        }
        else if (status == ScannerControlStatus::Disabled)
        {
            // Message leads to confusion, commented out 17.9.2020 EA
            //wmLogTr(eWarning, "QnxMsg.VI.ScanlabNotInit", "Scanlab scanner is not configured\n");
        }
        else
        {
            wmLogTr(eError, "QnxMsg.VI.ScanlabStartNOK", "Startup of Scanlab scanner was not successful\n");
        }
        bool foundKeyValue = false;
        const auto& laserPowerDelayCompensation = WeldHeadDefaults::instance().getInt("Laser_Power_Compensation_10us", 0, &foundKeyValue);
        m_pScanlab->setLaserPowerDelayCompensation(laserPowerDelayCompensation);
        wmLog(eDebug, "Laser power delay compensation (WeldHeadDefaults) found: %d\n", foundKeyValue);
        wmLog(eDebug, "Laser power delay compensation in 10us (int): %d\n", laserPowerDelayCompensation);
    }

	//############################################################################

	int cnt1Commands = m_oConfigParser.m_inCommandsList.size();

	for (int i = 0; i < cnt1Commands; ++i) {
		COMMAND_INFORMATION commandInfo = m_oConfigParser.m_inCommandsList.front();

		// analog outputs
		if(isLineLaser1Enabled() && (commandInfo.commandType == ALineLaser1Intens)){
            m_oLineLaser1IntensityProxyInfo = commandInfo.proxyInfo;
			activateAnalogOutput(m_oLineLaser1IntensityProxyInfo, "SetLineLaser1Intensity");
		}
		else if(isFieldLight1Enabled() && (commandInfo.commandType == AFieldLight1Intens)){
            m_oFieldLight1IntensityProxyInfo = commandInfo.proxyInfo;
			activateAnalogOutput(m_oFieldLight1IntensityProxyInfo, "SetFieldLight1Intensity");
		}
		else if(isLineLaser2Enabled() && (commandInfo.commandType == ALineLaser2Intens)){
            m_oLineLaser2IntensityProxyInfo = commandInfo.proxyInfo;
			activateAnalogOutput(m_oLineLaser2IntensityProxyInfo, "SetLineLaser2Intensity");
		}
		else if(isScanTrackerEnabled() && (commandInfo.commandType == ATrackerScanWidth)){
            m_oTrackerScanWidthProxyInfo = commandInfo.proxyInfo;
			activateAnalogOutput(m_oTrackerScanWidthProxyInfo, "SetScanWidth");
		}
		else if(isScanTrackerEnabled() && (commandInfo.commandType == ATrackerScanPos)){
            m_oTrackerScanPosProxyInfo = commandInfo.proxyInfo;
			activateAnalogOutput(m_oTrackerScanPosProxyInfo, "SetScanPos");
		}
		else if(isZCollimatorEnabled() && (getZCollimatorType() == eZColliAnalog) && (commandInfo.commandType == AZCAnalogIn)){
            m_oZCAnalogInProxyInfo = commandInfo.proxyInfo;
			activateAnalogOutput(m_oZCAnalogInProxyInfo, "SetZCAnalogIn");
		}
		else if(isLaserControlEnabled() && (commandInfo.commandType == ALCPowerOffset)){
            m_oLCPowerOffsetProxyInfo = commandInfo.proxyInfo;
			activateAnalogOutput(m_oLCPowerOffsetProxyInfo, "LC PowerOffset");
		}
		else if(isGenPurposeAnaOutEnabled(eAnalogOut1) && (commandInfo.commandType == AGenPurposeAnaOut1)){
            m_oGenPurposeAnaOutProxyInfo[eAnalogOut1] = commandInfo.proxyInfo;
			activateAnalogOutput(m_oGenPurposeAnaOutProxyInfo[eAnalogOut1], "GenPurposeAnalogOut1");
		}
		else if(isGenPurposeAnaOutEnabled(eAnalogOut2) && (commandInfo.commandType == AGenPurposeAnaOut2)){
            m_oGenPurposeAnaOutProxyInfo[eAnalogOut2] = commandInfo.proxyInfo;
			activateAnalogOutput(m_oGenPurposeAnaOutProxyInfo[eAnalogOut2], "GenPurposeAnalogOut2");
		}
		else if(isGenPurposeAnaOutEnabled(eAnalogOut3) && (commandInfo.commandType == AGenPurposeAnaOut3)){
            m_oGenPurposeAnaOutProxyInfo[eAnalogOut3] = commandInfo.proxyInfo;
			activateAnalogOutput(m_oGenPurposeAnaOutProxyInfo[eAnalogOut3], "GenPurposeAnalogOut3");
		}
		else if(isGenPurposeAnaOutEnabled(eAnalogOut4) && (commandInfo.commandType == AGenPurposeAnaOut4)){
            m_oGenPurposeAnaOutProxyInfo[eAnalogOut4] = commandInfo.proxyInfo;
			activateAnalogOutput(m_oGenPurposeAnaOutProxyInfo[eAnalogOut4], "GenPurposeAnalogOut4");
		}
		else if(isGenPurposeAnaOutEnabled(eAnalogOut5) && (commandInfo.commandType == AGenPurposeAnaOut5)){
            m_oGenPurposeAnaOutProxyInfo[eAnalogOut5] = commandInfo.proxyInfo;
			activateAnalogOutput(m_oGenPurposeAnaOutProxyInfo[eAnalogOut5], "GenPurposeAnalogOut5");
		}
		else if(isGenPurposeAnaOutEnabled(eAnalogOut6) && (commandInfo.commandType == AGenPurposeAnaOut6)){
            m_oGenPurposeAnaOutProxyInfo[eAnalogOut6] = commandInfo.proxyInfo;
			activateAnalogOutput(m_oGenPurposeAnaOutProxyInfo[eAnalogOut6], "GenPurposeAnalogOut6");
		}
		else if(isGenPurposeAnaOutEnabled(eAnalogOut7) && (commandInfo.commandType == AGenPurposeAnaOut7)){
            m_oGenPurposeAnaOutProxyInfo[eAnalogOut7] = commandInfo.proxyInfo;
			activateAnalogOutput(m_oGenPurposeAnaOutProxyInfo[eAnalogOut7], "GenPurposeAnalogOut7");
		}
		else if(isGenPurposeAnaOutEnabled(eAnalogOut8) && (commandInfo.commandType == AGenPurposeAnaOut8)){
            m_oGenPurposeAnaOutProxyInfo[eAnalogOut8] = commandInfo.proxyInfo;
			activateAnalogOutput(m_oGenPurposeAnaOutProxyInfo[eAnalogOut8], "GenPurposeAnalogOut8");
		}
		// analog inputs
		else if(isGenPurposeAnaInEnabled(eAnalogIn1) && (commandInfo.commandType == EGenPurposeAnaIn1)){
            m_oGenPurposeAnaInProxyInfo[eAnalogIn1] = commandInfo.proxyInfo;
			activateAnalogInput(m_oGenPurposeAnaInProxyInfo[eAnalogIn1], "GenPurposeAnaIn1");
		}
		else if(isGenPurposeAnaInEnabled(eAnalogIn2) && (commandInfo.commandType == EGenPurposeAnaIn2)){
            m_oGenPurposeAnaInProxyInfo[eAnalogIn2] = commandInfo.proxyInfo;
			activateAnalogInput(m_oGenPurposeAnaInProxyInfo[eAnalogIn2], "GenPurposeAnaIn2");
		}
		else if(isGenPurposeAnaInEnabled(eAnalogIn3) && (commandInfo.commandType == EGenPurposeAnaIn3)){
            m_oGenPurposeAnaInProxyInfo[eAnalogIn3] = commandInfo.proxyInfo;
			activateAnalogInput(m_oGenPurposeAnaInProxyInfo[eAnalogIn3], "GenPurposeAnaIn3");
		}
		else if(isGenPurposeAnaInEnabled(eAnalogIn4) && (commandInfo.commandType == EGenPurposeAnaIn4)){
            m_oGenPurposeAnaInProxyInfo[eAnalogIn4] = commandInfo.proxyInfo;
			activateAnalogInput(m_oGenPurposeAnaInProxyInfo[eAnalogIn4], "GenPurposeAnaIn4");
		}
		else if(isGenPurposeAnaInEnabled(eAnalogIn5) && (commandInfo.commandType == EGenPurposeAnaIn5)){
            m_oGenPurposeAnaInProxyInfo[eAnalogIn5] = commandInfo.proxyInfo;
			activateAnalogInput(m_oGenPurposeAnaInProxyInfo[eAnalogIn5], "GenPurposeAnaIn5");
		}
		else if(isGenPurposeAnaInEnabled(eAnalogIn6) && (commandInfo.commandType == EGenPurposeAnaIn6)){
            m_oGenPurposeAnaInProxyInfo[eAnalogIn6] = commandInfo.proxyInfo;
			activateAnalogInput(m_oGenPurposeAnaInProxyInfo[eAnalogIn6], "GenPurposeAnaIn6");
		}
		else if(isGenPurposeAnaInEnabled(eAnalogIn7) && (commandInfo.commandType == EGenPurposeAnaIn7)){
            m_oGenPurposeAnaInProxyInfo[eAnalogIn7] = commandInfo.proxyInfo;
			activateAnalogInput(m_oGenPurposeAnaInProxyInfo[eAnalogIn7], "GenPurposeAnaIn7");
		}
		else if(isGenPurposeAnaInEnabled(eAnalogIn8) && (commandInfo.commandType == EGenPurposeAnaIn8)){
            m_oGenPurposeAnaInProxyInfo[eAnalogIn8] = commandInfo.proxyInfo;
			activateAnalogInput(m_oGenPurposeAnaInProxyInfo[eAnalogIn8], "GenPurposeAnaIn8");
		}
		else if(isLaserPowerInputEnabled() && (commandInfo.commandType == ELaserPowerSignal)){
            m_oLaserPowerSignalProxyInfo = commandInfo.proxyInfo;
			activateAnalogInput(m_oLaserPowerSignalProxyInfo, "IncomingLaserPowerSignal");
		}
		else if(isRobotTrackSpeedEnabled() && (commandInfo.commandType == ERobotTrackSpeed)){
            m_oRobotTrackSpeedProxyInfo = commandInfo.proxyInfo;
			activateAnalogInput(m_oRobotTrackSpeedProxyInfo, "IncomingRobotTrackSpeed");
		}
		// oversampling inputs
		else if(isFastAnalogSignal1Enabled() && (commandInfo.commandType == EOversamplingSignal1)){
            m_oOversamplingSignal1ProxyInfo = commandInfo.proxyInfo;
			activateAnalogOversamplingInput(m_oOversamplingSignal1ProxyInfo, "IncomingOversamplingSignal1");
		}
		else if(isFastAnalogSignal2Enabled() && (commandInfo.commandType == EOversamplingSignal2)){
            m_oOversamplingSignal2ProxyInfo = commandInfo.proxyInfo;
			activateAnalogOversamplingInput(m_oOversamplingSignal2ProxyInfo, "IncomingOversamplingSignal2");
		}
		else if(isFastAnalogSignal3Enabled() && (commandInfo.commandType == EOversamplingSignal3)){
            m_oOversamplingSignal3ProxyInfo = commandInfo.proxyInfo;
			activateAnalogOversamplingInput(m_oOversamplingSignal3ProxyInfo, "IncomingOversamplingSignal3");
		}
		else if(isFastAnalogSignal4Enabled() && (commandInfo.commandType == EOversamplingSignal4)){
            m_oOversamplingSignal4ProxyInfo = commandInfo.proxyInfo;
			activateAnalogOversamplingInput(m_oOversamplingSignal4ProxyInfo, "IncomingOversamplingSignal4");
		}
		else if(isFastAnalogSignal5Enabled() && (commandInfo.commandType == EOversamplingSignal5)){
            m_oOversamplingSignal5ProxyInfo = commandInfo.proxyInfo;
			activateAnalogOversamplingInput(m_oOversamplingSignal5ProxyInfo, "IncomingOversamplingSignal5");
		}
		else if(isFastAnalogSignal6Enabled() && (commandInfo.commandType == EOversamplingSignal6)){
            m_oOversamplingSignal6ProxyInfo = commandInfo.proxyInfo;
			activateAnalogOversamplingInput(m_oOversamplingSignal6ProxyInfo, "IncomingOversamplingSignal6");
		}
		else if(isFastAnalogSignal7Enabled() && (commandInfo.commandType == EOversamplingSignal7)){
            m_oOversamplingSignal7ProxyInfo = commandInfo.proxyInfo;
			activateAnalogOversamplingInput(m_oOversamplingSignal7ProxyInfo, "IncomingOversamplingSignal7");
		}
		else if(isFastAnalogSignal8Enabled() && (commandInfo.commandType == EOversamplingSignal8)){
            m_oOversamplingSignal8ProxyInfo = commandInfo.proxyInfo;
			activateAnalogOversamplingInput(m_oOversamplingSignal8ProxyInfo, "IncomingOversamplingSignal8");
		}
        else if(isLWM40_No1_Enabled() && (commandInfo.commandType == ELWM40_1_Plasma)){
            m_oLWM40_1_PlasmaProxyInfo = commandInfo.proxyInfo;
            activateLWM40Input(m_oLWM40_1_PlasmaProxyInfo, "IncomingLWM40_1_Plasma");
        }
        else if(isLWM40_No1_Enabled() && (commandInfo.commandType == ELWM40_1_Temperature)){
            m_oLWM40_1_TemperatureProxyInfo = commandInfo.proxyInfo;
            activateLWM40Input(m_oLWM40_1_TemperatureProxyInfo, "IncomingLWM40_1_Temperature");
        }
        else if(isLWM40_No1_Enabled() && (commandInfo.commandType == ELWM40_1_BackReflection)){
            m_oLWM40_1_BackReflectionProxyInfo = commandInfo.proxyInfo;
            activateLWM40Input(m_oLWM40_1_BackReflectionProxyInfo, "IncomingLWM40_1_BackReflection");
        }
        else if(isLWM40_No1_Enabled() && (commandInfo.commandType == ELWM40_1_AnalogInput)){
            m_oLWM40_1_AnalogInputProxyInfo = commandInfo.proxyInfo;
            activateLWM40Input(m_oLWM40_1_AnalogInputProxyInfo, "IncomingLWM40_1_AnalogInput");
        }
        // digital outputs
		else if(isScanTrackerEnabled() && (commandInfo.commandType == ATrackerEnableDriver)){
            m_oSTEnableDriverProxyInfo = commandInfo.proxyInfo;
			activateDigitalOutput(m_oSTEnableDriverProxyInfo, "SetEnableDriver");
		}
		else if(isZCollimatorEnabled() && (getZCollimatorType() == eZColliAnalog) && (commandInfo.commandType == AZCRefTravel)){
            m_oZCRefTravelProxyInfo = commandInfo.proxyInfo;
			activateDigitalOutput(m_oZCRefTravelProxyInfo, "Z-Coll RefTravel");
		}
		else if(isZCollimatorEnabled() && (getZCollimatorType() == eZColliAnalog) && (commandInfo.commandType == AZCAutomatic)){
            m_oZCAutomaticProxyInfo = commandInfo.proxyInfo;
			activateDigitalOutput(m_oZCAutomaticProxyInfo, "Z-Coll Automatic");
		}
		else if(isLaserControlEnabled() && (commandInfo.commandType == ALCStartSignal)){
            m_oLCStartSignalProxyInfo = commandInfo.proxyInfo;
			activateDigitalOutput(m_oLCStartSignalProxyInfo, "LC StartSignal");
		}
		// digital inputs
		else if(isScanTrackerEnabled() && (commandInfo.commandType == ETrackerScannerOK)){
            m_oSTScannerOkProxyInfo = commandInfo.proxyInfo;
			activateDigitalInput(m_oSTScannerOkProxyInfo, "GetScannerOK");
			m_oStartBitScannerOK = m_oSTScannerOkProxyInfo.nStartBit;
		}
		else if(isScanTrackerEnabled() && (commandInfo.commandType == ETrackerScannerLimits)){
            m_oSTScannerLimitsProxyInfo = commandInfo.proxyInfo;
			activateDigitalInput(m_oSTScannerLimitsProxyInfo, "GetScannerLimits");
			m_oStartBitScannerLimits = m_oSTScannerLimitsProxyInfo.nStartBit;
		}
		else if(isZCollimatorEnabled() && (getZCollimatorType() == eZColliAnalog) && (commandInfo.commandType == EZCError)){
            m_oZCErrorProxyInfo = commandInfo.proxyInfo;
			activateDigitalInput(m_oZCErrorProxyInfo, "Z-Coll Error");
			m_oStartBitZCError = m_oZCErrorProxyInfo.nStartBit;
		}
		else if(isZCollimatorEnabled() && (getZCollimatorType() == eZColliAnalog) && (commandInfo.commandType == EZCPosReached)){
            m_oZCPosReachedProxyInfo = commandInfo.proxyInfo;
			activateDigitalInput(m_oZCPosReachedProxyInfo, "Z-Coll PosReached");
			m_oStartBitZCPosReached = m_oZCPosReachedProxyInfo.nStartBit;
		}
		else if(isLaserControlEnabled() && (commandInfo.commandType == ELCErrorSignal)){
            m_oLCErrorSignalProxyInfo = commandInfo.proxyInfo;
			activateDigitalInput(m_oLCErrorSignalProxyInfo, "LC ErrorSignal");
			m_oStartBitLCErrorSignal = m_oLCErrorSignalProxyInfo.nStartBit;
		}
		else if(isLaserControlEnabled() && (commandInfo.commandType == ELCReadySignal)){
            m_oLCReadySignalProxyInfo = commandInfo.proxyInfo;
			activateDigitalInput(m_oLCReadySignalProxyInfo, "LC ReadySignal");
			m_oStartBitLCReadySignal = m_oLCReadySignalProxyInfo.nStartBit;
		}
		else if(isLaserControlEnabled() && (commandInfo.commandType == ELCLimitWarning)){
            m_oLCLimitWarningProxyInfo = commandInfo.proxyInfo;
			activateDigitalInput(m_oLCLimitWarningProxyInfo, "LC LimitWarning");
			m_oStartBitLCLimitWarning = m_oLCLimitWarningProxyInfo.nStartBit;
		}
        else if(isLED_IlluminationEnabled() && (m_oLEDControllerType == eLEDTypePP820) && (commandInfo.commandType == ELEDTemperatureHigh)){
            m_oLEDTemperatureHighProxyInfo = commandInfo.proxyInfo;
            activateDigitalInput(m_oLEDTemperatureHighProxyInfo, "LED Temperature High");
            m_oStartBitLEDTemperatureHigh = m_oLEDTemperatureHighProxyInfo.nStartBit;
        }
		// encoder inputs
		else if(isEncoderInput1Enabled() && (commandInfo.commandType == EEncoderInput1)){
            m_oEncoderInput1ProxyInfo = commandInfo.proxyInfo;
			activateEncoderInput(m_oEncoderInput1ProxyInfo, "EncoderInput1");
			// encoder output/reset
            m_oEncoderReset1ProxyInfo = commandInfo.proxyInfo;
			activateEncoderOutput(m_oEncoderReset1ProxyInfo, "EncoderReset1");
		}
		else if(isEncoderInput2Enabled() && (commandInfo.commandType == EEncoderInput2)){
            m_oEncoderInput2ProxyInfo = commandInfo.proxyInfo;
			activateEncoderInput(m_oEncoderInput2ProxyInfo, "EncoderInput2");
			// encoder output/reset
            m_oEncoderInput2ProxyInfo = commandInfo.proxyInfo;
			activateEncoderOutput(m_oEncoderInput2ProxyInfo, "EncoderReset2");
		}

		m_oConfigParser.m_inCommandsList.pop_front();
	}

	//############################################################################

	if (isAxisXEnabled())
	{
		bool oFound = false;

	    // gespeicherte SoftLimits Aktivierung aus WeldHeadDefaults.xml abholen
	    bool oSoftLimitsActive = WeldHeadDefaults::instance().getBool("AxisX_SoftLimitsActive", false, &oFound);
	    wmLog(eDebug, "AxisX_SoftLimitsActive (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte SoftLimits Aktivierung aus VI_Config.xml abholen
	    	oSoftLimitsActive = m_oConfigParser.areSoftLimitsAxisXActive();
	        wmLog(eDebug, "AxisX_SoftLimitsActive (VI_Config)(bool): %d\n", oSoftLimitsActive);
	        // SoftLimits Aktivierung von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setBool("AxisX_SoftLimitsActive", oSoftLimitsActive);
	    }
	    else
        {
	        wmLog(eDebug, "AxisX_SoftLimitsActive (WeldHeadDefaults)(bool): %d\n", oSoftLimitsActive);
        }

	    // gespeichertes SoftLowerLimit aus WeldHeadDefaults.xml abholen
	    int oSoftLowerLimit = WeldHeadDefaults::instance().getInt("AxisX_SoftLowerLimit", -1000, &oFound);
	    wmLog(eDebug, "AxisX_SoftLowerLimit (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeichertes SoftLowerLimit aus VI_Config.xml abholen
	    	oSoftLowerLimit = m_oConfigParser.getLowerSoftEndAxisX();
	        wmLog(eDebug, "AxisX_SoftLowerLimit (VI_Config)(int): %d\n", oSoftLowerLimit);
	        // SoftLimits Aktivierung von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setInt("AxisX_SoftLowerLimit", oSoftLowerLimit);
	    }
	    else
        {
	        wmLog(eDebug, "AxisX_SoftLowerLimit (WeldHeadDefaults)(bool): %d\n", oSoftLowerLimit);
        }

	    // gespeichertes SoftUpperLimit aus WeldHeadDefaults.xml abholen
	    int oSoftUpperLimit = WeldHeadDefaults::instance().getInt("AxisX_SoftUpperLimit", -30000, &oFound);
	    wmLog(eDebug, "AxisX_SoftUpperLimit (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeichertes SoftUpperLimit aus VI_Config.xml abholen
	    	oSoftUpperLimit = m_oConfigParser.getUpperSoftEndAxisX();
	        wmLog(eDebug, "AxisX_SoftUpperLimit (VI_Config)(int): %d\n", oSoftUpperLimit);
	        // SoftLimits Aktivierung von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setInt("AxisX_SoftUpperLimit", oSoftUpperLimit);
	    }
	    else
        {
	        wmLog(eDebug, "AxisX_SoftUpperLimit (WeldHeadDefaults)(bool): %d\n", oSoftUpperLimit);
        }

	    // gespeicherte HomingDirPos aus WeldHeadDefaults.xml abholen
	    bool oHomingDirPos = WeldHeadDefaults::instance().getBool("AxisX_HomingDirPos", false, &oFound);
	    wmLog(eDebug, "AxisX_HomingDirPos (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte HomingDirPos aus VI_Config.xml abholen
	    	oHomingDirPos = m_oConfigParser.getHomingDirPosAxisX();
	        wmLog(eDebug, "AxisX_HomingDirPos (VI_Config)(bool): %d\n", oHomingDirPos);
	        // HomingDirPos von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setBool("AxisX_HomingDirPos", oHomingDirPos);
	    }
	    else
        {
	        wmLog(eDebug, "AxisX_HomingDirPos (WeldHeadDefaults)(bool): %d\n", oHomingDirPos);
        }

	    // gespeicherte MountingRightTop aus WeldHeadDefaults.xml abholen
	    bool oMountingRightTop = WeldHeadDefaults::instance().getBool("AxisX_MountingRightTop", false, &oFound);
	    wmLog(eDebug, "AxisX_MountingRightTop (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte MountingRightTop aus VI_Config.xml abholen
	    	oMountingRightTop = m_oConfigParser.getMountingRightTopAxisX();
	        wmLog(eDebug, "AxisX_MountingRightTop (VI_Config)(bool): %d\n", oMountingRightTop);
	        // MountingRightTop von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setBool("AxisX_MountingRightTop", oMountingRightTop);
	    }
	    else
        {
	        wmLog(eDebug, "AxisX_MountingRightTop (WeldHeadDefaults)(bool): %d\n", oMountingRightTop);
        }

        if ((m_oConfigParser.getProductCodeAxisX() != 0) &&
            (m_oConfigParser.getVendorIDAxisX() != 0) &&
            (m_oConfigParser.getInstanceAxisX() != 0))
        {
            wmLog(eDebug, "ProductCode of Axis X: %x\n", m_oConfigParser.getProductCodeAxisX());
            if (m_oConfigParser.getProductCodeAxisX() == PRODUCTCODE_ACCELNET)
            {
                m_pStatesHeadX = new StateMachine(interface::eAxisX,
                        oSoftLimitsActive,oSoftLowerLimit,oSoftUpperLimit,
                        oHomingDirPos,oMountingRightTop,
                        m_oConfigParser.getProductCodeAxisX(),m_oConfigParser.getVendorIDAxisX(),
                        m_oConfigParser.getInstanceAxisX(), m_oConfigParser.isAxisXHomeable(),
                        m_cbHeadIsReady, m_cbHeadValueReached, m_cbHeadError, m_oSensorProxy,
                        m_rEthercatOutputsProxy);
            }
            else if (m_oConfigParser.getProductCodeAxisX() == PRODUCTCODE_EPOS4)
            {
                m_pStatesHeadX = new StateMachineV2(interface::eAxisX,
                        oSoftLimitsActive,oSoftLowerLimit,oSoftUpperLimit,
                        oHomingDirPos,oMountingRightTop,
                        m_oConfigParser.getProductCodeAxisX(),m_oConfigParser.getVendorIDAxisX(),
                        m_oConfigParser.getInstanceAxisX(), m_oConfigParser.isAxisXHomeable(),
                        m_cbHeadIsReady, m_cbHeadValueReached, m_cbHeadError, m_oSensorProxy,
                        m_rEthercatOutputsProxy);
            }
            else
            {
                wmLogTr(eError,"QnxMsg.VI.ABCDEF", "Wrong Productcode for Axis X");
            }
            if (m_pStatesHeadX != nullptr)
            {
                m_pStatesHeadX->SetAxisVelocity(WeldHeadDefaults::instance().getInt("X_Axis_Velocity", 100));
                m_pStatesHeadX->SetAxisAcceleration(WeldHeadDefaults::instance().getInt("X_Axis_Acceleration", 100));
                m_pStatesHeadX->SetAxisDeceleration(WeldHeadDefaults::instance().getInt("X_Axis_Acceleration", 100));
            }
        }
        usleep(20*1000); // 20ms
	}

	if (isAxisYEnabled())
	{
		bool oFound = false;

	    // gespeicherte SoftLimits Aktivierung aus WeldHeadDefaults.xml abholen
	    bool oSoftLimitsActive = WeldHeadDefaults::instance().getBool("AxisY_SoftLimitsActive", false, &oFound);
	    wmLog(eDebug, "AxisY_SoftLimitsActive (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte SoftLimits Aktivierung aus VI_Config.xml abholen
	    	oSoftLimitsActive = m_oConfigParser.areSoftLimitsAxisYActive();
	        wmLog(eDebug, "AxisY_SoftLimitsActive (VI_Config)(bool): %d\n", oSoftLimitsActive);
	        // SoftLimits Aktivierung von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setBool("AxisY_SoftLimitsActive", oSoftLimitsActive);
	    }
	    else
        {
	        wmLog(eDebug, "AxisY_SoftLimitsActive (WeldHeadDefaults)(bool): %d\n", oSoftLimitsActive);
        }

	    // gespeichertes SoftLowerLimit aus WeldHeadDefaults.xml abholen
	    int oSoftLowerLimit = WeldHeadDefaults::instance().getInt("AxisY_SoftLowerLimit", -1000, &oFound);
	    wmLog(eDebug, "AxisY_SoftLowerLimit (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeichertes SoftLowerLimit aus VI_Config.xml abholen
	    	oSoftLowerLimit = m_oConfigParser.getLowerSoftEndAxisY();
	        wmLog(eDebug, "AxisY_SoftLowerLimit (VI_Config)(int): %d\n", oSoftLowerLimit);
	        // SoftLimits Aktivierung von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setInt("AxisY_SoftLowerLimit", oSoftLowerLimit);
	    }
	    else
        {
	        wmLog(eDebug, "AxisY_SoftLowerLimit (WeldHeadDefaults)(bool): %d\n", oSoftLowerLimit);
        }

	    // gespeichertes SoftUpperLimit aus WeldHeadDefaults.xml abholen
	    int oSoftUpperLimit = WeldHeadDefaults::instance().getInt("AxisY_SoftUpperLimit", -30000, &oFound);
	    wmLog(eDebug, "AxisY_SoftUpperLimit (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeichertes SoftUpperLimit aus VI_Config.xml abholen
	    	oSoftUpperLimit = m_oConfigParser.getUpperSoftEndAxisY();
	        wmLog(eDebug, "AxisY_SoftUpperLimit (VI_Config)(int): %d\n", oSoftUpperLimit);
	        // SoftLimits Aktivierung von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setInt("AxisY_SoftUpperLimit", oSoftUpperLimit);
	    }
	    else
        {
	        wmLog(eDebug, "AxisY_SoftUpperLimit (WeldHeadDefaults)(bool): %d\n", oSoftUpperLimit);
        }

	    // gespeicherte HomingDirPos aus WeldHeadDefaults.xml abholen
	    bool oHomingDirPos = WeldHeadDefaults::instance().getBool("AxisY_HomingDirPos", false, &oFound);
	    wmLog(eDebug, "AxisY_HomingDirPos (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte HomingDirPos aus VI_Config.xml abholen
	    	oHomingDirPos = m_oConfigParser.getHomingDirPosAxisY();
	        wmLog(eDebug, "AxisY_HomingDirPos (VI_Config)(bool): %d\n", oHomingDirPos);
	        // HomingDirPos von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setBool("AxisY_HomingDirPos", oHomingDirPos);
	    }
	    else
        {
	        wmLog(eDebug, "AxisY_HomingDirPos (WeldHeadDefaults)(bool): %d\n", oHomingDirPos);
        }

	    // gespeicherte MountingRightTop aus WeldHeadDefaults.xml abholen
	    bool oMountingRightTop = WeldHeadDefaults::instance().getBool("AxisY_MountingRightTop", false, &oFound);
	    wmLog(eDebug, "AxisY_MountingRightTop (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte MountingRightTop aus VI_Config.xml abholen
	    	oMountingRightTop = m_oConfigParser.getMountingRightTopAxisY();
	        wmLog(eDebug, "AxisY_MountingRightTop (VI_Config)(bool): %d\n", oMountingRightTop);
	        // MountingRightTop von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setBool("AxisY_MountingRightTop", oMountingRightTop);
	    }
	    else
        {
	        wmLog(eDebug, "AxisY_MountingRightTop (WeldHeadDefaults)(bool): %d\n", oMountingRightTop);
        }

        if ((m_oConfigParser.getProductCodeAxisY() != 0) &&
            (m_oConfigParser.getVendorIDAxisY() != 0) &&
            (m_oConfigParser.getInstanceAxisY() != 0))
        {
            wmLog(eDebug, "ProductCode of Axis Y: %x\n", m_oConfigParser.getProductCodeAxisY());
            if (m_oConfigParser.getProductCodeAxisY() == PRODUCTCODE_ACCELNET)
            {
                m_pStatesHeadY = new StateMachine(interface::eAxisY,
                        oSoftLimitsActive,oSoftLowerLimit,oSoftUpperLimit,
                        oHomingDirPos,oMountingRightTop,
                        m_oConfigParser.getProductCodeAxisY(),m_oConfigParser.getVendorIDAxisY(),
                        m_oConfigParser.getInstanceAxisY(), m_oConfigParser.isAxisYHomeable(),
                        m_cbHeadIsReady, m_cbHeadValueReached, m_cbHeadError, m_oSensorProxy,
                        m_rEthercatOutputsProxy);
            }
            else if (m_oConfigParser.getProductCodeAxisY() == PRODUCTCODE_EPOS4)
            {
                m_pStatesHeadY = new StateMachineV2(interface::eAxisY,
                        oSoftLimitsActive,oSoftLowerLimit,oSoftUpperLimit,
                        oHomingDirPos,oMountingRightTop,
                        m_oConfigParser.getProductCodeAxisY(),m_oConfigParser.getVendorIDAxisY(),
                        m_oConfigParser.getInstanceAxisY(), m_oConfigParser.isAxisYHomeable(),
                        m_cbHeadIsReady, m_cbHeadValueReached, m_cbHeadError, m_oSensorProxy,
                        m_rEthercatOutputsProxy);
            }
            else
            {
                wmLogTr(eError,"QnxMsg.VI.ABCDEF", "Wrong Productcode for Axis Y");
            }
            if (m_pStatesHeadY != nullptr)
            {
                m_pStatesHeadY->SetAxisVelocity(WeldHeadDefaults::instance().getInt("Y_Axis_Velocity", 100));
                m_pStatesHeadY->SetAxisAcceleration(WeldHeadDefaults::instance().getInt("Y_Axis_Acceleration", 100));
                m_pStatesHeadY->SetAxisDeceleration(WeldHeadDefaults::instance().getInt("Y_Axis_Acceleration", 100));
            }
        }
        usleep(20*1000); // 20ms
	}

	if (isAxisZEnabled())
	{
		bool oFound = false;

	    // gespeicherte SoftLimits Aktivierung aus WeldHeadDefaults.xml abholen
	    bool oSoftLimitsActive = WeldHeadDefaults::instance().getBool("AxisZ_SoftLimitsActive", false, &oFound);
	    wmLog(eDebug, "AxisZ_SoftLimitsActive (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte SoftLimits Aktivierung aus VI_Config.xml abholen
	    	oSoftLimitsActive = m_oConfigParser.areSoftLimitsAxisZActive();
	        wmLog(eDebug, "AxisZ_SoftLimitsActive (VI_Config)(bool): %d\n", oSoftLimitsActive);
	        // SoftLimits Aktivierung von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setBool("AxisZ_SoftLimitsActive", oSoftLimitsActive);
	    }
	    else
        {
	        wmLog(eDebug, "AxisZ_SoftLimitsActive (WeldHeadDefaults)(bool): %d\n", oSoftLimitsActive);
        }

	    // gespeichertes SoftLowerLimit aus WeldHeadDefaults.xml abholen
	    int oSoftLowerLimit = WeldHeadDefaults::instance().getInt("AxisZ_SoftLowerLimit", -1000, &oFound);
	    wmLog(eDebug, "AxisZ_SoftLowerLimit (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeichertes SoftLowerLimit aus VI_Config.xml abholen
	    	oSoftLowerLimit = m_oConfigParser.getLowerSoftEndAxisZ();
	        wmLog(eDebug, "AxisZ_SoftLowerLimit (VI_Config)(int): %d\n", oSoftLowerLimit);
	        // SoftLimits Aktivierung von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setInt("AxisZ_SoftLowerLimit", oSoftLowerLimit);
	    }
	    else
        {
	        wmLog(eDebug, "AxisZ_SoftLowerLimit (WeldHeadDefaults)(bool): %d\n", oSoftLowerLimit);
        }

	    // gespeichertes SoftUpperLimit aus WeldHeadDefaults.xml abholen
	    int oSoftUpperLimit = WeldHeadDefaults::instance().getInt("AxisZ_SoftUpperLimit", -30000, &oFound);
	    wmLog(eDebug, "AxisZ_SoftUpperLimit (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeichertes SoftUpperLimit aus VI_Config.xml abholen
	    	oSoftUpperLimit = m_oConfigParser.getUpperSoftEndAxisZ();
	        wmLog(eDebug, "AxisZ_SoftUpperLimit (VI_Config)(int): %d\n", oSoftUpperLimit);
	        // SoftLimits Aktivierung von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setInt("AxisZ_SoftUpperLimit", oSoftUpperLimit);
	    }
	    else
        {
	        wmLog(eDebug, "AxisZ_SoftUpperLimit (WeldHeadDefaults)(bool): %d\n", oSoftUpperLimit);
        }

	    // gespeicherte HomingDirPos aus WeldHeadDefaults.xml abholen
	    bool oHomingDirPos = WeldHeadDefaults::instance().getBool("AxisZ_HomingDirPos", false, &oFound);
	    wmLog(eDebug, "AxisZ_HomingDirPos (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte HomingDirPos aus VI_Config.xml abholen
	    	oHomingDirPos = m_oConfigParser.getHomingDirPosAxisZ();
	        wmLog(eDebug, "AxisZ_HomingDirPos (VI_Config)(bool): %d\n", oHomingDirPos);
	        // HomingDirPos von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setBool("AxisZ_HomingDirPos", oHomingDirPos);
	    }
	    else
        {
	        wmLog(eDebug, "AxisZ_HomingDirPos (WeldHeadDefaults)(bool): %d\n", oHomingDirPos);
        }

	    // gespeicherte MountingRightTop aus WeldHeadDefaults.xml abholen
	    bool oMountingRightTop = WeldHeadDefaults::instance().getBool("AxisZ_MountingRightTop", false, &oFound);
	    wmLog(eDebug, "AxisZ_MountingRightTop (WeldHeadDefaults) found: %d\n", oFound);
	    if (oFound == false) // value is not present in WeldHeadDefaults.xml
	    {
	        // gespeicherte MountingRightTop aus VI_Config.xml abholen
	    	oMountingRightTop = m_oConfigParser.getMountingRightTopAxisZ();
	        wmLog(eDebug, "AxisZ_MountingRightTop (VI_Config)(bool): %d\n", oMountingRightTop);
	        // MountingRightTop von VI_Config.xml in WeldHeadDefaults.xml transferieren
	    	WeldHeadDefaults::instance().setBool("AxisZ_MountingRightTop", oMountingRightTop);
	    }
	    else
        {
	        wmLog(eDebug, "AxisZ_MountingRightTop (WeldHeadDefaults)(bool): %d\n", oMountingRightTop);
        }

        if ((m_oConfigParser.getProductCodeAxisZ() != 0) &&
            (m_oConfigParser.getVendorIDAxisZ() != 0) &&
            (m_oConfigParser.getInstanceAxisZ() != 0))
        {
            wmLog(eDebug, "ProductCode of Axis Z: %x\n", m_oConfigParser.getProductCodeAxisZ());
            if (m_oConfigParser.getProductCodeAxisZ() == PRODUCTCODE_ACCELNET)
            {
                m_pStatesHeadZ = new StateMachine(interface::eAxisZ,
                        oSoftLimitsActive,oSoftLowerLimit,oSoftUpperLimit,
                        oHomingDirPos,oMountingRightTop,
                        m_oConfigParser.getProductCodeAxisZ(),m_oConfigParser.getVendorIDAxisZ(),
                        m_oConfigParser.getInstanceAxisZ(), m_oConfigParser.isAxisZHomeable(),
                        m_cbHeadIsReady, m_cbHeadValueReached, m_cbHeadError, m_oSensorProxy,
                        m_rEthercatOutputsProxy);
            }
            else if (m_oConfigParser.getProductCodeAxisZ() == PRODUCTCODE_EPOS4)
            {
                m_pStatesHeadZ = new StateMachineV2(interface::eAxisZ,
                        oSoftLimitsActive,oSoftLowerLimit,oSoftUpperLimit,
                        oHomingDirPos,oMountingRightTop,
                        m_oConfigParser.getProductCodeAxisZ(),m_oConfigParser.getVendorIDAxisZ(),
                        m_oConfigParser.getInstanceAxisZ(), m_oConfigParser.isAxisZHomeable(),
                        m_cbHeadIsReady, m_cbHeadValueReached, m_cbHeadError, m_oSensorProxy,
                        m_rEthercatOutputsProxy);
            }
            else
            {
                wmLogTr(eError,"QnxMsg.VI.ABCDEF", "Wrong Productcode for Axis Z");
            }
            if (m_pStatesHeadZ != nullptr)
            {
                m_pStatesHeadZ->SetAxisVelocity(WeldHeadDefaults::instance().getInt("Z_Axis_Velocity", 100));
                m_pStatesHeadZ->SetAxisAcceleration(WeldHeadDefaults::instance().getInt("Z_Axis_Acceleration", 100));
                m_pStatesHeadZ->SetAxisDeceleration(WeldHeadDefaults::instance().getInt("Z_Axis_Acceleration", 100));
            }
        }
        usleep(20*1000); // 20ms
	}

	m_oSensorIdsEnabled.insert(std::make_pair(eWeldHeadAxisXPos, 	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eWeldHeadAxisYPos, 	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eWeldHeadAxisZPos, 	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeAnalogIn1,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eLaserPowerSignal, 	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eEncoderInput1, 		false));
	m_oSensorIdsEnabled.insert(std::make_pair(eEncoderInput2, 		false));
	m_oSensorIdsEnabled.insert(std::make_pair(eRobotTrackSpeed, 	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eOversamplingSignal1, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eOversamplingSignal2, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eOversamplingSignal3, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eOversamplingSignal4, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eOversamplingSignal5, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eOversamplingSignal6, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eOversamplingSignal7, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eOversamplingSignal8, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eScannerXPosition,    false));
	m_oSensorIdsEnabled.insert(std::make_pair(eScannerYPosition,    false));
	m_oSensorIdsEnabled.insert(std::make_pair(eLWM40_1_Plasma,      false));
	m_oSensorIdsEnabled.insert(std::make_pair(eLWM40_1_Temperature, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eLWM40_1_BackReflection, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eLWM40_1_AnalogInput, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeAnalogIn2,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeAnalogIn3,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeAnalogIn4,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeAnalogIn5,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeAnalogIn6,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeAnalogIn7,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeAnalogIn8,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eZCPositionDigV1,     false));
	m_oSensorIdsEnabled.insert(std::make_pair(eScannerWeldingFinished, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eFiberSwitchPosition, false));
    m_oSensorIdsEnabled.insert(std::make_pair(eContourPrepared, false));

	poco_assert_dbg(m_oSensorIdsEnabled.size() == eCountourPrepared + 1 - eExternSensorMin);

    m_pDebugFile1 = nullptr;
    m_oDebugInfoDebugFile1 = false;

    for(int i = 0;i < 10;i++)
    {
        for(int j = 0;j < 1200;j++)
        {
            m_oArrayDebugFile1[i][j] = 0;
        }
    }

    if (m_oDebugInfoDebugFile1)
    {
        m_pDebugFile1 = fopen("Oversamp_DebugFile.txt", "w");
        if (m_pDebugFile1 == NULL)
        {
            wmLog(eDebug, "WeldingHEadControl: fopen of pDebugFile failed (001)\n");
        }
    }

    m_pDebugFile2 = nullptr;
    m_oDebugInfoDebugFile2 = false;
    m_oWriteIndexDebugFile2 = 0;
    for(int i = 0;i < LENGTH_OF_DEBUGFILE2;i++)
    {
        m_oArrayDebugFile2[i] = 0;
    }

    if (isZCollimatorEnabled() && (getZCollimatorType() == eZColliDigitalV1))
    {
        std::string oZCollimator_IP_Address = SystemConfiguration::instance().getString("ZCollimator_IP_Address", "192.168.170.3");
        wmLog(eDebug, "oZCollimator_IP_Address: <%s>\n", oZCollimator_IP_Address.c_str());
        m_oTmlCommunicator.SetTmlControllerIpAddress(oZCollimator_IP_Address);

        bool oRetValue = m_oTmlCommunicator.connectTmlController();
        if (oRetValue == true)
        {
            wmLogTr(eInfo, "QnxMsg.VI.ZCollConnSucc", "Connection to Z-collimator controller was successful\n");

            uint16_t oSRLRegister = 0;
            uint16_t oSRHRegister = 0;
            uint16_t oMERRegister = 0;
            char oPrintString[81];
            m_oTmlCommunicator.readStatus(oSRLRegister, oSRHRegister, oMERRegister, oPrintString);
            wmLog(eDebug, "TML status: %s\n", oPrintString);
            bool oFault = static_cast<bool>(oSRHRegister & 0x8000);
            bool oAxisStatus = static_cast<bool>(oSRLRegister & 0x8000);
            if (oFault || !oAxisStatus)
            {
                wmLog(eDebug, "ZColliV2: Fault-Flag is set or AxisStatus is not set\n");
                wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
            }

            double oActPosition = m_oTmlCommunicator.getActualPositionUm(oPrintString);
            wmLog(eDebug, "TML position: %s\n", oPrintString);
#if !EMVTEST_NEW_ZCOLLIMATOR
            m_oZCPositionDigV1ReadOut.store(static_cast<int32_t>(oActPosition));
#endif
            m_oZCActPosition = static_cast<int>(oActPosition);
            doZCollDriving(eAbsoluteDriving, 8854);
        }
        else
        {
            wmFatal(eExtEquipment, "QnxMsg.VI.ZCollConnNotSucc", "Connection to Z-collimator controller was not successful\n");
            m_oZCActPosition = 0;
            m_oZCollimatorEnable = false;
        }
    }

#if CYCLE_TIMING_VIA_SERIAL_PORT
    ///////////////////////////////////////////////////////
    // open serial driver for debug outputs
    ///////////////////////////////////////////////////////
    g_oDebugSerialFd = open("/dev/ttyS0", O_RDWR | O_NOCTTY);
    printf("g_oDebugSerialFd: %d\n", g_oDebugSerialFd);
    if (g_oDebugSerialFd == -1)
    {
        printf("error in open\n");
        perror("");
    }
    g_oDTR01_flag = TIOCM_DTR;
    g_oRTS02_flag = TIOCM_RTS;
#endif
}

WeldingHeadControl::~WeldingHeadControl() {

#if CYCLE_TIMING_VIA_SERIAL_PORT
    close(g_oDebugSerialFd);
#endif

#if EMVTEST_NEW_ZCOLLIMATOR
    if (m_oZCPositionDigV1ReadOutThread_ID != 0)
    {
        if (pthread_cancel(m_oZCPositionDigV1ReadOutThread_ID) != 0)
        {
            wmLog(eDebug, "was not able to abort thread\n");
        }
    }
#endif

    if (m_oSendSensorDataThread_ID != 0)
    {
        if (pthread_cancel(m_oSendSensorDataThread_ID) != 0)
        {
            wmLog(eDebug, "was not able to abort thread\n");
        }
    }

    if (isZCollimatorEnabled() && (getZCollimatorType() == eZColliDigitalV1))
    {
        bool oRetValue = m_oTmlCommunicator.disconnectTmlController();
        if (oRetValue == true)
        {
            wmLogTr(eInfo, "QnxMsg.VI.ZCollDiscSucc", "Disconnection to Z-collimator controller was successful\n");
        }
        else
        {
            wmLogTr(eError, "QnxMsg.VI.ZCollDiscSucc", "Disconnection to Z-collimator controller was not successful\n");
        }
    }

    if (m_oDebugInfoDebugFile2)
    {
        m_pDebugFile2 = fopen("DebugFile2.txt", "w");
        if (m_pDebugFile2 == NULL)
        {
            wmLog(eDebug, "WeldingHeadControl: fopen of pDebugFile failed (002)\n");
        }
        else
        {
            for(int i = 0;i < LENGTH_OF_DEBUGFILE2;i++)
            {
                fprintf(m_pDebugFile2, "%6d\n", m_oArrayDebugFile2[i]);
            }
            fclose(m_pDebugFile2);
        }
    }

    if (m_oDebugInfoDebugFile1)
    {
        for(int i = 0;i < 1200;i++)
        {
            for(int j = 0;j < 10;j++)
            {
                fprintf(m_pDebugFile1, "%6d, ", m_oArrayDebugFile1[j][i]);
            }
            fprintf(m_pDebugFile1, "\n");
        }

        fclose(m_pDebugFile1);
    }

	cleanBuffer();

	if(m_pDataToLaserControl != NULL)
	{
		delete m_pDataToLaserControl;
	}

	if(m_pSerialToTracker != NULL)
	{
		delete m_pSerialToTracker;
	}

    if (isLiquidLensControllerEnabled())
    {
        if (m_lensDriverIsOperational)
        {
            m_lensDriver.deinit();
        }
    }

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE
	if ( isLED_IlluminationEnabled() )
	{
		if (m_oLED_DriverIsOperational)
		{
			m_oLEDdriver.deinit();
		}
	}
#endif

	delete m_pStatesHeadX;
	delete m_pStatesHeadY;
	delete m_pStatesHeadZ;

	delete m_cbHeadIsReady;
	delete m_cbHeadValueReached;
	delete m_cbHeadError;
}

void WeldingHeadControl::activateAnalogOutput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText)
{
    if (p_rProxyInfo.nProductCode == PRODUCTCODE_EL4102)
    {
        p_rProxyInfo.m_oProductIndex = eProductIndex_EL4102;
    }
    else if (p_rProxyInfo.nProductCode == PRODUCTCODE_EL4132)
    {
        p_rProxyInfo.m_oProductIndex = eProductIndex_EL4132;
    }
    else
    {
        // falscher ProductCode fuer Analog Output
    }

    p_rProxyInfo.m_oInstance = static_cast<EcatInstance>(p_rProxyInfo.nInstance);

	if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN1_)
    {
        p_rProxyInfo.m_oChannel = eChannel1;
	}
	else if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN2_)
    {
        p_rProxyInfo.m_oChannel = eChannel2;
	}
	else
    {
        // falscher channel fuer Analog Output
    }

    p_rProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "%s:\n", p_rInfoText.c_str());
	wmLog(eDebug, "m_oActive:   0x%x\n", p_rProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", p_rProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", p_rProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", p_rProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", p_rProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", p_rProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", p_rProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", p_rProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", p_rProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", p_rProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");
}

void WeldingHeadControl::activateAnalogInput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText)
{
    if (p_rProxyInfo.nProductCode == PRODUCTCODE_EL3102)
    {
        p_rProxyInfo.m_oProductIndex = eProductIndex_EL3102;
    }
    else
    {
        // falscher ProductCode fuer Analog Input
    }

    p_rProxyInfo.m_oInstance = static_cast<EcatInstance>(p_rProxyInfo.nInstance);

	if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN1_)
    {
        p_rProxyInfo.m_oChannel = eChannel1;
	}
	else if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN2_)
    {
        p_rProxyInfo.m_oChannel = eChannel2;
	}
	else
    {
        // falscher channel fuer Analog Input
    }

    p_rProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "%s:\n", p_rInfoText.c_str());
	wmLog(eDebug, "m_oActive:   0x%x\n", p_rProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", p_rProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", p_rProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", p_rProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", p_rProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", p_rProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", p_rProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", p_rProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", p_rProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", p_rProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");
}

void WeldingHeadControl::activateAnalogOversamplingInput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText)
{
    if (p_rProxyInfo.nProductCode == PRODUCTCODE_EL3702)
    {
        p_rProxyInfo.m_oProductIndex = eProductIndex_EL3702;
    }
    else
    {
        // falscher ProductCode fuer Analog Input
    }

    p_rProxyInfo.m_oInstance = static_cast<EcatInstance>(p_rProxyInfo.nInstance);

	if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN1_)
    {
        p_rProxyInfo.m_oChannel = eChannel1;
	}
	else if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN2_)
    {
        p_rProxyInfo.m_oChannel = eChannel2;
	}
	else
    {
        // falscher channel fuer Analog Input
    }

    p_rProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "%s:\n", p_rInfoText.c_str());
	wmLog(eDebug, "m_oActive:   0x%x\n", p_rProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", p_rProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", p_rProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", p_rProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", p_rProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", p_rProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", p_rProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", p_rProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", p_rProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", p_rProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");
}

void WeldingHeadControl::activateLWM40Input(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText)
{
    if (p_rProxyInfo.nProductCode == PRODUCTCODE_FRONTEND)
    {
        p_rProxyInfo.m_oProductIndex = eProductIndex_Frontend;
    }
    else
    {
        // falscher ProductCode fuer LWM40
    }

    p_rProxyInfo.m_oInstance = static_cast<EcatInstance>(p_rProxyInfo.nInstance);

	if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN1_)
    {
        p_rProxyInfo.m_oChannel = eChannel1;
	}
	else if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN2_)
    {
        p_rProxyInfo.m_oChannel = eChannel2;
	}
	else if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN3_)
    {
        p_rProxyInfo.m_oChannel = eChannel3;
	}
	else if (p_rProxyInfo.nSlaveType == _ANALOG_CHAN4_)
    {
        p_rProxyInfo.m_oChannel = eChannel4;
	}
	else
    {
        // falscher channel fuer LWM40
    }

    p_rProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "%s:\n", p_rInfoText.c_str());
	wmLog(eDebug, "m_oActive:   0x%x\n", p_rProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", p_rProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", p_rProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", p_rProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", p_rProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", p_rProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", p_rProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", p_rProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", p_rProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", p_rProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");
}

void WeldingHeadControl::activateDigitalOutput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText)
{
    if (p_rProxyInfo.nProductCode == PRODUCTCODE_EL2008)
    {
        p_rProxyInfo.m_oProductIndex = eProductIndex_EL2008;
    }
    else
    {
        // falscher ProductCode fuer Digital Output
    }
	// TODO: Digitale Ausgaenge auf Gateway ermoeglichen. Derzeit ist Ausgabefunktion nicht dafuer vorbereitet.
	//		 Aktuell sind hier auch keine digitalen Ausgaenge auf dem Gateway noetig.
	//		 05.03.13 EA

    p_rProxyInfo.m_oInstance = static_cast<EcatInstance>(p_rProxyInfo.nInstance);

    p_rProxyInfo.m_oChannel = static_cast<EcatChannel>(p_rProxyInfo.nStartBit + 1);

    p_rProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "%s:\n", p_rInfoText.c_str());
	wmLog(eDebug, "m_oActive:   0x%x\n", p_rProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", p_rProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", p_rProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", p_rProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", p_rProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", p_rProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", p_rProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", p_rProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", p_rProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", p_rProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");
}

void WeldingHeadControl::activateDigitalInput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText)
{
    if (p_rProxyInfo.nProductCode == PRODUCTCODE_EL1018)
    {
        p_rProxyInfo.m_oProductIndex = eProductIndex_EL1018;
    }
    else
    {
        // falscher ProductCode fuer Digital Input
    }

    p_rProxyInfo.m_oInstance = static_cast<EcatInstance>(p_rProxyInfo.nInstance);

    p_rProxyInfo.m_oChannel = static_cast<EcatChannel>(p_rProxyInfo.nStartBit + 1);

    p_rProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "%s:\n", p_rInfoText.c_str());
	wmLog(eDebug, "m_oActive:   0x%x\n", p_rProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", p_rProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", p_rProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", p_rProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", p_rProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", p_rProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", p_rProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", p_rProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", p_rProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", p_rProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");
}

void WeldingHeadControl::activateEncoderInput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText)
{
    switch (p_rProxyInfo.nProductCode)
    {
        case PRODUCTCODE_EL5101:
            p_rProxyInfo.m_oProductIndex = eProductIndex_EL5101;
            break;
        case PRODUCTCODE_EL5151:
            p_rProxyInfo.m_oProductIndex = eProductIndex_EL5151;
            break;
        default:
            // falscher ProductCode fuer Encoder Input
            break;
    }

    p_rProxyInfo.m_oInstance = static_cast<EcatInstance>(p_rProxyInfo.nInstance);

    p_rProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "%s:\n", p_rInfoText.c_str());
	wmLog(eDebug, "m_oActive:   0x%x\n", p_rProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", p_rProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", p_rProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", p_rProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", p_rProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", p_rProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", p_rProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", p_rProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", p_rProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", p_rProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");
}

void WeldingHeadControl::activateEncoderOutput(SLAVE_PROXY_INFORMATION &p_rProxyInfo, const std::string &p_rInfoText)
{
    switch (p_rProxyInfo.nProductCode)
    {
        case PRODUCTCODE_EL5101:
            p_rProxyInfo.m_oProductIndex = eProductIndex_EL5101;
            break;
        case PRODUCTCODE_EL5151:
            p_rProxyInfo.m_oProductIndex = eProductIndex_EL5151;
            break;
        default:
            // falscher ProductCode fuer Encoder Input
            break;
    }

    p_rProxyInfo.m_oInstance = static_cast<EcatInstance>(p_rProxyInfo.nInstance);

    p_rProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "%s:\n", p_rInfoText.c_str());
	wmLog(eDebug, "m_oActive:   0x%x\n", p_rProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", p_rProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", p_rProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", p_rProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", p_rProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", p_rProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", p_rProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", p_rProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", p_rProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", p_rProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");
}

int WeldingHeadControl::parseTrackerCalibrationFile(std::string& CalibFilename)
{
	char helpStrg[120];
	FILE *calibFileFd = fopen(CalibFilename.c_str(), "rb");
	if (calibFileFd != NULL)
	{
		while(fgets(helpStrg, 120, calibFileFd) != NULL)
		{
			helpStrg[strlen(helpStrg) - 2] = 0x00; // eliminiert CR * LF

			std::string dummy(helpStrg);
			std::string dummy1;
			std::string dummy2;
			std::string dummy3;

			if (dummy.find("freq") != std::string::npos)
			{
				interpretTrackerCalibLine(dummy, dummy1, dummy2, dummy3);
				int index = atoi(dummy2.c_str()); // Index in Array
				int value = atoi(dummy3.c_str()); // Frequenz
				m_oTrackerCalibArray[index].m_oFrequency = value;
			}
			if (dummy.find("maxVolt") != std::string::npos)
			{
				interpretTrackerCalibLine(dummy, dummy1, dummy2, dummy3);
				int index = atoi(dummy2.c_str()); // Index in Array
				double value = atof(dummy3.c_str()); // max. moegliche Volt
				m_oTrackerCalibArray[index].m_oMaxVolt = value;
			}
			if (dummy.find("maxWidth") != std::string::npos)
			{
				interpretTrackerCalibLine(dummy, dummy1, dummy2, dummy3);
				int index = atoi(dummy2.c_str()); // Index in Array
				int value = static_cast<int>(atof(dummy3.c_str()) * 1000); // Auslenkung in [um] fuer max. moegliche Volt
				m_oTrackerCalibArray[index].m_oMaxWidth250 = value;
			}
		}
		fclose (calibFileFd);
	}
	else
	{
		wmLogTr( eWarning, "QnxMsg.VI.TrackerCALFileNotF1", "ScanTracker Calibration File not found !\n" );
		wmLogTr( eWarning, "QnxMsg.VI.TrackerCALFileNotF2", "Standard values for ScanTracker Calibration are used!\n" );

		m_oTrackerCalibArray[0].m_oFrequency = 30;
		m_oTrackerCalibArray[0].m_oMaxVolt = 10.0;
		m_oTrackerCalibArray[0].m_oMaxWidth250 = 16400;
		m_oTrackerCalibArray[1].m_oFrequency = 40;
		m_oTrackerCalibArray[1].m_oMaxVolt = 10.0;
		m_oTrackerCalibArray[1].m_oMaxWidth250 = 16400;
		m_oTrackerCalibArray[2].m_oFrequency = 50;
		m_oTrackerCalibArray[2].m_oMaxVolt = 10.0;
		m_oTrackerCalibArray[2].m_oMaxWidth250 = 16400;
		m_oTrackerCalibArray[3].m_oFrequency = 100;
		m_oTrackerCalibArray[3].m_oMaxVolt = 10.0;
		m_oTrackerCalibArray[3].m_oMaxWidth250 = 16400;
		m_oTrackerCalibArray[4].m_oFrequency = 150;
		m_oTrackerCalibArray[4].m_oMaxVolt = 10.0;
		m_oTrackerCalibArray[4].m_oMaxWidth250 = 16000;
		m_oTrackerCalibArray[5].m_oFrequency = 200;
		m_oTrackerCalibArray[5].m_oMaxVolt = 7.0;
		m_oTrackerCalibArray[5].m_oMaxWidth250 = 10600;
		m_oTrackerCalibArray[6].m_oFrequency = 250;
		m_oTrackerCalibArray[6].m_oMaxVolt = 4.5;
		m_oTrackerCalibArray[6].m_oMaxWidth250 = 6600;
		m_oTrackerCalibArray[7].m_oFrequency = 300;
		m_oTrackerCalibArray[7].m_oMaxVolt = 3.0;
		m_oTrackerCalibArray[7].m_oMaxWidth250 = 4200;
		m_oTrackerCalibArray[8].m_oFrequency = 350;
		m_oTrackerCalibArray[8].m_oMaxVolt = 2.5;
		m_oTrackerCalibArray[8].m_oMaxWidth250 = 3200;
		m_oTrackerCalibArray[9].m_oFrequency = 400;
		m_oTrackerCalibArray[9].m_oMaxVolt = 2.0;
		m_oTrackerCalibArray[9].m_oMaxWidth250 = 2400;
		m_oTrackerCalibArray[10].m_oFrequency = 450;
		m_oTrackerCalibArray[10].m_oMaxVolt = 1.5;
		m_oTrackerCalibArray[10].m_oMaxWidth250 = 1600;
		m_oTrackerCalibArray[11].m_oFrequency = 500;
		m_oTrackerCalibArray[11].m_oMaxVolt = 1.5;
		m_oTrackerCalibArray[11].m_oMaxWidth250 = 1600;
		m_oTrackerCalibArray[12].m_oFrequency = 550;
		m_oTrackerCalibArray[12].m_oMaxVolt = 1.5;
		m_oTrackerCalibArray[12].m_oMaxWidth250 = 1400;
		m_oTrackerCalibArray[13].m_oFrequency = 600;
		m_oTrackerCalibArray[13].m_oMaxVolt = 1.5;
		m_oTrackerCalibArray[13].m_oMaxWidth250 = 1200;
		m_oTrackerCalibArray[14].m_oFrequency = 650;
		m_oTrackerCalibArray[14].m_oMaxVolt = 1.0;
		m_oTrackerCalibArray[14].m_oMaxWidth250 = 600;
		m_oTrackerCalibArray[15].m_oFrequency = 700;
		m_oTrackerCalibArray[15].m_oMaxVolt = 1.0;
		m_oTrackerCalibArray[15].m_oMaxWidth250 = 600;
		m_oTrackerCalibArray[16].m_oFrequency = 750;
		m_oTrackerCalibArray[16].m_oMaxVolt = 1.0;
		m_oTrackerCalibArray[16].m_oMaxWidth250 = 600;
	}
	return 0;
}

void WeldingHeadControl::interpretTrackerCalibLine(std::string& p_rFullLine, std::string& p_rPart1, std::string& p_rPart2, std::string& p_rPart3)
{
	p_rPart1.clear();
	p_rPart2.clear();
	p_rPart3.clear();

	int oState = 0;
	unsigned int oPos = 0;
	while (oPos < p_rFullLine.size()) // solange Zeichen im String
	{
		switch (oState)
		{
		case 0:
			if (isdigit(p_rFullLine[oPos]) == 0) // Zeichen ist keine Ziffer
			{
				p_rPart1.push_back(p_rFullLine[oPos]);
			}
			else // Zeichen ist Ziffer
			{
				p_rPart2.push_back(p_rFullLine[oPos]);
				oState = 1;
			}
			break;
		case 1:
			if (isdigit(p_rFullLine[oPos]) != 0) // Zeichen ist Ziffer
			{
				p_rPart2.push_back(p_rFullLine[oPos]);
			}
			else if (p_rFullLine[oPos] == '=') // Zeichen ist Gleichheitszeichen
			{
				oState = 2;
			}
			break;
		case 2:
			p_rPart3.push_back(p_rFullLine[oPos]);
			oState = 3;
			break;
		case 3:
			p_rPart3.push_back(p_rFullLine[oPos]);
			break;
		default:
			break;
		}
		oPos++;
	}
}

void WeldingHeadControl::CalculateFocalLengthDependencies(void)
{
	// calculate the range of the ScanPosition
	m_oMaxScanPosUM = static_cast<int>(6000.0 * (static_cast<double>(m_oFocalLength) / 250.0));
	m_oMaxScanPosMM = static_cast<double>(m_oMaxScanPosUM) / 1000.0;
	m_oMinScanPosUM = static_cast<int>(-6000.0 * (static_cast<double>(m_oFocalLength) / 250.0));
	m_oMinScanPosMM = static_cast<double>(m_oMinScanPosUM) / 1000.0;

	// calculate the gradient of the Volt-Width-Relation
	for(int i = 0;i < MAX_TRACKER_CALIB_DATA;i++)
	{
		m_oTrackerCalibArray[i].m_oMaxWidth = static_cast<int>(static_cast<double>(m_oTrackerCalibArray[i].m_oMaxWidth250)
				* (static_cast<double>(m_oFocalLength) / 250.0));
		m_oTrackerCalibArray[i].m_oGradient = m_oTrackerCalibArray[i].m_oMaxVolt / static_cast<double>(m_oTrackerCalibArray[i].m_oMaxWidth);
	}

	// log the limits for the ScanPosition
	wmLog(eDebug, "m_oMaxScanPosUM: %d, m_oMaxScanPosMM: %f, m_oMinScanPosUM: %d, m_oMinScanPosMM: %f\n",
			m_oMaxScanPosUM, m_oMaxScanPosMM, m_oMinScanPosUM, m_oMinScanPosMM);

	// log the content of the Scantracker Calibration Array
	wmLog(eDebug, "Content of ScanTracker Calibration Array\n");
	wmLog(eDebug, "----------------------------------------\n");
	for(int i = 0;i < MAX_TRACKER_CALIB_DATA;i++)
	{
		char oFreqVal[20];
		sprintf(oFreqVal, "%3d", m_oTrackerCalibArray[i].m_oFrequency);
		char oMVoltVal[20];
		sprintf(oMVoltVal, "%5.2f", m_oTrackerCalibArray[i].m_oMaxVolt);
		char oMWidth25Val[20];
		sprintf(oMWidth25Val, "%5d", m_oTrackerCalibArray[i].m_oMaxWidth250);
		char oMWidthVal[20];
		sprintf(oMWidthVal, "%5d", m_oTrackerCalibArray[i].m_oMaxWidth);
		char oGradVal[20];
		sprintf(oGradVal, "%8.6f", m_oTrackerCalibArray[i].m_oGradient);
		wmLog(eDebug, "frequency: %s , maxVolt: %s , maxWidth250: %s , maxWidth: %s , gradient: %s\n",
				oFreqVal,
				oMVoltVal,
				oMWidth25Val,
				oMWidthVal,
				oGradVal);
	}
}

void WeldingHeadControl::IncomingGenPurposeAnaIn_V2(AnalogInputNo p_oInput)
{
    const std::lock_guard<std::mutex> lock(m_mutex);

    TSmartArrayPtr<int>::ShArrayPtr* pValuesGPAI;
    Sensor oSensorId;
    switch(p_oInput)
    {
        case eAnalogIn1:
            pValuesGPAI = m_pValuesGPAI1;
            oSensorId = eGenPurposeAnalogIn1;
            break;
        case eAnalogIn2:
            pValuesGPAI = m_pValuesGPAI2;
            oSensorId = eGenPurposeAnalogIn2;
            break;
        case eAnalogIn3:
            pValuesGPAI = m_pValuesGPAI3;
            oSensorId = eGenPurposeAnalogIn3;
            break;
        case eAnalogIn4:
            pValuesGPAI = m_pValuesGPAI4;
            oSensorId = eGenPurposeAnalogIn4;
            break;
        case eAnalogIn5:
            pValuesGPAI = m_pValuesGPAI5;
            oSensorId = eGenPurposeAnalogIn5;
            break;
        case eAnalogIn6:
            pValuesGPAI = m_pValuesGPAI6;
            oSensorId = eGenPurposeAnalogIn6;
            break;
        case eAnalogIn7:
            pValuesGPAI = m_pValuesGPAI7;
            oSensorId = eGenPurposeAnalogIn7;
            break;
        case eAnalogIn8:
            pValuesGPAI = m_pValuesGPAI8;
            oSensorId = eGenPurposeAnalogIn8;
            break;
        default:
            wmLogTr(eError, "QnxMsg.VI.AnaInWrongIn", "analog input number does not fit to possible analog inputs\n");
            pValuesGPAI = m_pValuesGPAI1;
            oSensorId = eGenPurposeAnalogIn1;
            break;
    }

    if(m_imageNrGPAI[p_oInput] < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[oSensorId] == true)
    {
        pValuesGPAI[0][0] = (int)m_oGenPurposeAnaIn[p_oInput].load();
        m_context.setImageNumber(m_imageNrGPAI[p_oInput]);
        m_oSensorProxy.data(oSensorId,m_context,image::Sample(pValuesGPAI[0], 1));
        ++m_imageNrGPAI[p_oInput];
    }
}

void WeldingHeadControl::IncomingLaserPowerSignal_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if(m_imageNrLP < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[eLaserPowerSignal] == true)
    {
        m_pValuesLP[0][0] = (int)m_oLaserPowerSignal.load();
        m_context.setImageNumber(m_imageNrLP);
        m_oSensorProxy.data(eLaserPowerSignal,m_context,image::Sample(m_pValuesLP[0], 1));
        ++m_imageNrLP;
    }
}

void WeldingHeadControl::IncomingOversamplingSignal1_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrOverSmp1 < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eOversamplingSignal1] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oOversamplingSignal1RingBufferMutex);
            if (m_oOversamplingSignal1RingBuffer.GetWriteIndex() >= m_oOversamplingSignal1RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oOversamplingSignal1RingBuffer.GetWriteIndex()
                        - m_oOversamplingSignal1RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oOversamplingSignal1RingBuffer.GetBufferSize()
                        - m_oOversamplingSignal1RingBuffer.GetReadIndex()
                        + m_oOversamplingSignal1RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oOversamplingSignal1RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oOversamplingSignal1RingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesOverSmp1[0][i] = (int)m_oOversamplingSignal1RingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrOverSmp1);
        m_oSensorProxy.data(eOversamplingSignal1, m_context, image::Sample(m_pValuesOverSmp1[0], oValuesToSend));
#if 0
        if (m_oDebugInfoDebugFile2)
        {
            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                if (m_oWriteIndexDebugFile2 < LENGTH_OF_DEBUGFILE2)
                {
                    m_oArrayDebugFile2[m_oWriteIndexDebugFile2] = m_pValuesOverSmp1[0][i];
                    m_oWriteIndexDebugFile2++;
                }
            }
        }
#endif
        ++m_imageNrOverSmp1;
    }
}

void WeldingHeadControl::IncomingOversamplingSignal2_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrOverSmp2 < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eOversamplingSignal2] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oOversamplingSignal2RingBufferMutex);
            if (m_oOversamplingSignal2RingBuffer.GetWriteIndex() >= m_oOversamplingSignal2RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oOversamplingSignal2RingBuffer.GetWriteIndex()
                        - m_oOversamplingSignal2RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oOversamplingSignal2RingBuffer.GetBufferSize()
                        - m_oOversamplingSignal2RingBuffer.GetReadIndex()
                        + m_oOversamplingSignal2RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oOversamplingSignal2RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oOversamplingSignal2RingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesOverSmp2[0][i] = (int)m_oOversamplingSignal2RingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrOverSmp2);
        m_oSensorProxy.data(eOversamplingSignal2, m_context, image::Sample(m_pValuesOverSmp2[0], oValuesToSend));
        ++m_imageNrOverSmp2;
    }
}

void WeldingHeadControl::IncomingOversamplingSignal3_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrOverSmp3 < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eOversamplingSignal3] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oOversamplingSignal3RingBufferMutex);
            if (m_oOversamplingSignal3RingBuffer.GetWriteIndex() >= m_oOversamplingSignal3RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oOversamplingSignal3RingBuffer.GetWriteIndex()
                        - m_oOversamplingSignal3RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oOversamplingSignal3RingBuffer.GetBufferSize()
                        - m_oOversamplingSignal3RingBuffer.GetReadIndex()
                        + m_oOversamplingSignal3RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oOversamplingSignal3RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oOversamplingSignal3RingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesOverSmp3[0][i] = (int)m_oOversamplingSignal3RingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrOverSmp3);
        m_oSensorProxy.data(eOversamplingSignal3, m_context, image::Sample(m_pValuesOverSmp3[0], oValuesToSend));
#if 0
        if (m_oDebugInfoDebugFile2)
        {
            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                if (m_oWriteIndexDebugFile2 < LENGTH_OF_DEBUGFILE2)
                {
                    m_oArrayDebugFile2[m_oWriteIndexDebugFile2] = m_pValuesOverSmp3[0][i];
                    m_oWriteIndexDebugFile2++;
                }
            }
        }
#endif
        ++m_imageNrOverSmp3;
    }
}

void WeldingHeadControl::IncomingOversamplingSignal4_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrOverSmp4 < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eOversamplingSignal4] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oOversamplingSignal4RingBufferMutex);
            if (m_oOversamplingSignal4RingBuffer.GetWriteIndex() >= m_oOversamplingSignal4RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oOversamplingSignal4RingBuffer.GetWriteIndex()
                        - m_oOversamplingSignal4RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oOversamplingSignal4RingBuffer.GetBufferSize()
                        - m_oOversamplingSignal4RingBuffer.GetReadIndex()
                        + m_oOversamplingSignal4RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oOversamplingSignal4RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oOversamplingSignal4RingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesOverSmp4[0][i] = (int)m_oOversamplingSignal4RingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrOverSmp4);
        m_oSensorProxy.data(eOversamplingSignal4, m_context, image::Sample(m_pValuesOverSmp4[0], oValuesToSend));
        ++m_imageNrOverSmp4;
    }
}

void WeldingHeadControl::IncomingOversamplingSignal5_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrOverSmp5 < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eOversamplingSignal5] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oOversamplingSignal5RingBufferMutex);
            if (m_oOversamplingSignal5RingBuffer.GetWriteIndex() >= m_oOversamplingSignal5RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oOversamplingSignal5RingBuffer.GetWriteIndex()
                        - m_oOversamplingSignal5RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oOversamplingSignal5RingBuffer.GetBufferSize()
                        - m_oOversamplingSignal5RingBuffer.GetReadIndex()
                        + m_oOversamplingSignal5RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oOversamplingSignal5RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oOversamplingSignal5RingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesOverSmp5[0][i] = (int)m_oOversamplingSignal5RingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrOverSmp5);
        m_oSensorProxy.data(eOversamplingSignal5, m_context, image::Sample(m_pValuesOverSmp5[0], oValuesToSend));
        ++m_imageNrOverSmp5;
    }
}

void WeldingHeadControl::IncomingOversamplingSignal6_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrOverSmp6 < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eOversamplingSignal6] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oOversamplingSignal6RingBufferMutex);
            if (m_oOversamplingSignal6RingBuffer.GetWriteIndex() >= m_oOversamplingSignal6RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oOversamplingSignal6RingBuffer.GetWriteIndex()
                        - m_oOversamplingSignal6RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oOversamplingSignal6RingBuffer.GetBufferSize()
                        - m_oOversamplingSignal6RingBuffer.GetReadIndex()
                        + m_oOversamplingSignal6RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oOversamplingSignal6RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oOversamplingSignal6RingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesOverSmp6[0][i] = (int)m_oOversamplingSignal6RingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrOverSmp6);
        m_oSensorProxy.data(eOversamplingSignal6, m_context, image::Sample(m_pValuesOverSmp6[0], oValuesToSend));
        ++m_imageNrOverSmp6;
    }
}

void WeldingHeadControl::IncomingOversamplingSignal7_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrOverSmp7 < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eOversamplingSignal7] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oOversamplingSignal7RingBufferMutex);
            if (m_oOversamplingSignal7RingBuffer.GetWriteIndex() >= m_oOversamplingSignal7RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oOversamplingSignal7RingBuffer.GetWriteIndex()
                        - m_oOversamplingSignal7RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oOversamplingSignal7RingBuffer.GetBufferSize()
                        - m_oOversamplingSignal7RingBuffer.GetReadIndex()
                        + m_oOversamplingSignal7RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oOversamplingSignal7RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oOversamplingSignal7RingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesOverSmp7[0][i] = (int)m_oOversamplingSignal7RingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrOverSmp7);
        m_oSensorProxy.data(eOversamplingSignal7, m_context, image::Sample(m_pValuesOverSmp7[0], oValuesToSend));
        ++m_imageNrOverSmp7;
    }
}

void WeldingHeadControl::IncomingOversamplingSignal8_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrOverSmp8 < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eOversamplingSignal8] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oOversamplingSignal8RingBufferMutex);
            if (m_oOversamplingSignal8RingBuffer.GetWriteIndex() >= m_oOversamplingSignal8RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oOversamplingSignal8RingBuffer.GetWriteIndex()
                        - m_oOversamplingSignal8RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oOversamplingSignal8RingBuffer.GetBufferSize()
                        - m_oOversamplingSignal8RingBuffer.GetReadIndex()
                        + m_oOversamplingSignal8RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oOversamplingSignal8RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oOversamplingSignal8RingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesOverSmp8[0][i] = (int)m_oOversamplingSignal8RingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrOverSmp8);
        m_oSensorProxy.data(eOversamplingSignal8, m_context, image::Sample(m_pValuesOverSmp8[0], oValuesToSend));
        ++m_imageNrOverSmp8;
    }
}

void WeldingHeadControl::IncomingLWM40_1_Plasma_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrLWM40_1_Plasma < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eLWM40_1_Plasma] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oLWM40_1_PlasmaRingBufferMutex);
            if (m_oLWM40_1_PlasmaRingBuffer.GetWriteIndex() >= m_oLWM40_1_PlasmaRingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oLWM40_1_PlasmaRingBuffer.GetWriteIndex()
                        - m_oLWM40_1_PlasmaRingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oLWM40_1_PlasmaRingBuffer.GetBufferSize()
                        - m_oLWM40_1_PlasmaRingBuffer.GetReadIndex()
                        + m_oLWM40_1_PlasmaRingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oLWM40_1_PlasmaRingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oLWM40_1_PlasmaRingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesLWM40_1_Plasma[0][i] = (int)m_oLWM40_1_PlasmaRingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrLWM40_1_Plasma);
        m_oSensorProxy.data(eLWM40_1_Plasma, m_context, image::Sample(m_pValuesLWM40_1_Plasma[0], oValuesToSend));
        ++m_imageNrLWM40_1_Plasma;
    }
}

void WeldingHeadControl::IncomingLWM40_1_Temperature_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrLWM40_1_Temperature < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eLWM40_1_Temperature] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oLWM40_1_TemperatureRingBufferMutex);
            if (m_oLWM40_1_TemperatureRingBuffer.GetWriteIndex() >= m_oLWM40_1_TemperatureRingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oLWM40_1_TemperatureRingBuffer.GetWriteIndex()
                        - m_oLWM40_1_TemperatureRingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oLWM40_1_TemperatureRingBuffer.GetBufferSize()
                        - m_oLWM40_1_TemperatureRingBuffer.GetReadIndex()
                        + m_oLWM40_1_TemperatureRingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oLWM40_1_TemperatureRingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oLWM40_1_TemperatureRingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesLWM40_1_Temperature[0][i] = (int)m_oLWM40_1_TemperatureRingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrLWM40_1_Temperature);
        m_oSensorProxy.data(eLWM40_1_Temperature, m_context, image::Sample(m_pValuesLWM40_1_Temperature[0], oValuesToSend));
        ++m_imageNrLWM40_1_Temperature;
    }
}

void WeldingHeadControl::IncomingLWM40_1_BackReflection_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrLWM40_1_BackReflection < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eLWM40_1_BackReflection] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oLWM40_1_BackReflectionRingBufferMutex);
            if (m_oLWM40_1_BackReflectionRingBuffer.GetWriteIndex() >= m_oLWM40_1_BackReflectionRingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oLWM40_1_BackReflectionRingBuffer.GetWriteIndex()
                        - m_oLWM40_1_BackReflectionRingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oLWM40_1_BackReflectionRingBuffer.GetBufferSize()
                        - m_oLWM40_1_BackReflectionRingBuffer.GetReadIndex()
                        + m_oLWM40_1_BackReflectionRingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oLWM40_1_BackReflectionRingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oLWM40_1_BackReflectionRingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesLWM40_1_BackReflection[0][i] = (int)m_oLWM40_1_BackReflectionRingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrLWM40_1_BackReflection);
        m_oSensorProxy.data(eLWM40_1_BackReflection, m_context, image::Sample(m_pValuesLWM40_1_BackReflection[0], oValuesToSend));
        ++m_imageNrLWM40_1_BackReflection;
    }
}

void WeldingHeadControl::IncomingLWM40_1_AnalogInput_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if( (m_imageNrLWM40_1_AnalogInput < (int)m_interval.nbTriggers() ) && (m_oSensorIdsEnabled[eLWM40_1_AnalogInput] == true) )
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            FastMutex::ScopedLock lock(m_oLWM40_1_AnalogInputRingBufferMutex);
            if (m_oLWM40_1_AnalogInputRingBuffer.GetWriteIndex() >= m_oLWM40_1_AnalogInputRingBuffer.GetReadIndex())
            {
                oIndexDiff = m_oLWM40_1_AnalogInputRingBuffer.GetWriteIndex()
                        - m_oLWM40_1_AnalogInputRingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_oLWM40_1_AnalogInputRingBuffer.GetBufferSize()
                        - m_oLWM40_1_AnalogInputRingBuffer.GetReadIndex()
                        + m_oLWM40_1_AnalogInputRingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_oLWM40_1_AnalogInputRingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_oLWM40_1_AnalogInputRingBuffer.GetValuesPerImageCycle();
            }

            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                m_pValuesLWM40_1_AnalogInput[0][i] = (int)m_oLWM40_1_AnalogInputRingBuffer.Read();
            }
        } // mutex can be released
        m_context.setImageNumber(m_imageNrLWM40_1_AnalogInput);
        m_oSensorProxy.data(eLWM40_1_AnalogInput, m_context, image::Sample(m_pValuesLWM40_1_AnalogInput[0], oValuesToSend));
#if 0
        if (m_oDebugInfoDebugFile2)
        {
            for(unsigned int i = 0;i < oValuesToSend;i++)
            {
                if (m_oWriteIndexDebugFile2 < LENGTH_OF_DEBUGFILE2)
                {
                    m_oArrayDebugFile2[m_oWriteIndexDebugFile2] = m_pValuesLWM40_1_AnalogInput[0][i];
                    m_oWriteIndexDebugFile2++;
                }
            }
        }
#endif
        ++m_imageNrLWM40_1_AnalogInput;
    }
}

void WeldingHeadControl::IncomingScannerOK(unsigned char p_oValue)
{
	static bool oOldScannerOK = false;

	unsigned char oMask = 0;
	oMask = 1 << (m_oStartBitScannerOK);
	bool oActScannerOK = p_oValue & oMask;

	if(oActScannerOK != oOldScannerOK) // Signal has changed
	{
		TrackerInfo oTrackerInfo;
		if (oActScannerOK) // ScannerOK is now high, means OK
		{
			if (m_oTrackerDriverOnOff) // TrackerDriver is on
			{
				m_oScannerOK = true;
				oTrackerInfo.ScannerOK = true;
				//wmLogTr( eInfo, "QnxMsg.VI.Tracker_OK_ok", "ScanTracker signals: function ok\n" );
				wmLog( eDebug, "ScanTracker: ScannerOK is now high (001)\n" );
			}
			else // TrackerDriver is off
			{
				m_oScannerOK = true;
				oTrackerInfo.ScannerOK = true;
				//wmLogTr( eInfo, "QnxMsg.VI.Tracker_OK_ok", "ScanTracker signals: function ok\n" );
				wmLog( eDebug, "ScanTracker: ScannerOK is now high (002)\n" );
			}
		}
		else // ScannerOK is now low, means NOK
		{
			if (m_oTrackerDriverOnOff) // TrackerDriver is on
			{
				m_oScannerOK = false;
				oTrackerInfo.ScannerOK = false;
				//wmLogTr( eError, "QnxMsg.VI.Tracker_OK_nok", "ScanTracker signals: failure\n" );
				wmLog( eDebug, "ScanTracker: ScannerOK is now low (001)\n" );
				SetTrackerScanWidth(true); // in FailSafe-Zustand gehen
				SetTrackerScanPos(true); // in FailSafe-Zustand gehen
				wmFatal(eAxis, "QnxMsg.VI.STsignalsNOK", "Scantracker signals an error, Scanner is no longer in the OK state !\n");
			}
			else // TrackerDriver is off
			{
				m_oScannerOK = false;
				oTrackerInfo.ScannerOK = false;
				//wmLogTr( eError, "QnxMsg.VI.Tracker_OK_nok", "ScanTracker signals: failure\n" );
				wmLog( eDebug, "ScanTracker: ScannerOK is now low (002)\n" );
				SetTrackerScanWidth(true); // in FailSafe-Zustand gehen
				SetTrackerScanPos(true); // in FailSafe-Zustand gehen
			}
		}
		oTrackerInfo.ScannerLimits = m_oScannerLimits;
		trackerInfo(oTrackerInfo);

		oOldScannerOK = oActScannerOK;
	}
}

void WeldingHeadControl::IncomingScannerLimits(unsigned char p_oValue)
{
	static bool oOldScannerLimits = false;

	unsigned char oMask = 0;
	oMask = 1 << (m_oStartBitScannerLimits);
	bool oActScannerLimits = p_oValue & oMask;

	if(oActScannerLimits != oOldScannerLimits) // Signal has changed
	{
		TrackerInfo oTrackerInfo;
		if (oActScannerLimits) // ScannerLimits is now high, means OK
		{
			if (m_oTrackerDriverOnOff) // TrackerDriver is on
			{
				m_oScannerLimits = true;
				oTrackerInfo.ScannerLimits = true;
				//wmLogTr( eInfo, "QnxMsg.VI.Tracker_Limits_ok", "ScanTracker signals: limits ok\n" );
				wmLog( eDebug, "ScanTracker: ScannerLimits is now high (001)\n" );
			}
			else // TrackerDriver is off
			{
				m_oScannerLimits = true;
				oTrackerInfo.ScannerLimits = true;
				//wmLogTr( eInfo, "QnxMsg.VI.Tracker_Limits_ok", "ScanTracker signals: limits ok\n" );
				wmLog( eDebug, "ScanTracker: ScannerLimits is now high (002)\n" );
			}
		}
		else // ScannerLimits is now low, means NOK
		{
			if (m_oTrackerDriverOnOff) // TrackerDriver is on
			{
				m_oScannerLimits = false;
				oTrackerInfo.ScannerLimits = false;
				// Soll hier auch eine wmFatal Meldung geschickt werden ?
				wmLogTr( eError, "QnxMsg.VI.Tracker_Limits_nok", "ScanTracker signals: limits eceeded\n" );
				wmLog( eDebug, "ScanTracker: ScannerLimits is now low (001)\n" );
			}
			else // TrackerDriver is off
			{
				m_oScannerLimits = false;
				oTrackerInfo.ScannerLimits = false;
				//wmLogTr( eError, "QnxMsg.VI.Tracker_Limits_nok", "ScanTracker signals: limits eceeded\n" );
				wmLog( eDebug, "ScanTracker: ScannerLimits is now low (002)\n" );
			}
		}
		oTrackerInfo.ScannerOK = m_oScannerOK;
		trackerInfo(oTrackerInfo);

		oOldScannerLimits = oActScannerLimits;
	}
}

void WeldingHeadControl::IncomingEncoderInput1_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if(m_imageNrENC1 < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[eEncoderInput1] == true)
    {
        m_pValuesENC1[0][0] = (int)m_oEncoderInput1.load();
        m_context.setImageNumber(m_imageNrENC1);
        m_oSensorProxy.data(eEncoderInput1,m_context,image::Sample(m_pValuesENC1[0], 1));
        ++m_imageNrENC1;
    }
}

void WeldingHeadControl::IncomingEncoderInput2_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if(m_imageNrENC2 < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[eEncoderInput2] == true)
    {
        m_pValuesENC2[0][0] = (int)m_oEncoderInput2.load();
        m_context.setImageNumber(m_imageNrENC2);
        m_oSensorProxy.data(eEncoderInput2,m_context,image::Sample(m_pValuesENC2[0], 1));
        ++m_imageNrENC2;
    }
}

void WeldingHeadControl::ClearEncoderCounter(EncoderNumberType p_oEncoderNo)
{
	wmLog(eDebug, "ResetEncoderCounter: EncoderNo: %d\n", p_oEncoderNo);

	if (p_oEncoderNo == eEncoderInputNo1)
	{
        if (m_oEncoderReset1ProxyInfo.m_oActive)
        {
            m_rEthercatOutputsProxy.ecatEncoderOut(m_oEncoderReset1ProxyInfo.m_oProductIndex,
                                                   m_oEncoderReset1ProxyInfo.m_oInstance,
                                                   (uint16_t) 0x04, (uint32_t) 0);
			usleep(2 * 1000); // 2ms
            m_rEthercatOutputsProxy.ecatEncoderOut(m_oEncoderReset1ProxyInfo.m_oProductIndex,
                                                   m_oEncoderReset1ProxyInfo.m_oInstance,
                                                   (uint16_t) 0x00, (uint32_t) 0);
        }
	}
	else if (p_oEncoderNo == eEncoderInputNo2)
	{
        if (m_oEncoderReset2ProxyInfo.m_oActive)
        {
            m_rEthercatOutputsProxy.ecatEncoderOut(m_oEncoderReset2ProxyInfo.m_oProductIndex,
                                                   m_oEncoderReset2ProxyInfo.m_oInstance,
                                                   (uint16_t) 0x04, (uint32_t) 0);
			usleep(2 * 1000); // 2ms
            m_rEthercatOutputsProxy.ecatEncoderOut(m_oEncoderReset2ProxyInfo.m_oProductIndex,
                                                   m_oEncoderReset2ProxyInfo.m_oInstance,
                                                   (uint16_t) 0x00, (uint32_t) 0);
        }
	}
	else
	{
        // falscher Encoder !
	}

	//if (info->m_productCode == EL5032)
	//{
	//	// Patch fuer Singleturn Encoder !
	//	m_oEncoderRevolutions = 0;
	//}
}

void WeldingHeadControl::IncomingRobotTrackSpeed_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if(m_imageNrRobSpeed < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[eRobotTrackSpeed] == true)
    {
        m_pValuesRobSpeed[0][0] = (int)m_oRobotTrackSpeed.load();
        m_context.setImageNumber(m_imageNrRobSpeed);
        m_oSensorProxy.data(eRobotTrackSpeed,m_context,image::Sample(m_pValuesRobSpeed[0], 1));
        ++m_imageNrRobSpeed;
    }
}

void WeldingHeadControl::IncomingScannerXPosition_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if(m_imageNrScannerXPos < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[eScannerXPosition] == true)
    {
        if (m_pScanlab != nullptr)
        {
            m_pValuesScannerXPos[0][0] = static_cast<int>(m_pScanlab->GetScannerActXPosition() * 1000);
        }
        else
        {
            m_pValuesScannerXPos[0][0] = static_cast<int>(0);
        }
        m_context.setImageNumber(m_imageNrScannerXPos);
        m_oSensorProxy.data(eScannerXPosition,m_context,image::Sample(m_pValuesScannerXPos[0], 1));
wmLog(eDebug, "IncomingScannerXPosition_V2 (%d)\n", m_pValuesScannerXPos[0][0]);
        ++m_imageNrScannerXPos;
    }
}

void WeldingHeadControl::IncomingScannerYPosition_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if(m_imageNrScannerYPos < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[eScannerYPosition] == true)
    {
        if (m_pScanlab != nullptr)
        {
            m_pValuesScannerYPos[0][0] = static_cast<int>(m_pScanlab->GetScannerActYPosition() * 1000);
        }
        else
        {
            m_pValuesScannerYPos[0][0] = static_cast<int>(0);
        }
        m_context.setImageNumber(m_imageNrScannerYPos);
        m_oSensorProxy.data(eScannerYPosition,m_context,image::Sample(m_pValuesScannerYPos[0], 1));
wmLog(eDebug, "IncomingScannerYPosition_V2 (%d)\n", m_pValuesScannerYPos[0][0]);
        ++m_imageNrScannerYPos;
    }
}

void WeldingHeadControl::IncomingFiberSwitchPosition_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if (m_imageNrFiberSwitchPos < int( m_interval.nbTriggers()) && m_oSensorIdsEnabled[eFiberSwitchPosition] == true)
    {
        if (m_pScanlab != nullptr)
        {
            m_pValuesFiberSwitchPos[0][0] = m_pScanlab->GetOCTReferenceArmActual() - 1;
        }
        else
        {
            m_pValuesFiberSwitchPos[0][0] = static_cast<int>(0);
        }
        m_context.setImageNumber(m_imageNrFiberSwitchPos);
        m_oSensorProxy.data(eFiberSwitchPosition,m_context,image::Sample(m_pValuesFiberSwitchPos[0], 1));
        ++m_imageNrFiberSwitchPos;
    }
}

void WeldingHeadControl::IncomingZCPositionDigV1_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if(m_imageNrZCPositionDigV1 < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[eZCPositionDigV1] == true)
    {
        m_pValuesZCPositionDigV1[0][0] = static_cast<int>(m_oZCPositionDigV1ReadOut.load() );
        m_context.setImageNumber(m_imageNrZCPositionDigV1);
        m_oSensorProxy.data(eZCPositionDigV1,m_context,image::Sample(m_pValuesZCPositionDigV1[0], 1));
wmLog(eDebug, "IncomingZCPositionDigV1_V2 (%d)\n", m_pValuesZCPositionDigV1[0][0]);
        ++m_imageNrZCPositionDigV1;
    }
}

void WeldingHeadControl::IncomingScannerWeldingFinished_V2(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if(m_imageNrScannerWeldingFinished < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[eScannerWeldingFinished] == true)
    {
        m_pValuesScannerWeldingFinished[0][0] = static_cast<int>(m_pScanlab->GetScannerWeldingFinished() );
        m_context.setImageNumber(m_imageNrScannerWeldingFinished);
        m_oSensorProxy.data(eScannerWeldingFinished,m_context,image::Sample(m_pValuesScannerWeldingFinished[0], 1));
wmLog(eDebug, "IncomingScannerWeldingFinished_V2 (%d)\n", m_pValuesScannerWeldingFinished[0][0]);
        ++m_imageNrScannerWeldingFinished;
    }
}

void WeldingHeadControl::IncomingContourPreparedFinished(void)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    if (m_imageNrContourPreparedFinished < int(m_interval.nbTriggers()) && m_oSensorIdsEnabled[eContourPrepared] == true)
    {
        m_pValuesContourPreparedFinished[0][0] = static_cast<int>(m_pScanlab->GetContourPreparedFinished());
        m_context.setImageNumber(m_imageNrContourPreparedFinished);
        m_oSensorProxy.data(eContourPrepared,m_context,image::Sample(m_pValuesContourPreparedFinished[0], 1));
        wmLog(eDebug, "IncomingContourPreparedFinished (%d)\n", m_pValuesContourPreparedFinished[0][0]);
        ++m_imageNrContourPreparedFinished;
    }
}

void WeldingHeadControl::IncomingZCError(unsigned char p_oValue)
{
	static bool oOldZCError = false;

	unsigned char oMask = 0;
	oMask = 1 << (m_oStartBitZCError);
	bool oActZCError = p_oValue & oMask;

	if(oActZCError != oOldZCError)
	{
		//wmLog(eDebug, "IncomingZCError: Value: %d\n", oActZCError);
//		wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
		oOldZCError = oActZCError;
	}
	m_oZCError = oActZCError;
}

#define ZC_STATES_DEBUG 0

void WeldingHeadControl::IncomingZCPosReached(unsigned char p_oValue)
{
	static bool oOldZCPosReached = false;

	unsigned char oMask = 0;
	oMask = 1 << (m_oStartBitZCPosReached);
	bool oActZCPosReached = p_oValue & oMask;

	if(oActZCPosReached != oOldZCPosReached)
	{
		wmLog(eDebug, "IncomingZCPosReached: Value: %d\n", oActZCPosReached);
		oOldZCPosReached = oActZCPosReached;
	}
	m_oZCPosReached = oActZCPosReached;

	// Ablauf bei Ausloesen von Automatic (Fahren auf Position)
	switch(m_oZCStateVarAutomatic)
	{
	case 0:
		if (m_oZCAutomaticIsActive)
		{
#if ZC_STATES_DEBUG
			wmLog(eDebug, "gehe in state 1.1\n");
#endif
			m_oZCStateVarAutomatic = 1;
		}
		break;
	case 1:
		if (!oActZCPosReached) // PosReached false
		{
#if ZC_STATES_DEBUG
			wmLog(eDebug, "gehe in state 1.2\n");
#endif
			m_oZCStateVarAutomatic = 2;
		}
		break;
	case 2:
		if (oActZCPosReached) // PosReached true
		{
#if ZC_STATES_DEBUG
			wmLog(eDebug, "gehe in state 1.0\n");
#endif
			m_oZCRefTravelEvent.set();
			m_oZCStateVarAutomatic = 0;
			SetZCAutomatic(false);
//			m_oZCActPosition = m_oZCNewPositionValue;
			if(!m_oZCError) // Fehler wird durch Low-Zustand angezeigt
			{
				wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
			}

		}
		break;
	default:
		break;
	}

	// Ablauf bei Ausloesen von RefTravel (Referenzieren des Z-Collimator)
	//	static int m_oZCStateVarRefTravel = 0;
	static int loopCounter = 0;
	switch(m_oZCStateVarRefTravel)
	{
	case 0:
		if (m_oZCRefTravelIsActive)
		{
#if ZC_STATES_DEBUG
			wmLog(eDebug, "gehe in state 2.1\n");
#endif
			m_oZCStateVarRefTravel = 1;
		}
		break;
	case 1:
		//wmLog(eDebug, "ich bin in state 2.1\n");
		if (!oActZCPosReached) // PosReached false
		{
#if ZC_STATES_DEBUG
			wmLog(eDebug, "gehe in state 2.2\n");
#endif
			loopCounter = 0;
			m_oZCStateVarRefTravel = 2;
		}
		break;
	case 2:
		loopCounter++;
		if (loopCounter > 2000) // mehr als 2 Sek sind vergangen
		{
			if (oActZCPosReached) // PosReached true
			{
#if ZC_STATES_DEBUG
				wmLog(eDebug, "gehe in state 2.0\n");
#endif
				//m_oZCRefTravelEvent.set();
				m_oZCStateVarRefTravel = 0;
				SetZCRefTravel(false);
				if(!m_oZCError) // Fehler wird durch Low-Zustand angezeigt
				{
					wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
				}

			}
		}
		break;
	default:
		break;
	}
}

void WeldingHeadControl::SetZCRefTravel(bool p_oOnOff)
{
	uint8_t oMask = 0;
	uint8_t oSendValue = 0;
	if(p_oOnOff)
	{
		oSendValue = oSendValue | (1 << m_oZCRefTravelProxyInfo.nStartBit);
	}
	oMask = oMask | (1 << m_oZCRefTravelProxyInfo.nStartBit);

	wmLog(eDebug, "SetZCRefTravel: SendValue: %X Mask: %X\n", oSendValue, oMask);

    if (m_oZCRefTravelProxyInfo.m_oActive)
    {
        m_rEthercatOutputsProxy.ecatDigitalOut(m_oZCRefTravelProxyInfo.m_oProductIndex,
                                               m_oZCRefTravelProxyInfo.m_oInstance,
                                               (uint8_t) oSendValue,
                                               (uint8_t) oMask);
    }

	m_oZCRefTravelIsActive = p_oOnOff;

	if ((!p_oOnOff)&&(m_oZCError)) // wenn kein Error(low active)
	{
		usleep(2 * 1000);
		SetZCAnalogInValue(8000); // Fahren in die Mitte des Fahrbereichs (in um)
		usleep(2 * 1000);
		SetZCAutomatic(true);
        m_oZCActPosition = 8000;
	}
}

void WeldingHeadControl::SetZCAutomatic(bool p_oOnOff)
{
	unsigned char oMask = 0;
	unsigned char oSendValue = 0;
	if(p_oOnOff)
	{
		oSendValue = oSendValue | (1 << m_oZCAutomaticProxyInfo.nStartBit);
	}
	oMask = oMask | (1 << m_oZCAutomaticProxyInfo.nStartBit);

	wmLog(eDebug, "SetZCAutomatic: SendValue: %X Mask: %X\n", oSendValue, oMask);

    if (m_oZCAutomaticProxyInfo.m_oActive)
    {
        m_rEthercatOutputsProxy.ecatDigitalOut(m_oZCAutomaticProxyInfo.m_oProductIndex,
                                               m_oZCAutomaticProxyInfo.m_oInstance,
                                               (uint8_t) oSendValue,
                                               (uint8_t) oMask);
    }

	m_oZCAutomaticIsActive = p_oOnOff;
}

void WeldingHeadControl::SetZCAnalogInValue(int p_oValue)
{
	// Wert kommt in um an, auf den gesamten Fahrbereich bezogen (0um -> 16000um)
	// Jetzt den Wert in die Analog-Spannung umrechnen (0V - 10V) (siehe TI Z-Coll, Doku 95650)
	float oAnalogValue = ((static_cast<float>(p_oValue) / 1000.0) * 0.5882) + 0.294;

	if(oAnalogValue > 9.706)
	{
		oAnalogValue = 9.706;
		wmLogTr( eError, "QnxMsg.VI.ZCollMaxPos", "position of Z-collimator is limited due to max. possible position ! (to %f V)\n", oAnalogValue);
	}
	if(oAnalogValue < 0.294)
	{
		oAnalogValue = 0.294;
		wmLogTr( eError, "QnxMsg.VI.ZCollMinPos", "position of Z-collimator is limited due to min. possible position ! (to %f V)\n", oAnalogValue);
	}

	//10V -> 32768
	//5V -> 16384
	unsigned short oBinValueUShort = static_cast<unsigned short>(3276.8 * oAnalogValue);
	if (oBinValueUShort > 0x7FFF) oBinValueUShort = 0x7FFF;

	wmLog(eDebug, "SetZCAnalogInValue: Value: %dum %fV (%d)\n", p_oValue, oAnalogValue, oBinValueUShort);

    if (m_oZCAnalogInProxyInfo.m_oActive)
    {
        m_rEthercatOutputsProxy.ecatAnalogOut(m_oZCAnalogInProxyInfo.m_oProductIndex,
                                              m_oZCAnalogInProxyInfo.m_oInstance,
                                              m_oZCAnalogInProxyInfo.m_oChannel,
                                              (uint16_t) oBinValueUShort);
    }
//	???	m_oZCNewPositionValue = p_oValue; // Wert fuer Position in um
}

void WeldingHeadControl::SetZCSystemOffset(double p_oZCSystemOffset)
{
    if (p_oZCSystemOffset != m_oZCSystemOffset)
    {
        WeldHeadDefaults::instance().setDouble("ZCollimator_SystemOffset", p_oZCSystemOffset);
    }
    m_oZCSystemOffset = p_oZCSystemOffset;
    doZCollDriving(eRelativeDriving, 0);
}

void WeldingHeadControl::SetZCLensToFocusRatio(double p_oZCLensToFocusRatio)
{
    if (p_oZCLensToFocusRatio != m_oZCLensToFocusRatio)
    {
        WeldHeadDefaults::instance().setDouble("ZCollimator_LensToFocusRatio", p_oZCLensToFocusRatio);
    }
    m_oZCLensToFocusRatio = p_oZCLensToFocusRatio;
}

void WeldingHeadControl::setLiquidLensPosition(double value)
{
    m_liquidLensPosition = value;

    LEDI_ParametersT parameters{};
    parameters.led_enable = 1;
    parameters.led_brightness = static_cast<int>(value * 1000.0);

    m_lensDriver.updateall(&parameters);
}

void WeldingHeadControl::IncomingLCErrorSignal(unsigned char p_oValue)
{
	static bool oOldLCErrorSignal = false;

	unsigned char oMask = 0;
	oMask = 1 << (m_oStartBitLCErrorSignal);
	bool oActLCErrorSignal = p_oValue & oMask;

	if(oActLCErrorSignal != oOldLCErrorSignal)
	{
		wmLog(eDebug, "IncomingLCErrorSignal: Value: %d\n", oActLCErrorSignal);
		oOldLCErrorSignal = oActLCErrorSignal;
	}
}

void WeldingHeadControl::IncomingLCReadySignal(unsigned char p_oValue)
{
	static bool oOldLCReadySignal = false;

	unsigned char oMask = 0;
	oMask = 1 << (m_oStartBitLCReadySignal);
	bool oActLCReadySignal = p_oValue & oMask;

	if(oActLCReadySignal != oOldLCReadySignal)
	{
		wmLog(eDebug, "IncomingLCReadySignal: Value: %d\n", oActLCReadySignal);
		oOldLCReadySignal = oActLCReadySignal;
	}
}

void WeldingHeadControl::IncomingLCLimitWarning(unsigned char p_oValue)
{
	static bool oOldLCLimitWarning = false;

	unsigned char oMask = 0;
	oMask = 1 << (m_oStartBitLCLimitWarning);
	bool oActLCLimitWarning = p_oValue & oMask;

	if(oActLCLimitWarning != oOldLCLimitWarning)
	{
		wmLog(eDebug, "IncomingLCLimitWarning: Value: %d\n", oActLCLimitWarning);
		oOldLCLimitWarning = oActLCLimitWarning;
	}
}

void WeldingHeadControl::SetLCStartSignal(bool p_oOnOff)
{
	// Neuer Zustand m_oLCStartSignal-Var setzen
	m_oLCStartSignal = p_oOnOff;

	unsigned char oMask = 0;
	unsigned char oSendValue = 0;
	if(m_oLCStartSignal)
	{
		oSendValue = oSendValue | (1 << m_oLCStartSignalProxyInfo.nStartBit);
	}
	oMask = oMask | (1 << m_oLCStartSignalProxyInfo.nStartBit);

	wmLog(eDebug, "SetLCStartSignal: SendValue: %X Mask: %X\n", oSendValue, oMask);

    if (m_oLCStartSignalProxyInfo.m_oActive)
    {
        m_rEthercatOutputsProxy.ecatDigitalOut(m_oLCStartSignalProxyInfo.m_oProductIndex,
                                               m_oLCStartSignalProxyInfo.m_oInstance,
                                               (uint8_t) oSendValue,
                                               (uint8_t) oMask);
    }
}

void WeldingHeadControl::SetLCPowerOffset(int p_oValue) // Interface: viWeldHeadSubscribe (event)
{
	unsigned short oOffsetValue;
	if (p_oValue < 0)
	{
		oOffsetValue = 0;
	}
	else if (p_oValue > 10000)
	{
		oOffsetValue = 10000;
	}
	else
	{
		oOffsetValue = static_cast<unsigned short>(p_oValue);
	}

	//folgendes bei EL4102
	//10V -> 32768
	//5V -> 16384
	//0V -> 0
	unsigned short oBinValueUShort = static_cast<unsigned short>(3.2768 * oOffsetValue);
	if (oBinValueUShort > 0x7FFF) oBinValueUShort = 0x7FFF;
	wmLog(eDebug, "SetLCPowerOffset: Value: %d (%d)\n", oOffsetValue, oBinValueUShort);

    if (m_oLCPowerOffsetProxyInfo.m_oActive)
    {
        m_rEthercatOutputsProxy.ecatAnalogOut(m_oLCPowerOffsetProxyInfo.m_oProductIndex,
                                              m_oLCPowerOffsetProxyInfo.m_oInstance,
                                              m_oLCPowerOffsetProxyInfo.m_oChannel,
                                              (uint16_t) oBinValueUShort);
    }
}

void WeldingHeadControl::SetGenPurposeAnaOut(OutputID p_oOutputNo, int p_oValue) // Interface: viWeldHeadSubscribe (event)
{
	// Der Parameter p_oValue enthaelt den Volt-Wert (-10V bis +10V) als mikroVolt in einer Integer Variable

	//folgendes bei EL4132
	//10V -> 32767
	//0V -> 0
	//-10V -> -32768

	// Analog-Ausgang kann maximal +- 10 Volt verarbeiten
	if(p_oValue > 10000) // als uVolt
	{
		wmLogTr( eError, "QnxMsg.VI.AnaOutUpLimVolt", "General Purpose Analog Output [%d]: Value at upper boundary limited ! (%f Volt)\n", p_oOutputNo, 10.0);
		p_oValue = 10000;
	}
	if(p_oValue < -10000) // als uVolt
	{
		wmLogTr( eError, "QnxMsg.VI.AnaOutLowLimVolt", "General Purpose Analog Output [%d]: Value at lower boundary limited ! (%f Volt)\n", p_oOutputNo, -10.0);
		p_oValue = -10000;
	}

	// Umrechnung Volt in Binaerwert fuer Klemme
	unsigned short oBinValueUShort = static_cast<unsigned short>(p_oValue * 3.2767);
	if (p_oValue > 1000) // wir sind im hoeheren positiven Bereich
	{
		if (oBinValueUShort > 0x7FFF) oBinValueUShort = 0x7FFF; // unbedingt Sprung in negativen Bereich verhindern !
	}
	if (p_oValue < -1000) // wir sind im tieferen negativen Bereich
	{
		if (oBinValueUShort < 0x8000) oBinValueUShort = 0x8000; // unbedingt Sprung in positiven Bereich verhindern !
	}

	wmLog(eDebug, "SetGenPurposeAnaOut[%d]: Volt: %f , Binary: %d, 0x%x\n", p_oOutputNo, static_cast<float>(p_oValue / 1000.0), oBinValueUShort, oBinValueUShort);

    AnalogOutputNo oAnalogOutput;
    switch(p_oOutputNo)
    {
        case eOutput1:
            oAnalogOutput = eAnalogOut1;
            break;
        case eOutput2:
            oAnalogOutput = eAnalogOut2;
            break;
        case eOutput3:
            oAnalogOutput = eAnalogOut3;
            break;
        case eOutput4:
            oAnalogOutput = eAnalogOut4;
            break;
        case eOutput5:
            oAnalogOutput = eAnalogOut5;
            break;
        case eOutput6:
            oAnalogOutput = eAnalogOut6;
            break;
        case eOutput7:
            oAnalogOutput = eAnalogOut7;
            break;
        case eOutput8:
            oAnalogOutput = eAnalogOut8;
            break;
        default:
            wmLogTr(eError, "QnxMsg.VI.AnaOutWrongOut", "analog output number does not fit to possible analog outputs\n");
            oAnalogOutput = eAnalogOut1;
            break;
    }

    if (m_oGenPurposeAnaOutProxyInfo[oAnalogOutput].m_oActive)
    {
        m_rEthercatOutputsProxy.ecatAnalogOut(m_oGenPurposeAnaOutProxyInfo[oAnalogOutput].m_oProductIndex,
                                              m_oGenPurposeAnaOutProxyInfo[oAnalogOutput].m_oInstance,
                                              m_oGenPurposeAnaOutProxyInfo[oAnalogOutput].m_oChannel,
                                              (uint16_t) oBinValueUShort);
    }
}

void WeldingHeadControl::SetLWM40_No1_AmpPlasma(int p_oValue)
{
    // on KeyValue side: value must be between 0 and 6
    if (p_oValue < 0)
    {
        p_oValue = 0;
    }
    if (p_oValue > 6)
    {
        p_oValue = 6;
    }

    // on LWM 4.0 side: value must be between 1 and 7
    m_oLWM40_1_AmpPlasma = p_oValue + 1;

    EcatFRONTENDOutput oFrontendOutput;
    oFrontendOutput.m_oAmpCH1 = m_oLWM40_1_AmpPlasma;
    oFrontendOutput.m_oAmpCH2 = m_oLWM40_1_AmpTemperature;
    oFrontendOutput.m_oAmpCH3 = m_oLWM40_1_AmpBackReflection;
    oFrontendOutput.m_oAmpCH4 = m_oLWM40_1_AmpAnalogInput;
    oFrontendOutput.m_oOversampling = 50;

    m_rEthercatOutputsProxy.ecatFRONTENDOut(eProductIndex_Frontend, eInstance1, oFrontendOutput);
}

void WeldingHeadControl::SetLWM40_No1_AmpTemperature(int p_oValue)
{
    // on KeyValue side: value must be between 0 and 6
    if (p_oValue < 0)
    {
        p_oValue = 0;
    }
    if (p_oValue > 6)
    {
        p_oValue = 6;
    }

    // on LWM 4.0 side: value must be between 1 and 7
    m_oLWM40_1_AmpTemperature = p_oValue + 1;

    EcatFRONTENDOutput oFrontendOutput;
    oFrontendOutput.m_oAmpCH1 = m_oLWM40_1_AmpPlasma;
    oFrontendOutput.m_oAmpCH2 = m_oLWM40_1_AmpTemperature;
    oFrontendOutput.m_oAmpCH3 = m_oLWM40_1_AmpBackReflection;
    oFrontendOutput.m_oAmpCH4 = m_oLWM40_1_AmpAnalogInput;
    oFrontendOutput.m_oOversampling = 50;

    m_rEthercatOutputsProxy.ecatFRONTENDOut(eProductIndex_Frontend, eInstance1, oFrontendOutput);
}

void WeldingHeadControl::SetLWM40_No1_AmpBackReflection(int p_oValue)
{
    // on KeyValue side: value must be between 0 and 6
    if (p_oValue < 0)
    {
        p_oValue = 0;
    }
    if (p_oValue > 6)
    {
        p_oValue = 6;
    }

    // on LWM 4.0 side: value must be between 1 and 7
    m_oLWM40_1_AmpBackReflection = p_oValue + 1;

    EcatFRONTENDOutput oFrontendOutput;
    oFrontendOutput.m_oAmpCH1 = m_oLWM40_1_AmpPlasma;
    oFrontendOutput.m_oAmpCH2 = m_oLWM40_1_AmpTemperature;
    oFrontendOutput.m_oAmpCH3 = m_oLWM40_1_AmpBackReflection;
    oFrontendOutput.m_oAmpCH4 = m_oLWM40_1_AmpAnalogInput;
    oFrontendOutput.m_oOversampling = 50;

    m_rEthercatOutputsProxy.ecatFRONTENDOut(eProductIndex_Frontend, eInstance1, oFrontendOutput);
}

void WeldingHeadControl::SetLWM40_No1_AmpAnalogInput(int p_oValue)
{
    // on KeyValue side: value must be between 0 and 6
    if (p_oValue < 0)
    {
        p_oValue = 0;
    }
    if (p_oValue > 6)
    {
        p_oValue = 6;
    }

    // on LWM 4.0 side: value must be between 1 and 7
    m_oLWM40_1_AmpAnalogInput = p_oValue + 1;

    EcatFRONTENDOutput oFrontendOutput;
    oFrontendOutput.m_oAmpCH1 = m_oLWM40_1_AmpPlasma;
    oFrontendOutput.m_oAmpCH2 = m_oLWM40_1_AmpTemperature;
    oFrontendOutput.m_oAmpCH3 = m_oLWM40_1_AmpBackReflection;
    oFrontendOutput.m_oAmpCH4 = m_oLWM40_1_AmpAnalogInput;
    oFrontendOutput.m_oOversampling = 50;

    m_rEthercatOutputsProxy.ecatFRONTENDOut(eProductIndex_Frontend, eInstance1, oFrontendOutput);
}

int WeldingHeadControl::GetLWM40_No1_AmpPlasma(void)
{
    // on LWM 4.0 side: value must be between 1 and 7
    // on KeyValue side: value must be between 0 and 6
    return (m_oLWM40_1_AmpPlasma - 1);
}

int WeldingHeadControl::GetLWM40_No1_AmpTemperature(void)
{
    // on LWM 4.0 side: value must be between 1 and 7
    // on KeyValue side: value must be between 0 and 6
    return (m_oLWM40_1_AmpTemperature - 1);
}

int WeldingHeadControl::GetLWM40_No1_AmpBackReflection(void)
{
    // on LWM 4.0 side: value must be between 1 and 7
    // on KeyValue side: value must be between 0 and 6
    return (m_oLWM40_1_AmpBackReflection - 1);
}

int WeldingHeadControl::GetLWM40_No1_AmpAnalogInput(void)
{
    // on LWM 4.0 side: value must be between 1 and 7
    // on KeyValue side: value must be between 0 and 6
    return (m_oLWM40_1_AmpAnalogInput - 1);
}

void WeldingHeadControl::ScanmasterResult(ScanmasterResultType p_oResultType, ResultDoubleArray const& p_rScanmasterResult) // Interface: viWeldHeadSubscribe (event)
{
    if (m_pScanlab != nullptr)
    {
        if (p_oResultType == eScanmasterSeamWelding || p_oResultType == eScanmasterSeamWeldingAndEndOfSeamMarker)
        {
            if (!m_liveModeIsOnAtBurst)
            {
                m_pScanlab->ScanmasterWeldingData(p_rScanmasterResult, p_oResultType == eScanmasterSeamWeldingAndEndOfSeamMarker);
            }
        }
        else if (p_oResultType == eScanmasterScannerMoving)
        {
            m_pScanlab->ScanmasterScannerMoving(p_rScanmasterResult);
        }
        else if (p_oResultType == eScanmasterSpotWelding)
        {
            if (!m_liveModeIsOnAtBurst)
            {
                m_pScanlab->ScanmasterScannerSpotWelding(p_rScanmasterResult);
            }
        }
        else if (p_oResultType == eScanmasterPrepareContour)
        {
            m_pScanlab->ScanmasterPrepareList(p_rScanmasterResult);
        }
        else if (p_oResultType == eScanmasterWeldPreparedContour)
        {
            if (!m_liveModeIsOnAtBurst)
            {
                m_pScanlab->ScanmasterWeldPreparedList(p_rScanmasterResult, p_oResultType == eScanmasterWeldPreparedContour);
            }
        }
        else
        {
            // should never happen
            wmLog(eDebug, "wrong ResultType for ScanmasterResult\n");
        }
    }
}

void WeldingHeadControl::ScanmasterHeight(double height)
{
    if (m_pScanlab != nullptr)
    {
        m_pScanlab->ScannerHeightCompensation(height);
    }
}

bool WeldingHeadControl::getLEDEnable(LEDPanelNo ledPanel)
{
    return getLEDPanelOnOff(ledPanel);
}

bool WeldingHeadControl::setLEDEnable(LEDPanelNo ledPanel, bool enabled)
{
    setLEDPanelOnOff(ledPanel, enabled);
    SetLEDPanelIntensity();

    return true;
}

void WeldingHeadControl::SetLineLaser1Intensity(void)
{
	if(m_oLineLaser1Intensity > 100) m_oLineLaser1Intensity = 100;
	if(m_oLineLaser1Intensity < 0) m_oLineLaser1Intensity = 0;

	unsigned short oIntensityValue;

	if (m_oLineLaser1OnOff) // Linienlaser ist eingeschaltet
	{
		oIntensityValue = static_cast<unsigned short>(m_oLineLaser1Intensity);
	}
	else // Linienlaser ist ausgeschaltet
	{
		oIntensityValue = 0; // Intensitaet abschalten
	}

	m_oLineLas1IntensToWin = m_oLineLaser1Intensity;
	m_oLineLas1OnOffToWin = m_oLineLaser1OnOff;

	if (!isLineLaser1OutputViaCamera())
	{
		//10V -> 32768
		//100 Prozent == 5V
		//5V -> 16384
		unsigned short oBinValueUShort = 163.84 * oIntensityValue;

		wmLog(eDebug, "SetLineLaser1Intensity: Value: %d%% (%d)\n", oIntensityValue, oBinValueUShort);

        if (m_oLineLaser1IntensityProxyInfo.m_oActive)
        {
            m_rEthercatOutputsProxy.ecatAnalogOut(m_oLineLaser1IntensityProxyInfo.m_oProductIndex,
                                                  m_oLineLaser1IntensityProxyInfo.m_oInstance,
                                                  m_oLineLaser1IntensityProxyInfo.m_oChannel,
                                                  (uint16_t) oBinValueUShort);
        }
    }
}

void WeldingHeadControl::SetLineLaser2Intensity(void)
{
	if(m_oLineLaser2Intensity > 100) m_oLineLaser2Intensity = 100;
	if(m_oLineLaser2Intensity < 0) m_oLineLaser2Intensity = 0;

	unsigned short oIntensityValue;

	if (m_oLineLaser2OnOff) // Linienlaser ist eingeschaltet
	{
		oIntensityValue = static_cast<unsigned short>(m_oLineLaser2Intensity);
	}
	else // Linienlaser ist ausgeschaltet
	{
		oIntensityValue = 0; // Intensitaet abschalten
	}

	m_oLineLas2IntensToWin = m_oLineLaser2Intensity;
	m_oLineLas2OnOffToWin = m_oLineLaser2OnOff;

	if (!isLineLaser2OutputViaCamera())
	{
		//10V -> 32768
		//100 Prozent == 5V
		//5V -> 16384
		unsigned short oBinValueUShort = 163.84 * oIntensityValue;

		wmLog(eDebug, "SetLineLaser2Intensity: Value: %d%% (%d)\n", oIntensityValue, oBinValueUShort);

        if (m_oLineLaser2IntensityProxyInfo.m_oActive)
        {
            if (m_oLineLaser2IntensityProxyInfo.nProductCode == PRODUCTCODE_EL2008)
            {
                // this is true with SOUVIS6x00 and a non dimmable line laser 1
                // in this case the VI_Config.xml should carry the product code for the EL2008 digital out terminal for line laser 2
                uint8_t sendValue{0x00};
                uint8_t mask{0x00};
                switch (m_oLineLaser2IntensityProxyInfo.m_oChannel)
                {
                    case eChannel1:
                        sendValue = 0x01;
                        mask = 0x01;
                        break;
                    case eChannel2:
                        sendValue = 0x02;
                        mask = 0x02;
                        break;
                    default:
                        sendValue = 0x01;
                        mask = 0x01;
                        break;
                }
                if (!m_oLineLaser2OnOff)
                {
                    sendValue = 0x00;
                }
                m_rEthercatOutputsProxy.ecatDigitalOut(eProductIndex_EL2008,
                                                       m_oLineLaser2IntensityProxyInfo.m_oInstance,
                                                       sendValue,
                                                       mask);
            }
            else
            {
                m_rEthercatOutputsProxy.ecatAnalogOut(m_oLineLaser2IntensityProxyInfo.m_oProductIndex,
                                                    m_oLineLaser2IntensityProxyInfo.m_oInstance,
                                                    m_oLineLaser2IntensityProxyInfo.m_oChannel,
                                                    (uint16_t) oBinValueUShort);
            }
        }
	}
}

void WeldingHeadControl::SetFieldLight1Intensity(void)
{
	if(m_oFieldLight1Intensity > 100) m_oFieldLight1Intensity = 100;
	if(m_oFieldLight1Intensity < 0) m_oFieldLight1Intensity = 0;

	unsigned short oIntensityValue;

	if (m_oFieldLight1OnOff) // Feldbeleuchtung ist eingeschaltet
	{
		oIntensityValue = static_cast<unsigned short>(m_oFieldLight1Intensity);
	}
	else // Feldbeleuchtung ist ausgeschaltet
	{
		oIntensityValue = 0; // Intensitaet abschalten
	}

	m_oFieldL1IntensToWin = m_oFieldLight1Intensity;
	m_oFieldL1OnOffToWin = m_oFieldLight1OnOff;

	if (!isFieldLight1OutputViaCamera())
	{
#if 0
        // Originally, the output was set to the range of 0V to 3.3V
		//10V -> 32768
		//100 Prozent == 3.3V
		//3.3V -> 10814
		unsigned short oBinValueUShort = 108.14 * oIntensityValue;
#endif
        // new range should be from 0V to 5V (like the line lasers) Axel Hatwig at Daily Scrum 2.10.2019
		//10V -> 32768
		//100 Prozent == 5V
		//5V -> 16384
		unsigned short oBinValueUShort = 163.84 * oIntensityValue;

		wmLog(eDebug, "SetFieldLight1Intensity: Value: %d%% (%d)\n", oIntensityValue, oBinValueUShort);

        if (m_oFieldLight1IntensityProxyInfo.m_oActive)
        {
            m_rEthercatOutputsProxy.ecatAnalogOut(m_oFieldLight1IntensityProxyInfo.m_oProductIndex,
                                                  m_oFieldLight1IntensityProxyInfo.m_oInstance,
                                                  m_oFieldLight1IntensityProxyInfo.m_oChannel,
                                                  (uint16_t) oBinValueUShort);
        }
	}
}

void WeldingHeadControl::setLEDPanelIntensity(LEDPanelNo p_oPanelNo, int p_oIntensity)
{
    m_oLEDI_Parameters[p_oPanelNo].led_brightness = p_oIntensity;
}

void WeldingHeadControl::setLEDPanelOnOff(LEDPanelNo p_oPanelNo, bool p_oOnOff)
{
    m_oLEDI_Parameters[p_oPanelNo].led_enable = p_oOnOff;
}

void WeldingHeadControl::SetLEDPanelIntensity(void)
{
	int oIntensityValue[ANZ_LEDI_PARAMETERS];

	wmLog(eDebug, "SetLEDPanelIntensity\n");
	for(int i = 0;i < ANZ_LEDI_PARAMETERS;i++)
	{
		if(m_oLEDI_Parameters[i].led_brightness > 100) m_oLEDI_Parameters[i].led_brightness = 100;
		if(m_oLEDI_Parameters[i].led_brightness < 0) m_oLEDI_Parameters[i].led_brightness = 0;

		if (m_oLEDI_Parameters[i].led_enable) // LED Panel ist eingeschaltet
		{
			oIntensityValue[i] = m_oLEDI_Parameters[i].led_brightness;
		}
		else // LED Panel ist ausgeschaltet
		{
			oIntensityValue[i] = 0; // Intensitaet abschalten
		}

		// TODO: Neue Parameter an wmMain senden
		//m_oLineLas1IntensToWin = oIntensityValue[i]; ?????
		//m_oLineLas1OnOffToWin = m_oLineLaser1OnOff;
	}

	char oLogStrg1[81];
	char oLogStrg2[81];
	sprintf(oLogStrg1, "SetLEDPanelIntensity1: Value: ");
	sprintf(oLogStrg2, "%3d %3d %3d %3d", oIntensityValue[0], oIntensityValue[1], oIntensityValue[2], oIntensityValue[3]);
	wmLog(eDebug, "%s%s\n", oLogStrg1, oLogStrg2);
	sprintf(oLogStrg1, "SetLEDPanelIntensity2: Value: ");
	sprintf(oLogStrg2, "%3d %3d %3d %3d", oIntensityValue[4], oIntensityValue[5], oIntensityValue[6], oIntensityValue[7]);
	wmLog(eDebug, "%s%s\n", oLogStrg1, oLogStrg2);
}

void WeldingHeadControl::setLEDPanelPulseWidth(LEDPanelNo p_oPanelNo, int p_oPulseWidth)
{
    if (m_oLEDControllerType == eLEDTypePP520)
    {
        if ((p_oPulseWidth % 20) == 0)
        {
            // nothing to do, p_oPulseWidth is a multiple of 20us
        }
        else if ((p_oPulseWidth % 20) <= 10)
        {
            // round down p_oPulseWidth to the nearest lower multiple of 20us
            int oTemp = p_oPulseWidth / 20;
            p_oPulseWidth = oTemp * 20;
        }
        else if ((p_oPulseWidth % 20) > 10)
        {
            // round up p_oPulseWidth to the nearest upper multiple of 20us
            int oTemp = p_oPulseWidth / 20;
            p_oPulseWidth = (oTemp + 1) * 20;
        }
    }
	m_oLEDI_Parameters[p_oPanelNo].led_pulse_width = p_oPulseWidth;
}

void WeldingHeadControl::SetLEDPanelPulseWidth(void)
{
	wmLog(eDebug, "SetLEDPanelPulseWidth\n");
	for(int i = 0;i < ANZ_LEDI_PARAMETERS;i++)
	{
		if(m_oLEDI_Parameters[i].led_pulse_width > 400) m_oLEDI_Parameters[i].led_pulse_width = 400;
		if(m_oLEDI_Parameters[i].led_pulse_width < 40) m_oLEDI_Parameters[i].led_pulse_width = 40;

		// TODO: Neue Parameter an wmMain senden
		//m_oLineLas1IntensToWin = oIntensityValue[i]; ?????
		//m_oLineLas1OnOffToWin = m_oLineLaser1OnOff;
	}

    char oLogStrg1[121];
    char oLogStrg2[121];
    sprintf(oLogStrg1, "SetLEDPanelPulseWidth1: Value: ");
    sprintf(oLogStrg2, "%3d %3d %3d %3d",
            m_oLEDI_Parameters[0].led_pulse_width, m_oLEDI_Parameters[1].led_pulse_width,
            m_oLEDI_Parameters[2].led_pulse_width, m_oLEDI_Parameters[3].led_pulse_width);
    wmLog(eDebug, "%s%s\n", oLogStrg1, oLogStrg2);
    sprintf(oLogStrg1, "SetLEDPanelPulseWidth1: Value: ");
    sprintf(oLogStrg2, "%3d %3d %3d %3d",
            m_oLEDI_Parameters[4].led_pulse_width, m_oLEDI_Parameters[5].led_pulse_width,
            m_oLEDI_Parameters[6].led_pulse_width, m_oLEDI_Parameters[7].led_pulse_width);
    wmLog(eDebug, "%s%s\n", oLogStrg1, oLogStrg2);
}

void WeldingHeadControl::sendLEDData()
{
    SetLEDPanelIntensity();
    SetLEDPanelPulseWidth();
#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE
	if ( isLED_IlluminationEnabled() )
	{
		if (m_oLED_DriverIsOperational)
		{
			LEDI_ParametersT *inpars;
			inpars = const_cast<LEDI_ParametersT *>(m_oLEDI_Parameters);
			if (isSOUVIS6000_Application())
			{
				m_oLEDdriver.updateall_in_once(inpars);
			}
			else
			{
				m_oLEDdriver.updateall(inpars);
			}
			m_oLED_ParameterChanged = true;
		}
	}
#endif
}

void WeldingHeadControl::LED_SaveIntensityPersistent(void)
{
	wmLog(eDebug, "LED_SaveIntensityPersistent\n");

	if (!m_oCycleIsOn) //Speichern nur wenn kein Zyklus aktiv ist, d.h. es handelt sich nicht um Parameter aus einem HW-Parametersatz
	{
        WeldHeadDefaults::instance().setInt("LEDPanel1Intensity", m_oLEDI_Parameters[LED_PANEL_1].led_brightness);
        WeldHeadDefaults::instance().setInt("LEDPanel2Intensity", m_oLEDI_Parameters[LED_PANEL_2].led_brightness);
        WeldHeadDefaults::instance().setInt("LEDPanel3Intensity", m_oLEDI_Parameters[LED_PANEL_3].led_brightness);
        WeldHeadDefaults::instance().setInt("LEDPanel4Intensity", m_oLEDI_Parameters[LED_PANEL_4].led_brightness);
        WeldHeadDefaults::instance().setInt("LEDPanel5Intensity", m_oLEDI_Parameters[LED_PANEL_5].led_brightness);
        WeldHeadDefaults::instance().setInt("LEDPanel6Intensity", m_oLEDI_Parameters[LED_PANEL_6].led_brightness);
        WeldHeadDefaults::instance().setInt("LEDPanel7Intensity", m_oLEDI_Parameters[LED_PANEL_7].led_brightness);
        WeldHeadDefaults::instance().setInt("LEDPanel8Intensity", m_oLEDI_Parameters[LED_PANEL_8].led_brightness);
        WeldHeadDefaults::instance().setBool("LEDPanel1OnOff", m_oLEDI_Parameters[LED_PANEL_1].led_enable);
        WeldHeadDefaults::instance().setBool("LEDPanel2OnOff", m_oLEDI_Parameters[LED_PANEL_2].led_enable);
        WeldHeadDefaults::instance().setBool("LEDPanel3OnOff", m_oLEDI_Parameters[LED_PANEL_3].led_enable);
        WeldHeadDefaults::instance().setBool("LEDPanel4OnOff", m_oLEDI_Parameters[LED_PANEL_4].led_enable);
        WeldHeadDefaults::instance().setBool("LEDPanel5OnOff", m_oLEDI_Parameters[LED_PANEL_5].led_enable);
        WeldHeadDefaults::instance().setBool("LEDPanel6OnOff", m_oLEDI_Parameters[LED_PANEL_6].led_enable);
        WeldHeadDefaults::instance().setBool("LEDPanel7OnOff", m_oLEDI_Parameters[LED_PANEL_7].led_enable);
        WeldHeadDefaults::instance().setBool("LEDPanel8OnOff", m_oLEDI_Parameters[LED_PANEL_8].led_enable);
        WeldHeadDefaults::instance().setInt("LEDPanel1PulseWidth", m_oLEDI_Parameters[LED_PANEL_1].led_pulse_width);
        WeldHeadDefaults::instance().setInt("LEDPanel2PulseWidth", m_oLEDI_Parameters[LED_PANEL_2].led_pulse_width);
        WeldHeadDefaults::instance().setInt("LEDPanel3PulseWidth", m_oLEDI_Parameters[LED_PANEL_3].led_pulse_width);
        WeldHeadDefaults::instance().setInt("LEDPanel4PulseWidth", m_oLEDI_Parameters[LED_PANEL_4].led_pulse_width);
        WeldHeadDefaults::instance().setInt("LEDPanel5PulseWidth", m_oLEDI_Parameters[LED_PANEL_5].led_pulse_width);
        WeldHeadDefaults::instance().setInt("LEDPanel6PulseWidth", m_oLEDI_Parameters[LED_PANEL_6].led_pulse_width);
        WeldHeadDefaults::instance().setInt("LEDPanel7PulseWidth", m_oLEDI_Parameters[LED_PANEL_7].led_pulse_width);
        WeldHeadDefaults::instance().setInt("LEDPanel8PulseWidth", m_oLEDI_Parameters[LED_PANEL_8].led_pulse_width);
	}

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE
	if ( isLED_IlluminationEnabled() )
	{
		if (m_oLED_DriverIsOperational)
		{
			m_oLEDdriver.writeSetupPersistent();
		}
	}
#endif
}

void WeldingHeadControl::IncomingLEDTemperatureHigh(unsigned char p_oValue)
{
    static bool oOldLEDTemperatureHigh = true; // signal is low active, high state is ok state
    static uint32_t oSignalIsStable{0};
    static bool oSignalIsActive{false};
    const uint32_t oDebouncingTime{100}; // time is in ms, triggered via EtherCAT cycle

    unsigned char oMask = 0;
    oMask = 1 << m_oStartBitLEDTemperatureHigh;
    bool oActLEDTemperatureHigh = p_oValue & oMask;

    if (oActLEDTemperatureHigh != oOldLEDTemperatureHigh) // signal status has changed
    {
        wmLog(eDebug, "IncomingLEDTemperatureHigh: Value: %d\n", oActLEDTemperatureHigh);
        oOldLEDTemperatureHigh = oActLEDTemperatureHigh;
        oSignalIsStable = 0;
        oSignalIsActive = !oActLEDTemperatureHigh; // signal is low active
    }
    else
    {
        oSignalIsStable++;
    }

    if (oSignalIsActive && (oSignalIsStable >= oDebouncingTime))
    {
        wmFatal(eLighting, "QnxMsg.VI.OverTempLEDContr", "Temperature of LED controller too high !\n");
        oSignalIsActive = false;
    }
}

void WeldingHeadControl::SetFocalLength(int p_oFocalLength)
{
	if (p_oFocalLength != m_oFocalLength)
	{
		WeldHeadDefaults::instance().setInt("ScanTrackerFocalLength", p_oFocalLength);
	}
	m_oFocalLength = p_oFocalLength;
	CalculateFocalLengthDependencies(); // anhand der neuen Brennweite die Grenzen neu berechnen

	// Nach Aenderung der Brennweite muessen Scanbreite und Scanposition auf Gueltigkeit ueberprueft werden
	// Ausserdem werden neue Kennlinien aktiviert
	SetTrackerScanPos(false);
	SetTrackerScanWidth(false);
}

void WeldingHeadControl::SetTrackerFrequencyStep(TrackerFrequencyStep p_oFreq)
{
	m_oTrackerFrequencyStep = p_oFreq;
	if (m_pSerialToTracker != NULL)
	{
		m_pSerialToTracker->setTrackerFrequencyStep(m_oTrackerFrequencyStep);
	}
	switch (m_oTrackerFrequencyStep)
	{
		case eFreq30:
			m_oTrackerFrequencyBoth = 30;
			break;
		case eFreq40:
			m_oTrackerFrequencyBoth = 40;
			break;
		case eFreq50:
			m_oTrackerFrequencyBoth = 50;
			break;
		case eFreq100:
			m_oTrackerFrequencyBoth = 100;
			break;
		case eFreq150:
			m_oTrackerFrequencyBoth = 150;
			break;
		case eFreq200:
			m_oTrackerFrequencyBoth = 200;
			break;
		case eFreq250:
			m_oTrackerFrequencyBoth = 250;
			break;
		case eFreq300:
			m_oTrackerFrequencyBoth = 300;
			break;
		case eFreq350:
			m_oTrackerFrequencyBoth = 350;
			break;
		case eFreq400:
			m_oTrackerFrequencyBoth = 400;
			break;
		case eFreq450:
			m_oTrackerFrequencyBoth = 450;
			break;
		case eFreq500:
			m_oTrackerFrequencyBoth = 500;
			break;
		case eFreq550:
			m_oTrackerFrequencyBoth = 550;
			break;
		case eFreq600:
			m_oTrackerFrequencyBoth = 600;
			break;
		case eFreq650:
			m_oTrackerFrequencyBoth = 650;
			break;
		case eFreq700:
			m_oTrackerFrequencyBoth = 700;
			break;
		case eFreq750:
			m_oTrackerFrequencyBoth = 750;
			break;
		default:
			m_oTrackerFrequencyBoth = 30;
			break;
	}
	// Nach Aenderung der Frequenz muss Scanbreite auf Gueltigkeit ueberprueft werden
	// Ausserdem wird eine neue Kennlinie aktiviert
	SetTrackerScanWidth(false);
}

void WeldingHeadControl::SetTrackerFrequencyCont(unsigned short p_oFreq)
{
	m_oTrackerFrequencyCont = p_oFreq;
	if (m_pSerialToTracker != NULL)
	{
		m_pSerialToTracker->setTrackerFrequencyCont(m_oTrackerFrequencyCont);
	}
	m_oTrackerFrequencyBoth = m_oTrackerFrequencyCont;
	// Nach Aenderung der Frequenz muss Scanbreite auf Gueltigkeit ueberprueft werden
	// Ausserdem wird eine neue Kennlinie aktiviert
	SetTrackerScanWidth(false);
}

void WeldingHeadControl::SetTrackerScanWidth(bool p_oFailSafeRequest)
{
	unsigned short oNewScanWidth;

	if (!m_oScanWidthOutOfGapWidth) // Es soll eine feste Scanbreite verwendet werden
	{
		oNewScanWidth = static_cast<unsigned short>(m_oScanWidthFixed);
	}
	else
	{
		oNewScanWidth = static_cast<unsigned short>(m_oScanWidthControlled);
	}
	if ((p_oFailSafeRequest) && (!m_oTrackerExpertMode))
	{
		oNewScanWidth = 0; // im Fehlerfall alles auf 0 setzen !
	}
	if ((!m_oSeamIsOn) && (!m_oTrackerExpertMode))
	{
		oNewScanWidth = 0; // wenn keine Naht alles auf 0 setzen !
	}

	// Search for the first entry in the table that is larger than the current frequency
	int oArrayIndex = 0;
	for(int i = 0;i < MAX_TRACKER_CALIB_DATA;i++)
	{
		if (static_cast<int>(m_oTrackerFrequencyBoth) < m_oTrackerCalibArray[i].m_oFrequency)
		{
			oArrayIndex = i;
			break;
		}
	}
	// avoid underflow of oArrayIndex (does not change the result)
	if (m_oTrackerFrequencyBoth < 30)
	{
		oArrayIndex = 1;
	}
	// Calculate the interpolation factor
	double oHelpDouble1 = m_oTrackerCalibArray[oArrayIndex].m_oFrequency - m_oTrackerCalibArray[oArrayIndex - 1].m_oFrequency;
	double oHelpDouble2 = static_cast<int>(m_oTrackerFrequencyBoth) - m_oTrackerCalibArray[oArrayIndex - 1].m_oFrequency;
	double oFactor = oHelpDouble2 / oHelpDouble1;
	// avoid negative factor (does not change the result)
	if (m_oTrackerFrequencyBoth < 30)
	{
		oFactor = 0.0;
	}
	// Calculate the interpolated oMaxWidth
	int oHelpInt1 = m_oTrackerCalibArray[oArrayIndex].m_oMaxWidth - m_oTrackerCalibArray[oArrayIndex - 1].m_oMaxWidth;
	int oHelpInt2 = static_cast<int>(static_cast<double>(oHelpInt1) * oFactor);
	int oMaxWidth =  m_oTrackerCalibArray[oArrayIndex - 1].m_oMaxWidth + oHelpInt2;
	// Calculate the interpolated oGradient
	oHelpDouble1 = m_oTrackerCalibArray[oArrayIndex].m_oGradient - m_oTrackerCalibArray[oArrayIndex - 1].m_oGradient;
	oHelpDouble2 = oHelpDouble1 * oFactor;
	double oGradient =  m_oTrackerCalibArray[oArrayIndex - 1].m_oGradient + oHelpDouble2;
	// Calculate the interpolated oMaxVolt
	oHelpDouble1 = m_oTrackerCalibArray[oArrayIndex].m_oMaxVolt - m_oTrackerCalibArray[oArrayIndex - 1].m_oMaxVolt;
	oHelpDouble2 = oHelpDouble1 * oFactor;
	double oMaxVolt =  m_oTrackerCalibArray[oArrayIndex - 1].m_oMaxVolt + oHelpDouble2;

	// Messwert fuer die Spaltbreite kommt in Micro-Metern an
	// Scantracker kann frequenzabhaengig maximal xx mm Amplitude verarbeiten
	if(oNewScanWidth > oMaxWidth)
	{
		wmLogTr( eError, "QnxMsg.VI.ScanWidthUpLimMM", "ScanTracker: ScanWidth at upper boundary limited ! (%f mm)\n",
				static_cast<double>(oMaxWidth) / 1000.0);
		oNewScanWidth = static_cast<unsigned short>(oMaxWidth);
		m_oScanWidthLimitedUM = true;
	}
	else if(oNewScanWidth < 0)
	{
		wmLogTr( eError, "QnxMsg.VI.ScanWidthLowLimMM", "ScanTracker: ScanWidth at lower boundary limited ! (%f mm)\n", 0.0);
		oNewScanWidth = 0;
		m_oScanWidthLimitedUM = true;
	}
	else
	{
		m_oScanWidthLimitedUM = false;
	}

	// Scan-Amplitude auf maximal zulaessige Amplitude begrenzen
	if (m_oScanPosUMSent >= 0)
	{
		// positive Grenze ist massgebend
		if ((m_oScanPosUMSent + (oNewScanWidth / 2)) > m_oTrackerMaxAmplitude)
		{
			oNewScanWidth = (m_oTrackerMaxAmplitude - m_oScanPosUMSent) * 2;
			wmLogTr( eError, "QnxMsg.VI.ScanMaxAmplWidth", "ScanTracker: ScanWidth is limited due to max. scan amplitude ! (to %f mm)\n",
					(static_cast<float>(oNewScanWidth) / 1000.0));
		}
	}
	else
	{
		// neative Grenze ist massgebend
		if ((m_oScanPosUMSent - (oNewScanWidth / 2)) < (-m_oTrackerMaxAmplitude))
		{
			oNewScanWidth = (m_oTrackerMaxAmplitude + m_oScanPosUMSent) * 2;
			wmLogTr( eError, "QnxMsg.VI.ScanMaxAmplWidth", "ScanTracker: ScanWidth is limited due to max. scan amplitude ! (to %f mm)\n",
					(static_cast<float>(oNewScanWidth) / 1000.0));
		}
	}

	//10V -> 32767
	//0V -> 0
	//maxVolt -> xx um (frequenzabhaengig)
	//0V -> 0um

	// Umrechnung Breite (in um) in Volt
	// Variable oAktVolt muss volatile sein, sonst geht nachfolgender Vergleich schief !
	volatile double oAktVolt = static_cast<double>(oNewScanWidth) * oGradient;

	// Scantracker kann frequenzabhaengig maximal xx Volt Amplitude verarbeiten
	if(oAktVolt > oMaxVolt)
	{
		wmLogTr( eError, "QnxMsg.VI.ScanWidthUpLimVolt", "ScanTracker: ScanWidth at upper boundary limited ! (%f Volt)\n",
				oMaxVolt);
		oAktVolt = oMaxVolt;
		m_oScanWidthLimitedVolt = true;
	}
	else if(oAktVolt < 0)
	{
		wmLogTr( eError, "QnxMsg.VI.ScanWidthLowLimVolt", "ScanTracker: ScanWidth at lower boundary limited ! (%f Volt)\n", 0.0);
		oAktVolt = 0;
		m_oScanWidthLimitedVolt = true;
	}
	else
	{
		m_oScanWidthLimitedVolt = false;
	}

	// Umrechnung Volt in Binaerwert fuer Klemme
	unsigned short oBinValueUShort = static_cast<unsigned short>(oAktVolt * 3276.7);
	if (oBinValueUShort > 0x7FFF) oBinValueUShort = 0x7FFF;

	wmLog(eDebug, "SetTrackerScanWidth: ScanWidth: %d , Volt: %f , Binary: %d, 0x%x\n", oNewScanWidth, oAktVolt, oBinValueUShort, oBinValueUShort);

    if (m_oTrackerScanWidthProxyInfo.m_oActive)
    {
        m_rEthercatOutputsProxy.ecatAnalogOut(m_oTrackerScanWidthProxyInfo.m_oProductIndex,
                                              m_oTrackerScanWidthProxyInfo.m_oInstance,
                                              m_oTrackerScanWidthProxyInfo.m_oChannel,
                                              (uint16_t) oBinValueUShort);
    }

	// Werte fuer Anzeige in wmMain bereitstellen
	m_oScanWidthUMSent = oNewScanWidth;
	m_oScanWidthVoltSent = static_cast<int>(oAktVolt * 1000.0);
}

void WeldingHeadControl::SetScanWidthOutOfGapWidth(bool onOff)
{
    m_oScanWidthOutOfGapWidth = onOff;
    if (isScanTrackerEnabled())
    {
        SetTrackerScanWidth(false);
        if (!onOff) m_oScanWidthFixedWasSet = true;
    }
    else if (isScanlabScannerEnabled() && (getScannerGeneralMode() == ScannerGeneralMode::eScantracker2DMode))
    {
        if (m_pScanlab != nullptr)
        {
            m_pScanlab->SetScanWidthOutOfGapWidth(onOff);
        }
    }
}

void WeldingHeadControl::SetTrackerScanWidthFixed(int value)
{
	m_oScanWidthFixed = value;
	SetTrackerScanWidth(false);
	//m_oScanWidthFixedWasSet = true;
}

void WeldingHeadControl::SetTrackerScanWidthControlled(int value) // Interface: viWeldHeadSubscribe (event)
{
	if (value < 0)
	{
		value = 0; // Scanbreite darf nicht negativ werden
	}

	// Standard Regelfunktion -> Geradengleichung
	value = m_oGapWidthToScanWidthOffset + (m_oGapWidthToScanWidthGradient * value);

	m_oScanWidthControlled = value;
    if (isScanTrackerEnabled())
    {
        SetTrackerScanWidth(false);
    }
    else if (isScanlabScannerEnabled() && (getScannerGeneralMode() == ScannerGeneralMode::eScantracker2DMode))
    {
        if (m_pScanlab != nullptr)
        {
            m_pScanlab->SetTrackerScanWidthControlled(value);
        }
    }
}

void WeldingHeadControl::SetTrackerScanPos(bool p_oFailSafeRequest)
{
	short oNewScanPos;

	if (!m_oScanPosOutOfGapPos) // Es soll eine feste Scanposition verwendet werden
	{
		oNewScanPos = m_oScanPosFixed;
	}
	else
	{
		oNewScanPos = m_oScanPosControlled;
	}
	if ((p_oFailSafeRequest) && (!m_oTrackerExpertMode))
	{
		oNewScanPos = 0; // im Fehlerfall alles auf 0 setzen !
	}
	if ((!m_oSeamIsOn) && (!m_oTrackerExpertMode))
	{
		oNewScanPos = 0; // wenn keine Naht alles auf 0 setzen !
	}

	// Messwert fuer die Spaltposition kommt in Micro-Metern an
	// Scantracker kann maximal Offset von m_oMinScanPosUM (-6000um@250mm) bis m_oMaxScanPosUM (6000um@250mm) verarbeiten
	if(oNewScanPos > m_oMaxScanPosUM)
	{
		wmLogTr( eError, "QnxMsg.VI.ScanPosUpLimMM", "ScanTracker: ScanPosition at upper boundary limited ! (%f mm)\n", m_oMaxScanPosMM);
		oNewScanPos = m_oMaxScanPosUM;
		m_oScanPosLimitedUM = true;
	}
	else if(oNewScanPos < m_oMinScanPosUM)
	{
		wmLogTr( eError, "QnxMsg.VI.ScanPosLowLimMM", "ScanTracker: ScanPosition at lower boundary limited ! (%f mm)\n", m_oMinScanPosMM);
		oNewScanPos = m_oMinScanPosUM;
		m_oScanPosLimitedUM = true;
	}
	else
	{
		m_oScanPosLimitedUM = false;
	}

	// Scan-Position auf maximal zulaessige Amplitude begrenzen
	if (oNewScanPos > m_oTrackerMaxAmplitude)
	{
		oNewScanPos = m_oTrackerMaxAmplitude;
		wmLogTr( eError, "QnxMsg.VI.ScanMaxAmplPos", "ScanTracker: ScanPosition is limited due to max. scan amplitude ! (to %f mm)\n",
				(static_cast<float>(oNewScanPos) / 1000.0));
	}
	else if (oNewScanPos < (-m_oTrackerMaxAmplitude))
	{
		oNewScanPos = -m_oTrackerMaxAmplitude;
		wmLogTr( eError, "QnxMsg.VI.ScanMaxAmplPos", "ScanTracker: ScanPosition is limited due to max. scan amplitude ! (to %f mm)\n",
				(static_cast<float>(oNewScanPos) / 1000.0));
	}

	//10V -> 32767
	//0V -> 0
	//-10V -> -32768
	//10V -> m_oMaxScanPosUM (6000 um@250mm)
	//0V -> 0um
	//-10V -> m_oMinScanPosUM (-6000 um@250mm)

	// Umrechnung Position (in um) in Volt
	// Variable oAktVolt muss volatile sein, sonst geht nachfolgender Vergleich schief !
	volatile double oAktVolt = static_cast<double>(oNewScanPos) * (10.0 / static_cast<double>(m_oMaxScanPosUM));

	// Scantracker kann maximal +- 10 Volt Offset verarbeiten
	if(oAktVolt > 10.0)
	{
		wmLogTr( eError, "QnxMsg.VI.ScanPosUpLimVolt", "ScanTracker: ScanPosition at upper boundary limited ! (%f Volt)\n", 10.0);
		oAktVolt = 10.0;
		m_oScanPosLimitedVolt = true;
	}
	else if(oAktVolt < -10.0)
	{
		wmLogTr( eError, "QnxMsg.VI.ScanPosLowLimVolt", "ScanTracker: ScanPosition at lower boundary limited ! (%f Volt)\n", -10.0);
		oAktVolt = -10.0;
		m_oScanPosLimitedVolt = true;
	}
	else
	{
		m_oScanPosLimitedVolt = false;
	}

	// Umrechnung Volt in Binaerwert fuer Klemme
	unsigned short oBinValueUShort = static_cast<unsigned short>(oAktVolt * 3276.7);
	if (oNewScanPos > 1000) // wir sind im hoeheren positiven Bereich
	{
		if (oBinValueUShort > 0x7FFF) oBinValueUShort = 0x7FFF; // unbedingt Sprung in negativen Bereich verhindern !
	}
	if (oNewScanPos < -1000) // wir sind im tieferen negativen Bereich
	{
		if (oBinValueUShort < 0x8000) oBinValueUShort = 0x8000; // unbedingt Sprung in positiven Bereich verhindern !
	}

	wmLog(eDebug, "SetTrackerScanPos: ScanPos: %d , Volt: %f , Binary: %d, 0x%x\n", oNewScanPos, oAktVolt, oBinValueUShort, oBinValueUShort);

    if (m_oTrackerScanPosProxyInfo.m_oActive)
    {
        m_rEthercatOutputsProxy.ecatAnalogOut(m_oTrackerScanPosProxyInfo.m_oProductIndex,
                                              m_oTrackerScanPosProxyInfo.m_oInstance,
                                              m_oTrackerScanPosProxyInfo.m_oChannel,
                                              (uint16_t) oBinValueUShort);
    }

	// Werte fuer Anzeige in wmMain bereitstellen
	m_oScanPosUMSent = oNewScanPos;
	m_oScanPosVoltSent = static_cast<int>(oAktVolt * 1000.0);

	// fuer Pruefung auf Ueberschreitung m_oTrackerMaxAmplitude muss hier noch die Scanbreite neu augegeben werden
	SetTrackerScanWidth(p_oFailSafeRequest);
}

void WeldingHeadControl::SetScanPosOutOfGapPos(bool onOff)
{
    m_oScanPosOutOfGapPos = onOff;
    if (isScanTrackerEnabled())
    {
        SetTrackerScanPos(false);
        if (!onOff) m_oScanPosFixedWasSet = true;
    }
    else if (isScanlabScannerEnabled() && (getScannerGeneralMode() == ScannerGeneralMode::eScantracker2DMode))
    {
        if (m_pScanlab != nullptr)
        {
            m_pScanlab->SetScanPosOutOfGapPos(onOff);
        }
    }
}

void WeldingHeadControl::SetTrackerScanPosFixed(int value)
{
	m_oScanPosFixed = value;
	SetTrackerScanPos(false);
	//m_oScanPosFixedWasSet = true;
}

void WeldingHeadControl::SetTrackerScanPosControlled(int value) // Interface: viWeldHeadSubscribe (event)
{
	m_oScanPosControlled = value;
    if (isScanTrackerEnabled())
    {
        SetTrackerScanPos(false);
    }
    else if (isScanlabScannerEnabled() && (getScannerGeneralMode() == ScannerGeneralMode::eScantracker2DMode))
    {
        if (m_pScanlab != nullptr)
        {
            m_pScanlab->SetTrackerScanPosControlled(value);
        }
    }
}

void WeldingHeadControl::SetTrackerDriverOnOff(bool p_oOnOff)
{
	if (!p_oOnOff) // Abschalten ist gefordert
	{
		// Neuer Zustand TrackerDriver-Var sofort setzen
		m_oTrackerDriverOnOff = p_oOnOff;
	}

	unsigned char oMask = 0;
	unsigned char oSendValue = 0;
	if(p_oOnOff)
	{
		oSendValue = oSendValue | (1 << m_oSTEnableDriverProxyInfo.nStartBit);
	}
	oMask = oMask | (1 << m_oSTEnableDriverProxyInfo.nStartBit);

	wmLog(eDebug, "SetTrackerDriverOnOff: SendValue: %X Mask: %X\n", oSendValue, oMask);

    if (m_oSTEnableDriverProxyInfo.m_oActive)
    {
        m_rEthercatOutputsProxy.ecatDigitalOut(m_oSTEnableDriverProxyInfo.m_oProductIndex,
                                               m_oSTEnableDriverProxyInfo.m_oInstance,
                                               (uint8_t) oSendValue,
                                               (uint8_t) oMask);
    }

	if(isScanTrackerEnabled())
	{
        // Beim Einschalten des TrackerDrivers dauert es ca. 25ms bis ScannerOK und ScannerLimits auf High gehen
        // Beim Ausschalten des TrackerDrivers dauert es ca. 25ms - 50ms (oder laenger !) bis ScannerOK und ScannerLimits auf Low gehen
        int oLoopLimit = 65;
        if (p_oOnOff) // Driver soll eingeschaltet werden
        {
            oLoopLimit = 65; // Einschalten schaerfer ueberwachen
        }
        else
        {
            oLoopLimit = 20; // Ausschalten nur kurz ueberwachen
        }
        bool oReady = false;
        int oLoop = 0;
    //	while(!oReady && (oLoop < 50))
        while(!oReady && (oLoop < oLoopLimit))
        {
            usleep(1 * 1000);
            if (p_oOnOff) // Driver soll eingeschaltet werden
            {
                if (m_oScannerOK) // Driver meldet OK Zustand zurueck
                {
                    oReady = true;
                }
            }
            else
            {
                if (!m_oScannerOK) // Driver meldet NOK Zustand zurueck
                {
                    oReady = true;
                }
            }
            oLoop++;
        }
        wmLog(eDebug, "SetTrackerDriverOnOff: loop: %d\n", oLoop);
        if (!oReady)
        {
            if (p_oOnOff) // Driver soll eingeschaltet werden
            {
                // Timeout beim Einschalten des Drivers --> notReady State
                wmFatal(eAxis, "QnxMsg.VI.ST_Timeout", "Scantracker signals an error, Scanner doesn't reach the OK state !\n");
            }
            else
            {
                // Timeout beim Ausschalten des Drivers --> Warnung
                //wmLogTr(eWarning, "QnxMsg.VI.ST_DownTimeLate", "Switch off Scantracker takes very long time !\n");
            }
        }
	}

	if (p_oOnOff) // Einschalten war gefordert
	{
		// Neuer Zustand TrackerDriver-Var erst nach vollstaendigem Einschalten setzen
		m_oTrackerDriverOnOff = p_oOnOff;
	}
}

void WeldingHeadControl::SetTrackerMaxAmplitude(int p_oTrackerMaxAmplitude)
{
	if (p_oTrackerMaxAmplitude != m_oTrackerMaxAmplitude)
	{
		WeldHeadDefaults::instance().setInt("ScanTrackerMaxAmplitude", p_oTrackerMaxAmplitude);
	}
	m_oTrackerMaxAmplitude = p_oTrackerMaxAmplitude;
	SetTrackerScanPos(false);
}

void WeldingHeadControl::SetTrackerExpertMode(bool onOff)
{
	m_oTrackerExpertMode = onOff;
	if (!m_oTrackerExpertMode) // ExpertMode is switched off
	{
		// set outputs to 0V
		SetTrackerScanPos(false);
		SetTrackerScanWidth(false);
	}
}

void WeldingHeadControl::doZCollDriving(DrivingType p_oDrivingType, int value) // Interface: viWeldHeadSubscribe (event)
{
	// ACHTUNG: folgende Umrechnung wird bis auf Weiteres im Graph durchgefuehrt !!!
    // hier Umrechnung von Hoehenaenderung Fokuspunkt in Hoehenaenderung ZCollimator durchfuehren !
	// bei Audi:
	// Kollimierlinse: 150 mm / Fokussierlinse: 300 mm
	// 2mm Aenderung Kollimerlinse -> 4mm Aenderung Fokuspunkt ! ?
	// Result bringt die neue Position des Fokuspunktes, d.h. ZCollimator muss hier den halben Wert fahren
	// ZColli-Wert = Result-Wert * (Kollimierlinse / Fokussierlinse)
	// muss der Wert danach negiert werden ?

    if (getZCollimatorType() == eZColliAnalog)
    {
        int oAbsolutePosition;
        if (p_oDrivingType == eAbsoluteDriving)
        {
            oAbsolutePosition = value;
        }
        else // eRelativeDriving
        {
            wmLog(eDebug, "doZCollDriving rel: %d, %f, %f\n", value, m_oZCSystemOffset.load(), m_oZCLensToFocusRatio.load());
            int oNewRelativePos = 0;
            // m_oZCSystemOffset: in mm, determined for focus position
            // m_oZCLensToFocusRatio: divider, transforming focus position to lens position of Z-Collimator
            // parameter "value": new relative position in um, determined for focus position, without SystemOffset, relative to middle of operating range
            oNewRelativePos = value + (m_oZCSystemOffset * 1000); // new relative position in um for focus position, with SystemOffset, relative to middle of operating range
            oNewRelativePos = static_cast<int>(static_cast<double>(oNewRelativePos) / m_oZCLensToFocusRatio); // new relative position for lens position, relative to middle of operating range
            oAbsolutePosition = oNewRelativePos + 8000;
        }

        // Fahrbereichsgrenze ZCollimator in positiver Richtung (Kollimierlinse nach unten)
        if(oAbsolutePosition > 16000)
        {
            oAbsolutePosition = 16000;
            wmLogTr( eError, "QnxMsg.VI.ZCollMaxPos", "position of Z-collimator is limited due to max. possible position ! (to %f mm)\n",
                    (static_cast<float>(oAbsolutePosition) / 1000.0));
        }
        // Fahrbereichsgrenze ZCollimator in negativer Richtung (Kollimierlinse nach oben)
        if(oAbsolutePosition < 0)
        {
            oAbsolutePosition = 0;
            wmLogTr( eError, "QnxMsg.VI.ZCollMinPos", "position of Z-collimator is limited due to min. possible position ! (to %f mm)\n",
                    (static_cast<float>(oAbsolutePosition) / 1000.0));
        }

        if (m_oZCStateVarAutomatic == 0) // wenn aktuell keine andere Positionierung aktiv
        {
            if (std::abs(oAbsolutePosition - m_oZCActPosition) > 10 ) // nur Fahren wenn Differenz zu bisher groesser als 10 um
            {
                SetZCAnalogInValue(oAbsolutePosition);
                usleep(2 * 1000);
                SetZCAutomatic(true);
                m_oZCActPosition = oAbsolutePosition;
            }
        }
        else
        {
            // Ausgabe Fehlermeldung / Warnmeldung ??
        }

        if(!m_oZCError) // Fehler wird durch Low-Zustand angezeigt
        {
            wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
        }
    }
    else if (getZCollimatorType() == eZColliDigitalV1)
    {
        auto oDrivingStart = std::chrono::high_resolution_clock::now();
        int oAbsolutePosition;
        if (p_oDrivingType == eAbsoluteDriving)
        {
            wmLog(eDebug, "doZCollDriving abs: %d\n", value);
            oAbsolutePosition = value;
        }
        else // eRelativeDriving
        {
            wmLog(eDebug, "doZCollDriving rel: %d, %f, %f\n", value, m_oZCSystemOffset.load(), m_oZCLensToFocusRatio.load());
            int oNewRelativePos = 0;
            // m_oZCSystemOffset: in mm, determined for focus position
            // m_oZCLensToFocusRatio: divider, transforming focus position to lens position of Z-Collimator
            // parameter "value": new relative position in um, determined for focus position, without SystemOffset, relative to middle of operating range
            oNewRelativePos = value + (m_oZCSystemOffset * 1000); // new relative position in um for focus position, with SystemOffset, relative to middle of operating range
            oNewRelativePos = static_cast<int>(static_cast<double>(oNewRelativePos) / m_oZCLensToFocusRatio); // new relative position for lens position, relative to middle of operating range
            oAbsolutePosition = oNewRelativePos + 8854;
        }

        // Fahrbereichsgrenze ZCollimator in positiver Richtung (Kollimierlinse nach unten)
        if(oAbsolutePosition > 16785)
        {
            oAbsolutePosition = 16785;
            wmLogTr( eError, "QnxMsg.VI.ZCollMaxPos", "position of Z-collimator is limited due to max. possible position ! (to %f mm)\n",
                    (static_cast<float>(oAbsolutePosition) / 1000.0));
        }
        // Fahrbereichsgrenze ZCollimator in negativer Richtung (Kollimierlinse nach oben)
        if(oAbsolutePosition < 925)
        {
            oAbsolutePosition = 925;
            wmLogTr( eError, "QnxMsg.VI.ZCollMinPos", "position of Z-collimator is limited due to min. possible position ! (to %f mm)\n",
                    (static_cast<float>(oAbsolutePosition) / 1000.0));
        }

        if (!m_oZCDrivingV2IsActive.load()) // wenn aktuell keine andere Positionierung aktiv
        {
            if (std::abs(oAbsolutePosition - m_oZCActPosition) > 10 ) // nur Fahren wenn Differenz zu bisher groesser als 10 um
            {
                if (isZCollimatorEnabled() )
                {
                    m_oZCDrivingV2IsActive.store(true);
                    if (m_pScanlab != nullptr)
                    {
                        m_pScanlab->SetZCDrivingV2IsActive(true);
                    }
                    wmLog(eDebug, "doZCollDriving: Value: %d um\n", oAbsolutePosition);

                    // give the digital Z-Collimator the start command for moving
                    m_oNextAbsolutePositionV2Um = static_cast<double>(oAbsolutePosition);
                    char oPrintString[81];
                    if (!m_oTmlCommunicator.driveTo(m_oNextAbsolutePositionV2Um, oPrintString))
                    {
                        wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
                    }
#if  DEBUG_NEW_ZCOLLIMATOR
                    wmLog(eDebug, "%s\n", oPrintString);
#endif

                    pthread_attr_t oPthreadAttr;
                    pthread_attr_init(&oPthreadAttr);
                    pthread_t oWaitingThreadID;
                    if (pthread_create(&oWaitingThreadID, &oPthreadAttr, &ZCollEndOfDrivingThread, this) != 0)
                    {
                        char oStrErrorBuffer[256];

                        wmLog(eDebug, "pthread_create failed (%s)(%s)\n", "001", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
                        wmFatal(eInternalError, "QnxMsg.VI.IDMCreateThreadFail", "Cannot start thread (%s)\n", "002");
                    }
                }
                m_oZCActPosition = oAbsolutePosition;
            }
        }
        auto oDrivingStop = std::chrono::high_resolution_clock::now();
        int oDuration = std::chrono::duration_cast<std::chrono::microseconds>(oDrivingStop-oDrivingStart).count();
        wmLog(eDebug, "Duration doZCollDriving in us: %d\n", oDuration );
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.ZCollWrongType", "wrong Z-Collimator type\n");
    }
}

void WeldingHeadControl::ZCWaitForEndOfDriving(void)
{
    auto oWaitingStart = std::chrono::high_resolution_clock::now();

#if  DEBUG_NEW_ZCOLLIMATOR
    bool oMotionComplete = false;
#endif
    bool oTargetReached = false;
    bool oFault = false;
    bool oAxisStatus = true;

    // wait for end of moving
    uint16_t oSRLRegister = 0; // status register low part
    uint16_t oSRHRegister = 0; // status register high part
    uint16_t oMERRegister = 0; // motion error register
    char oPrintString[81];
    usleep(2*1000); // first a startup delay of 2ms
    m_oTmlCommunicator.readStatus(oSRLRegister, oSRHRegister, oMERRegister, oPrintString);
#if  DEBUG_NEW_ZCOLLIMATOR
    oMotionComplete  = static_cast<bool>(oSRLRegister & 0x0400);
#endif
    oTargetReached = static_cast<bool>(oSRHRegister & 0x0200);
    oFault = static_cast<bool>(oSRHRegister & 0x8000);
    oAxisStatus = static_cast<bool>(oSRLRegister & 0x8000);
    if (oFault || !oAxisStatus)
    {
        wmLog(eDebug, "ZColliV2: Fault-Flag is set or AxisStatus is not set\n");
        wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
    }
#if  DEBUG_NEW_ZCOLLIMATOR
    wmLog(eDebug, "TML status: %s motion: %d,target:%d\n", oPrintString, oMotionComplete, oTargetReached);
    double oActualPosition = m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
#endif
    int oTimeout = 0;
    while ((!oTargetReached) && (oTimeout < 500)) // wait for TargetReached
    {
        usleep(500); // 500us
        m_oTmlCommunicator.readStatus(oSRLRegister, oSRHRegister, oMERRegister, oPrintString);
#if  DEBUG_NEW_ZCOLLIMATOR
        oMotionComplete  = static_cast<bool>(oSRLRegister & 0x0400);
#endif
        oTargetReached = static_cast<bool>(oSRHRegister & 0x0200);
        oFault = static_cast<bool>(oSRHRegister & 0x8000);
        oAxisStatus = static_cast<bool>(oSRLRegister & 0x8000);
        if (oFault || !oAxisStatus)
        {
            wmLog(eDebug, "ZColliV2: Fault-Flag is set or AxisStatus is not set\n");
            wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
        }
#if  DEBUG_NEW_ZCOLLIMATOR
        wmLog(eDebug, "TML status: %s motion: %d,target:%d\n", oPrintString, oMotionComplete, oTargetReached);
        oActualPosition = m_oTmlCommunicator.getActualPositionUm(oPrintString);
        wmLog(eDebug, "TML position: %s\n", oPrintString);
#endif
        oTimeout++;
    }
    if (oTimeout >= 500)
    {
        wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
    }
    else
    {
    }
#if DEBUG_NEW_ZCOLLIMATOR
    wmLog(eDebug, "Ziel erreicht (oTargetReached is true)\n");
#endif
#if !EMVTEST_NEW_ZCOLLIMATOR
    m_oZCPositionDigV1ReadOut.store(static_cast<int32_t>(m_oNextAbsolutePositionV2Um));
#endif

    auto oWaitingStop = std::chrono::high_resolution_clock::now();
    int oDuration = std::chrono::duration_cast<std::chrono::microseconds>(oWaitingStop-oWaitingStart).count();
    wmLog(eDebug, "Duration ZCWaitForEndOfDriving in us: %d\n", oDuration );

#if  DEBUG_NEW_ZCOLLIMATOR
#if 0
    wmLog(eDebug, "******************************\n");
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    usleep(10*1000);
    m_oTmlCommunicator.getActualPositionUm(oPrintString);
    wmLog(eDebug, "TML position: %s\n", oPrintString);
    wmLog(eDebug, "******************************\n");
#endif
#endif

    m_oZCDrivingV2IsActive.store(false);
    if (m_pScanlab != nullptr)
    {
        m_pScanlab->SetZCDrivingV2IsActive(false);
    }
}

int WeldingHeadControl::GetZCActualPositionV2()
{
    if ( !m_oZCDrivingV2IsActive.load() && !m_oZCHomingV2IsActive.load() )
    {
        return m_oTmlCommunicator.getActualPositionUm();
    }

    return 0;
}

//*****************************************************************************
//* viWeldHeadSubscribe Interface (Event)                                     *
//*****************************************************************************

//Info requested -> send headInfo event
void WeldingHeadControl::RequestHeadInfo(HeadAxisID axis) // Interface: viWeldHeadSubscribe (event)
{
	if (axis == interface::eAxisX)
	{
		HeadInfo info;
		memset(&info, 0, sizeof(info));
		if(m_pStatesHeadX != NULL)
		{
			m_pStatesHeadX->RequestHeadInfo(info);
		}
		info.m_oLineLaser1OnOff = m_oLineLas1OnOffToWin;
		info.m_oLineLaser1Intensity = m_oLineLas1IntensToWin;
		info.m_oLineLaser2OnOff = m_oLineLas2OnOffToWin;
		info.m_oLineLaser2Intensity = m_oLineLas2IntensToWin;
		info.m_oFieldLight1OnOff = m_oFieldL1OnOffToWin;
		info.m_oFieldLight1Intensity = m_oFieldL1IntensToWin;
		info.m_oLEDChan1Para = m_oLEDI_Parameters[LED_PANEL_1].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_1].led_enable) info.m_oLEDChan1Para |= 0x8000;
		info.m_oLEDChan2Para = m_oLEDI_Parameters[LED_PANEL_2].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_2].led_enable) info.m_oLEDChan2Para |= 0x8000;
		info.m_oLEDChan3Para = m_oLEDI_Parameters[LED_PANEL_3].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_3].led_enable) info.m_oLEDChan3Para |= 0x8000;
		info.m_oLEDChan4Para = m_oLEDI_Parameters[LED_PANEL_4].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_4].led_enable) info.m_oLEDChan4Para |= 0x8000;
		info.m_oLEDChan1PulseWidth = m_oLEDI_Parameters[LED_PANEL_1].led_pulse_width;
		info.m_oLEDChan2PulseWidth = m_oLEDI_Parameters[LED_PANEL_2].led_pulse_width;
		info.m_oLEDChan3PulseWidth = m_oLEDI_Parameters[LED_PANEL_3].led_pulse_width;
		info.m_oLEDChan4PulseWidth = m_oLEDI_Parameters[LED_PANEL_4].led_pulse_width;

		m_outMotionDataServer.headInfo(axis,info); // Interface: viWeldHeadPublish (event)
	}
	if (axis == interface::eAxisY)
	{
		HeadInfo info;
		memset(&info, 0, sizeof(info));
		if(m_pStatesHeadY != NULL)
		{
			m_pStatesHeadY->RequestHeadInfo(info);
		}
		info.m_oLineLaser1OnOff = m_oLineLas1OnOffToWin;
		info.m_oLineLaser1Intensity = m_oLineLas1IntensToWin;
		info.m_oLineLaser2OnOff = m_oLineLas2OnOffToWin;
		info.m_oLineLaser2Intensity = m_oLineLas2IntensToWin;
		info.m_oFieldLight1OnOff = m_oFieldL1OnOffToWin;
		info.m_oFieldLight1Intensity = m_oFieldL1IntensToWin;
		info.m_oLEDChan1Para = m_oLEDI_Parameters[LED_PANEL_1].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_1].led_enable) info.m_oLEDChan1Para |= 0x8000;
		info.m_oLEDChan2Para = m_oLEDI_Parameters[LED_PANEL_2].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_2].led_enable) info.m_oLEDChan2Para |= 0x8000;
		info.m_oLEDChan3Para = m_oLEDI_Parameters[LED_PANEL_3].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_3].led_enable) info.m_oLEDChan3Para |= 0x8000;
		info.m_oLEDChan4Para = m_oLEDI_Parameters[LED_PANEL_4].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_4].led_enable) info.m_oLEDChan4Para |= 0x8000;
		info.m_oLEDChan1PulseWidth = m_oLEDI_Parameters[LED_PANEL_1].led_pulse_width;
		info.m_oLEDChan2PulseWidth = m_oLEDI_Parameters[LED_PANEL_2].led_pulse_width;
		info.m_oLEDChan3PulseWidth = m_oLEDI_Parameters[LED_PANEL_3].led_pulse_width;
		info.m_oLEDChan4PulseWidth = m_oLEDI_Parameters[LED_PANEL_4].led_pulse_width;

		m_outMotionDataServer.headInfo(axis,info); // Interface: viWeldHeadPublish (event)
	}
	if (axis == interface::eAxisZ)
	{
		HeadInfo info;
		memset(&info, 0, sizeof(info));
		if(m_pStatesHeadZ != NULL)
		{
			m_pStatesHeadZ->RequestHeadInfo(info);
		}
		info.m_oLineLaser1OnOff = m_oLineLas1OnOffToWin;
		info.m_oLineLaser1Intensity = m_oLineLas1IntensToWin;
		info.m_oLineLaser2OnOff = m_oLineLas2OnOffToWin;
		info.m_oLineLaser2Intensity = m_oLineLas2IntensToWin;
		info.m_oFieldLight1OnOff = m_oFieldL1OnOffToWin;
		info.m_oFieldLight1Intensity = m_oFieldL1IntensToWin;
		info.m_oLEDChan1Para = m_oLEDI_Parameters[LED_PANEL_1].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_1].led_enable) info.m_oLEDChan1Para |= 0x8000;
		info.m_oLEDChan2Para = m_oLEDI_Parameters[LED_PANEL_2].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_2].led_enable) info.m_oLEDChan2Para |= 0x8000;
		info.m_oLEDChan3Para = m_oLEDI_Parameters[LED_PANEL_3].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_3].led_enable) info.m_oLEDChan3Para |= 0x8000;
		info.m_oLEDChan4Para = m_oLEDI_Parameters[LED_PANEL_4].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_4].led_enable) info.m_oLEDChan4Para |= 0x8000;
		info.m_oLEDChan1PulseWidth = m_oLEDI_Parameters[LED_PANEL_1].led_pulse_width;
		info.m_oLEDChan2PulseWidth = m_oLEDI_Parameters[LED_PANEL_2].led_pulse_width;
		info.m_oLEDChan3PulseWidth = m_oLEDI_Parameters[LED_PANEL_3].led_pulse_width;
		info.m_oLEDChan4PulseWidth = m_oLEDI_Parameters[LED_PANEL_4].led_pulse_width;

		m_outMotionDataServer.headInfo(axis,info); // Interface: viWeldHeadPublish (event)
	}
	if (axis == interface::eAxisTracker)
	{
		HeadInfo info;
		memset(&info, 0, sizeof(info));
		if(isScanTrackerEnabled())
		{
			info.m_oSoftLimitsActive = m_oTrackerExpertMode; // m_oSoftLimitsActive is recycled for m_oTrackerExpertMode !
			info.m_oAxisStatusBits = 0;
//			if (m_oScannerOK) info.m_oAxisStatusBits |= 0x01;
//			if (m_oScannerLimits) info.m_oAxisStatusBits |= 0x02;
			info.m_oAxisStatusBits |= 0x01;
			info.m_oAxisStatusBits |= 0x02;
			if ((m_oScanWidthLimitedUM) || (m_oScanWidthLimitedVolt)) info.m_oAxisStatusBits |= 0x04;
			if ((m_oScanPosLimitedUM) || (m_oScanPosLimitedVolt)) info.m_oAxisStatusBits |= 0x08;
			info.m_oScanWidthUMSent = m_oScanWidthUMSent;
			info.m_oScanWidthVoltSent = m_oScanWidthVoltSent;
			info.m_oScanPosUMSent = m_oScanPosUMSent;
			info.m_oScanPosVoltSent = m_oScanPosVoltSent;
		}
		info.m_oLineLaser1OnOff = m_oLineLas1OnOffToWin;
		info.m_oLineLaser1Intensity = m_oLineLas1IntensToWin;
		info.m_oLineLaser2OnOff = m_oLineLas2OnOffToWin;
		info.m_oLineLaser2Intensity = m_oLineLas2IntensToWin;
		info.m_oFieldLight1OnOff = m_oFieldL1OnOffToWin;
		info.m_oFieldLight1Intensity = m_oFieldL1IntensToWin;
		info.m_oLEDChan1Para = m_oLEDI_Parameters[LED_PANEL_1].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_1].led_enable) info.m_oLEDChan1Para |= 0x8000;
		info.m_oLEDChan2Para = m_oLEDI_Parameters[LED_PANEL_2].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_2].led_enable) info.m_oLEDChan2Para |= 0x8000;
		info.m_oLEDChan3Para = m_oLEDI_Parameters[LED_PANEL_3].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_3].led_enable) info.m_oLEDChan3Para |= 0x8000;
		info.m_oLEDChan4Para = m_oLEDI_Parameters[LED_PANEL_4].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_4].led_enable) info.m_oLEDChan4Para |= 0x8000;
		info.m_oLEDChan1PulseWidth = m_oLEDI_Parameters[LED_PANEL_1].led_pulse_width;
		info.m_oLEDChan2PulseWidth = m_oLEDI_Parameters[LED_PANEL_2].led_pulse_width;
		info.m_oLEDChan3PulseWidth = m_oLEDI_Parameters[LED_PANEL_3].led_pulse_width;
		info.m_oLEDChan4PulseWidth = m_oLEDI_Parameters[LED_PANEL_4].led_pulse_width;

		m_outMotionDataServer.headInfo(axis,info); // Interface: viWeldHeadPublish (event)
	}
	if (axis == interface::eAxisNone)
	{
		HeadInfo info;
		memset(&info, 0, sizeof(info));
		info.m_oLineLaser1OnOff = m_oLineLas1OnOffToWin;
		info.m_oLineLaser1Intensity = m_oLineLas1IntensToWin;
		info.m_oLineLaser2OnOff = m_oLineLas2OnOffToWin;
		info.m_oLineLaser2Intensity = m_oLineLas2IntensToWin;
		info.m_oFieldLight1OnOff = m_oFieldL1OnOffToWin;
		info.m_oFieldLight1Intensity = m_oFieldL1IntensToWin;
		info.m_oLEDChan1Para = m_oLEDI_Parameters[LED_PANEL_1].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_1].led_enable) info.m_oLEDChan1Para |= 0x8000;
		info.m_oLEDChan2Para = m_oLEDI_Parameters[LED_PANEL_2].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_2].led_enable) info.m_oLEDChan2Para |= 0x8000;
		info.m_oLEDChan3Para = m_oLEDI_Parameters[LED_PANEL_3].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_3].led_enable) info.m_oLEDChan3Para |= 0x8000;
		info.m_oLEDChan4Para = m_oLEDI_Parameters[LED_PANEL_4].led_brightness;
		if (m_oLEDI_Parameters[LED_PANEL_4].led_enable) info.m_oLEDChan4Para |= 0x8000;
		info.m_oLEDChan1PulseWidth = m_oLEDI_Parameters[LED_PANEL_1].led_pulse_width;
		info.m_oLEDChan2PulseWidth = m_oLEDI_Parameters[LED_PANEL_2].led_pulse_width;
		info.m_oLEDChan3PulseWidth = m_oLEDI_Parameters[LED_PANEL_3].led_pulse_width;
		info.m_oLEDChan4PulseWidth = m_oLEDI_Parameters[LED_PANEL_4].led_pulse_width;

		m_outMotionDataServer.headInfo(axis,info); // Interface: viWeldHeadPublish (event)
	}
}

void WeldingHeadControl::SetHeadMode(HeadAxisID axis, MotionMode mode, bool bHome) // Interface: viWeldHeadSubscribe (event)
{
    if (m_oDebugInfo_AxisController)
    {
        char oString1[81];
        char oString2[81];
        sprintf(oString1, "SetHeadMode: axis: %d, mode:%d, ", axis, mode);
        sprintf(oString2, "bHome:%d", bHome);
        wmLog(eDebug, "%s%s\n", oString1, oString2);
    }
	if (axis == interface::eAxisX)
	{
		if (m_pStatesHeadX != NULL) {
			m_pStatesHeadX->SetHeadMode(mode,bHome, true);
		}
	}
	if (axis == interface::eAxisY)
	{
		if (m_pStatesHeadY != NULL) {
			m_pStatesHeadY->SetHeadMode(mode,bHome, true);
		}
	}
	if (axis == interface::eAxisZ)
	{
		if (m_pStatesHeadZ != NULL) {
			m_pStatesHeadZ->SetHeadMode(mode,bHome, true);
		}
	}
}

void WeldingHeadControl::SetHeadValue(HeadAxisID axis, int value, MotionMode mode) // Interface: viWeldHeadSubscribe (event)
{
    if (m_oDebugInfo_AxisController)
    {
        char oString1[81];
        char oString2[81];
        sprintf(oString1, "SetHeadValue: axis: %d, value:%d, ", axis, value);
        sprintf(oString2, "mode:%d", mode);
        wmLog(eDebug, "%s%s\n", oString1, oString2);
    }
	if (axis == interface::eAxisX)
	{
		if (mode == Position_Absolute)
		{
			m_oRequiredPositionX = value;
		}
		else if (mode == Position_Relative)
		{
			m_oRequiredPositionX += value;
		}
		if (m_pStatesHeadX != NULL) {
			m_pStatesHeadX->SetHeadValue(value, mode);
		}
	}
	if (axis == interface::eAxisY)
	{
		if (mode == Position_Absolute)
		{
			m_oRequiredPositionY = value;
		}
		else if (mode == Position_Relative)
		{
			m_oRequiredPositionY += value;
		}
		if (m_pStatesHeadY != NULL) {
			m_pStatesHeadY->SetHeadValue(value, mode);
		}
	}
	if (axis == interface::eAxisZ)
	{
		if (mode == Position_Absolute)
		{
			m_oRequiredPositionZ = value;
		}
		else if (mode == Position_Relative)
		{
			m_oRequiredPositionZ += value;
		}
		if (m_pStatesHeadZ != NULL) {
			m_pStatesHeadZ->SetHeadValue(value, mode);
		}
	}
}

//*****************************************************************************
//* viWeldHeadPublish Interface (Event)                                       *
//*****************************************************************************

void WeldingHeadControl::HeadValueReached(HeadAxisID axis, MotionMode mode, int value) {
	wmLog(eDebug, "WeldingHeadControl: Head %d -> ValueReached... Value: %d\tMode: %d\n", axis,value, mode);
	m_outMotionDataServer.headValueReached(axis,mode,value); // Interface: viWeldHeadPublish (event)
}

void WeldingHeadControl::HeadIsReady(HeadAxisID axis, MotionMode mode) {
	wmLog(eDebug, "WeldingHeadControl: Head %d -> IsReady... Mode: %d\n", axis,mode);
	m_outMotionDataServer.headIsReady(axis,mode); // Interface: viWeldHeadPublish (event)
}

void WeldingHeadControl::HeadError(HeadAxisID axis, ErrorCode errorCode, int value){
	wmLog(eDebug, "WeldingHeadControl: Head %d -> headError... code: %d Value: %d\n", axis,errorCode,value);
	m_outMotionDataServer.headError(axis,errorCode,value); // Interface: viWeldHeadPublish (event)
}

void WeldingHeadControl::trackerInfo(TrackerInfo p_oTrackerInfo) {
	wmLog(eDebug, "WeldingHeadControl::trackerInfo: %d , %d\n", p_oTrackerInfo.ScannerOK, p_oTrackerInfo.ScannerLimits);
	m_outMotionDataServer.trackerInfo(p_oTrackerInfo); // Interface: viWeldHeadPublish (event)
}

//*****************************************************************************
//* triggerCmd Interface (Event)                                              *
//*****************************************************************************

/**
 * Passt das Senden der Sensordaten an die Bildfrequenz an.
 * @param ids Sensor IDs
 * @param context TriggerContext
 * @param interval Interval
 */
void WeldingHeadControl::burst(const std::vector<int>& ids, TriggerContext const& context, TriggerInterval const& interval)
{
wmLog(eDebug, "burst: nbTrig:%d, Delta[um]%d, Dist[ns]:%d, State:%d\n", interval.nbTriggers(), interval.triggerDelta(), interval.triggerDistance(), interval.state());

    if (interval.state() == workflow::eLiveMode)
    {
        m_liveModeIsOnAtBurst.store(true);
    }
    else
    {
        m_liveModeIsOnAtBurst.store(false);
    }

    // enable all extern sensors which are within sensor id list
    for (int id : ids)
    {
        const Sensor oId = Sensor(id);
        if (oId < eExternSensorMin || oId > eExternSensorMax)
        {
            continue; // id is not an extern sensor id
        } // if
        try
        {
            m_oSensorIdsEnabled.at(oId) = true;
        }
        catch (const std::out_of_range& oExc)
        {
            // nothing to do, prevents from inserting unknown sensor id
        }
    } // for

    bool oAtLeastOneSensorRequested = false;
    for(auto id: m_oSensorIdsEnabled)
    {
        if (id.second)
        {
            oAtLeastOneSensorRequested = true;
        }
    }
    if (!oAtLeastOneSensorRequested)
    {
        return;
    }

    m_oOversamplingSignal1RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oOversamplingSignal1RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eOversamplingSignal1]) m_oOversamplingSignal1RingBuffer.SetReadIsActive(true);
    m_oOversamplingSignal2RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oOversamplingSignal2RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eOversamplingSignal2]) m_oOversamplingSignal2RingBuffer.SetReadIsActive(true);
    m_oOversamplingSignal3RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oOversamplingSignal3RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eOversamplingSignal3]) m_oOversamplingSignal3RingBuffer.SetReadIsActive(true);
    m_oOversamplingSignal4RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oOversamplingSignal4RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eOversamplingSignal4]) m_oOversamplingSignal4RingBuffer.SetReadIsActive(true);
    m_oOversamplingSignal5RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oOversamplingSignal5RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eOversamplingSignal5]) m_oOversamplingSignal5RingBuffer.SetReadIsActive(true);
    m_oOversamplingSignal6RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oOversamplingSignal6RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eOversamplingSignal6]) m_oOversamplingSignal6RingBuffer.SetReadIsActive(true);
    m_oOversamplingSignal7RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oOversamplingSignal7RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eOversamplingSignal7]) m_oOversamplingSignal7RingBuffer.SetReadIsActive(true);
    m_oOversamplingSignal8RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oOversamplingSignal8RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eOversamplingSignal8]) m_oOversamplingSignal8RingBuffer.SetReadIsActive(true);

    m_oLWM40_1_PlasmaRingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oLWM40_1_PlasmaRingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eLWM40_1_Plasma]) m_oLWM40_1_PlasmaRingBuffer.SetReadIsActive(true);
    m_oLWM40_1_TemperatureRingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oLWM40_1_TemperatureRingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eLWM40_1_Temperature]) m_oLWM40_1_TemperatureRingBuffer.SetReadIsActive(true);
    m_oLWM40_1_BackReflectionRingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oLWM40_1_BackReflectionRingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eLWM40_1_BackReflection]) m_oLWM40_1_BackReflectionRingBuffer.SetReadIsActive(true);
    m_oLWM40_1_AnalogInputRingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_oLWM40_1_AnalogInputRingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eLWM40_1_AnalogInput]) m_oLWM40_1_AnalogInputRingBuffer.SetReadIsActive(true);

#if EMVTEST_NEW_ZCOLLIMATOR
    if (m_oSensorIdsEnabled[eZCPositionDigV1])
    {
        if (getZCollimatorType() != eZColliDigitalV1)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Z-Collimator position is only available for second generation Z-Collimator !\n");
        }
        StartZCPositionDigV1ReadOutThread();
    }
#endif

    StartSendSensorDataThread(interval.triggerDistance());

    const std::lock_guard<std::mutex> lock(m_mutex);
	m_context = context;
	m_interval = interval;

    for(int i = eAnalogIn1;i <= eAnalogIn8;i++)
    {
        m_imageNrGPAI[i] = 0;
    }
	m_imageNrLP = 0;
	m_imageNrRobSpeed = 0;
	m_imageNrENC1 = 0;
	m_imageNrENC2 = 0;

	m_imageNrOverSmp1 = 0;
	m_imageNrOverSmp2 = 0;
	m_imageNrOverSmp3 = 0;
	m_imageNrOverSmp4 = 0;
	m_imageNrOverSmp5 = 0;
	m_imageNrOverSmp6 = 0;
	m_imageNrOverSmp7 = 0;
	m_imageNrOverSmp8 = 0;

	m_imageNrScannerXPos = 0;
	m_imageNrScannerYPos = 0;
	m_imageNrFiberSwitchPos = 0;
	m_imageNrZCPositionDigV1 = 0;
	m_imageNrScannerWeldingFinished = 0;
    m_imageNrContourPreparedFinished = 0;

	m_imageNrLWM40_1_Plasma = 0;
	m_imageNrLWM40_1_Temperature = 0;
	m_imageNrLWM40_1_BackReflection = 0;
	m_imageNrLWM40_1_AnalogInput = 0;

	m_triggerDistanceNanoSecs = m_interval.triggerDistance(); // m_interval has triggerDist info in nanoseconds

	cleanBuffer();

	unsigned int ecatSamplesPerTrigger = 1;

	m_pValuesGPAI1 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesGPAI1[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGPAI1[i][j] = 0;
	}

	m_pValuesGPAI2 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesGPAI2[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGPAI2[i][j] = 0;
	}

	m_pValuesGPAI3 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesGPAI3[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGPAI3[i][j] = 0;
	}

	m_pValuesGPAI4 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesGPAI4[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGPAI4[i][j] = 0;
	}

	m_pValuesGPAI5 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesGPAI5[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGPAI5[i][j] = 0;
	}

	m_pValuesGPAI6 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesGPAI6[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGPAI6[i][j] = 0;
	}

	m_pValuesGPAI7 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesGPAI7[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGPAI7[i][j] = 0;
	}

	m_pValuesGPAI8 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesGPAI8[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGPAI8[i][j] = 0;
	}

	m_pValuesLP = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesLP[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesLP[i][j] = 0;
	}

	m_pValuesRobSpeed = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesRobSpeed[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < (ecatSamplesPerTrigger);j++) m_pValuesRobSpeed[i][j] = 0;
	}

	m_pValuesENC1 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesENC1[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesENC1[i][j] = 0;
	}

	m_pValuesENC2 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesENC2[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesENC2[i][j] = 0;
	}

	m_pValuesOverSmp1 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesOverSmp1[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesOverSmp1[i][j] = 0;
	}

	m_pValuesOverSmp2 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesOverSmp2[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesOverSmp2[i][j] = 0;
	}

	m_pValuesOverSmp3 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesOverSmp3[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesOverSmp3[i][j] = 0;
	}

	m_pValuesOverSmp4 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesOverSmp4[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesOverSmp4[i][j] = 0;
	}

	m_pValuesOverSmp5 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesOverSmp5[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesOverSmp5[i][j] = 0;
	}

	m_pValuesOverSmp6 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesOverSmp6[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesOverSmp6[i][j] = 0;
	}

	m_pValuesOverSmp7 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesOverSmp7[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesOverSmp7[i][j] = 0;
	}

	m_pValuesOverSmp8 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesOverSmp8[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesOverSmp8[i][j] = 0;
	}

	m_pValuesScannerXPos = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesScannerXPos[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < (ecatSamplesPerTrigger);j++) m_pValuesScannerXPos[i][j] = 0;
	}

	m_pValuesScannerYPos = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesScannerYPos[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < (ecatSamplesPerTrigger);j++) m_pValuesScannerYPos[i][j] = 0;
	}

    m_pValuesFiberSwitchPos = new TSmartArrayPtr<int>::ShArrayPtr[1];
    for (unsigned int i = 0; i < 1; ++i)
    {
        m_pValuesFiberSwitchPos[i] = new int[ecatSamplesPerTrigger];
        for (unsigned int j = 0; j < (ecatSamplesPerTrigger); j++)
        {
            m_pValuesFiberSwitchPos[i][j] = 0;
        }
    }

	m_pValuesZCPositionDigV1 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesZCPositionDigV1[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < (ecatSamplesPerTrigger);j++) m_pValuesZCPositionDigV1[i][j] = 0;
	}

	m_pValuesScannerWeldingFinished = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
		m_pValuesScannerWeldingFinished[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < (ecatSamplesPerTrigger);j++) m_pValuesScannerWeldingFinished[i][j] = 0;
	}

	m_pValuesContourPreparedFinished = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i)
    {
		m_pValuesContourPreparedFinished[i] = new int[ecatSamplesPerTrigger];
		for(unsigned int j = 0; j < (ecatSamplesPerTrigger);j++)
        {
            m_pValuesContourPreparedFinished[i][j] = 0;
        }
	}

	m_pValuesLWM40_1_Plasma = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesLWM40_1_Plasma[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesLWM40_1_Plasma[i][j] = 0;
	}

	m_pValuesLWM40_1_Temperature = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesLWM40_1_Temperature[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesLWM40_1_Temperature[i][j] = 0;
	}

	m_pValuesLWM40_1_BackReflection = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesLWM40_1_BackReflection[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesLWM40_1_BackReflection[i][j] = 0;
	}

	m_pValuesLWM40_1_AnalogInput = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i) {
        float oFloat = static_cast<float>(m_triggerDistanceNanoSecs) / 1000000.0;
        unsigned int oInt = std::ceil(oFloat);
		m_pValuesLWM40_1_AnalogInput[i] = new int[100 * oInt];
		for(unsigned int j = 0; j < (100 * oInt);j++) m_pValuesLWM40_1_AnalogInput[i][j] = 0;
	}

	if (m_pStatesHeadX != NULL) m_pStatesHeadX->burst(context, interval);
	if (m_pStatesHeadY != NULL) m_pStatesHeadY->burst(context, interval);
	if (m_pStatesHeadZ != NULL) m_pStatesHeadZ->burst(context, interval);
    if (m_pScanlab != nullptr)
    {
        m_pScanlab->burst(context, interval);
    }
} //release lock

void WeldingHeadControl::cancel(int id)
{
    StopSendSensorDataThread();
#if EMVTEST_NEW_ZCOLLIMATOR
    if (m_oSensorIdsEnabled[eZCPositionDigV1]) StopZCPositionDigV1ReadOutThread();
#endif

    m_oOversamplingSignal1RingBuffer.SetReadIsActive(false);
    m_oOversamplingSignal2RingBuffer.SetReadIsActive(false);
    m_oOversamplingSignal3RingBuffer.SetReadIsActive(false);
    m_oOversamplingSignal4RingBuffer.SetReadIsActive(false);
    m_oOversamplingSignal5RingBuffer.SetReadIsActive(false);
    m_oOversamplingSignal6RingBuffer.SetReadIsActive(false);
    m_oOversamplingSignal7RingBuffer.SetReadIsActive(false);
    m_oOversamplingSignal8RingBuffer.SetReadIsActive(false);

    m_oLWM40_1_PlasmaRingBuffer.SetReadIsActive(false);
    m_oLWM40_1_TemperatureRingBuffer.SetReadIsActive(false);
    m_oLWM40_1_BackReflectionRingBuffer.SetReadIsActive(false);
    m_oLWM40_1_AnalogInputRingBuffer.SetReadIsActive(false);
//wmLog(eDebug, "cancel\n");

    const std::lock_guard<std::mutex> lock(m_mutex);
	m_context = TriggerContext(0,0,0);
	m_interval = TriggerInterval(0,0);

    for(int i = eAnalogIn1;i <= eAnalogIn8;i++)
    {
        m_imageNrGPAI[i] = 0;
    }
	m_imageNrLP = 0;
	m_imageNrRobSpeed = 0;
	m_imageNrENC1 = 0;
	m_imageNrENC2 = 0;

	m_imageNrOverSmp1 = 0;
	m_imageNrOverSmp2 = 0;
	m_imageNrOverSmp3 = 0;
	m_imageNrOverSmp4 = 0;
	m_imageNrOverSmp5 = 0;
	m_imageNrOverSmp6 = 0;
	m_imageNrOverSmp7 = 0;
	m_imageNrOverSmp8 = 0;

	m_imageNrScannerXPos = 0;
	m_imageNrScannerYPos = 0;
	m_imageNrFiberSwitchPos = 0;
	m_imageNrZCPositionDigV1 = 0;
	m_imageNrScannerWeldingFinished = 0;
    m_imageNrContourPreparedFinished = 0;

	m_imageNrLWM40_1_Plasma = 0;
	m_imageNrLWM40_1_Temperature = 0;
	m_imageNrLWM40_1_BackReflection = 0;
	m_imageNrLWM40_1_AnalogInput = 0;

	m_triggerDistanceNanoSecs = 0;
	cleanBuffer();

	if (m_pStatesHeadX != NULL) m_pStatesHeadX->cancel(id);
	if (m_pStatesHeadY != NULL) m_pStatesHeadY->cancel(id);
	if (m_pStatesHeadZ != NULL) m_pStatesHeadZ->cancel(id);
    if (m_pScanlab != nullptr)
    {
        m_pScanlab->cancel();
    }

	// reset enabled states for all extern sensors
	for (auto& rSensorIdEnabled : m_oSensorIdsEnabled) {
		rSensorIdEnabled.second	=	false;
	} // for
}

//clean buffer
void WeldingHeadControl::cleanBuffer(){
	if(m_pValuesGPAI1 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesGPAI1[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGPAI1;
		m_pValuesGPAI1 = NULL;
	}

	if(m_pValuesGPAI2 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesGPAI2[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGPAI2;
		m_pValuesGPAI2 = NULL;
	}

	if(m_pValuesGPAI3 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesGPAI3[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGPAI3;
		m_pValuesGPAI3 = NULL;
	}

	if(m_pValuesGPAI4 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesGPAI4[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGPAI4;
		m_pValuesGPAI4 = NULL;
	}

	if(m_pValuesGPAI5 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesGPAI5[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGPAI5;
		m_pValuesGPAI5 = NULL;
	}

	if(m_pValuesGPAI6 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesGPAI6[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGPAI6;
		m_pValuesGPAI6 = NULL;
	}

	if(m_pValuesGPAI7 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesGPAI7[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGPAI7;
		m_pValuesGPAI7 = NULL;
	}

	if(m_pValuesGPAI8 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesGPAI8[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGPAI8;
		m_pValuesGPAI8 = NULL;
	}

	if(m_pValuesLP != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesLP[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesLP;
		m_pValuesLP = NULL;
	}

	if(m_pValuesRobSpeed != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesRobSpeed[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesRobSpeed;
		m_pValuesRobSpeed = NULL;
	}

	if(m_pValuesENC1 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesENC1[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesENC1;
		m_pValuesENC1 = NULL;
	}

	if(m_pValuesENC2 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesENC2[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesENC2;
		m_pValuesENC2 = NULL;
	}

	if(m_pValuesOverSmp1 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesOverSmp1[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesOverSmp1;
		m_pValuesOverSmp1 = NULL;
	}

	if(m_pValuesOverSmp2 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesOverSmp2[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesOverSmp2;
		m_pValuesOverSmp2 = NULL;
	}

	if(m_pValuesOverSmp3 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesOverSmp3[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesOverSmp3;
		m_pValuesOverSmp3 = NULL;
	}

	if(m_pValuesOverSmp4 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesOverSmp4[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesOverSmp4;
		m_pValuesOverSmp4 = NULL;
	}

	if(m_pValuesOverSmp5 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesOverSmp5[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesOverSmp5;
		m_pValuesOverSmp5 = NULL;
	}

	if(m_pValuesOverSmp6 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesOverSmp6[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesOverSmp6;
		m_pValuesOverSmp6 = NULL;
	}

	if(m_pValuesOverSmp7 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesOverSmp7[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesOverSmp7;
		m_pValuesOverSmp7 = NULL;
	}

	if(m_pValuesOverSmp8 != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesOverSmp8[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesOverSmp8;
		m_pValuesOverSmp8 = NULL;
	}

	if(m_pValuesScannerXPos != nullptr){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesScannerXPos[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesScannerXPos;
		m_pValuesScannerXPos = nullptr;
	}

	if(m_pValuesScannerYPos != nullptr){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesScannerYPos[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesScannerYPos;
		m_pValuesScannerYPos = nullptr;
	}

    if (m_pValuesFiberSwitchPos != nullptr)
    {
        for (unsigned int i = 0; i < 1; ++i)
        {
            m_pValuesFiberSwitchPos[i] = 0; // decrement smart pointer reference counter
        }
        delete [] m_pValuesFiberSwitchPos;
        m_pValuesFiberSwitchPos = nullptr;
    }

	if(m_pValuesZCPositionDigV1 != nullptr){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesZCPositionDigV1[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesZCPositionDigV1;
		m_pValuesZCPositionDigV1 = nullptr;
	}

	if (m_pValuesScannerWeldingFinished != nullptr)
    {
		for (unsigned int i = 0; i < 1; ++i)
        {
			m_pValuesScannerWeldingFinished[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesScannerWeldingFinished;
		m_pValuesScannerWeldingFinished = nullptr;
	}

	if(m_pValuesContourPreparedFinished != nullptr){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesContourPreparedFinished[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesContourPreparedFinished;
		m_pValuesContourPreparedFinished = nullptr;
	}

	if(m_pValuesLWM40_1_Plasma != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesLWM40_1_Plasma[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesLWM40_1_Plasma;
		m_pValuesLWM40_1_Plasma = NULL;
	}

	if(m_pValuesLWM40_1_Temperature != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesLWM40_1_Temperature[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesLWM40_1_Temperature;
		m_pValuesLWM40_1_Temperature = NULL;
	}

	if(m_pValuesLWM40_1_BackReflection != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesLWM40_1_BackReflection[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesLWM40_1_BackReflection;
		m_pValuesLWM40_1_BackReflection = NULL;
	}

	if(m_pValuesLWM40_1_AnalogInput != NULL){
		for (unsigned int i = 0; i < 1; ++i) {
			m_pValuesLWM40_1_AnalogInput[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesLWM40_1_AnalogInput;
		m_pValuesLWM40_1_AnalogInput = NULL;
	}
}

//*****************************************************************************
//* inspection Interface (Event) (nur Teile der Funktionen)                   *
//*****************************************************************************

void WeldingHeadControl::startAutomaticmode(uint32_t producttype, uint32_t productnumber) // Interface inspection
{
	m_oCycleIsOn = true;

	m_oScanWidthFixedWasSet = false;
	m_oScanPosFixedWasSet = false;

	if (m_pDataToLaserControl != nullptr)
	{
		m_pDataToLaserControl->setInspectCycleIsOn(true);
		int oResult = m_pDataToLaserControl->openTcpConnectionToLC();
		if (oResult == -1)
		{
			// Fehler aufgetreten beim Oeffnen der Netzwerkverbindung
		}
	}

	// Zyklusstart an StateMachine-Threads der Achsen weitergeben
	if (m_pStatesHeadX != nullptr)
	{
		m_pStatesHeadX->startAutomaticmode();
	}
	if (m_pStatesHeadY != nullptr)
	{
		m_pStatesHeadY->startAutomaticmode();
	}
	if (m_pStatesHeadZ != nullptr)
	{
		m_pStatesHeadZ->startAutomaticmode();
	}

    if (m_pScanlab != nullptr)
    {
        m_pScanlab->startAutomaticmode();
    }
}

void WeldingHeadControl::stopAutomaticmode()           // Interface inspection
{
	m_oCycleIsOn = false;

	if (m_pDataToLaserControl != nullptr)
	{
		m_pDataToLaserControl->setInspectCycleIsOn(false);
		int oResult = m_pDataToLaserControl->closeTcpConnectionToLC();
		if (oResult == -1)
		{
			// Fehler aufgetreten beim Schliessen der Netzwerkverbindung
		}
	}

	// Zyklusstart an StateMachine-Threads der Achsen weitergeben
	if (m_pStatesHeadX != nullptr)
	{
		m_pStatesHeadX->stopAutomaticmode();
	}
	if (m_pStatesHeadY != nullptr)
	{
		m_pStatesHeadY->stopAutomaticmode();
	}
	if (m_pStatesHeadZ != nullptr)
	{
		m_pStatesHeadZ->stopAutomaticmode();
	}

	// Bei Zyklusende Scantracker-Driver abschalten und Position und Amplitude zuruecksetzen
	if(isScanTrackerEnabled())
	{
		SetTrackerScanWidth(true); // in FailSafe-Zustand gehen
		SetTrackerScanPos(true); // in FailSafe-Zustand gehen
		SetTrackerDriverOnOff(false);
	}
}

void WeldingHeadControl::start(int seamnumber)         // Interface inspection
{
	m_oSeamIsOn = true;
	if(isScanTrackerEnabled())
	{
		m_oTrackerExpertMode = false;
		if (m_oScanWidthFixedWasSet)
		{
			SetTrackerScanWidth(false);
		}
		if (m_oScanPosFixedWasSet)
		{
			SetTrackerScanPos(false);
		}
	}
	if (isLaserControlEnabled())
	{
		SetLCStartSignal(true);
	}
    if (m_pScanlab != nullptr)
    {
        m_pScanlab->ScannerResetLastError();
        m_pScanlab->SeamStart(seamnumber);
    }
}

void WeldingHeadControl::end(int seamnumber)           // Interface inspection
{
	if (isLaserControlEnabled())
	{
		SetLCStartSignal(false);
	}
	// Bei Nahtende nur Position und Amplitude zuruecknehmen, Zustand Tracker-Driver bleibt unveraendert.
	if(isScanTrackerEnabled())
	{
		SetTrackerScanWidth(true); // in FailSafe-Zustand gehen
		SetTrackerScanPos(true); // in FailSafe-Zustand gehen
	}
    if (m_pScanlab != nullptr)
    {
        m_pScanlab->SeamEnd();
        m_pScanlab->ScannerOperatingState();
    }
	m_oSeamIsOn = false;
}

void WeldingHeadControl::info(int seamsequence)        // Interface inspection
{
    if (m_pScanlab != nullptr)
    {
        m_pScanlab->ScannerOperatingState();
        m_pScanlab->ScannerResetLastError();
    }
}

void WeldingHeadControl::setDebugInfo_AxisController(bool p_oDebugInfo_AxisController)
{
    m_oDebugInfo_AxisController = p_oDebugInfo_AxisController;

    if (m_pStatesHeadX != nullptr)
    {
        m_pStatesHeadX->setDebugInfo_AxisController(m_oDebugInfo_AxisController);
    }
    if (m_pStatesHeadY != nullptr)
    {
        m_pStatesHeadY->setDebugInfo_AxisController(m_oDebugInfo_AxisController);
    }
    if (m_pStatesHeadZ != nullptr)
    {
        m_pStatesHeadZ->setDebugInfo_AxisController(m_oDebugInfo_AxisController);
    }
};

//*****************************************************************************
//* weldHead Interface (Message)                                              *
//*****************************************************************************

bool WeldingHeadControl::setHeadPos(HeadAxisID axis, int value) // Interface: weldHead (message)
{
    if (m_oDebugInfo_AxisController)
    {
        wmLog(eDebug, "setHeadPos: axis: %d, value:%d\n", axis, value);
    }
	bool ret = false;
	if (axis == interface::eAxisX)
	{
		if (m_pStatesHeadX != NULL)
		{
			ret = m_pStatesHeadX->setHeadPosMsg(value);
		}
	}
	if (axis == interface::eAxisY)
	{
		if (m_pStatesHeadY != NULL)
		{
			ret = m_pStatesHeadY->setHeadPosMsg(value);
		}
	}
	if (axis == interface::eAxisZ)
	{
		if (m_pStatesHeadZ != NULL)
		{
			ret = m_pStatesHeadZ->setHeadPosMsg(value);
		}
	}
	return ret;
}

bool WeldingHeadControl::setHeadMode(HeadAxisID axis, MotionMode mode, bool bHome) // Interface: weldHead (message)
{
    if (m_oDebugInfo_AxisController)
    {
        wmLog(eDebug, "setHeadMode: axis: %d, mode:%d, bHome:%d\n", axis, mode, bHome);
    }
	bool ret = false;
	if (axis == interface::eAxisX)
	{
		if (m_pStatesHeadX != NULL)
		{
			ret = m_pStatesHeadX->setHeadModeMsg(mode,bHome);
		}
	}
	if (axis == interface::eAxisY)
	{
		if (m_pStatesHeadY != NULL)
		{
			ret = m_pStatesHeadY->setHeadModeMsg(mode,bHome);
		}
	}
	if (axis == interface::eAxisZ)
	{
		if (m_pStatesHeadZ != NULL)
		{
			ret = m_pStatesHeadZ->setHeadModeMsg(mode,bHome);
		}
	}
	return ret;
}

int WeldingHeadControl::getHeadPosition(HeadAxisID axis) // Interface: weldHead (message)
{
	int ret = 0;
	if (axis == interface::eAxisX)
	{
		if (m_pStatesHeadX != NULL)
		{
			ret = m_pStatesHeadX->getHeadPositionMsg();
		}
	}
	if (axis == interface::eAxisY)
	{
		if (m_pStatesHeadY != NULL)
		{
			ret = m_pStatesHeadY->getHeadPositionMsg();
		}
	}
	if (axis == interface::eAxisZ)
	{
		if (m_pStatesHeadZ != NULL)
		{
			ret = m_pStatesHeadZ->getHeadPositionMsg();
		}
	}
	return ret;
}

int WeldingHeadControl::getLowerLimit(HeadAxisID axis) // Interface: weldHead (message)
{
	int ret = 0;
	if (axis == interface::eAxisX)
	{
		if (m_pStatesHeadX != NULL)
		{
			ret = m_pStatesHeadX->getLowerLimitMsg();
		}
	}
	if (axis == interface::eAxisY)
	{
		if (m_pStatesHeadY != NULL)
		{
			ret = m_pStatesHeadY->getLowerLimitMsg();
		}
	}
	if (axis == interface::eAxisZ)
	{
		if (m_pStatesHeadZ != NULL)
		{
			ret = m_pStatesHeadZ->getLowerLimitMsg();
		}
	}
	return ret;
}

int WeldingHeadControl::getUpperLimit(HeadAxisID axis) // Interface: weldHead (message)
{
	int ret = 0;
	if (axis == interface::eAxisX)
	{
		if (m_pStatesHeadX != NULL)
		{
			ret = m_pStatesHeadX->getUpperLimitMsg();
		}
	}
	if (axis == interface::eAxisY)
	{
		if (m_pStatesHeadY != NULL)
		{
			ret = m_pStatesHeadY->getUpperLimitMsg();
		}
	}
	if (axis == interface::eAxisZ)
	{
		if (m_pStatesHeadZ != NULL)
		{
			ret = m_pStatesHeadZ->getUpperLimitMsg();
		}
	}
	return ret;
}

bool WeldingHeadControl::doZCollHoming(void) // Interface: weldHead (message)
{
    wmLog(eDebug, "doZCollHoming Start\n");
    bool oRet = false;
    if (getZCollimatorType() == eZColliAnalog)
    {
        m_oZCRefTravelEvent.reset();
        SetZCRefTravel(true);
        oRet = m_oZCRefTravelEvent.tryWait(20000);
        if (!oRet)
        {
            m_oZCStateVarAutomatic = 0;
            SetZCAutomatic(false);
            m_oZCStateVarRefTravel = 0;
            SetZCRefTravel(false);
            wmFatal(eAxis, "QnxMsg.VI.ZCollErrHoming", "Error while doing reference travelling of Z-collimator\n");
        }

        if(!m_oZCError) // Fehler wird durch Low-Zustand angezeigt
        {
            wmFatal(eAxis, "QnxMsg.VI.ZCollError", "Error while travelling of Z-collimator\n");
        }
    }
    else if (getZCollimatorType() == eZColliDigitalV1)
    {
        auto oDrivingStart = std::chrono::high_resolution_clock::now();
        m_oZCHomingV2IsActive.store(true);

        bool oFault = false;
        bool oAxisStatus = true;

        // give the digital Z-Collimator the start command for homing
        if (!m_oTmlCommunicator.doZCollHoming())
        {
            wmFatal(eAxis, "QnxMsg.VI.ZCollErrHoming", "Error while doing reference travelling of Z-collimator\n");
        }

        // wait for end of homing
        uint16_t oSRLRegister = 0; // status register low part
        uint16_t oSRHRegister = 0; // status register high part
        uint16_t oMERRegister = 0; // motion error register
        char oPrintString[81];
        usleep(3000*1000); // first a startup delay of 3s
        m_oTmlCommunicator.readStatus(oSRLRegister, oSRHRegister, oMERRegister, oPrintString);
        oFault = static_cast<bool>(oSRHRegister & 0x8000);
        oAxisStatus = static_cast<bool>(oSRLRegister & 0x8000);
        if (oFault || !oAxisStatus)
        {
            wmLog(eDebug, "ZColliV2: Fault-Flag is set or AxisStatus is not set\n");
            wmFatal(eAxis, "QnxMsg.VI.ZCollErrHoming", "Error while doing reference travelling of Z-collimator\n");
        }
#if  DEBUG_NEW_ZCOLLIMATOR
        wmLog(eDebug, "TML status: %s\n", oPrintString);
#endif
        int oTimeout = 0;
        while (((oSRLRegister & 0xC400) != 0xC400) && (oTimeout < 50))
        {
            usleep(100*1000); // 100ms
            m_oTmlCommunicator.readStatus(oSRLRegister, oSRHRegister, oMERRegister, oPrintString);
            oFault = static_cast<bool>(oSRHRegister & 0x8000);
            oAxisStatus = static_cast<bool>(oSRLRegister & 0x8000);
            if (oFault || !oAxisStatus)
            {
                wmLog(eDebug, "ZColliV2: Fault-Flag is set or AxisStatus is not set\n");
                wmFatal(eAxis, "QnxMsg.VI.ZCollErrHoming", "Error while doing reference travelling of Z-collimator\n");
            }
#if  DEBUG_NEW_ZCOLLIMATOR
            wmLog(eDebug, "TML status: %s\n", oPrintString);
#endif
            oTimeout++;
        }
        if (oTimeout >= 50)
        {
            wmFatal(eAxis, "QnxMsg.VI.ZCollErrHoming", "Error while doing reference travelling of Z-collimator\n");
            oRet = false;
        }
        else
        {
#if !EMVTEST_NEW_ZCOLLIMATOR
            m_oZCPositionDigV1ReadOut.store(8960); // Position after homing ?
#endif
            usleep(20*1000);
            m_oZCActPosition = 900; // force a driving command
            doZCollDriving(eAbsoluteDriving, 8854);
            usleep(10*1000); // 10ms
            oTimeout = 0;
            while ((m_oZCDrivingV2IsActive.load()) && (oTimeout < 100))
            {
                usleep(10*1000); // 10ms
                oTimeout++;
            }
            oRet = true;
        }

        m_oZCHomingV2IsActive.store(false);
        auto oDrivingStop = std::chrono::high_resolution_clock::now();
        int oDuration = std::chrono::duration_cast<std::chrono::microseconds>(oDrivingStop-oDrivingStart).count();
        wmLog(eDebug, "Duration doZCollHoming in us: %d\n", oDuration );
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.ZCollWrongType", "wrong Z-Collimator type\n");
        oRet = false;
    }

    wmLog(eDebug, "doZCollHoming End: %d\n", oRet);
    return oRet;
}

bool WeldingHeadControl::reloadFiberSwitchCalibration(void)
{
    if (m_pScanlab != nullptr)
    {
        m_pScanlab->reloadFiberSwitchCalibration();
    }
    return true;
}

bool WeldingHeadControl::weldForScannerCalibration(const std::vector<geo2d::DPoint>& points, double laserPowerInPct, double durationInMs, double jumpSpeedInMmPerSec)
{
    if (m_pScanlab != nullptr)
    {
        return m_pScanlab->weldForScannerCalibration(points, laserPowerInPct, durationInMs, jumpSpeedInMmPerSec);
    }
    return false;
}

bool WeldingHeadControl::doZCollDrivingRelative(int value) // Interface: weldHead (message)
{
    wmLog(eDebug, "doZCollDrivingRelative Start, value: %d\n", value);
    bool returnValue{true};

    if (getZCollimatorType() == eZColliAnalog)
    {
        wmFatal(eAxis, "QnxMsg.VI.ZCollWrongType", "wrong Z-Collimator type\n");
        returnValue = false;
    }
    else if (getZCollimatorType() == eZColliDigitalV1)
    {
        doZCollDriving(eRelativeDriving, value);
        usleep(10 * 1000);
        int timeout{0};
        while ((m_oZCDrivingV2IsActive.load()) && (timeout < 100))
        {
            usleep(10 * 1000);
            timeout++;
        }
        if (timeout >= 100)
        {
            // wmFatal is sent in ZCWaitForEndOfDriving
            returnValue = false;
        }
    }

    wmLog(eDebug, "doZCollDrivingRelative End: %d\n", returnValue);
    return returnValue;
}

//*****************************************************************************
//* Interface EthercatInputs
//*****************************************************************************

void WeldingHeadControl::ecatDigitalIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t value) // Interface EthercatInputs
{
    if (m_oSTScannerOkProxyInfo.m_oActive)
    {
        if (m_oSTScannerOkProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oSTScannerOkProxyInfo.m_oInstance == instance)
            {
                IncomingScannerOK(value);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

   if (m_oSTScannerLimitsProxyInfo.m_oActive)
    {
        if (m_oSTScannerLimitsProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oSTScannerLimitsProxyInfo.m_oInstance == instance)
            {
                IncomingScannerLimits(value);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

   if (m_oZCErrorProxyInfo.m_oActive)
    {
        if (m_oZCErrorProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oZCErrorProxyInfo.m_oInstance == instance)
            {
                IncomingZCError(value);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

   if (m_oZCPosReachedProxyInfo.m_oActive)
    {
        if (m_oZCPosReachedProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oZCPosReachedProxyInfo.m_oInstance == instance)
            {
                IncomingZCPosReached(value);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

   if (m_oLCErrorSignalProxyInfo.m_oActive)
    {
        if (m_oLCErrorSignalProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oLCErrorSignalProxyInfo.m_oInstance == instance)
            {
                IncomingLCErrorSignal(value);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

   if (m_oLCReadySignalProxyInfo.m_oActive)
    {
        if (m_oLCReadySignalProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oLCReadySignalProxyInfo.m_oInstance == instance)
            {
                IncomingLCReadySignal(value);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

   if (m_oLCLimitWarningProxyInfo.m_oActive)
    {
        if (m_oLCLimitWarningProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oLCLimitWarningProxyInfo.m_oInstance == instance)
            {
                IncomingLCLimitWarning(value);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

   if (m_oLEDTemperatureHighProxyInfo.m_oActive)
    {
        if (m_oLEDTemperatureHighProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oLEDTemperatureHighProxyInfo.m_oInstance == instance)
            {
                IncomingLEDTemperatureHigh(value);
            }
        }
        else
        {
            wmLog(eDebug, "wrong productIndex for LEDTemperatureHigh\n");
        }
    }
}

void WeldingHeadControl::ecatAnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t statusCH1, uint16_t valueCH1, uint8_t statusCH2, uint16_t valueCH2) // Interface EthercatInputs
{
    for(int i = eAnalogIn1;i <= eLastAnalogIn;i++)
    {
        if (m_oGenPurposeAnaInProxyInfo[i].m_oActive)
        {
            if (m_oGenPurposeAnaInProxyInfo[i].m_oProductIndex == productIndex)
            {
                if (m_oGenPurposeAnaInProxyInfo[i].m_oInstance == instance)
                {
                    if (m_oGenPurposeAnaInProxyInfo[i].m_oChannel == eChannel1)
                    {
                        m_oGenPurposeAnaIn[i].store((int16_t)valueCH1);
                    }
                    else if (m_oGenPurposeAnaInProxyInfo[i].m_oChannel == eChannel2)
                    {
                        m_oGenPurposeAnaIn[i].store((int16_t)valueCH2);
                    }
                    else
                    {
                        // falscher channel !
                    }
                }
            }
            else
            {
                // falscher productIndex !
            }
        }
    }

    if (m_oLaserPowerSignalProxyInfo.m_oActive)
    {
        if (m_oLaserPowerSignalProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oLaserPowerSignalProxyInfo.m_oInstance == instance)
            {
                if (m_oLaserPowerSignalProxyInfo.m_oChannel == eChannel1)
                {
                    m_oLaserPowerSignal.store((int16_t)valueCH1);
                }
                else if (m_oLaserPowerSignalProxyInfo.m_oChannel == eChannel2)
                {
                    m_oLaserPowerSignal.store((int16_t)valueCH2);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

    if (m_oRobotTrackSpeedProxyInfo.m_oActive)
    {
        if (m_oRobotTrackSpeedProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oRobotTrackSpeedProxyInfo.m_oInstance == instance)
            {
                if (m_oRobotTrackSpeedProxyInfo.m_oChannel == eChannel1)
                {
                    m_oRobotTrackSpeed.store((int16_t)valueCH1);
                }
                else if (m_oRobotTrackSpeedProxyInfo.m_oChannel == eChannel2)
                {
                    m_oRobotTrackSpeed.store((int16_t)valueCH2);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void WeldingHeadControl::ecatAnalogOversamplingInCH1(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) // Interface EthercatInputs
{
    // Oversampling Signal 1
    if (m_oOversamplingSignal1ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal1ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal1ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal1ProxyInfo.m_oChannel == eChannel1)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal1RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal1RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal1RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 2
    if (m_oOversamplingSignal2ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal2ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal2ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal2ProxyInfo.m_oChannel == eChannel1)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal2RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal2RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal2RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 3
    if (m_oOversamplingSignal3ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal3ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal3ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal3ProxyInfo.m_oChannel == eChannel1)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal3RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal3RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal3RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 4
    if (m_oOversamplingSignal4ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal4ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal4ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal4ProxyInfo.m_oChannel == eChannel1)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal4RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal4RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal4RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 5
    if (m_oOversamplingSignal5ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal5ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal5ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal5ProxyInfo.m_oChannel == eChannel1)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal5RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal5RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal5RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 6
    if (m_oOversamplingSignal6ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal6ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal6ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal6ProxyInfo.m_oChannel == eChannel1)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal6RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal6RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal6RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 7
    if (m_oOversamplingSignal7ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal7ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal7ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal7ProxyInfo.m_oChannel == eChannel1)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal7RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal7RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal7RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 8
    if (m_oOversamplingSignal8ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal8ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal8ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal8ProxyInfo.m_oChannel == eChannel1)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal8RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal8RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal8RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void WeldingHeadControl::ecatAnalogOversamplingInCH2(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) // Interface EthercatInputs
{
    // Oversampling Signal 1
    if (m_oOversamplingSignal1ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal1ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal1ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal1ProxyInfo.m_oChannel == eChannel2)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal1RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal1RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal1RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 2
    if (m_oOversamplingSignal2ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal2ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal2ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal2ProxyInfo.m_oChannel == eChannel2)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal2RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal2RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal2RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 3
    if (m_oOversamplingSignal3ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal3ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal3ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal3ProxyInfo.m_oChannel == eChannel2)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal3RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal3RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal3RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 4
    if (m_oOversamplingSignal4ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal4ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal4ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal4ProxyInfo.m_oChannel == eChannel2)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal4RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal4RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal4RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 5
    if (m_oOversamplingSignal5ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal5ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal5ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal5ProxyInfo.m_oChannel == eChannel2)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal5RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal5RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal5RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 6
    if (m_oOversamplingSignal6ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal6ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal6ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal6ProxyInfo.m_oChannel == eChannel2)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal6RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal6RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal6RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 7
    if (m_oOversamplingSignal7ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal7ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal7ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal7ProxyInfo.m_oChannel == eChannel2)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal7RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal7RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal7RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
    // Oversampling Signal 8
    if (m_oOversamplingSignal8ProxyInfo.m_oActive)
    {
        if (m_oOversamplingSignal8ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oOversamplingSignal8ProxyInfo.m_oInstance == instance)
            {
                if (m_oOversamplingSignal8ProxyInfo.m_oChannel == eChannel2)
                {
                    FastMutex::ScopedLock lock(m_oOversamplingSignal8RingBufferMutex);
                    for(auto value: data)
                    {
                        m_oOversamplingSignal8RingBuffer.Write(value);
                    }
                    m_oOversamplingSignal8RingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void WeldingHeadControl::ecatLWMCh1PlasmaIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) // Interface EthercatInputs
{
    // LWM40 Plasma signal
    if (m_oLWM40_1_PlasmaProxyInfo.m_oActive)
    {
        if (m_oLWM40_1_PlasmaProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oLWM40_1_PlasmaProxyInfo.m_oInstance == instance)
            {
                if (m_oLWM40_1_PlasmaProxyInfo.m_oChannel == eChannel1)
                {
                    FastMutex::ScopedLock lock(m_oLWM40_1_PlasmaRingBufferMutex);
                    for(auto value: data)
                    {
                        m_oLWM40_1_PlasmaRingBuffer.Write(value);
                    }
                    m_oLWM40_1_PlasmaRingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void WeldingHeadControl::ecatLWMCh2TempIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) // Interface EthercatInputs
{
    // LWM40 Temperature signal
    if (m_oLWM40_1_TemperatureProxyInfo.m_oActive)
    {
        if (m_oLWM40_1_TemperatureProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oLWM40_1_TemperatureProxyInfo.m_oInstance == instance)
            {
                if (m_oLWM40_1_TemperatureProxyInfo.m_oChannel == eChannel2)
                {
                    FastMutex::ScopedLock lock(m_oLWM40_1_TemperatureRingBufferMutex);
                    for(auto value: data)
                    {
                        m_oLWM40_1_TemperatureRingBuffer.Write(value);
                    }
                    m_oLWM40_1_TemperatureRingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void WeldingHeadControl::ecatLWMCh3BackRefIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) // Interface EthercatInputs
{
    // LWM40 BackReflection signal
    if (m_oLWM40_1_BackReflectionProxyInfo.m_oActive)
    {
        if (m_oLWM40_1_BackReflectionProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oLWM40_1_BackReflectionProxyInfo.m_oInstance == instance)
            {
                if (m_oLWM40_1_BackReflectionProxyInfo.m_oChannel == eChannel3)
                {
                    FastMutex::ScopedLock lock(m_oLWM40_1_BackReflectionRingBufferMutex);
                    for(auto value: data)
                    {
                        m_oLWM40_1_BackReflectionRingBuffer.Write(value);
                    }
                    m_oLWM40_1_BackReflectionRingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void WeldingHeadControl::ecatLWMCh4AnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT16 &data) // Interface EthercatInputs
{
    // LWM40 AnalogInput signal
    if (m_oLWM40_1_AnalogInputProxyInfo.m_oActive)
    {
        if (m_oLWM40_1_AnalogInputProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oLWM40_1_AnalogInputProxyInfo.m_oInstance == instance)
            {
                if (m_oLWM40_1_AnalogInputProxyInfo.m_oChannel == eChannel4)
                {
                    FastMutex::ScopedLock lock(m_oLWM40_1_AnalogInputRingBufferMutex);
                    for(auto value: data)
                    {
                        m_oLWM40_1_AnalogInputRingBuffer.Write(value);
                    }
                    m_oLWM40_1_AnalogInputRingBuffer.SetValuesPerECATCycle(size);
                }
                else
                {
                    // falscher channel !
                }
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void WeldingHeadControl::ecatEncoderIn(EcatProductIndex productIndex, EcatInstance instance, uint16_t status, uint32_t counterValue, uint32_t latchValue) // Interface EthercatInputs
{
   if (m_oEncoderInput1ProxyInfo.m_oActive)
    {
        if (m_oEncoderInput1ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oEncoderInput1ProxyInfo.m_oInstance == instance)
            {
                m_oEncoderInput1.store((uint32_t)counterValue);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

   if (m_oEncoderInput2ProxyInfo.m_oActive)
    {
        if (m_oEncoderInput2ProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oEncoderInput2ProxyInfo.m_oInstance == instance)
            {
                m_oEncoderInput2.store((uint32_t)counterValue);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

// thread for waiting for end of driving Z-Collimator Version 2
static void *ZCollEndOfDrivingThread(void* p_pArg)
{
    prctl(PR_SET_NAME, "ZCWaitForEndOfDriving");
    pthread_detach(pthread_self());

    WeldingHeadControl* pWeldingHeadControl = static_cast<WeldingHeadControl *>(p_pArg);

    pWeldingHeadControl->ZCWaitForEndOfDriving();
    return NULL;
}

/***************************************************************************/
/* StartSendSensorDataThread                                               */
/***************************************************************************/

void WeldingHeadControl::StartSendSensorDataThread(unsigned long p_oCycleTimeSendSensorData_ns)
{
    ///////////////////////////////////////////////////////
    // start thread for sending sensor data
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    if (pthread_attr_init(&oPthreadAttr) != 0)
    {
        char errorBuffer[256];
        wmLog(eDebug, "pthread_attr_init failed (%s)(%s)\n", "001", strerror_r(errno, errorBuffer, sizeof(errorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.SetThreadAttrFail", "Cannot set thread attributes (%s)\n", "001");
    }

    system::makeThreadRealTime(system::Priority::Sensors, &oPthreadAttr);

    m_oDataToSendSensorDataThread.m_pWeldingHeadControl = this;
    m_oDataToSendSensorDataThread.m_oCycleTimeSendSensorData_ns = p_oCycleTimeSendSensorData_ns;

    if (pthread_create(&m_oSendSensorDataThread_ID, &oPthreadAttr, &SendSensorDataThread, &m_oDataToSendSensorDataThread) != 0)
    {
        char oStrErrorBuffer[256];

        wmLog(eDebug, "pthread_create failed (%s)(%s)\n", "002", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMCreateThreadFail", "Cannot start thread (%s)\n", "002");
    }
}

/***************************************************************************/
/* StopSendSensorDataThread                                                */
/***************************************************************************/

void WeldingHeadControl::StopSendSensorDataThread(void)
{
    ///////////////////////////////////////////////////////
    // stop thread for sending sensor data
    ///////////////////////////////////////////////////////
    if (m_oSendSensorDataThread_ID != 0)
    {
        if (pthread_cancel(m_oSendSensorDataThread_ID) != 0)
        {
            wmLog(eDebug, "was not able to abort thread\n");
        }
        else
        {
            m_oSendSensorDataThread_ID = 0;
        }
    }

}

/***************************************************************************/
/* SendSensorDataFunction                                                  */
/***************************************************************************/

void WeldingHeadControl::SendSensorDataFunction(void)
{
//wmLog(eDebug, "WeldingHeadControl::SendSensorDataFunction\n");

    // analog inputs
    for(int i = eAnalogIn1;i <= eAnalogIn8;i++)
    {
        if (m_oGenPurposeAnaInProxyInfo[i].m_oActive)
        {
            IncomingGenPurposeAnaIn_V2(static_cast<AnalogInputNo>(i));
        }
    }
    if (m_oLaserPowerSignalProxyInfo.m_oActive)
    {
        IncomingLaserPowerSignal_V2();
    }
    if (m_oRobotTrackSpeedProxyInfo.m_oActive)
    {
        IncomingRobotTrackSpeed_V2();
    }
    if (m_oEncoderInput1ProxyInfo.m_oActive)
    {
        IncomingEncoderInput1_V2();
    }
    if (m_oEncoderInput2ProxyInfo.m_oActive)
    {
        IncomingEncoderInput2_V2();
    }

    if (m_oOversamplingSignal1ProxyInfo.m_oActive)
    {
        IncomingOversamplingSignal1_V2();
    }
    if (m_oOversamplingSignal2ProxyInfo.m_oActive)
    {
        IncomingOversamplingSignal2_V2();
    }
    if (m_oOversamplingSignal3ProxyInfo.m_oActive)
    {
        IncomingOversamplingSignal3_V2();
    }
    if (m_oOversamplingSignal4ProxyInfo.m_oActive)
    {
        IncomingOversamplingSignal4_V2();
    }
    if (m_oOversamplingSignal5ProxyInfo.m_oActive)
    {
        IncomingOversamplingSignal5_V2();
    }
    if (m_oOversamplingSignal6ProxyInfo.m_oActive)
    {
        IncomingOversamplingSignal6_V2();
    }
    if (m_oOversamplingSignal7ProxyInfo.m_oActive)
    {
        IncomingOversamplingSignal7_V2();
    }
    if (m_oOversamplingSignal8ProxyInfo.m_oActive)
    {
        IncomingOversamplingSignal8_V2();
    }

    if (m_oLWM40_1_PlasmaProxyInfo.m_oActive)
    {
        IncomingLWM40_1_Plasma_V2();
    }
    if (m_oLWM40_1_TemperatureProxyInfo.m_oActive)
    {
        IncomingLWM40_1_Temperature_V2();
    }
    if (m_oLWM40_1_BackReflectionProxyInfo.m_oActive)
    {
        IncomingLWM40_1_BackReflection_V2();
    }
    if (m_oLWM40_1_AnalogInputProxyInfo.m_oActive)
    {
        IncomingLWM40_1_AnalogInput_V2();
    }

    if (m_pStatesHeadX != nullptr)
    {
        m_pStatesHeadX->IncomingMotionData();
    }
    if (m_pStatesHeadY != nullptr)
    {
        m_pStatesHeadY->IncomingMotionData();
    }
    if (m_pStatesHeadZ != nullptr)
    {
        m_pStatesHeadZ->IncomingMotionData();
    }

    if (isScanlabScannerEnabled())
    {
        IncomingScannerXPosition_V2();
        IncomingScannerYPosition_V2();
        IncomingFiberSwitchPosition_V2();
        IncomingScannerWeldingFinished_V2();
        IncomingContourPreparedFinished();
    }
    if (isZCollimatorEnabled())
    {
        IncomingZCPositionDigV1_V2();
    }
}

/***************************************************************************/
/* SendSensorDataThread                                                    */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void *SendSensorDataThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "SendSensorData");
    pthread_detach(pthread_self());
    struct timespec oWakeupTime;
    int retValue;

    auto pDataToSendSensorDataThread = static_cast<struct DataToSendSensorDataThread *>(p_pArg);
    auto pWeldingHeadControl = pDataToSendSensorDataThread->m_pWeldingHeadControl;
    unsigned long oCycleTimeSendSensorData_ns = pDataToSendSensorDataThread->m_oCycleTimeSendSensorData_ns;

    wmLog(eDebug, "SendSensorDataThread is started: Cycle Time: %d\n", oCycleTimeSendSensorData_ns);

    // calculate time for next send
    clock_gettime(CLOCK_TO_USE, &oWakeupTime);
    oWakeupTime.tv_nsec += oCycleTimeSendSensorData_ns;
    while(oWakeupTime.tv_nsec >= NSEC_PER_SEC)
    {
        oWakeupTime.tv_nsec -= NSEC_PER_SEC;
        oWakeupTime.tv_sec++;
    }

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pWeldingHeadControl->SendSensorDataFunction();
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);

    while(true)
    {
        retValue = clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &oWakeupTime, NULL);
        if (retValue)
        {
            char oStrErrorBuffer[256];

            wmLog(eDebug, "clock_nanosleep failed (%s)\n", strerror_r(retValue, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
            wmLogTr(eError, "QnxMsg.VI.IDMCyclicSleepFail", "Sleeping time for cycle loop failed\n");
            break;
        }

#if CYCLE_TIMING_VIA_SERIAL_PORT
        static bool oToggleCyclicTask = true;
        if (oToggleCyclicTask)
        {
            int oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIS, &g_oDTR01_flag);
            if (oRetVal != 0)
            {
                printf("Error in ioctl\n");
                perror("");
            }
            oToggleCyclicTask = false;
        }
        else
        {
            int oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIC, &g_oDTR01_flag);
            if (oRetVal != 0)
            {
                printf("Error in ioctl\n");
                perror("");
            }
            oToggleCyclicTask = true;
        }
#endif

#if CYCLE_TIMING_VIA_SERIAL_PORT
        int oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIS, &g_oRTS02_flag);
        if (oRetVal != 0)
        {
            printf("Error in ioctl\n");
            perror("");
        }
#endif
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
        pWeldingHeadControl->SendSensorDataFunction();
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
#if CYCLE_TIMING_VIA_SERIAL_PORT
        oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIC, &g_oRTS02_flag);
        if (oRetVal != 0)
        {
            printf("Error in ioctl\n");
            perror("");
        }
#endif

        // calculate time for next send
        oWakeupTime.tv_nsec += oCycleTimeSendSensorData_ns;
        while(oWakeupTime.tv_nsec >= NSEC_PER_SEC)
        {
            oWakeupTime.tv_nsec -= NSEC_PER_SEC;
            oWakeupTime.tv_sec++;
        }
    }

    return NULL;
}

/***************************************************************************/
/* StartZCPositionDigV1ReadOutThread                                       */
/***************************************************************************/

void WeldingHeadControl::StartZCPositionDigV1ReadOutThread(void)
{
    ///////////////////////////////////////////////////////
    // start thread for read out ZC position
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToZCPositionDigV1ReadOutThread.m_pWeldingHeadControl = this;

    if (pthread_create(&m_oZCPositionDigV1ReadOutThread_ID, &oPthreadAttr, &ZCPositionDigV1ReadOutThread, &m_oDataToZCPositionDigV1ReadOutThread) != 0)
    {
        char oStrErrorBuffer[256];

        wmLog(eDebug, "pthread_create failed (%s)(%s)\n", "002", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMCreateThreadFail", "Cannot start thread (%s)\n", "002");
    }
}

/***************************************************************************/
/* StopZCPositionDigV1ReadOutThread                                        */
/***************************************************************************/

void WeldingHeadControl::StopZCPositionDigV1ReadOutThread(void)
{
    ///////////////////////////////////////////////////////
    // stop thread for read out ZC position
    ///////////////////////////////////////////////////////
    if (m_oZCPositionDigV1ReadOutThread_ID != 0)
    {
        if (pthread_cancel(m_oZCPositionDigV1ReadOutThread_ID) != 0)
        {
            wmLog(eDebug, "was not able to abort thread\n");
        }
        else
        {
            m_oZCPositionDigV1ReadOutThread_ID = 0;
        }
    }

}

/***************************************************************************/
/* ZCPositionDigV1ReadOutFunction                                          */
/***************************************************************************/

void WeldingHeadControl::ZCPositionDigV1ReadOutFunction(void)
{
#if EMVTEST_NEW_ZCOLLIMATOR
    int32_t oPosition = 0;
    if (isZCollimatorEnabled() && (getZCollimatorType() == eZColliDigitalV1))
    {
        oPosition = static_cast<int32_t>( std::lround(m_oTmlCommunicator.getActualPositionUm() ) ); // needs approx. 1ms
    }
    else
    {
        oPosition = 0;
    }
    m_oZCPositionDigV1ReadOut.store(oPosition);
#endif
}

/***************************************************************************/
/* ZCPositionDigV1ReadOutThread                                            */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void *ZCPositionDigV1ReadOutThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "ZCPositionDigV1ReadOut");
    pthread_detach(pthread_self());

    auto pDataToZCPositionDigV1ReadOutThread = static_cast<struct DataToZCPositionDigV1ReadOutThread *>(p_pArg);
    auto pWeldingHeadControl = pDataToZCPositionDigV1ReadOutThread->m_pWeldingHeadControl;

    wmLog(eDebug, "ZCPositionDigV1ReadOut is started\n");

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pWeldingHeadControl->ZCPositionDigV1ReadOutFunction();
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);

    while(true)
    {
        usleep(10*1000); // 10 ms polling time

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
        pWeldingHeadControl->ZCPositionDigV1ReadOutFunction();
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    }

    return NULL;
}

void WeldingHeadControl::saveKeyValueToWeldHeadDefaults(const std::string& key, int value)
{
    WeldHeadDefaults::instance().setInt(key, value);
}

} // namespace ethercat

} // namespace precitec

