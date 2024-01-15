/*
 * VI_InspectionControl.cpp
 *
 *  Created on: 08.04.2010
 *      Author: f.agrawal, a.egger
 */

#include <module/moduleLogger.h>

#include "viInspectionControl/VI_InspectionControl.h"
#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"
#include <sys/prctl.h>
#include <chrono>
#include "../include/viInspectionControl/dbNotificationServer.h"

namespace precitec
{

    using namespace interface;

namespace ethercat
{

#define SIGNAL_CHANGE_DELTA 10

#define PULSE_WIDTH_ERROR_SIGNAL_CONTINUOUS   50
#define DEBUG_AUTOINSPECT_STATEMACHINE 0

#define DEBUG_SCANMASTER               0

// folgende CLOCK... verwenden
#define CLOCK_TO_USE CLOCK_MONOTONIC

// folgendes definiert die Anzahl ns pro Sekunde
#define NSEC_PER_SEC    (1000000000)

// Folgendes definiert eine Zykluszeit von 1ms
#define CYCLE_TIME_NS   (1000000)

#define CYCLE_TIMING_VIA_SERIAL_PORT     0

#define S6K_DURATION_OF_RESULTS_DIALOG   40.0

#define LWM_DEVICE_TRIGGER_SIMULATION    0

static void *ZCollAutoHomingThread(void* arg);

void* CyclicTaskThread(void *p_pArg);

void* SendSensorDataThread(void *p_pArg);

///////////////////////////////////////////////////////////
// global variables for debugging purposes
///////////////////////////////////////////////////////////

#if CYCLE_TIMING_VIA_SERIAL_PORT
int g_oDebugSerialFd;
int g_oDTR01_flag;
int g_oRTS02_flag;
#endif

VI_InspectionControl::VI_InspectionControl(TInspection<EventProxy>& _inspectionProxy,
										   TInspectionCmd<EventProxy>& _inspectCmdProxy,
										   TSensor<AbstractInterface> &sensorProxy,
										   TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy,
										   TS6K_InfoToProcesses<EventProxy>& p_rS6K_InfoToProcessesProxy,
						 				   TDb<MsgProxy>& p_rDbProxy,
                                           DbNotificationServer* p_rDBNotificationServer
                                          ):

	inspectionProxy(_inspectionProxy),
	inspectionCmdProxy(_inspectCmdProxy),
	m_oSensorProxy (sensorProxy),
	m_rEthercatOutputsProxy(p_rEthercatOutputsProxy),
	m_rS6K_InfoToProcessesProxy(p_rS6K_InfoToProcessesProxy),
	m_rDbProxy(p_rDbProxy),	
	m_imageNrGenPurposeDigIn1(0),
	m_pValuesGenPurposeDigIn1(nullptr),
	m_imageNrGenPurposeDigInMultiple(0),
	m_pValuesGenPurposeDigInMultiple(nullptr),
	m_imageNrS6K_CS_LeadingResults(0),
	m_pValuesS6K_CS_LeadingResults1(nullptr),
	m_pValuesS6K_CS_LeadingResults2(nullptr),
	m_pValuesS6K_CS_LeadingResults3(nullptr),
	m_pValuesS6K_CS_LeadingResults4(nullptr),
	m_oTriggerEmergencyStop(false),
	m_oInspectCycleIsOn(false),
	m_oInspectInfoIsOn(false),
	m_oFirstInspectInfoSet(false),
	m_oInspectSeamIsOn(false),
	m_oSeamHasNoResults(false),
	m_oInspectionIncomplete(false),
	m_oWaitingForCycleAckn(false),
	m_oInspectCycleAck(false),
	m_simulateHW(false),
	m_oCycleAcknTimeoutCounter(0),
	m_oAutoInspectCycleStarted(false),
	m_oAutoInspectCycleStopped(false),
	m_oAutoInspectCycleAckn(false),
    m_oProductTypeContinuously(0),
    m_oProductNumberContinuously(0),
    m_oInspectTimeMsContinuously(100),
    m_oAutoInspectIsFirstCycle(true),
    m_oGatewayDataLength(20),
	m_oPositionResults(0),
	m_oGenPurposeDigOut1(0),
	m_oGenPurposeDigOut2(0),
	m_oGenPurposeDigOut3(0),
	m_oGenPurposeDigOut4(0),
	m_oGenPurposeDigOut5(0),
	m_oGenPurposeDigOut6(0),
	m_oGenPurposeDigOut7(0),
	m_oGenPurposeDigOut8(0),
	m_oGenPurposeDigInAddress(0),
	m_oGenPurposeDigInValue(0),
	m_oGenPurposeDigIn1(0),
	m_oGenPurposeDigIn2(0),
	m_oGenPurposeDigIn3(0),
	m_oGenPurposeDigIn4(0),
	m_oGenPurposeDigIn5(0),
	m_oGenPurposeDigIn6(0),
	m_oGenPurposeDigIn7(0),
	m_oGenPurposeDigIn8(0),
	m_oGenPurposeDigIn1_Single(0),
	m_bGlasNotPresent(false),
	m_bGlasDirty(false),
	m_bTempGlasFail(false),
	m_bTempHeadFail(false),
	m_bGlasNotPresentCounter(0),
	m_bGlasDirtyCounter(0),
	m_bTempGlasFailCounter(0),
	m_bTempHeadFailCounter(0),
	m_pChangeToStandardModeInfo(nullptr),
	m_pSeamseriesNrInfo(NULL),
	m_pSeamNrInfo(NULL),
	m_pCalibrationTypeInfo(NULL),
	m_pProductTypeInfo(NULL),
	m_pProductNumberInfo(NULL),
	m_pExtendedProductInfoInfo(nullptr),
	m_pGetProductTypeFullInfo(NULL),
	m_pGetProductNumberFullInfo(NULL),
    m_oSendSensorDataThread_ID(0),
    m_oCyclicTaskThread_ID(0),
    m_pS6K_CycleData_Info(nullptr),
    m_oS6K_CycleDataInput(0),
    m_pS6K_SeamNo_Info(nullptr),
    m_oS6K_SeamNoInput(0),
    m_oS6K_Automatic_Seam_No_Buffer(1),
    m_oS6K_Automatic_Seam_No_TimeoutActive(false),
    m_oS6K_Automatic_Seam_No_TimeoutCounter(0),
    m_oS6K_SouvisActiveStatus(false),
    m_oS6K_SouvisInspectionStatus(false),
    m_oS6K_QuitSystemFaultStatus(false),
    m_oS6K_MachineReadyStatus(false),
    m_pS6K_ProductNumber_Info(nullptr),
    m_oS6K_ProductNumberInput(0),
    m_oS6K_AcknQualityDataStatus(false),
    m_oS6K_CycleDataValidStatus(false),
    m_oS6K_SeamNoValidStatus(false),
    m_oS6K_AcknResultDataStatus(false),
    m_oS6K_MakePicturesStatus(false),
    m_oS6K_SeamErrorCat1Accu(0xFFFFFF),
    m_oS6K_SeamErrorCat2Accu(0xFFFFFF),
    m_oS6K_RequestResultDataDialog(false),
    m_oS6K_ResultDataState(0),
    m_oS6K_ResultDataDialogActive(false),
    m_oS6K_SeamEndTimeoutActive(false),
    m_oS6K_SeamEndTimeoutCounter(0),
    m_oS6K_ImageCountUsed(12),
	m_oTriggerCalibrationBlocked(false),
	m_oSeamNoInAnalogMode(0),
	m_oDebugInfo_SOURING(false),
	m_oDebugInfo_SOURING_Extd(false),
    m_oS6K_SouvisActiveStatusBuffer(false),
    m_oS6K_MaxSouvisSpeed(300), // [100mm/min] -> here: 30000 mm/min -> 30 m/min
    m_oS6K_NumberOfPresentSeams(1),
    m_oS6K_SouvisPresent(true),
    m_oS6K_SouvisSelected(true),
    m_oSM_StepCount(1),
    m_oSM_SeamCount(1),
    m_oSM_StartSeamserieSequence(false),
    m_oSM_SeamseriesNumber(0),
    m_oSM_TimePerSeam(100),
    m_oSM_BurstWasReceived(false),
    m_oSM_SeamProcessingEnded(false),
    m_oSM_SeamProcessingEndSeamNumber(0),
    m_oSM_TimeoutOccured(false),
    m_oSM_AcknowledgeStepStatus(false),
    m_oDebugInfo_SCANMASTER(false),
    m_pS6K_CS_LeadingResultsInput(nullptr),
    m_pS6K_CS_LeadingResultsOutput(nullptr),
    m_oS6K_CS_CurrMeasuresPerResult(1),
    m_oControlSimulationIsOn(false),
    m_spotWeldingIsOn(false),
	m_oProductType(1), // ProductType 0 ist Live-Produkt !
	m_oProductNumber(0),
	m_oExtendedProductInfo(100, 0x00),
	m_oChangeToStandardMode(false),
	m_oSeamNr(0),
	m_oSeamNrShadow(0),
	m_oNoSeams(1),
	m_oNoSeamSeries(1),
	m_oNoProducts(1),
	m_oSeamseries(0),
	m_oSeamseriesShadow(0),
	m_oCalibrationType(0),
	m_oSumErrorLatchedAccu(false),
	m_oSumErrorSeamSeriesAccu(false),
	m_oSumErrorSeamSeriesCounter(0),
	m_oSumErrorSeamSeriesIsHigh(false),
	m_oSumErrorSeamAccu(false),
	m_oQualityErrorAccuCycle(0x0000),
	m_oQualityErrorAccuSeamSeries(0x0000),
	m_oQualityErrorAccuSeam(0x0000),
	m_oCalibResultsBuffer(0x00),
	m_oSystemErrorAccu(0x0000),
	m_oAcknResultsReadyFull(false),
	m_oProductTypeFull(1), // ProductType 0 ist Live-Produkt !
	m_oProductNumberFull(0),
	m_oProductNumberFromUserEnable(0),
	m_oAnalogTriggerLevelVolt(5.0),
	m_oAnalogTriggerLevelBin(16383),
    m_dbNotificationServer(p_rDBNotificationServer)
{
	// SystemConfig Switch for GenPurposeDigOut1
	m_oGenPurposeDigOut1Enable = SystemConfiguration::instance().getBool("GenPurposeDigOut1Enable", false);
	wmLog(eDebug, "m_oGenPurposeDigOut1Enable (bool): %d\n", m_oGenPurposeDigOut1Enable);
	// SystemConfig Switch for SeamEndDigOut1
	m_oSeamEndDigOut1Enable = SystemConfiguration::instance().getBool("SeamEndDigOut1Enable", false);
	wmLog(eDebug, "m_oSeamEndDigOut1Enable (bool): %d\n", m_oSeamEndDigOut1Enable);
	// SystemConfig Switch for GenPurposeDigOutMultiple
	m_oGenPurposeDigOutMultipleEnable = SystemConfiguration::instance().getBool("GenPurposeDigOutMultipleEnable", false);
	wmLog(eDebug, "m_oGenPurposeDigOutMultipleEnable (bool): %d\n", m_oGenPurposeDigOutMultipleEnable);
	// SystemConfig Switch for GenPurposeDigIn1
	m_oGenPurposeDigIn1Enable = SystemConfiguration::instance().getBool("GenPurposeDigIn1Enable", false);
	wmLog(eDebug, "m_oGenPurposeDigIn1Enable (bool): %d\n", m_oGenPurposeDigIn1Enable);
	// SystemConfig Switch for GenPurposeDigInMultiple
	m_oGenPurposeDigInMultipleEnable = SystemConfiguration::instance().getBool("GenPurposeDigInMultipleEnable", false);
	wmLog(eDebug, "m_oGenPurposeDigInMultipleEnable (bool): %d\n", m_oGenPurposeDigInMultipleEnable);
	// SystemConfig Switch for ProductNumberFromWMEnable
	m_oProductNumberFromWMEnable = SystemConfiguration::instance().getBool("ProductNumberFromWMEnable", false);
	wmLog(eDebug, "m_oProductNumberFromWMEnable (bool): %d\n", m_oProductNumberFromWMEnable);
	// SystemConfig Switch for CabinetTemperatureOkEnable
	m_oCabinetTemperatureOkEnable = SystemConfiguration::instance().getBool("CabinetTemperatureOkEnable", false);
	wmLog(eDebug, "m_oCabinetTemperatureOkEnable (bool): %d\n", m_oCabinetTemperatureOkEnable);

	// SystemConfig Switches for Fieldbus variations and functions
    m_oContinuouslyModeActive = SystemConfiguration::instance().getBool("ContinuouslyModeActive", false);
	wmLog(eDebug, "m_oContinuouslyModeActive (bool): %d\n", m_oContinuouslyModeActive);
	m_oFieldbusInterfaceStandard = SystemConfiguration::instance().getBool("FieldbusInterfaceStandard", false);
	wmLog(eDebug, "m_oFieldbusInterfaceStandard (bool): %d\n", m_oFieldbusInterfaceStandard);
	m_oFieldbusInterfaceLight = SystemConfiguration::instance().getBool("FieldbusInterfaceLight", false);
	wmLog(eDebug, "m_oFieldbusInterfaceLight (bool): %d\n", m_oFieldbusInterfaceLight);
	m_oFieldbusInterfaceFull = SystemConfiguration::instance().getBool("FieldbusInterfaceFull", false);
	wmLog(eDebug, "m_oFieldbusInterfaceFull (bool): %d\n", m_oFieldbusInterfaceFull);
	m_oFieldbusExtendedProductInfo = SystemConfiguration::instance().getBool("FieldbusExtendedProductInfo", false);
	wmLog(eDebug, "m_oFieldbusExtendedProductInfo (bool): %d\n", m_oFieldbusExtendedProductInfo);
	m_oProductTypeOutOfInterfaceFull = SystemConfiguration::instance().getBool("ProductTypeOutOfInterfaceFull", false);
	wmLog(eDebug, "m_oProductTypeOutOfInterfaceFull (bool): %d\n", m_oProductTypeOutOfInterfaceFull);
	m_oProductNumberOutOfInterfaceFull = SystemConfiguration::instance().getBool("ProductNumberOutOfInterfaceFull", false);
	wmLog(eDebug, "m_oProductNumberOutOfInterfaceFull (bool): %d\n", m_oProductNumberOutOfInterfaceFull);
	m_oQualityErrorFieldOnSeamSeries = SystemConfiguration::instance().getBool("QualityErrorFieldOnSeamSeries", false);
	wmLog(eDebug, "m_oQualityErrorFieldOnSeamSeries (bool): %d\n", m_oQualityErrorFieldOnSeamSeries);
	if (isFieldbusInterfaceStandardEnabled() && isFieldbusInterfaceFullEnabled())
	{
		wmLogTr(eError, "QnxMsg.VI.IFFullAndIFStd", "Fieldbus Interface Standard cannot be combined with Fieldbus Interface Full !\n");
	}

	// SystemConfig Switches for HeadMonitorGateway
	m_oHeadMonitorGatewayEnable = SystemConfiguration::instance().getBool("HeadMonitorGatewayEnable", false);
	wmLog(eDebug, "m_oHeadMonitorGatewayEnable (bool): %d\n", m_oHeadMonitorGatewayEnable);
	m_oHeadMonitorSendsNotReady = SystemConfiguration::instance().getBool("HeadMonitorSendsNotReady", false);
	wmLog(eDebug, "m_oHeadMonitorSendsNotReady (bool): %d\n", m_oHeadMonitorSendsNotReady);

	// SystemConfig Switch for switching off NIOResults
	m_oNIOResultSwitchedOff = SystemConfiguration::instance().getBool("NIOResultSwitchedOff", false);
	wmLog(eDebug, "m_oNIOResultSwitchedOff (bool): %d\n", m_oNIOResultSwitchedOff);

    // SystemConfig Switches for SOUVIS6000 application and functions
    m_oIsSOUVIS6000_Application = SystemConfiguration::instance().getBool("SOUVIS6000_Application", false);
    wmLog(eDebug, "m_oIsSOUVIS6000_Application (bool): %d\n", m_oIsSOUVIS6000_Application);
    m_oSOUVIS6000_Machine_Type = static_cast<SOUVIS6000MachineType>(SystemConfiguration::instance().getInt("SOUVIS6000_Machine_Type", 0) );
    wmLog(eDebug, "m_oSOUVIS6000_Machine_Type (int): %d\n", static_cast<int>(m_oSOUVIS6000_Machine_Type));
    m_oSOUVIS6000_Is_PreInspection = SystemConfiguration::instance().getBool("SOUVIS6000_Is_PreInspection", false);
    wmLog(eDebug, "m_oSOUVIS6000_Is_PreInspection (bool): %d\n", m_oSOUVIS6000_Is_PreInspection);
    m_oSOUVIS6000_Is_PostInspection_Top = SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Top", false);
    wmLog(eDebug, "m_oSOUVIS6000_Is_PostInspection_Top (bool): %d\n", m_oSOUVIS6000_Is_PostInspection_Top);
    m_oSOUVIS6000_Is_PostInspection_Bottom = SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Bottom", false);
    wmLog(eDebug, "m_oSOUVIS6000_Is_PostInspection_Bottom (bool): %d\n", m_oSOUVIS6000_Is_PostInspection_Bottom);
    m_oSOUVIS6000_Automatic_Seam_No = SystemConfiguration::instance().getBool("SOUVIS6000_Automatic_Seam_No", false);
    wmLog(eDebug, "m_oSOUVIS6000_Automatic_Seam_No (bool): %d\n", m_oSOUVIS6000_Automatic_Seam_No);
    m_oSOUVIS6000_CrossSectionMeasurementEnable = SystemConfiguration::instance().getBool("SOUVIS6000_CrossSectionMeasurementEnable", false);
    wmLog(eDebug, "m_oSOUVIS6000_CrossSectionMeasurementEnable (bool): %d\n", m_oSOUVIS6000_CrossSectionMeasurementEnable);
    m_oSOUVIS6000_CrossSection_Leading_System = SystemConfiguration::instance().getBool("SOUVIS6000_CrossSection_Leading_System", false);
    wmLog(eDebug, "m_oSOUVIS6000_CrossSection_Leading_System (bool): %d\n", m_oSOUVIS6000_CrossSection_Leading_System);

    // SystemConfig Switches for SCANMASTER application and functions
    m_oIsSCANMASTER_Application = SystemConfiguration::instance().getBool("SCANMASTER_Application", false);
    wmLog(eDebug, "m_oIsSCANMASTER_Application (bool): %d\n", m_oIsSCANMASTER_Application);
    m_oIsSCANMASTER_Application_BootupState = m_oIsSCANMASTER_Application;
    m_oIsSCANMASTER_ThreeStepInterface = SystemConfiguration::instance().getBool("SCANMASTER_ThreeStepInterface", false);
    wmLog(eDebug, "m_oIsSCANMASTER_ThreeStepInterface (bool): %d\n", m_oIsSCANMASTER_ThreeStepInterface);
    if (isSCANMASTER_ThreeStepInterface())
    {
        m_oSM_StepCount.store(3);
    }
    m_oIsSCANMASTER_GeneralApplication = SystemConfiguration::instance().getBool("SCANMASTER_GeneralApplication", false);
    wmLog(eDebug, "m_oIsSCANMASTER_GeneralApplication (bool): %d\n", m_oIsSCANMASTER_GeneralApplication);
    if ( isSCANMASTER_ThreeStepInterface() && isSCANMASTER_GeneralApplication() )
    {
        wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Only one Subproject of SCANMASTER_Application is possible !\n");
    }
    if (isSCANMASTER_Application())
    {
        if (isSCANMASTER_ThreeStepInterface())
        {
            wmLog(eWarning, "SCANMASTER_ThreeStepInterface is deprecated !\n");
            wmLog(eWarning, "SCANMASTER_ThreeStepInterface will be removed in a future release !\n");
            wmLog(eWarning, "Please switch to SCANMASTER_GeneralApplication soon !\n");
        }
    }

    // SystemConfig Switches for communication to LWM device
    m_isCommunicationToLWMDeviceActive = SystemConfiguration::instance().getBool("Communication_To_LWM_Device_Enable", false);
    wmLog(eDebug, "m_isCommunicationToLWMDeviceActive (bool): %d\n", m_isCommunicationToLWMDeviceActive);

    //############################################################################

    InspectionControlDefaults::instance().toStdOut();

    int oTempInt = InspectionControlDefaults::instance().getInt("Analog_Trigger_Level", 5000);
    m_oAnalogTriggerLevelVolt = static_cast<float>(oTempInt) / 1000.0;
    m_oAnalogTriggerLevelBin = static_cast<int>((m_oAnalogTriggerLevelVolt / 10.0) * 32767.0); // only valid for analog input terminal EL3102
    char oLoggerStrg[81];
    sprintf(oLoggerStrg, "AnalogTriggerLevel: %6.3f, %d\n", m_oAnalogTriggerLevelVolt, m_oAnalogTriggerLevelBin);
    wmLog(eDebug, "%s", oLoggerStrg);

    //############################################################################

	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeDigIn1, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeDigIn2, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeDigIn3, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeDigIn4, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeDigIn5, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeDigIn6, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeDigIn7, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eGenPurposeDigIn8, false));
	m_oSensorIdsEnabled.insert(std::make_pair(eS6K_Leading_Result1,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eS6K_Leading_Result2,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eS6K_Leading_Result3,	false));
	m_oSensorIdsEnabled.insert(std::make_pair(eS6K_Leading_Result4,	false));

	poco_assert_dbg(m_oSensorIdsEnabled.size() == eS6K_Leading_Result4 + 1 - eExternSensorMin);

    for(int i = 0;i < MAX_PROXY_PER_DEVICE;i++)
    {
        m_oDig8InProxyInfo[i].m_oActive = false;
        m_oDig8OutProxyInfo[i].m_oActive = false;
        m_oGatewayInProxyInfo[i].m_oActive = false;
        m_oGatewayOutProxyInfo[i].m_oActive = false;
        m_oAnalogInProxyInfo[i].m_oActive = false;
        m_oAnalogOversampInProxyInfo[i].m_oActive = false;
    }

    m_oHMSignalsProxyInfo.m_oActive = false;
    m_oHMOutputProxyInfo.m_oActive = false;

    for(int i = 0;i < S6K_COLLECT_LENGTH;i++)
    {
        m_oS6K_CollectResultData[i].m_oEdgePositionValue = 0;
        m_oS6K_CollectResultData[i].m_oEdgePositionImgNo = -1;
        m_oS6K_CollectResultData[i].m_oEdgePositionPos = 0;
        m_oS6K_CollectResultData[i].m_oEdgePositionValid = false;
        m_oS6K_CollectResultData[i].m_oGapWidthValue = 0;
        m_oS6K_CollectResultData[i].m_oGapWidthImgNo = -1;
        m_oS6K_CollectResultData[i].m_oGapWidthPos = 0;
        m_oS6K_CollectResultData[i].m_oGapWidthValid = false;
        m_oS6K_CollectResultData[i].m_oReadyToInsert = false;
    }
    m_oS6K_CollectResultDataWrite = 0;
    m_oS6K_CollectResultDataRead  = 0;

    m_oSM_MeasureTasks.clear();

    pthread_mutex_init(&m_oValueToSendMutex, NULL);

    if (isSOUVIS6000_Application())
    {
        if (isSOUVIS6000_CrossSectionMeasurementEnable() && !isSOUVIS6000_CrossSection_Leading_System())
        {
            for(auto oBuffer : m_oS6K_CS_LeadingResultsBufferArray)
            {
                oBuffer.m_oValid = false;
                oBuffer.m_oProductNo = 0;
                oBuffer.m_oBatchID = 0;
                oBuffer.m_oSeamNo = 0;
                oBuffer.m_oMeasuresPerResult = 1;
            }
            m_oS6K_CS_CurrentLeadingResultsInput = 0;
            m_pS6K_CS_LeadingResultsInput = &m_oS6K_CS_LeadingResultsBufferArray[m_oS6K_CS_CurrentLeadingResultsInput];
            m_oS6K_CS_CurrentLeadingResultsOutput = 0;
            m_pS6K_CS_LeadingResultsOutput = &m_oS6K_CS_LeadingResultsBufferArray[m_oS6K_CS_CurrentLeadingResultsOutput];
        }
    }

    if (isCommunicationToLWMDeviceActive())
    {
        m_TCPClientLWM = std::make_unique<TCPClientLWM>();
        if (m_TCPClientLWM == nullptr)
        {
            wmLog(eDebug, "can't create m_pTCPClientSRING !\n");
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(001)");
        }
        else
        {
            m_TCPClientLWM->initSendRequest(&m_LWMSendRequestCondVar, &m_LWMSendRequestMutex, &m_LWMSendRequestVariable);

            m_TCPClientLWM->connectSimpleIOSelectionCallback([&](void) { return LWMSimpleIOSelectionReceived(); });
            m_TCPClientLWM->connectSimpleIOStoppCallback([&](void) { return LWMSimpleIOStoppReceived(); });
            m_TCPClientLWM->connectSimpleIOTriggerCallback([&](void) { return LWMSimpleIOTriggerReceived(); });
            m_TCPClientLWM->connectSimpleIOErrorCallback([&](void) { return LWMSimpleIOErrorReceived(); });
            m_TCPClientLWM->connectResultsRangesCallback([&](void) { return LWMResultsRangesReceived(); });
            m_TCPClientLWM->connectConnectionOKCallback([&](void) { return LWMConnectionOK(); });
            m_TCPClientLWM->connectConnectionNOKCallback([&](void) { return LWMConnectionNOK(); });
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

void VI_InspectionControl::Init(bool simulateHW){

    m_simulateHW = simulateHW;

    if(!m_simulateHW){
        SAXParser parser;
        parser.setFeature(XMLReader::FEATURE_NAMESPACES, true);
        parser.setFeature(XMLReader::FEATURE_NAMESPACE_PREFIXES, true);
        parser.setContentHandler(&m_oMyVIConfigParser);
        parser.setProperty(XMLReader::PROPERTY_LEXICAL_HANDLER, static_cast<LexicalHandler*> (&m_oMyVIConfigParser));

        std::string ConfigFilename(getenv("WM_BASE_DIR"));
        ConfigFilename += "/config/VI_Config.xml";
        Poco::Path configPath( ConfigFilename );
        try
        {
            std::cout << "parsing configuration file: " << configPath.toString() << std::endl;
            wmLogTr(eInfo, "QnxMsg.VI.XMLFileRead", "parsing configuration file: %s\n", configPath.toString().c_str());
            parser.parse( configPath.toString() );
            std::cout << "configuration file successful parsed: " << configPath.toString() << std::endl;
            wmLogTr(eInfo, "QnxMsg.VI.XMLFileReadOK", "configuration file successful parsed: %s\n", configPath.toString().c_str());
        }
        catch (Poco::Exception& e)
        {
            std::cerr << "error while parsing configuration file: " << e.displayText() << std::endl;
            wmLogTr(eError, "QnxMsg.VI.XMLFileReadErr", "error while parsing configuration file: %s\n", e.displayText().c_str());
            return;
        }

        pthread_mutex_init(&m_oProductTypeMutex, NULL);
        pthread_mutex_init(&m_oProductNumberMutex, NULL);
        pthread_mutex_init(&m_oExtendedProductInfoMutex, NULL);
        pthread_mutex_init(&m_oSeamSeriesMutex, NULL);
        pthread_mutex_init(&m_oSeamNrMutex, NULL);
        pthread_mutex_init(&m_oCalibrationTypeMutex, NULL);
        pthread_mutex_init(&m_oGenPurposeDigIn1Mutex, NULL);
        pthread_mutex_init(&m_oGenPurposeDigInAddressMutex, NULL);
        pthread_mutex_init(&m_oAcknResultsReadyFullMutex, NULL);
        pthread_mutex_init(&m_oProductTypeFullMutex, NULL);
        pthread_mutex_init(&m_oProductNumberFullMutex, NULL);

        for (unsigned int i = 0; i < m_oMyVIConfigParser.m_inCommandList.size(); ++i) {
            COMMAND_INFORMATION* commandInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);

            wmLog(eDebug, "CommandType: %d\n", commandInfo->commandType);
            wmLog(eDebug, "ProductCode: 0x%x\n", commandInfo->proxyInfo.nProductCode);
            wmLog(eDebug, "VendorID:    0x%x\n", commandInfo->proxyInfo.nVendorID);
            wmLog(eDebug, "SlaveType:   %d\n", commandInfo->proxyInfo.nSlaveType);
            wmLog(eDebug, "Instance:    %d\n", commandInfo->proxyInfo.nInstance);
            wmLog(eDebug, "StartBit:    %d\n", commandInfo->proxyInfo.nStartBit);
            wmLog(eDebug, "Length:      %d\n", commandInfo->proxyInfo.nLength);
            wmLog(eDebug, "--------------------------------------------\n");

            bool foundProxy = false;
            for (unsigned int j = 0; j < m_proxyReceiverList.size(); ++j) {

                if(m_proxyReceiverList[j].proxyInfo.nProductCode == commandInfo->proxyInfo.nProductCode
                        && m_proxyReceiverList[j].proxyInfo.nVendorID == commandInfo->proxyInfo.nVendorID
                        && m_proxyReceiverList[j].proxyInfo.nInstance == commandInfo->proxyInfo.nInstance
                ){
                    foundProxy = true;
                    break;
                }
            }
            if(!foundProxy){
                //neuen receiver Proxy eintragen
                m_proxyReceiverList.push_back(*commandInfo);
            }

            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == EChangeToStandardMode)
            {
                m_pChangeToStandardModeInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == ESeamseriesNr)
            {
                m_pSeamseriesNrInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == ESeamNr)
            {
                m_pSeamNrInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == ECalibrationType)
            {
                m_pCalibrationTypeInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == EGenPurposeDigInAddress)
            {
                m_pGenPurposeDigInAddressInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == EGenPurposeDigIn1)
            {
                m_pGenPurposeDigIn1Info = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == EProductType)
            {
                m_pProductTypeInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == EProductNumber)
            {
                m_pProductNumberInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == EExtendedProductInfo)
            {
                m_pExtendedProductInfoInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }

            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == EGetProductTypeFull)
            {
                m_pGetProductTypeFullInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == EGetProductNumberFull)
            {
                m_pGetProductNumberFullInfo = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == E_S6K_CycleData)
            {
                m_pS6K_CycleData_Info = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == E_S6K_SeamNo)
            {
                m_pS6K_SeamNo_Info = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
            if (m_oMyVIConfigParser.m_inCommandList[i].commandType == E_S6K_ProductNumber)
            {
                m_pS6K_ProductNumber_Info = &(m_oMyVIConfigParser.m_inCommandList[i]);
            }
        }

        for (unsigned int i = 0; i < m_proxyReceiverList.size(); ++i) {

            //Receiver starten und registrieren
            switch (m_proxyReceiverList[i].proxyInfo.nSlaveType) {

            case _DIG_8BIT_:{
                int oInstanceIndex = (int)m_proxyReceiverList[i].proxyInfo.nInstance - 1;
                if (oInstanceIndex >= MAX_PROXY_PER_DEVICE)
                {
                    // too much Digital 8 Bit inputs, only maximum MAX_PROXY_PER_DEVICE possible
                    oInstanceIndex = MAX_PROXY_PER_DEVICE -1;
                }
                m_oDig8InProxyInfo[oInstanceIndex].nSlaveType = m_proxyReceiverList[i].proxyInfo.nSlaveType;
                m_oDig8InProxyInfo[oInstanceIndex].nProductCode = m_proxyReceiverList[i].proxyInfo.nProductCode;
                m_oDig8InProxyInfo[oInstanceIndex].nVendorID = m_proxyReceiverList[i].proxyInfo.nVendorID;
                m_oDig8InProxyInfo[oInstanceIndex].nInstance = m_proxyReceiverList[i].proxyInfo.nInstance;
                m_oDig8InProxyInfo[oInstanceIndex].nStartBit = m_proxyReceiverList[i].proxyInfo.nStartBit;
                m_oDig8InProxyInfo[oInstanceIndex].nTriggerLevel = m_proxyReceiverList[i].proxyInfo.nTriggerLevel;
                m_oDig8InProxyInfo[oInstanceIndex].nLength = m_proxyReceiverList[i].proxyInfo.nLength;
                if (m_oDig8InProxyInfo[oInstanceIndex].nProductCode == PRODUCTCODE_EL1018)
                {
                    m_oDig8InProxyInfo[oInstanceIndex].m_oProductIndex = eProductIndex_EL1018;
                }
                else
                {
                    // falscher ProductCode fuer Digital 8 Bit Input
                }
                m_oDig8InProxyInfo[oInstanceIndex].m_oInstance = static_cast<EcatInstance>(m_oDig8InProxyInfo[oInstanceIndex].nInstance);
                m_oDig8InProxyInfo[oInstanceIndex].m_oChannel = eChannel1;
                m_oDig8InProxyInfo[oInstanceIndex].m_oActive = true;

                wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                wmLog(eDebug, "Digital 8 Bit Input no: %d\n", oInstanceIndex);
                wmLog(eDebug, "m_oActive:   0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].m_oActive);
                wmLog(eDebug, "VendorID:    0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].nVendorID);
                wmLog(eDebug, "ProductCode: 0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].nProductCode);
                wmLog(eDebug, "Instance:    0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].nInstance);
                wmLog(eDebug, "SlaveType:   0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].nSlaveType);
                wmLog(eDebug, "StartBit:    0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].nStartBit);
                wmLog(eDebug, "Length:      0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].nLength);
                wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].m_oProductIndex);
                wmLog(eDebug, "m_oInstance: 0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].m_oInstance);
                wmLog(eDebug, "m_oChannel:  0x%x\n", m_oDig8InProxyInfo[oInstanceIndex].m_oChannel);
                wmLog(eDebug, "--------------------------------------------------\n");

                break;
            }
            case _GATEWAY_:{
                int oInstanceIndex = (int)m_proxyReceiverList[i].proxyInfo.nInstance - 1;
                if (oInstanceIndex >= MAX_PROXY_PER_DEVICE)
                {
                    // too much Digital 8 Bit inputs, only maximum MAX_PROXY_PER_DEVICE possible
                    oInstanceIndex = MAX_PROXY_PER_DEVICE -1;
                }
                m_oGatewayInProxyInfo[oInstanceIndex].nSlaveType = m_proxyReceiverList[i].proxyInfo.nSlaveType;
                m_oGatewayInProxyInfo[oInstanceIndex].nProductCode = m_proxyReceiverList[i].proxyInfo.nProductCode;
                m_oGatewayInProxyInfo[oInstanceIndex].nVendorID = m_proxyReceiverList[i].proxyInfo.nVendorID;
                m_oGatewayInProxyInfo[oInstanceIndex].nInstance = m_proxyReceiverList[i].proxyInfo.nInstance;
                m_oGatewayInProxyInfo[oInstanceIndex].nStartBit = m_proxyReceiverList[i].proxyInfo.nStartBit;
                m_oGatewayInProxyInfo[oInstanceIndex].nTriggerLevel = m_proxyReceiverList[i].proxyInfo.nTriggerLevel;
                m_oGatewayInProxyInfo[oInstanceIndex].nLength = m_proxyReceiverList[i].proxyInfo.nLength;
                if (m_oGatewayInProxyInfo[oInstanceIndex].nProductCode == PRODUCTCODE_ANYBUS_GW)
                {
                    m_oGatewayInProxyInfo[oInstanceIndex].m_oProductIndex = eProductIndex_Anybus_GW;
                }
                else
                {
                    // falscher ProductCode fuer Gateway Input
                }
                m_oGatewayInProxyInfo[oInstanceIndex].m_oInstance = static_cast<EcatInstance>(m_oGatewayInProxyInfo[oInstanceIndex].nInstance);
                m_oGatewayInProxyInfo[oInstanceIndex].m_oChannel = eChannel1;
                m_oGatewayInProxyInfo[oInstanceIndex].m_oActive = true;

                wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                wmLog(eDebug, "Gateway Input no: %d\n", oInstanceIndex);
                wmLog(eDebug, "m_oActive:   0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].m_oActive);
                wmLog(eDebug, "VendorID:    0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].nVendorID);
                wmLog(eDebug, "ProductCode: 0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].nProductCode);
                wmLog(eDebug, "Instance:    0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].nInstance);
                wmLog(eDebug, "SlaveType:   0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].nSlaveType);
                wmLog(eDebug, "StartBit:    0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].nStartBit);
                wmLog(eDebug, "Length:      0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].nLength);
                wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].m_oProductIndex);
                wmLog(eDebug, "m_oInstance: 0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].m_oInstance);
                wmLog(eDebug, "m_oChannel:  0x%x\n", m_oGatewayInProxyInfo[oInstanceIndex].m_oChannel);
                wmLog(eDebug, "--------------------------------------------------\n");

                break;
            }
            case _ANALOG_CHAN1_:
            case _ANALOG_CHAN2_:{
                if (m_proxyReceiverList[i].proxyInfo.nProductCode == PRODUCTCODE_EL3102)
                {
                    int oInstanceIndex = (int)m_proxyReceiverList[i].proxyInfo.nInstance - 1;
                    if (oInstanceIndex >= MAX_PROXY_PER_DEVICE)
                    {
                        // too much Analog inputs, only maximum MAX_PROXY_PER_DEVICE possible
                        oInstanceIndex = MAX_PROXY_PER_DEVICE -1;
                    }
                    m_oAnalogInProxyInfo[oInstanceIndex].nSlaveType = m_proxyReceiverList[i].proxyInfo.nSlaveType;
                    m_oAnalogInProxyInfo[oInstanceIndex].nProductCode = m_proxyReceiverList[i].proxyInfo.nProductCode;
                    m_oAnalogInProxyInfo[oInstanceIndex].nVendorID = m_proxyReceiverList[i].proxyInfo.nVendorID;
                    m_oAnalogInProxyInfo[oInstanceIndex].nInstance = m_proxyReceiverList[i].proxyInfo.nInstance;
                    m_oAnalogInProxyInfo[oInstanceIndex].nStartBit = m_proxyReceiverList[i].proxyInfo.nStartBit;
                    m_oAnalogInProxyInfo[oInstanceIndex].nTriggerLevel = m_proxyReceiverList[i].proxyInfo.nTriggerLevel;
                    m_oAnalogInProxyInfo[oInstanceIndex].nLength = m_proxyReceiverList[i].proxyInfo.nLength;
                    if (m_oAnalogInProxyInfo[oInstanceIndex].nProductCode == PRODUCTCODE_EL3102)
                    {
                        m_oAnalogInProxyInfo[oInstanceIndex].m_oProductIndex = eProductIndex_EL3102;
                    }
                    else
                    {
                        // falscher ProductCode fuer Analog Input
                    }
                    m_oAnalogInProxyInfo[oInstanceIndex].m_oInstance = static_cast<EcatInstance>(m_oAnalogInProxyInfo[oInstanceIndex].nInstance);
                    m_oAnalogInProxyInfo[oInstanceIndex].m_oChannel = eChannel1;
                    m_oAnalogInProxyInfo[oInstanceIndex].m_oActive = true;

                    wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                    wmLog(eDebug, "Analog Input no: %d\n", oInstanceIndex);
                    wmLog(eDebug, "m_oActive:   0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].m_oActive);
                    wmLog(eDebug, "VendorID:    0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].nVendorID);
                    wmLog(eDebug, "ProductCode: 0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].nProductCode);
                    wmLog(eDebug, "Instance:    0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].nInstance);
                    wmLog(eDebug, "SlaveType:   0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].nSlaveType);
                    wmLog(eDebug, "StartBit:    0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].nStartBit);
                    wmLog(eDebug, "Length:      0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].nLength);
                    wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].m_oProductIndex);
                    wmLog(eDebug, "m_oInstance: 0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].m_oInstance);
                    wmLog(eDebug, "m_oChannel:  0x%x\n", m_oAnalogInProxyInfo[oInstanceIndex].m_oChannel);
                    wmLog(eDebug, "--------------------------------------------------\n");
                }
                else if (m_proxyReceiverList[i].proxyInfo.nProductCode == PRODUCTCODE_EL3702)
                {
                    int oInstanceIndex = (int)m_proxyReceiverList[i].proxyInfo.nInstance - 1;
                    if (oInstanceIndex >= MAX_PROXY_PER_DEVICE)
                    {
                        // too much Analog inputs, only maximum MAX_PROXY_PER_DEVICE possible
                        oInstanceIndex = MAX_PROXY_PER_DEVICE -1;
                    }
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].nSlaveType = m_proxyReceiverList[i].proxyInfo.nSlaveType;
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].nProductCode = m_proxyReceiverList[i].proxyInfo.nProductCode;
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].nVendorID = m_proxyReceiverList[i].proxyInfo.nVendorID;
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].nInstance = m_proxyReceiverList[i].proxyInfo.nInstance;
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].nStartBit = m_proxyReceiverList[i].proxyInfo.nStartBit;
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].nTriggerLevel = m_proxyReceiverList[i].proxyInfo.nTriggerLevel;
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].nLength = m_proxyReceiverList[i].proxyInfo.nLength;
                    if (m_oAnalogOversampInProxyInfo[oInstanceIndex].nProductCode == PRODUCTCODE_EL3702)
                    {
                        m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oProductIndex = eProductIndex_EL3702;
                    }
                    else
                    {
                        // falscher ProductCode fuer Analog Input
                    }
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oInstance = static_cast<EcatInstance>(m_oAnalogOversampInProxyInfo[oInstanceIndex].nInstance);
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oChannel = eChannel1;
                    m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oActive = true;

                    wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                    wmLog(eDebug, "Analog Oversampling Input no: %d\n", oInstanceIndex);
                    wmLog(eDebug, "m_oActive:   0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oActive);
                    wmLog(eDebug, "VendorID:    0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].nVendorID);
                    wmLog(eDebug, "ProductCode: 0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].nProductCode);
                    wmLog(eDebug, "Instance:    0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].nInstance);
                    wmLog(eDebug, "SlaveType:   0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].nSlaveType);
                    wmLog(eDebug, "StartBit:    0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].nStartBit);
                    wmLog(eDebug, "Length:      0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].nLength);
                    wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oProductIndex);
                    wmLog(eDebug, "m_oInstance: 0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oInstance);
                    wmLog(eDebug, "m_oChannel:  0x%x\n", m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oChannel);
                    wmLog(eDebug, "--------------------------------------------------\n");
                }

                break;
            }
            default:{
                //TODO: Falsch konfiguriert?
                break;
            }
            }//end switch
        }//end for iterate Receiver

        //#############################################################################################################

        // generate SendOut-Ports

        for (unsigned int i = 0; i < m_oMyVIConfigParser.m_outCommandList.size(); ++i) {
            COMMAND_INFORMATION* commandOutInfo = &(m_oMyVIConfigParser.m_outCommandList[i]);

            wmLog(eDebug, "CommandType: %d\n", commandOutInfo->commandType);
            wmLog(eDebug, "ProductCode: 0x%x\n", commandOutInfo->proxyInfo.nProductCode);
            wmLog(eDebug, "VendorID:    0x%x\n", commandOutInfo->proxyInfo.nVendorID);
            wmLog(eDebug, "SlaveType:   %d\n", commandOutInfo->proxyInfo.nSlaveType);
            wmLog(eDebug, "Instance:    %d\n", commandOutInfo->proxyInfo.nInstance);
            wmLog(eDebug, "StartBit:    %d\n", commandOutInfo->proxyInfo.nStartBit);
            wmLog(eDebug, "Length:      %d\n", commandOutInfo->proxyInfo.nLength);
            wmLog(eDebug, "--------------------------------------------\n");

            bool foundProxy = false;
            for (unsigned int j = 0; j < m_senderList.size(); ++j) {

                if(m_senderList[j].proxyInfo.nProductCode == commandOutInfo->proxyInfo.nProductCode
                        && m_senderList[j].proxyInfo.nVendorID == commandOutInfo->proxyInfo.nVendorID
                        && m_senderList[j].proxyInfo.nInstance == commandOutInfo->proxyInfo.nInstance
                ){
                    foundProxy = true;
                    break;
                }
            }
            if(!foundProxy){
                //neuen Sender eintragen
                m_senderList.push_back(*commandOutInfo);
            }
        }

        for (unsigned int i = 0; i < m_senderList.size(); ++i) {

            //Sender initialisieren
            switch (m_senderList[i].proxyInfo.nSlaveType) {

            case _DIG_8BIT_:{
                int oInstanceIndex = (int)m_senderList[i].proxyInfo.nInstance - 1;
                if (oInstanceIndex >= MAX_PROXY_PER_DEVICE)
                {
                    // too much Digital 8 Bit inputs, only maximum MAX_PROXY_PER_DEVICE possible
                    oInstanceIndex = MAX_PROXY_PER_DEVICE -1;
                }
                m_oDig8OutProxyInfo[oInstanceIndex].nSlaveType = m_senderList[i].proxyInfo.nSlaveType;
                m_oDig8OutProxyInfo[oInstanceIndex].nProductCode = m_senderList[i].proxyInfo.nProductCode;
                m_oDig8OutProxyInfo[oInstanceIndex].nVendorID = m_senderList[i].proxyInfo.nVendorID;
                m_oDig8OutProxyInfo[oInstanceIndex].nInstance = m_senderList[i].proxyInfo.nInstance;
                m_oDig8OutProxyInfo[oInstanceIndex].nStartBit = m_senderList[i].proxyInfo.nStartBit;
                m_oDig8OutProxyInfo[oInstanceIndex].nTriggerLevel = m_senderList[i].proxyInfo.nTriggerLevel;
                m_oDig8OutProxyInfo[oInstanceIndex].nLength = m_senderList[i].proxyInfo.nLength;
                if (m_oDig8OutProxyInfo[oInstanceIndex].nProductCode == PRODUCTCODE_EL2008)
                {
                    m_oDig8OutProxyInfo[oInstanceIndex].m_oProductIndex = eProductIndex_EL2008;
                }
                else
                {
                    // falscher ProductCode fuer Digital 8 Bit output
                }
                m_oDig8OutProxyInfo[oInstanceIndex].m_oInstance = static_cast<EcatInstance>(m_oDig8OutProxyInfo[oInstanceIndex].nInstance);
                m_oDig8OutProxyInfo[oInstanceIndex].m_oChannel = eChannel1;
                m_oDig8OutProxyInfo[oInstanceIndex].m_oActive = true;

                wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                wmLog(eDebug, "Digital 8 Bit Output no: %d\n", oInstanceIndex);
                wmLog(eDebug, "m_oActive:   0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].m_oActive);
                wmLog(eDebug, "VendorID:    0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].nVendorID);
                wmLog(eDebug, "ProductCode: 0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].nProductCode);
                wmLog(eDebug, "Instance:    0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].nInstance);
                wmLog(eDebug, "SlaveType:   0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].nSlaveType);
                wmLog(eDebug, "StartBit:    0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].nStartBit);
                wmLog(eDebug, "Length:      0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].nLength);
                wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].m_oProductIndex);
                wmLog(eDebug, "m_oInstance: 0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].m_oInstance);
                wmLog(eDebug, "m_oChannel:  0x%x\n", m_oDig8OutProxyInfo[oInstanceIndex].m_oChannel);
                wmLog(eDebug, "--------------------------------------------------\n");

                break;
            }
            case _GATEWAY_:{
                int oInstanceIndex = (int)m_senderList[i].proxyInfo.nInstance - 1;
                if (oInstanceIndex >= MAX_PROXY_PER_DEVICE)
                {
                    // too much Digital 8 Bit inputs, only maximum MAX_PROXY_PER_DEVICE possible
                    oInstanceIndex = MAX_PROXY_PER_DEVICE -1;
                }
                m_oGatewayOutProxyInfo[oInstanceIndex].nSlaveType = m_senderList[i].proxyInfo.nSlaveType;
                m_oGatewayOutProxyInfo[oInstanceIndex].nProductCode = m_senderList[i].proxyInfo.nProductCode;
                m_oGatewayOutProxyInfo[oInstanceIndex].nVendorID = m_senderList[i].proxyInfo.nVendorID;
                m_oGatewayOutProxyInfo[oInstanceIndex].nInstance = m_senderList[i].proxyInfo.nInstance;
                m_oGatewayOutProxyInfo[oInstanceIndex].nStartBit = m_senderList[i].proxyInfo.nStartBit;
                m_oGatewayOutProxyInfo[oInstanceIndex].nTriggerLevel = m_senderList[i].proxyInfo.nTriggerLevel;
                m_oGatewayOutProxyInfo[oInstanceIndex].nLength = m_senderList[i].proxyInfo.nLength;
                if (m_oGatewayOutProxyInfo[oInstanceIndex].nProductCode == PRODUCTCODE_ANYBUS_GW)
                {
                    m_oGatewayOutProxyInfo[oInstanceIndex].m_oProductIndex = eProductIndex_Anybus_GW;
                }
                else
                {
                    // falscher ProductCode fuer Gateway Input
                }
                m_oGatewayOutProxyInfo[oInstanceIndex].m_oInstance = static_cast<EcatInstance>(m_oGatewayOutProxyInfo[oInstanceIndex].nInstance);
                m_oGatewayOutProxyInfo[oInstanceIndex].m_oChannel = eChannel1;
                m_oGatewayOutProxyInfo[oInstanceIndex].m_oActive = true;

                wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                wmLog(eDebug, "Gateway Output no: %d\n", oInstanceIndex);
                wmLog(eDebug, "m_oActive:   0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].m_oActive);
                wmLog(eDebug, "VendorID:    0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].nVendorID);
                wmLog(eDebug, "ProductCode: 0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].nProductCode);
                wmLog(eDebug, "Instance:    0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].nInstance);
                wmLog(eDebug, "SlaveType:   0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].nSlaveType);
                wmLog(eDebug, "StartBit:    0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].nStartBit);
                wmLog(eDebug, "Length:      0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].nLength);
                wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].m_oProductIndex);
                wmLog(eDebug, "m_oInstance: 0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].m_oInstance);
                wmLog(eDebug, "m_oChannel:  0x%x\n", m_oGatewayOutProxyInfo[oInstanceIndex].m_oChannel);
                wmLog(eDebug, "--------------------------------------------------\n");

                break;
            }
            default:{
                //TODO: Falsch konfiguriert?
                break;
            }
            }//end switch

            FindCommandSender(m_senderList[i]);
        }//end for iterate Receiver

        //HeadmonitorGateway
        if (isHeadMonitorGatewayEnabled())
        {
            if(m_oMyVIConfigParser.m_HMinGatewayInfo.isValid)
            {
                if (m_oMyVIConfigParser.m_HMinGatewayInfo.nSlaveType == _DIG_8BIT_)
                {
                    m_oHMSignalsProxyInfo.nSlaveType = m_oMyVIConfigParser.m_HMinGatewayInfo.nSlaveType;
                    m_oHMSignalsProxyInfo.nProductCode = m_oMyVIConfigParser.m_HMinGatewayInfo.nProductCode;
                    m_oHMSignalsProxyInfo.nVendorID = m_oMyVIConfigParser.m_HMinGatewayInfo.nVendorID;
                    m_oHMSignalsProxyInfo.nInstance = m_oMyVIConfigParser.m_HMinGatewayInfo.nInstance;
                    m_oHMSignalsProxyInfo.nStartBit = 0;
                    m_oHMSignalsProxyInfo.nTriggerLevel = 0;
                    m_oHMSignalsProxyInfo.nLength = 0;
                    if (m_oHMSignalsProxyInfo.nProductCode == PRODUCTCODE_EL1018)
                    {
                        m_oHMSignalsProxyInfo.m_oProductIndex = eProductIndex_EL1018;
                    }
                    else
                    {
                        // falscher ProductCode fuer Head Monitor Input
                    }
                    m_oHMSignalsProxyInfo.m_oInstance = static_cast<EcatInstance>(m_oHMSignalsProxyInfo.nInstance);
                    m_oHMSignalsProxyInfo.m_oChannel = eChannel1;
                    m_oHMSignalsProxyInfo.m_oActive = true;

                    wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                    wmLog(eDebug, "Head Monitor Input:\n");
                    wmLog(eDebug, "m_oActive:   0x%x\n", m_oHMSignalsProxyInfo.m_oActive);
                    wmLog(eDebug, "VendorID:    0x%x\n", m_oHMSignalsProxyInfo.nVendorID);
                    wmLog(eDebug, "ProductCode: 0x%x\n", m_oHMSignalsProxyInfo.nProductCode);
                    wmLog(eDebug, "Instance:    0x%x\n", m_oHMSignalsProxyInfo.nInstance);
                    wmLog(eDebug, "SlaveType:   0x%x\n", m_oHMSignalsProxyInfo.nSlaveType);
                    wmLog(eDebug, "StartBit:    0x%x\n", m_oHMSignalsProxyInfo.nStartBit);
                    wmLog(eDebug, "Length:      0x%x\n", m_oHMSignalsProxyInfo.nLength);
                    wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oHMSignalsProxyInfo.m_oProductIndex);
                    wmLog(eDebug, "m_oInstance: 0x%x\n", m_oHMSignalsProxyInfo.m_oInstance);
                    wmLog(eDebug, "m_oChannel:  0x%x\n", m_oHMSignalsProxyInfo.m_oChannel);
                    wmLog(eDebug, "--------------------------------------------------\n");
                }
                if(m_oMyVIConfigParser.m_HMoutGatewayInfo.isValid)
                {
                    if(m_oMyVIConfigParser.m_HMoutGatewayInfo.nSlaveType == _DIG_8BIT_)
                    {
                        m_oHMOutputProxyInfo.nSlaveType = m_oMyVIConfigParser.m_HMoutGatewayInfo.nSlaveType;
                        m_oHMOutputProxyInfo.nProductCode = m_oMyVIConfigParser.m_HMoutGatewayInfo.nProductCode;
                        m_oHMOutputProxyInfo.nVendorID = m_oMyVIConfigParser.m_HMoutGatewayInfo.nVendorID;
                        m_oHMOutputProxyInfo.nInstance = m_oMyVIConfigParser.m_HMoutGatewayInfo.nInstance;
                        unsigned int oLowestBitPos = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitGlasNotPresent;
                        if (m_oMyVIConfigParser.m_HMoutGatewayInfo.bitGlasDirty < oLowestBitPos) oLowestBitPos = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitGlasDirty;
                        if (m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempGlasFail < oLowestBitPos) oLowestBitPos = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempGlasFail;
                        if (m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempHeadFail < oLowestBitPos) oLowestBitPos = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempHeadFail;
                        m_oHMOutputProxyInfo.nStartBit = oLowestBitPos;
                        m_oHMOutputProxyInfo.nTriggerLevel = 0;
                        m_oHMOutputProxyInfo.nLength = 4;
                        if (m_oHMOutputProxyInfo.nProductCode == PRODUCTCODE_EL2008)
                        {
                            m_oHMOutputProxyInfo.m_oProductIndex = eProductIndex_EL2008;
                        }
                        else
                        {
                            // falscher ProductCode fuer Head Monitor Input
                        }
                        m_oHMOutputProxyInfo.m_oInstance = static_cast<EcatInstance>(m_oHMOutputProxyInfo.nInstance);
                        m_oHMOutputProxyInfo.m_oChannel = eChannel1;
                        m_oHMOutputProxyInfo.m_oActive = true;

                        wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                        wmLog(eDebug, "Head Monitor Input:\n");
                        wmLog(eDebug, "m_oActive:   0x%x\n", m_oHMOutputProxyInfo.m_oActive);
                        wmLog(eDebug, "VendorID:    0x%x\n", m_oHMOutputProxyInfo.nVendorID);
                        wmLog(eDebug, "ProductCode: 0x%x\n", m_oHMOutputProxyInfo.nProductCode);
                        wmLog(eDebug, "Instance:    0x%x\n", m_oHMOutputProxyInfo.nInstance);
                        wmLog(eDebug, "SlaveType:   0x%x\n", m_oHMOutputProxyInfo.nSlaveType);
                        wmLog(eDebug, "StartBit:    0x%x\n", m_oHMOutputProxyInfo.nStartBit);
                        wmLog(eDebug, "Length:      0x%x\n", m_oHMOutputProxyInfo.nLength);
                        wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oHMOutputProxyInfo.m_oProductIndex);
                        wmLog(eDebug, "m_oInstance: 0x%x\n", m_oHMOutputProxyInfo.m_oInstance);
                        wmLog(eDebug, "m_oChannel:  0x%x\n", m_oHMOutputProxyInfo.m_oChannel);
                        wmLog(eDebug, "--------------------------------------------------\n");

                        m_senderHMOutput.info.proxyInfo = m_oHMOutputProxyInfo;
                        m_senderHMOutput.info.commandType = static_cast<CommandType>(0);
                    }
                    if(m_oMyVIConfigParser.m_HMoutGatewayInfo.nSlaveType == _GATEWAY_)
                    {
                        m_oHMOutputProxyInfo.nSlaveType = m_oMyVIConfigParser.m_HMoutGatewayInfo.nSlaveType;
                        m_oHMOutputProxyInfo.nProductCode = m_oMyVIConfigParser.m_HMoutGatewayInfo.nProductCode;
                        m_oHMOutputProxyInfo.nVendorID = m_oMyVIConfigParser.m_HMoutGatewayInfo.nVendorID;
                        m_oHMOutputProxyInfo.nInstance = m_oMyVIConfigParser.m_HMoutGatewayInfo.nInstance;
                        unsigned int oLowestBitPos = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitGlasNotPresent;
                        if (m_oMyVIConfigParser.m_HMoutGatewayInfo.bitGlasDirty < oLowestBitPos) oLowestBitPos = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitGlasDirty;
                        if (m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempGlasFail < oLowestBitPos) oLowestBitPos = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempGlasFail;
                        if (m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempHeadFail < oLowestBitPos) oLowestBitPos = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempHeadFail;
                        m_oHMOutputProxyInfo.nStartBit = oLowestBitPos;
                        m_oHMOutputProxyInfo.nTriggerLevel = 0;
                        m_oHMOutputProxyInfo.nLength = 4;
                        if (m_oHMOutputProxyInfo.nProductCode == PRODUCTCODE_ANYBUS_GW)
                        {
                            m_oHMOutputProxyInfo.m_oProductIndex = eProductIndex_Anybus_GW;
                        }
                        else
                        {
                            // falscher ProductCode fuer Head Monitor Input
                        }
                        m_oHMOutputProxyInfo.m_oInstance = static_cast<EcatInstance>(m_oHMOutputProxyInfo.nInstance);
                        m_oHMOutputProxyInfo.m_oChannel = eChannel1;
                        m_oHMOutputProxyInfo.m_oActive = true;

                        wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
                        wmLog(eDebug, "Head Monitor Input:\n");
                        wmLog(eDebug, "m_oActive:   0x%x\n", m_oHMOutputProxyInfo.m_oActive);
                        wmLog(eDebug, "VendorID:    0x%x\n", m_oHMOutputProxyInfo.nVendorID);
                        wmLog(eDebug, "ProductCode: 0x%x\n", m_oHMOutputProxyInfo.nProductCode);
                        wmLog(eDebug, "Instance:    0x%x\n", m_oHMOutputProxyInfo.nInstance);
                        wmLog(eDebug, "SlaveType:   0x%x\n", m_oHMOutputProxyInfo.nSlaveType);
                        wmLog(eDebug, "StartBit:    0x%x\n", m_oHMOutputProxyInfo.nStartBit);
                        wmLog(eDebug, "Length:      0x%x\n", m_oHMOutputProxyInfo.nLength);
                        wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oHMOutputProxyInfo.m_oProductIndex);
                        wmLog(eDebug, "m_oInstance: 0x%x\n", m_oHMOutputProxyInfo.m_oInstance);
                        wmLog(eDebug, "m_oChannel:  0x%x\n", m_oHMOutputProxyInfo.m_oChannel);
                        wmLog(eDebug, "--------------------------------------------------\n");

                        m_senderHMOutput.info.proxyInfo = m_oHMOutputProxyInfo;
                        m_senderHMOutput.info.commandType = static_cast<CommandType>(0);
                    }                    
                }
            }
        }
        //END HeadmonitorGateway
    }

    if (isContinuouslyModeActive()) // do in continuously mode
    {
        TriggerAutomatic(false, 0, 0, "no info"); // dummy call for initialize the static var inside the function with false !
        if (m_simulateHW)
        {
            TriggerContinuously(false, 0, 0); // dummy call for initialize the static var inside the function with false !
        }
    }

    m_oTriggerCalibrationBlocked = false;
    // Wenn alle notwendigen Bedingungen erfuellt sind, dann starte thread fuer Auto-Homing Z-Collimator
    if (SystemConfiguration::instance().getBool("ForceHomingOfAxis", true) == true)
    {
        if (SystemConfiguration::instance().getBool("ZCollimatorEnable", false) == true)
        {
            if (SystemConfiguration::instance().getBool("ZCollAutoHomingEnable", false) == true)
            {
                m_oTriggerCalibrationBlocked = true;

                // Zeiger auf VI_InspectionControl an Thread uebergeben
                m_oDataToZCollAutoHomingThread.m_pVI_InspectionControl = this;
                // start thread for auto homing ZCollimator
                pthread_t ZCollAutoHomingThreadID;
                pthread_create(&ZCollAutoHomingThreadID, NULL, &ZCollAutoHomingThread, &m_oDataToZCollAutoHomingThread);
            }
        }
    }
    
    //fetching
   // m_dbNotificationServer.fetchAllProductsData();
    
} //end Init()

bool VI_InspectionControl::FindCommandSender(COMMAND_INFORMATION& info ){
	bool ret = false;

	for (unsigned int i = 0; i < m_oMyVIConfigParser.m_outCommandList.size(); ++i) {
		COMMAND_INFORMATION* commandOutInfo = &(m_oMyVIConfigParser.m_outCommandList[i]);

		if(commandOutInfo->proxyInfo.nProductCode == info.proxyInfo.nProductCode
				&& commandOutInfo->proxyInfo.nVendorID == info.proxyInfo.nVendorID
				&& commandOutInfo->proxyInfo.nInstance == info.proxyInfo.nInstance
		){
			switch (commandOutInfo->commandType) {
			case ASystemReady:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderSystemReady = senderWrapper;
				ret = true;
				break;
			}
			case AInspectCycleAckn:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderInspectCycleAckn = senderWrapper;
				ret = true;
				break;
			}
            case AInspectionPreStartAckn:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderInspectionPreStartAckn = senderWrapper;
                ret = true;
                break;
            }
            case AInspectionStartEndAckn:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderInspectionStartEndAckn = senderWrapper;
                ret = true;
                break;
            }
			case AInspectionOK:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderInspectionOK = senderWrapper;
				ret = true;
				break;
			}
			case ASumErrorLatched:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderSumErrorLatched = senderWrapper;
				ret = true;
				break;
			}
			case AInspectionIncomplete:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderInspectionIncomplete = senderWrapper;
				ret = true;
				break;
			}
			case ASumErrorSeam:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderSumErrorSeam = senderWrapper;
				ret = true;
				break;
			}
			case ASumErrorSeamSeries:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderSumErrorSeamSeries = senderWrapper;
				ret = true;
				break;
			}
			case ASystemErrorField:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderSystemErrorField = senderWrapper;
				ret = true;
				break;
			}
			case AQualityErrorField:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderQualityErrorField = senderWrapper;
				ret = true;
				break;
			}
			case ACalibResultsField:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderCalibResultsField = senderWrapper;
				ret = true;
				break;
			}
			case APositionResultsField:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderGenPurposeDigOut1 = senderWrapper;
				ret = true;
				break;
			}
			case AProductType:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderProductTypeMirror = senderWrapper;
				ret = true;
				break;
			}
			case AProductNumber:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderProductNumberMirror = senderWrapper;
				ret = true;
				break;
			}
			case AExtendedProductInfo:{
				if (isFieldbusExtendedProductInfoEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderExtendedProductInfoMirror = senderWrapper;
					ret = true;
				}
				break;
			}
			case ACabinetTemperatureOk:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderCabinetTemperatureOk = senderWrapper;
				ret = true;
				break;
			}
			case AGenPurposeDigInAckn:{
				SenderWrapper senderWrapper;
				senderWrapper.info = *commandOutInfo;
				m_senderGenPurposeDigInAckn = senderWrapper;
				ret = true;
				break;
			}
			case AAcknProductNumberFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderAcknProductNumberFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ATriggerResultsReadyFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderTriggerResultsReadyFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ASetInspectionOKFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderSetInspectionOKFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ASetSumErrorLatchedFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderSetSumErrorLatchedFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ASetQualityErrorFieldFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderSetQualityErrorFieldFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ASetSystemReadyStatusFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderSetSystemReadyStatusFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ASetCabinetTemperatureOkFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderSetCabinetTemperatureOkFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ASetHMSignalsFieldFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderSetHMSignalsFieldFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ASetSystemErrorFieldFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderSetSystemErrorFieldFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ASetProductTypeFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderSetProductTypeFull = senderWrapper;
					ret = true;
				}
				break;
			}
			case ASetProductNumberFull:{
				if (isFieldbusInterfaceFullEnabled())
				{
					SenderWrapper senderWrapper;
					senderWrapper.info = *commandOutInfo;
					m_senderSetProductNumberFull = senderWrapper;
					ret = true;
				}
				break;
			}

            case A_S6K_SystemFault:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_SystemFault = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_SystemReady:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_SystemReady = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_FastStopDoubleBlank:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_FastStopDoubleBlank = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_SeamErrorCat1:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_SeamErrorCat1 = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_SeamErrorCat2:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_SeamErrorCat2 = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_QualityDataValid:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_QualityDataValid = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_AcknCycleData:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_AcknCycleData = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_CycleDataMirror:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_CycleDataMirror = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_SeamNoMirror:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_SeamNoMirror = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_AcknSeamNo:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_AcknSeamNo = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultDataValid:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultDataValid = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultDataCount:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultDataCount = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage1:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage1] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage2:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage2] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage3:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage3] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage4:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage4] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage5:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage5] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage6:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage6] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage7:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage7] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage8:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage8] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage9:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage9] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage10:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage10] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage11:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage11] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage12:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage12] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage13:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage13] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage14:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage14] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage15:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage15] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage16:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage16] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage17:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage17] = senderWrapper;
                ret = true;
                break;
            }
            case A_S6K_ResultsImage18:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderS6K_ResultsImage[eS6K_ResultsImage18] = senderWrapper;
                ret = true;
                break;
            }
            case A_SM_ProcessingActive:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderSM_ProcessingActive = senderWrapper;
                ret = true;
                break;
            }
            case A_SM_TakeoverStep:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderSM_TakeoverStep = senderWrapper;
                ret = true;
                break;
            }
            case A_SM_StepField:
            {
                SenderWrapper senderWrapper;
                senderWrapper.info = *commandOutInfo;
                m_senderSM_StepField = senderWrapper;
                ret = true;
                break;
            }
			default:{
				break;
			}
			}
		}
	}
	return ret;
}

void VI_InspectionControl::ecatDigitalIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t value) // Interface EthercatInputs
{
    int oInstanceIndex = (int)instance - 1;

    if (m_oDig8InProxyInfo[oInstanceIndex].m_oActive)
    {
        if (m_oDig8InProxyInfo[oInstanceIndex].m_oProductIndex == productIndex)
        {
            if (m_oDig8InProxyInfo[oInstanceIndex].m_oInstance == instance)
            {
                ProxyReceive8Bit(productIndex, instance, value);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

    if (m_oHMSignalsProxyInfo.m_oActive)
    {
        if (m_oHMSignalsProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oHMSignalsProxyInfo.m_oInstance == instance)
            {
                IncomingHMSignals(value);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }

#if 0
	static bool oToggle = true;
	uint8_t oSendValue = 0x00;
	if (oToggle)
	{
		oToggle = false;
		oSendValue = 0x01;
	}
	else
	{
		oToggle = true;
		oSendValue = 0x00;
	}
	m_rEthercatOutputsProxy.ecatDigitalOut(eProductIndex_EL2008, eInstance1, (uint8_t) oSendValue, (uint8_t)0x01);
#endif
}

void VI_InspectionControl::ecatGatewayIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT8 &data) // Interface EthercatInputs
{
    static uint8_t oBuffer[MAX_PROXY_PER_DEVICE][160] = {{0}};

    m_oGatewayDataLength = size;
    int oInstanceIndex = (int)instance - 1;

#if 0
	bool oChangeFlag = false;
	for(unsigned int i = 0;i < size;i++)
	{
		if (oBuffer[oInstanceIndex][i] != data[i]) oChangeFlag = true;
	}
	if (oChangeFlag)
	{
		printf("ecatGatewayIn: oInstanceIndex: %d size: %u , ", oInstanceIndex, size);
		for (unsigned int i = 0;i < size;i++)
		{
			printf("%02X ", data[i]);
		}
		printf("\n");
        printf("productIndex: %d, instance: %d\n", static_cast<int>(productIndex), static_cast<int>(instance));
	}
#endif

	for(unsigned int i = 0;i < size;i++)
	{
		oBuffer[oInstanceIndex][i] = data[i];
	}

    if (m_oGatewayInProxyInfo[oInstanceIndex].m_oActive)
    {
        //if (m_oGatewayInProxyInfo[oInstanceIndex].m_oProductIndex == productIndex)
        if ((productIndex == eProductIndex_Anybus_GW) ||
            (productIndex == eProductIndex_Kunbus_GW))
        {
            if (m_oGatewayInProxyInfo[oInstanceIndex].m_oInstance == instance)
            {
                ProxyReceiveGateway(productIndex, instance, size, (char *)&oBuffer[oInstanceIndex][0]);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void VI_InspectionControl::ecatAnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t statusCH1, uint16_t valueCH1, uint8_t statusCH2, uint16_t valueCH2) // Interface EthercatInputs
{
    int oInstanceIndex = (int)instance - 1;

    if (m_oAnalogInProxyInfo[oInstanceIndex].m_oActive)
    {
        if (m_oAnalogInProxyInfo[oInstanceIndex].m_oProductIndex == productIndex)
        {
            if (m_oAnalogInProxyInfo[oInstanceIndex].m_oInstance == instance)
            {
                ProxyReceiveAnalog(productIndex, instance, eChannel1, (int16_t)valueCH1);
                ProxyReceiveAnalog(productIndex, instance, eChannel2, (int16_t)valueCH2);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void VI_InspectionControl::ecatAnalogOversamplingInCH1(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) // Interface EthercatInputs
{
    int oInstanceIndex = (int)instance - 1;

    if (m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oActive)
    {
        if (m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oProductIndex == productIndex)
        {
            if (m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oInstance == instance)
            {
                ProxyReceiveAnalogOversamp(productIndex, instance, eChannel1, size, data);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void VI_InspectionControl::ecatAnalogOversamplingInCH2(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data) // Interface EthercatInputs
{
    int oInstanceIndex = (int)instance - 1;

    if (m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oActive)
    {
        if (m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oProductIndex == productIndex)
        {
            if (m_oAnalogOversampInProxyInfo[oInstanceIndex].m_oInstance == instance)
            {
                ProxyReceiveAnalogOversamp(productIndex, instance, eChannel2, size, data);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void VI_InspectionControl::ProxyReceive8Bit(EcatProductIndex productIndex, EcatInstance p_oInstance, unsigned char p_oData){

    uint32_t oProductType = 0x00;
    if (productIndex == eProductIndex_EL1018)
    {
        oProductType = PRODUCTCODE_EL1018;
    }

	for (unsigned int i = 0; i < m_oMyVIConfigParser.m_inCommandList.size(); ++i) {
		COMMAND_INFORMATION* info = &(m_oMyVIConfigParser.m_inCommandList[i]);
//		if(proxyID.nCountInstance == info->proxyInfo.nInstance
//			&& proxyID.nProductCode == info->proxyInfo.nProductCode
//			&& proxyID.nVendorID == info->proxyInfo.nVendorID
//		){
		if((static_cast<unsigned int>(p_oInstance) == info->proxyInfo.nInstance) &&
           (oProductType == info->proxyInfo.nProductCode))
		{
			//Uebereinstimmung Empfangsdaten und Mapping
			bool checkBit = CheckBit(p_oData, *info);

			switch (info->commandType) {
                case ETriggerStartStopContinuously:{
                    if (isContinuouslyModeActive()) // do in continuously mode
                    {
                        // 23.11.2011 EA:
                        // Wenn beim Aufstarten schon ein productType anliegt wird er ansonsten erst
                        // beim zweiten Aufruf von TriggerAutomatic verwendet
                        // Deshalb hier ein zusaetzliches Auslesen des productType
                        // ACHTUNG: Funktioniert nur wenn productType auf gleicher nInstance
                        // TODO bessere Loesung fuer mehrere Instanzen
                        pthread_mutex_lock(&m_oProductTypeMutex);
                        COMMAND_INFORMATION* localInfo = NULL;
                        for (int j = 0; j < (int)m_oMyVIConfigParser.m_inCommandList.size(); ++j)
                        {
                            if (m_oMyVIConfigParser.m_inCommandList[j].commandType == EProductType)
                            {
                                localInfo = &(m_oMyVIConfigParser.m_inCommandList[j]);
                            }
                        }
                        if (localInfo != NULL)
                        {
                            m_oProductType = GetBits(p_oData,*localInfo);
                            if (m_oProductType == 0) m_oProductType = 1; // ProductType 0 ist Live-Produkt !
                        }
                        pthread_mutex_unlock(&m_oProductTypeMutex);
                        bool oTriggerBit = CheckBit(p_oData, *info);
                        TriggerContinuously(oTriggerBit, m_oProductType, m_oProductNumber);
                    }
                    break;
                }
                case ETriggerStartStopAutomatic:{
                    if (!isContinuouslyModeActive()) // do in normal cyclic mode
                    {
                        // 23.11.2011 EA:
                        // Wenn beim Aufstarten schon ein productType anliegt wird er ansonsten erst
                        // beim zweiten Aufruf von TriggerAutomatic verwendet
                        // Deshalb hier ein zusaetzliches Auslesen des productType
                        // ACHTUNG: Funktioniert nur wenn productType auf gleicher nInstance
                        // TODO bessere Loesung fuer mehrere Instanzen
                        pthread_mutex_lock(&m_oProductTypeMutex);
                        COMMAND_INFORMATION* localInfo = NULL;
                        for (int j = 0; j < (int)m_oMyVIConfigParser.m_inCommandList.size(); ++j)
                        {
                            if (m_oMyVIConfigParser.m_inCommandList[j].commandType == EProductType)
                            {
                                localInfo = &(m_oMyVIConfigParser.m_inCommandList[j]);
                            }
                        }
                        if (localInfo != NULL)
                        {
                            m_oProductType = GetBits(p_oData,*localInfo);
                            if (m_oProductType == 0) m_oProductType = 1; // ProductType 0 ist Live-Produkt !
                        }
                        pthread_mutex_unlock(&m_oProductTypeMutex);
                        bool oTriggerBit = CheckBit(p_oData, *info);
                        TriggerAutomatic(oTriggerBit, m_oProductType, m_oProductNumber, "no info");
                    }
                    break;
                }
				case ETriggerStartStopCalibration:{
					if (!m_oTriggerCalibrationBlocked) TriggerCalibration(checkBit, m_oCalibrationType);
					break;
				}
				case ETriggerHomingYAxis:{
					TriggerHomingYAxis(checkBit);
					break;
				}
                case ETriggerInspectionInfo:{
                    if (!isContinuouslyModeActive()) // do in normal cyclic mode
                    {
                        TriggerInspectInfo(checkBit, m_oSeamseriesShadow);
                    }
                    break;
                }
                case ETriggerInspectionStartEnd:{
                    // do in normal cyclic mode
                    if ((!isContinuouslyModeActive()) &&
                        (!isSCANMASTER_Application()))
                    {
                        TriggerInspectStartStop(checkBit, m_oSeamNrShadow);
                    }
                    break;
                }
				case ETriggerUnblockLineLaser:{
					TriggerUnblockLineLaser(checkBit);
					break;
				}
				case ETriggerQuitSystemFault:{
					TriggerQuitSystemFault(checkBit);
					break;
				}
				case EProductType:{

					pthread_mutex_lock(&m_oProductTypeMutex);
					m_oProductType = GetBits(p_oData,*info);
					if (m_oProductType == 0) m_oProductType = 1; // ProductType 0 ist Live-Produkt !
					pthread_mutex_unlock(&m_oProductTypeMutex);

					break;
				}
				case EProductNumber:{
					// Produktnummer wird als Binaerwert ueber die definierte Laenge eingelesen.
					// TODO: evtl. anderes Nummernformat z.B. BCD-Ziffern, Buchstaben ?
					pthread_mutex_lock(&m_oProductNumberMutex);
					if (!isProductNumberFromWMEnabled())
					{
						m_oProductNumber = GetBits(p_oData,*info);
					}
					pthread_mutex_unlock(&m_oProductNumberMutex);
					break;
				}
				case EExtendedProductInfo:{
					// not possible with digital inputs !
					break;
				}
				case ESeamseriesNr:{

					pthread_mutex_lock(&m_oSeamSeriesMutex);
					m_oSeamseriesShadow = GetBits(p_oData,*info);
					if (m_oSeamseriesShadow != 0) m_oSeamseriesShadow--;
					pthread_mutex_unlock(&m_oSeamSeriesMutex);

					break;
				}
				case ESeamNr:{

					pthread_mutex_lock(&m_oSeamNrMutex);
					m_oSeamNrShadow = GetBits(p_oData,*info);
					if (m_oSeamNrShadow != 0) m_oSeamNrShadow--;
					pthread_mutex_unlock(&m_oSeamNrMutex);

					break;
				}
				case ECalibrationType:{
					pthread_mutex_lock(&m_oCalibrationTypeMutex);
					m_oCalibrationType = GetBits(p_oData,*info);
					pthread_mutex_unlock(&m_oCalibrationTypeMutex);
					break;
				}
				case EInvertedTriggerEmergencyStop:{
					TriggerEmergencyStop(!checkBit);
					break;
				}
				case ECabinetTemperatureOk:{
					if (isCabinetTemperatureOkEnabled())
					{
						TriggerCabinetTemperatureOk(checkBit);
					}
					break;
				}
				case EGenPurposeDigIn1:{
					pthread_mutex_lock(&m_oGenPurposeDigIn1Mutex);
					// Rueckgabe von GetBits in 16 Bit mit Vorzeichen wandeln
					short oTempShort = (short) GetBits(p_oData,*info);
					if(isGenPurposeDigIn1Enabled())
					{
						m_oGenPurposeDigIn1_Single.store( (int) oTempShort);
					}
					else if (isGenPurposeDigInMultipleEnabled())
					{
						// Ablegen des neuen Eingabewertes in einer Zwischenvariablen
						m_oGenPurposeDigInValue = (int) oTempShort;
					}
					else
					{
						m_oGenPurposeDigIn1 = (int) oTempShort;
					}
					pthread_mutex_unlock(&m_oGenPurposeDigIn1Mutex);
					break;
				}
				case EGenPurposeDigInAddress:{
					if (isGenPurposeDigInMultipleEnabled())
					{
						// Ablegen der neuen Adresse fuer den Eingabewert
						pthread_mutex_lock(&m_oGenPurposeDigInAddressMutex);
						m_oGenPurposeDigInAddress = GetBits(p_oData, *info);
						pthread_mutex_unlock(&m_oGenPurposeDigInAddressMutex);
					}
					break;
				}
                case EGenPurposeDigInTakeOver:{
                    if (isGenPurposeDigInMultipleEnabled())
                    {
                        if (!isContinuouslyModeActive()) // do in normal cyclic mode
                        {
                            // pruefen auf Uebernahmesignal und bei steigender Flanke kopieren der Zwischenvariablen in die Zielvariable
                            // Quittierungssignal fuer Feldbus setzen bzw. ruecksetzen
                            TriggerGenPurposeDigInTakeOver(checkBit, m_oGenPurposeDigInAddress, m_oGenPurposeDigInValue);
                        }
                    }
                    break;
                }
                case EGenPurposeDigOutTakeOver:{
                    if (isGenPurposeDigOutMultipleEnabled())
                    {
                        if (!isContinuouslyModeActive()) // do in normal cyclic mode
                        {
                            // pruefen auf Uebernahmesignal und bei steigender Flanke kopieren der Ergebnisvariablen auf den Feldbus
                            // Quittierungssignal fuer Feldbus setzen bzw. ruecksetzen
                            TriggerGenPurposeDigOutTakeOver(checkBit, m_oGenPurposeDigInAddress);
                        }
                    }
                    break;
                }
                case E_SM_AcknowledgeStep:
                {
                    m_oSM_AcknowledgeStepStatus.store(checkBit);
                    break;
                }

                case E_S6K_MakePictures:
                {
                    m_oS6K_MakePicturesStatus = CheckBit(p_oData, *info);
                    break;
                }

				default:{
					break;
				}
			}
		}
	}
}

void VI_InspectionControl::ProxyReceiveGateway(EcatProductIndex productIndex, EcatInstance p_oInstance, short p_oBytes, char* p_pData){

    if (m_oControlSimulationIsOn)
    {
        return;
    }

    uint32_t oProductType = 0x00;
    if (productIndex == eProductIndex_Anybus_GW)
    {
        oProductType = PRODUCTCODE_ANYBUS_GW;
    }

	for (unsigned int i = 0; i < m_oMyVIConfigParser.m_inCommandList.size(); ++i) {
		COMMAND_INFORMATION* info = &(m_oMyVIConfigParser.m_inCommandList[i]);
//		if(proxyID.nCountInstance == info->proxyInfo.nInstance
//				&& proxyID.nProductCode == info->proxyInfo.nProductCode
//				&& proxyID.nVendorID == info->proxyInfo.nVendorID
//		){
        uint32_t oProxyProductType = 0x00;
        if (info->proxyInfo.nProductCode == PRODUCTCODE_ANYBUS_GW)
        {
            oProxyProductType = PRODUCTCODE_ANYBUS_GW;
        }
		if((static_cast<unsigned int>(p_oInstance) == info->proxyInfo.nInstance) &&
           (oProductType == oProxyProductType))
		{
			switch (info->commandType) {
                case ETriggerStartStopContinuously:{
                    if (isContinuouslyModeActive()) // do in continuously mode
                    {
                        unsigned int oProductType = 1; // ProductType 0 ist Live-Produkt !
                        if (m_pProductTypeInfo != NULL)
                        {
                            oProductType = GetBits((unsigned char*)p_pData, *m_pProductTypeInfo);
                            if (oProductType == 0) oProductType = 1; // ProductType 0 ist Live-Produkt !
                        }
                        unsigned int oProductNumber = 0;
                        if (m_pProductNumberInfo != NULL)
                        {
                            oProductNumber = GetBits((unsigned char*)p_pData, *m_pProductNumberInfo);
                        }
                        bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
                        TriggerContinuously(oTriggerBit, oProductType, oProductNumber);
                        AutoInspectCycleGeneration();
                    }
                    break;
                }
                case ETriggerStartStopAutomatic:{
                    if (!isContinuouslyModeActive()) // do in normal cyclic mode
                    {
                        if (m_pChangeToStandardModeInfo != nullptr)
                        {
                            m_oChangeToStandardMode = CheckBit((unsigned char*)p_pData, *m_pChangeToStandardModeInfo);
                        }
                        unsigned int oProductType = 1; // ProductType 0 ist Live-Produkt !
                        if (m_pProductTypeInfo != NULL)
                        {
                            oProductType = GetBits((unsigned char*)p_pData, *m_pProductTypeInfo);
                            if (oProductType == 0) oProductType = 1; // ProductType 0 ist Live-Produkt !
                        }
                        unsigned int oProductNumber = 0;
                        if (m_pProductNumberInfo != NULL)
                        {
                            oProductNumber = GetBits((unsigned char*)p_pData, *m_pProductNumberInfo);
                        }
                        std::string oExtendedProductInfo(100, 0x00);
                        if (isFieldbusExtendedProductInfoEnabled())
                        {
                            if (m_pExtendedProductInfoInfo != nullptr)
                            {
                                GetString((unsigned char*) p_pData, *m_pExtendedProductInfoInfo, oExtendedProductInfo);
                            }
                        }
                        else
                        {
                            oExtendedProductInfo.assign("no info");
                        }
                        bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
                        TriggerAutomatic(oTriggerBit, oProductType, oProductNumber, oExtendedProductInfo);
                    }
                    break;
                }
				case ETriggerStartStopCalibration:{
					unsigned int oCalibrationType = 0;
					if (m_pCalibrationTypeInfo != NULL)
					{
						oCalibrationType = GetBits((unsigned char*)p_pData, *m_pCalibrationTypeInfo);
					}
					bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
					if (!m_oTriggerCalibrationBlocked) TriggerCalibration(oTriggerBit, oCalibrationType);
					break;
				}
				case ETriggerHomingYAxis:{
					bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
					TriggerHomingYAxis(oTriggerBit);
					break;
				}
                case ETriggerInspectionInfo:{
                    if (!isContinuouslyModeActive()) // do in normal cyclic mode
                    {
                        unsigned int oSeamseries = 0;
                        if (m_pSeamseriesNrInfo != NULL)
                        {
                            oSeamseries = GetBits((unsigned char*)p_pData, *m_pSeamseriesNrInfo);
                            if (oSeamseries != 0) oSeamseries--;
                        }
                        bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
                        TriggerInspectInfo(oTriggerBit, oSeamseries);
                    }
                    break;
                }
                case ETriggerInspectionPreStart:{
                    // do in normal cyclic mode
                    if ((!isContinuouslyModeActive()) &&
                        (!isSCANMASTER_Application()))
                    {
                        unsigned int oSeamNr = 0;
                        if (m_pSeamNrInfo != NULL)
                        {
                            oSeamNr = GetBits((unsigned char*)p_pData, *m_pSeamNrInfo);
                            if (oSeamNr != 0) oSeamNr--;
                        }
                        bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
                        TriggerInspectionLWMPreStart(oTriggerBit, oSeamNr);
                    }
                    break;
                }
                case ETriggerInspectionStartEnd:{
                    // do in normal cyclic mode
                    if ((!isContinuouslyModeActive()) &&
                        (!isSCANMASTER_Application()))
                    {
                        unsigned int oSeamNr = 0;
                        if (m_pSeamNrInfo != NULL)
                        {
                            oSeamNr = GetBits((unsigned char*)p_pData, *m_pSeamNrInfo);
                            if (oSeamNr != 0) oSeamNr--;
                        }
                        bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
                        TriggerInspectStartStop(oTriggerBit, oSeamNr);
                    }
                    break;
                }
				case ETriggerUnblockLineLaser:{
					bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
					TriggerUnblockLineLaser(oTriggerBit);
					break;
				}
				case ETriggerQuitSystemFault:
				{
					bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
					TriggerQuitSystemFault(oTriggerBit);
					break;
				}
				case EInvertedTriggerEmergencyStop:{
					bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
					TriggerEmergencyStop(!oTriggerBit);
					break;
				}
				case ECabinetTemperatureOk:{
					if (isCabinetTemperatureOkEnabled())
					{
						bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
						TriggerCabinetTemperatureOk(oTriggerBit);
					}
					break;
				}
				case EGenPurposeDigIn1:{
					pthread_mutex_lock(&m_oGenPurposeDigIn1Mutex);
					// Rueckgabe von GetBits in 16 Bit mit Vorzeichen wandeln
					short oTempShort = (short) GetBits((unsigned char*)p_pData,*info);
					if(isGenPurposeDigIn1Enabled())
					{
						m_oGenPurposeDigIn1_Single.store( (int) oTempShort);
					}
					else if (isGenPurposeDigInMultipleEnabled())
					{
					}
					else
					{
						m_oGenPurposeDigIn1 = (int) oTempShort;
					}
					pthread_mutex_unlock(&m_oGenPurposeDigIn1Mutex);
					break;
				}
                case EGenPurposeDigInTakeOver:{
                    if (isGenPurposeDigInMultipleEnabled())
                    {
                        if (!isContinuouslyModeActive()) // do in normal cyclic mode
                        {
                            // pruefen auf Uebernahmesignal und bei steigender Flanke kopieren der Zwischenvariablen in die Zielvariable
                            // Quittierungssignal fuer Feldbus setzen bzw. ruecksetzen
                            unsigned int oGenPurposeDigInAddress = 0;
                            if (m_pGenPurposeDigInAddressInfo != NULL)
                            {
                                oGenPurposeDigInAddress = GetBits((unsigned char*)p_pData, *m_pGenPurposeDigInAddressInfo);
                            }
                            int oGenPurposeDigInValue = 0;
                            if (m_pGenPurposeDigIn1Info != NULL)
                            {
                                short oTempShort = (short) GetBits((unsigned char*)p_pData,*m_pGenPurposeDigIn1Info);
                                oGenPurposeDigInValue = (int) oTempShort;
                            }
                            bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
                            TriggerGenPurposeDigInTakeOver(oTriggerBit, oGenPurposeDigInAddress, oGenPurposeDigInValue);
                        }
                    }
                    break;
                }
                case EGenPurposeDigOutTakeOver:{
                    if (isGenPurposeDigOutMultipleEnabled())
                    {
                        if (!isContinuouslyModeActive()) // do in normal cyclic mode
                        {
                            // pruefen auf Uebernahmesignal und bei steigender Flanke kopieren der Ergebnisvariablen auf den Feldbus
                            // Quittierungssignal fuer Feldbus setzen bzw. ruecksetzen
                            unsigned int oGenPurposeDigInAddress = 0;
                            if (m_pGenPurposeDigInAddressInfo != NULL)
                            {
                                oGenPurposeDigInAddress = GetBits((unsigned char*)p_pData, *m_pGenPurposeDigInAddressInfo);
                            }
                            bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
                            TriggerGenPurposeDigOutTakeOver(oTriggerBit, oGenPurposeDigInAddress);
                        }
                    }
                    break;
                }
				case ETriggerProductNumberFull:{
					if (isFieldbusInterfaceFullEnabled())
					{
						unsigned int oProductTypeFull = 1; // ProductType 0 ist Live-Produkt !
						if (m_pGetProductTypeFullInfo != NULL)
						{
							oProductTypeFull = GetBits((unsigned char*)p_pData, *m_pGetProductTypeFullInfo);
							if (oProductTypeFull == 0) oProductTypeFull = 1; // ProductType 0 ist Live-Produkt !
						}
						unsigned int oProductNumberFull = 0;
						if (m_pGetProductNumberFullInfo != NULL)
						{
							oProductNumberFull = GetBits((unsigned char*)p_pData, *m_pGetProductNumberFullInfo);
						}
						bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
						TriggerProductNumberFull(oTriggerBit, oProductTypeFull, oProductNumberFull);
					}
					break;
				}
				case EAcknResultsReadyFull:{
					if (isFieldbusInterfaceFullEnabled())
					{
						bool oAnswerBit = CheckBit((unsigned char*)p_pData, *info);
						pthread_mutex_lock(&m_oAcknResultsReadyFullMutex);
						m_oAcknResultsReadyFull = oAnswerBit;
						pthread_mutex_unlock(&m_oAcknResultsReadyFullMutex);
						HandshakeForResultsReady();
					}
					break;
				}
				case ETriggerQuitSystemFaultFull:{
					if (isFieldbusInterfaceFullEnabled())
					{
						bool oTriggerBit = CheckBit((unsigned char*)p_pData, *info);
						TriggerQuitSystemFaultFull(oTriggerBit);
					}
					break;
				}

				case ESeamseriesNr:{
					pthread_mutex_lock(&m_oSeamSeriesMutex);
					m_oSeamseriesShadow = GetBits((unsigned char*)p_pData,*info);
					if (m_oSeamseriesShadow != 0) m_oSeamseriesShadow--;
					pthread_mutex_unlock(&m_oSeamSeriesMutex);

					break;
				}
				case ESeamNr:{
					pthread_mutex_lock(&m_oSeamNrMutex);
					m_oSeamNrShadow = GetBits((unsigned char*)p_pData,*info);
					if (m_oSeamNrShadow != 0) m_oSeamNrShadow--;
					pthread_mutex_unlock(&m_oSeamNrMutex);

					break;
				}


                case E_S6K_SouvisActive:
                {
                    m_oS6K_SouvisActiveStatus = CheckBit((unsigned char*)p_pData, *info);
                    break;
                }
                case E_S6K_SouvisInspection:
                {
                    m_oS6K_SouvisInspectionStatus = CheckBit((unsigned char*)p_pData, *info);
                    break;
                }
                case E_S6K_QuitSystemFault:
                {
                    m_oS6K_QuitSystemFaultStatus = CheckBit((unsigned char*)p_pData, *info);
                    break;
                }
                case E_S6K_MachineReady:
                {
                    m_oS6K_MachineReadyStatus = CheckBit((unsigned char*)p_pData, *info);
                    break;
                }
                case E_S6K_AcknQualityData:
                {
                    m_oS6K_AcknQualityDataStatus = CheckBit((unsigned char*)p_pData, *info);
                    break;
                }
                case E_S6K_CycleDataValid:
                {
                    m_oS6K_CycleDataInput = 0;
                    if (m_pS6K_CycleData_Info != NULL)
                    {
                        m_oS6K_CycleDataInput = GetBits((unsigned char*)p_pData, *m_pS6K_CycleData_Info);
                    }
                    m_oS6K_ProductNumberInput = 0;
                    if (m_pS6K_ProductNumber_Info != NULL)
                    {
                        m_oS6K_ProductNumberInput = GetBits((unsigned char*)p_pData, *m_pS6K_ProductNumber_Info);
                    }
                    m_oS6K_CycleDataValidStatus = CheckBit((unsigned char*)p_pData, *info);
                    break;
                }
                case E_S6K_SeamNoValid:
                {
                    if ( !isSOUVIS6000_Automatic_Seam_No() )
                    {
                        static bool oldSeamNoValidStatus{CheckBit((unsigned char*)p_pData, *info)};
                        bool newSeamNoValidStatus = CheckBit((unsigned char*)p_pData, *info);
                        if (newSeamNoValidStatus != oldSeamNoValidStatus)
                        {
                            if (newSeamNoValidStatus) // rising edge
                            {
                                wmLog(eDebug, "E_S6K_SeamNoValid: newSeamNoValidStatus: %d\n", newSeamNoValidStatus);
                                static uint32_t newSeamNoInput{0};
                                if (m_pS6K_SeamNo_Info != NULL)
                                {
                                    newSeamNoInput = GetBits((unsigned char*)p_pData, *m_pS6K_SeamNo_Info);
                                    wmLog(eDebug, "E_S6K_SeamNoValid: newSeamNoInput: %d\n", newSeamNoInput);
                                }
                                m_oS6K_SeamNoInput.store(newSeamNoInput);
                            }
                            else // falling edge
                            {
                                wmLog(eDebug, "E_S6K_SeamNoValid: newSeamNoValidStatus: %d\n", newSeamNoValidStatus);
                            }
                            oldSeamNoValidStatus = newSeamNoValidStatus;
                        }
                        m_oS6K_SeamNoValidStatus.store(newSeamNoValidStatus);
                    }
                    break;
                }
                case E_S6K_AcknResultData:
                {
                    m_oS6K_AcknResultDataStatus = CheckBit((unsigned char*)p_pData, *info);
                    break;
                }
                case E_SM_AcknowledgeStep:
                {
                    m_oSM_AcknowledgeStepStatus.store(CheckBit((unsigned char*)p_pData, *info));
                    break;
                }

                case E_S6K_MakePictures:
                {
                    m_oS6K_MakePicturesStatus.store(CheckBit((unsigned char*)p_pData, *info));
                    break;
                }

				default:{
					break;
				}
			}
		}
	}
	if (m_oWaitingForCycleAckn)
	{
		m_oCycleAcknTimeoutCounter++;
		if (m_oCycleAcknTimeoutCounter > 5000) // 5 Sekunden
		{
			wmFatal(eInternalError, "QnxMsg.VI.CycStartTimeout", "system cannot acknowledge cycle start\n");
			m_oWaitingForCycleAckn = false;
		}
	}

    if (p_oInstance == eInstance1) // use only Instance 1 for timer counting, otherwise time is too short
    {
        if (isContinuouslyModeActive()) // do in continuously mode
        {
            if (m_oSumErrorSeamSeriesIsHigh)
            {
                m_oSumErrorSeamSeriesCounter++;
                if (m_oSumErrorSeamSeriesCounter >= PULSE_WIDTH_ERROR_SIGNAL_CONTINUOUS) // value in ms
                {
                    BoolSender(false, &m_senderSumErrorSeamSeries);
                    m_oQualityErrorAccuCycle = 0x0000;
                    FieldSender((uint64_t)m_oQualityErrorAccuCycle, &m_senderQualityErrorField);
                    m_oSumErrorSeamSeriesIsHigh = false;
                }
            }
        }
    }
}

#define LOCK_TIME_FOR_ANALOG_INPUTS    20

void VI_InspectionControl::ProxyReceiveAnalog(EcatProductIndex productIndex, EcatInstance p_oInstance, EcatChannel p_oChannel, int16_t p_oData)
{
    uint32_t oProductType = 0x00;
    if (productIndex == eProductIndex_EL3102)
    {
        oProductType = PRODUCTCODE_EL3102;
    }

    for (unsigned int i = 0; i < m_oMyVIConfigParser.m_inCommandList.size(); ++i)
    {
        COMMAND_INFORMATION* info = &(m_oMyVIConfigParser.m_inCommandList[i]);
        if((static_cast<unsigned int>(p_oInstance) == info->proxyInfo.nInstance) &&
           (oProductType == info->proxyInfo.nProductCode) &&
           (p_oChannel == info->proxyInfo.m_oChannel))
        {
            switch (info->commandType)
            {
                case ETriggerStartStopAutomatic:
                {
                    if (!isContinuouslyModeActive()) // do in normal cyclic mode
                    {
                        bool oTriggerState = (p_oData >= m_oAnalogTriggerLevelBin);
                        static bool oOldTriggerState = (p_oData >= m_oAnalogTriggerLevelBin);
                        static int oLockCounter = 0;
                        unsigned int oProductType = 1; // ProductType 0 ist Live-Produkt !
                        unsigned int oProductNumber = 0; // serial number of part
                        if (oLockCounter <= 0)
                        {
                            TriggerAutomatic(oTriggerState, oProductType, oProductNumber, "no info");
                            if (oTriggerState != oOldTriggerState)
                            {
                                oOldTriggerState = oTriggerState;
                                oLockCounter = LOCK_TIME_FOR_ANALOG_INPUTS - 1;
                            }
                        }
                        else
                        {
                            TriggerAutomatic(oOldTriggerState, oProductType, oProductNumber, "no info");
                            oLockCounter--;
                        }
                    }
                    break;
                }
                case ETriggerInspectionStartEnd:
                {
                    // do in normal cyclic mode
                    if ((!isContinuouslyModeActive()) &&
                        (!isSCANMASTER_Application()))
                    {
                        bool oTriggerState = (p_oData >= m_oAnalogTriggerLevelBin);
                        static bool oOldTriggerState = (p_oData >= m_oAnalogTriggerLevelBin);
                        static int oLockCounter = 0;
                        if (oLockCounter <= 0)
                        {
                            TriggerInspectStartStop(oTriggerState, m_oSeamNoInAnalogMode);
                            if (oTriggerState != oOldTriggerState)
                            {
                                oOldTriggerState = oTriggerState;
                                oLockCounter = LOCK_TIME_FOR_ANALOG_INPUTS - 1;
                            }
                        }
                        else
                        {
                            TriggerInspectStartStop(oOldTriggerState, m_oSeamNoInAnalogMode);
                            oLockCounter--;
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
}

void VI_InspectionControl::ProxyReceiveAnalogOversamp(EcatProductIndex productIndex, EcatInstance p_oInstance, EcatChannel p_oChannel, uint8_t p_oSize, stdVecINT16 p_oData)
{
    uint32_t oProductType = 0x00;
    if (productIndex == eProductIndex_EL3702)
    {
        oProductType = PRODUCTCODE_EL3702;
    }

    for (unsigned int i = 0; i < m_oMyVIConfigParser.m_inCommandList.size(); ++i)
    {
        COMMAND_INFORMATION* info = &(m_oMyVIConfigParser.m_inCommandList[i]);
        if((static_cast<unsigned int>(p_oInstance) == info->proxyInfo.nInstance) &&
           (oProductType == info->proxyInfo.nProductCode) &&
           (p_oChannel == info->proxyInfo.m_oChannel))
        {
            switch (info->commandType)
            {
                case ETriggerStartStopAutomatic:
                {
                    if (!isContinuouslyModeActive()) // do in normal cyclic mode
                    {
                        bool oTriggerState = (p_oData[p_oSize - 2] >= m_oAnalogTriggerLevelBin);
                        static bool oOldTriggerState = (p_oData[p_oSize - 2] >= m_oAnalogTriggerLevelBin);
                        static int oLockCounter = 0;
                        unsigned int oProductType = 1; // ProductType 0 ist Live-Produkt !
                        unsigned int oProductNumber = 0; // serial number of part
                        if (oLockCounter <= 0)
                        {
                            TriggerAutomatic(oTriggerState, oProductType, oProductNumber, "no info");
                            if (oTriggerState != oOldTriggerState)
                            {
                                oOldTriggerState = oTriggerState;
                                oLockCounter = LOCK_TIME_FOR_ANALOG_INPUTS - 1;
                            }
                        }
                        else
                        {
                            TriggerAutomatic(oOldTriggerState, oProductType, oProductNumber, "no info");
                            oLockCounter--;
                        }
                    }
                    break;
                }
                case ETriggerInspectionStartEnd:
                {
                    // do in normal cyclic mode
                    if ((!isContinuouslyModeActive()) &&
                        (!isSCANMASTER_Application()))
                    {
                        bool oTriggerState = (p_oData[p_oSize - 2] >= m_oAnalogTriggerLevelBin);
                        static bool oOldTriggerState = (p_oData[p_oSize - 2] >= m_oAnalogTriggerLevelBin);
                        static int oLockCounter = 0;
                        if (oLockCounter <= 0)
                        {
                            TriggerInspectStartStop(oTriggerState, m_oSeamNoInAnalogMode);
                            if (oTriggerState != oOldTriggerState)
                            {
                                oOldTriggerState = oTriggerState;
                                oLockCounter = LOCK_TIME_FOR_ANALOG_INPUTS - 1;
                            }
                        }
                        else
                        {
                            TriggerInspectStartStop(oOldTriggerState, m_oSeamNoInAnalogMode);
                            oLockCounter--;
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
}

bool VI_InspectionControl::CheckBit(unsigned char data, COMMAND_INFORMATION& info)
{
	//8 Bit Dig
	int bitNr = info.proxyInfo.nStartBit;
	unsigned char mask = 1 << (bitNr);
	unsigned char result = data & mask;
	return result;
}

bool VI_InspectionControl::CheckBit(unsigned char* data, COMMAND_INFORMATION& info)
{
	//Gateway
	int bitNr = info.proxyInfo.nStartBit;
	unsigned char mask = 1 << (bitNr % 8);
	unsigned char result = data[bitNr / 8] & mask;
	return result;
}

int VI_InspectionControl::GetBits(unsigned char data, COMMAND_INFORMATION& info){

	int startBit = info.proxyInfo.nStartBit;
	int nBits = info.proxyInfo.nLength;
	unsigned char mask = 0;
	mask = (0xff >> (8-nBits));
	mask = mask << startBit;
	unsigned char result = (data & mask) >> startBit;

	return result;
}

unsigned int VI_InspectionControl::GetBits(unsigned char* p_pData, COMMAND_INFORMATION& p_rInfo)
{
	int oStartBit = p_rInfo.proxyInfo.nStartBit;
	int oNumBits = p_rInfo.proxyInfo.nLength;

	unsigned int oResult = 0; // result variable
	unsigned int oResultMask = 0x01; // mask for the next bit in the result variable

	// do for all configured bits, starting with the first
    for(int oLoop = oStartBit;oLoop < oStartBit + oNumBits;oLoop++)
    {
    	int oByteNo = oLoop / 8; // this is the byte position of the actual bit in the byte stream (data)
    	int oRem    = oLoop % 8; // this is the bit position of the actual bit in the byte
    	int oMask = 1 << oRem; // this ist the mask for the actual bit in the byte
    	if (p_pData[oByteNo] & oMask) // when bit is set then...
    	{
    		oResult |= oResultMask; // ...set the next bit in the result
    	}
    	oResultMask = oResultMask << 1; // shift mask for the next bit in result
    }
	return oResult;
}

void VI_InspectionControl::GetString(unsigned char* p_pData, COMMAND_INFORMATION& p_rInfo, std::string& p_oStringObject)
{
    int oStartByte = p_rInfo.proxyInfo.nStartBit / 8;
    if ((p_rInfo.proxyInfo.nStartBit % 8) != 0)
    {
        wmLog(eError, "Start of a string must be on a byte boundary !\n");
    }

    int oLength = p_rInfo.proxyInfo.nLength / 8;
    if ((p_rInfo.proxyInfo.nLength % 8) != 0)
    {
        wmLog(eError, "Length of a string must be a multiple of a byte !\n");
    }

    p_oStringObject.assign((const char *)&p_pData[oStartByte], oLength);
}

void VI_InspectionControl::activateControlSimulation(bool p_oState)
{
    m_oControlSimulationIsOn = p_oState;
    TriggerAutomatic(false, 0, 0, "no info"); // dummy call for initialize the static var inside the function with false !
};

void VI_InspectionControl::genPurposeDigIn(uint8_t p_oAddress, int16_t p_oDigInValue)
{
    if (isGenPurposeDigIn1Enabled())
    {
        m_oGenPurposeDigIn1_Single.store( (int) p_oDigInValue);
    }
    else if (isGenPurposeDigInMultipleEnabled())
    {
        if (!m_oContinuouslyModeActive)
        {
            TriggerGenPurposeDigInTakeOver(true, p_oAddress, p_oDigInValue);
            TriggerGenPurposeDigInTakeOver(false, p_oAddress, p_oDigInValue);
        }
    }
    else
    {
        pthread_mutex_lock(&m_oGenPurposeDigIn1Mutex);
        m_oGenPurposeDigIn1 = (int) p_oDigInValue;
        pthread_mutex_unlock(&m_oGenPurposeDigIn1Mutex);
    }
}

//############################################################################################################
//############################################################################################################

// is only called if m_HMinGatewayInfo.isValid == true !
void VI_InspectionControl::IncomingHMSignals(uint8_t value)
{
	static bool oFirstCall{true};

	unsigned int& nInGlasNotPresent = m_oMyVIConfigParser.m_HMinGatewayInfo.bitGlasNotPresent;
	unsigned int& nInGlasDirty = m_oMyVIConfigParser.m_HMinGatewayInfo.bitGlasDirty;
	unsigned int& nInTempGlasFail = m_oMyVIConfigParser.m_HMinGatewayInfo.bitTempGlasFail;
	unsigned int& nInTempHeadFail = m_oMyVIConfigParser.m_HMinGatewayInfo.bitTempHeadFail;

	unsigned char mask = 0;

	mask = 1 << (nInGlasNotPresent);
	bool bGlasNotPresent = value & mask;
	mask = 0;

	mask = 1 << (nInGlasDirty);
	bool bGlasDirty = value & mask;
	mask = 0;

	mask = 1 << (nInTempGlasFail);
	bool bTempGlasFail = value & mask;
	mask = 0;

	mask = 1 << (nInTempHeadFail);
	bool bTempHeadFail = value & mask;
	mask = 0;

	bool sendHMSignals = false;

	Poco::FastMutex::ScopedLock lock(m_mutex);

	if ((bGlasNotPresent != m_bGlasNotPresent) || oFirstCall)
	{
		if (oFirstCall) m_bGlasNotPresentCounter = SIGNAL_CHANGE_DELTA;
		if (++m_bGlasNotPresentCounter >= SIGNAL_CHANGE_DELTA)
		{
			m_bGlasNotPresentCounter = 0;
			m_bGlasNotPresent = bGlasNotPresent;
			sendHMSignals = true;
			if (bGlasNotPresent)
			{
				wmLogTr( eInfo, "QnxMsg.VI.HM_GlasPres_ok", "Head-Monitor signals: Glas present\n" );
			}
			else
			{
				if (sendsHeadMonitorNotReady())
				{
					wmFatal( eHeadMonitor, "QnxMsg.VI.HM_GlasPres_nok", "Head-Monitor signals: Glas not present\n" );
				}
				else
				{
					wmLogTr( eError, "QnxMsg.VI.HM_GlasPres_nok", "Head-Monitor signals: Glas not present\n" );
				}
			}
		}
	}
	else
	{
		if (m_bGlasNotPresentCounter > 0)
		{
			m_bGlasNotPresentCounter = 0;
		}
	}

	if ((bGlasDirty != m_bGlasDirty) || oFirstCall)
	{
		if (oFirstCall) m_bGlasDirtyCounter = SIGNAL_CHANGE_DELTA;
		if (++m_bGlasDirtyCounter >= SIGNAL_CHANGE_DELTA)
		{
			m_bGlasDirtyCounter = 0;
			m_bGlasDirty = bGlasDirty;
			sendHMSignals = true;
			if (bGlasDirty)
			{
				wmLogTr( eInfo, "QnxMsg.VI.HM_GlasDirty_ok", "Head-Monitor signals: Glas clean\n" );
			}
			else
			{
				if (sendsHeadMonitorNotReady())
				{
					wmFatal( eHeadMonitor, "QnxMsg.VI.HM_GlasDirty_nok", "Head-Monitor signals: Glas dirty\n" );
				}
				else
				{
					wmLogTr( eError, "QnxMsg.VI.HM_GlasDirty_nok", "Head-Monitor signals: Glas dirty\n" );
				}
			}
		}
	}
	else
	{
		if (m_bGlasDirtyCounter > 0)
		{
			m_bGlasDirtyCounter = 0;
		}
	}

	if ((bTempGlasFail != m_bTempGlasFail) || oFirstCall)
	{
		if (oFirstCall) m_bTempGlasFailCounter = SIGNAL_CHANGE_DELTA;
		if (++m_bTempGlasFailCounter >= SIGNAL_CHANGE_DELTA)
		{
			m_bTempGlasFailCounter = 0;
			m_bTempGlasFail = bTempGlasFail;
			sendHMSignals = true;
			if (bTempGlasFail)
			{
				wmLogTr( eInfo, "QnxMsg.VI.HM_GlasTemp_ok", "Head-Monitor signals: Glas temperature ok\n" );
			}
			else
			{
				if (sendsHeadMonitorNotReady())
				{
					wmFatal( eHeadMonitor, "QnxMsg.VI.HM_GlasTemp_nok", "Head-Monitor signals: Glas temperature too high\n" );
				}
				else
				{
					wmLogTr( eError, "QnxMsg.VI.HM_GlasTemp_nok", "Head-Monitor signals: Glas temperature too high\n" );
				}
			}
		}
	}
	else
	{
		if (m_bTempGlasFailCounter > 0)
		{
			m_bTempGlasFailCounter = 0;
		}
	}

	if ((bTempHeadFail != m_bTempHeadFail) || oFirstCall)
	{
		if (oFirstCall) m_bTempHeadFailCounter = SIGNAL_CHANGE_DELTA;
		if (++m_bTempHeadFailCounter >= SIGNAL_CHANGE_DELTA)
		{
			m_bTempHeadFailCounter = 0;
			m_bTempHeadFail = bTempHeadFail;
			sendHMSignals = true;
			if (bTempHeadFail)
			{
				wmLogTr( eInfo, "QnxMsg.VI.HM_HeadTemp_ok", "Head-Monitor signals: Head temperature ok\n" );
			}
			else
			{
				if (sendsHeadMonitorNotReady())
				{
					wmFatal( eHeadMonitor, "QnxMsg.VI.HM_HeadTemp_nok", "Head-Monitor signals: Head temperature too high\n" );
				}
				else
				{
					wmLogTr( eError, "QnxMsg.VI.HM_HeadTemp_nok", "Head-Monitor signals: Head temperature too high\n" );
				}
			}
		}
	}
	else
	{
		if (m_bTempHeadFailCounter > 0)
		{
			m_bTempHeadFailCounter = 0;
		}
	}

	if (oFirstCall)
	{
		wmLog(eDebug, "First call of IncomingHMSignals\n");
		oFirstCall = false;
	}

	if (sendHMSignals)
	{
		SendHMSignals();
	}
}

// is only called if IncomingHMSignals is called, that is if m_HMinGatewayInfo.isValid == true !
void VI_InspectionControl::SendHMSignals()
{
    unsigned int& nOutGlasNotPresent = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitGlasNotPresent;
    unsigned int& nOutGlasDirty = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitGlasDirty;
    unsigned int& nOutTempGlasFail = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempGlasFail;
    unsigned int& nOutTempHeadFail = m_oMyVIConfigParser.m_HMoutGatewayInfo.bitTempHeadFail;

    if(m_oMyVIConfigParser.m_HMoutGatewayInfo.isValid == true)
    {
        if(m_oMyVIConfigParser.m_HMoutGatewayInfo.nSlaveType == _DIG_8BIT_)
        {
            unsigned char mask = 0;
            unsigned char sendValue = 0;

            if(m_bGlasNotPresent)
            {
                sendValue = sendValue | (1 << nOutGlasNotPresent);
            }
            if(m_bGlasDirty)
            {
                sendValue = sendValue | (1 << nOutGlasDirty);
            }
            if(m_bTempGlasFail)
            {
                sendValue = sendValue | (1 << nOutTempGlasFail);
            }
            if(m_bTempHeadFail)
            {
                sendValue = sendValue | (1 << nOutTempHeadFail);
            }

            mask = mask | (1<<nOutGlasNotPresent);
            mask = mask | (1<<nOutGlasDirty);
            mask = mask | (1<<nOutTempGlasFail);
            mask = mask | (1<<nOutTempHeadFail);

            sendValue = sendValue >> (m_senderHMOutput.info.proxyInfo.nStartBit % 8);
            FieldSender((uint64_t)sendValue, &m_senderHMOutput);
        }
        else if(m_oMyVIConfigParser.m_HMoutGatewayInfo.nSlaveType == _GATEWAY_)
        {
            unsigned char mask[_GATEWAY_DATA_SIZE_] = {0};
            unsigned char sendValue[_GATEWAY_DATA_SIZE_] = {0};

            //wert setzten
            if(m_bGlasNotPresent)
            {
                int byte = nOutGlasNotPresent/8;
                sendValue[byte] = (sendValue[byte]) |  (1 << (nOutGlasNotPresent%8));
            }
            if(m_bGlasDirty)
            {
                int byte = nOutGlasDirty/8;
                sendValue[byte] = (sendValue[byte]) |  (1 << (nOutGlasDirty%8));
            }
            if(m_bTempGlasFail)
            {
                int byte = nOutTempGlasFail/8;
                sendValue[byte] = (sendValue[byte]) |  (1 << (nOutTempGlasFail%8));
            }
            if(m_bTempHeadFail)
            {
                int byte = nOutTempHeadFail/8;
                sendValue[byte] = (sendValue[byte]) |  (1 << (nOutTempHeadFail%8));
            }

            int byte = 0;
            //maske setzten
            byte = nOutGlasNotPresent/8;
            mask[byte] = (mask[byte]) | (1 << (nOutGlasNotPresent%8));

            byte = nOutGlasDirty/8;
            mask[byte] = (mask[byte]) | (1 << (nOutGlasDirty%8));

            byte = nOutTempGlasFail/8;
            mask[byte] = (mask[byte]) | (1 << (nOutTempGlasFail%8));

            byte = nOutTempHeadFail/8;
            mask[byte] = (mask[byte]) | (1 << (nOutTempHeadFail%8));

            // TODO pruefen, ob alle Bits im gleichen Byte liegen
            sendValue[byte] = sendValue[byte] >> (m_senderHMOutput.info.proxyInfo.nStartBit % 8);
            FieldSender((uint64_t)sendValue[byte], &m_senderHMOutput);
        }
    }

    if (isFieldbusInterfaceFullEnabled())
    {
        unsigned int oBuffer = 0;

        if(m_bGlasNotPresent)
        {
            oBuffer |= 0x01;
        }
        if(m_bGlasDirty)
        {
            oBuffer |= 0x02;
        }
        if(m_bTempGlasFail)
        {
            oBuffer |= 0x04;
        }
        if(m_bTempHeadFail)
        {
            oBuffer |= 0x08;
        }
        FieldSender((uint64_t)oBuffer, &m_senderSetHMSignalsFieldFull);
    }
}

//############################################################################################################
//############################################################################################################

VI_InspectionControl::~VI_InspectionControl() {

#if CYCLE_TIMING_VIA_SERIAL_PORT
    close(g_oDebugSerialFd);
#endif

    if (m_oSendSensorDataThread_ID != 0)
    {
        if (pthread_cancel(m_oSendSensorDataThread_ID) != 0)
        {
            wmLog(eDebug, "was not able to abort thread\n");
        }
    }

    if (m_oCyclicTaskThread_ID != 0)
    {
        if (pthread_cancel(m_oCyclicTaskThread_ID) != 0)
        {
            wmLog(eDebug, "was not able to abort thread\n");
        }
    }

    pthread_mutex_destroy(&m_oValueToSendMutex);

	pthread_mutex_destroy(&m_oProductNumberFullMutex);
	pthread_mutex_destroy(&m_oProductTypeFullMutex);
	pthread_mutex_destroy(&m_oAcknResultsReadyFullMutex);
	pthread_mutex_destroy(&m_oGenPurposeDigInAddressMutex);
	pthread_mutex_destroy(&m_oGenPurposeDigIn1Mutex);
	pthread_mutex_destroy(&m_oCalibrationTypeMutex);
	pthread_mutex_destroy(&m_oSeamNrMutex);
	pthread_mutex_destroy(&m_oSeamSeriesMutex);
	pthread_mutex_destroy(&m_oExtendedProductInfoMutex);
	pthread_mutex_destroy(&m_oProductNumberMutex);
	pthread_mutex_destroy(&m_oProductTypeMutex);
}

void VI_InspectionControl::TriggerContinuously(bool p_oTriggerBit, unsigned int p_oProductType, unsigned int p_oProductNumber)
{
    static bool oOldState = p_oTriggerBit;

    if (p_oTriggerBit) // Bit ist gesetzt
    {
        if (oOldState) // Bit war bereits gesetzt
        {
        }
        else // Bit war nicht gesetzt -> steigende Flanke
        {
            wmLogTr(eInfo, "QnxMsg.VI.ContModeActiveHigh", "continuously mode started: product-no: %d part-no: %u\n", p_oProductType, p_oProductNumber);
            m_oProductTypeContinuously = p_oProductType;
            FieldSender((uint64_t)p_oProductType, &m_senderProductTypeMirror);
            m_oProductNumberContinuously = p_oProductNumber;
            m_oAutoInspectCycleStarted = true;
        }
    }
    else // Bit ist nicht gesetzt
    {
        if (oOldState) // Bit war gesetzt -> fallende Flanke
        {
            wmLogTr(eInfo, "QnxMsg.VI.ContModeActiveLow", "continuously mode stopped\n");
            m_oAutoInspectCycleStopped = true;
        }
        else // Bit war bereits nicht gesetzt
        {
        }
    }
    oOldState = p_oTriggerBit;
}

void VI_InspectionControl::AutoInspectCycleGeneration(void)
{
    static int oStateVariable = 0;
    static int oWaitCounter = 0;

    switch(oStateVariable)
    {
        case 0: // nothing to do
            {
                if (m_oAutoInspectCycleStarted)
                {
                    m_oAutoInspectIsFirstCycle = true;
                    m_oAutoInspectCycleStarted = false;
                    oWaitCounter = 0;
#if DEBUG_AUTOINSPECT_STATEMACHINE
                    wmLog(eDebug, "AutoInspectCycleGeneration: goto 1\n");
#endif
                    oStateVariable = 1;
                }
            }
            break;
        case 1: // waiting for some milliseconds
            {
                oWaitCounter++;
                if (oWaitCounter >= 1) // 1ms
                {
#if DEBUG_AUTOINSPECT_STATEMACHINE
                    wmLog(eDebug, "AutoInspectCycleGeneration: goto 2\n");
#endif
                    oStateVariable = 2;
                }
            }
            break;
        case 2: // set CycleStart on
            {
                m_oAutoInspectCycleAckn = false;
                if (m_oAutoInspectIsFirstCycle)
                {
#if DEBUG_AUTOINSPECT_STATEMACHINE
                    wmLog(eDebug, "AutoInspect: First cycle\n");
#endif
                }
                else
                {
#if DEBUG_AUTOINSPECT_STATEMACHINE
                    wmLog(eDebug, "AutoInspect: not First cycle\n");
#endif
                }
                TriggerAutomatic(true, m_oProductTypeContinuously, m_oProductNumberContinuously, "no info");
                m_oAutoInspectIsFirstCycle = false;
                oWaitCounter = 0;
#if DEBUG_AUTOINSPECT_STATEMACHINE
                wmLog(eDebug, "AutoInspectCycleGeneration: goto 3\n");
#endif
                oStateVariable = 3;
            }
            break;
        case 3: // waiting for CycleStart Acknowledge
            {
                oWaitCounter++;
                if (m_oAutoInspectCycleAckn)
                {
#if DEBUG_AUTOINSPECT_STATEMACHINE
                    wmLog(eDebug, "AutoInspectCycleGeneration: goto 4 (1)\n");
#endif
                    oStateVariable = 4;
                }
                if (oWaitCounter >= 1500) // 1500ms
                {
#if DEBUG_AUTOINSPECT_STATEMACHINE
                    wmLog(eError, "Timeout while waiting for m_oAutoInspectCycleAckn\n");
                    wmLog(eDebug, "AutoInspectCycleGeneration: goto 4 (2)\n");
#endif
                    oStateVariable = 4;
                }
            }
            break;
        case 4: // set InspectStartStop on
            {
                m_oInspectTimeMsContinuously = 100;
#if DEBUG_AUTOINSPECT_STATEMACHINE
                wmLog(eDebug, "AutoInspectCycleGeneration (1): %d ms\n", m_oInspectTimeMsContinuously.load());
#endif
                TriggerInspectStartStop(true, 0);
                oWaitCounter = 0;
#if DEBUG_AUTOINSPECT_STATEMACHINE
                wmLog(eDebug, "AutoInspectCycleGeneration: goto 5\n");
#endif
                oStateVariable = 5;
            }
            break;
        case 5: // waiting for seam end
            {
                oWaitCounter++;
                if (oWaitCounter >= m_oInspectTimeMsContinuously) // waiting for duration of seam
                {
#if DEBUG_AUTOINSPECT_STATEMACHINE
                    wmLog(eDebug, "AutoInspectCycleGeneration (2): %d ms\n", m_oInspectTimeMsContinuously.load());
                    wmLog(eDebug, "AutoInspectCycleGeneration: goto 6 (1)\n");
#endif
                    oStateVariable = 6;
                }
                if (m_oAutoInspectCycleStopped) // last cycle in continuous mode is shortened
                {
#if DEBUG_AUTOINSPECT_STATEMACHINE
                    wmLog(eDebug, "AutoInspectCycleGeneration: seam is shortened due to end of continuous mode: %d ms\n", oWaitCounter);
                    wmLog(eDebug, "AutoInspectCycleGeneration: goto 6 (2)\n");
#endif
                    oStateVariable = 6;
                }
            }
            break;
        case 6: // set InspectStartStop off
            {
                TriggerInspectStartStop(false, 0);
                oWaitCounter = 0;
#if DEBUG_AUTOINSPECT_STATEMACHINE
                wmLog(eDebug, "AutoInspectCycleGeneration: goto 7\n");
#endif
                oStateVariable = 7;
            }
            break;
        case 7: // waiting for some milliseconds
            {
                oWaitCounter++;
                if (oWaitCounter >= 1) // 1ms
                {
#if DEBUG_AUTOINSPECT_STATEMACHINE
                    wmLog(eDebug, "AutoInspectCycleGeneration: goto 8\n");
#endif
                    oStateVariable = 8;
                }
            }
            break;
        case 8: // set CycleStart off
            {
                TriggerAutomatic(false, m_oProductTypeContinuously, m_oProductNumberContinuously, "no info");
                oWaitCounter = 0;
#if DEBUG_AUTOINSPECT_STATEMACHINE
                wmLog(eDebug, "AutoInspectCycleGeneration: goto 9\n");
#endif
                oStateVariable = 9;
            }
            break;
        case 9: // waiting for some milliseconds
            {
                oWaitCounter++;
                if (oWaitCounter >= 70) // 70ms additional break time, to give the gui some peace ...
                {
                    if (m_oAutoInspectCycleStopped)
                    {
                        // switch off line lasers only at the end of continuous mode
                        inspectionProxy.linelaser(false);
                        m_oAutoInspectCycleStopped = false;
#if DEBUG_AUTOINSPECT_STATEMACHINE
                        wmLog(eDebug, "AutoInspectCycleGeneration: goto 0\n");
#endif
                        oStateVariable = 0;
                    }
                    else
                    {
#if DEBUG_AUTOINSPECT_STATEMACHINE
                        wmLog(eDebug, "AutoInspectCycleGeneration: goto 1\n");
#endif
                        oStateVariable = 1;
                    }
                }
            }
            break;
        default:
            {
            }
            break;
    }
}

void VI_InspectionControl::TriggerAutomatic(bool p_oTriggerBit, unsigned int p_oProductType, unsigned int p_oProductNumber, const std::string& p_rExtendedProductInfo)
{
	static bool oOldState = p_oTriggerBit;

	static unsigned int oProductType = 1; // ProductType 0 ist Live-Produkt !
	static unsigned int oProductNumber = 0;
	static std::string oExtendedProductInfo(100, 0x00);

	pthread_mutex_lock(&m_oProductTypeMutex);
	pthread_mutex_lock(&m_oProductNumberMutex);
	pthread_mutex_lock(&m_oExtendedProductInfoMutex);
	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
            if (isCommunicationToLWMDeviceActive())
            {
                if (m_TCPClientLWM != nullptr)
                {
                    m_TCPClientLWM->setSendRequestWatchdog();
                    triggerLWMSendRequest();
                }
                if (!m_LWMConnectionIsOn)
                {
                    wmFatal(eExtEquipment, "QnxMsg.VI.CommToLWMfailed", "Communication to LWM device is not possible\n");
                }
            }
            if (m_oIsSCANMASTER_Application_BootupState)
            {
                m_oIsSCANMASTER_Application = !m_oChangeToStandardMode; // change to standard mode resp. back to scanmaster mode
            }
            if (isSCANMASTER_Application())
            {
                if (isSCANMASTER_ThreeStepInterface())
                {
                    wmLog(eWarning, "SCANMASTER_ThreeStepInterface is deprecated !\n");
                    wmLog(eWarning, "SCANMASTER_ThreeStepInterface will be removed in a future release !\n");
                    wmLog(eWarning, "Please switch to SCANMASTER_GeneralApplication soon !\n");
                }
            }
			m_oProductType = p_oProductType;
			m_oProductNumber = p_oProductNumber;
			m_oExtendedProductInfo = p_rExtendedProductInfo;
			// m_oProductType ueberschreiben mit dem Wert aus dem Interface Full
			if (isFieldbusInterfaceFullEnabled() && isProductTypeOutOfInterfaceFullEnabled())
			{
				pthread_mutex_lock(&m_oProductTypeFullMutex);
				m_oProductType = m_oProductTypeFull;
				pthread_mutex_unlock(&m_oProductTypeFullMutex);
			}
			// m_oProductNumber ueberschreiben mit dem Wert aus dem Interface Full
			if (isFieldbusInterfaceFullEnabled() && isProductNumberOutOfInterfaceFullEnabled())
			{
				pthread_mutex_lock(&m_oProductNumberFullMutex);
				m_oProductNumber = m_oProductNumberFull;
				pthread_mutex_unlock(&m_oProductNumberFullMutex);
			}
			if (((!m_oMyVIConfigParser.getProductNumberExtern()) || isProductNumberFromWMEnabled()) && not isProductNumberFromUserEnabled() )
			{
				struct timespec oTimeStamp;
				clock_gettime(CLOCK_REALTIME, &oTimeStamp);
				struct tm *ptmVar;
				ptmVar = localtime(&oTimeStamp.tv_sec);
				unsigned int oHelpUInt = 0;
				oHelpUInt  = ptmVar->tm_sec + (ptmVar->tm_min * 100) + (ptmVar->tm_hour * 10000);
				oHelpUInt += (ptmVar->tm_mday * 1000000) + ((ptmVar->tm_mon + 1) * 100000000);
				m_oProductNumber = oHelpUInt;
			}
	        wmLogTr(eInfo, "QnxMsg.VI.InspCycleActiveHigh", "inspection cycle started: product-no: %d part-no: %u\n", m_oProductType, m_oProductNumber);
            if (isContinuouslyModeActive() && ( !m_oAutoInspectIsFirstCycle )) // do in first cycle of continuously mode 
            {
                // bit 31 is used for indicating, that no hardware-parameter-set is processed and no prepositioning of the axes is done
                inspectionProxy.startAutomaticmode((m_oProductType | 0x80000000), m_oProductNumber, m_oExtendedProductInfo);
            }
            else
            {
                inspectionProxy.startAutomaticmode(m_oProductType, m_oProductNumber, m_oExtendedProductInfo);
            }

			oProductType = m_oProductType;
			oProductNumber = m_oProductNumber;
			oExtendedProductInfo.assign(m_oExtendedProductInfo);
            if (!isContinuouslyModeActive()) // do in normal cyclic mode
            {
                BoolSender(false, &m_senderInspectionOK);
                BoolSender(false, &m_senderSumErrorLatched);
                m_oSumErrorSeamSeriesAccu = false;
                BoolSender(false, &m_senderSumErrorSeamSeries);
                m_oSumErrorSeamAccu = false;
                BoolSender(false, &m_senderSumErrorSeam);
                m_oQualityErrorAccuCycle = 0x0000;
                m_oQualityErrorAccuSeamSeries = 0x0000;
                m_oQualityErrorAccuSeam = 0x0000;
                FieldSender((uint64_t)m_oQualityErrorAccuCycle, &m_senderQualityErrorField);
                BoolSender(false, &m_senderInspectionIncomplete);
            }
			m_oSumErrorLatchedAccu = false;
			m_oFirstInspectInfoSet = false;
			m_oInspectionIncomplete = false;
			m_oCycleAcknTimeoutCounter = 0;
			m_oWaitingForCycleAckn = true;
            m_oSeamNoInAnalogMode = 0;
			m_oInspectCycleIsOn = true;
            if (isCommunicationToLWMDeviceActive())
            {
                DetermineLWMParameter(m_oProductType);
            }
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			if (m_oInspectSeamIsOn) // Seam Inspection ist noch aktiv
			{
				// Seam Inspection vorzeitig beenden
				wmLogTr(eWarning, "QnxMsg.VI.CycEndInspActive", "inspection cycle stopped, but there is still a seam inspection active: seam inspection is stopped\n");
				wmLogTr(eInfo, "QnxMsg.VI.InspSeamActiveLow", "seam inspection stopped\n");
                TriggerInspectStartStop(false, m_oSeamNr);
			}
			if (m_oInspectInfoIsOn) // Nahtfolge ist noch aktiv
			{
				// Nahtfolge vorzeitig beenden
				wmLogTr(eWarning, "QnxMsg.VI.CycEndSSeriesActive", "inspection cycle stopped, but there is still a seam series active: seam series is stopped\n");
				wmLogTr(eInfo, "QnxMsg.VI.TakeOverSeamSerLow", "takeover seam-series stopped\n");
				m_oInspectInfoIsOn = false;
			}
			if (m_oInspectCycleIsOn) // Zyklus ist noch aktiv
			{
				wmLogTr(eInfo, "QnxMsg.VI.InspCycleActiveLow", "inspection cycle stopped\n");
				inspectionProxy.stopAutomaticmode();
				FieldSender((uint64_t)oProductType, &m_senderProductTypeMirror);
                if (!isContinuouslyModeActive()) // do in normal cyclic mode
                {
                    StringSender(oExtendedProductInfo, &m_senderExtendedProductInfoMirror);
                    FieldSender((uint64_t)oProductNumber, &m_senderProductNumberMirror);
                    FieldSender((uint64_t)m_oQualityErrorAccuCycle, &m_senderQualityErrorField);
                    if (m_oSumErrorLatchedAccu)
                    {
                        BoolSender(false, &m_senderInspectionOK);
                        BoolSender(true, &m_senderSumErrorLatched);
                    }
                    else
                    {
                        BoolSender(true, &m_senderInspectionOK);
                        BoolSender(false, &m_senderSumErrorLatched);
                    }
                    BoolSender(false, &m_senderInspectCycleAckn); // Signal InspectCycleAckn zuruecksetzen
                    if (m_oInspectionIncomplete) BoolSender(true, &m_senderInspectionIncomplete);
                }
				m_oStartHandshakeForResultsReady = true;
				m_oInspectCycleAck = false;;
				m_oInspectCycleIsOn = false;
			}
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}

	oOldState = p_oTriggerBit;
	pthread_mutex_unlock(&m_oExtendedProductInfoMutex);
	pthread_mutex_unlock(&m_oProductNumberMutex);
	pthread_mutex_unlock(&m_oProductTypeMutex);
}

void VI_InspectionControl::TriggerInspectInfo(bool p_oTriggerBit, unsigned int p_oSeamseries)
{
	static bool oOldState = false;

	pthread_mutex_lock(&m_oSeamSeriesMutex);
	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			m_oSeamseries = p_oSeamseries;
			if (m_oInspectCycleIsOn) // Zyklus ist aktiv
			{
				if (!m_oInspectSeamIsOn) // keine seam inspection aktiv
				{
					wmLogTr(eInfo, "QnxMsg.VI.TakeOverSeamSerHigh", "takeover seam-series started: seam-series-no: %u\n",(m_oSeamseries+1));
					inspectionProxy.info(m_oSeamseries);
                    if (!isContinuouslyModeActive()) // do in normal cyclic mode
                    {
                        BoolSender(false, &m_senderSumErrorSeamSeries);
                        m_oSumErrorSeamSeriesAccu = false;
                        m_oQualityErrorAccuSeamSeries = 0x0000;
                        if (isQualityErrorFieldOnSeamSeries()) // output if switch is on
                        {
                            FieldSender((uint64_t)m_oQualityErrorAccuSeamSeries, &m_senderQualityErrorField);
                        }
                    }
					m_oFirstInspectInfoSet = true;
                    m_oSeamNoInAnalogMode = 0;
					m_oInspectInfoIsOn = true;

                    if ( isSCANMASTER_Application() )
                    {
                        if ( isSCANMASTER_ThreeStepInterface() )
                        {
                            uint32_t oSM_SeamCountTotal = DetermineSeamCount(m_oProductType, m_oSeamseries);
                            if ((oSM_SeamCountTotal % m_oSM_StepCount.load()) != 0)
                            {
                                wmLogTr(eError, "QnxMsg.VI.SMSeamsNotDivis", "Number of seams in seam serie (%d,%d) is not divisable through number of steps (%d)\n",
                                        (m_oSeamseries + 1), oSM_SeamCountTotal, m_oSM_StepCount.load());
                            }
                            oSM_SeamCountTotal /= m_oSM_StepCount.load();
                            m_oSM_SeamCount.store(oSM_SeamCountTotal);
                            wmLog(eDebug, "ProductType: %d, Seamseries: %d, SeamCount: %d, StepCount: %d\n",
                                  m_oProductType, m_oSeamseries, m_oSM_SeamCount.load(), m_oSM_StepCount.load());
                            m_oSM_NIOBuffer.clear();
                            m_oSM_NIOBuffer.resize(m_oSM_SeamCount.load());
                            for(unsigned int i = 0;i < m_oSM_NIOBuffer.size();i++) m_oSM_NIOBuffer[i] = false;
                            m_oSM_SeamseriesNumber.store(m_oSeamseries);
                            if (!m_oControlSimulationIsOn)
                            {
                                m_oSM_StartSeamserieSequence.store(true);
                            }
                        }
                        else if ( isSCANMASTER_GeneralApplication() )
                        {
                            uint32_t oSM_SeamCountTotal = DetermineSeamCount(m_oProductType, m_oSeamseries);
                            m_oSM_SeamCount.store(oSM_SeamCountTotal);
                            wmLog(eDebug, "ProductType: %d, Seamseries: %d, SeamCount: %d\n",
                                  m_oProductType, m_oSeamseries, m_oSM_SeamCount.load());
                            m_oSM_NIOBuffer.clear();
                            m_oSM_NIOBuffer.resize(m_oSM_SeamCount.load());
                            for(unsigned int i = 0;i < m_oSM_NIOBuffer.size();i++) m_oSM_NIOBuffer[i] = false;
                            m_oSM_SeamseriesNumber.store(m_oSeamseries);
                            if (!m_oControlSimulationIsOn)
                            {
                                m_oSM_StartSeamserieSequence.store(true);
                            }
                        }
                        if ( isGenPurposeDigOutMultipleEnabled() )
                        {
                            m_oGenPurposeDigOut1 = 0;
                            m_oGenPurposeDigOut2 = 0;
                            m_oGenPurposeDigOut3 = 0;
                            m_oGenPurposeDigOut4 = 0;
                            m_oGenPurposeDigOut5 = 0;
                            m_oGenPurposeDigOut6 = 0;
                            m_oGenPurposeDigOut7 = 0;
                            m_oGenPurposeDigOut8 = 0;
                        }
                    }
				}
				else
				{
					wmLogTr(eWarning, "QnxMsg.VI.SSeriesInspActive", "takeover seam-series set, but there is a seam inspection active: seam-series ignored\n");
				}
			}
			else // kein aktiver Zyklus
			{
				wmLogTr(eWarning, "QnxMsg.VI.SSeriesNoCycle", "takeover seam-series set, but there is no cycle active: seam-series ignored\n");
			}
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			if (m_oInspectInfoIsOn) // Nahtfolge ist noch aktiv
			{
				wmLogTr(eInfo, "QnxMsg.VI.TakeOverSeamSerLow", "takeover seam-series stopped\n");
				m_oInspectInfoIsOn = false;
			}
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
	pthread_mutex_unlock(&m_oSeamSeriesMutex);
}

void VI_InspectionControl::TriggerInspectionLWMPreStart(bool triggerBit, unsigned int seamNo)
{
    static bool oldState{false};

    pthread_mutex_lock(&m_oProductNumberMutex);
    pthread_mutex_lock(&m_oSeamSeriesMutex);
    if (triggerBit) // bit ist set
    {
        if (oldState) // bit was already set
        {
        }
        else // bit was not set -> rising edge
        {
            m_oSeamNr = seamNo;
            if (m_waitForSimpleIOResultsRanges) // previous seam not totally finished
            {
                wmLogTr(eError, "QnxMsg.VI.PrestartSeamNoEnd", "inspection prestart started, but the previous seam is not totally finished\n");
            }
            if (m_oInspectCycleIsOn) // we have an active cycle
            {
                wmLogTr(eInfo, "QnxMsg.VI.InspSeamPrestHigh", "inspection prestart started: seam-series-no: %u seam-no: %u\n",(m_oSeamseries+1),(m_oSeamNr+1));
                if ((isCommunicationToLWMDeviceActive()) && (m_LWMParameterVector[m_oSeamseries][seamNo].m_LWMInspectionActive))
                {
#if LWM_DEVICE_TRIGGER_SIMULATION
                    setCabinetTemperature(false);
                    usleep(2 * 1000);
#endif
                    // send selection telegram to LWM
                    m_LWMSimpleIOSelectionAckn.store(false);
                    m_LWMSimpleIOErrorReceived.store(false);
                    m_LWMSimpleIOTriggerReceived.store(false);
                    m_delayedSimpleIOSelectionError = false;

                    SimpleIOSelectionDataType simpleIOSelectionData{};
                    simpleIOSelectionData.m_requestAcknowledge = true;
                    simpleIOSelectionData.m_systemActivated = true;
                    simpleIOSelectionData.m_programNumber = m_LWMParameterVector[m_oSeamseries][seamNo].m_LWMProgramNumber;
                    // TODO use also extendedProductInfo ?
                    simpleIOSelectionData.m_comment = std::to_string(m_oProductNumber) + "-" + std::to_string(seamNo + 1);
                    if (m_TCPClientLWM != nullptr)
                    {
                        m_TCPClientLWM->setSendRequestSimpleIOSelection(simpleIOSelectionData);
                        triggerLWMSendRequest();
                    }
                    m_waitForSimpleIOSelectionTimeout = 0;
                    m_waitForSimpleIOSelectionAckn.store(true);
                }
                else
                {
                    m_waitForSimpleIOSelectionAckn.store(false);
                    BoolSender(true, &m_senderInspectionPreStartAckn);
                }
            }
            else // no active cycle
            {
                wmLogTr(eError, "QnxMsg.VI.PrestartNoCycle", "inspection prestart started, but there is no cycle active: inspection prestart ignored\n");
                m_delayedSimpleIOSelectionError = true;
            }
        }
    }
    else // bit is not set
    {
        if (oldState) // bit was set -> falling edge
        {
            wmLogTr(eInfo, "QnxMsg.VI.InspSeamPrestLow", "inspection prestart stopped\n");
        }
        else // bit was already not set
        {
        }
    }
    oldState = triggerBit;
    pthread_mutex_unlock(&m_oSeamSeriesMutex);
    pthread_mutex_unlock(&m_oProductNumberMutex);
}

void VI_InspectionControl::TriggerInspectStartStop(bool p_oTriggerBit, unsigned int p_oSeamNr)
{
	static bool oOldState = false;

	pthread_mutex_lock(&m_oSeamSeriesMutex);
	pthread_mutex_lock(&m_oSeamNrMutex);
	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			m_oSeamNr = p_oSeamNr;
            if (m_waitForSimpleIOSelectionAckn) // previous seam prestart not totally finished
            {
                wmLogTr(eError, "QnxMsg.VI.SeamPrestartNoEnd", "seam inspection started, but the previous seam prestart is not totally finished\n");
            }
			if (m_oInspectCycleIsOn) // Zyklus ist aktiv
			{
				wmLogTr(eInfo, "QnxMsg.VI.InspSeamActiveHigh", "seam inspection started: seam-series-no: %u seam-no: %u\n",(m_oSeamseries+1),(m_oSeamNr+1));
				inspectionProxy.start(m_oSeamNr);
                if (!isContinuouslyModeActive()) // do in normal cyclic mode
                {
                    BoolSender(false, &m_senderSumErrorSeam);
                    m_oSumErrorSeamAccu = false;
                    m_oQualityErrorAccuSeam = 0x0000;
                    if (!isQualityErrorFieldOnSeamSeries()) // output if switch is off
                    {
                        FieldSender((uint64_t)m_oQualityErrorAccuSeam, &m_senderQualityErrorField);
                    }
                    if (isSeamEndDigOut1Enabled())
                    {
                        FieldSender((uint64_t)0, &m_senderGenPurposeDigOut1);
                    }
                    FieldSender((uint64_t)0, &m_senderGenPurposeDigOut1);
                }
				m_oPositionResults = 0;
				m_oSeamHasNoResults = true;
                m_oSeamNoInAnalogMode++;
				m_oInspectSeamIsOn = true;
                if ( !isSCANMASTER_Application() )
                {
                    if ( isGenPurposeDigOutMultipleEnabled() )
                    {
                        m_oGenPurposeDigOut1 = 0;
                        m_oGenPurposeDigOut2 = 0;
                        m_oGenPurposeDigOut3 = 0;
                        m_oGenPurposeDigOut4 = 0;
                        m_oGenPurposeDigOut5 = 0;
                        m_oGenPurposeDigOut6 = 0;
                        m_oGenPurposeDigOut7 = 0;
                        m_oGenPurposeDigOut8 = 0;
                    }
                    if ((isCommunicationToLWMDeviceActive()) && (m_LWMParameterVector[m_oSeamseries][m_oSeamNr].m_LWMInspectionActive))
                    {
                        BoolSender(false, &m_senderInspectionStartEndAckn);
                        m_LWMResultsRangesReceived.store(false);
#if LWM_DEVICE_TRIGGER_SIMULATION
                        setCabinetTemperature(true);
#endif
                    }
                }
			}
			else // kein aktiver Zyklus
			{
				wmLogTr(eError, "QnxMsg.VI.InspNoCycle", "seam inspection started, but there is no cycle active: seam inspection ignored\n");
			}
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			if (m_oInspectSeamIsOn) // Naht Inspektion ist noch aktiv
			{
				wmLogTr(eInfo, "QnxMsg.VI.InspSeamActiveLow", "seam inspection stopped\n");
				inspectionProxy.end(m_oSeamNr);
				if (isSeamEndDigOut1Enabled())
				{
                    if (!isContinuouslyModeActive()) // do in normal cyclic mode
                    {
                        FieldSender((uint64_t)m_oPositionResults, &m_senderGenPurposeDigOut1);
                    }
				}
				m_oInspectSeamIsOn = false;
				if (m_oSeamHasNoResults)
				{
                    if ( !isSCANMASTER_Application() )
                    {
                        if (isSOUVIS6000_Application())
                        {
                            wmFatal(eImageAcquisition, "QnxMsg.VI.InspIncomplete", "seam inspection is incomplete (no results)\n");
                        }
                        else
                        {
                            wmLogTr(eError, "QnxMsg.VI.InspIncomplete", "seam inspection is incomplete (no results)\n");
                            m_oSumErrorLatchedAccu = true;
                            m_oInspectionIncomplete = true;
                            if (isContinuouslyModeActive()) // do in continuously mode
                            {
                                setSumErrorLatched(true);
                            }
                        }
                    }
                }
                if (!isSCANMASTER_Application())
                {
                    BoolSender(false, &m_senderInspectionPreStartAckn);
                    if ((isCommunicationToLWMDeviceActive()) && (m_LWMParameterVector[m_oSeamseries][m_oSeamNr].m_LWMInspectionActive))
                    {
                        if (!m_LWMSimpleIOTriggerReceived.load()) // there is no trigger telegram arrived
                        {
                            wmLogTr(eError, "QnxMsg.VI.ParaWrong1LWM", "There is a problem with the LWM parameterization\n");
                            wmLogTr(eError, "QnxMsg.VI.ParaWrong2LWM", "LWM has not set a trigger\n");
                            SM_SendNOKResultAtTimeout(LWMStandardResult, LWMStandardResult);
                            if (m_TCPClientLWM != nullptr)
                            {
                                m_TCPClientLWM->setSendRequestSimpleIOStopp();
                                triggerLWMSendRequest();
                            }
                        }
                        if (m_delayedSimpleIOSelectionError) // there is a pending error out of LWM selection
                        {
                            SM_SendNOKResultAtTimeout(LWMStandardResult, LWMStandardResult);
                        }
                        m_waitForSimpleIOResultsRangesTimeout = 0;
                        m_waitForSimpleIOResultsRanges.store(true);
                    }
                    else
                    {
                        BoolSender(true, &m_senderInspectionStartEndAckn);
                    }
                }
            }
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
	pthread_mutex_unlock(&m_oSeamNrMutex);
	pthread_mutex_unlock(&m_oSeamSeriesMutex);
}

void VI_InspectionControl::TriggerUnblockLineLaser(bool p_oTriggerBit)
{
	static bool oOldState = false;

	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.LineLaserActiveHigh", "linelaser switched on\n");
			inspectionProxy.linelaser(true);
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.LineLaserActiveLow", "linelaser switched off\n");
			inspectionProxy.linelaser(false);
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
}

void VI_InspectionControl::TriggerCalibration(bool p_oTriggerBit, unsigned int p_oCalibrationType)
{
	static bool oOldState = false;

	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			pthread_mutex_lock(&m_oCalibrationTypeMutex);
			m_oCalibrationType = p_oCalibrationType;
			wmLogTr(eInfo, "QnxMsg.VI.CalActiveHigh", "calibration started: calibration-type: %d\n", m_oCalibrationType);
			m_oCalibResultsBuffer = 0x00;
			FieldSender((uint64_t)m_oCalibResultsBuffer, &m_senderCalibResultsField);
			inspectionCmdProxy.requestCalibration(m_oCalibrationType);
			pthread_mutex_unlock(&m_oCalibrationTypeMutex);
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.CalActiveLow", "calibration stopped\n");
			m_oCalibResultsBuffer &= ~0x01;
			FieldSender((uint64_t)m_oCalibResultsBuffer, &m_senderCalibResultsField);
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
}

void VI_InspectionControl::TriggerHomingYAxis(bool p_oTriggerBit)
{
	static bool oOldState = false;

	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.AxisYHomeHigh", "homing Y-axis started\n");
			inspectionCmdProxy.requestCalibration(eHomingY);
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.AxisYHomeLow", "homing Y-axis stopped\n");
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
}

void VI_InspectionControl::TriggerQuitSystemFault(bool p_oTriggerBit)
{
	static bool oOldState = false;

	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.SystemFaultQuitHigh", "clear system fault rising edge\n");
			inspectionCmdProxy.quitSystemFault();
            if (isSOUVIS6000_Application())
            {
                BoolSender(false, &m_senderS6K_FastStopDoubleBlank);
            }
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.SystemFaultQuitLow", "clear system fault falling edge\n");
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
}

void VI_InspectionControl::TriggerQuitSystemFaultFull(bool p_oTriggerBit)
{
	static bool oOldState = false;

	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.SystemFaultQuitHigh", "clear system fault rising edge\n");
			inspectionCmdProxy.quitSystemFault();
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.SystemFaultQuitLow", "clear system fault falling edge\n");
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
}

void VI_InspectionControl::TriggerEmergencyStop(bool p_oTriggerBit)
{
	static bool oOldState = false;

	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.EmergencyStop", "emergency stop\n");
			m_oTriggerEmergencyStop = true;
			inspectionCmdProxy.emergencyStop();
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.ResetEmergencyStop", "Reset emergency stop\n");
			m_oTriggerEmergencyStop = false;
			inspectionCmdProxy.resetEmergencyStop();
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
}

void VI_InspectionControl::TriggerCabinetTemperatureOk(bool p_oTriggerBit)
{
	static bool oOldState = false;

	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.CabinetTempOk", "cabinet temperature is ok\n");
			setCabinetTemperature(true);
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			wmLogTr(eWarning, "QnxMsg.VI.CabinetTempNok", "cabinet temperature is not ok\n");
			setCabinetTemperature(false);
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
}

void VI_InspectionControl::TriggerGenPurposeDigInTakeOver(bool p_oTriggerBit, unsigned int p_oGenPurposeDigInAddress, int p_oGenPurposeDigInValue) {
	static bool oOldState = false;

	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			if ((m_oInspectCycleIsOn)&&(!m_oInspectSeamIsOn)) // Zyklus ist aktiv und Naht Inspektion ist nicht aktiv
			{
				pthread_mutex_lock(&m_oGenPurposeDigInAddressMutex);
				pthread_mutex_lock(&m_oGenPurposeDigIn1Mutex);

				m_oGenPurposeDigInAddress = p_oGenPurposeDigInAddress;
				m_oGenPurposeDigInValue = p_oGenPurposeDigInValue;

				wmLogTr(eInfo, "QnxMsg.VI.DigInTakeOverHigh", "takeover digital inputs: rising edge: address: %d value: %d\n",
						(m_oGenPurposeDigInAddress + 1), m_oGenPurposeDigInValue);
				switch (m_oGenPurposeDigInAddress)
				{
					case 0:
					{
						m_oGenPurposeDigIn1 = m_oGenPurposeDigInValue;
						break;
					}
					case 1:
					{
						m_oGenPurposeDigIn2 = m_oGenPurposeDigInValue;
						break;
					}
					case 2:
					{
						m_oGenPurposeDigIn3 = m_oGenPurposeDigInValue;
						break;
					}
					case 3:
					{
						m_oGenPurposeDigIn4 = m_oGenPurposeDigInValue;
						break;
					}
					case 4:
					{
						m_oGenPurposeDigIn5 = m_oGenPurposeDigInValue;
						break;
					}
					case 5:
					{
						m_oGenPurposeDigIn6 = m_oGenPurposeDigInValue;
						break;
					}
					case 6:
					{
						m_oGenPurposeDigIn7 = m_oGenPurposeDigInValue;
						break;
					}
					case 7:
					{
						m_oGenPurposeDigIn8 = m_oGenPurposeDigInValue;
						break;
					}
					default:
					{
						wmLogTr(eError, "QnxMsg.VI.DigInWrongAddress", "takeover digital inputs: wrong address: %d\n", m_oGenPurposeDigInAddress);
						break;
					}
				}
				pthread_mutex_unlock(&m_oGenPurposeDigIn1Mutex);
				pthread_mutex_unlock(&m_oGenPurposeDigInAddressMutex);
				BoolSender(true, &m_senderGenPurposeDigInAckn); // Signal GenPurposeDigInAckn setzen
			}
			else // kein aktiver Zyklus oder Naht Inspektion ist aktiv
			{
				if (!m_oInspectCycleIsOn)
				{
					wmLogTr(eError, "QnxMsg.VI.DigInNoCycle", "takeover digital inputs set, but there is no cycle active: takeover ignored\n");
				}
				else if (m_oInspectSeamIsOn)
				{
					wmLogTr(eError, "QnxMsg.VI.DigInSeamActive", "takeover digital inputs set, but there is a seam inspection active: takeover ignored\n");
				}
				else
				{
					wmLogTr(eError, "QnxMsg.VI.DigInError", "takeover digital inputs set, but there is a unknown error: takeover ignored\n");
				}
			}
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.DigInTakeOverLow", "takeover digital inputs: falling edge\n");
			BoolSender(false, &m_senderGenPurposeDigInAckn); // Signal GenPurposeDigInAckn zuruecksetzen
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
}

void VI_InspectionControl::TriggerGenPurposeDigOutTakeOver(bool p_oTriggerBit, unsigned int p_oGenPurposeDigInAddress)
{
    static bool oOldState = false;
    static bool oWaitForSendingAcknSignal = false;
    static int oWaitCounter = 0;

    if (oWaitForSendingAcknSignal)
    {
        oWaitCounter++;
        if (oWaitCounter >= 5)
        {
            BoolSender(true, &m_senderGenPurposeDigInAckn); // Signal GenPurposeDigInAckn setzen
            oWaitForSendingAcknSignal = false;
        }
    }

    if (p_oTriggerBit) // Bit ist gesetzt
    {
        if (oOldState) // Bit war bereits gesetzt
        {
        }
        else // Bit war nicht gesetzt -> steigende Flanke
        {
            if ((m_oInspectCycleIsOn)&&(!m_oInspectSeamIsOn)) // Zyklus ist aktiv und Naht Inspektion ist nicht aktiv
            {
                pthread_mutex_lock(&m_oGenPurposeDigInAddressMutex);

                m_oGenPurposeDigInAddress = p_oGenPurposeDigInAddress;

                switch (m_oGenPurposeDigInAddress)
                {
                    case 0:
                    {
                        wmLogTr(eInfo, "QnxMsg.VI.DigOutTakeOverHigh", "takeover digital outputs: rising edge: address: %d value: %d\n", (m_oGenPurposeDigInAddress + 1), m_oGenPurposeDigOut1 ); // ABCDEF
                        FieldSender((uint64_t)m_oGenPurposeDigOut1, &m_senderGenPurposeDigOut1);
                        break;
                    }
                    case 1:
                    {
                        wmLogTr(eInfo, "QnxMsg.VI.DigOutTakeOverHigh", "takeover digital outputs: rising edge: address: %d value: %d\n", (m_oGenPurposeDigInAddress + 1), m_oGenPurposeDigOut2 ); // ABCDEF
                        FieldSender((uint64_t)m_oGenPurposeDigOut2, &m_senderGenPurposeDigOut1);
                        break;
                    }
                    case 2:
                    {
                        wmLogTr(eInfo, "QnxMsg.VI.DigOutTakeOverHigh", "takeover digital outputs: rising edge: address: %d value: %d\n", (m_oGenPurposeDigInAddress + 1), m_oGenPurposeDigOut3 ); // ABCDEF
                        FieldSender((uint64_t)m_oGenPurposeDigOut3, &m_senderGenPurposeDigOut1);
                        break;
                    }
                    case 3:
                    {
                        wmLogTr(eInfo, "QnxMsg.VI.DigOutTakeOverHigh", "takeover digital outputs: rising edge: address: %d value: %d\n", (m_oGenPurposeDigInAddress + 1), m_oGenPurposeDigOut4 ); // ABCDEF
                        FieldSender((uint64_t)m_oGenPurposeDigOut4, &m_senderGenPurposeDigOut1);
                        break;
                    }
                    case 4:
                    {
                        wmLogTr(eInfo, "QnxMsg.VI.DigOutTakeOverHigh", "takeover digital outputs: rising edge: address: %d value: %d\n", (m_oGenPurposeDigInAddress + 1), m_oGenPurposeDigOut5 ); // ABCDEF
                        FieldSender((uint64_t)m_oGenPurposeDigOut5, &m_senderGenPurposeDigOut1);
                        break;
                    }
                    case 5:
                    {
                        wmLogTr(eInfo, "QnxMsg.VI.DigOutTakeOverHigh", "takeover digital outputs: rising edge: address: %d value: %d\n", (m_oGenPurposeDigInAddress + 1), m_oGenPurposeDigOut6 ); // ABCDEF
                        FieldSender((uint64_t)m_oGenPurposeDigOut6, &m_senderGenPurposeDigOut1);
                        break;
                    }
                    case 6:
                    {
                        wmLogTr(eInfo, "QnxMsg.VI.DigOutTakeOverHigh", "takeover digital outputs: rising edge: address: %d value: %d\n", (m_oGenPurposeDigInAddress + 1), m_oGenPurposeDigOut7 ); // ABCDEF
                        FieldSender((uint64_t)m_oGenPurposeDigOut7, &m_senderGenPurposeDigOut1);
                        break;
                    }
                    case 7:
                    {
                        wmLogTr(eInfo, "QnxMsg.VI.DigOutTakeOverHigh", "takeover digital outputs: rising edge: address: %d value: %d\n", (m_oGenPurposeDigInAddress + 1), m_oGenPurposeDigOut8 ); // ABCDEF
                        FieldSender((uint64_t)m_oGenPurposeDigOut8, &m_senderGenPurposeDigOut1);
                        break;
                    }
                    default:
                    {
                        wmLogTr(eError, "QnxMsg.VI.DigOutWrongAddress", "takeover digital outputs: wrong address: %d\n", m_oGenPurposeDigInAddress); // ABCDEF
                        break;
                    }
                }
                pthread_mutex_unlock(&m_oGenPurposeDigInAddressMutex);
                oWaitForSendingAcknSignal = true;
                oWaitCounter = 0;
            }
            else // kein aktiver Zyklus oder Naht Inspektion ist aktiv
            {
                if (!m_oInspectCycleIsOn)
                {
                    wmLogTr(eError, "QnxMsg.VI.DigOutNoCycle", "takeover digital outputs set, but there is no cycle active: takeover ignored\n"); // ABCDEF
                }
                else if (m_oInspectSeamIsOn)
                {
                    wmLogTr(eError, "QnxMsg.VI.DigOutSeamActive", "takeover digital outputs set, but there is a seam inspection active: takeover ignored\n"); // ABCDEF
                }
                else
                {
                    wmLogTr(eError, "QnxMsg.VI.DigOutError", "takeover digital outputs set, but there is a unknown error: takeover ignored\n"); // ABCDEF
                }
            }
        }
    }
    else // Bit ist nicht gesetzt
    {
        if (oOldState) // Bit war gesetzt -> fallende Flanke
        {
            wmLogTr(eInfo, "QnxMsg.VI.DigOutTakeOverLow", "takeover digital outputs: falling edge\n"); // ABCDEF
            BoolSender(false, &m_senderGenPurposeDigInAckn); // Signal GenPurposeDigInAckn zuruecksetzen
        }
        else // Bit war bereits nicht gesetzt
        {
        }
    }
    oOldState = p_oTriggerBit;
}

void VI_InspectionControl::TriggerProductNumberFull(bool p_oTriggerBit, unsigned int p_oProductTypeFull, unsigned int p_oProductNumberFull)
{
	static bool oOldState = false;

	if (p_oTriggerBit) // Bit ist gesetzt
	{
		if (oOldState) // Bit war bereits gesetzt
		{
		}
		else // Bit war nicht gesetzt -> steigende Flanke
		{
			pthread_mutex_lock(&m_oProductTypeFullMutex);
			pthread_mutex_lock(&m_oProductNumberFullMutex);
			m_oProductTypeFull = p_oProductTypeFull;
			m_oProductNumberFull = p_oProductNumberFull;
			wmLogTr(eInfo, "QnxMsg.VI.TOverHighPDataFull", "takeover product data (interface full): rising edge: product-no: %d part-no: %u\n", m_oProductTypeFull, m_oProductNumberFull);
			pthread_mutex_unlock(&m_oProductNumberFullMutex);
			pthread_mutex_unlock(&m_oProductTypeFullMutex);
			BoolSender(true, &m_senderAcknProductNumberFull); // Signal GenPurposeDigInAckn setzen
		}
	}
	else // Bit ist nicht gesetzt
	{
		if (oOldState) // Bit war gesetzt -> fallende Flanke
		{
			wmLogTr(eInfo, "QnxMsg.VI.TOverLowPDataFull", "takeover product data (interface full): falling edge\n");
			BoolSender(false, &m_senderAcknProductNumberFull); // Signal GenPurposeDigInAckn zuruecksetzen
		}
		else // Bit war bereits nicht gesetzt
		{
		}
	}
	oOldState = p_oTriggerBit;
}

void VI_InspectionControl::HandshakeForResultsReady()
{
	static int oStateVariable = 0;
	static int oWaitCounter = 0;

	switch(oStateVariable)
	{
		case 0: // nothing to do
			{
				if (m_oStartHandshakeForResultsReady)
				{
					m_oStartHandshakeForResultsReady = false;
					wmLog(eDebug, "HandshakeForResultsReady goto 1\n");
					oStateVariable = 1;
				}
			}
			break;
		case 1: // output ProductNumber and results
			{
				if (m_oSumErrorLatchedAccu)
				{
					BoolSender(false, &m_senderSetInspectionOKFull);
					BoolSender(true, &m_senderSetSumErrorLatchedFull);
					wmLog(eDebug, "HandshakeFull set to NIO\n");
				}
				else
				{
					BoolSender(true, &m_senderSetInspectionOKFull);
					BoolSender(false, &m_senderSetSumErrorLatchedFull);
					wmLog(eDebug, "HandshakeFull set to IO\n");
				}
				FieldSender((uint64_t)m_oQualityErrorAccuCycle, &m_senderSetQualityErrorFieldFull);
				char oHelpStrg[81];
                unsigned int oDummy = (unsigned int)m_oQualityErrorAccuCycle;
				sprintf(oHelpStrg, "%04X", oDummy);
				wmLog(eDebug, "HandshakeFull set QualityBits: %s\n", oHelpStrg);
				pthread_mutex_lock(&m_oProductTypeFullMutex);
				FieldSender((uint64_t)m_oProductTypeFull, &m_senderSetProductTypeFull);
				pthread_mutex_unlock(&m_oProductTypeFullMutex);
				pthread_mutex_lock(&m_oProductNumberFullMutex);
				FieldSender((uint64_t)m_oProductNumberFull, &m_senderSetProductNumberFull);
				pthread_mutex_unlock(&m_oProductNumberFullMutex);
				oWaitCounter = 0;
				wmLog(eDebug, "HandshakeForResultsReady goto 2\n");
				oStateVariable = 2;
			}
			break;
		case 2: // waiting for some milliseconds
			{
				oWaitCounter++;
				if (oWaitCounter >= 50)
				{
					wmLog(eDebug, "HandshakeForResultsReady goto 3\n");
					oStateVariable = 3;
				}
			}
			break;
		case 3: // set TakeoverResultsReadyFull
			{
				BoolSender(true, &m_senderTriggerResultsReadyFull);
				wmLog(eDebug, "HandshakeFull set TakeoverResultsReadyFull true\n");
				wmLog(eDebug, "HandshakeForResultsReady goto 4\n");
				oStateVariable = 4;
			}
			break;
		case 4: // waiting for AcknowledgeResultsReadyFull high
			{
				pthread_mutex_lock(&m_oAcknResultsReadyFullMutex);
				if (m_oAcknResultsReadyFull == true)
				{
					wmLog(eDebug, "HandshakeForResultsReady goto 5\n");
					oStateVariable = 5;
				}
				pthread_mutex_unlock(&m_oAcknResultsReadyFullMutex);
			}
			break;
		case 5: // reset TakeoverResultsReadyFull
			{
				BoolSender(false, &m_senderTriggerResultsReadyFull);
				wmLog(eDebug, "HandshakeFull set TakeoverResultsReadyFull false\n");
				wmLog(eDebug, "HandshakeForResultsReady goto 6\n");
				oStateVariable = 6;
			}
			break;
		case 6: // waiting for AcknowledgeResultsReadyFull low
			{
				pthread_mutex_lock(&m_oAcknResultsReadyFullMutex);
				if (m_oAcknResultsReadyFull == false)
				{
					wmLog(eDebug, "HandshakeForResultsReady goto 0\n");
					oStateVariable = 0;
				}
				pthread_mutex_unlock(&m_oAcknResultsReadyFullMutex);
			}
			break;
		default:
			{
			}
			break;
	}
}

void VI_InspectionControl::StartProductTeachInMode(){
	//TODO: Ueber HW Ansteuerbar machen
	inspectionCmdProxy.startProductTeachInMode();
}
void VI_InspectionControl::AbortProductTeachInMode(){
	//TODO: Ueber HW Ansteuerbar machen
	inspectionCmdProxy.abortProductTeachInMode();
}

void VI_InspectionControl::setSystemReady (bool onoff){

	wmLog(eDebug, "VI_InspectionControl::setSystemReady: %d\n", onoff);
	BoolSender(onoff, &m_senderSystemReady);
	BoolSender(onoff, &m_senderSetSystemReadyStatusFull);
    if ( isSOUVIS6000_Application() )
    {
        if (onoff)
        {
            BoolSender(false, &m_senderS6K_SystemFault);
            BoolSender(true, &m_senderS6K_SystemReady);
        }
        else
        {
            BoolSender(true, &m_senderS6K_SystemFault);
            BoolSender(false, &m_senderS6K_SystemReady);
        }
        m_rS6K_InfoToProcessesProxy.systemReadyInfo(onoff);
    }
}

void VI_InspectionControl::setSystemErrorField (int systemErrorField){

	if (m_oSystemErrorAccu != (unsigned int)systemErrorField)
	{
		m_oSystemErrorAccu = (unsigned int)systemErrorField;
		FieldSender((uint64_t)systemErrorField, &m_senderSystemErrorField);
		FieldSender((uint64_t)systemErrorField, &m_senderSetSystemErrorFieldFull);
	}
	wmLog(eDebug, "VI_InspectionControl::setSystemErrorField: 0x%x,0x%x\n", systemErrorField, m_oSystemErrorAccu.load());
}

void VI_InspectionControl::setSumErrorLatched (bool onoff)
{
    if (isNIOResultSwitchedOff())
    {
        onoff = false;
    }

    wmLog(eDebug, "VI_InspectionControl::setSumErrorLatched: %d\n", onoff);
    if (onoff)
    {
        m_oSumErrorLatchedAccu = true;

        if (!isContinuouslyModeActive()) // do in normal cyclic mode
        {
            if (!m_oSumErrorSeamSeriesAccu)
            {
                BoolSender(true, &m_senderSumErrorSeamSeries);
                m_oSumErrorSeamSeriesAccu = true;
            }

            if (!m_oSumErrorSeamAccu)
            {
                BoolSender(true, &m_senderSumErrorSeam);
                m_oSumErrorSeamAccu = true;
            }
        }
        else // do in continuously mode
        {
            if (!m_oSumErrorSeamSeriesIsHigh)
            {
                BoolSender(true, &m_senderSumErrorSeamSeries);
            }
            m_oSumErrorSeamSeriesIsHigh = true;
            m_oSumErrorSeamSeriesCounter = 0;
         }
    }
}

void VI_InspectionControl::setQualityErrorField (int qualityErrorField)
{
    if (!isContinuouslyModeActive()) // do in normal cyclic mode
    {
        m_oQualityErrorAccuCycle |= (unsigned int)qualityErrorField;
        wmLog(eDebug, "VI_InspectionControl::setQualityErrorFieldCycle: 0x%x,0x%x\n", qualityErrorField, m_oQualityErrorAccuCycle.load());
        if ((m_oQualityErrorAccuSeamSeries | (unsigned int)qualityErrorField) != m_oQualityErrorAccuSeamSeries)
        {
            m_oQualityErrorAccuSeamSeries |= (unsigned int)qualityErrorField;
            if (isQualityErrorFieldOnSeamSeries()) // output if switch is on
            {
                FieldSender((uint64_t)m_oQualityErrorAccuSeamSeries, &m_senderQualityErrorField);
            }
        }
        if ((m_oQualityErrorAccuSeam | (unsigned int)qualityErrorField) != m_oQualityErrorAccuSeam)
        {
            m_oQualityErrorAccuSeam |= (unsigned int)qualityErrorField;
            if (!isQualityErrorFieldOnSeamSeries()) // output if switch is off
            {
                FieldSender((uint64_t)m_oQualityErrorAccuSeam, &m_senderQualityErrorField);
            }
        }
    }
    else // do in continuously mode
    {
        if ((m_oQualityErrorAccuCycle | (unsigned int)qualityErrorField) != m_oQualityErrorAccuCycle)
        {
            m_oQualityErrorAccuCycle |= (unsigned int)qualityErrorField;
            FieldSender((uint64_t)m_oQualityErrorAccuCycle, &m_senderQualityErrorField);
        }
    }
}

void VI_InspectionControl::setFastStopDoubleBlank(void)
{
    wmLog(eDebug, "VI_InspectionControl::setFastStopDoubleBlank\n");
    if (isSOUVIS6000_Application())
    {
        BoolSender(true, &m_senderS6K_FastStopDoubleBlank);
    }
}

void VI_InspectionControl::setInspectCycleAckn (bool onoff){

	wmLog(eDebug, "VI_InspectionControl::setInspectCycleAckn: %d\n", onoff);
    if (!isContinuouslyModeActive()) // do in normal cyclic mode
    {
        BoolSender(onoff, &m_senderInspectCycleAckn);
    }
	m_oInspectCycleAck = onoff;
	m_oWaitingForCycleAckn = false;
    m_oAutoInspectCycleAckn = true;
}

bool VI_InspectionControl::inspectCycleAckn(){

	return m_oInspectCycleAck;
}

void VI_InspectionControl::setCalibrationFinished (bool result){

	wmLog(eDebug, "VI_InspectionControl::setCalibrationFinished: %d\n", result);
	m_oCalibResultsBuffer |= 0x01;
	if (result)
	{
		// Calibration was successful
		m_oCalibResultsBuffer |= 0x02;
		m_oCalibResultsBuffer &= ~0x04;
	}
	else
	{
		// Calibration was faulty
		m_oCalibResultsBuffer &= ~0x02;
		m_oCalibResultsBuffer |= 0x04;
	}
	FieldSender((uint64_t)m_oCalibResultsBuffer, &m_senderCalibResultsField);
	if (m_oTriggerCalibrationBlocked)
	{
		TriggerCalibration(false, 226);
		m_oTriggerCalibrationBlocked = false;
	}
}

void VI_InspectionControl::setPositionResultsField (int positionResultsField){

	wmLog(eDebug, "VI_InspectionControl::setPositionResultsField: 0x%x\n", positionResultsField);
	m_oPositionResults = positionResultsField;
}

void VI_InspectionControl::setGenPurposeDigOut (OutputID outputNo, int value){

	if (isGenPurposeDigOut1Enabled())
	{
		wmLog(eDebug, "VI_InspectionControl::setGenPurposeDigOut: %d,0x%x\n", outputNo, value);
        if ( outputNo == interface::eOutput1 )
        {
            FieldSender((uint64_t)value, &m_senderGenPurposeDigOut1);
        }
	}
    if (isGenPurposeDigOutMultipleEnabled())
    {
        wmLog(eDebug, "VI_InspectionControl::setGenPurposeDigOut: %d,0x%x\n", outputNo, value);
        switch(outputNo)
        {
            case interface::eOutput1:
                m_oGenPurposeDigOut1 = value;
                break;
            case interface::eOutput2:
                m_oGenPurposeDigOut2 = value;
                break;
            case interface::eOutput3:
                m_oGenPurposeDigOut3 = value;
                break;
            case interface::eOutput4:
                m_oGenPurposeDigOut4 = value;
                break;
            case interface::eOutput5:
                m_oGenPurposeDigOut5 = value;
                break;
            case interface::eOutput6:
                m_oGenPurposeDigOut6 = value;
                break;
            case interface::eOutput7:
                m_oGenPurposeDigOut7 = value;
                break;
            case interface::eOutput8:
                m_oGenPurposeDigOut8 = value;
                break;
            default:
                wmLogTr(eError, "QnxMsg.VI.ABCDEF", "setGenPurposeDigOut: wrong output number: %d\n", outputNo);
                break;
        }
    }
}

void VI_InspectionControl::setS6K_EdgePosition(int32_t p_oEdgePosition, int32_t p_oImageNumber, int64_t p_oPosition)
{
    if (isSOUVIS6000_Is_PreInspection())
    {
        if (m_oDebugInfo_SOURING)
        {
            char oLogString[81];
            sprintf(oLogString, "%5d, %3d, %ld", p_oEdgePosition, p_oImageNumber, p_oPosition);
            wmLog(eDebug, "VI_InspectionControl::setS6K_EdgePosition: %s\n", oLogString);
        }

        m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oEdgePositionValue = p_oEdgePosition;
        m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oEdgePositionImgNo = p_oImageNumber;
        m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oEdgePositionPos = p_oPosition;
        m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oEdgePositionValid = true;
        if ((m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oGapWidthImgNo == m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oEdgePositionImgNo) &&
            (m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oGapWidthValid == true))
        {
            // both necessary results are recorded
            m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oReadyToInsert = true;
            m_oS6K_CollectResultDataWrite = (m_oS6K_CollectResultDataWrite + 1) % S6K_COLLECT_LENGTH;
        }
    }
}

void VI_InspectionControl::setS6K_GapWidth(int32_t p_oGapWidth, int32_t p_oImageNumber, int64_t p_oPosition)
{
    if (isSOUVIS6000_Is_PreInspection())
    {
        if (m_oDebugInfo_SOURING)
        {
            char oLogString[81];
            sprintf(oLogString, "%5d, %3d, %ld", p_oGapWidth, p_oImageNumber, p_oPosition);
            wmLog(eDebug, "VI_InspectionControl::setS6K_GapWidth:     %s\n", oLogString);
        }

        m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oGapWidthValue = p_oGapWidth;
        m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oGapWidthImgNo = p_oImageNumber;
        m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oGapWidthPos = p_oPosition;
        m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oGapWidthValid = true;
        if ((m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oEdgePositionImgNo == m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oGapWidthImgNo) &&
            (m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oEdgePositionValid == true))
        {
            // both necessary results are recorded
            m_oS6K_CollectResultData[m_oS6K_CollectResultDataWrite].m_oReadyToInsert = true;
            m_oS6K_CollectResultDataWrite = (m_oS6K_CollectResultDataWrite + 1) % S6K_COLLECT_LENGTH;
        }
    }
}

void VI_InspectionControl::setS6K_QualityResults(int32_t p_oSeamNo, S6K_QualityData_S1S2 p_oQualityData)
{
    uint32_t oMask = (0x01 << p_oSeamNo);
    if (p_oQualityData.m_oQualityErrorCat1 != 0x00)
    {
        m_oS6K_SeamErrorCat1Accu &= ~oMask;
    }
    if (p_oQualityData.m_oQualityErrorCat2 != 0x00)
    {
        m_oS6K_SeamErrorCat2Accu &= ~oMask;
    }

    if (m_inspectionToS6k)
    {
        m_inspectionToS6k->setS6K_QualityResults(p_oSeamNo, p_oQualityData);
    }
}

void VI_InspectionControl::setS6K_CS_DataBlock (CS_TCP_MODE p_oSendCmd, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                                uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock)
{
    if (m_inspectionToS6k)
    {
        m_inspectionToS6k->setS6K_CS_DataBlock(p_oSendCmd, p_oSeamNo, p_oBlockNo, p_oFirstMeasureInBlock, p_oMeasureCntInBlock, p_oMeasuresPerResult, p_oValuesPerMeasure, p_oCS_DataBlock);
    }
}

void VI_InspectionControl::setS6K_CS_MeasuresPerResult (uint16_t p_oMeasuresPerResult)
{
    if (p_oMeasuresPerResult != m_oS6K_CS_CurrMeasuresPerResult.load() )
    {
        wmLogTr(eError, "QnxMsg.VI.CSDiffMeasInRes", "There are different counts of measurements per result !\n");
    }
}

void VI_InspectionControl::setCabinetTemperature(bool isOk){
	BoolSender(isOk, &m_senderCabinetTemperatureOk);
	BoolSender(isOk, &m_senderSetCabinetTemperatureOkFull);
}

void VI_InspectionControl::setAnalogTriggerLevelVolt(float p_oTriggerLevelVolt)
{ 
    m_oAnalogTriggerLevelVolt = p_oTriggerLevelVolt;
    m_oAnalogTriggerLevelBin = static_cast<int>((m_oAnalogTriggerLevelVolt / 10.0) * 32767.0); // only valid for analog input terminal EL3102
    char oLoggerStrg[81];
    sprintf(oLoggerStrg, "AnalogTriggerLevel: %6.3f, %d\n", m_oAnalogTriggerLevelVolt, m_oAnalogTriggerLevelBin);
    wmLog(eDebug, "%s", oLoggerStrg);

    int oTempInt = static_cast<int>(m_oAnalogTriggerLevelVolt * 1000.0);
    InspectionControlDefaults::instance().setInt("Analog_Trigger_Level", oTempInt);
}

void VI_InspectionControl::SimulateSignalNotReady(int relatedException)
{
	inspectionCmdProxy.signalNotReady(relatedException);
}

void VI_InspectionControl::SimulateQuitSystemFault()
{
	inspectionCmdProxy.quitSystemFault();
}

void VI_InspectionControl::BoolSender(bool value, SenderWrapper* senderWrapper){

	if (senderWrapper == NULL)
	{
		return;
	}

	COMMAND_INFORMATION& info = senderWrapper->info;

	switch (senderWrapper->info.proxyInfo.nSlaveType) {
		case _DIG_8BIT_:{
			uint8_t oSendValue = 0x00;
			uint8_t oSendMask = 1 << info.proxyInfo.nStartBit;
			if(value){
				oSendValue = 1 << info.proxyInfo.nStartBit;
			}
            if (m_oDig8OutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oActive)
            {
                m_rEthercatOutputsProxy.ecatDigitalOut(m_oDig8OutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oProductIndex,
                                                        m_oDig8OutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oInstance,
                                                        (uint8_t) oSendValue, (uint8_t) oSendMask);
            }
			break;
		}
		case _GATEWAY_:{
            if (m_oControlSimulationIsOn)
            {
                return;
            }
			uint8_t oBytePos = (info.proxyInfo.nStartBit / 8);
			uint8_t oSendMask = 1 << (info.proxyInfo.nStartBit % 8);
			uint8_t oSendValue = 0x00;
			if(value)
			{
				oSendValue = 1 << (info.proxyInfo.nStartBit % 8);
			}

	    	stdVecUINT8 oTempVecValue;
	    	oTempVecValue.assign(m_oGatewayDataLength, 0x00);
	    	oTempVecValue[oBytePos] = oSendValue;
	    	stdVecUINT8 oTempVecMask;
	    	oTempVecMask.assign(m_oGatewayDataLength, 0x00);
	    	oTempVecMask[oBytePos] = oSendMask;
            if (m_oGatewayOutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oActive)
            {
                m_rEthercatOutputsProxy.ecatGatewayOut(m_oGatewayOutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oProductIndex,
                                                        m_oGatewayOutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oInstance,
                                                        (uint8_t)oTempVecValue.size(), oTempVecValue, oTempVecMask);
            }
			break;
		}
		default:
			break;
	}
}

void VI_InspectionControl::FieldSender(uint64_t field, SenderWrapper* senderWrapper){

	if (senderWrapper == NULL)
	{
		return;
	}

	COMMAND_INFORMATION& info = senderWrapper->info;

	switch (senderWrapper->info.proxyInfo.nSlaveType) {
		case _DIG_8BIT_:{
			uint8_t oStartBit = info.proxyInfo.nStartBit; // position of the first bit in the fieldbus configuration
			uint8_t oNumBits = info.proxyInfo.nLength; // length of the field in the fieldbus confguration
			uint8_t oMask = 0x01; // mask for the next bit in the field variable

			uint8_t oSendValue = 0x00;
			uint8_t oSendMask = 0x00;

			// do for all configured bits, starting with the first
			for(uint8_t oLoop = 0; oLoop < oNumBits;oLoop++)
			{
				uint8_t oRem = (oLoop + oStartBit) % 8; // this is the bit position of the actual bit in the byte
				uint8_t setMask = 1 << oRem; // this is the mask for the actual bit in the byte
				if (field & oMask) // when bit is set then...
				{
					oSendValue |=  setMask; // ...set the bit in the byte
				}
				oSendMask |=  setMask; // set the bit in the mask anyway
				oMask = oMask << 1; // shift mask for the next bit in the field
			}
            if (m_oDig8OutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oActive)
            {
                m_rEthercatOutputsProxy.ecatDigitalOut(m_oDig8OutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oProductIndex,
                                                        m_oDig8OutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oInstance,
                                                        (uint8_t) oSendValue, (uint8_t) oSendMask);
            }
			break;
		}
		case _GATEWAY_:{
            if (m_oControlSimulationIsOn)
            {
                return;
            }
			uint32_t oStartBit = info.proxyInfo.nStartBit; // position of the first bit in the fieldbus configuration
			uint32_t oNumBits = info.proxyInfo.nLength; // length of the field in the fieldbus confguration
			uint64_t oMask = 0x01; // mask for the next bit in the field variable, same bitwidth as field variable !

	    	stdVecUINT8 oTempVecValue;
	    	oTempVecValue.assign(m_oGatewayDataLength, 0x00);
	    	stdVecUINT8 oTempVecMask;
	    	oTempVecMask.assign(m_oGatewayDataLength, 0x00);

			// do for all configured bits, starting with the first
			for(uint32_t oLoop = 0;oLoop < oNumBits;oLoop++)
			{
				uint32_t oBytePos = (oStartBit + oLoop) / 8; // this is the byte position of the actual bit in the byte stream (m_values)
				uint32_t oRem = (oStartBit + oLoop) % 8; // this is the bit position of the actual bit in the byte
				uint32_t oSetMask = 1 << oRem; // this is the mask for the actual bit in the byte
				if (field & oMask) // when bit is set then...
				{
					oTempVecValue[oBytePos] |= oSetMask;
				}
				oTempVecMask[oBytePos] |= oSetMask;
				oMask = oMask << 1; // shift mask for the next bit in the field
			}
            if (m_oGatewayOutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oActive)
            {
                m_rEthercatOutputsProxy.ecatGatewayOut(m_oGatewayOutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oProductIndex,
                                                        m_oGatewayOutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oInstance,
                                                        (uint8_t)oTempVecValue.size(), oTempVecValue, oTempVecMask);
            }
			break;
		}
		default:
			break;
	}
}

void VI_InspectionControl::StringSender(const std::string& p_oStringVar, SenderWrapper* senderWrapper)
{
    if (senderWrapper == NULL)
    {
        return;
    }

    COMMAND_INFORMATION& info = senderWrapper->info;

    switch (senderWrapper->info.proxyInfo.nSlaveType)
    {
        case _DIG_8BIT_:
        {
            wmLog(eError, "can send a string via digital output terminals\n");
            break;
        }
        case _GATEWAY_:
        {
            if (m_oControlSimulationIsOn)
            {
                return;
            }

            unsigned int oStartByte = info.proxyInfo.nStartBit / 8;
            if ((info.proxyInfo.nStartBit % 8) != 0)
            {
                wmLog(eError, "Start of a string must be on a byte boundary !\n");
            }

            unsigned int oLength = info.proxyInfo.nLength / 8;
            if ((info.proxyInfo.nLength % 8) != 0)
            {
                wmLog(eError, "Length of a string must be a multiple of a byte !\n");
            }

            stdVecUINT8 oTempVecValue;
            oTempVecValue.assign(m_oGatewayDataLength, 0x00);
            stdVecUINT8 oTempVecMask;
            oTempVecMask.assign(m_oGatewayDataLength, 0x00);

            for(uint32_t oLoop = 0;oLoop < oLength;oLoop++)
            {
                oTempVecValue[oStartByte + oLoop] = p_oStringVar[oLoop];
                oTempVecMask[oStartByte + oLoop] = 0xFF;
            }

            if (m_oGatewayOutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oActive)
            {
                m_rEthercatOutputsProxy.ecatGatewayOut(m_oGatewayOutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oProductIndex,
                                                        m_oGatewayOutProxyInfo[(info.proxyInfo.nInstance - 1)].m_oInstance,
                                                        (uint8_t)oTempVecValue.size(), oTempVecValue, oTempVecMask);
            }
            break;
        }
        default:
            break;
    }
}

void VI_InspectionControl::IncomingGenPurposeDigIn1_Single(void)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    if(m_imageNrGenPurposeDigIn1 < int( m_interval.nbTriggers() ) && m_oSensorIdsEnabled[eGenPurposeDigIn1] == true)
    {
        m_pValuesGenPurposeDigIn1[0][0] = (int)m_oGenPurposeDigIn1_Single.load();
        m_context.setImageNumber(m_imageNrGenPurposeDigIn1);
        m_oSensorProxy.data(eGenPurposeDigIn1,m_context,image::Sample(m_pValuesGenPurposeDigIn1[0], 1));
        ++m_imageNrGenPurposeDigIn1;
    }
}

void VI_InspectionControl::IncomingGenPurposeDigIn_Multiple(void)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    if(m_imageNrGenPurposeDigInMultiple < int( m_interval.nbTriggers() ))
    {
        m_context.setImageNumber(m_imageNrGenPurposeDigInMultiple);

        pthread_mutex_lock(&m_oGenPurposeDigIn1Mutex);
        if (m_oSensorIdsEnabled[eGenPurposeDigIn1] == true)
        {
            m_pValuesGenPurposeDigInMultiple[0][0] = (int)m_oGenPurposeDigIn1;
            m_oSensorProxy.data(eGenPurposeDigIn1,m_context,image::Sample(m_pValuesGenPurposeDigInMultiple[0], 1));
        }
        if (m_oSensorIdsEnabled[eGenPurposeDigIn2] == true)
        {
            m_pValuesGenPurposeDigInMultiple[0][0] = (int)m_oGenPurposeDigIn2;
            m_oSensorProxy.data(eGenPurposeDigIn2,m_context,image::Sample(m_pValuesGenPurposeDigInMultiple[0], 1));
        }
        if (m_oSensorIdsEnabled[eGenPurposeDigIn3] == true)
        {
            m_pValuesGenPurposeDigInMultiple[0][0] = (int)m_oGenPurposeDigIn3;
            m_oSensorProxy.data(eGenPurposeDigIn3,m_context,image::Sample(m_pValuesGenPurposeDigInMultiple[0], 1));
        }
        if (m_oSensorIdsEnabled[eGenPurposeDigIn4] == true)
        {
            m_pValuesGenPurposeDigInMultiple[0][0] = (int)m_oGenPurposeDigIn4;
            m_oSensorProxy.data(eGenPurposeDigIn4,m_context,image::Sample(m_pValuesGenPurposeDigInMultiple[0], 1));
        }
        if (m_oSensorIdsEnabled[eGenPurposeDigIn5] == true)
        {
            m_pValuesGenPurposeDigInMultiple[0][0] = (int)m_oGenPurposeDigIn5;
            m_oSensorProxy.data(eGenPurposeDigIn5,m_context,image::Sample(m_pValuesGenPurposeDigInMultiple[0], 1));
        }
        if (m_oSensorIdsEnabled[eGenPurposeDigIn6] == true)
        {
            m_pValuesGenPurposeDigInMultiple[0][0] = (int)m_oGenPurposeDigIn6;
            m_oSensorProxy.data(eGenPurposeDigIn6,m_context,image::Sample(m_pValuesGenPurposeDigInMultiple[0], 1));
        }
        if (m_oSensorIdsEnabled[eGenPurposeDigIn7] == true)
        {
            m_pValuesGenPurposeDigInMultiple[0][0] = (int)m_oGenPurposeDigIn7;
            m_oSensorProxy.data(eGenPurposeDigIn7,m_context,image::Sample(m_pValuesGenPurposeDigInMultiple[0], 1));
        }
        if (m_oSensorIdsEnabled[eGenPurposeDigIn8] == true)
        {
            m_pValuesGenPurposeDigInMultiple[0][0] = (int)m_oGenPurposeDigIn8;
            m_oSensorProxy.data(eGenPurposeDigIn8,m_context,image::Sample(m_pValuesGenPurposeDigInMultiple[0], 1));
        }
        pthread_mutex_unlock(&m_oGenPurposeDigIn1Mutex);

        ++m_imageNrGenPurposeDigInMultiple;
    }
}

void VI_InspectionControl::IncomingS6K_CS_LeadingResults(void)
{
    Poco::FastMutex::ScopedLock lock(m_mutex);
    if( (m_imageNrS6K_CS_LeadingResults < int(m_interval.nbTriggers())) &&
        ((m_oSensorIdsEnabled[eS6K_Leading_Result1] == true) ||
         (m_oSensorIdsEnabled[eS6K_Leading_Result2] == true) ||
         (m_oSensorIdsEnabled[eS6K_Leading_Result3] == true) ||
         (m_oSensorIdsEnabled[eS6K_Leading_Result4] == true)) )
    {
        m_context.setImageNumber(m_imageNrS6K_CS_LeadingResults);

        static int16_t leadingResult1 = 0;
        static int16_t leadingResult2 = 0;
        static int16_t leadingResult3 = 0;
        static int16_t leadingResult4 = 0;
        static int oErrCounterNoResults = 0;
        static int oErrCounterNoPointer = 0;
        if (m_imageNrS6K_CS_LeadingResults == 0)
        {
            leadingResult1 = 0;
            leadingResult2 = 0;
            leadingResult3 = 0;
            leadingResult4 = 0;
            oErrCounterNoResults = 0;
            oErrCounterNoPointer = 0; 
        }

        if(m_pS6K_CS_LeadingResultsOutput != nullptr)
        {
            for(int i = 0;i < m_oS6K_CS_CurrMeasuresPerResult.load();i++)
            {
                if ( !m_pS6K_CS_LeadingResultsOutput->m_oResults.empty() )
                {
                    auto m_oCS_BlockLine = m_pS6K_CS_LeadingResultsOutput->m_oResults.front();
                    leadingResult1 = m_oCS_BlockLine[CS_IDX_RESULT1];
                    leadingResult2 = m_oCS_BlockLine[CS_IDX_RESULT2];
                    leadingResult3 = m_oCS_BlockLine[CS_IDX_RESULT3];
                    leadingResult4 = m_oCS_BlockLine[CS_IDX_RESULT4];
                    m_pS6K_CS_LeadingResultsOutput->m_oResults.pop();
                }
                else
                {
                    if ((i == 0) || (i == (m_oS6K_CS_CurrMeasuresPerResult.load() - 1)))
                    {
                        if (oErrCounterNoResults < 10)
                        {
                            wmLogTr(eError, "QnxMsg.VI.CSNoResultLeft", "There are no results left from the leading system\n");
                            oErrCounterNoResults++;
                        }
                    }
                }
                m_pValuesS6K_CS_LeadingResults1[0][i] = (int)leadingResult1;
                m_pValuesS6K_CS_LeadingResults2[0][i] = (int)leadingResult2;
                m_pValuesS6K_CS_LeadingResults3[0][i] = (int)leadingResult3;
                m_pValuesS6K_CS_LeadingResults4[0][i] = (int)leadingResult4;
            }
        }
        else
        {
            if (oErrCounterNoPointer < 10)
            {
                wmLogTr(eError, "QnxMsg.VI.CSNoResultAtAll", "There are no results from the leading system\n");
                oErrCounterNoPointer++;
            }
            for(int i = 0;i < m_oS6K_CS_CurrMeasuresPerResult.load();i++)
            {
                m_pValuesS6K_CS_LeadingResults1[0][i] = (int)0;
                m_pValuesS6K_CS_LeadingResults2[0][i] = (int)0;
                m_pValuesS6K_CS_LeadingResults3[0][i] = (int)0;
                m_pValuesS6K_CS_LeadingResults4[0][i] = (int)0;
            }
        }

        if (m_oS6K_MakePicturesStatus.load() == true)
        {
            if (m_oSensorIdsEnabled[eS6K_Leading_Result1] == true)
            {
                m_oSensorProxy.data(eS6K_Leading_Result1,m_context,image::Sample(m_pValuesS6K_CS_LeadingResults1[0], m_oS6K_CS_CurrMeasuresPerResult.load()));
                //wmLog(eDebug, "ConCav: %d,%d ... %d,%d\n", 
                //      m_pValuesS6K_CS_LeadingResults1[0][0],
                //      m_pValuesS6K_CS_LeadingResults1[0][1],
                //      m_pValuesS6K_CS_LeadingResults1[0][m_oS6K_CS_CurrMeasuresPerResult.load() - 2],
                //      m_pValuesS6K_CS_LeadingResults1[0][m_oS6K_CS_CurrMeasuresPerResult.load() - 1]);
            }
            if (m_oSensorIdsEnabled[eS6K_Leading_Result2] == true)
            {
                m_oSensorProxy.data(eS6K_Leading_Result2,m_context,image::Sample(m_pValuesS6K_CS_LeadingResults2[0], m_oS6K_CS_CurrMeasuresPerResult.load()));
                //wmLog(eDebug, "HiDiff: %d,%d ... %d,%d\n", 
                //      m_pValuesS6K_CS_LeadingResults2[0][0],
                //      m_pValuesS6K_CS_LeadingResults2[0][1],
                //      m_pValuesS6K_CS_LeadingResults2[0][m_oS6K_CS_CurrMeasuresPerResult.load() - 2],
                //      m_pValuesS6K_CS_LeadingResults2[0][m_oS6K_CS_CurrMeasuresPerResult.load() - 1]);
            }
            if (m_oSensorIdsEnabled[eS6K_Leading_Result3] == true)
            {
                m_oSensorProxy.data(eS6K_Leading_Result3,m_context,image::Sample(m_pValuesS6K_CS_LeadingResults3[0], m_oS6K_CS_CurrMeasuresPerResult.load()));
                //wmLog(eDebug, "ConPos: %d,%d ... %d,%d\n", 
                //      m_pValuesS6K_CS_LeadingResults3[0][0],
                //      m_pValuesS6K_CS_LeadingResults3[0][1],
                //      m_pValuesS6K_CS_LeadingResults3[0][m_oS6K_CS_CurrMeasuresPerResult.load() - 2],
                //      m_pValuesS6K_CS_LeadingResults3[0][m_oS6K_CS_CurrMeasuresPerResult.load() - 1]);
            }
            if (m_oSensorIdsEnabled[eS6K_Leading_Result4] == true)
            {
                m_oSensorProxy.data(eS6K_Leading_Result4,m_context,image::Sample(m_pValuesS6K_CS_LeadingResults4[0], m_oS6K_CS_CurrMeasuresPerResult.load()));
            }
        }

        ++m_imageNrS6K_CS_LeadingResults;
    }
}

void VI_InspectionControl::passS6K_CS_DataBlock_To_Inspect (uint32_t p_oProductNo, uint32_t p_oBatchID, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                                            uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock)
{
    static uint32_t oOldBatchID = 0;
    static uint32_t oOldSeamNo = 0;

    if ((p_oBatchID != oOldBatchID) || // new batch ID, means it is a new inspection cycle
        (p_oSeamNo != oOldSeamNo)) // new seam number is also a reason to change to next buffer
    {
        m_pS6K_CS_LeadingResultsInput->m_oValid = true; // old buffer is now valid
        // from now on use next buffer
        m_oS6K_CS_CurrentLeadingResultsInput = (m_oS6K_CS_CurrentLeadingResultsInput + 1) % S6K_CS_LEADINGRESULTS_BUFFER_COUNT;
        m_pS6K_CS_LeadingResultsInput = &m_oS6K_CS_LeadingResultsBufferArray[m_oS6K_CS_CurrentLeadingResultsInput];
        if(m_pS6K_CS_LeadingResultsInput != nullptr)
        {
            m_pS6K_CS_LeadingResultsInput->m_oValid = false; // new buffer is now invalid
            m_pS6K_CS_LeadingResultsInput->m_oProductNo = p_oProductNo;
            m_pS6K_CS_LeadingResultsInput->m_oBatchID = p_oBatchID;
            m_pS6K_CS_LeadingResultsInput->m_oSeamNo = p_oSeamNo;
            m_pS6K_CS_LeadingResultsInput->m_oMeasuresPerResult = p_oMeasuresPerResult;
            while( !m_pS6K_CS_LeadingResultsInput->m_oResults.empty() )
            {
                m_pS6K_CS_LeadingResultsInput->m_oResults.pop();
            }
        }
        else
        {
            wmLogTr(eError, "QnxMsg.VI.CSNoValidPtr", "no valid pointer to queue object\n");
        }
        oOldBatchID = p_oBatchID;
        oOldSeamNo = p_oSeamNo;
    }

    if(m_pS6K_CS_LeadingResultsInput != nullptr)
    {
        for(int i = 0;i < p_oMeasureCntInBlock;i++)
        {
            m_pS6K_CS_LeadingResultsInput->m_oResults.push(p_oCS_DataBlock[i]);
        }
        wmLog(eDebug, "passS6K_CS_DataBlock_To_Inspect: p_oProductNo: %d, p_oBatchID: %d, p_oSeamNo: %d, buffer: %d\n",
              p_oProductNo, p_oBatchID, p_oSeamNo, m_oS6K_CS_CurrentLeadingResultsInput);
        wmLog(eDebug, "m_oCS_LeadingResultsInput.size(): %d\n", m_pS6K_CS_LeadingResultsInput->m_oResults.size() );
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.CSNoValidPtr", "no valid pointer to queue object\n");
    }
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
void VI_InspectionControl::burst(const std::vector<int>& ids, TriggerContext const& context, TriggerInterval const& interval)
{
wmLog(eDebug, "burst: nbTrig:%d, Delta[um]%d, Dist[ns]:%d, State:%d\n", interval.nbTriggers(), interval.triggerDelta(), interval.triggerDistance(), interval.state());

    // following is necessary for continuous application, regardless of the requested sensor ids
    m_oInspectTimeMsContinuously = (interval.triggerDistance() / 1000000) * interval.nbTriggers();

    // following is necessary for SOUVIS6000 application, regardless of the requested sensor ids
    if ( isSOUVIS6000_Application() &&
         isSOUVIS6000_Is_PreInspection() )
    {
        if (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouRing)
        {
            // calculate m_oS6K_ImageCountUsed as length of the result ringbuffer
            uint32_t oTriggerDeltaUM = interval.triggerDelta();
            uint32_t oTriggerDeltaMM = oTriggerDeltaUM / 1000;
            if ((oTriggerDeltaUM % 1000) != 0)
            {
                wmLogTr(eError, "QnxMsg.VI.TrigNotMM", "Trigger Distance is not on a millimeter boundary !\n");
            }
            wmLog(eDebug, "m_oS6K_MaxSouvisSpeed: %d\n", m_oS6K_MaxSouvisSpeed);
            float oTempFloat1 = (static_cast<float>(m_oS6K_MaxSouvisSpeed) / 10) / 60;
            wmLog(eDebug, "oTriggerDeltaMM: %d\n", oTriggerDeltaMM);
            float oTempFloat2 = static_cast<float>(oTriggerDeltaMM) / oTempFloat1;
            wmLog(eDebug, "S6K_DURATION_OF_RESULTS_DIALOG: %f\n", S6K_DURATION_OF_RESULTS_DIALOG);
            float oTempFloat3 = S6K_DURATION_OF_RESULTS_DIALOG / oTempFloat2;
            float oTempFloat4 = roundf(oTempFloat3);
            uint32_t oTempInt1 = static_cast<uint32_t>(oTempFloat4);
            oTempInt1++;
            // at least results of 5 images in one result dialog
            if (oTempInt1 < 5) oTempInt1 = 5;
            if (oTempInt1 >  18)
            {
                wmLogTr(eError, "QnxMsg.VI.S6KResDiaTooLong1", "S6K: too many images results for the result dialog !\n");
                wmLogTr(eError, "QnxMsg.VI.S6KResDiaTooLong2", "S6K: only results of maximum 18 images are possible !\n");
                oTempInt1 = 18;
            }
            m_oS6K_ImageCountUsed = oTempInt1;
        }
        else if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
        {
            m_oS6K_ImageCountUsed = 3;
        }
        wmLog(eDebug, "m_oS6K_ImageCountUsed: %d\n", m_oS6K_ImageCountUsed.load());
    }

    // following is necessary for ScanmMster application, regardless of the requested sensor ids
    if ( isSCANMASTER_Application() )
    {
        m_oSM_BurstWasReceived.store(true);
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
            // nothing to do, prevents from inserting unknwon sensor id
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

    StartSendSensorDataThread(interval.triggerDistance());

	Poco::FastMutex::ScopedLock lock(m_mutex);
	m_context = context;
	m_interval = interval;

	m_imageNrGenPurposeDigIn1 = 0;
	m_imageNrGenPurposeDigInMultiple = 0;
	m_imageNrS6K_CS_LeadingResults = 0;

	m_triggerDistanceNanoSecs = m_interval.triggerDistance(); // m_interval has triggerDist info in nanoseconds
    
	cleanBuffer();

	int ecatSamplesPerTrigger = 1;

	m_pValuesGenPurposeDigIn1 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i)
	{
		m_pValuesGenPurposeDigIn1[i] = new int[ecatSamplesPerTrigger];
		for(int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGenPurposeDigIn1[i][j] = 0;
	}
	m_pValuesGenPurposeDigInMultiple = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i)
	{
		m_pValuesGenPurposeDigInMultiple[i] = new int[ecatSamplesPerTrigger];
		for(int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesGenPurposeDigInMultiple[i][j] = 0;
	}

	m_pValuesS6K_CS_LeadingResults1 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i)
	{
		m_pValuesS6K_CS_LeadingResults1[i] = new int[m_oS6K_CS_CurrMeasuresPerResult.load()];
		for(unsigned int j = 0; j < m_oS6K_CS_CurrMeasuresPerResult.load();j++) m_pValuesS6K_CS_LeadingResults1[i][j] = 0;
	}
	m_pValuesS6K_CS_LeadingResults2 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i)
	{
		m_pValuesS6K_CS_LeadingResults2[i] = new int[m_oS6K_CS_CurrMeasuresPerResult.load()];
		for(unsigned int j = 0; j < m_oS6K_CS_CurrMeasuresPerResult.load();j++) m_pValuesS6K_CS_LeadingResults2[i][j] = 0;
	}
	m_pValuesS6K_CS_LeadingResults3 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i)
	{
		m_pValuesS6K_CS_LeadingResults3[i] = new int[m_oS6K_CS_CurrMeasuresPerResult.load()];
		for(unsigned int j = 0; j < m_oS6K_CS_CurrMeasuresPerResult.load();j++) m_pValuesS6K_CS_LeadingResults3[i][j] = 0;
	}
	m_pValuesS6K_CS_LeadingResults4 = new TSmartArrayPtr<int>::ShArrayPtr[1];
	for (unsigned int i = 0; i < 1; ++i)
	{
		m_pValuesS6K_CS_LeadingResults4[i] = new int[m_oS6K_CS_CurrMeasuresPerResult.load()];
		for(unsigned int j = 0; j < m_oS6K_CS_CurrMeasuresPerResult.load();j++) m_pValuesS6K_CS_LeadingResults4[i][j] = 0;
	}
} //release lock

void VI_InspectionControl::cancel(int id)
{
    StopSendSensorDataThread();

	Poco::FastMutex::ScopedLock lock(m_mutex);
	m_context = TriggerContext(0,0,0);
	m_interval = TriggerInterval(0,0);

	m_imageNrGenPurposeDigIn1 = 0;

	m_imageNrGenPurposeDigInMultiple = 0;

	m_imageNrS6K_CS_LeadingResults = 0;

	cleanBuffer();

	// reset enabled states for all extern sensors
	for (auto& rSensorIdEnabled : m_oSensorIdsEnabled)
	{
		rSensorIdEnabled.second	=	false;
	} // for
}

//clean buffer
void VI_InspectionControl::cleanBuffer()
{
	if(m_pValuesGenPurposeDigIn1 != NULL)
	{
		for (unsigned int i = 0; i < 1; ++i)
		{
			m_pValuesGenPurposeDigIn1[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGenPurposeDigIn1;
		m_pValuesGenPurposeDigIn1 = NULL;
	}

	if(m_pValuesGenPurposeDigInMultiple != NULL)
	{
		for (unsigned int i = 0; i < 1; ++i)
		{
			m_pValuesGenPurposeDigInMultiple[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesGenPurposeDigInMultiple;
		m_pValuesGenPurposeDigInMultiple = NULL;
	}

	if(m_pValuesS6K_CS_LeadingResults1 != NULL)
	{
		for (unsigned int i = 0; i < 1; ++i)
		{
			m_pValuesS6K_CS_LeadingResults1[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesS6K_CS_LeadingResults1;
		m_pValuesS6K_CS_LeadingResults1 = NULL;
	}
	if(m_pValuesS6K_CS_LeadingResults2 != NULL)
	{
		for (unsigned int i = 0; i < 1; ++i)
		{
			m_pValuesS6K_CS_LeadingResults2[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesS6K_CS_LeadingResults2;
		m_pValuesS6K_CS_LeadingResults2 = NULL;
	}
	if(m_pValuesS6K_CS_LeadingResults3 != NULL)
	{
		for (unsigned int i = 0; i < 1; ++i)
		{
			m_pValuesS6K_CS_LeadingResults3[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesS6K_CS_LeadingResults3;
		m_pValuesS6K_CS_LeadingResults3 = NULL;
	}
	if(m_pValuesS6K_CS_LeadingResults4 != NULL)
	{
		for (unsigned int i = 0; i < 1; ++i)
		{
			m_pValuesS6K_CS_LeadingResults4[i] = 0; // decrement smart pointer reference counter
		}
		delete [] m_pValuesS6K_CS_LeadingResults4;
		m_pValuesS6K_CS_LeadingResults4 = NULL;
	}
}

void VI_InspectionControl::ResultsReceivedNumber(unsigned long p_oResultCount)
{
	printf("VI_InspectionControl::ResultsReceivedNumber: %lu\n", p_oResultCount);
}

void VI_InspectionControl::ResultsReceivedFlag(bool p_oOK)
{
	m_oSeamHasNoResults = false;
}

void VI_InspectionControl::NIOReceived(int p_oSeamSeries, int p_oSeamNo, int p_oSeamInterval)
{
    if ( isSCANMASTER_Application() )
    {
        if ( isSCANMASTER_ThreeStepInterface() )
        {
            m_oSM_NIOBuffer[p_oSeamNo % m_oSM_SeamCount.load()] = true;
        }
        else if (isSCANMASTER_GeneralApplication() )
        {
            m_oSM_NIOBuffer[p_oSeamNo] = true;
        }
    }
}

void VI_InspectionControl::setTriggerDistNanoSecs(long lTriggerDistNanoSecs) // used in AutoRunner
{
    m_triggerDistanceNanoSecs = lTriggerDistNanoSecs;
}

//*****************************************************************************
//*****************************************************************************

uint32_t VI_InspectionControl::DetermineSeamCount(uint32_t p_oProductType, uint32_t p_oSeamseries)
{
    Poco::UUID oStationUUID = Poco::UUID( ConnectionConfiguration::instance().getString("Station.UUID", Poco::UUID::null().toString() ) );
    
#if DEBUG_SCANMASTER
    auto oStartTime = std::chrono::high_resolution_clock::now();
    wmLog(eDebug, "DetermineSeamCount: m_oStationUUID:   %s  ; Product type: %d \n", oStationUUID.toString().c_str(), p_oProductType);
#endif
    
    uint32_t oSeamCount = 0;
   
    auto oProductList = m_dbNotificationServer->getProducts();
    
    auto productIter = std::find_if(oProductList->begin(), oProductList->end(),  [p_oProductType](const auto& product) 
    {
           return product.productType() == p_oProductType;
    });          
  

    if (productIter != oProductList->end()) // product found in product List
    {
        auto oMeasureTaskMap = m_dbNotificationServer->getMeasureTasksPerProductMap();
        wmLog(eDebug, "DetermineSeamCount: m_oSM_MeasureTasks.size() <%d>\n", oMeasureTaskMap->size());
        
        auto it = oMeasureTaskMap->find(productIter->productID());
        if(it != oMeasureTaskMap->end()) // product found in MeasureTasks map
        {
            
          //  const auto& measureTasks = (*oMeasureTaskMap)[productIter->productID()];
            for (const auto& task : it->second)
            {
#if DEBUG_SCANMASTER
                char oHelpStrg1[121];
                sprintf(oHelpStrg1, "lev: %1d, serie: %1d , seam: %2d , ",
                task.level(), task.seamseries(), task.seam());
                char oHelpStrg2[121];
                sprintf(oHelpStrg2, "inter: %2d , intLev: %1d , ",
                task.seaminterval(), task.intervalLevel());
                char oHelpStrg3[121];
                sprintf(oHelpStrg3, "len: %5d , name: %s , ",
                (task.length() / 1000), task.name().c_str());
                char oHelpStrg4[121];
                sprintf(oHelpStrg4, "graphName: %s", task.graphName().c_str());
                wmLog(eDebug, "MeasTask: %s%s%s%s\n", oHelpStrg1, oHelpStrg2, oHelpStrg3, oHelpStrg4);
                sprintf(oHelpStrg4, "trgDel: %2d, vel: %3d , noTrg: %2d",
                task.triggerDelta(), task.velocity(), task.nrSeamTriggers());
                wmLog(eDebug, "MeasTask: %s%s%s\n", oHelpStrg1, oHelpStrg2, oHelpStrg4);
#endif

                if (static_cast<uint32_t>(task.seamseries()) == p_oSeamseries && task.level() == 1)
                {
                    ++oSeamCount;
                }
            }
        }// end if - we found the product in the measuretask map
    }

#if DEBUG_SCANMASTER
    auto oEndTime = std::chrono::high_resolution_clock::now();
    auto oduration = std::chrono::duration_cast<std::chrono::microseconds>(oEndTime - oStartTime);
    wmLog(eDebug, "DetermineSeamCount ends- execution time: %d ms \n", oduration.count()  );
#endif
    
    return oSeamCount;
}

void VI_InspectionControl::DetermineLWMParameter(uint32_t p_oProductType)
{
    for (unsigned int i = 0; i < m_LWMParameterVector.size(); i++)
    {
        m_LWMParameterVector[i].clear();
    }
    m_LWMParameterVector.clear();

    Poco::UUID oStationUUID = Poco::UUID(ConnectionConfiguration::instance().getString("Station.UUID", Poco::UUID::null().toString()));
#if DEBUG_SCANMASTER
    wmLog(eDebug, "DetermineLWMParameter: m_oStationUUID:   %s\n", oStationUUID.toString().c_str());
#endif

    Poco::UUID oProductID;
    
    auto oProductList = m_dbNotificationServer->getProducts();
    
    auto oIterProd = std::find_if(oProductList->begin(), oProductList->end(),  [p_oProductType](const auto& product) 
    {
           return product.productType() == p_oProductType;
   });  

  
//    for (ProductList::iterator oIterProd = m_productList.begin(); oIterProd!=m_productList.end(); ++oIterProd)
//    {
   if (oIterProd != oProductList->end()) // product found in product list
   {
#if DEBUG_SCANMASTER
        wmLog(eDebug, "DetermineLWMParameter: productID:   %s\n", oIterProd->productID().toString().c_str());
#endif
        wmLog(eDebug, "DetermineLWMParameter: productType: %d, name: %s\n", oIterProd->productType(), oIterProd->name().c_str());
        oProductID = oIterProd->productID();        
            
            
        auto oMeasureTaskMap = m_dbNotificationServer->getMeasureTasksPerProductMap();
        wmLog(eDebug, "DetermineLWMParameter: getMeasureTasks returns m_oSM_MeasureTasks.size() <%d>\n", m_oSM_MeasureTasks.size());
        
        auto oMeasureTasks = oMeasureTaskMap->find(oIterProd->productID());
        if(oMeasureTasks != oMeasureTaskMap->end()) // product found in MeasureTasks map
        {
            
            int numberSeamSeries{0};
                
            for (const auto& task : oMeasureTasks->second)
            {
#if DEBUG_SCANMASTER
                char oHelpStrg1[121];
                sprintf(oHelpStrg1, "lev: %1d, serie: %1d , seam: %2d , ",
                        oIterMeasTask->level(), oIterMeasTask->seamseries(), oIterMeasTask->seam());
                char oHelpStrg2[121];
                sprintf(oHelpStrg2, "inter: %2d , intLev: %1d , ",
                        oIterMeasTask->seaminterval(), oIterMeasTask->intervalLevel());
                char oHelpStrg3[121];
                sprintf(oHelpStrg3, "len: %5d , name: %s , ",
                        (oIterMeasTask->length() / 1000), oIterMeasTask->name().c_str());
                char oHelpStrg4[121];
                sprintf(oHelpStrg4, "graphName: %s", oIterMeasTask->graphName().c_str());
                wmLog(eDebug, "MeasTask: %s%s%s%s\n", oHelpStrg1, oHelpStrg2, oHelpStrg3, oHelpStrg4);
                sprintf(oHelpStrg4, "trgDel: %2d, vel: %3d , noTrg: %2d",
                        oIterMeasTask->triggerDelta(), oIterMeasTask->velocity(), oIterMeasTask->nrSeamTriggers());
                wmLog(eDebug, "MeasTask: %s%s%s\n", oHelpStrg1, oHelpStrg2, oHelpStrg4);
#endif
                
                if (task.level() == 0) // level: seam series
                {
                    numberSeamSeries++;
                }
            }

            
           

            for (int i = 0; i < numberSeamSeries; i++)
            {
                int numberSeams = std::count_if(oMeasureTasks->second.begin(), oMeasureTasks->second.end(), [i](const auto& task) 
                {
                    return task.seamseries() == i && task.level() == 1;
                });

                m_LWMParameterVector.emplace_back(std::vector<LWMParameterStruct>(numberSeams));
            }

            for (const auto& oIterMeasTask : oMeasureTasks->second)
            {
                if (oIterMeasTask.level() == 1) // level: seam
                {
                    Poco::UUID hwParametersUUID = oIterMeasTask.hwParametersatzID();
                    if (!hwParametersUUID.isNull())
                    {
                        const auto& oParamsIter = m_dbNotificationServer->getParameterSet()->find(hwParametersUUID);               
    
                        if (oParamsIter != m_dbNotificationServer->getParameterSet()->end())
                        {
#if DEBUG_SCANMASTER
                            wmLog(eDebug, "serie: %d , seam: %d\n", oIterMeasTask->seamseries(), oIterMeasTask->seam());
#endif
                            struct LWMParameterStruct LWMParameter{false, 0};
                            for (auto parameter : oParamsIter->second)
                            {
                                if (parameter->type() == TBool)
                                {
                                    if (parameter->name() == "LWM_Inspection_Active")
                                    {
                                        LWMParameter.m_LWMInspectionActive = parameter->value<bool>();
#if DEBUG_SCANMASTER
                                        wmLog(eDebug, "name: %s, type: %s value: %d\n", parameter->name(), parameter->typeToStr().c_str(), parameter->value<bool>());
#endif
                                    }
                                }
                                else if (parameter->type() == TInt)
                                {
                                    if (parameter->name() == "LWM_Program_Number")
                                    {
                                        LWMParameter.m_LWMProgramNumber = parameter->value<int>();
#if DEBUG_SCANMASTER
                                        wmLog(eDebug, "name: %s, type: %s value: %d\n", parameter->name(), parameter->typeToStr().c_str(), parameter->value<int>());
#endif
                                    }
                                }
                            } // end parameters 'for loop' 
                            m_LWMParameterVector[oIterMeasTask.seamseries()][oIterMeasTask.seam()] = LWMParameter;
                        } // end 'if' parameters found in parameters map
                    } // end 'if' parameters exist
                } // end 'if' level seam
            } // end measuretasks 'for loop'
        } // end 'if' product found in MeasureTasks map
    }// end 'if' product found in product list
#if DEBUG_SCANMASTER
    wmLog(eDebug, "********************\n");
    for (unsigned int i = 0; i < m_LWMParameterVector.size(); i++)
    {
        for (unsigned int j = 0; j < m_LWMParameterVector[i].size(); j++)
        {
            wmLog(eDebug, "series: %d, seam: %d\n", i, j);
            wmLog(eDebug, "active: %d, number: %d\n", m_LWMParameterVector[i][j].m_LWMInspectionActive, m_LWMParameterVector[i][j].m_LWMProgramNumber);
        }
    }
    wmLog(eDebug, "********************\n");
#endif
}

//*****************************************************************************
//*****************************************************************************

// thread for auto homing ZCollimator
static void *ZCollAutoHomingThread(void* p_pArg)
{
    prctl(PR_SET_NAME, "ZCollAutoHomingThread");
	struct DataToZCollAutoHomingThread* pDataToZCollAutoHomingThread;
	VI_InspectionControl* pVI_InspectionControl;

	pDataToZCollAutoHomingThread = static_cast<struct DataToZCollAutoHomingThread *>(p_pArg);
	pVI_InspectionControl = pDataToZCollAutoHomingThread->m_pVI_InspectionControl;

	wmLog(eDebug, "ZCollAutoHomingThread started\n");
	sleep(30); // wait 30 seconds prior to start working
	wmLog(eDebug, "ZCollAutoHomingThread active\n");
	pVI_InspectionControl->TriggerCalibration(true, 226);
	sleep(1);
	wmLog(eDebug, "ZCollAutoHomingThread ended\n");

	return NULL;
}

/***************************************************************************/
/* StartCyclicTaskThread                                                   */
/***************************************************************************/

void VI_InspectionControl::StartCyclicTaskThread(void)
{
    ///////////////////////////////////////////////////////
    // Thread für zyklischen Ablauf starten
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToCyclicTaskThread.m_pVI_InspectionControl = this;

    if (pthread_create(&m_oCyclicTaskThread_ID, &oPthreadAttr, &CyclicTaskThread, &m_oDataToCyclicTaskThread) != 0)
    {
        char oStrErrorBuffer[256];

        wmLog(eDebug, "pthread_create failed (%s)(%s)\n", "001", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMCreateThreadFail", "Cannot start thread (%s)\n", "001");
    }
}

void VI_InspectionControl::sendPositionValueToSimotion(int32_t p_oMsgData)
{
    const int BUFFER_ARRAY_LENGTH = 5;
    const int VALUE_ARRAY_LENGTH = 20;

    struct ValueItem
    {
        int m_oPosition;
        int m_oLaserCorr;
        int m_oImgMode;
        int m_oGapWidth;
    };

    struct BufferItem
    {
        bool m_oInputBlocked;
        bool m_oOutputBlocked;
        int m_oNextValueItem;
        bool m_oSendIsRequested;
        int m_oSendTrialsCnt;
        struct ValueItem m_oValueArray[VALUE_ARRAY_LENGTH];
    };

    static struct BufferItem oBufferArray[BUFFER_ARRAY_LENGTH];
    static int oInputIndex = 0;
    static int oOutputIndex = 0;

    char statusStrg[121];

    // pruefen, ob die internen Variablen zurueckgesetzt werden sollen (bei Zyklusstart)
    if (p_oMsgData == eS6K_Command_ResetVars)
    {
        oInputIndex = 0;
        oOutputIndex = 0;
        for(int i = 0;i < BUFFER_ARRAY_LENGTH;i++)
        {
            oBufferArray[i].m_oInputBlocked = false;
            oBufferArray[i].m_oOutputBlocked = false;
            oBufferArray[i].m_oNextValueItem = 0;
            oBufferArray[i].m_oSendIsRequested = false;
            oBufferArray[i].m_oSendTrialsCnt = 0;
            for(int j = 0;j < VALUE_ARRAY_LENGTH;j++)
            {
                oBufferArray[i].m_oValueArray[j].m_oPosition = 0;
                oBufferArray[i].m_oValueArray[j].m_oLaserCorr = 0;
                oBufferArray[i].m_oValueArray[j].m_oImgMode = 0;
                oBufferArray[i].m_oValueArray[j].m_oGapWidth = 0;
            }
        }
        if (m_oDebugInfo_SOURING_Extd)
        {
            wmLog(eDebug, "sendPositionValueToSimotion: Alles zurueckgesetzt\n");
        }
        return; // danach ganz schnell wieder zurueck
    }

    // pruefen, ob etwas zu tun ist
    if (p_oMsgData == eS6K_Command_NoData) // Aufruf ohne neue Daten von souvResults
    {
        bool oSendIsRequested = false;
        for(int i = 0;i < BUFFER_ARRAY_LENGTH;i++)
        {
            if (oBufferArray[i].m_oSendIsRequested) oSendIsRequested = true;
        }
        if (!oSendIsRequested) // es ist kein Sendevorgang pendent
        {
            return; // wenn nichts zu tun, dann ganz schnell wieder zurueck
        }
    }
    else // Aufruf mit neuen Daten von souvResults
    {
        // hier noch ueberpruefen, ob oValueArray ueberhaupt frei ist !
        if (oBufferArray[oInputIndex].m_oNextValueItem >= m_oS6K_ImageCountUsed)
        {
            wmLogTr(eError, "QnxMsg.VI.S6KRingBFault1", "S6K: unable to write the new results in the result buffer ! %s\n", "(001)");
        }

        // oValueArray ist leer, es beginnt ein neues Sammeln von Ergebnissen
        if (oBufferArray[oInputIndex].m_oNextValueItem == 0)
        {
            // hier noch ueberpruefen, ob oValueArray ueberhaupt frei ist !
            if (oBufferArray[oInputIndex].m_oInputBlocked)
            {
                wmLogTr(eError, "QnxMsg.VI.S6KRingBFault1", "S6K: unable to write the new results in the result buffer ! %s\n", "(002)");
            }
            if (oBufferArray[oInputIndex].m_oOutputBlocked)
            {
                wmLogTr(eError, "QnxMsg.VI.S6KRingBFault1", "S6K: unable to write the new results in the result buffer ! %s\n", "(003)");
            }

            oBufferArray[oInputIndex].m_oInputBlocked = true;
            if (m_oDebugInfo_SOURING_Extd)
            {
                sprintf(statusStrg, "%d", oInputIndex);
                wmLog(eDebug, "sendPositionValueToSimotion: neuer Input Zyklus, Buffer: %s\n", statusStrg);
            }
        }

        if (p_oMsgData != eS6K_CommandLastSend)
        {
            oBufferArray[oInputIndex].m_oValueArray[oBufferArray[oInputIndex].m_oNextValueItem].m_oPosition  = (m_oS6K_CollectResultData[(p_oMsgData & 0xFF)].m_oEdgePositionPos / 1000); // Ist-Position (bildNr * g_oImgDistance)
            oBufferArray[oInputIndex].m_oValueArray[oBufferArray[oInputIndex].m_oNextValueItem].m_oLaserCorr = m_oS6K_CollectResultData[(p_oMsgData & 0xFF)].m_oEdgePositionValue; // Laserkorrektur (pos2cnc)
            oBufferArray[oInputIndex].m_oValueArray[oBufferArray[oInputIndex].m_oNextValueItem].m_oImgMode   = 0; // Bildmodus (imgMode)
            oBufferArray[oInputIndex].m_oValueArray[oBufferArray[oInputIndex].m_oNextValueItem].m_oGapWidth  = m_oS6K_CollectResultData[(p_oMsgData & 0xFF)].m_oGapWidthValue; // Spaltmass (wire2cnc)
            m_oS6K_CollectResultData[(p_oMsgData & 0xFF)].m_oEdgePositionImgNo = -1;
            m_oS6K_CollectResultData[(p_oMsgData & 0xFF)].m_oEdgePositionValid = false;
            m_oS6K_CollectResultData[(p_oMsgData & 0xFF)].m_oGapWidthImgNo = -1;
            m_oS6K_CollectResultData[(p_oMsgData & 0xFF)].m_oGapWidthValid = false;
            m_oS6K_CollectResultData[(p_oMsgData & 0xFF)].m_oReadyToInsert = false;
            oBufferArray[oInputIndex].m_oNextValueItem++;
        }
        else
        {
            // Im Fall von Nahtende muss Datenframe aufgefuellt werden
//            if (oBufferArray[oInputIndex].m_oNextValueItem != 0) // oValueArray ist nicht leer
//            {
                // Array auffuellen
                while(oBufferArray[oInputIndex].m_oNextValueItem < m_oS6K_ImageCountUsed)
                {
                    oBufferArray[oInputIndex].m_oValueArray[oBufferArray[oInputIndex].m_oNextValueItem].m_oPosition  = 0xFFFF; // Ist-Position (bildNr * g_oImgDistance)
                    oBufferArray[oInputIndex].m_oValueArray[oBufferArray[oInputIndex].m_oNextValueItem].m_oLaserCorr = 0;      // Laserkorrektur (pos2cnc)
                    oBufferArray[oInputIndex].m_oValueArray[oBufferArray[oInputIndex].m_oNextValueItem].m_oImgMode = 0;        // Bildmodus (imgMode)
                    oBufferArray[oInputIndex].m_oValueArray[oBufferArray[oInputIndex].m_oNextValueItem].m_oGapWidth  = 0;      // Spaltmass (wire2cnc)
                    oBufferArray[oInputIndex].m_oNextValueItem++;
                }
//            }
        }

        if (oBufferArray[oInputIndex].m_oNextValueItem == m_oS6K_ImageCountUsed) // m_oS6K_ImageCountUsed Eintraege sind im Array
        {
            // Input Buffer zum Senden freigeben
            // hier noch ueberpruefen, ob Buffer zum Senden ueberhaupt frei ist !
            if (oBufferArray[oInputIndex].m_oOutputBlocked)
            {
                wmLogTr(eError, "QnxMsg.VI.S6KRingBFault2", "S6K: unable to transfer the new results for sending ! %s\n", "(001)");
            }
            else
            {
                // Input Buffer zum Senden freigeben
                oBufferArray[oInputIndex].m_oOutputBlocked = true;
                oBufferArray[oInputIndex].m_oSendIsRequested = true;
                oBufferArray[oInputIndex].m_oSendTrialsCnt = 0;
                // Input Buffer wieder fuer Eingabe freigeben
                oBufferArray[oInputIndex].m_oInputBlocked = false;
            }

            // Input auf naechsten Buffer umschalten
            int oNextInputIndex = (oInputIndex + 1) % BUFFER_ARRAY_LENGTH;
            // hier noch ueberpruefen, ob Buffer zur Eingabe ueberhaupt frei ist !
            if (oBufferArray[oNextInputIndex].m_oOutputBlocked)
            {
                wmLogTr(eError, "QnxMsg.VI.S6KRingBFault3", "S6K: next buffer is not free for new input ! %s\n", "(001)");
            }
            if (oBufferArray[oNextInputIndex].m_oInputBlocked)
            {
                wmLogTr(eError, "QnxMsg.VI.S6KRingBFault3", "S6K: next buffer is not free for new input ! %s\n", "(002)");
            }
            else
            {
                oInputIndex = oNextInputIndex;
                oBufferArray[oInputIndex].m_oNextValueItem = 0;
            }
        }
    }

    bool oSendIsRequested = false;
    for(int i = 0;i < BUFFER_ARRAY_LENGTH;i++)
    {
        if (oBufferArray[i].m_oSendIsRequested) oSendIsRequested = true;
    }
    if (oSendIsRequested) // jetzt Daten ueber Feldbus versenden
    {
        if (oBufferArray[oOutputIndex].m_oSendIsRequested)
        {
            if (m_oS6K_ResultDataDialogActive) // letztes Senden immer noch aktiv !!!
            {
                if (oBufferArray[oOutputIndex].m_oSendTrialsCnt < 8)
                {
                    if (m_oDebugInfo_SOURING_Extd)
                    {
                        wmLog(eDebug, "simSouring: neue Werte, aber Vorgaenger immer noch aktiv ! -> neuer Versuch\n");
                    }
                    oBufferArray[oOutputIndex].m_oSendTrialsCnt++;
                }
                else
                {
                    wmLogTr(eError, "QnxMsg.VI.S6KRingBFault4", "S6K: cannot send new values, acknowledge is missing !\n");
                    // hier jetzt aktueller Sendevorgang abbrechen
                    oBufferArray[oOutputIndex].m_oSendIsRequested = false;
                    oBufferArray[oOutputIndex].m_oSendTrialsCnt = 0;
                    oBufferArray[oOutputIndex].m_oOutputBlocked = false;
                    m_oS6K_ResultDataDialogActive.store(false);
                    m_oS6K_ResultDataState.store(0);
                }
                return; // Ruecksprung und neuer Versuch beim naechsten Aufruf
            }
            m_oS6K_ResultDataDialogActive = true;
            if (m_oDebugInfo_SOURING_Extd)
            {
                sprintf(statusStrg, "%d", oOutputIndex);
                wmLog(eDebug, "simSouring: Buffer %s wird jetzt gesendet\n", statusStrg);
            }

            pthread_mutex_lock(&m_oValueToSendMutex);

            for(int i = 0;i < m_oS6K_ImageCountUsed;i++)
            {
                m_oValueToSend[i] = ( (uint64_t)((oBufferArray[oOutputIndex].m_oValueArray[i].m_oPosition  & 0x00FF) >> 0) << 0  ) +
                                    ( (uint64_t)((oBufferArray[oOutputIndex].m_oValueArray[i].m_oPosition  & 0xFF00) >> 8) << 8  ) +
                                    ( (uint64_t)((oBufferArray[oOutputIndex].m_oValueArray[i].m_oLaserCorr & 0x00FF) >> 0) << 16 ) +
                                    ( (uint64_t)((oBufferArray[oOutputIndex].m_oValueArray[i].m_oLaserCorr & 0xFF00) >> 8) << 24 ) +
                                    ( (uint64_t)((oBufferArray[oOutputIndex].m_oValueArray[i].m_oGapWidth  & 0x00FF) >> 0) << 32 ) +
                                    ( (uint64_t)((oBufferArray[oOutputIndex].m_oValueArray[i].m_oGapWidth  & 0xFF00) >> 8) << 40 );
            }

            if (m_oDebugInfo_SOURING)
            {
                for(int i = 0;i < m_oS6K_ImageCountUsed;i++)
                {
                    signed short dummy1  =  (signed short)((m_oValueToSend[i] & 0x00000000FFFF) >> 0);
                    signed short dummy2  =  (signed short)((m_oValueToSend[i] & 0x0000FFFF0000) >> 16);
                    signed short dummy3  =  (signed short)((m_oValueToSend[i] & 0xFFFF00000000) >> 32);

                    sprintf(statusStrg, "%5d,%5d,%5d", dummy1, dummy2, dummy3);
                    wmLog(eDebug, "simSouring: %s\n", statusStrg);
                }
            }

            pthread_mutex_unlock(&m_oValueToSendMutex);

            // Values to Fieldbus-Card
            m_oS6K_RequestResultDataDialog = true;

            oBufferArray[oOutputIndex].m_oSendIsRequested = false;
            oBufferArray[oOutputIndex].m_oOutputBlocked = false;
            oBufferArray[oOutputIndex].m_oSendTrialsCnt = 0;

            // Output auf naechsten Buffer umschalten
            oOutputIndex = (oOutputIndex + 1) % BUFFER_ARRAY_LENGTH;
            if (m_oDebugInfo_SOURING_Extd)
            {
                sprintf(statusStrg, "%d", oOutputIndex);
                wmLog(eDebug, "simSouring: naechster Sende-Puffer: %s (001)\n", statusStrg);
            }
        }
        else // globales sendIsRequested aktiv, aber in oOutputIndex ist sendIsRequested inaktiv !
        {
            if (m_oDebugInfo_SOURING_Extd)
            {
                sprintf(statusStrg, "(%d)", oOutputIndex);
                wmLog(eDebug, "simSouring: Senden global angefordert, aber aktueller Puffer %s kein SendRequest\n", statusStrg);
            }
            // Output auf naechsten Buffer umschalten
            oOutputIndex = (oOutputIndex + 1) % BUFFER_ARRAY_LENGTH;
            if (m_oDebugInfo_SOURING_Extd)
            {
                sprintf(statusStrg, "%d", oOutputIndex);
                wmLog(eDebug, "simSouring: naechster Sende-Puffer: %s (002)\n", statusStrg);
            }
        }
    }
}

void VI_InspectionControl::S6KCyclicTaskFunction(void)
{
    static bool oRequestQualityResultDialog = false;

    ////////////////////////////////////////////
    // processing dialog for takeover cycle data
    ////////////////////////////////////////////
    static bool oS6K_Old_CycleDataValidStatus = false;
    static uint32_t oCycleDataInput = 0;
    static uint32_t oCycleDataInputLong = 0;
    static uint32_t oProductNumberInput = 0;

    // signal has changed it's status
    if (m_oS6K_CycleDataValidStatus != oS6K_Old_CycleDataValidStatus)
    {
        if (m_oS6K_CycleDataValidStatus == true) // signal is now high
        {
            if (oS6K_Old_CycleDataValidStatus == false) // signal was low -> rising edge of signal
            {
                if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
                {
                    m_oS6K_ProductNumberInput.store(m_oS6K_ProductNoFromTCP);
                    oCycleDataInputLong = m_oS6K_CycleDataInput.load();
                    oCycleDataInput = (m_oS6K_CycleDataInput.load() & 0xFFFF); // only 16 Bit are relevant
                }
                else
                {
                    oCycleDataInput = m_oS6K_CycleDataInput.load();
                }
                wmLog(eDebug, "rising edge: CycleDataValid Cyc:%d, Prod:%d\n", oCycleDataInput, m_oS6K_ProductNumberInput.load());
                // pass on the cycle data to TCPCommunication
                m_rS6K_InfoToProcessesProxy.sendS6KCmdToProcesses(eS6K_CmdBatchID, oCycleDataInput);
                // pass on the raw product number to TCPCommunication
                m_rS6K_InfoToProcessesProxy.sendS6KCmdToProcesses(eS6K_CmdProductNumber, m_oS6K_ProductNumberInput);
                oProductNumberInput = m_oS6K_ProductNumberInput;
                if (oProductNumberInput == 0) oProductNumberInput = 1;
                BoolSender(true, &m_senderS6K_AcknCycleData);
            }
            else // signal was high -> no change
            {
            }
        }
        else // signal is now low
        {
            if (oS6K_Old_CycleDataValidStatus == true) // signal was high -> falling edge of signal
            {
                wmLog(eDebug, "falling edge: CycleDataValid\n");
                BoolSender(false, &m_senderS6K_AcknCycleData);
            }
            else // signal was low -> no change
            {
            }
        }
        oS6K_Old_CycleDataValidStatus = m_oS6K_CycleDataValidStatus;
    }

    /////////////////////////////////////////////
    // processing dialog for takeover seam number
    /////////////////////////////////////////////
    static bool oS6K_Old_SeamNoValidStatus = false;
    static uint32_t oS6K_SeamNoInputMirror = 0;
    static uint32_t oS6K_SeamNoInput = 0;

    // signal has changed it's status
    if (m_oS6K_SeamNoValidStatus != oS6K_Old_SeamNoValidStatus)
    {
        if (m_oS6K_SeamNoValidStatus == true) // signal is now high
        {
            if (oS6K_Old_SeamNoValidStatus == false) // signal was low -> rising edge of signal
            {
                wmLog(eDebug, "rising edge: SeamNoValid Seam:%d\n", m_oS6K_SeamNoInput.load());
                oS6K_SeamNoInputMirror = m_oS6K_SeamNoInput;
                oS6K_SeamNoInput = oS6K_SeamNoInputMirror;
                if (oS6K_SeamNoInput != 0) oS6K_SeamNoInput--;

                inspectionProxy.seamPreStart(oS6K_SeamNoInput);

                if (isSOUVIS6000_CrossSectionMeasurementEnable() && !isSOUVIS6000_CrossSection_Leading_System())
                {
                    static unsigned int searchIndex{0};
                    const size_t arraySize{m_oS6K_CS_LeadingResultsBufferArray.size()};
                    bool notFound{true};
                    for (size_t i = 0; i < arraySize; i++)
                    {
                        if ((m_oS6K_CS_LeadingResultsBufferArray[searchIndex].m_oProductNo == oProductNumberInput) &&
                            (m_oS6K_CS_LeadingResultsBufferArray[searchIndex].m_oBatchID == oCycleDataInput) &&
                            (m_oS6K_CS_LeadingResultsBufferArray[searchIndex].m_oSeamNo == oS6K_SeamNoInputMirror))
                        {
                            m_oS6K_CS_CurrentLeadingResultsOutput = searchIndex;
                            m_pS6K_CS_LeadingResultsOutput = &m_oS6K_CS_LeadingResultsBufferArray[m_oS6K_CS_CurrentLeadingResultsOutput];
                            m_oS6K_CS_CurrMeasuresPerResult.store(m_pS6K_CS_LeadingResultsOutput->m_oMeasuresPerResult);
                            notFound = false;
                            searchIndex++;
                            if (searchIndex >= arraySize)
                            {
                                searchIndex = 0;
                            }
                            wmLog(eDebug, "for outputting CS data use now buffer %d\n", m_oS6K_CS_CurrentLeadingResultsOutput);
                            break;
                        }
                        searchIndex++;
                        if (searchIndex >= arraySize)
                        {
                            searchIndex = 0;
                        }
                    }
                    if (notFound)
                    {
                        m_pS6K_CS_LeadingResultsOutput = nullptr;
                        wmLogTr(eError, "QnxMsg.VI.CSCombNotPresent", "Combination of ProductNumber, BatchID and SeamNo not present in Leading Results !\n");
                    }
                }

                if ( !isSOUVIS6000_Automatic_Seam_No() )
                {
                    BoolSender(true, &m_senderS6K_AcknSeamNo);
                }
            }
            else // signal was high -> no change
            {
            }
        }
        else // signal is now low
        {
            if (oS6K_Old_SeamNoValidStatus == true) // signal was high -> falling edge of signal
            {
                wmLog(eDebug, "falling edge: SeamNoValid\n");
                if ( !isSOUVIS6000_Automatic_Seam_No() )
                {
                    BoolSender(false, &m_senderS6K_AcknSeamNo);
                }
            }
            else // signal was low -> no change
            {
            }
        }
        oS6K_Old_SeamNoValidStatus = m_oS6K_SeamNoValidStatus;
    }

    ///////////////////////////////////////////////
    // processing signal input S6K_SouvisInspection
    ///////////////////////////////////////////////
    static bool oS6K_Old_SouvisInspectionStatus = false;

    // signal has changed it's status
    if (m_oS6K_SouvisInspectionStatus != oS6K_Old_SouvisInspectionStatus)
    {
        if (m_oS6K_SouvisInspectionStatus == true) // signal is now high
        {
            if (oS6K_Old_SouvisInspectionStatus == false) // signal was low -> rising edge of signal
            {
                wmLog(eDebug, "rising edge: SouvisInspection\n");
                m_oS6K_SouvisActiveStatusBuffer = m_oS6K_SouvisActiveStatus;
                // abhaengig von m_oS6K_SouvisActiveStatusBuffer
                if ((m_oS6K_SouvisActiveStatusBuffer) && (m_oS6K_SouvisSelected))
                {
                    if (isSOUVIS6000_Is_PreInspection())
                    {
                        // reset data input buffer
                        for(int i = 0;i < S6K_COLLECT_LENGTH;i++)
                        {
                            m_oS6K_CollectResultData[i].m_oEdgePositionValue = 0;
                            m_oS6K_CollectResultData[i].m_oEdgePositionImgNo = -1;
                            m_oS6K_CollectResultData[i].m_oEdgePositionPos = 0;
                            m_oS6K_CollectResultData[i].m_oEdgePositionValid = false;
                            m_oS6K_CollectResultData[i].m_oGapWidthValue = 0;
                            m_oS6K_CollectResultData[i].m_oGapWidthImgNo = -1;
                            m_oS6K_CollectResultData[i].m_oGapWidthPos = 0;
                            m_oS6K_CollectResultData[i].m_oGapWidthValid = false;
                            m_oS6K_CollectResultData[i].m_oReadyToInsert = false;
                        }
                        m_oS6K_CollectResultDataWrite = 0;
                        m_oS6K_CollectResultDataRead  = 0;

                        // reset ringbuffer for sending data
                        sendPositionValueToSimotion(eS6K_Command_ResetVars);
                    }

                    // ProductNumber == ProductType is read from fieldbus
                    // CycleData == BatchID is read from Fieldbus
                    inspectionProxy.startAutomaticmode(oProductNumberInput, oCycleDataInput, "no info");
                    wmLogTr(eInfo, "QnxMsg.VI.InspCycleActiveHigh", "inspection cycle started: product-no: %d part-no: %u\n", oProductNumberInput, oCycleDataInput);

                    if ( isSOUVIS6000_Automatic_Seam_No() )
                    {
                        m_oS6K_Automatic_Seam_No_Buffer = 1;
                        m_oS6K_Automatic_Seam_No_TimeoutActive = true;
                        m_oS6K_Automatic_Seam_No_TimeoutCounter = 0;
                    }
                }
                m_oS6K_SeamErrorCat1Accu = 0xFFFFFF;
                m_oS6K_SeamErrorCat2Accu = 0xFFFFFF;
            }
            else // signal was high -> no change
            {
            }
        }
        else // signal is now low
        {
            if (oS6K_Old_SouvisInspectionStatus == true) // signal was high -> falling edge of signal
            {
                wmLog(eDebug, "falling edge: SouvisInspection\n");
                // abhaengig von m_oS6K_SouvisActiveStatusBuffer
                if ((m_oS6K_SouvisActiveStatusBuffer) && (m_oS6K_SouvisSelected))
                {
                    inspectionProxy.stopAutomaticmode();
                    oRequestQualityResultDialog = true;
                    m_rS6K_InfoToProcessesProxy.sendRequestQualityData();
                    wmLogTr(eInfo, "QnxMsg.VI.InspCycleActiveLow", "inspection cycle stopped\n");
                }
            }
            else // signal was low -> no change
            {
            }
        }
        oS6K_Old_SouvisInspectionStatus = m_oS6K_SouvisInspectionStatus;
    }

    /////////////////////////////////////////////////////
    // processing digital signal for Make Pictures 
    /////////////////////////////////////////////////////
    static bool oS6K_Old_MakePicturesStatus = false;

    // signal has changed it's status
    if (m_oS6K_MakePicturesStatus != oS6K_Old_MakePicturesStatus)
    {
        if (m_oS6K_MakePicturesStatus == true) // signal is now high
        {
            if (oS6K_Old_MakePicturesStatus == false) // signal was low -> rising edge of signal
            {
                wmLog(eDebug, "rising edge: MakePictures\n");
                // abhaengig von m_oS6K_SouvisActiveStatusBuffer
                if ((m_oS6K_SouvisActiveStatusBuffer) && (m_oS6K_SouvisSelected))
                {
                    if (isSOUVIS6000_Is_PreInspection())
                    {
                        // reset ringbuffer for sending data
                        sendPositionValueToSimotion(eS6K_Command_ResetVars);
                    }
                    inspectionProxy.start(oS6K_SeamNoInput);
                    wmLogTr(eInfo, "QnxMsg.VI.InspSeamActiveHigh", "seam inspection started: seam-series-no: %u seam-no: %u\n",1 ,(oS6K_SeamNoInput+1));
                }
            }
            else // signal was high -> no change
            {
            }
        }
        else // signal is now low
        {
            if (oS6K_Old_MakePicturesStatus == true) // signal was high -> falling edge of signal
            {
                wmLog(eDebug, "falling edge: MakePictures\n");
                // abhaengig von m_oS6K_SouvisActiveStatusBuffer
                if ((m_oS6K_SouvisActiveStatusBuffer) && (m_oS6K_SouvisSelected))
                {
                    inspectionProxy.end(oS6K_SeamNoInput);
                    m_oS6K_SeamEndTimeoutActive = true;
                    m_oS6K_SeamEndTimeoutCounter = 0;
                    wmLogTr(eInfo, "QnxMsg.VI.InspSeamActiveLow", "seam inspection stopped\n");
                }
            }
            else // signal was low -> no change
            {
            }
        }
        oS6K_Old_MakePicturesStatus = m_oS6K_MakePicturesStatus;
    }

    /////////////////////////////////////////////////////
    // dialog for transmitting quality results to machine
    /////////////////////////////////////////////////////
    static int oQualityResultState = 0;
    static int oQualityResultDelayTime = 0;
    static int oQualityResultDelayCounter = 0;

    switch(oQualityResultState)
    {
        case 0:
            // waiting for request to transmit quality data
            if (oRequestQualityResultDialog)
            {
                oRequestQualityResultDialog = false;
                oQualityResultDelayTime = 50; // 50ms
                oQualityResultDelayCounter = 0;
                oQualityResultState = 1;
                if (m_oDebugInfo_SOURING_Extd)
                {
                    wmLog(eDebug, "oQualityResultState: goto 1\n");
                }
            }
            break;
        case 1:
            // writing quality results onto fieldbus
            if (oQualityResultDelayCounter < oQualityResultDelayTime)
            {
                oQualityResultDelayCounter++;
            }
            else
            {
                if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
                {
                    FieldSender((uint64_t)oCycleDataInputLong, &m_senderS6K_CycleDataMirror);
                }
                else
                {
                    FieldSender((uint64_t)oCycleDataInput, &m_senderS6K_CycleDataMirror);
                }
                FieldSender((uint64_t)m_oS6K_SeamErrorCat1Accu, &m_senderS6K_SeamErrorCat1);
                FieldSender((uint64_t)m_oS6K_SeamErrorCat2Accu, &m_senderS6K_SeamErrorCat2);
                wmLog(eDebug, "m_oS6K_SeamErrorCat1Accu: %x\n", m_oS6K_SeamErrorCat1Accu.load());
                wmLog(eDebug, "m_oS6K_SeamErrorCat2Accu: %x\n", m_oS6K_SeamErrorCat2Accu.load());
                oQualityResultDelayTime = 5; // 5ms
                oQualityResultDelayCounter = 0;
                oQualityResultState = 2;
                if (m_oDebugInfo_SOURING_Extd)
                {
                    wmLog(eDebug, "oQualityResultState: goto 2\n");
                }
            }
            break;
        case 2:
            // set quality results valid
            if (oQualityResultDelayCounter < oQualityResultDelayTime)
            {
                oQualityResultDelayCounter++;
            }
            else
            {
                BoolSender(true, &m_senderS6K_QualityDataValid);
                oQualityResultState = 3;
                if (m_oDebugInfo_SOURING_Extd)
                {
                    wmLog(eDebug, "oQualityResultState: goto 3\n");
                }
            }
            break;
        case 3:
            // waiting for acknowledge from machine is high
            if (m_oS6K_AcknQualityDataStatus == true)
            {
                // reset quality results valid
                BoolSender(false, &m_senderS6K_QualityDataValid);
                oQualityResultState = 4;
                if (m_oDebugInfo_SOURING_Extd)
                {
                    wmLog(eDebug, "oQualityResultState: goto 4\n");
                }
            }
            break;
        case 4:
            // waiting for acknowledge from machine is low
            if (m_oS6K_AcknQualityDataStatus == false)
            {
                oQualityResultState = 0;
                if (m_oDebugInfo_SOURING_Extd)
                {
                    wmLog(eDebug, "oQualityResultState: goto 0\n");
                }
            }
            break;
        default:
            break;
    }

    ///////////////////////////////////////////////////
    // dialog for transmitting image results to machine
    ///////////////////////////////////////////////////
    //static int oResultDataState = 0;
    static int oResultDataDelayTime = 0;
    static int oResultDataDelayCounter = 0;
    static bool oSeamNoToZero = false;

    switch(m_oS6K_ResultDataState.load())
    {
        case 0:
            // waiting for request to transmit result data
            if (m_oS6K_RequestResultDataDialog)
            {
                m_oS6K_RequestResultDataDialog = false;
                m_oS6K_ResultDataState.store(1);
                if (m_oDebugInfo_SOURING_Extd)
                {
                    wmLog(eDebug, "oResultDataState: goto 1\n");
                }
            }
            break;
        case 1:
            // writing result data onto fieldbus
            FieldSender((uint64_t)m_oS6K_ImageCountUsed, &m_senderS6K_ResultDataCount);

            pthread_mutex_lock(&m_oValueToSendMutex);
            for(int i = 0;i < m_oS6K_ImageCountUsed;i++)
            {
                FieldSender((uint64_t)m_oValueToSend[i], &m_senderS6K_ResultsImage[i]);

                if ((m_oValueToSend[i] & 0x000000FFFF) == 0)
                {
                    // mask the position value of the actual result
                    // if the position is 0, then this is the first image of a seam
                    // the interface specification since version 1.3 wants the mirroring of the seam number at this point
                    FieldSender((uint64_t)oS6K_SeamNoInputMirror, &m_senderS6K_SeamNoMirror);
                }
                if ((m_oValueToSend[i] & 0x000000FFFF) == 0xFFFF)
                {
                    // mask the position value of the actual result
                    // if the position is 0xFFFF, then this is after the last image of a seam
                    // the interface specification since version 1.3 wants that seam number is set to 0, after the last dialog of the seam
                    oSeamNoToZero = true;
                }
            }
            pthread_mutex_unlock(&m_oValueToSendMutex);

            oResultDataDelayTime = 3; // 3ms
            oResultDataDelayCounter = 0;
            m_oS6K_ResultDataState.store(2);
            if (m_oDebugInfo_SOURING_Extd)
            {
                wmLog(eDebug, "oResultDataState: goto 2\n");
            }
            break;
        case 2:
            // set result data valid
            if (oResultDataDelayCounter < oResultDataDelayTime)
            {
                oResultDataDelayCounter++;
            }
            else
            {
                BoolSender(true, &m_senderS6K_ResultDataValid);
                m_oS6K_ResultDataState.store(3);
                if (m_oDebugInfo_SOURING_Extd)
                {
                    wmLog(eDebug, "oResultDataState: goto 3\n");
                }
            }
            break;
        case 3:
            // waiting for acknowledge from machine is high
            if (m_oS6K_AcknResultDataStatus == true)
            {
                // reset result data valid
                BoolSender(false, &m_senderS6K_ResultDataValid);
                m_oS6K_ResultDataState.store(4);
                if (m_oDebugInfo_SOURING_Extd)
                {
                    wmLog(eDebug, "oResultDataState: goto 4\n");
                }
            }
            break;
        case 4:
            // waiting for acknowledge from machine is low
            if (m_oS6K_AcknResultDataStatus == false)
            {
                m_oS6K_ResultDataDialogActive = false;
                if (oSeamNoToZero) // in case of the last dialog -> direct mirror of seam no to 0
                {
                    FieldSender((uint64_t)0, &m_senderS6K_SeamNoMirror);
                    oSeamNoToZero = false;
                }
                m_oS6K_ResultDataState.store(0);
                if (m_oDebugInfo_SOURING_Extd)
                {
                    wmLog(eDebug, "oResultDataState: goto 0\n");
                }
            }
            break;
        default:
            break;
    }

    if (m_oS6K_SeamEndTimeoutActive)
    {
        m_oS6K_SeamEndTimeoutCounter++;
        if (m_oS6K_SeamEndTimeoutCounter > 40) // 40 ms, muss eine Dialogdauer ermoeglichen
        {
            wmLog(eDebug, "SeamEndTimeout has expired\n");
            if (isSOUVIS6000_Is_PreInspection())
            {
                sendPositionValueToSimotion(eS6K_CommandLastSend);
            }
            m_oS6K_SeamEndTimeoutActive = false;
            m_oS6K_SeamEndTimeoutCounter = 0;

            if ( isSOUVIS6000_Automatic_Seam_No() )
            {
                m_oS6K_Automatic_Seam_No_TimeoutActive = true;
                m_oS6K_Automatic_Seam_No_TimeoutCounter = 0;
            }
        }
    }

    if (m_oS6K_Automatic_Seam_No_TimeoutActive)
    {
        m_oS6K_Automatic_Seam_No_TimeoutCounter++;
        if (m_oS6K_Automatic_Seam_No_TimeoutCounter == 10) // 10 ms
        {
            wmLog(eDebug, "Automatic_Seam_No_TimeoutActive has expired\n");
            if (m_oS6K_Automatic_Seam_No_Buffer <= m_oS6K_NumberOfPresentSeams)
            {
                m_oS6K_SeamNoInput.store(m_oS6K_Automatic_Seam_No_Buffer);
                m_oS6K_SeamNoValidStatus.store(true);
                m_oS6K_Automatic_Seam_No_Buffer++;
            }
        }
        else if (m_oS6K_Automatic_Seam_No_TimeoutCounter >= 15) // 15 ms
        {
            m_oS6K_SeamNoValidStatus.store(false);
            m_oS6K_Automatic_Seam_No_TimeoutActive = false;
            m_oS6K_Automatic_Seam_No_TimeoutCounter = 0;
        }
    }

    /////////////////////////////////////////////////////////////////
    // triggering sendPositionValueToSimotion, ringbuffer for results
    /////////////////////////////////////////////////////////////////
    if (isSOUVIS6000_Is_PreInspection())
    {
        if (m_oS6K_CollectResultDataRead != m_oS6K_CollectResultDataWrite)
        {
            // content of struct at read index should now be valid
            if (m_oS6K_CollectResultData[m_oS6K_CollectResultDataRead].m_oReadyToInsert != true)
            {
                // there is a problem, this flag should be true at this time
                wmLogTr(eError, "QnxMsg.VI.S6KBufNotFree", "cannot insert new values in buffer !\n");
            }
            sendPositionValueToSimotion(m_oS6K_CollectResultDataRead);
            // step forward read index after using the data at read index
            m_oS6K_CollectResultDataRead = (m_oS6K_CollectResultDataRead + 1) % S6K_COLLECT_LENGTH;
        }
        else
        {
            sendPositionValueToSimotion(eS6K_Command_NoData);
        }
    }

    /////////////////////////////////////////////////////
    // processing signal input for QuitSystemFault
    /////////////////////////////////////////////////////
    // use the same function as with non SOUVIS6000 application
    TriggerQuitSystemFault(m_oS6K_QuitSystemFaultStatus.load());

    /////////////////////////////////////////////////////
    // processing signal input for MachineReady
    /////////////////////////////////////////////////////
    static bool oS6K_Old_MachineReadyStatus = false;

    // signal has changed it's status
    if (m_oS6K_MachineReadyStatus != oS6K_Old_MachineReadyStatus)
    {
        if (m_oS6K_MachineReadyStatus == true) // signal is now high
        {
            if (oS6K_Old_MachineReadyStatus == false) // signal was low -> rising edge of signal
            {
                wmLog(eDebug, "rising edge: MachineReady\n");
                // TCPCommunication shall send statusData block to machine
                m_rS6K_InfoToProcessesProxy.sendRequestStatusData();
            }
            else // signal was high -> no change
            {
            }
        }
        else // signal is now low
        {
            if (oS6K_Old_MachineReadyStatus == true) // signal was high -> falling edge of signal
            {
                wmLog(eDebug, "falling edge: MachineReady\n");
            }
            else // signal was low -> no change
            {
            }
        }
        oS6K_Old_MachineReadyStatus = m_oS6K_MachineReadyStatus;
    }
}

/***************************************************************************/
/* SM_ThreeStepCyclicTaskFunction                                          */
/***************************************************************************/

void VI_InspectionControl::SM_ThreeStepCyclicTaskFunction(void)
{
    static int oStateVariable = 0;
    static int oWaitCounter = 0;
    static int oStepNo = 1;
    static int oSeamNo = 1;

    switch(oStateVariable)
    {
        case 0: // nothing to do
            {
                if (m_oSM_StartSeamserieSequence.load())
                {
                    BoolSender(true, &m_senderSM_ProcessingActive);
                    m_oSM_StartSeamserieSequence.store(false);
                    m_oSM_TimeoutOccured.store(false);
                    oStepNo = 1;
                    oSeamNo = 1;
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: rising edge of <Take-Over Seam Series>: takeover processing and start actions (goto 1)\n");
                    }
                    oStateVariable = 1;
                }
            }
            break;
        case 1: // waiting for some milliseconds
            {
                oWaitCounter++;
                if (oWaitCounter >= 10) // 10ms
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: After short delay: Output <Step Bit 0 ... Bit 2> (goto 2)\n");
                    }
                    oStateVariable = 2;
                }
            }
            break;
        case 2: // output step number
            {
                FieldSender(oStepNo, &m_senderSM_StepField);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: After short delay: set signal <Take-Over Step> (goto 3)\n");
                }
                oStateVariable = 3;
            }
            break;
        case 3: // waiting for some milliseconds and set afterwards TakeoverStep
            {
                oWaitCounter++;
                if (oWaitCounter >= 10) // 10ms
                {
                    BoolSender(true, &m_senderSM_TakeoverStep);
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Wait for high status of signal <Acknowledge Step> (goto 4)\n");
                    }
                    oStateVariable = 4;
                }
            }
            break;
        case 4: // waiting for high status of m_oSM_AcknowledgeStepStatus
            {
                oWaitCounter++;
                if (oWaitCounter >= 30000) // Timeout 30 seconds
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Waiting for <Acknowledge Step> high status has timeout (goto 100)\n");
                    }
                    oStateVariable = 100;
                }
                if (m_oSM_AcknowledgeStepStatus.load()) // acknowledge signal is set
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: reset signal <Take-Over Step>  (goto 5)\n");
                    }
                    oStateVariable = 5;
                }
            }
            break;
        case 5: // reset TakeoverStep
            {
                BoolSender(false, &m_senderSM_TakeoverStep);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Wait for low status of signal <Acknowledge Step> (goto 6)\n");
                }
                oStateVariable = 6;
            }
            break;
        case 6: // waiting for low status of m_oSM_AcknowledgeStepStatus
            {
                oWaitCounter++;
                if (oWaitCounter >= 30000) // Timeout 30 seconds
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Waiting for <Acknowledge Step> low status has timeout (goto 101)\n");
                    }
                    oStateVariable = 101;
                }
                if (!m_oSM_AcknowledgeStepStatus.load()) // acknowledge signal is reset
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: now a seam is processed (goto 10)\n");
                    }
                    oStateVariable = 10;
                }
            }
            break;
        case 10: // set InspectStartStop on
            {
                if (oStepNo == static_cast<int>(m_oSM_StepCount.load())) // this is a seam with welding (in last step)
                {
                    // check if there is a NIO, if it is NIO do not weld this seam !
                    if (static_cast<bool>(m_oSM_NIOBuffer[oSeamNo -1]) == true) // there is a NIO
                    {
                        wmLogTr(eWarning, "QnxMsg.VI.SMSeamNotWelded", "Seam number %d is not welded due to former NOK", oSeamNo);
                        ++oSeamNo; // go to next seam
                        if (oSeamNo <= static_cast<int>(m_oSM_SeamCount.load()))
                        {
                            oWaitCounter = 0;
                            if (m_oDebugInfo_SCANMASTER)
                            {
                                wmLog(eDebug, "ScanMaster: now the next seam is processed (goto 10)\n");
                            }
                            oStateVariable = 10;
                        }
                        else // there is no more seam to weld
                        {
                            BoolSender(false, &m_senderSM_ProcessingActive);
                            oWaitCounter = 0;
                            if (m_oDebugInfo_SCANMASTER)
                            {
                                wmLog(eDebug, "ScanMaster: there is no more seam to weld, release processing and wait for new sequence (goto 0)\n");
                            }
                            oStateVariable = 0;
                        }
                        break; // restart the state machine
                    }
                }

                int oSeamIndex = ((oStepNo - 1) * m_oSM_SeamCount.load()) + (oSeamNo - 1);
                m_oSM_BurstWasReceived.store(false);
                TriggerInspectStartStop(true, oSeamIndex);
                // determine the time for the seam
                uint32_t oVelocity = 1000; // [um/s]
                uint32_t oTriggerDelta = 1000; // [um]
                uint32_t oSeamLength  = 0; // [mm]
                for( MeasureTaskList::iterator oIterMeasTask = m_oSM_MeasureTasks.begin(); oIterMeasTask!=m_oSM_MeasureTasks.end(); ++oIterMeasTask )
                {
                    if ((static_cast<uint32_t>(oIterMeasTask->seamseries()) == m_oSM_SeamseriesNumber) && 
                        (oIterMeasTask->seam() == oSeamIndex) && 
                        (oIterMeasTask->level() == 1)) // level: seam
                    {
                        oVelocity = oIterMeasTask->velocity();
                        oTriggerDelta = oIterMeasTask->triggerDelta();
                    }
                    if ((static_cast<uint32_t>(oIterMeasTask->seamseries()) == m_oSM_SeamseriesNumber) && 
                        (oIterMeasTask->seam() == oSeamIndex) && 
                        (oIterMeasTask->level() == 2)) // level: seam interval
                    {
                        oSeamLength += oIterMeasTask->length();
                    }
                }
                uint32_t oTimePerImage = (oTriggerDelta * 1000) / oVelocity; // [ms]
                uint32_t oImageCount = oSeamLength / oTriggerDelta;
                m_oSM_TimePerSeam = (oTimePerImage * oImageCount); // [ms]
                if (oStepNo == static_cast<int>(m_oSM_StepCount.load())) // this is a seam with welding
                {
                    m_oSM_TimePerSeam += 0; // 0ms reserve time
                }
                else
                {
                    m_oSM_TimePerSeam += 0; // 0ms reserve time
                }
                wmLog(eDebug, "Len: %d, TimeImg: %d, ImgCnt: %d, TimeSeam: %d\n", oSeamLength, oTimePerImage, oImageCount, m_oSM_TimePerSeam);

                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Wait for start of image acquisition (goto 11)\n");
                }
                oStateVariable = 11;
            }
            break;
        case 11: // waiting for start of image acquisition
            {
                oWaitCounter++;
                if (oWaitCounter >= 1000) // Timeout 1 second
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Waiting for start of image acquisition has timeout (goto 102)\n");
                    }
                    oStateVariable = 102;
                }
                if (m_oSM_BurstWasReceived.load())
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Wait for end of seam processing (goto 12)\n");
                    }
                    oStateVariable = 12;
                }
            }
            break;
        case 12: // waiting for duration of seam
            {
                oWaitCounter++;
                if (oWaitCounter >= static_cast<int>(m_oSM_TimePerSeam)) // [ms]
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Time for seam processing has expired (goto 13)\n");
                    }
                    oStateVariable = 13;
                }
            }
            break;
        case 13: // set InspectStartStop off
            {
                int oSeamIndex = ((oStepNo - 1) * m_oSM_SeamCount.load()) + (oSeamNo - 1);
                TriggerInspectStartStop(false, oSeamIndex);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Seam end was internally signalled, wait some milliseconds (goto 14)\n");
                }
                oStateVariable = 14;
            }
            break;
        case 14: // waiting for some milliseconds
            {
                oWaitCounter++;
                if (oWaitCounter >= 5) // 5ms
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Time after seam end has expired (goto 15)\n");
                    }
                    oStateVariable = 15;
                }
            }
            break;
        case 15: // goto start state or to next seam
            {
                if (oSeamNo < static_cast<int>(m_oSM_SeamCount.load())) // next seam must be triggered
                {
                    ++oSeamNo;
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Next seam in same step must be processed (goto 10)\n");
                    }
                    oStateVariable = 10;
                }
                else if (oStepNo < static_cast<int>(m_oSM_StepCount.load())) // next step must be triggered
                {
                    oSeamNo = 1;
                    ++oStepNo;
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Next step must be started (goto 1)\n");
                    }
                    oStateVariable = 1;
                }
                else // all seams are triggered, goto start
                {
                    BoolSender(false, &m_senderSM_ProcessingActive);
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: All steps and seams are processed, release processing and wait for new sequence (goto 0)\n");
                    }
                    oStateVariable = 0;
                }
            }
            break;
        case 100: // Timeout in state machine: rising edge of Acknowledge Step
            {
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout", "There is a timeout in ScanMaster state machine\n");
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout1", "Waiting for <Acknowledge Step> high status has timeout\n");
                m_oSM_TimeoutOccured.store(true);
                BoolSender(false, &m_senderSM_TakeoverStep);
                BoolSender(false, &m_senderSM_ProcessingActive);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Processing of sequence is canceled, release processing and wait for new sequence (goto 0)\n");
                }
                oStateVariable = 0;
            }
            break;
        case 101: // Timeout in state machine: falling edge of Acknowledge Step
            {
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout", "There is a timeout in ScanMaster state machine\n");
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout2", "Waiting for <Acknowledge Step> low status has timeout\n");
                m_oSM_TimeoutOccured.store(true);
                BoolSender(false, &m_senderSM_ProcessingActive);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Processing of sequence is canceled, release processing and wait for new sequence (goto 0)\n");
                }
                oStateVariable = 0;
            }
            break;
        case 102: // Timeout in state machine: waiting for start of image acquisition
            {
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout", "There is a timeout in ScanMaster state machine\n");
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout3", "Waiting for start of image acquisition has timeout\n");
                m_oSM_TimeoutOccured.store(true);
                int oSeamIndex = ((oStepNo - 1) * m_oSM_SeamCount.load()) + (oSeamNo - 1);
                TriggerInspectStartStop(false, oSeamIndex);
                BoolSender(false, &m_senderSM_ProcessingActive);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Processing of sequence is canceled, release processing and wait for new sequence (goto 0)\n");
                }
                oStateVariable = 0;
            }
            break;
        default:
            {
            }
            break;
    }
}

/***************************************************************************/
/* SM_GeneralCyclicTaskFunction                                            */
/***************************************************************************/

void VI_InspectionControl::SM_SetEndOfSeamMarker(int p_oSeamNoForResult)
{
    m_oSM_SeamProcessingEnded.store(true);
    m_oSM_SeamProcessingEndSeamNumber.store(p_oSeamNoForResult);
    wmLog(eDebug, "Got EndOfSeamMarker: seamNo: %d\n", p_oSeamNoForResult);
}

void VI_InspectionControl::SM_SetSpotWelding()
{
    m_spotWeldingIsOn = true;
}

void VI_InspectionControl::SM_SendNOKResultAtTimeout(const ResultType resultType, const ResultType nioType)
{
    if (m_resultsProxy)
    {
        GeoDoublearray geoArray{};
        geoArray.ref().getData().push_back(1.0);
        geoArray.ref().getRank().push_back(filter::ValueRankType::eRankMax);
        auto result = interface::ResultDoubleArray
        {
            {},                      // FilterId
            resultType,              // ResultType
            nioType,                 // NioType
            {},                      // ImageContext
            geoArray,                // GeoValue
            {-100000.0, 100000.0},   // Deviation
            true                     // IsNio
        };
        m_resultsProxy->nio(result);
    }
}

VI_InspectionControl::StateMachineTransition VI_InspectionControl::SM_GeneralCyclicTaskFunction(void)
{
    static int oStateVariable = 0;
    static int oWaitCounter = 0;
    static int oSeamNo = 1;
    static int lastSeamLWMSelection = 0;
#if LWM_DEVICE_TRIGGER_SIMULATION
    static bool searchSeamShadow{false};
#endif

    StateMachineTransition transition{StateMachineTransition::WaitForNextCycle};

    switch(oStateVariable)
    {
        case 0: // nothing to do
            {
                if (m_oSM_StartSeamserieSequence.load())
                {
                    BoolSender(true, &m_senderSM_ProcessingActive);
                    m_oSM_StartSeamserieSequence.store(false);
                    m_oSM_TimeoutOccured.store(false);
                    oSeamNo = 1;
                    lastSeamLWMSelection = 0;
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: rising edge of <Take-Over Seam Series>: takeover processing and start actions (goto 10)\n");
                    }
                    oStateVariable = 10;
                }
            }
            break;
        case 10:
            {
                int oSeamIndex = (oSeamNo - 1);
                int oSeamSeriesIndex = m_oSM_SeamseriesNumber.load();
                oWaitCounter = 0;
                if ((isCommunicationToLWMDeviceActive()) && (m_LWMParameterVector[oSeamSeriesIndex][oSeamIndex].m_LWMInspectionActive))
                {
#if LWM_DEVICE_TRIGGER_SIMULATION
                    setCabinetTemperature(false);
                    searchSeamShadow = false;
#endif
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: we have to activate the LWM device (goto 20)\n");
                    }
                    oStateVariable = 20;
                }
                else
                {
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: now a seam is processed (goto 100)\n");
                    }
                    oStateVariable = 100;
                }
                transition = StateMachineTransition::Direct;
            }
            break;
        case 20: // LWM selection senden
            {
                if (lastSeamLWMSelection < oSeamNo)
                {
                    int oSeamIndex = (oSeamNo - 1);
                    int oSeamSeriesIndex = m_oSM_SeamseriesNumber.load();
                    m_LWMSimpleIOSelectionAckn.store(false);
                    m_LWMSimpleIOErrorReceived.store(false);

                    SimpleIOSelectionDataType simpleIOSelectionData{};
                    simpleIOSelectionData.m_requestAcknowledge = true;
                    simpleIOSelectionData.m_systemActivated = true;
                    simpleIOSelectionData.m_programNumber = m_LWMParameterVector[oSeamSeriesIndex][oSeamIndex].m_LWMProgramNumber;
                    // TODO was ist mit extendedProductInfo ?
                    simpleIOSelectionData.m_comment = std::to_string(m_oProductNumber) + "-" + std::to_string(oSeamNo);
                    if (m_TCPClientLWM != nullptr)
                    {
                        m_TCPClientLWM->setSendRequestSimpleIOSelection(simpleIOSelectionData);
                        triggerLWMSendRequest();
                    }
                    lastSeamLWMSelection = oSeamNo;
                }

                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Wait for LWM acknowledge of selection (goto 50)\n");
                }
                oStateVariable = 50;
                transition = StateMachineTransition::Direct;
            }
            break;
        case 50: // Ackn auf LWM selection abwarten
            {
                oWaitCounter++;
                if (oWaitCounter >= 100) // Timeout 100ms
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Waiting for LWM acknowledge (selection) has timeout (goto 1020)\n");
                    }
                    oStateVariable = 1020;
                }
                if (m_LWMSimpleIOErrorReceived.load())
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Waiting for LWM acknowledge (selection) brings error (goto 1040)\n");
                    }
                    oStateVariable = 1040;
                }
                if (m_LWMSimpleIOSelectionAckn.load())
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: now a seam is processed (goto 100)\n");
                    }
                    oStateVariable = 100;
                    transition = StateMachineTransition::Direct;
                }
            }
            break;
        case 100: // set InspectStartStop on
            {
                int oSeamIndex = (oSeamNo - 1);
                m_oSM_BurstWasReceived.store(false);
                m_oSM_SeamProcessingEnded.store(false);
                m_LWMSimpleIOTriggerReceived.store(false);
                m_LWMResultsRangesReceived.store(false);
                TriggerInspectStartStop(true, oSeamIndex);
#if LWM_DEVICE_TRIGGER_SIMULATION
                if (searchSeamShadow == false)
                {
                    setCabinetTemperature(true);
                    searchSeamShadow = true;
                }
#endif
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Wait for start of image acquisition (goto 110)\n");
                }
                oStateVariable = 110;
            }
            break;
        case 110: // waiting for start of image acquisition
            {
                oWaitCounter++;
                if (oWaitCounter >= 1000) // Timeout 1 second
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Waiting for start of image acquisition has timeout (goto 1000)\n");
                    }
                    oStateVariable = 1000;
                }
                if (m_oSM_BurstWasReceived.load())
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Wait for end of seam processing (goto 120)\n");
                    }
                    oStateVariable = 120;
                }
            }
            break;
        case 120: // waiting for duration of seam
            {
                oWaitCounter++;
                if ((!m_spotWeldingIsOn && oWaitCounter >= 60000) || (m_spotWeldingIsOn && oWaitCounter >= 300000)) // Timeout 60 second, or 5 minute timeout in case of spot welding.
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Waiting for end of seam processing has timeout (goto 1010)\n");
                    }
                    oStateVariable = 1010;
                    if (m_spotWeldingIsOn)
                    {
                        m_spotWeldingIsOn = false;
                    }
                }
                if (m_oSM_SeamProcessingEnded.load())
                {
                    int oSeamIndex = (oSeamNo - 1);
                    if (m_oSM_SeamProcessingEndSeamNumber.load() == oSeamIndex)
                    {
                        oWaitCounter = 0;
                        if (m_oDebugInfo_SCANMASTER)
                        {
                            wmLog(eDebug, "ScanMaster: Seam processing has ended (goto 130)\n");
                        }
                        oStateVariable = 130;
                        transition = StateMachineTransition::Direct;
                    }
                }
                if ((isCommunicationToLWMDeviceActive()) && (m_LWMParameterVector[m_oSM_SeamseriesNumber.load()][oSeamNo - 1].m_LWMInspectionActive))
                {
                    if (m_LWMSimpleIOTriggerReceived.load())
                    {
#if LWM_DEVICE_TRIGGER_SIMULATION
                        if (searchSeamShadow == true)
                        {
                            setCabinetTemperature(false);
                            searchSeamShadow = false;
                        }
#endif
                        // is there another seam in seam series ?
                        if (oSeamNo < static_cast<int>(m_oSM_SeamCount.load()))
                        {
                            // there is at least one additional seam
                            int oSeamIndex = oSeamNo; // SeamNo is ident to the SeamIndex of the next seam
                            int oSeamSeriesIndex = m_oSM_SeamseriesNumber.load();
                            if ((m_LWMParameterVector[m_oSM_SeamseriesNumber.load()][oSeamIndex].m_LWMInspectionActive))
                            {
                                // seam is with LWM inspection
                                if (lastSeamLWMSelection < (oSeamNo + 1))
                                {
                                    // LWM isn't selected yet
                                    m_LWMSimpleIOSelectionAckn.store(false);
                                    m_LWMSimpleIOErrorReceived.store(false);

                                    SimpleIOSelectionDataType simpleIOSelectionData{};
                                    simpleIOSelectionData.m_requestAcknowledge = true;
                                    simpleIOSelectionData.m_systemActivated = true;
                                    simpleIOSelectionData.m_programNumber = m_LWMParameterVector[oSeamSeriesIndex][oSeamIndex].m_LWMProgramNumber;
                                    // TODO was ist mit extendedProductInfo ?
                                    simpleIOSelectionData.m_comment = std::to_string(m_oProductNumber) + "-" + std::to_string(oSeamIndex + 1);
                                    if (m_TCPClientLWM != nullptr)
                                    {
                                        m_TCPClientLWM->setSendRequestSimpleIOSelection(simpleIOSelectionData);
                                        triggerLWMSendRequest();
                                    }
                                    lastSeamLWMSelection = oSeamNo + 1;
                                }
                            }
                        }
                    }
                }
            }
            break;
        case 130: // set InspectStartStop off
            {
                int oSeamIndex = (oSeamNo - 1);
                TriggerInspectStartStop(false, oSeamIndex);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Seam end was internally signalled, wait some milliseconds (goto 140)\n");
                }
                oStateVariable = 140;
            }
            break;
        case 140:
            {
                oWaitCounter = 0;
                if ((isCommunicationToLWMDeviceActive()) && (m_LWMParameterVector[m_oSM_SeamseriesNumber.load()][oSeamNo - 1].m_LWMInspectionActive))
                {
                    if (!m_LWMSimpleIOTriggerReceived.load()) // there is no trigger telegram arrived
                    {
                        wmLogTr(eError, "QnxMsg.VI.ParaWrong1LWM", "There is a problem with the LWM parameterization\n");
                        wmLogTr(eError, "QnxMsg.VI.ParaWrong2LWM", "LWM has not set a trigger\n");
                        SM_SendNOKResultAtTimeout(LWMStandardResult, LWMStandardResult);
                        if (m_TCPClientLWM != nullptr)
                        {
                            m_TCPClientLWM->setSendRequestSimpleIOStopp();
                            triggerLWMSendRequest();
                        }
                    }
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Waiting for result telegram of LWM (goto 150)\n");
                    }
                    oStateVariable = 150;
                }
                else
                {
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Time after seam end has expired (goto 160)\n");
                    }
                    oStateVariable = 160;
                }
                transition = StateMachineTransition::Direct;
            }
            break;
        case 150:
            {
                oWaitCounter++;
                if (oWaitCounter >= 200) // Timeout 200 millisecond
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Waiting for result telegram of LWM has timeout (goto 1030)\n");
                    }
                    oStateVariable = 1030;
                }
                if (m_LWMResultsRangesReceived.load())
                {
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Received result telegram of LWM (goto 160)\n");
                    }
                    oStateVariable = 160;
                    transition = StateMachineTransition::Direct;
                }
            }
            break;
        case 160: // goto start state or to next seam
            {
                if (oSeamNo < static_cast<int>(m_oSM_SeamCount.load())) // next seam must be triggered
                {
                    ++oSeamNo;
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: Next seam in same step must be processed (goto 10)\n");
                    }
                    oStateVariable = 10;
                    transition = StateMachineTransition::Direct;
                }
                else // all seams are triggered, goto start
                {
                    BoolSender(false, &m_senderSM_ProcessingActive);
                    oWaitCounter = 0;
                    if (m_oDebugInfo_SCANMASTER)
                    {
                        wmLog(eDebug, "ScanMaster: All steps and seams are processed, release processing and wait for new sequence (goto 0)\n");
                    }
                    oStateVariable = 0;
                }
            }
            break;
        case 1000: // Timeout in state machine: waiting for start of image acquisition
            {
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout", "There is a timeout in ScanMaster state machine\n");
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout3", "Waiting for start of image acquisition has timeout\n");
                m_oSM_TimeoutOccured.store(true);
                int oSeamIndex = (oSeamNo - 1);
                SM_SendNOKResultAtTimeout(CoordPosition, NoResultsError);
                TriggerInspectStartStop(false, oSeamIndex);
                BoolSender(false, &m_senderSM_ProcessingActive);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Processing of sequence is canceled, release processing and wait for new sequence (goto 0)\n");
                }
                oStateVariable = 0;
            }
            break;
        case 1010: // Timeout in state machine: waiting for end of seam processing
            {
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout", "There is a timeout in ScanMaster state machine\n");
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout4", "Waiting for end of seam processing has timeout\n");
                m_oSM_TimeoutOccured.store(true);
                int oSeamIndex = (oSeamNo - 1);
                SM_SendNOKResultAtTimeout(CoordPosition, NoResultsError);
                TriggerInspectStartStop(false, oSeamIndex);
                BoolSender(false, &m_senderSM_ProcessingActive);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Processing of sequence is canceled, release processing and wait for new sequence (goto 0)\n");
                }
                oStateVariable = 0;
            }
            break;
        case 1020: // Timeout in state machine: waiting for LWM acknowledge (selection)
            {
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout", "There is a timeout in ScanMaster state machine\n");
                wmLogTr(eError, "QnxMsg.VI.LWMTimeoutSelect", "Waiting for acknowledge of LWM (selection) has timeout\n");
                m_oSM_TimeoutOccured.store(true);
                int oSeamIndex = (oSeamNo - 1);
                SM_SendNOKResultAtTimeout(LWMStandardResult, LWMStandardResult);
                TriggerInspectStartStop(false, oSeamIndex);
                BoolSender(false, &m_senderSM_ProcessingActive);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Processing of sequence is canceled, release processing and wait for new sequence (goto 0)\n");
                }
                oStateVariable = 0;
            }
            break;
        case 1030: // Timeout in state machine: waiting for LWM result
            {
                wmLogTr(eError, "QnxMsg.VI.SMStateTimeout", "There is a timeout in ScanMaster state machine\n");
                wmLogTr(eError, "QnxMsg.VI.LWMTimeoutResult", "Waiting for result of LWM has timeout\n");
                m_oSM_TimeoutOccured.store(true);
                int oSeamIndex = (oSeamNo - 1);
                SM_SendNOKResultAtTimeout(LWMStandardResult, LWMStandardResult);
                TriggerInspectStartStop(false, oSeamIndex);
                BoolSender(false, &m_senderSM_ProcessingActive);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Processing of sequence is canceled, release processing and wait for new sequence (goto 0)\n");
                }
                oStateVariable = 0;
            }
            break;
        case 1040: // waiting for LWM acknowledge (selection) brings an error
            {
                wmLogTr(eError, "QnxMsg.VI.LWMResultError1", "There is an error in ScanMaster state machine\n");
                wmLogTr(eError, "QnxMsg.VI.LWMResultError2", "Acknowledge of LWM (selection) brings an error\n");
                m_oSM_TimeoutOccured.store(true);
                int oSeamIndex = (oSeamNo - 1);
                SM_SendNOKResultAtTimeout(LWMStandardResult, LWMStandardResult);
                TriggerInspectStartStop(false, oSeamIndex);
                BoolSender(false, &m_senderSM_ProcessingActive);
                oWaitCounter = 0;
                if (m_oDebugInfo_SCANMASTER)
                {
                    wmLog(eDebug, "ScanMaster: Processing of sequence is canceled, release processing and wait for new sequence (goto 0)\n");
                }
                oStateVariable = 0;
            }
            break;
        default:
            {
            }
            break;
    }

    return transition;
}

void VI_InspectionControl::TriggerSM_AcknowledgeStep(bool state)
{
    if (state)
    {
        m_oSM_AcknowledgeStepStatus.store(true);
    }
    else
    {
        m_oSM_AcknowledgeStepStatus.store(false);
    }
}

void VI_InspectionControl::LWMSimpleIOSelectionReceived(void) // Callback from TCPClientLWM
{
    m_LWMSimpleIOSelectionAckn.store(true);
    wmLog(eDebug, "VI_InspectionControl::LWMSimpleIOSelectionReceived\n");
}

void VI_InspectionControl::LWMSimpleIOStoppReceived(void) // Callback from TCPClientLWM
{
    m_LWMSimpleIOStoppAckn.store(true);
    wmLog(eDebug, "VI_InspectionControl::LWMSimpleIOStoppReceived\n");
}

void VI_InspectionControl::LWMSimpleIOTriggerReceived(void) // Callback from TCPClientLWM
{
    m_LWMSimpleIOTriggerReceived.store(true);
    wmLog(eDebug, "VI_InspectionControl::LWMSimpleIOTriggerReceived\n");
#if LWM_DEVICE_TRIGGER_SIMULATION
    if ( !isSCANMASTER_Application() )
    {
        setCabinetTemperature(false);
    }
#endif
}

void VI_InspectionControl::LWMSimpleIOErrorReceived(void) // Callback from TCPClientLWM
{
    wmLog(eDebug, "VI_InspectionControl::LWMSimpleIOErrorReceived\n");

    if (m_TCPClientLWM != nullptr)
    {
        int32_t errorStatus = m_TCPClientLWM->getErrorStatus();
        char helpString[21]{};
        sprintf(helpString, "%02X", errorStatus);
        wmLogTr(eError, "QnxMsg.VI.LWMGeneralError", "LWM device signals error, status is: 0x%s\n", helpString);
    }

    m_LWMSimpleIOErrorReceived.store(true);
}

void VI_InspectionControl::LWMResultsRangesReceived(void) // Callback from TCPClientLWM
{
    wmLog(eDebug, "VI_InspectionControl::LWMResultsRangesReceived\n");

    if (m_TCPClientLWM != nullptr)
    {
        ResultsRangesDataType resultsRanges{};

        m_TCPClientLWM->getResultsRanges(resultsRanges);
        wmLog(eDebug, "Program: %d Result: %d Comment: %s\n", resultsRanges.m_programNumber, resultsRanges.m_overallResult, resultsRanges.m_comment.c_str());

        if (m_resultsProxy)
        {
            GeoDoublearray geoArray{};
            geoArray.ref().getData().push_back(1.0);
            geoArray.ref().getRank().push_back(filter::ValueRankType::eRankMax);
            auto result = interface::ResultDoubleArray
            {
                {},                      // FilterId
                LWMStandardResult,       // ResultType
                LWMStandardResult,       // NioType
                {},                      // ImageContext
                geoArray,                // GeoValue
                {-100000.0, 100000.0},   // Deviation
                false                    // IsNio
            };
            if (resultsRanges.m_overallResult > 0) // LWM has an error detected
            {
                result.setNio(true);
                m_resultsProxy->nio(result);
            }
            else
            {
                m_resultsProxy->result(result);
            }
        }
    }

    m_LWMResultsRangesReceived.store(true);
}

void VI_InspectionControl::LWMConnectionOK(void) // Callback from TCPClientLWM
{
    lwmWatchdogDivider = -20000; // the first watchdog test after reconnect should be delayed longer
    m_LWMConnectionIsOn.store(true);
    wmLog(eDebug, "VI_InspectionControl::LWMConnectionOK\n");
}

void VI_InspectionControl::LWMConnectionNOK(void) // Callback from TCPClientLWM
{
    m_LWMConnectionIsOn.store(false);
    wmLog(eDebug, "VI_InspectionControl::LWMConnectionNOK\n");
}

void VI_InspectionControl::lwmWatchdogRequest(void)
{
    if (!m_oInspectCycleIsOn)
    {
        if (m_LWMConnectionIsOn.load())
        {
            if (m_TCPClientLWM != nullptr)
            {
                lwmWatchdogDivider++;
                if (lwmWatchdogDivider >= 10000) // 10 second
                {
                    m_TCPClientLWM->setSendRequestWatchdog();
                    triggerLWMSendRequest();
                    lwmWatchdogDivider = 0;
                }
            }
        }
    }
}

void VI_InspectionControl::triggerLWMSendRequest(void)
{
    std::lock_guard<std::mutex> lock(m_LWMSendRequestMutex);
    m_LWMSendRequestVariable = true;
    m_LWMSendRequestCondVar.notify_one();
}

void VI_InspectionControl::waitForSimpleIOSelectionAckn(void)
{
    if (m_waitForSimpleIOSelectionAckn.load())
    {
        m_waitForSimpleIOSelectionTimeout++;
        if (m_LWMSimpleIOSelectionAckn.load())
        {
            BoolSender(true, &m_senderInspectionPreStartAckn);
            m_waitForSimpleIOSelectionAckn.store(false);
        }
        if (m_LWMSimpleIOErrorReceived.load())
        {
            BoolSender(true, &m_senderInspectionPreStartAckn);
            wmLogTr(eError, "QnxMsg.VI.LWMResultError2", "Acknowledge of LWM (selection) brings an error\n");
            m_delayedSimpleIOSelectionError = true;
            m_waitForSimpleIOSelectionAckn.store(false);
        }
        if (m_waitForSimpleIOSelectionTimeout > 50) // 50ms
        {
            BoolSender(true, &m_senderInspectionPreStartAckn);
            wmLogTr(eError, "QnxMsg.VI.LWMTimeoutSelect", "Waiting for acknowledge of LWM (selection) has timeout\n");
            m_delayedSimpleIOSelectionError = true;
            m_waitForSimpleIOSelectionAckn.store(false);
        }
    }
}

void VI_InspectionControl::waitForSimpleIOResultsRanges(void)
{
    if (m_waitForSimpleIOResultsRanges.load())
    {
        m_waitForSimpleIOResultsRangesTimeout++;
        if (m_LWMResultsRangesReceived.load())
        {
            BoolSender(true, &m_senderInspectionStartEndAckn);
            m_waitForSimpleIOResultsRanges.store(false);
        }
        if (m_waitForSimpleIOResultsRangesTimeout > 200) // 200ms
        {
            BoolSender(true, &m_senderInspectionStartEndAckn);
            wmLogTr(eError, "QnxMsg.VI.LWMTimeoutResult", "Waiting for result of LWM has timeout\n");
            SM_SendNOKResultAtTimeout(LWMStandardResult, LWMStandardResult); // can be reused in WM mode
            m_waitForSimpleIOResultsRanges.store(false);
        }
    }
}

/***************************************************************************/
/* CyclicTaskThread                                                        */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void *CyclicTaskThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "CyclicTask");
    system::makeThreadRealTime(system::Priority::EtherCATDependencies);
    struct timespec oWakeupTime;
    int retValue;

    auto pDataToCyclicTaskThread = static_cast<struct DataToCyclicTaskThread *>(p_pArg);
    auto pVI_InspectionControl = pDataToCyclicTaskThread->m_pVI_InspectionControl;

    wmLog(eDebug, "CyclicTaskThread is started\n");

    bool oSOUVIS6000_Application = SystemConfiguration::instance().getBool("SOUVIS6000_Application", false);
    bool oSCANMASTER_Application = SystemConfiguration::instance().getBool("SCANMASTER_Application", false);
    bool oSCANMASTER_ThreeStepInterface = SystemConfiguration::instance().getBool("SCANMASTER_ThreeStepInterface", false);
    bool oSCANMASTER_GeneralApplication = SystemConfiguration::instance().getBool("SCANMASTER_GeneralApplication", false);
    bool communicationToLWMDeviceEnable = SystemConfiguration::instance().getBool("Communication_To_LWM_Device_Enable", false);

    usleep(2000 * 1000); // 2 seconds delay before starting to feed interfaces

    wmLog(eDebug, "CyclicTaskThread is active\n");

    clock_gettime(CLOCK_TO_USE, &oWakeupTime);
    oWakeupTime.tv_sec += 1; // start in future
    oWakeupTime.tv_nsec = 0;

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

        if ( oSOUVIS6000_Application )
        {
            pVI_InspectionControl->S6KCyclicTaskFunction();
        }
        if ( oSCANMASTER_Application )
        {
            if ( oSCANMASTER_ThreeStepInterface )
            {
                pVI_InspectionControl->SM_ThreeStepCyclicTaskFunction();
            }
            else if ( oSCANMASTER_GeneralApplication )
            {
                VI_InspectionControl::StateMachineTransition transition;
                do
                {
                    transition = pVI_InspectionControl->SM_GeneralCyclicTaskFunction();
                } while (transition == VI_InspectionControl::StateMachineTransition::Direct);
            }
        }

        if (communicationToLWMDeviceEnable)
        {
            pVI_InspectionControl->lwmWatchdogRequest();
            pVI_InspectionControl->waitForSimpleIOSelectionAckn();
            pVI_InspectionControl->waitForSimpleIOResultsRanges();
        }

        // calculate time for next send
        oWakeupTime.tv_nsec += CYCLE_TIME_NS;
        while(oWakeupTime.tv_nsec >= NSEC_PER_SEC)
        {
            oWakeupTime.tv_nsec -= NSEC_PER_SEC;
            oWakeupTime.tv_sec++;
        }
    }

    return NULL;
}

/***************************************************************************/
/* StartSendSensorDataThread                                               */
/***************************************************************************/

void VI_InspectionControl::StartSendSensorDataThread(unsigned long p_oCycleTimeSendSensorData_ns)
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

    m_oDataToSendSensorDataThread.m_pVI_InspectionControl = this;
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

void VI_InspectionControl::StopSendSensorDataThread(void)
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

void VI_InspectionControl::SendSensorDataFunction(void)
{
//wmLog(eDebug, "VI_InspectionControl::SendSensorDataFunction\n");

    if(isGenPurposeDigIn1Enabled())
    {
        IncomingGenPurposeDigIn1_Single();
    }
    if(isGenPurposeDigInMultipleEnabled())
    {
        IncomingGenPurposeDigIn_Multiple();
    }
    if (isSOUVIS6000_Application())
    {
        if (isSOUVIS6000_CrossSectionMeasurementEnable() && !isSOUVIS6000_CrossSection_Leading_System())
        {
            IncomingS6K_CS_LeadingResults();
        }
    }
}






/***************************************************************************/
/* SendSensorDataThread                                                    */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void *SendSensorDataThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "SendSensorDataThread");
    pthread_detach(pthread_self());
    struct timespec oWakeupTime;
    int retValue;

    auto pDataToSendSensorDataThread = static_cast<struct DataToSendSensorDataThread *>(p_pArg);
    auto pVI_InspectionControl = pDataToSendSensorDataThread->m_pVI_InspectionControl;
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
    pVI_InspectionControl->SendSensorDataFunction();
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
        pVI_InspectionControl->SendSensorDataFunction();
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



} // namespace ethercat
} // namespace precitec

