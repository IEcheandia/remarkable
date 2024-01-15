/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		f.agrawal, HS
 * 	@date		2010
 * 	@brief		Receives and dispatches hardware signals
 */


#ifndef VI_InspectionControl_H_
#define VI_InspectionControl_H_

#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib>
#include <math.h>
#include <sstream>
#include <atomic>
#include <condition_variable>

#include "message/module.h"
// die Basisklasse
#include "module/baseModule.h"

#include "event/inspection.proxy.h"
#include "event/inspectionCmd.proxy.h"
#include "event/sensor.proxy.h"
#include "event/ethercatOutputs.proxy.h"
#include "event/S6K_InfoToProcesses.proxy.h"
#include "event/inspectionToS6k.interface.h"
#include "message/db.proxy.h"
#include "event/results.proxy.h"

#include "event/inspectionOut.interface.h"
#include "event/ethercatInputs.h"

#include "common/triggerContext.h"
#include "common/triggerInterval.h"

#include "message/calibration.interface.h"

#include "DefsHW.h"

#include "viInspectionControl/InspectionControlDefaults.h"
#include "viInspectionControl/SAX_VIConfigParser.h"
#include "viInspectionControl/TCPClientLWM.h"

#include "event/dbNotification.interface.h"
#include "dbNotificationServer.h"

namespace precitec
{

using namespace interface;

namespace ethercat
{

#define MAX_PROXY_PER_DEVICE  6

struct SeamType {
      int  imageCount;
      int  sampleCount;
      unsigned int  seamNumber;
};
struct SeamSerieType {
      std::vector<SeamType> seams;
      unsigned int seamSerieNumber;
};
struct ProductType {
      std::vector<SeamSerieType> seamSeries;
      unsigned int productNumber;
};
struct SequenceType {
      std::vector<ProductType> products;
};

struct SenderWrapper{
	COMMAND_INFORMATION info; ///< COMMAND INFORMATION
};

struct LWMParameterStruct
{
    bool m_LWMInspectionActive;
    int32_t m_LWMProgramNumber;
};

struct S6K_CollectResultDataStruct
{
    int32_t m_oEdgePositionValue;
    int32_t m_oEdgePositionImgNo;
    int64_t m_oEdgePositionPos;
    bool    m_oEdgePositionValid;
    int32_t m_oGapWidthValue;
    int32_t m_oGapWidthImgNo;
    int64_t m_oGapWidthPos;
    bool    m_oGapWidthValid;
    bool    m_oReadyToInsert;
};
#define S6K_COLLECT_LENGTH 10

struct S6K_CS_LeadingResultsStruct
{
    bool m_oValid;
    std::queue<CS_BlockLineType> m_oResults;
    uint32_t m_oProductNo;
    uint32_t m_oBatchID;
    uint16_t m_oSeamNo;
    uint16_t m_oMeasuresPerResult;
};
#define S6K_CS_LEADINGRESULTS_BUFFER_COUNT  50

enum S6K_CommandToSendPosition {eS6K_Command_NoData = -10000, eS6K_Command_ResetVars = -10001, eS6K_CommandLastSend = -10002};
enum S6K_ResultsImageIndex {eS6K_ResultsImage1 = 0, eS6K_ResultsImage2, eS6K_ResultsImage3, eS6K_ResultsImage4, eS6K_ResultsImage5, eS6K_ResultsImage6, 
                            eS6K_ResultsImage7, eS6K_ResultsImage8, eS6K_ResultsImage9, eS6K_ResultsImage10, eS6K_ResultsImage11, eS6K_ResultsImage12, 
                            eS6K_ResultsImage13, eS6K_ResultsImage14, eS6K_ResultsImage15, eS6K_ResultsImage16, eS6K_ResultsImage17, eS6K_ResultsImage18, eS6K_ResultsImageCount};

class VI_InspectionControl;

struct DataToZCollAutoHomingThread
{
	VI_InspectionControl* m_pVI_InspectionControl;
};

struct DataToCyclicTaskThread
{
    VI_InspectionControl* m_pVI_InspectionControl;
};

struct DataToSendSensorDataThread
{
    VI_InspectionControl* m_pVI_InspectionControl;
    unsigned long m_oCycleTimeSendSensorData_ns;
};

/**
 * Das Mapping und die dazugehoerige Ansteuerung der HW
 **/
class VI_InspectionControl : public TInspectionOut<AbstractInterface>{

public:

	/**
	 * Ctor.
	 * Stellt alle noetigen Verbindungen ueber das VMI-Framework her
	 * @param _inspectionProxy InspectionProxy
	 * @param _inspectCmdProxy InspectComdProxy
	 * @return
	 **/
	VI_InspectionControl(TInspection<EventProxy>& _inspectionProxy,
						 TInspectionCmd<EventProxy>& _inspectCmdProxy,
						 TSensor<AbstractInterface> &sensorProxy,
						 TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy,
						 TS6K_InfoToProcesses<EventProxy>& p_rS6K_InfoToProcessesProxy,
						 TDb<MsgProxy>& p_rDbProxy,
                         DbNotificationServer* p_rDBNotificationServer = nullptr
                         );
	virtual ~VI_InspectionControl();

	//send to subscriber


private:
	bool CheckBit(unsigned char data, COMMAND_INFORMATION& info);
	bool CheckBit(unsigned char* data, COMMAND_INFORMATION& info);

	int GetBits(unsigned char data, COMMAND_INFORMATION& info);
	unsigned int GetBits(unsigned char* p_pData, COMMAND_INFORMATION& p_rInfo);

    void GetString(unsigned char* p_pData, COMMAND_INFORMATION& p_rInfo, std::string& p_oStringObject);

	//Send
	bool FindCommandSender(COMMAND_INFORMATION& info );

	//Receive
	void ProxyReceive8Bit(EcatProductIndex productIndex, EcatInstance p_oInstance, unsigned char p_oData);
	void ProxyReceiveGateway(EcatProductIndex productIndex, EcatInstance p_oInstance, short p_oBytes, char* p_pData);
	void ProxyReceiveAnalog(EcatProductIndex productIndex, EcatInstance p_oInstance, EcatChannel p_oChannel, int16_t p_oData);
	void ProxyReceiveAnalogOversamp(EcatProductIndex productIndex, EcatInstance p_oInstance, EcatChannel p_oChannel, uint8_t p_oSize, stdVecINT16 p_oData);

	void IncomingGenPurposeDigIn1_Single(void);
	void IncomingGenPurposeDigIn_Multiple(void);
    void IncomingS6K_CS_LeadingResults(void);
	bool m_oStartHandshakeForResultsReady;
	void HandshakeForResultsReady(void);

    void AutoInspectCycleGeneration(void);

    void sendPositionValueToSimotion(int32_t p_oMsgData);

    uint32_t DetermineSeamCount(uint32_t p_oProductType, uint32_t p_oSeamseries);
    void DetermineLWMParameter(uint32_t p_oProductType);

    void SM_SendNOKResultAtTimeout(const ResultType resultType, const ResultType nioType);

	TInspection<EventProxy>& inspectionProxy;
	TInspectionCmd<EventProxy>& inspectionCmdProxy;
	TSensor<AbstractInterface>    &m_oSensorProxy;
	TEthercatOutputs<EventProxy>& m_rEthercatOutputsProxy;
	TS6K_InfoToProcesses<EventProxy>& m_rS6K_InfoToProcessesProxy;
	TDb<MsgProxy>& m_rDbProxy;

	TriggerContext m_context;
	Poco::FastMutex m_mutex;
	TriggerInterval m_interval;
	long m_triggerDistanceNanoSecs;
   	std::map<Sensor, bool>	m_oSensorIdsEnabled;

	int m_imageNrGenPurposeDigIn1;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGenPurposeDigIn1;

	int m_imageNrGenPurposeDigInMultiple;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesGenPurposeDigInMultiple;

	int m_imageNrS6K_CS_LeadingResults;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesS6K_CS_LeadingResults1;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesS6K_CS_LeadingResults2;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesS6K_CS_LeadingResults3;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesS6K_CS_LeadingResults4;

	std::vector<COMMAND_INFORMATION> m_proxyReceiverList;
	std::vector<COMMAND_INFORMATION> m_senderList;

	SLAVE_PROXY_INFORMATION m_oDig8InProxyInfo[MAX_PROXY_PER_DEVICE];
	SLAVE_PROXY_INFORMATION m_oDig8OutProxyInfo[MAX_PROXY_PER_DEVICE];
	SLAVE_PROXY_INFORMATION m_oGatewayInProxyInfo[MAX_PROXY_PER_DEVICE];
	SLAVE_PROXY_INFORMATION m_oGatewayOutProxyInfo[MAX_PROXY_PER_DEVICE];
	SLAVE_PROXY_INFORMATION m_oAnalogInProxyInfo[MAX_PROXY_PER_DEVICE];
	SLAVE_PROXY_INFORMATION m_oAnalogOversampInProxyInfo[MAX_PROXY_PER_DEVICE];

	bool m_oTriggerEmergencyStop;

	bool m_oInspectCycleIsOn;
	bool m_oInspectInfoIsOn;
	bool m_oFirstInspectInfoSet;
	bool m_oInspectSeamIsOn;
	bool m_oSeamHasNoResults;
	bool m_oInspectionIncomplete;
	std::atomic<bool> m_oWaitingForCycleAckn;
	std::atomic<bool> m_oInspectCycleAck;
	bool m_simulateHW;
	std::atomic_uint m_oCycleAcknTimeoutCounter;

    std::atomic<bool> m_oAutoInspectCycleStarted;
    std::atomic<bool> m_oAutoInspectCycleStopped;
    std::atomic<bool> m_oAutoInspectCycleAckn;
    std::atomic<unsigned int> m_oProductTypeContinuously;
    std::atomic<unsigned int> m_oProductNumberContinuously;
    std::atomic<int> m_oInspectTimeMsContinuously;
    std::atomic<bool> m_oAutoInspectIsFirstCycle;

	pthread_mutex_t m_oProductTypeMutex;
	pthread_mutex_t m_oProductNumberMutex;
	pthread_mutex_t m_oExtendedProductInfoMutex;
	pthread_mutex_t m_oSeamSeriesMutex;
	pthread_mutex_t m_oSeamNrMutex;
	pthread_mutex_t m_oCalibrationTypeMutex;
	pthread_mutex_t m_oAcknResultsReadyFullMutex;
	pthread_mutex_t m_oProductTypeFullMutex;
	pthread_mutex_t m_oProductNumberFullMutex;

	// Send
	SenderWrapper m_senderSystemReady;
	SenderWrapper m_senderInspectCycleAckn;
	SenderWrapper m_senderInspectionPreStartAckn;
	SenderWrapper m_senderInspectionStartEndAckn;
	SenderWrapper m_senderInspectionOK;
	SenderWrapper m_senderSumErrorLatched;
	SenderWrapper m_senderInspectionIncomplete;
	SenderWrapper m_senderSumErrorSeam;
	SenderWrapper m_senderSumErrorSeamSeries;
	SenderWrapper m_senderSystemErrorField;
	SenderWrapper m_senderQualityErrorField;
	SenderWrapper m_senderCalibResultsField;
	SenderWrapper m_senderGenPurposeDigOut1;
	SenderWrapper m_senderProductTypeMirror;
	SenderWrapper m_senderProductNumberMirror;
	SenderWrapper m_senderExtendedProductInfoMirror;
	SenderWrapper m_senderCabinetTemperatureOk;
	SenderWrapper m_senderGenPurposeDigInAckn;

	SenderWrapper m_senderAcknProductNumberFull;
	SenderWrapper m_senderTriggerResultsReadyFull;
	SenderWrapper m_senderSetInspectionOKFull;
	SenderWrapper m_senderSetSumErrorLatchedFull;
	SenderWrapper m_senderSetQualityErrorFieldFull;
	SenderWrapper m_senderSetSystemReadyStatusFull;
	SenderWrapper m_senderSetCabinetTemperatureOkFull;
	SenderWrapper m_senderSetHMSignalsFieldFull;
	SenderWrapper m_senderSetSystemErrorFieldFull;
	SenderWrapper m_senderSetProductTypeFull;
	SenderWrapper m_senderSetProductNumberFull;

	void BoolSender(bool value, SenderWrapper* senderWrapper);
	void FieldSender(uint64_t field, SenderWrapper* senderWrapper);
	void StringSender(const std::string& p_oStringVar, SenderWrapper* senderWrapper);

    uint32_t m_oGatewayDataLength;

	int m_oPositionResults; ///< Buffer for PositionResults out of filter, is sent to Profibus at seam end
	int m_oGenPurposeDigOut1; ///< Buffer for General Purpose 16 Bit Output of image processing, is sent to Profibus on request
	int m_oGenPurposeDigOut2; ///< Buffer for General Purpose 16 Bit Output of image processing, is sent to Profibus on request
	int m_oGenPurposeDigOut3; ///< Buffer for General Purpose 16 Bit Output of image processing, is sent to Profibus on request
	int m_oGenPurposeDigOut4; ///< Buffer for General Purpose 16 Bit Output of image processing, is sent to Profibus on request
	int m_oGenPurposeDigOut5; ///< Buffer for General Purpose 16 Bit Output of image processing, is sent to Profibus on request
	int m_oGenPurposeDigOut6; ///< Buffer for General Purpose 16 Bit Output of image processing, is sent to Profibus on request
	int m_oGenPurposeDigOut7; ///< Buffer for General Purpose 16 Bit Output of image processing, is sent to Profibus on request
	int m_oGenPurposeDigOut8; ///< Buffer for General Purpose 16 Bit Output of image processing, is sent to Profibus on request

	bool m_oGenPurposeDigOut1Enable;
	bool m_oSeamEndDigOut1Enable;
	bool m_oGenPurposeDigOutMultipleEnable;

	bool m_oGenPurposeDigIn1Enable;
	bool m_oGenPurposeDigInMultipleEnable;
	pthread_mutex_t m_oGenPurposeDigIn1Mutex;
	pthread_mutex_t m_oGenPurposeDigInAddressMutex;
	int m_oGenPurposeDigInAddress;
	int m_oGenPurposeDigInValue;
	int m_oGenPurposeDigIn1; ///< General Purpose 16 Bit Input via Fieldbus for Processing in Filter-Graph
	int m_oGenPurposeDigIn2; ///< General Purpose 16 Bit Input via Fieldbus for Processing in Filter-Graph
	int m_oGenPurposeDigIn3; ///< General Purpose 16 Bit Input via Fieldbus for Processing in Filter-Graph
	int m_oGenPurposeDigIn4; ///< General Purpose 16 Bit Input via Fieldbus for Processing in Filter-Graph
	int m_oGenPurposeDigIn5; ///< General Purpose 16 Bit Input via Fieldbus for Processing in Filter-Graph
	int m_oGenPurposeDigIn6; ///< General Purpose 16 Bit Input via Fieldbus for Processing in Filter-Graph
	int m_oGenPurposeDigIn7; ///< General Purpose 16 Bit Input via Fieldbus for Processing in Filter-Graph
	int m_oGenPurposeDigIn8; ///< General Purpose 16 Bit Input via Fieldbus for Processing in Filter-Graph
    std::atomic<int32_t>m_oGenPurposeDigIn1_Single;

	bool m_oProductNumberFromWMEnable;
	bool m_oCabinetTemperatureOkEnable;

    bool m_oContinuouslyModeActive;
	bool m_oFieldbusInterfaceStandard;
	bool m_oFieldbusInterfaceLight;
	bool m_oFieldbusInterfaceFull;
	bool m_oFieldbusExtendedProductInfo;
	bool m_oProductTypeOutOfInterfaceFull;
	bool m_oProductNumberOutOfInterfaceFull;
	bool m_oQualityErrorFieldOnSeamSeries;

	bool m_oHeadMonitorGatewayEnable;
	bool m_oHeadMonitorSendsNotReady;

    bool m_oNIOResultSwitchedOff;

    bool m_oIsSOUVIS6000_Application;
    SOUVIS6000MachineType m_oSOUVIS6000_Machine_Type;
    bool m_oSOUVIS6000_Is_PreInspection;
    bool m_oSOUVIS6000_Is_PostInspection_Top;
    bool m_oSOUVIS6000_Is_PostInspection_Bottom;
    bool m_oSOUVIS6000_Automatic_Seam_No;
    bool m_oSOUVIS6000_CrossSectionMeasurementEnable;
    bool m_oSOUVIS6000_CrossSection_Leading_System;

    bool m_oIsSCANMASTER_Application;
    bool m_oIsSCANMASTER_Application_BootupState;
    bool m_oIsSCANMASTER_ThreeStepInterface;
    bool m_oIsSCANMASTER_GeneralApplication;

    bool m_isCommunicationToLWMDeviceActive{false};

	void IncomingHMSignals(uint8_t value);
	void SendHMSignals();
	SLAVE_PROXY_INFORMATION m_oHMSignalsProxyInfo;
	SLAVE_PROXY_INFORMATION m_oHMOutputProxyInfo;
	SenderWrapper m_senderHMOutput;

	bool m_bGlasNotPresent;
	bool m_bGlasDirty;
	bool m_bTempGlasFail;
	bool m_bTempHeadFail;

   	short m_bGlasNotPresentCounter;
   	short m_bGlasDirtyCounter;
   	short m_bTempGlasFailCounter;
   	short m_bTempHeadFailCounter;

	COMMAND_INFORMATION* m_pChangeToStandardModeInfo;
	COMMAND_INFORMATION* m_pSeamseriesNrInfo;
	COMMAND_INFORMATION* m_pSeamNrInfo;
	COMMAND_INFORMATION* m_pCalibrationTypeInfo;
	COMMAND_INFORMATION* m_pGenPurposeDigInAddressInfo;
	COMMAND_INFORMATION* m_pGenPurposeDigIn1Info;
	COMMAND_INFORMATION* m_pProductTypeInfo;
	COMMAND_INFORMATION* m_pProductNumberInfo;
	COMMAND_INFORMATION* m_pExtendedProductInfoInfo;

	COMMAND_INFORMATION* m_pGetProductTypeFullInfo;
	COMMAND_INFORMATION* m_pGetProductNumberFullInfo;

    pthread_t m_oSendSensorDataThread_ID;
    struct DataToSendSensorDataThread m_oDataToSendSensorDataThread;

    pthread_t m_oCyclicTaskThread_ID;
    struct DataToCyclicTaskThread m_oDataToCyclicTaskThread;

    COMMAND_INFORMATION* m_pS6K_CycleData_Info;
    std::atomic<uint32_t> m_oS6K_CycleDataInput;
    COMMAND_INFORMATION* m_pS6K_SeamNo_Info;
    std::atomic<uint32_t> m_oS6K_SeamNoInput;
    uint32_t m_oS6K_Automatic_Seam_No_Buffer;
    bool m_oS6K_Automatic_Seam_No_TimeoutActive;
    uint32_t m_oS6K_Automatic_Seam_No_TimeoutCounter;
    std::atomic<bool> m_oS6K_SouvisActiveStatus;
    std::atomic<bool> m_oS6K_SouvisInspectionStatus;
    std::atomic<bool> m_oS6K_QuitSystemFaultStatus;
    std::atomic<bool> m_oS6K_MachineReadyStatus;
    COMMAND_INFORMATION* m_pS6K_ProductNumber_Info;
    std::atomic<uint32_t> m_oS6K_ProductNumberInput;
    std::atomic<bool> m_oS6K_AcknQualityDataStatus;
    std::atomic<bool> m_oS6K_CycleDataValidStatus;
    std::atomic<bool> m_oS6K_SeamNoValidStatus;
    std::atomic<bool> m_oS6K_AcknResultDataStatus;
    std::atomic<bool> m_oS6K_MakePicturesStatus;

    std::atomic<uint32_t> m_oS6K_SeamErrorCat1Accu;
    std::atomic<uint32_t> m_oS6K_SeamErrorCat2Accu;

    SenderWrapper m_senderS6K_SystemFault;
    SenderWrapper m_senderS6K_SystemReady;
    SenderWrapper m_senderS6K_FastStopDoubleBlank;
    SenderWrapper m_senderS6K_SeamErrorCat1;
    SenderWrapper m_senderS6K_SeamErrorCat2;
    SenderWrapper m_senderS6K_QualityDataValid;
    SenderWrapper m_senderS6K_AcknCycleData;
    SenderWrapper m_senderS6K_CycleDataMirror;
    SenderWrapper m_senderS6K_SeamNoMirror;
    SenderWrapper m_senderS6K_AcknSeamNo;
    SenderWrapper m_senderS6K_ResultDataValid;
    SenderWrapper m_senderS6K_ResultDataCount;
    SenderWrapper m_senderS6K_ResultsImage[eS6K_ResultsImageCount];

    std::atomic<bool> m_oS6K_RequestResultDataDialog;
    std::atomic<uint32_t> m_oS6K_ResultDataState;
    std::atomic<bool> m_oS6K_ResultDataDialogActive;
    bool m_oS6K_SeamEndTimeoutActive;
    uint32_t m_oS6K_SeamEndTimeoutCounter;
    std::atomic<int32_t> m_oS6K_ImageCountUsed;

    pthread_mutex_t m_oValueToSendMutex;
    uint64_t m_oValueToSend[eS6K_ResultsImageCount];

    struct S6K_CollectResultDataStruct m_oS6K_CollectResultData[S6K_COLLECT_LENGTH];
    int m_oS6K_CollectResultDataWrite;
    int m_oS6K_CollectResultDataRead;

    struct DataToZCollAutoHomingThread m_oDataToZCollAutoHomingThread;
	bool m_oTriggerCalibrationBlocked; ///< blockiert die Ausfuehrung von TriggerCalibration solange AutoHomingThread aktiv

    unsigned int m_oSeamNoInAnalogMode; ///< seam number, used at seam start and only if analog trigger is on

    bool m_oDebugInfo_SOURING;
    bool m_oDebugInfo_SOURING_Extd;

    bool m_LWMInspectionActive{false};
    int32_t m_LWMProgramNumber{0};

    bool m_oS6K_SouvisActiveStatusBuffer;
    uint32_t m_oS6K_MaxSouvisSpeed;
    uint32_t m_oS6K_NumberOfPresentSeams;
    uint32_t m_oS6K_ProductNoFromTCP{0};
    bool m_oS6K_SouvisPresent;
    bool m_oS6K_SouvisSelected;

    std::atomic<uint32_t> m_oSM_StepCount;
    std::atomic<uint32_t> m_oSM_SeamCount;
    std::atomic<bool> m_oSM_StartSeamserieSequence;
    std::atomic<uint32_t> m_oSM_SeamseriesNumber;
    uint32_t m_oSM_TimePerSeam;
    std::atomic<bool> m_oSM_BurstWasReceived;
    std::atomic<bool> m_oSM_SeamProcessingEnded;
    std::atomic<int> m_oSM_SeamProcessingEndSeamNumber;
    MeasureTaskList m_oSM_MeasureTasks;
    std::map<Poco::UUID, std::vector<interface::MeasureTask>> m_MeasureTask_per_Product; //The UUID (key) is the Product ID which the MeasureTask corresponds to
    paramSet_t m_ParamSet;
    std::vector<bool>m_oSM_NIOBuffer;
    std::atomic<bool>m_oSM_TimeoutOccured;

    SenderWrapper m_senderSM_ProcessingActive;
    SenderWrapper m_senderSM_TakeoverStep;
    SenderWrapper m_senderSM_StepField;

    std::atomic<bool> m_oSM_AcknowledgeStepStatus;

    bool m_oDebugInfo_SCANMASTER;

    std::array<struct S6K_CS_LeadingResultsStruct, S6K_CS_LEADINGRESULTS_BUFFER_COUNT> m_oS6K_CS_LeadingResultsBufferArray;
    struct S6K_CS_LeadingResultsStruct *m_pS6K_CS_LeadingResultsInput;
    struct S6K_CS_LeadingResultsStruct *m_pS6K_CS_LeadingResultsOutput;
    int m_oS6K_CS_CurrentLeadingResultsInput;
    int m_oS6K_CS_CurrentLeadingResultsOutput;
    std::atomic<uint16_t> m_oS6K_CS_CurrMeasuresPerResult;

    std::shared_ptr<TInspectionToS6k<AbstractInterface>> m_inspectionToS6k;
    std::shared_ptr<interface::TResults<interface::AbstractInterface>> m_resultsProxy;
    
    bool m_oControlSimulationIsOn;
    bool m_spotWeldingIsOn;

    std::unique_ptr<TCPClientLWM> m_TCPClientLWM{nullptr};
    std::vector<std::vector<LWMParameterStruct>> m_LWMParameterVector{};

    std::condition_variable m_LWMSendRequestCondVar;
    std::mutex m_LWMSendRequestMutex;
    bool m_LWMSendRequestVariable;

    std::atomic<bool> m_LWMSimpleIOSelectionAckn{false};
    std::atomic<bool> m_LWMSimpleIOStoppAckn{false};
    std::atomic<bool> m_LWMSimpleIOTriggerReceived{false};
    std::atomic<bool> m_LWMSimpleIOErrorReceived{false};
    std::atomic<bool> m_LWMResultsRangesReceived{false};
    std::atomic<bool> m_LWMConnectionIsOn{false};
    int lwmWatchdogDivider{0};
    std::atomic<bool> m_waitForSimpleIOSelectionAckn{false};
    int m_waitForSimpleIOSelectionTimeout{0};
    bool m_delayedSimpleIOSelectionError{false};
    std::atomic<bool> m_waitForSimpleIOResultsRanges{false};
    int m_waitForSimpleIOResultsRangesTimeout{0};

public:

	/// Initialisierung, mit HW?
	void Init(bool simulateHW);

    void StartCyclicTaskThread(void);
    void S6KCyclicTaskFunction(void);

    void SM_ThreeStepCyclicTaskFunction(void);
    enum class StateMachineTransition
    {
        WaitForNextCycle,
        Direct
    };
    StateMachineTransition SM_GeneralCyclicTaskFunction(void);

    void StartSendSensorDataThread(unsigned long p_oCycleTimeSendSensorData_ns);
    void StopSendSensorDataThread(void);
    void SendSensorDataFunction(void);
    void lwmWatchdogRequest(void);
    void triggerLWMSendRequest(void);
    void waitForSimpleIOSelectionAckn(void);
    void waitForSimpleIOResultsRanges(void);

	//inspectionOut abstract interface
	/// Setzt den Ausgang fuer das System Ready Signal
	void setSystemReady (bool onoff) override;
	/// Setzt das Bitfeld fuer die Systemfehler
	void setSystemErrorField (int systemErrorField) override;
	/// Setzt den Ausgang fuer den Summen-Qualitaetsfehler
	void setSumErrorLatched (bool onoff) override;

	/// Setzt das Bitfeld fuer die Qualitaetsfehler
	void setQualityErrorField (int qualityErrorField) override;
	/// Setzt den Ausgang fuer das Acknowledge des Zyklusstarts
	void setInspectCycleAckn (bool onoff) override;
	/// Ist der Ausgang fuer das Cycle-Acknowledge gerade gesetzt?
	bool inspectCycleAckn();
	/// Setzt den Ausgang fuer Kalibration beendet
	void setCalibrationFinished (bool result) override;
	/// Setzt das Feld fuer die Kantenposition
	void setPositionResultsField (int positionResultsField);
	/// Setzt einen 16Bit Ausgang auf dem Feldbus
	void setGenPurposeDigOut (OutputID outputNo, int value);
	/// setzt das Feld CabinetTemperatureOk
	virtual void setCabinetTemperature(bool isOk);
	/// Uebergibt die Kantenposition fuer SOUVIS6000 an den InspectionControl
	void setS6K_EdgePosition(int32_t p_oEdgePosition, int32_t p_oImageNumber, int64_t p_oPosition);
	/// Uebergibt die Spaltbreite fuer SOUVIS6000 an den InspectionControl
	void setS6K_GapWidth(int32_t p_oGapWidth, int32_t p_oImageNumber, int64_t p_oPosition);
	/// Uebergibt die Qualitaetsergebnisse fuer SOUVIS6000 an TCPCommunication
	void setS6K_QualityResults(int32_t p_oSeamNo, S6K_QualityData_S1S2 p_oQualityData);
	void setS6K_CS_DataBlock (CS_TCP_MODE p_oSendCmd, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                      uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock);
	void setS6K_CS_MeasuresPerResult (uint16_t p_oMeasuresPerResult);
    void setFastStopDoubleBlank(void);

	void ecatDigitalIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t value);           // Interface EthercatInputs
	void ecatGatewayIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecUINT8 &data);       // Interface EthercatInputs
	void ecatAnalogIn(EcatProductIndex productIndex, EcatInstance instance, uint8_t statusCH1, uint16_t valueCH1, uint8_t statusCH2, uint16_t valueCH2); // Interface EthercatInputs
	void ecatAnalogOversamplingInCH1(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data); // Interface EthercatInputs
	void ecatAnalogOversamplingInCH2(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, const stdVecINT16 &data); // Interface EthercatInputs

	void TriggerContinuously(bool p_oTriggerBit, unsigned int p_oProductType, unsigned int p_oProductNumber); ///< StartStopEndlosbetrieb
	void TriggerAutomatic(bool p_oTriggerBit, unsigned int p_oProductType, unsigned int p_oProductNumber, const std::string& p_rExtendedProductInfo); ///< StartStopAutomaticMode
	void TriggerCalibration(bool p_oTriggerBit, unsigned int p_oCalibrationType); ///< StartStopCalibrationMode
	void TriggerHomingYAxis(bool p_oTriggerBit); ///< TriggerHomingYAxis
	void TriggerInspectInfo(bool p_oTriggerBit, unsigned int p_oSeamseries); ///< InspectInfo
    void TriggerInspectionLWMPreStart(bool triggerBit, unsigned int seamNo);
	void TriggerInspectStartStop(bool p_oTriggerBit, unsigned int p_oSeamNr); ///< StartStopInspect
	void TriggerUnblockLineLaser(bool p_oTriggerBit); ///< UnblockLineLaser
	void TriggerQuitSystemFault(bool p_oTriggerBit); ///< QuitSystemFault
	void TriggerQuitSystemFaultFull(bool p_oTriggerBit); ///< QuitSystemFault
	void TriggerEmergencyStop(bool p_oTriggerBit); ///<TriggerEmergencyStop
	void TriggerCabinetTemperatureOk(bool p_oTriggerBit); ///<TriggerCabinetTemperatureOk
	void TriggerGenPurposeDigInTakeOver(bool p_oTriggerBit, unsigned int p_oGenPurposeDigInAddress, int p_oGenPurposeDigInValue); ///< GenPurposeDigInTakeOver
	void TriggerGenPurposeDigOutTakeOver(bool p_oTriggerBit, unsigned int p_oGenPurposeDigInAddress); ///< GenPurposeDigOutTakeOver
	void TriggerProductNumberFull(bool p_oTriggerBit, unsigned int p_oProductTypeFull, unsigned int p_oProductNumberFull);
	void TriggerSM_AcknowledgeStep(bool state);

	void StartProductTeachInMode(); ///< StartProductTeachInMode
	void AbortProductTeachInMode(); ///< AbortProductTeachInMode
	void SimulateSignalNotReady(int relatedException);
	void SimulateQuitSystemFault();

	bool IsInEmergencyStopState() { return m_oTriggerEmergencyStop; }

	bool isGenPurposeDigOut1Enabled(void) { return m_oGenPurposeDigOut1Enable; }
	bool isSeamEndDigOut1Enabled(void) { return m_oSeamEndDigOut1Enable; }
	bool isGenPurposeDigOutMultipleEnabled(void) { return m_oGenPurposeDigOutMultipleEnable; }
	bool isGenPurposeDigIn1Enabled(void) { return m_oGenPurposeDigIn1Enable; }
	bool isGenPurposeDigInMultipleEnabled(void) { return m_oGenPurposeDigInMultipleEnable; }

	bool isProductNumberFromWMEnabled(void) { return m_oProductNumberFromWMEnable; }
	bool isProductNumberFromUserEnabled(void) { return m_oProductNumberFromUserEnable; }
	bool isCabinetTemperatureOkEnabled(void) { return m_oCabinetTemperatureOkEnable; }

	bool isContinuouslyModeActive(void) { return m_oContinuouslyModeActive; }
	bool isFieldbusInterfaceStandardEnabled(void) { return m_oFieldbusInterfaceStandard; }
	bool isFieldbusInterfaceLightEnabled(void) { return m_oFieldbusInterfaceLight; }
	bool isFieldbusInterfaceFullEnabled(void) { return m_oFieldbusInterfaceFull; }
	bool isFieldbusExtendedProductInfoEnabled(void) { return m_oFieldbusExtendedProductInfo; }
	bool isProductTypeOutOfInterfaceFullEnabled(void) { return m_oProductTypeOutOfInterfaceFull; }
	bool isProductNumberOutOfInterfaceFullEnabled(void) { return m_oProductNumberOutOfInterfaceFull; }
	bool isQualityErrorFieldOnSeamSeries(void) { return m_oQualityErrorFieldOnSeamSeries; }

	bool isHeadMonitorGatewayEnabled(void) { return m_oHeadMonitorGatewayEnable; }
	bool sendsHeadMonitorNotReady(void) { return m_oHeadMonitorSendsNotReady; }

	bool isNIOResultSwitchedOff(void) { return m_oNIOResultSwitchedOff; }

    bool isSOUVIS6000_Application(void) { return m_oIsSOUVIS6000_Application; }
    SOUVIS6000MachineType getSOUVIS6000MachineType(void) { return m_oSOUVIS6000_Machine_Type; }
    bool isSOUVIS6000_Is_PreInspection(void) { return m_oSOUVIS6000_Is_PreInspection; }
    bool isSOUVIS6000_Is_PostInspection_Top(void) { return m_oSOUVIS6000_Is_PostInspection_Top; }
    bool isSOUVIS6000_Is_PostInspection_Bottom(void) { return m_oSOUVIS6000_Is_PostInspection_Bottom; }
    bool isSOUVIS6000_Automatic_Seam_No(void) { return m_oSOUVIS6000_Automatic_Seam_No; }
    bool isSOUVIS6000_CrossSectionMeasurementEnable(void) { return m_oSOUVIS6000_CrossSectionMeasurementEnable; }
    bool isSOUVIS6000_CrossSection_Leading_System(void) { return m_oSOUVIS6000_CrossSection_Leading_System; }

    bool isSCANMASTER_Application(void) { return m_oIsSCANMASTER_Application; }
    bool isSCANMASTER_ThreeStepInterface(void) { return m_oIsSCANMASTER_ThreeStepInterface; }
    bool isSCANMASTER_GeneralApplication(void) { return m_oIsSCANMASTER_GeneralApplication; }

    bool isCommunicationToLWMDeviceActive(void) { return m_isCommunicationToLWMDeviceActive; }

    float getAnalogTriggerLevelVolt() { return m_oAnalogTriggerLevelVolt; }
    void setAnalogTriggerLevelVolt(float p_oTriggerLevelVolt);

    long getTriggerDistNanoSecs() { return m_triggerDistanceNanoSecs; } // used in AutoRunner
    void setTriggerDistNanoSecs(long lTriggerDistNanoSecs); // used in AutoRunner

    bool getLWMInspectionActive(void) { return m_LWMInspectionActive; };
    void setLWMInspectionActive(bool LWMInspectionActive) { m_LWMInspectionActive = LWMInspectionActive; };
    int getLWMProgramNumber(void) { return m_LWMProgramNumber; };
    void setLWMProgramNumber(int LWMProgramNumber) { m_LWMProgramNumber = LWMProgramNumber; };

    void LWMSimpleIOSelectionReceived(void); // Callback from TCPClientLWM
    void LWMSimpleIOStoppReceived(void); // Callback from TCPClientLWM
    void LWMSimpleIOTriggerReceived(void); // Callback from TCPClientLWM
    void LWMSimpleIOErrorReceived(void); // Callback from TCPClientLWM
    void LWMResultsRangesReceived(void); // Callback from TCPClientLWM
    void LWMConnectionOK(void); // Callback from TCPClientLWM
    void LWMConnectionNOK(void); // Callback from TCPClientLWM

    bool getDebugInfo_SOURING(void) { return m_oDebugInfo_SOURING; };
    void setDebugInfo_SOURING(bool p_oDebugInfo_SOURING) { m_oDebugInfo_SOURING = p_oDebugInfo_SOURING; };
    bool getDebugInfo_SOURING_Extd(void) { return m_oDebugInfo_SOURING_Extd; };
    void setDebugInfo_SOURING_Extd(bool p_oDebugInfo_SOURING_Extd) { m_oDebugInfo_SOURING_Extd = p_oDebugInfo_SOURING_Extd; };

    void maxSouvisSpeed(uint32_t p_oSpeed) {m_oS6K_MaxSouvisSpeed = p_oSpeed;}; // Interface S6K_InfoFromProcesses
    void souvisControlBits(bool p_oSouvisPresent, bool p_oSouvisSelected) {m_oS6K_SouvisPresent = p_oSouvisPresent; m_oS6K_SouvisSelected = p_oSouvisSelected; }; // Interface S6K_InfoFromProcesses
    void passS6K_CS_DataBlock_To_Inspect (uint32_t p_oProductNo, uint32_t p_oBatchID, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                          uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock);
    void numberOfPresentSeams(uint32_t p_oSeams) {m_oS6K_NumberOfPresentSeams = p_oSeams;}; // Interface S6K_InfoFromProcesses
    void productNoFromTCP(uint32_t p_oProductNo) {m_oS6K_ProductNoFromTCP = p_oProductNo;}; // Interface S6K_InfoFromProcesses

    bool getDebugInfo_SCANMASTER(void) { return m_oDebugInfo_SCANMASTER; };
    void setDebugInfo_SCANMASTER(bool p_oDebugInfo_SCANMASTER) { m_oDebugInfo_SCANMASTER = p_oDebugInfo_SCANMASTER; };

    void SM_SetEndOfSeamMarker(int p_oSeamNoForResult);
    void SM_SetSpotWelding();

    // interface controlSimulation
    void activateControlSimulation(bool p_oState);
    void genPurposeDigIn(uint8_t p_oAddress, int16_t p_oDigInValue);

	/**
	 * Passt das Senden der Sensordaten an die Bildfrequenz an.
	 * @param ids Sensor IDs
	 * @param context TriggerContext
	 * @param interval Interval
	 */
	void burst(const std::vector<int>& ids, TriggerContext const& context, TriggerInterval const& interval);
	void cancel(int id);
	void cleanBuffer();

	void ResultsReceivedNumber(unsigned long p_oResultCount);
	void ResultsReceivedFlag(bool p_oOK);
    void NIOReceived(int p_oSeamSeries, int p_oSeamNo, int p_oSeamInterval);

    void setInspectionToS6k(const std::shared_ptr<TInspectionToS6k<AbstractInterface>> &inspectionToS6k)
    {
        m_inspectionToS6k = inspectionToS6k;
    }

    void setResultsProxy(const std::shared_ptr<interface::TResults<interface::AbstractInterface>> &proxy)
    {
        m_resultsProxy = proxy;
    }
 
	SAX_VIConfigParser m_oMyVIConfigParser;

	unsigned int m_oProductType; ///< Bauteil- Typ
	unsigned int m_oProductNumber; ///< Seriennummer Bauteil
	std::string m_oExtendedProductInfo; ///< Seriennummer Bauteil
	bool m_oChangeToStandardMode;
	unsigned int m_oSeamNr; ///< Nahtnummer, verwendet bei Nahtstart
	unsigned int m_oSeamNrShadow; ///< Nahtnummer, eingelsen von Eingabegeraet
	unsigned int m_oNoSeams; ///< Anzahl zu bearbeitender Naehte innerhalb einer Nahtfolge (nicht uebergreifend)
	unsigned int m_oNoSeamSeries; ///< Anzahl zu bearbeitender Nahtfolgen innerhalb eines Automatikzyklus
	unsigned int m_oNoProducts; ///< AutoRunner: Anzahl der zu bearbeitenden Produkttypen (startet immer bei 1)
	unsigned int m_oSeamseries; ///< Nahtfolgenummer, verwendet bei Nahtfolgestart
	unsigned int m_oSeamseriesShadow; ///< Nahtfolgenummer, eingelsen von Eingabegeraet
	unsigned int m_oCalibrationType; ///< Art der gewuenschten Kalibration

	std::atomic<bool> m_oSumErrorLatchedAccu; ///< Accumulator for the Sum-Error-Bit during the ongoing inspection cycle
	std::atomic<bool> m_oSumErrorSeamSeriesAccu; ///< Accumulator for the SeamSeries Sum-Error-Bit
	std::atomic<int> m_oSumErrorSeamSeriesCounter; ///< counter for the duration of the SeamSeries Sum-Error-Bit
	std::atomic<bool> m_oSumErrorSeamSeriesIsHigh; ///< flag for the SeamSeries Sum-Error-Bit in continuously mode
	std::atomic<bool> m_oSumErrorSeamAccu; ///< Accumulator for the Seam Sum-Error-Bit
	std::atomic<unsigned int> m_oQualityErrorAccuCycle; ///< Accumulator for the QualityErrorField during the ongoing inspection cycle
	std::atomic<unsigned int> m_oQualityErrorAccuSeamSeries; ///< Accumulator for the QualityErrorField during the ongoing seam series
	std::atomic<unsigned int> m_oQualityErrorAccuSeam; ///< Accumulator for the QualityErrorField during the ongoing seam
	std::atomic<unsigned int> m_oCalibResultsBuffer; ///< Buffer for the Bits, that show the calibration results to the fieldbus
	std::atomic<unsigned int> m_oSystemErrorAccu; ///< Accumulator for the SystemErrorField

	bool m_oAcknResultsReadyFull;
	unsigned int m_oProductTypeFull; ///< Bauteil-Typ ueber Interface Full
	unsigned int m_oProductNumberFull; ///< Seriennummer Bauteil ueber Interface Full

	bool m_oProductNumberFromUserEnable;

    float m_oAnalogTriggerLevelVolt;
    int m_oAnalogTriggerLevelBin;
 
    DbNotificationServer*                   m_dbNotificationServer;
};

} // namespace ethercat
} // namespace precitec

#endif /* VI_InspectionControl_H_ */
