/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		EA
 * 	@date		2016
 * 	@brief		Controls the EtherCAT bus
 */

#ifndef ETHERCATMASTER_H_
#define ETHERCATMASTER_H_

#include <vector>

#include "Poco/Mutex.h"
#include "Poco/Event.h"

#include "Poco/Version.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Text.h"
#include "Poco/DOM/DOMWriter.h"
#include "Poco/XML/XMLWriter.h"
#include "Poco/FileStream.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/NamedNodeMap.h"

#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/NodeIterator.h"
#include "Poco/DOM/NodeFilter.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/SAX/InputSource.h"
#include "Poco/SAX/XMLReader.h"
#include "Poco/Exception.h"
#include "Poco/SAX/SAXParser.h"
#include "Poco/SAX/LexicalHandler.h"
#include "Poco/UUID.h"
#include "Poco/Util/Application.h"

#include "event/ethercatInputs.proxy.h"
#include "event/ethercatInputsToService.proxy.h"

#include "event/ethercatOutputs.h"
#include "event/systemStatus.h"

#include "EtherCATMaster/SAX_VIConfigParser.h"

#if defined __linux__
	#include "common/ethercat.h"
#else
	#include "AtEthercat.h"
#endif

#include "ecrt.h"

#define MAX_SAFE_STACK  (256 * 1024)

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#define MAX_NBR_PER_SLAVE           10

#define MAX_EL2008_COUNT            10

#define MAX_EL4102_COUNT            10

#define MAX_EL4132_COUNT            10

#define MAX_EL5101_COUNT            4
#define MAX_EL5151_COUNT            4

#define MAX_GATEWAY_COUNT           4
#define MAX_GATEWAY_OUTPUT_LENGTH   40

#define GATEWAY1_DATA_LENGHT        20
#define GATEWAY2_DATA_LENGHT        20

#define MAX_ACCELNET_COUNT          4
#define MAX_EPOS4_COUNT             4

#define MAX_FRONTEND_COUNT          4

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

namespace precitec
{
	using namespace interface;
	using Poco::FastMutex;

namespace ethercat
{

///////////////////////////////////////////////////////////
// typedefs
///////////////////////////////////////////////////////////

typedef struct {
    uint16_t m_oAlias; /**< The slaves alias if not equal to 0. */
    uint16_t m_oPosition; /**< Offset of the slave in the ring. */
    uint32_t m_oVendorId; /**< Vendor-ID stored on the slave. */
    uint32_t m_oProductCode; /**< Product-Code stored on the slave. */
    ec_slave_config_t *m_pSlaveConfig;
    ec_slave_config_state_t m_oSlaveState = {};
} wm_slave_data_t;

typedef struct {
	unsigned int m_oNbr_EK1100 = 0;
	unsigned int m_oNbr_EL1018 = 0;
	unsigned int m_oNbr_EL2008 = 0;
	unsigned int m_oNbr_EL3102 = 0;
	unsigned int m_oNbr_EL3702 = 0;
	unsigned int m_oNbr_EL4102 = 0;
	unsigned int m_oNbr_EL4132 = 0;
	unsigned int m_oNbr_EL5101 = 0;
    unsigned int m_numberOfEL5151 = 0;
	unsigned int m_oNbr_GATEWAY = 0;
	unsigned int m_oNbr_ACCELNET = 0;
	unsigned int m_oNbr_EPOS4 = 0;
	unsigned int m_oNbr_COMPAX = 0;
	unsigned int m_oNbr_EK1310 = 0;
	unsigned int m_oNbr_FRONTEND = 0;
} wm_slave_number_t;

class EtherCATMaster; // forward declaration

struct DataToECATDebugDataThread
{
	EtherCATMaster* m_pEtherCATMaster;
};

struct DataToECATCycleTaskThread
{
	EtherCATMaster* m_pEtherCATMaster;
};

struct DataToCheckProcessesThread
{
	EtherCATMaster* m_pEtherCATMaster;
};

class EtherCATMaster
{

public:
	EtherCATMaster(TEthercatInputs<EventProxy>& p_rEthercatInputsProxy, TEthercatInputsToService<EventProxy> &p_rEthercatInputsToServiceProxy);
	virtual ~EtherCATMaster(void);

	SAX_VIConfigParser* getConfigParser() { return &m_oConfigParser; }

    void StartDebugDataThread(void);
	void ECATDebugData(void);

    void StartCycleTaskThread(void);
	void ECATCyclicTask(void);
    void stopThreads();

    void StartCheckProcessesThread(void);
	void InitCheckProcesses(void);
	void CheckProcesses(void);

	void ecatDigitalOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t value, uint8_t mask);           // Interface EthercatOutputs
	void ecatAnalogOut(EcatProductIndex productIndex, EcatInstance instance, EcatChannel channel, uint16_t value);  // Interface EthercatOutputs
	void ecatGatewayOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, stdVecUINT8 data, stdVecUINT8 mask);       // Interface EthercatOutputs
	void ecatEncoderOut(EcatProductIndex productIndex, EcatInstance instance, uint16_t command, uint32_t setCounterValue);  // Interface EthercatOutputs
	void ecatAxisOut(EcatProductIndex productIndex, EcatInstance instance, EcatAxisOutput axisOutput);  // Interface EthercatOutputs
	void ecatRequestSlaveInfo(void);  // Interface EthercatOutputs

    void sendAllData(bool enable)
    {
        // not mutex locked on purpose: in worst case we have a lost update resulting in either
        // one frame being sent too often or too late
        m_sendAllData = enable;
    }
	void ecatFRONTENDOut(EcatProductIndex productIndex, EcatInstance instance, EcatFRONTENDOutput frontendOutput);  // Interface EthercatOutputs

    void resetDebugFileVars(void);

    void operationState(OperationState state); // Interface systemStatus

private:
	int InitializeEtherCATMaster(void);

    template <typename T>
    int SDODownload(uint16_t p_oSlavePosition, uint16_t p_oIndex, uint8_t p_oSubIndex, T p_oValue);
    template <typename T>
    int SDOUpload(uint16_t p_oSlavePosition, uint16_t p_oIndex, uint8_t p_oSubIndex, T& p_oValue);
    int SDOConfigurationEPOS4(int p_oEPOS4Index);

	int StartupEtherCATMaster(void);

    void EPOS4_ReadSDO_U8(ec_sdo_request_t *m_pEPOS4_Request, uint8_t &p_oRegister);
    void EPOS4_WriteSDO_S32(ec_sdo_request_t *m_pEPOS4_Request, int32_t p_oRegister);

	void check_master_state(void);
	void check_slave_config_states(void);
	void check_domain1_state(void);

    void sendSlaveInfo(void);

    char * InsertTimeStamp(char * p_pTimeStampStrg);

    /**
    * Disables cstates to improve latency. For more information see https://access.redhat.com/articles/65410
    *
    * "Modern CPUs are quite aggressive in their desire to transition into power-saving states (called C-states).
    * Unfortunately, transitioning from power saving states back to fully-powered-up-running state takes time and
    * can introduce undesired application delays when powering on components, refilling caches, etc.
    *
    * Real-time applications can avoid these delays by preventing the system from making C-state transitions.
    * [snip]
    * If more fine-grained control of power saving states is desired, a latency sensitive application may use the
    * Power management Quality of Service (PM QOS) interface, /dev/cpu_dma_latency, to prevent the processor from
    * entering deep sleep states and causing unexpected latencies when exiting deep sleep states.
    * Opening /dev/cpu_dma_latency and writing a zero to it will prevent transitions to deep sleep states while the
    * file descriptor is held open. Additionally, writing a zero to it emulates the idle=poll behavior."
    **/
    void disableCStates();

	pthread_t m_oECATDebugDataThread;
    struct DataToECATDebugDataThread m_oDataToECATDebugDataThread;

	pthread_t m_oECATCycleTaskThread;
    struct DataToECATCycleTaskThread m_oDataToECATCycleTaskThread;

    pthread_t m_oCheckProcessesThread;
    struct DataToCheckProcessesThread m_oDataToCheckProcessesThread;

	TEthercatInputs<EventProxy>& m_rEthercatInputsProxy;
	TEthercatInputsToService<EventProxy>& m_rEthercatInputsToServiceProxy;

	SAX_VIConfigParser m_oConfigParser;

    bool m_oFieldbusViaSeparateFieldbusBoard;

	/****************************************************************************/

	// EtherCAT
	ec_master_t *m_pEtherCATMaster = NULL;
	ec_master_state_t m_oMasterState = {};

	ec_domain_t *m_pDomain1 = NULL;
	ec_domain_state_t m_oDomain1State = {};

	// process data
	uint8_t *m_pDomain1_pd = NULL;

    size_t m_oBytesCountInDomain;

    /****************************************************************************/

	std::vector <wm_slave_data_t> m_oSlaveDirectory;
	wm_slave_number_t m_oSlaveNumbers;

	std::vector <ec_pdo_entry_reg_t> m_oPdoEntryRegistration;

	std::vector <EC_T_GET_SLAVE_INFO> m_oSlaveInfo4Service;
    EC_T_GET_SLAVE_INFO m_oSlaveInfoArray[MAX_SLAVE_COUNT];

	/****************************************************************************/

	// offsets for PDO entries
	unsigned int GATEWAY_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int GATEWAY_Output_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL1018_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL2008_Output_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL3102_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL3702_CH1_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL3702_CH2_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL3702_Add_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL4102_Output_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL4132_Output_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL5101_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EL5101_Output_Offset[MAX_NBR_PER_SLAVE];
    unsigned int EL5151InputOffset[MAX_NBR_PER_SLAVE];
    unsigned int EL5151OutputOffset[MAX_NBR_PER_SLAVE];
	unsigned int ACCELNET_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int ACCELNET_Output_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EPOS4_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EPOS4_Output_Offset[MAX_NBR_PER_SLAVE];
	unsigned int EK1310_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int FRONTEND_Input_Offset[MAX_NBR_PER_SLAVE];
	unsigned int FRONTEND_Output_Offset[MAX_NBR_PER_SLAVE];

	/****************************************************************************/

	pthread_mutex_t oEL2008OutLock;
	uint8_t m_oEL2008OutputBuffer[MAX_EL2008_COUNT];

	pthread_mutex_t oEL4102OutLock;
	uint16_t m_oEL4102OutputBuffer[MAX_EL4102_COUNT][2]; // channel 1 and channel 2

	pthread_mutex_t oEL4132OutLock;
	uint16_t m_oEL4132OutputBuffer[MAX_EL4132_COUNT][2]; // channel 1 and channel 2

	pthread_mutex_t oEL5101OutLock;
	uint32_t m_oEL5101OutputBuffer[MAX_EL5101_COUNT][2]; // 16 Bit command and 32 Bit setCounterValue

    pthread_mutex_t m_EL5151OutputLock;
    uint32_t m_EL5151OutputBuffer[MAX_EL5151_COUNT][2]; // 16 Bit command and 32 Bit setCounterValue

	pthread_mutex_t oGatewayOutLock;
	uint8_t m_oGatewayOutputBuffer[MAX_GATEWAY_COUNT][MAX_GATEWAY_OUTPUT_LENGTH];

	pthread_mutex_t oACCELNETOutLock;
	EcatAxisOutput m_oACCELNETOutputBuffer[MAX_ACCELNET_COUNT];

    pthread_mutex_t oEPOS4OutLock;
    EcatAxisOutput m_oEPOS4OutputBuffer[MAX_EPOS4_COUNT];
    ec_slave_config_t *m_pEPOS4SlaveConfig[MAX_EPOS4_COUNT];
    uint16_t m_oEPOS4SlavePosition[MAX_EPOS4_COUNT];
    ec_sdo_request_t *m_pEPOS4_ErrorRegister_Request[MAX_EPOS4_COUNT];
    ec_sdo_request_t *m_pEPOS4_HomeOffset_Request[MAX_EPOS4_COUNT];

	pthread_mutex_t oFRONTENDOutLock;
	EcatFRONTENDOutput m_oFRONTENDOutputBuffer[MAX_FRONTEND_COUNT];

    int m_cStateFd;

    static const int ANZ_BUFFER = 5000;
    int16_t m_oOversampDebugBuffer[ANZ_BUFFER][100];
    int16_t m_oOversampDebugBufferCycCnt[ANZ_BUFFER];
    int64_t m_oOversampDebugBufferTime1[ANZ_BUFFER];
    int64_t m_oOversampDebugBufferTime2[ANZ_BUFFER];
    uint16_t m_oWriteIndex;
    uint16_t m_oReadIndex;
    bool m_oDebugFileFull;

    struct InfoStruct m_oInfoSystemReady;
    struct InfoStruct m_oInfoSystemErrorField;
    struct InfoStruct m_oInfoSystemReadyFull;
    struct InfoStruct m_oInfoSystemErrorFieldFull;

    pthread_mutex_t m_oCheckProcessesMutex;
    int m_oSystemReadyOffset;
    unsigned char m_oSystemReadyMask;
    bool m_oSystemReadyState;
    int m_oSystemReadyOffsetFull;
    unsigned char m_oSystemReadyMaskFull;
    int m_oSystemErrorFieldOffset;
    unsigned char m_oSystemErrorFieldMask1;
    unsigned char m_oSystemErrorFieldMask2;
    int m_oSystemErrorFieldValue;
    int m_oSystemErrorFieldOffsetFull;
    unsigned char m_oSystemErrorFieldMask1Full;
    unsigned char m_oSystemErrorFieldMask2Full;

    bool m_sendAllData = false;

    bool m_firstOperationStateReceived{false};
    bool m_allSlavesOperational{false};
    bool m_ethercatInputsActive{false};

};

} // namespace ethercat

} // namespace precitec

#endif /* ETHERCATMASTER_H_ */

