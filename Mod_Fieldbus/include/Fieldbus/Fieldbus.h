/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Controls the Fieldbus
 */

#ifndef FIELDBUS_H_
#define FIELDBUS_H_

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

#include "Fieldbus/SAX_VIConfigParser.h"

#include "common/systemConfiguration.h"

#include "cifxlinux.h"
#include "rcX_Public.h"

#if defined __linux__
    #include "common/ethercat.h"
#else
    #include "AtEthercat.h"
#endif

#define MAX_SAFE_STACK  (256 * 1024)

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#define MAX_FIELDBUS_BOARDS        2
#define MAX_FIELDBUS_CHANNELS      2
#define MAX_FIELDBUS_DATA_LENGTH   160

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

class Fieldbus; // forward declaration

struct DataToFieldbusCyclicTaskThread
{
    Fieldbus* m_pFieldbus;
};

struct DataToCheckProcessesThread
{
    Fieldbus* m_pFieldbus;
};

class Fieldbus
{

enum FieldbusBoard: int
{
    eFieldbusBoard1 = 0,
    eFieldbusBoard2 = 1,
};

enum FieldbusChannel: int
{
    eFieldbusChannel1 = 0,
    eFieldbusChannel2 = 1,
};

public:
    Fieldbus(TEthercatInputs<EventProxy>& p_rFieldbusInputsProxy, TEthercatInputsToService<EventProxy> &p_rFieldbusInputsToServiceProxy);
    virtual ~Fieldbus(void);

    SAX_VIConfigParser* getConfigParser() { return &m_oConfigParser; }

    void StartCyclicTaskThread(void);
    int StartupFieldbus(void);
    void FieldbusCyclicTask(void);
    void stopThreads();

    void StartCheckProcessesThread(void);
    void InitCheckProcesses(void);
    void CheckProcesses(void);

    void ecatGatewayOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, stdVecUINT8 data, stdVecUINT8 mask);       // Interface EthercatOutputs
    void ecatRequestSlaveInfo(void);  // Interface EthercatOutputs
    void sendAllData(bool enable)  // Interface EthercatOutputs
    {
        // not mutex locked on purpose: in worst case we have a lost update resulting in either
        // one frame being sent too often or too late
        m_sendAllData = enable;
    }

    void operationState(OperationState state); // Interface systemStatus

private:
    bool isSOUVIS6000_Application(void) { return m_oIsSOUVIS6000_Application; }
    SOUVIS6000MachineType getSOUVIS6000MachineType(void) { return m_oSOUVIS6000_Machine_Type; }
    bool isSOUVIS6000_Is_PreInspection(void) { return m_oSOUVIS6000_Is_PreInspection; }
    bool isSOUVIS6000_Is_PostInspection_Top(void) { return m_oSOUVIS6000_Is_PostInspection_Top; }
    bool isSOUVIS6000_Is_PostInspection_Bottom(void) { return m_oSOUVIS6000_Is_PostInspection_Bottom; }
    precitec::interface::FieldbusType getFieldbusType(void) { return m_oFieldbusBoard_TypeOfFieldbus; }
    bool isFieldbusExtendedProductInfoEnabled(void) { return m_oFieldbusExtendedProductInfo; }

    void ShowHilscherError(int32_t p_oErrorValue, char* p_oErrorText, size_t p_oErrorTextLen);

    bool ClearFieldbusConfigDirectory(void);
    bool ClearFieldbusConfigDirectoryFull(void);
    bool CopyFieldbusConfigFile(const std::string& p_oSource, const std::string& p_oDestination);
    void CopyFieldbusConfigurationS6K_SRING(void);
    void CopyFieldbusConfigurationS6K_SSPEED(void);
    int CopyFieldbusConfigurationEthernetIP(void);
    int CopyFieldbusConfigurationEthernetIPFull(void);
    int CopyFieldbusConfigurationProfinet(void);
    int CopyFieldbusConfigurationProfinetFull(void);
    int CopyFieldbusConfigurationEtherCAT(void);
    int CopyFieldbusConfigurationEtherCATFull(void);
    int ParameterizationEthernetIP(int boardNumber);
    int ParameterizationProfinet([[maybe_unused]] int boardNumber);
    int ParameterizationEtherCAT([[maybe_unused]] int boardNumber);

    template <typename T>
    int parameterization(T parameterizationFunction);

    int InitializeFieldbus(void);

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

    bool m_oIsSOUVIS6000_Application;
    SOUVIS6000MachineType m_oSOUVIS6000_Machine_Type;
    bool m_oSOUVIS6000_Is_PreInspection;
    bool m_oSOUVIS6000_Is_PostInspection_Top;
    bool m_oSOUVIS6000_Is_PostInspection_Bottom;

    bool m_oFieldbusViaSeparateFieldbusBoard;
    bool m_oFieldbusTwoSeparateFieldbusBoards;
    FieldbusType m_oFieldbusBoard_TypeOfFieldbus;
    std::string m_oFieldbusBoard_IP_Address[MAX_FIELDBUS_BOARDS];
    std::string m_oFieldbusBoard_Netmask[MAX_FIELDBUS_BOARDS];

    bool m_oFieldbusExtendedProductInfo;

    pthread_t m_oFieldbusCyclicTaskThread;
    struct DataToFieldbusCyclicTaskThread m_oDataToFieldbusCyclicTaskThread;

    pthread_t m_oCheckProcessesThread;
    struct DataToCheckProcessesThread m_oDataToCheckProcessesThread;

    TEthercatInputs<EventProxy>& m_rFieldbusInputsProxy;
    TEthercatInputsToService<EventProxy>& m_rFieldbusInputsToServiceProxy;

    SAX_VIConfigParser m_oConfigParser;

    uint32_t m_oActFieldbusDatalength;

    /****************************************************************************/

    std::vector <EC_T_GET_SLAVE_INFO> m_oSlaveInfo4Service;
    EC_T_GET_SLAVE_INFO m_oSlaveInfoArray[MAX_FIELDBUS_BOARDS];

    /****************************************************************************/

    bool m_oHilscherInitIsReady;

    CIFXHANDLE m_oHilscherDriverHandle;

    CIFXHANDLE m_hilscherChannelHandle[MAX_FIELDBUS_BOARDS][MAX_FIELDBUS_CHANNELS];
    int m_boardsFound{0};
    char m_boardName[MAX_FIELDBUS_BOARDS][20]{};
    int m_channelsFound[MAX_FIELDBUS_BOARDS]{};

    pthread_mutex_t oFieldbusOutLock;
    uint8_t m_oFieldbusOutputBuffer[MAX_FIELDBUS_BOARDS][MAX_FIELDBUS_DATA_LENGTH]; // 160 bytes

    int m_cStateFd;

    /****************************************************************************/

    struct InfoStruct m_oInfoSystemReady;
    struct InfoStruct m_oInfoSystemErrorField;
    struct InfoStruct m_oInfoSystemReadyFull;
    struct InfoStruct m_oInfoSystemErrorFieldFull;
    struct InfoStruct m_oInfoS6KSystemFault;
    struct InfoStruct m_oInfoS6KSystemReady;

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
    int m_oS6KSystemFaultOffset;
    unsigned char m_oS6KSystemFaultMask;
    int m_oS6KSystemReadyOffset;
    unsigned char m_oS6KSystemReadyMask;

    /****************************************************************************/

    bool m_sendAllData = false;

    bool m_firstOperationStateReceived{false};
    bool m_fieldbusInputsActive{false};
};

} // namespace ethercat

} // namespace precitec

#endif /* FIELDBUS_H_ */

