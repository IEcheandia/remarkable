/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Controls the Fieldbus
 */

#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>
#include <arpa/inet.h>
#include <functional>

#include "Fieldbus/Fieldbus.h"
#include "module/moduleLogger.h"

#include "common/connectionConfiguration.h"
#include "system/realTimeSupport.h"

using Poco::XML::DOMParser;
using Poco::XML::InputSource;
using Poco::XML::Document;
using Poco::XML::NodeIterator;
using Poco::XML::NodeFilter;
using Poco::XML::Node;
using Poco::XML::AutoPtr;
using Poco::Exception;
using Poco::XML::NodeList;
using Poco::XML::NamedNodeMap;
using Poco::XML::SAXParser;
using Poco::XML::XMLReader;
using Poco::XML::LexicalHandler;
using Poco::FastMutex;

using namespace precitec::interface;

// folgende CLOCK... verwenden
#define CLOCK_TO_USE CLOCK_MONOTONIC

// folgendes definiert die Anzahl ns pro Sekunde
#define NSEC_PER_SEC    (1000000000)

// Folgendes definiert eine Zykluszeit von 1ms
#define CYCLE_TIME_NS   (1000000)

// Hilfs-Makros fuer Berechnungen im Nanosekunden-Bereich
#define TIMESPEC2NS(T) ((uint64_t) (T).tv_sec * NSEC_PER_SEC + (T).tv_nsec)
#define DIFF_NS(A, B) (((B).tv_sec - (A).tv_sec) * NSEC_PER_SEC + (B).tv_nsec - (A).tv_nsec)

// Debug Funktionalitaeten Ein- bzw. Aus-Schalten
#define ECAT_DEBUG_OUTPUTS           0
#define ECAT_CYCLE_TIMING_PRINTOUTS  0
#define ECAT_CYCLE_VIA_SERIAL_PORT   0

#define PROCESS_MONITOR_INIT_DEBUG   0
#define PROCESS_MONITOR_DEBUG        0

namespace precitec
{

using namespace interface;

namespace ethercat
{

///////////////////////////////////////////////////////////
// Prototyp fuer Thread Funktions
///////////////////////////////////////////////////////////

// Thread Funktion muss ausserhalb der Klasse sein

void* FieldbusCyclicTaskThread(void *p_pArg);

void* CheckProcessesThread(void *p_pArg);

///////////////////////////////////////////////////////////
// global variables for debugging purposes
///////////////////////////////////////////////////////////

int g_oDebugSerialFd;
int g_oDTR01_flag;
int g_oRTS02_flag;

static bool s_threadsStopped = false;

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

Fieldbus::Fieldbus(TEthercatInputs<EventProxy>& p_rFieldbusInputsProxy, TEthercatInputsToService<EventProxy> &p_rFieldbusInputsToServiceProxy):
                m_rFieldbusInputsProxy(p_rFieldbusInputsProxy),
                m_rFieldbusInputsToServiceProxy(p_rFieldbusInputsToServiceProxy),
                m_oActFieldbusDatalength(20),
                m_oHilscherInitIsReady(false),
                m_oHilscherDriverHandle(nullptr),
                m_cStateFd(-1),
                m_oSystemReadyOffset(-1),
                m_oSystemReadyMask(0x00),
                m_oSystemReadyState(true),
                m_oSystemReadyOffsetFull(-1),
                m_oSystemReadyMaskFull(0x00),
                m_oSystemErrorFieldOffset(-1),
                m_oSystemErrorFieldMask1(0x00),
                m_oSystemErrorFieldMask2(0x00),
                m_oSystemErrorFieldValue(0),
                m_oSystemErrorFieldOffsetFull(-1),
                m_oSystemErrorFieldMask1Full(0x00),
                m_oSystemErrorFieldMask2Full(0x00),
                m_oS6KSystemFaultOffset(-1),
                m_oS6KSystemFaultMask(0x00),
                m_oS6KSystemReadyOffset(-1),
                m_oS6KSystemReadyMask(0x00)
{
    // check if SystemConfig.xml is newly written
    bool tempBool = SystemConfiguration::instance().getBool("File_New_Created", false);
    if (tempBool)
    {
        wmFatal(eDataConsistency, "QnxMsg.Misc.SysConfIsNew1", "System Configuration file is damaged or missing\n");
        wmFatal(eDataConsistency, "QnxMsg.Misc.SysConfIsNew2", "System Configuration is lost and must be set again\n");
        SystemConfiguration::instance().setBool("File_New_Created", false);
    }

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

    // SystemConfig Switches for separate Fieldbus board
    m_oFieldbusViaSeparateFieldbusBoard = SystemConfiguration::instance().getBool("FieldbusViaSeparateFieldbusBoard", false);
    wmLog(eDebug, "m_oFieldbusViaSeparateFieldbusBoard (bool): %d\n", m_oFieldbusViaSeparateFieldbusBoard);
    m_oFieldbusTwoSeparateFieldbusBoards = SystemConfiguration::instance().getBool("FieldbusTwoSeparateFieldbusBoards", false);
    wmLog(eDebug, "m_oFieldbusTwoSeparateFieldbusBoards (bool): %d\n", m_oFieldbusTwoSeparateFieldbusBoards);
    m_oFieldbusBoard_TypeOfFieldbus = static_cast<FieldbusType>(SystemConfiguration::instance().getInt("FieldbusBoard_TypeOfFieldbus", 1) );
    wmLog(eDebug, "m_oFieldbusBoard_TypeOfFieldbus (int): %d\n", m_oFieldbusBoard_TypeOfFieldbus);
    m_oFieldbusBoard_IP_Address[FieldbusBoard::eFieldbusBoard1] = SystemConfiguration::instance().getString("FieldbusBoard_IP_Address", "192.168.10.10");
    wmLog(eDebug, "m_oFieldbusBoard_IP_Address[1]: <%s>\n", m_oFieldbusBoard_IP_Address[FieldbusBoard::eFieldbusBoard1].c_str());
    m_oFieldbusBoard_IP_Address[FieldbusBoard::eFieldbusBoard2] = SystemConfiguration::instance().getString("FieldbusBoard_2_IP_Address", "192.168.10.10");
    wmLog(eDebug, "m_oFieldbusBoard_IP_Address[2]: <%s>\n", m_oFieldbusBoard_IP_Address[FieldbusBoard::eFieldbusBoard2].c_str());
    m_oFieldbusBoard_Netmask[FieldbusBoard::eFieldbusBoard1] = SystemConfiguration::instance().getString("FieldbusBoard_Netmask", "255.255.255.0");
    wmLog(eDebug, "m_oFieldbusBoard_Netmask[1]: <%s>\n", m_oFieldbusBoard_Netmask[FieldbusBoard::eFieldbusBoard1].c_str());
    m_oFieldbusBoard_Netmask[FieldbusBoard::eFieldbusBoard2] = SystemConfiguration::instance().getString("FieldbusBoard_2_Netmask", "255.255.255.0");
    wmLog(eDebug, "m_oFieldbusBoard_Netmask[2]: <%s>\n", m_oFieldbusBoard_Netmask[FieldbusBoard::eFieldbusBoard2].c_str());
    m_oFieldbusExtendedProductInfo = SystemConfiguration::instance().getBool("FieldbusExtendedProductInfo", false);
    wmLog(eDebug, "m_oFieldbusExtendedProductInfo (bool): %d\n", m_oFieldbusExtendedProductInfo);

    for (int i = 0; i < MAX_FIELDBUS_BOARDS; i++)
    {
        for (int j = 0; j < MAX_FIELDBUS_CHANNELS; j++)
        {
            m_hilscherChannelHandle[i][j] = nullptr;
        }
    }

    // if there is no fieldbus board -> do nothing
    if ((!m_oFieldbusViaSeparateFieldbusBoard) && (!m_oFieldbusTwoSeparateFieldbusBoards))
    {
        return;
    }

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

    ///////////////////////////////////////////////////////
    // Inits
    ///////////////////////////////////////////////////////

    int retValue = pthread_mutex_init(&oFieldbusOutLock, nullptr);
    if (retValue != 0)
    {
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "oFieldbusOutLock", strerror(retValue));
    }
    for (int i = 0; i < MAX_FIELDBUS_BOARDS; i++)
    {
        for (int j = 0; j < MAX_FIELDBUS_DATA_LENGTH; j++)
        {
            m_oFieldbusOutputBuffer[i][j] = (uint8_t)0x00;
        }
    }

    retValue = pthread_mutex_init(&m_oCheckProcessesMutex, nullptr);
    if (retValue != 0)
    {
        wmLogTr(eError, "QnxMsg.VI.ECMMutexInitFailed", "Initialization of Mutex %s failed (%s) !\n", "m_oCheckProcessesMutex", strerror(retValue));
    }

    disableCStates();

    ////////////////////////////////////////////////////////////////////////////////
    // Copy appropriate configuration files for Fieldbus board in case of SOUVIS6000
    ////////////////////////////////////////////////////////////////////////////////

    if (isSOUVIS6000_Application())
    {
        m_oFieldbusTwoSeparateFieldbusBoards = false; // in case there is a wrong configuration

        m_oActFieldbusDatalength = 128; // default: 128 bytes datalength
        if (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouRing)
        {
            CopyFieldbusConfigurationS6K_SRING();
            m_oActFieldbusDatalength = 128; // 128 bytes datalength for souvis6000 with SRING/STRAC2
        }
        else if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
        {
            CopyFieldbusConfigurationS6K_SSPEED();
            m_oActFieldbusDatalength = 30; // 30 bytes datalength for souvis6000 with SSPEED, SOU-X, SBLATE
        }
        else
        {
            wmLog(eError, "SOUVIS6000 Machine Type is not supported !\n");
        }
    }
    else
    {
        if (!m_oFieldbusTwoSeparateFieldbusBoards) // there is only one fieldbus board
        {
            m_oActFieldbusDatalength = 20; // default: 20 bytes datalength
            if (getFieldbusType() == eFieldbusEthernetIP)
            {
                m_oActFieldbusDatalength = CopyFieldbusConfigurationEthernetIP();
            }
            else if (getFieldbusType() == eFieldbusProfinet)
            {
                m_oActFieldbusDatalength = CopyFieldbusConfigurationProfinet();
            }
            else if (getFieldbusType() == eFieldbusEtherCAT)
            {
                m_oActFieldbusDatalength = CopyFieldbusConfigurationEtherCAT();
            }
            else
            {
                wmLog(eError, "Fieldbus Type is not supported !\n");
            }
        }
        else // there are 2 fieldbus boards
        {
            m_oActFieldbusDatalength = 20; // default: 20 bytes datalength
            if (getFieldbusType() == eFieldbusEthernetIP)
            {
                m_oActFieldbusDatalength = CopyFieldbusConfigurationEthernetIPFull();
            }
            else if (getFieldbusType() == eFieldbusProfinet)
            {
                m_oActFieldbusDatalength = CopyFieldbusConfigurationProfinetFull();
            }
            else if (getFieldbusType() == eFieldbusEtherCAT)
            {
                m_oActFieldbusDatalength = CopyFieldbusConfigurationEtherCATFull();
            }
            else
            {
                wmLog(eError, "Fieldbus Type is not supported !\n");
            }
        }
    }

    ///////////////////////////////////////////////////////
    // startup fieldbus
    ///////////////////////////////////////////////////////

    int retVal = InitializeFieldbus();
    if (retVal == 0)
    {
        wmLogTr(eInfo, "QnxMsg.VI.FBInitOk", "Initialization of Fieldbus device was successful\n");
        m_oHilscherInitIsReady = true;
    }
    else
    {
        wmFatal(eBusSystem, "QnxMsg.VI.FBInitNotOk", "Initialization of Fieldbus device was NOT successful\n");
    }

#if ECAT_CYCLE_VIA_SERIAL_PORT
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

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

Fieldbus::~Fieldbus(void)
{
    // if there is no fieldbus board -> do nothing
    if ((!m_oFieldbusViaSeparateFieldbusBoard) && (!m_oFieldbusTwoSeparateFieldbusBoards))
    {
        return;
    }

    if (pthread_cancel(m_oCheckProcessesThread) != 0)
    {
        wmLog(eDebug, "was not able to abort thread\n");
    }

    if (pthread_cancel(m_oFieldbusCyclicTaskThread) != 0)
    {
        wmLog(eDebug, "was not able to abort thread\n");
    }

    for (int i = 0; i < MAX_FIELDBUS_BOARDS; i++)
    {
        for (int j = 0; j < MAX_FIELDBUS_CHANNELS; j++)
        {
            if (m_hilscherChannelHandle[i][j] != nullptr)
            {
                xChannelClose(m_hilscherChannelHandle[i][j]);
            }
        }
    }

    if (m_oHilscherDriverHandle != nullptr)
    {
        xDriverClose(m_oHilscherDriverHandle);
    }

    cifXDriverDeinit();

#if ECAT_CYCLE_VIA_SERIAL_PORT
    close(g_oDebugSerialFd);
#endif

    pthread_mutex_destroy(&m_oCheckProcessesMutex);
    pthread_mutex_destroy(&oFieldbusOutLock);

    // enable cstates
    if (m_cStateFd != -1)
    {
        close(m_cStateFd);
    }
}

///////////////////////////////////////////////////////////
// private members
///////////////////////////////////////////////////////////

void Fieldbus::disableCStates()
{
    struct stat s;
    if (stat("/dev/cpu_dma_latency", &s) == -1) {
        wmLog(eDebug, "ERROR: Could not stat /dev/cpu_dma_latency: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.ChangeLatFailed", "Change of latency failed ! %s\n", "(001)");
        return;
    }

    m_cStateFd = open("/dev/cpu_dma_latency", O_RDWR);
    if (m_cStateFd == -1) {
        wmLog(eDebug, "ERROR: Failed to open /dev/cpu_dma_latency: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.ChangeLatFailed", "Change of latency failed ! %s\n", "(002)");
        return;
    }

    const int32_t target = 0;
    if (write(m_cStateFd, &target, sizeof(target)) < 1) {
        wmLog(eDebug, "ERROR: Failed writing to /dev/cpu_dma_latency: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.ChangeLatFailed", "Change of latency failed ! %s\n", "(003)");
        close(m_cStateFd);
        return;
    }
    wmLog(eDebug, "Adjusted /dev/cpu_dma_latency to have lower latency\n");
}

void Fieldbus::stopThreads()
{
    m_fieldbusInputsActive = false;
    m_oSystemReadyState = false;
    for (int i = 0; i < MAX_FIELDBUS_BOARDS; i++)
    {
        for (int j = 0; j < MAX_FIELDBUS_DATA_LENGTH; j++)
        {
            m_oFieldbusOutputBuffer[i][j] = (uint8_t)0x00;
        }
    }
    usleep(1000 * 1000);
    s_threadsStopped = true;
    pthread_join(m_oCheckProcessesThread, nullptr);
    pthread_join(m_oFieldbusCyclicTaskThread, nullptr);
}

void Fieldbus::ShowHilscherError(int32_t p_oErrorValue, char* p_oErrorText, size_t p_oErrorTextLen)
{
    if(p_oErrorValue != CIFX_NO_ERROR)
    {
        xDriverGetErrorDescription( p_oErrorValue,  p_oErrorText, p_oErrorTextLen);
    }
    else
    {
        strcpy(p_oErrorText, "");
    }
}

bool Fieldbus::ClearFieldbusConfigDirectory(void)
{
    bool oReturnValue = true;

    try
    {
        Poco::File oConfigDirectory {"/opt/cifx/deviceconfig/FW/channel0"};
        std::vector<std::string> oConfigFiles;
        oConfigDirectory.list(oConfigFiles);
        for (auto it = oConfigFiles.begin(); it != oConfigFiles.end(); ++it)
        {
            Poco::File oTempFile {oConfigDirectory.path() + "/" + *it};
            oTempFile.remove();
        }
    }
    catch (Poco::Exception& exc)
    {
        wmLog(eDebug, "%s\n", exc.message());
        wmLogTr(eError, "QnxMsg.VI.FBCopyConfFail", "unable to copy fieldbus configuration %s\n", "(001)");
        oReturnValue = false;
    }

    return oReturnValue;
}

bool Fieldbus::ClearFieldbusConfigDirectoryFull(void)
{
    bool oReturnValue = true;

    try
    {
        Poco::File oConfigDirectory {"/opt/cifx/deviceconfig/Slot_1/channel0"};
        std::vector<std::string> oConfigFiles;
        oConfigDirectory.list(oConfigFiles);
        for (auto it = oConfigFiles.begin(); it != oConfigFiles.end(); ++it)
        {
            Poco::File oTempFile {oConfigDirectory.path() + "/" + *it};
            oTempFile.remove();
        }
    }
    catch (Poco::Exception& exc)
    {
        wmLog(eDebug, "%s\n", exc.message());
        wmLogTr(eError, "QnxMsg.VI.FBCopyConfFail", "unable to copy fieldbus configuration %s\n", "(001)");
        oReturnValue = false;
        return oReturnValue;
    }

    try
    {
        Poco::File oConfigDirectory {"/opt/cifx/deviceconfig/Slot_2/channel0"};
        std::vector<std::string> oConfigFiles;
        oConfigDirectory.list(oConfigFiles);
        for (auto it = oConfigFiles.begin(); it != oConfigFiles.end(); ++it)
        {
            Poco::File oTempFile {oConfigDirectory.path() + "/" + *it};
            oTempFile.remove();
        }
    }
    catch (Poco::Exception& exc)
    {
        wmLog(eDebug, "%s\n", exc.message());
        wmLogTr(eError, "QnxMsg.VI.FBCopyConfFail", "unable to copy fieldbus configuration %s\n", "(002)");
        oReturnValue = false;
        return oReturnValue;
    }

    return oReturnValue;
}

bool Fieldbus::CopyFieldbusConfigFile(const std::string& p_oSource, const std::string& p_oDestination)
{
    bool oReturnValue = true;

    try
    {
        Poco::File oSourceFile {p_oSource};
        if (oSourceFile.exists())
        {
            oSourceFile.copyTo(p_oDestination);
            Poco::File oDestinationFile {p_oDestination};
            if (!oDestinationFile.exists())
            {
                wmLogTr(eError, "QnxMsg.VI.FBCopyConfFail", "unable to copy fieldbus configuration %s\n", "(101)");
                oReturnValue = false;
            }
        }
        else
        {
            wmLogTr(eError, "QnxMsg.VI.FBCopyConfFail", "unable to copy fieldbus configuration %s\n", "(102)");
            oReturnValue = false;
        }
    }
    catch (Poco::Exception& exc)
    {
        wmLog(eDebug, "%s\n", exc.message());
        wmLogTr(eError, "QnxMsg.VI.FBCopyConfFail", "unable to copy fieldbus configuration %s\n", "(103)");
        oReturnValue = false;
    }

    return oReturnValue;
}

void Fieldbus::CopyFieldbusConfigurationS6K_SRING(void)
{
    // clear directory with fieldbus configuration
    if (!ClearFieldbusConfigDirectory())
    {
        wmLog(eDebug, "clearing /opt/cifx/deviceconfig/FW/channel0 failed\n");
    }

    if (isSOUVIS6000_Is_PreInspection())
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/SRING_souvis1/cifxpns.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SRING_souvis1/SRING_souvis1.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying SRING_souvis1.nxd failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SRING_souvis1/SRING_souvis1_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying SRING_souvis1_nwid.nxd failed\n");
        }
    }
    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/SRING_souvis2u/cifxpns.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SRING_souvis2u/SRING_souvis2u.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying SRING_souvis2u.nxd failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SRING_souvis2u/SRING_souvis2u_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying SRING_souvis2u_nwid.nxd failed\n");
        }
    }
    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/SRING_souvis2l/cifxpns.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SRING_souvis2l/SRING_souvis2l.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying SRING_souvis2l.nxd failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SRING_souvis2l/SRING_souvis2l_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying SRING_souvis2l_nwid.nxd failed\n");
        }
    }
}

void Fieldbus::CopyFieldbusConfigurationS6K_SSPEED(void)
{
    // clear directory with fieldbus configuration
    if (!ClearFieldbusConfigDirectory())
    {
        wmLog(eDebug, "clearing /opt/cifx/deviceconfig/FW/channel0 failed\n");
    }

    if (isSOUVIS6000_Is_PreInspection())
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/SSPEED_souvis1/cifxpns.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SSPEED_souvis1/SSPEED_souvis1.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying SSPEED_souvis1.nxd failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SSPEED_souvis1/SSPEED_souvis1_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying SSPEED_souvis1_nwid.nxd failed\n");
        }
    }
    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/SSPEED_souvis2u/cifxpns.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SSPEED_souvis2u/SSPEED_souvis2u.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying SSPEED_souvis2u.nxd failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SSPEED_souvis2u/SSPEED_souvis2u_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying SSPEED_souvis2u_nwid.nxd failed\n");
        }
    }
    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/SSPEED_souvis2l/cifxpns.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SSPEED_souvis2l/SSPEED_souvis2l.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying SSPEED_souvis2l.nxd failed\n");
        }

        if (!CopyFieldbusConfigFile("/opt/cifx/SSPEED_souvis2l/SSPEED_souvis2l_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying SSPEED_souvis2l_nwid.nxd failed\n");
        }
    }
}

int Fieldbus::CopyFieldbusConfigurationEthernetIP(void)
{
    int oDataLength = 20;

    // clear directory with fieldbus configuration
    if (!ClearFieldbusConfigDirectory())
    {
        wmLog(eDebug, "clearing /opt/cifx/deviceconfig/FW/channel0 failed\n");
    }

    if (isFieldbusExtendedProductInfoEnabled())
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_128B/cifxeis.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxeis.nxf"))
        {
            wmLog(eDebug, "copying cifxeis.nxf failed\n");
        }

        // copy first configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_128B/EIPSlave_128B_config.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying EIPSlave_128B_config.nxd failed\n");
        }

        // copy second configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_128B/EIPSlave_128B_config_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying EIPSlave_128B_config_nwid.nxd failed\n");
        }

        oDataLength = 128;
    }
    else
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_20B/cifxeis.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxeis.nxf"))
        {
            wmLog(eDebug, "copying cifxeis.nxf failed\n");
        }

        // copy first configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_20B/EIPSlave_20B_config.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying EIPSlave_20B_config.nxd failed\n");
        }

        // copy second configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_20B/EIPSlave_20B_config_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying EIPSlave_20B_config_nwid.nxd failed\n");
        }

        oDataLength = 20;
    }

    return oDataLength;
}

int Fieldbus::CopyFieldbusConfigurationEthernetIPFull(void)
{
    int oDataLength = 20;

    // clear directory with fieldbus configuration
    if (!ClearFieldbusConfigDirectoryFull())
    {
        wmLog(eDebug, "clearing /opt/cifx/deviceconfig/Slot_x/channel0 failed\n");
    }

    if (isFieldbusExtendedProductInfoEnabled())
    {
        wmLogTr(eError, "QnxMsg.VI.FBNoExtdInfo", "Extended product info is not supported with two fieldbus boards\n");
    }
    else
    {
        // copy firmware file board 1
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_20B/cifxeis.nxf", "/opt/cifx/deviceconfig/Slot_1/channel0/cifxeis.nxf"))
        {
            wmLog(eDebug, "copying cifxeis.nxf failed\n");
        }
        // copy first configuration file board 1
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_20B/EIPSlave_20B_config.nxd", "/opt/cifx/deviceconfig/Slot_1/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying EIPSlave_20B_config.nxd failed\n");
        }
        // copy second configuration file board 1
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_20B/EIPSlave_20B_config_nwid.nxd", "/opt/cifx/deviceconfig/Slot_1/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying EIPSlave_20B_config_nwid.nxd failed\n");
        }

        // copy firmware file board 2
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_20B_Full/cifxeis.nxf", "/opt/cifx/deviceconfig/Slot_2/channel0/cifxeis.nxf"))
        {
            wmLog(eDebug, "copying cifxeis.nxf failed\n");
        }
        // copy first configuration file board 2
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_20B_Full/EIPSlave_20B_Full_config.nxd", "/opt/cifx/deviceconfig/Slot_2/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying EIPSlave_20B_config.nxd failed\n");
        }
        // copy second configuration file board 2
        if (!CopyFieldbusConfigFile("/opt/cifx/EthernetIP_20B_Full/EIPSlave_20B_Full_config_nwid.nxd", "/opt/cifx/deviceconfig/Slot_2/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying EIPSlave_20B_config_nwid.nxd failed\n");
        }

        oDataLength = 20;
    }

    return oDataLength;
}

int Fieldbus::CopyFieldbusConfigurationProfinet(void)
{
    int oDataLength = 20;

    // clear directory with fieldbus configuration
    if (!ClearFieldbusConfigDirectory())
    {
        wmLog(eDebug, "clearing /opt/cifx/deviceconfig/FW/channel0 failed\n");
    }

    if (isFieldbusExtendedProductInfoEnabled())
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_128B/cifxpns.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }

        // copy first configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_128B/PNDevice_128B_config.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying PNDevice_128B_config.nxd failed\n");
        }

        // copy second configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_128B/PNDevice_128B_config_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying PNDevice_128B_config_nwid.nxd failed\n");
        }

        oDataLength = 128;
    }
    else
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_20B/cifxpns.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }

        // copy first configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_20B/PNDevice_20B_config.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying PNDevice_20B_config.nxd failed\n");
        }

        // copy second configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_20B/PNDevice_20B_config_nwid.nxd", "/opt/cifx/deviceconfig/FW/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying PNDevice_20B_config_nwid.nxd failed\n");
        }

        oDataLength = 20;
    }

    return oDataLength;
}

int Fieldbus::CopyFieldbusConfigurationProfinetFull(void)
{
    int oDataLength = 20;

    // clear directory with fieldbus configuration
    if (!ClearFieldbusConfigDirectoryFull())
    {
        wmLog(eDebug, "clearing /opt/cifx/deviceconfig/Slot_x/channel0 failed\n");
    }

    if (isFieldbusExtendedProductInfoEnabled())
    {
        wmLogTr(eError, "QnxMsg.VI.FBNoExtdInfo", "Extended product info is not supported with two fieldbus boards\n");
    }
    else
    {
        // copy firmware file board 1
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_20B/cifxpns.nxf", "/opt/cifx/deviceconfig/Slot_1/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }
        // copy first configuration file board 1
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_20B/PNDevice_20B_config.nxd", "/opt/cifx/deviceconfig/Slot_1/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying PNDevice_20B_config.nxd failed\n");
        }
        // copy second configuration file board 1
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_20B/PNDevice_20B_config_nwid.nxd", "/opt/cifx/deviceconfig/Slot_1/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying PNDevice_20B_config_nwid.nxd failed\n");
        }

        // copy firmware file board 2
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_20B_Full/cifxpns.nxf", "/opt/cifx/deviceconfig/Slot_2/channel0/cifxpns.nxf"))
        {
            wmLog(eDebug, "copying cifxpns.nxf failed\n");
        }
        // copy first configuration file board 2
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_20B_Full/PNDevice_20B_Full_config.nxd", "/opt/cifx/deviceconfig/Slot_2/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying PNDevice_20B_config.nxd failed\n");
        }
        // copy second configuration file board 2
        if (!CopyFieldbusConfigFile("/opt/cifx/Profinet_20B_Full/PNDevice_20B_Full_config_nwid.nxd", "/opt/cifx/deviceconfig/Slot_2/channel0/nwid.nxd"))
        {
            wmLog(eDebug, "copying PNDevice_20B_config_nwid.nxd failed\n");
        }

        oDataLength = 20;
    }

    return oDataLength;
}

int Fieldbus::CopyFieldbusConfigurationEtherCAT(void)
{
    int oDataLength = 20;

    // clear directory with fieldbus configuration
    if (!ClearFieldbusConfigDirectory())
    {
        wmLog(eDebug, "clearing /opt/cifx/deviceconfig/FW/channel0 failed\n");
    }

    if (isFieldbusExtendedProductInfoEnabled())
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/EtherCAT_128B/cifxecs.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxecs.nxf"))
        {
            wmLog(eDebug, "copying cifxecs.nxf failed\n");
        }

        // copy configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/EtherCAT_128B/ECATSlave_128B_config.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying ECATSlave_128B_config.nxd failed\n");
        }

        oDataLength = 128;
    }
    else
    {
        // copy firmware file
        if (!CopyFieldbusConfigFile("/opt/cifx/EtherCAT_20B/cifxecs.nxf", "/opt/cifx/deviceconfig/FW/channel0/cifxecs.nxf"))
        {
            wmLog(eDebug, "copying cifxecs.nxf failed\n");
        }

        // copy configuration file
        if (!CopyFieldbusConfigFile("/opt/cifx/EtherCAT_20B/ECATSlave_20B_config.nxd", "/opt/cifx/deviceconfig/FW/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying ECATSlave_20B_config.nxd failed\n");
        }

        oDataLength = 20;
    }

    return oDataLength;
}

int Fieldbus::CopyFieldbusConfigurationEtherCATFull(void)
{
    int oDataLength = 20;

    // clear directory with fieldbus configuration
    if (!ClearFieldbusConfigDirectoryFull())
    {
        wmLog(eDebug, "clearing /opt/cifx/deviceconfig/FW/channel0 failed\n");
    }

    if (isFieldbusExtendedProductInfoEnabled())
    {
        wmLogTr(eError, "QnxMsg.VI.FBNoExtdInfo", "Extended product info is not supported with two fieldbus boards\n");
    }
    else
    {
        // copy firmware file board 1
        if (!CopyFieldbusConfigFile("/opt/cifx/EtherCAT_20B/cifxecs.nxf", "/opt/cifx/deviceconfig/Slot_1/channel0/cifxecs.nxf"))
        {
            wmLog(eDebug, "copying cifxecs.nxf failed\n");
        }
        // copy configuration file board 1
        if (!CopyFieldbusConfigFile("/opt/cifx/EtherCAT_20B/ECATSlave_20B_config.nxd", "/opt/cifx/deviceconfig/Slot_1/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying ECATSlave_20B_config.nxd failed\n");
        }

        // copy firmware file board 2
        if (!CopyFieldbusConfigFile("/opt/cifx/EtherCAT_20B/cifxecs.nxf", "/opt/cifx/deviceconfig/Slot_2/channel0/cifxecs.nxf"))
        {
            wmLog(eDebug, "copying cifxecs.nxf failed\n");
        }
        // copy configuration file board 2
        if (!CopyFieldbusConfigFile("/opt/cifx/EtherCAT_20B/ECATSlave_20B_config.nxd", "/opt/cifx/deviceconfig/Slot_2/channel0/config.nxd"))
        {
            wmLog(eDebug, "copying ECATSlave_20B_config.nxd failed\n");
        }

        oDataLength = 20;
    }

    return oDataLength;
}

int Fieldbus::ParameterizationEthernetIP(int boardNumber)
{
    int32_t oReturnValue = CIFX_NO_ERROR;
    uint32_t oState = 0;

    oReturnValue = xChannelHostState(m_hilscherChannelHandle[boardNumber][FieldbusChannel::eFieldbusChannel1], CIFX_HOST_STATE_READY, &oState, 0);
    if(oReturnValue != CIFX_NO_ERROR)
    {
        char oErrorText[200];
        ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
        char oErrorNoText[20];
        sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
        wmLog(eDebug, "Fieldbus Error: %s (%s)\n", oErrorNoText, oErrorText);
        return 1;
    }

    oReturnValue = xChannelHostState(m_hilscherChannelHandle[boardNumber][FieldbusChannel::eFieldbusChannel1], CIFX_HOST_STATE_READ, &oState, 0);
    if(oReturnValue != CIFX_NO_ERROR)
    {
        char oErrorText[200];
        ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
        char oErrorNoText[20];
        sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
        wmLog(eDebug, "Fieldbus Error: %s (%s)\n", oErrorNoText, oErrorText);
        return 1;
    }

    uint32_t oReceiveCount = 0;
    uint32_t oSendCount = 0;
    CIFX_PACKET oSendPacket = {{0}};
    CIFX_PACKET oRecvPacket = {{0}};

    oReturnValue = xChannelGetMBXState(m_hilscherChannelHandle[boardNumber][FieldbusChannel::eFieldbusChannel1], &oReceiveCount, &oSendCount);
    while(oReceiveCount > 0)
    {
        oReturnValue = xChannelGetPacket(m_hilscherChannelHandle[boardNumber][FieldbusChannel::eFieldbusChannel1], sizeof(oRecvPacket), &oRecvPacket, 1000);
        oReceiveCount--;
    }

    oSendPacket.tHeader.ulDest   = 0;
    oSendPacket.tHeader.ulSrc    = 0;
    oSendPacket.tHeader.ulDestId = 0;
    oSendPacket.tHeader.ulSrcId  = 0;
    oSendPacket.tHeader.ulLen    = 0;
    oSendPacket.tHeader.ulId     = 0;
    oSendPacket.tHeader.ulState  = 0;
    oSendPacket.tHeader.ulCmd    = 0;
    oSendPacket.tHeader.ulExt    = 0;
    oSendPacket.tHeader.ulRout   = 0;

    oSendPacket.tHeader.ulDest   = 0x00000020; // HIL_PACKET_DEST_DEFAULT_CHANNEL
    oSendPacket.tHeader.ulLen    = 8 + 12;
    oSendPacket.tHeader.ulCmd    = 0x00002F86; // HIL_SET_FW_PARAMETER_REQ

    uint32_t oIPAddress = inet_network(m_oFieldbusBoard_IP_Address[boardNumber].c_str());
    uint32_t oNetmask = inet_network(m_oFieldbusBoard_Netmask[boardNumber].c_str());
    wmLog(eDebug, "m_oFieldbusBoard_IP_Address[%d]: <%s> <%x>\n", boardNumber, m_oFieldbusBoard_IP_Address[boardNumber].c_str(), oIPAddress);
    wmLog(eDebug, "m_oFieldbusBoard_Netmask[%d]:    <%s> <%x>\n", boardNumber, m_oFieldbusBoard_Netmask[boardNumber].c_str(), oNetmask);

    uint32_t *pAddrPtr = (uint32_t *)&oSendPacket.abData[0];
    *pAddrPtr = 0x3000A001; // PID_EIP_IP_CONFIGURATION
    pAddrPtr++;
    *pAddrPtr = 12;
    pAddrPtr++;
//    *pAddrPtr = 0xC0A80A0A; // IP address: 192.168.10.10
//    *pAddrPtr = 0xC0A80A0C; // IP address: 192.168.10.12
//    *pAddrPtr = 0xC0A80A2D; // IP address: 192.168.10.45
//    *pAddrPtr = 0xC0A80ADF; // IP address: 192.168.10.223
    *pAddrPtr = oIPAddress;
    pAddrPtr++;
    *pAddrPtr = oNetmask;
    pAddrPtr++;
    *pAddrPtr = 0x00000000; // Gateway address
    
    oReturnValue = xChannelPutPacket(m_hilscherChannelHandle[boardNumber][FieldbusChannel::eFieldbusChannel1], &oSendPacket, 1000);
    if(oReturnValue != CIFX_NO_ERROR)
    {
        char oErrorText[200];
        ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
        char oErrorNoText[20];
        sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
        wmLog(eDebug, "Fieldbus Error: %s (%s)\n", oErrorNoText, oErrorText);
        return 1;
    }

    return 0;
}

int Fieldbus::ParameterizationProfinet(int boardNumber)
{
    return 0;
}

int Fieldbus::ParameterizationEtherCAT(int boardNumber)
{
    return 0;
}

template <typename T>
int Fieldbus::parameterization(T parameterizationFunction)
{
    for (int i = 0; i < m_boardsFound; i++)
    {
        usleep(50 * 1000); // 50ms
        auto returnValue = parameterizationFunction(this, i);
        if (returnValue == 1)
        {
            return 1;
        }
    }
    return 0;
}

int Fieldbus::InitializeFieldbus(void)
{
    struct CIFX_LINUX_INIT oHilscherDriverInit;
    oHilscherDriverInit.init_options       = CIFX_DRIVER_INIT_AUTOSCAN;
    oHilscherDriverInit.iCardNumber        = 0;
    oHilscherDriverInit.fEnableCardLocking = 0;
    oHilscherDriverInit.base_dir           = nullptr;
    oHilscherDriverInit.poll_interval      = 0;
    oHilscherDriverInit.poll_priority      = 0;
    oHilscherDriverInit.poll_schedpolicy   = 0;
    oHilscherDriverInit.poll_StackSize     = 0;   /* set to 0 to use default */
    oHilscherDriverInit.trace_level        = 0xFF;
    oHilscherDriverInit.user_card_cnt      = 0;
    oHilscherDriverInit.user_cards         = nullptr;

    wmLog(eDebug, "cifXDriverInit\n");
    int32_t oReturnValue = CIFX_NO_ERROR;
    oReturnValue = cifXDriverInit(&oHilscherDriverInit);
    if(oReturnValue != CIFX_NO_ERROR)
    {
        char oErrorText[200];
        ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
        char oErrorNoText[20];
        sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
        wmLog(eDebug, "Fieldbus Error: %s (%s)\n", oErrorNoText, oErrorText);
        return 1;
    }

    wmLog(eDebug, "xDriverOpen\n");
    oReturnValue = CIFX_NO_ERROR;
    oReturnValue = xDriverOpen(&m_oHilscherDriverHandle);
    if (oReturnValue != CIFX_NO_ERROR)
    {
        char oErrorText[200];
        ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
        char oErrorNoText[20];
        sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
        wmLog(eDebug, "Fieldbus Error: %s (%s)\n", oErrorNoText, oErrorText);
        return 1;
    }

    wmLog(eDebug, "xDriverGetInformation\n");
    DRIVER_INFORMATION oHilscherDriverInfo = {{0}};
    oReturnValue = xDriverGetInformation(m_oHilscherDriverHandle, sizeof(oHilscherDriverInfo), &oHilscherDriverInfo);
    if (oReturnValue != CIFX_NO_ERROR)
    {
        char oErrorText[200];
        ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
        char oErrorNoText[20];
        sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
        wmLog(eDebug, "Fieldbus Error: %s (%s)\n", oErrorNoText, oErrorText);
        return 1;
    }

    wmLog(eDebug, "cifXGetDriverVersion\n");
    char oHilscherDriverVersion[32] = "";
    oReturnValue = cifXGetDriverVersion( sizeof(oHilscherDriverVersion)/sizeof(*oHilscherDriverVersion), oHilscherDriverVersion);
    if (oReturnValue != CIFX_NO_ERROR)
    {
        char oErrorText[200];
        ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
        char oErrorNoText[20];
        sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
        wmLog(eDebug, "Fieldbus Error: %s (%s)\n", oErrorNoText, oErrorText);
        return 1;
    }

    char oHelpStrg1[81];
    sprintf(oHelpStrg1, "%.32s", oHilscherDriverInfo.abDriverVersion);
    wmLog(eDebug, "Driver Version: %s, based on: %s\n", oHilscherDriverVersion, oHelpStrg1);

    unsigned long oBoardNo = 0;
    BOARD_INFORMATION oHilscherBoardInfo = {0};
    unsigned long oChannelNo = 0;

    // Iterate over all boards
    while(xDriverEnumBoards(m_oHilscherDriverHandle, oBoardNo, sizeof(oHilscherBoardInfo), &oHilscherBoardInfo) == CIFX_NO_ERROR)
    {
        wmLog(eDebug, "xDriverEnumBoards\n");
        wmLog(eDebug, "BOARD        : %u, %s\n", oBoardNo, oHilscherBoardInfo.abBoardName);
        if(strlen( (char*)oHilscherBoardInfo.abBoardAlias) != 0)
        {
            wmLog(eDebug, "Alias        : %s\n", oHilscherBoardInfo.abBoardAlias);
        }
        wmLog(eDebug, "DeviceNumber : %u\n",(long unsigned int)oHilscherBoardInfo.tSystemInfo.ulDeviceNumber);
        wmLog(eDebug, "SerialNumber : %u\n",(long unsigned int)oHilscherBoardInfo.tSystemInfo.ulSerialNumber);
        wmLog(eDebug, "Board ID     : %u\n",(long unsigned int)oHilscherBoardInfo.ulBoardID);
        sprintf(oHelpStrg1, "0x%08lX", (long unsigned int)oHilscherBoardInfo.ulSystemError);
        wmLog(eDebug, "System Error : %s\n",oHelpStrg1);
        wmLog(eDebug, "Channels     : %u\n",(long unsigned int)oHilscherBoardInfo.ulChannelCnt);
        wmLog(eDebug, "DPM Size     : %u\n",(long unsigned int)oHilscherBoardInfo.ulDpmTotalSize);
        wmLog(eDebug, "Board switch : %d\n", oHilscherBoardInfo.tSystemInfo.bDevIdNumber);
        if ((oHilscherBoardInfo.tSystemInfo.bDevIdNumber == 0) || (oHilscherBoardInfo.tSystemInfo.bDevIdNumber == 1))
        {
            strcpy(m_boardName[FieldbusBoard::eFieldbusBoard1], oHilscherBoardInfo.abBoardName);
        }
        else
        {
            strcpy(m_boardName[FieldbusBoard::eFieldbusBoard2], oHilscherBoardInfo.abBoardName);
        }

        oChannelNo = 0;
        CHANNEL_INFORMATION oHilscherChannelInfo = {{0}};

        // iterate over all channels on the current board
        while(xDriverEnumChannels(m_oHilscherDriverHandle, oBoardNo, oChannelNo, sizeof(oHilscherChannelInfo), &oHilscherChannelInfo) == CIFX_NO_ERROR)
        {
            wmLog(eDebug, "xDriverEnumChannels\n");
            wmLog(eDebug, "CHANNEL    : %u\n", oChannelNo);
            sprintf(oHelpStrg1, "%s", oHilscherChannelInfo.abFWName);
            wmLog(eDebug, "Firmware   : %s\n", oHelpStrg1);
            wmLog(eDebug, "Version    : %u.%u.%u build %u\n", 
                   oHilscherChannelInfo.usFWMajor,
                   oHilscherChannelInfo.usFWMinor,
                   oHilscherChannelInfo.usFWRevision,
                   oHilscherChannelInfo.usFWBuild);
            sprintf(oHelpStrg1, "%02u/%02u/%04u", 
                   oHilscherChannelInfo.bFWMonth,
                   oHilscherChannelInfo.bFWDay,
                   oHilscherChannelInfo.usFWYear);
            wmLog(eDebug, "Date       : %s\n", oHelpStrg1);

            wmLog(eDebug, "Device Nr. : %u\n",(long unsigned int)oHilscherChannelInfo.ulDeviceNumber);
            wmLog(eDebug, "Serial Nr. : %u\n",(long unsigned int)oHilscherChannelInfo.ulSerialNumber);
            sprintf(oHelpStrg1, "0x%08X", oHilscherChannelInfo.ulNetxFlags);
            wmLog(eDebug, "netX Flags : %s\n", oHelpStrg1);
            sprintf(oHelpStrg1, "0x%08X", oHilscherChannelInfo.ulHostFlags);
            wmLog(eDebug, "Host Flags : %s\n", oHelpStrg1);
            sprintf(oHelpStrg1, "0x%08X", oHilscherChannelInfo.ulHostCOSFlags);
            wmLog(eDebug, "Host COS   : %s\n", oHelpStrg1);
            sprintf(oHelpStrg1, "0x%08X", oHilscherChannelInfo.ulDeviceCOSFlags);
            wmLog(eDebug, "Device COS : %s\n", oHelpStrg1);
            ++oChannelNo;
        }
        if ((oHilscherBoardInfo.tSystemInfo.bDevIdNumber == 0) || (oHilscherBoardInfo.tSystemInfo.bDevIdNumber == 1))
        {
            m_channelsFound[FieldbusBoard::eFieldbusBoard1] = oChannelNo;
        }
        else
        {
            m_channelsFound[FieldbusBoard::eFieldbusBoard2] = oChannelNo;
        }
        ++oBoardNo;
    }
    m_boardsFound = oBoardNo;
    if (m_boardsFound > 2)
    {
        wmLogTr(eError, "QnxMsg.VI.FBTooManyBoard1", "There are more than 2 fieldbus boards used !\n");
        wmLogTr(eError, "QnxMsg.VI.FBTooManyBoard2", "The number of fieldbus boards is limited to 2 !\n");
        m_boardsFound = 2;
    }

    wmLog(eDebug, "****************************************\n");
    for (int i = 0; i < m_boardsFound; i++)
    {
        wmLog(eDebug, "boardName: <%s>\n", m_boardName[i]);

        for (int j = 0; j < m_channelsFound[i]; j++)
        {
            wmLog(eDebug, "xChannelOpen\n");
            oReturnValue = xChannelOpen(m_oHilscherDriverHandle, m_boardName[i], j, &m_hilscherChannelHandle[i][j]);
            if (oReturnValue != CIFX_NO_ERROR)
            {
                char oErrorText[200];
                ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
                char oErrorNoText[20];
                sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
                wmLog(eDebug, "Fieldbus Error: %s (%s)\n", oErrorNoText, oErrorText);
                return 1;
            }
            char helpString[81];
            sprintf(helpString, "%p", m_hilscherChannelHandle[i][j]);
            wmLog(eDebug, "ChannelHandle: %s\n", helpString);

            wmLog(eDebug, "xChannelInfo\n");
            CHANNEL_INFORMATION oHilscherChannelInfo = {{0}};
            oReturnValue = xChannelInfo(m_hilscherChannelHandle[i][j], sizeof(CHANNEL_INFORMATION), &oHilscherChannelInfo);
            if (oReturnValue != CIFX_NO_ERROR)
            {
                char oErrorText[200];
                ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
                char oErrorNoText[20];
                sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
                wmLog(eDebug, "Fieldbus Error: %s (%s)\n", oErrorNoText, oErrorText);
                return 1;
            }

            wmLog(eDebug, "Communication Channel Info:\n");
            wmLog(eDebug, "Device Number    : %u\n",(long unsigned int)oHilscherChannelInfo.ulDeviceNumber);
            wmLog(eDebug, "Serial Number    : %u\n",(long unsigned int)oHilscherChannelInfo.ulSerialNumber);
            sprintf(oHelpStrg1, "%s", oHilscherChannelInfo.abFWName);
            wmLog(eDebug, "Firmware         : %s\n", oHelpStrg1);
            wmLog(eDebug, "FW Version       : %u.%u.%u build %u\n", 
                oHilscherChannelInfo.usFWMajor,
                oHilscherChannelInfo.usFWMinor,
                oHilscherChannelInfo.usFWRevision,
                oHilscherChannelInfo.usFWBuild);
            sprintf(oHelpStrg1, "%02u/%02u/%04u", 
                oHilscherChannelInfo.bFWMonth,
                oHilscherChannelInfo.bFWDay,
                oHilscherChannelInfo.usFWYear);
            wmLog(eDebug, "FW Date          : %s\n", oHelpStrg1);
            wmLog(eDebug, "Mailbox Size     : %u\n",(long unsigned int)oHilscherChannelInfo.ulMailboxSize);
        }
    }

    // now generate the info for the Service-IO-Interface
    m_oSlaveInfo4Service.clear();
    // create SlaveInfo for Service-IO-Interface
    for (int i = 0; i < m_boardsFound; i++)
    {
        EC_T_GET_SLAVE_INFO oLocalSlaveInfo;
        sprintf(oLocalSlaveInfo.abyDeviceName, "Fieldbus (%d)", i + 1);
        oLocalSlaveInfo.wCfgPhyAddress = 1099 + i;
        oLocalSlaveInfo.dwVendorId    = VENDORID_HILSCHER;
        oLocalSlaveInfo.dwProductCode = PRODUCTCODE_FIELDBUS;
        oLocalSlaveInfo.dwPdOffsIn    = 1000 * 8; // Wert ist Bit-Position;
        oLocalSlaveInfo.dwPdSizeIn    = m_oActFieldbusDatalength * 8;
        oLocalSlaveInfo.dwPdOffsOut   = 1000 * 8; // Wert ist Bit-Position;
        oLocalSlaveInfo.dwPdSizeOut   = m_oActFieldbusDatalength * 8;
        m_oSlaveInfo4Service.push_back(oLocalSlaveInfo);
        memcpy(&m_oSlaveInfoArray[i], &m_oSlaveInfo4Service[i], sizeof(EC_T_GET_SLAVE_INFO));
    }

    int returnValue{0};
    switch (getFieldbusType())
    {
        case eFieldbusEthernetIP:
        {
            returnValue = parameterization(std::mem_fn(&Fieldbus::ParameterizationEthernetIP));
            break;
        }
        case eFieldbusProfinet:
        {
            returnValue = parameterization(std::mem_fn(&Fieldbus::ParameterizationProfinet));
            break;
        }
        case eFieldbusEtherCAT:
        {
            returnValue = parameterization(std::mem_fn(&Fieldbus::ParameterizationEtherCAT));
            break;
        }
        default:
        {
            wmLog(eError, "Fieldbus Type is not supported !\n");
            break;
        }
    }

    return returnValue;
}

int Fieldbus::StartupFieldbus(void)
{
    // waiting for initialization of Hilscher fieldbus board
    while(!m_oHilscherInitIsReady)
    {
        usleep(100*1000); // wait for 100ms
    }

    int32_t returnValue{CIFX_NO_ERROR};
    unsigned long state{};
    int loopBoard1{200};
    int loopBoard2{200};

    while ((loopBoard1 > 0) || (loopBoard2 > 0))
    {
        if (loopBoard1 > 0)
        {
            returnValue = xChannelBusState(m_hilscherChannelHandle[FieldbusBoard::eFieldbusBoard1][FieldbusChannel::eFieldbusChannel1], CIFX_BUS_STATE_ON, (TLR_UINT32*) &state, 1000);
            if (returnValue != CIFX_NO_ERROR)
            {
                char errorText[200];
                ShowHilscherError(returnValue, errorText, sizeof(errorText));
                char errorNoText[20];
                sprintf(errorNoText, "0x%X", (unsigned int)returnValue);
                wmLog(eDebug, "Fieldbus Error B1: %s (%s)\n", errorNoText, errorText);
                loopBoard1--;
            }
            else
            {
                loopBoard1 = -1;
            }
        }

        if (m_boardsFound > 1)
        {
            if (loopBoard2 > 0)
            {
                returnValue = xChannelBusState(m_hilscherChannelHandle[FieldbusBoard::eFieldbusBoard2][FieldbusChannel::eFieldbusChannel1], CIFX_BUS_STATE_ON, (TLR_UINT32*) &state, 1000);
                if (returnValue != CIFX_NO_ERROR)
                {
                    char errorText[200];
                    ShowHilscherError(returnValue, errorText, sizeof(errorText));
                    char errorNoText[20];
                    sprintf(errorNoText, "0x%X", (unsigned int)returnValue);
                    wmLog(eDebug, "Fieldbus Error B2: %s (%s)\n", errorNoText, errorText);
                    loopBoard2--;
                }
                else
                {
                    loopBoard2 = -1;
                }
            }
        }
        else
        {
            loopBoard2 = -1;
        }
        usleep(100 * 1000);
    }
    if ((loopBoard1 == -1) && (loopBoard2 == -1))
    {
        wmLogTr(eInfo, "QnxMsg.VI.FBStartOk", "Startup of Fieldbus communication was successful\n");
    }
    else
    {
        wmFatal(eBusSystem, "QnxMsg.VI.FBStartNotOk", "Startup of Fieldbus communication was NOT successful\n");
    }

    return 0;
}

void Fieldbus::StartCyclicTaskThread(void)
{
    // if there is no fieldbus board -> do nothing
    if ((!m_oFieldbusViaSeparateFieldbusBoard) && (!m_oFieldbusTwoSeparateFieldbusBoards))
    {
        return;
    }

    ///////////////////////////////////////////////////////
    // Thread für zyklischen Ablauf starten
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToFieldbusCyclicTaskThread.m_pFieldbus = this;

    if (pthread_create(&m_oFieldbusCyclicTaskThread, &oPthreadAttr, &FieldbusCyclicTaskThread, &m_oDataToFieldbusCyclicTaskThread) != 0)
    {
        wmFatal(eBusSystem, "QnxMsg.VI.ECMCreateThreadFail", "Cannot start thread for cyclic operation\n");
    }
}

void Fieldbus::FieldbusCyclicTask(void)
{
    int32_t oReturnValue = CIFX_NO_ERROR;
    uint8_t oFieldbusInputBuffer[MAX_FIELDBUS_BOARDS][MAX_FIELDBUS_DATA_LENGTH];
    uint8_t oFieldbusOutputBuffer[MAX_FIELDBUS_BOARDS][MAX_FIELDBUS_DATA_LENGTH];

#if ECAT_CYCLE_TIMING_PRINTOUTS
    struct timespec oStartTime;
    struct timespec oEndTime;
    static struct timespec oLastStartTime = {};
    uint64_t oStartToStart;
    static uint64_t oMinStartToStart = 9999999999;
    static uint64_t oMaxStartToStart = 0;
    uint64_t oStartToEnd;
    static uint64_t oMinStartToEnd = 9999999999;
    static uint64_t oMaxStartToEnd = 0;
    static uint32_t oDebugLoop = 0;

    clock_gettime(CLOCK_TO_USE, &oStartTime);
    oStartToStart = DIFF_NS(oLastStartTime, oStartTime);
    if (oDebugLoop != 0)
    {
        if (oStartToStart < oMinStartToStart)
        {
            oMinStartToStart = oStartToStart;
        }
        if (oStartToStart > oMaxStartToStart)
        {
            oMaxStartToStart = oStartToStart;
        }
    }
    oLastStartTime = oStartTime;
#endif

    // local copy of OutputBuffers, avoids overwriting by incoming interface functions
    pthread_mutex_lock(&oFieldbusOutLock);
    memcpy(oFieldbusOutputBuffer, m_oFieldbusOutputBuffer, sizeof(oFieldbusOutputBuffer));
    pthread_mutex_unlock(&oFieldbusOutLock);

    ////////////////////
    // receive process data
    ////////////////////
    static int oIOReadErrorCounter[MAX_FIELDBUS_BOARDS]{};
    for (int i = 0; i < m_boardsFound; i++)
    {
        oReturnValue = xChannelIORead(m_hilscherChannelHandle[i][FieldbusChannel::eFieldbusChannel1], 0, 0, m_oActFieldbusDatalength, oFieldbusInputBuffer[i], 20);
        if(oReturnValue != CIFX_NO_ERROR)
        {
            char oErrorText[200];
            ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
            char oErrorNoText[20];
            sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
            wmLog(eDebug, "Fieldbus Error (IO read): %s (%s)\n", oErrorNoText, oErrorText);

            oIOReadErrorCounter[i]++;
            if (oIOReadErrorCounter[i] >= 5)
            {
                switch(oReturnValue)
                {
                    case CIFX_DEV_NO_COM_FLAG:
                        wmLogTr(eError, "QnxMsg.VI.FBReadErr1", "Error: Fieldbus communication is off ! (read IO data)\n");
                        break;
                    case CIFX_DEV_NOT_READY:
                        wmLogTr(eError, "QnxMsg.VI.FBReadErr2", "Error: Fieldbus device is not ready ! (read IO data)\n");
                        break;
                    case CIFX_DEV_NOT_RUNNING:
                        wmLogTr(eError, "QnxMsg.VI.FBReadErr3", "Error: Fieldbus device is not running ! (read IO data)\n");
                        break;
                    case CIFX_DEV_EXCHANGE_FAILED:
                        wmLogTr(eError, "QnxMsg.VI.FBReadErr4", "Error: Fieldbus data exchange has failed ! (read IO data)\n");
                        break;
                    case CIFX_DEV_EXCHANGE_TIMEOUT:
                        wmLogTr(eError, "QnxMsg.VI.FBReadErr5", "Error: Fieldbus data exchange has timeout ! (read IO data)\n");
                        break;
                    default:
                        wmLogTr(eError, "QnxMsg.VI.FBReadIOData", "Error reading IO data area !\n");
                        break;
                }
                oIOReadErrorCounter[i] = 0;
            }
        }
        else
        {
            oIOReadErrorCounter[i] = 0;
        }
    }

    if (m_fieldbusInputsActive)
    {
        EtherCAT::EcatInData dataToProcesses;
        dataToProcesses.gateway.reserve(m_boardsFound);
        for (int i = 0; i < m_boardsFound; i++)
        {

            auto& gateway = dataToProcesses.gateway.emplace_back(eProductIndex_Anybus_GW, static_cast<EcatInstance>(i + 1));
            auto& oTempVec = gateway.data;
            oTempVec.reserve(m_oActFieldbusDatalength);
            for (unsigned int j = 0; j < m_oActFieldbusDatalength; j++)
            {
                oTempVec.push_back(uint8_t(oFieldbusInputBuffer[i][j]));
            }
        }
        m_rFieldbusInputsProxy.ecatData(dataToProcesses);
    }

    pthread_mutex_lock(&m_oCheckProcessesMutex);
    if ( !isSOUVIS6000_Application() )
    {
        // insert System ready Information
        if (m_oSystemReadyOffset != -1)
        {
            unsigned char oHelpUChar = oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oSystemReadyOffset];
            if (!m_oSystemReadyState)
            {
                oHelpUChar &= ~m_oSystemReadyMask;
            }
            oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oSystemReadyOffset] = oHelpUChar;
        }
        // insert System Error Information
        if (m_oSystemErrorFieldOffset != -1)
        {
            unsigned char oHelpUChar1 = oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oSystemErrorFieldOffset];
            unsigned char oHelpUChar2 = oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oSystemErrorFieldOffset + 1];
            if (m_oSystemErrorFieldValue != 0)
            {
                oHelpUChar1 |= m_oSystemErrorFieldMask1;
                oHelpUChar2 |= m_oSystemErrorFieldMask2;
            }
            oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oSystemErrorFieldOffset] = oHelpUChar1;
            oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oSystemErrorFieldOffset + 1] = oHelpUChar2;
        }
        if (m_boardsFound > 1)
        {
            // insert System ready Information interface full
            if (m_oSystemReadyOffsetFull != -1)
            {
                unsigned char oHelpUChar = oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard2][m_oSystemReadyOffsetFull];
                if (!m_oSystemReadyState)
                {
                    oHelpUChar &= ~m_oSystemReadyMaskFull;
                }
                oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard2][m_oSystemReadyOffsetFull] = oHelpUChar;
            }
            // insert System Error Information interface full
            if (m_oSystemErrorFieldOffsetFull != -1)
            {
                unsigned char oHelpUChar1 = oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard2][m_oSystemErrorFieldOffsetFull];
                unsigned char oHelpUChar2 = oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard2][m_oSystemErrorFieldOffsetFull + 1];
                if (m_oSystemErrorFieldValue != 0)
                {
                    oHelpUChar1 |= m_oSystemErrorFieldMask1Full;
                    oHelpUChar2 |= m_oSystemErrorFieldMask2Full;
                }
                oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard2][m_oSystemErrorFieldOffsetFull] = oHelpUChar1;
                oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard2][m_oSystemErrorFieldOffsetFull + 1] = oHelpUChar2;
            }
        }
    }
    else
    {
        // insert System fault Information
        if (m_oS6KSystemFaultOffset != -1)
        {
            unsigned char oHelpUChar = oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oS6KSystemFaultOffset];
            if (!m_oSystemReadyState)
            {
                oHelpUChar |= m_oS6KSystemFaultMask;
            }
            oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oS6KSystemFaultOffset] = oHelpUChar;
        }
        // insert System ready Information
        if (m_oS6KSystemReadyOffset != -1)
        {
            unsigned char oHelpUChar = oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oS6KSystemReadyOffset];
            if (!m_oSystemReadyState)
            {
                oHelpUChar &= ~m_oS6KSystemReadyMask;
            }
            oFieldbusOutputBuffer[FieldbusBoard::eFieldbusBoard1][m_oS6KSystemReadyOffset] = oHelpUChar;
        }
    }
    pthread_mutex_unlock(&m_oCheckProcessesMutex);

    /////////////////////
    // send process data
    /////////////////////
    static int oIOWriteErrorCounter[MAX_FIELDBUS_BOARDS]{};
    for (int i = 0; i < m_boardsFound; i++)
    {
        oReturnValue = xChannelIOWrite(m_hilscherChannelHandle[i][FieldbusChannel::eFieldbusChannel1], 0, 0, m_oActFieldbusDatalength, oFieldbusOutputBuffer[i], 20);
        if (oReturnValue != CIFX_NO_ERROR)
        {
            char oErrorText[200];
            ShowHilscherError(oReturnValue, oErrorText, sizeof(oErrorText));
            char oErrorNoText[20];
            sprintf(oErrorNoText, "0x%X", (unsigned int)oReturnValue);
            wmLog(eDebug, "Fieldbus Error (IO write): %s (%s)\n", oErrorNoText, oErrorText);

            oIOWriteErrorCounter[i]++;
            if (oIOWriteErrorCounter[i] >= 5)
            {
                switch (oReturnValue)
                {
                    case CIFX_DEV_NO_COM_FLAG:
                        wmLogTr(eError, "QnxMsg.VI.FBWriteErr1", "Error: Fieldbus communication is off ! (write IO data)\n");
                        break;
                    case CIFX_DEV_NOT_READY:
                        wmLogTr(eError, "QnxMsg.VI.FBWriteErr2", "Error: Fieldbus device is not ready ! (write IO data)\n");
                        break;
                    case CIFX_DEV_NOT_RUNNING:
                        wmLogTr(eError, "QnxMsg.VI.FBWriteErr3", "Error: Fieldbus device is not running ! (write IO data)\n");
                        break;
                    case CIFX_DEV_EXCHANGE_FAILED:
                        wmLogTr(eError, "QnxMsg.VI.FBWriteErr4", "Error: Fieldbus data exchange has failed ! (write IO data)\n");
                        break;
                    case CIFX_DEV_EXCHANGE_TIMEOUT:
                        wmLogTr(eError, "QnxMsg.VI.FBWriteErr5", "Error: Fieldbus data exchange has timeout ! (write IO data)\n");
                        break;
                    default:
                        wmLogTr(eError, "QnxMsg.VI.FBWriteIOData", "Error writing to IO data area !\n");
                        break;
                }
                oIOWriteErrorCounter[i] = 0;
            }
        }
        else
        {
            oIOWriteErrorCounter[i] = 0;
        }
    }

    ///////////////////////////////////////////////
    // send all input data for Service/IO on wmMain
    ///////////////////////////////////////////////
    if (m_sendAllData)
    {
        stdVecUINT8 oTempVec;
        for (int i = 0; i < m_boardsFound; i++)
        {
            // first insert input bytes
            for (unsigned int j = 0; j < m_oActFieldbusDatalength; j++)
            {
                oTempVec.push_back(uint8_t(oFieldbusInputBuffer[i][j]));
            }
            // second insert output bytes
            for (unsigned int j = 0; j < m_oActFieldbusDatalength; j++)
            {
                oTempVec.push_back(uint8_t(oFieldbusOutputBuffer[i][j]));
            }
        }
        if (m_fieldbusInputsActive)
        {
            m_rFieldbusInputsToServiceProxy.fieldbusAllDataIn((uint16_t)oTempVec.size(), oTempVec);
        }
    }

#if ECAT_CYCLE_TIMING_PRINTOUTS
    clock_gettime(CLOCK_TO_USE, &oEndTime);
    oStartToEnd = DIFF_NS(oStartTime, oEndTime);
    if (oStartToEnd < oMinStartToEnd)
    {
        oMinStartToEnd = oStartToEnd;
    }
    if (oStartToEnd > oMaxStartToEnd)
    {
        oMaxStartToEnd = oStartToEnd;
    }

    oDebugLoop++;
    if (oDebugLoop >= 1000)
    {
        printf("%10ld(%6ld), %10ld(%6ld), %10ld, %10ld\n", oMinStartToStart, (CYCLE_TIME_NS - oMinStartToStart), oMaxStartToStart, (oMaxStartToStart - CYCLE_TIME_NS),
               oMinStartToEnd, oMaxStartToEnd);
        oMinStartToStart = 9999999999;
        oMaxStartToStart = 0;
        oMinStartToEnd = 9999999999;
        oMaxStartToEnd = 0;
        oDebugLoop = 0;
    }
#endif
}

// Thread Funktion muss ausserhalb der Klasse sein
void *FieldbusCyclicTaskThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "CycleTaskThread");
    struct timespec oWakeupTime;
    int retValue;

    struct DataToFieldbusCyclicTaskThread* pDataToFieldbusCyclicTaskThread;
    Fieldbus* pFieldbus;

    pDataToFieldbusCyclicTaskThread = static_cast<struct DataToFieldbusCyclicTaskThread *>(p_pArg);
    pFieldbus = pDataToFieldbusCyclicTaskThread->m_pFieldbus;

    wmLog(eDebug, "FieldbusCyclicTaskThread is started\n");

    pthread_t oMyPthread_ID = pthread_self();

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(3, &cpuset);
    if (pthread_setaffinity_np(oMyPthread_ID, sizeof(cpuset), &cpuset) != 0)
    {
        wmLog(eDebug, "pthread_setaffinity_np failed!  error is: %s\n", strerror(errno));
        wmFatal(eBusSystem, "QnxMsg.VI.FBCyclicThreadInit", "Problem while initializing cyclic task ! %s\n", "(001)");
    }

    system::makeThreadRealTime(system::Priority::FieldBusCyclicTask);

    usleep(5000 * 1000); // 5 seconds delay before starting to startup fieldbus

    wmLog(eDebug, "FieldbusCyclicTaskThread is active\n");

    pFieldbus->StartupFieldbus();

    clock_gettime(CLOCK_TO_USE, &oWakeupTime);
    oWakeupTime.tv_sec += 1; // start in future
    oWakeupTime.tv_nsec = 0;

    bool oFirstLoop = true;
    while(!s_threadsStopped)
    {
        retValue = clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &oWakeupTime, nullptr);
        if (retValue)
        {
            wmLogTr(eError, "QnxMsg.VI.ECMCyclicSleepFail", "Sleeping time for cycle loop failed (%s) !\n", strerror(retValue));
            break;
        }

#if ECAT_CYCLE_VIA_SERIAL_PORT
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

        if (oFirstLoop)
        {
            oFirstLoop = false;
        }
#if ECAT_CYCLE_VIA_SERIAL_PORT
        int oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIS, &g_oRTS02_flag);
        if (oRetVal != 0)
        {
            printf("Error in ioctl\n");
            perror("");
        }
#endif
        pFieldbus->FieldbusCyclicTask();
#if ECAT_CYCLE_VIA_SERIAL_PORT
        oRetVal = ioctl(g_oDebugSerialFd, TIOCMBIC, &g_oRTS02_flag);
        if (oRetVal != 0)
        {
            printf("Error in ioctl\n");
            perror("");
        }
#endif

        oWakeupTime.tv_nsec += CYCLE_TIME_NS;
        while(oWakeupTime.tv_nsec >= NSEC_PER_SEC)
        {
            oWakeupTime.tv_nsec -= NSEC_PER_SEC;
            oWakeupTime.tv_sec++;
        }
    }

    return nullptr;
}

void Fieldbus::StartCheckProcessesThread(void)
{
    // if there is no fieldbus board -> do nothing
    if ((!m_oFieldbusViaSeparateFieldbusBoard) && (!m_oFieldbusTwoSeparateFieldbusBoards))
    {
        return;
    }

    ///////////////////////////////////////////////////////
    // start thread for process monitoring
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToCheckProcessesThread.m_pFieldbus = this;

    if (pthread_create(&m_oCheckProcessesThread, &oPthreadAttr, &CheckProcessesThread, &m_oDataToCheckProcessesThread) != 0)
    {
        wmFatal(eBusSystem, "QnxMsg.VI.CreateProcMonThFail", "Cannot start thread for process monitoring\n");
    }
}

void Fieldbus::InitCheckProcesses(void)
{
    memset(&m_oInfoSystemReady, 0, sizeof(m_oInfoSystemReady));
    memset(&m_oInfoSystemErrorField, 0, sizeof(m_oInfoSystemErrorField));
    memset(&m_oInfoSystemReadyFull, 0, sizeof(m_oInfoSystemReadyFull));
    memset(&m_oInfoSystemErrorFieldFull, 0, sizeof(m_oInfoSystemErrorFieldFull));
    memset(&m_oInfoS6KSystemFault, 0, sizeof(m_oInfoS6KSystemFault));
    memset(&m_oInfoS6KSystemReady, 0, sizeof(m_oInfoS6KSystemReady));

    if ( !isSOUVIS6000_Application() )
    {
        m_oConfigParser.getSystemReadyInfo(m_oInfoSystemReady);
        m_oConfigParser.getSystemErrorFieldInfo(m_oInfoSystemErrorField);
        m_oConfigParser.getSystemReadyInfoFull(m_oInfoSystemReadyFull);
        m_oConfigParser.getSystemErrorFieldInfoFull(m_oInfoSystemErrorFieldFull);
#if PROCESS_MONITOR_INIT_DEBUG
        printf("SystemReady m_oPresent:              %d\n", m_oInfoSystemReady.m_oPresent);
        printf("SystemReady m_oVendorID:             0x%X\n", m_oInfoSystemReady.m_oVendorID);
        printf("SystemReady m_oProductCode:          0x%X\n", m_oInfoSystemReady.m_oProductCode);
        printf("SystemReady m_oStartBit:             %d\n", m_oInfoSystemReady.m_oStartBit);
        printf("SystemReady m_oLength:               %d\n", m_oInfoSystemReady.m_oLength);
        printf("SystemReady m_oInstance:             %d\n", m_oInfoSystemReady.m_oInstance);
        printf("SystemErrorField m_oPresent:         %d\n", m_oInfoSystemErrorField.m_oPresent);
        printf("SystemErrorField m_oVendorID:        0x%X\n", m_oInfoSystemErrorField.m_oVendorID);
        printf("SystemErrorField m_oProductCode:     0x%X\n", m_oInfoSystemErrorField.m_oProductCode);
        printf("SystemErrorField m_oStartBit:        %d\n", m_oInfoSystemErrorField.m_oStartBit);
        printf("SystemErrorField m_oLength:          %d\n", m_oInfoSystemErrorField.m_oLength);
        printf("SystemErrorField m_oInstance:        %d\n", m_oInfoSystemErrorField.m_oInstance);
        printf("SystemReadyFull m_oPresent:          %d\n", m_oInfoSystemReadyFull.m_oPresent);
        printf("SystemReadyFull m_oVendorID:         0x%X\n", m_oInfoSystemReadyFull.m_oVendorID);
        printf("SystemReadyFull m_oProductCode:      0x%X\n", m_oInfoSystemReadyFull.m_oProductCode);
        printf("SystemReadyFull m_oStartBit:         %d\n", m_oInfoSystemReadyFull.m_oStartBit);
        printf("SystemReadyFull m_oLength:           %d\n", m_oInfoSystemReadyFull.m_oLength);
        printf("SystemReadyFull m_oInstance:         %d\n", m_oInfoSystemReadyFull.m_oInstance);
        printf("SystemErrorFieldFull m_oPresent:     %d\n", m_oInfoSystemErrorFieldFull.m_oPresent);
        printf("SystemErrorFieldFull m_oVendorID:    0x%X\n", m_oInfoSystemErrorFieldFull.m_oVendorID);
        printf("SystemErrorFieldFull m_oProductCode: 0x%X\n", m_oInfoSystemErrorFieldFull.m_oProductCode);
        printf("SystemErrorFieldFull m_oStartBit:    %d\n", m_oInfoSystemErrorFieldFull.m_oStartBit);
        printf("SystemErrorFieldFull m_oLength:      %d\n", m_oInfoSystemErrorFieldFull.m_oLength);
        printf("SystemErrorFieldFull m_oInstance:    %d\n", m_oInfoSystemErrorFieldFull.m_oInstance);
#endif
    }
    else
    {
        m_oConfigParser.getS6KSystemFaultInfo(m_oInfoS6KSystemFault);
        m_oConfigParser.getS6KSystemReadyInfo(m_oInfoS6KSystemReady);
#if PROCESS_MONITOR_INIT_DEBUG
        printf("S6KSystemFault m_oPresent:           %d\n", m_oInfoS6KSystemFault.m_oPresent);
        printf("S6KSystemFault m_oVendorID:          0x%X\n", m_oInfoS6KSystemFault.m_oVendorID);
        printf("S6KSystemFault m_oProductCode:       0x%X\n", m_oInfoS6KSystemFault.m_oProductCode);
        printf("S6KSystemFault m_oStartBit:          %d\n", m_oInfoS6KSystemFault.m_oStartBit);
        printf("S6KSystemFault m_oLength:            %d\n", m_oInfoS6KSystemFault.m_oLength);
        printf("S6KSystemFault m_oInstance:          %d\n", m_oInfoS6KSystemFault.m_oInstance);
        printf("S6KSystemReady m_oPresent:           %d\n", m_oInfoS6KSystemReady.m_oPresent);
        printf("S6KSystemReady m_oVendorID:          0x%X\n", m_oInfoS6KSystemReady.m_oVendorID);
        printf("S6KSystemReady m_oProductCode:       0x%X\n", m_oInfoS6KSystemReady.m_oProductCode);
        printf("S6KSystemReady m_oStartBit:          %d\n", m_oInfoS6KSystemReady.m_oStartBit);
        printf("S6KSystemReady m_oLength:            %d\n", m_oInfoS6KSystemReady.m_oLength);
        printf("S6KSystemReady m_oInstance:          %d\n", m_oInfoS6KSystemReady.m_oInstance);
#endif
    }

    pthread_mutex_lock(&m_oCheckProcessesMutex);
    int oGatewayInstance = 0;
    for(unsigned int i = 0;i < m_oSlaveInfo4Service.size();i++)
    {
#if PROCESS_MONITOR_INIT_DEBUG
        printf("----------\n");
        printf("dwProductCode: 0x%X\n", m_oSlaveInfo4Service[i].dwProductCode);
        printf("dwVendorId:    0x%X\n", m_oSlaveInfo4Service[i].dwVendorId);
        printf("wPortState:    %d\n", m_oSlaveInfo4Service[i].wPortState);
        printf("dwPdOffsOut:   %d\n", m_oSlaveInfo4Service[i].dwPdOffsOut);
        printf("dwPdSizeOut:   %d\n", m_oSlaveInfo4Service[i].dwPdSizeOut);
#endif

        if ( !isSOUVIS6000_Application() )
        {
            if (((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemReady.m_oProductCode) &&
                (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemReady.m_oVendorID)) ||
                ((PRODUCTCODE_ANYBUS_GW == m_oInfoSystemReady.m_oProductCode) &&
                (VENDORID_HMS == m_oInfoSystemReady.m_oVendorID)))
            {
                oGatewayInstance++;
            }

            if ((((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemReady.m_oProductCode) &&
                (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemReady.m_oVendorID)) ||
                ((PRODUCTCODE_ANYBUS_GW == m_oInfoSystemReady.m_oProductCode) &&
                (VENDORID_HMS == m_oInfoSystemReady.m_oVendorID))) &&
                (oGatewayInstance == m_oInfoSystemReady.m_oInstance))
            {
                m_oSystemReadyOffset = (m_oInfoSystemReady.m_oStartBit / 8); // currently only one fieldbus board in I/O range, no offset necessary
            }
            if ((((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemReadyFull.m_oProductCode) &&
                (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemReadyFull.m_oVendorID)) ||
                ((PRODUCTCODE_ANYBUS_GW == m_oInfoSystemReadyFull.m_oProductCode) &&
                (VENDORID_HMS == m_oInfoSystemReadyFull.m_oVendorID))) &&
                (oGatewayInstance == m_oInfoSystemReadyFull.m_oInstance))
            {
                m_oSystemReadyOffsetFull = (m_oInfoSystemReadyFull.m_oStartBit / 8);
            }
            if ((((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemErrorField.m_oProductCode) &&
                (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemErrorField.m_oVendorID)) ||
                ((PRODUCTCODE_ANYBUS_GW == m_oInfoSystemErrorField.m_oProductCode) &&
                (VENDORID_HMS == m_oInfoSystemErrorField.m_oVendorID))) &&
                (oGatewayInstance == m_oInfoSystemErrorField.m_oInstance))
            {
                m_oSystemErrorFieldOffset = (m_oInfoSystemErrorField.m_oStartBit / 8); // currently only one fieldbus board in I/O range, no offset necessary
            }
            if ((((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoSystemErrorFieldFull.m_oProductCode) &&
                (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoSystemErrorFieldFull.m_oVendorID)) ||
                ((PRODUCTCODE_ANYBUS_GW == m_oInfoSystemErrorFieldFull.m_oProductCode) &&
                (VENDORID_HMS == m_oInfoSystemErrorFieldFull.m_oVendorID))) &&
                (oGatewayInstance == m_oInfoSystemErrorFieldFull.m_oInstance))
            {
                m_oSystemErrorFieldOffsetFull = (m_oInfoSystemErrorFieldFull.m_oStartBit / 8);
            }
        }
        else
        {
            if (((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoS6KSystemFault.m_oProductCode) &&
                (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoS6KSystemFault.m_oVendorID)) ||
                ((PRODUCTCODE_ANYBUS_GW == m_oInfoS6KSystemFault.m_oProductCode) &&
                (VENDORID_HMS == m_oInfoS6KSystemFault.m_oVendorID)))
            {
                oGatewayInstance++;
            }

            if ((((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoS6KSystemFault.m_oProductCode) &&
                (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoS6KSystemFault.m_oVendorID)) ||
                ((PRODUCTCODE_ANYBUS_GW == m_oInfoS6KSystemFault.m_oProductCode) &&
                (VENDORID_HMS == m_oInfoS6KSystemFault.m_oVendorID))) &&
                (oGatewayInstance == m_oInfoS6KSystemFault.m_oInstance))
            {
                m_oS6KSystemFaultOffset = (m_oInfoS6KSystemFault.m_oStartBit / 8); // currently only one fieldbus board in I/O range, no offset necessary
            }
            if ((((m_oSlaveInfo4Service[i].dwProductCode == m_oInfoS6KSystemReady.m_oProductCode) &&
                (m_oSlaveInfo4Service[i].dwVendorId == m_oInfoS6KSystemReady.m_oVendorID)) ||
                ((PRODUCTCODE_ANYBUS_GW == m_oInfoS6KSystemReady.m_oProductCode) &&
                (VENDORID_HMS == m_oInfoS6KSystemReady.m_oVendorID))) &&
                (oGatewayInstance == m_oInfoS6KSystemReady.m_oInstance))
            {
                m_oS6KSystemReadyOffset = (m_oInfoS6KSystemReady.m_oStartBit / 8); // currently only one fieldbus board in I/O range, no offset necessary
            }
        }
    }
    m_oSystemReadyMask = 0x01 << (m_oInfoSystemReady.m_oStartBit % 8);
    m_oSystemReadyMaskFull = 0x01 << (m_oInfoSystemReadyFull.m_oStartBit % 8);
    if (eInternalError < 256)
    {
        m_oSystemErrorFieldMask1 = (unsigned char)(eInternalError);
        m_oSystemErrorFieldMask2 = 0x00;
        m_oSystemErrorFieldMask1Full = (unsigned char)(eInternalError);
        m_oSystemErrorFieldMask2Full = 0x00;
    }
    else
    {
        m_oSystemErrorFieldMask1 = 0x00;
        m_oSystemErrorFieldMask2 = (unsigned char)(eInternalError - 256);
        m_oSystemErrorFieldMask1Full = 0x00;
        m_oSystemErrorFieldMask2Full = (unsigned char)(eInternalError - 256);
    }
    m_oS6KSystemFaultMask = 0x01 << (m_oInfoS6KSystemFault.m_oStartBit % 8);
    m_oS6KSystemReadyMask = 0x01 << (m_oInfoS6KSystemReady.m_oStartBit % 8);

#if PROCESS_MONITOR_INIT_DEBUG
    if ( !isSOUVIS6000_Application() )
    {
        printf("m_oSystemReadyOffset:         %d\n",   m_oSystemReadyOffset);
        printf("m_oSystemReadyMask:           %02X\n", m_oSystemReadyMask);
        printf("m_oSystemReadyOffsetFull:     %d\n",   m_oSystemReadyOffsetFull);
        printf("m_oSystemReadyMaskFull:       %02X\n", m_oSystemReadyMaskFull);
        printf("m_oSystemErrorFieldOffset     %d\n",   m_oSystemErrorFieldOffset);
        printf("m_oSystemErrorFieldMask1:     %02X\n", m_oSystemErrorFieldMask1);
        printf("m_oSystemErrorFieldMask2:     %02X\n", m_oSystemErrorFieldMask2);
        printf("m_oSystemErrorFieldOffsetFull %d\n",   m_oSystemErrorFieldOffsetFull);
        printf("m_oSystemErrorFieldMask1Full: %02X\n", m_oSystemErrorFieldMask1Full);
        printf("m_oSystemErrorFieldMask2Full: %02X\n", m_oSystemErrorFieldMask2Full);
    }
    else
    {
        printf("m_oS6KSystemFaultOffset:      %d\n",   m_oS6KSystemFaultOffset);
        printf("m_oS6KSystemFaultMask:        %02X\n", m_oS6KSystemFaultMask);
        printf("m_oS6KSystemReadyOffset:      %d\n",   m_oS6KSystemReadyOffset);
        printf("m_oS6KSystemReadyMask:        %02X\n", m_oS6KSystemReadyMask);
    }
#endif

    pthread_mutex_unlock(&m_oCheckProcessesMutex);
}

void Fieldbus::CheckProcesses(void)
{
#if PROCESS_MONITOR_DEBUG
    printf("************************************\n");
#endif
    for(int i = 0;i <= LAST_KEY_INDEX;i++)
    {
        size_t dotPos;
        dotPos = precitec::interface::pidKeys[i].rfind('.');
        if(dotPos == std::string::npos)
        {
            continue;
        }

        std::string appName = precitec::interface::pidKeys[i].substr( 0, dotPos );
        std::string appNameWatch = appName;
        std::string appNameIsAlive = appName;
        std::string appNamePid = appName;

        appNameWatch.append(".Watch");
        appNameIsAlive.append(".IsAlive");
        appNamePid.append(".Pid");

        bool appNameWatchState = precitec::interface::ConnectionConfiguration::instance().getBool(appNameWatch, true);
        bool appNameIsAliveState = precitec::interface::ConnectionConfiguration::instance().getBool(appNameIsAlive, false);
        int appNamePidNo = precitec::interface::ConnectionConfiguration::instance().getInt(appNamePid, 0);

#if PROCESS_MONITOR_DEBUG
        printf("------%s------\n", appName.c_str());
        printf("%s: %d\n", appNameWatch.c_str(), appNameWatchState);
        printf("%s: %d\n", appNameIsAlive.c_str(), appNameIsAliveState);
        printf("%s: %d\n", appNamePid.c_str(), appNamePidNo);
#endif

        bool oProcessIsDead = false;
        if (appNameWatchState) // Prozess soll ueberwacht werden
        {
            if (!appNameIsAliveState) // ConnectServer meldet: Prozess lebt nicht mehr
            {
#if PROCESS_MONITOR_DEBUG
                printf("%s: %d\n", appNameIsAlive.c_str(), appNameIsAliveState);
#endif
                oProcessIsDead = true;
            }
            if (appNamePidNo == 0) // Der Prozess wurde nie korrekt gestartet
            {
#if PROCESS_MONITOR_DEBUG
                printf("%s: %d\n", appNamePid.c_str(), appNamePidNo);
#endif
                oProcessIsDead = true;
            }
            else // Prozess wurde gestartet
            {
                int appNameGpid = getpgid(appNamePidNo); // read group ID, if process doesn't exists, an error is returned
                if (appNameGpid == -1) // Auf den Prozess kann nicht zugegriffen werden
                {
#if PROCESS_MONITOR_DEBUG
                    printf("pgid of %s: %d\n", appName.c_str(), appNameGpid);
#endif
                    oProcessIsDead = true;
                }
            }
        } // if (appNameWatchState)
        if (oProcessIsDead)
        {
            printf("--------> %s is no longer running <--------\n", appName.c_str());
            wmLogTr(eError, "QnxMsg.VI.ProcMonMissing", "process is not longer alive: %s\n", appName.c_str());
            wmFatal(eInternalError, "QnxMsg.VI.ProcMonMissing", "process is not longer alive: %s\n", appName.c_str());
            pthread_mutex_lock(&m_oCheckProcessesMutex);
            m_oSystemReadyState = false;
            m_oSystemErrorFieldValue = eInternalError;
            pthread_mutex_unlock(&m_oCheckProcessesMutex);
        } // if (oProcessIsDead)
        sched_yield();
    } // <= LAST_KEY_INDEX
}

// Thread Funktion muss ausserhalb der Klasse sein
void *CheckProcessesThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "CheckProcesses");
    struct DataToCheckProcessesThread* pDataToCheckProcessesThread;
    Fieldbus* pFieldbus;

    pDataToCheckProcessesThread = static_cast<struct DataToCheckProcessesThread *>(p_pArg);
    pFieldbus = pDataToCheckProcessesThread->m_pFieldbus;

    wmLog(eDebug, "CheckProcessesThread is started\n");

    sleep(60); // 60 seconds delay before monitoring processes
    pFieldbus->InitCheckProcesses();

    wmLog(eDebug, "CheckProcessesThread is active\n");

    while(!s_threadsStopped)
    {
        pFieldbus->CheckProcesses();
        sleep(10); // wait 10 seconds to next check
    }

    return nullptr;
}

///////////////////////////////////////////////////////////
// interface functions (public member)
///////////////////////////////////////////////////////////

void Fieldbus::ecatGatewayOut(EcatProductIndex productIndex, EcatInstance instance, uint8_t size, stdVecUINT8 data, stdVecUINT8 mask) // Interface EthercatOutputs
{
    // if there is no fieldbus board -> do nothing
    if ((!m_oFieldbusViaSeparateFieldbusBoard) && (!m_oFieldbusTwoSeparateFieldbusBoards)) // communication via Gateway
    {
        // do nothing
        return;
    }

    if (m_boardsFound == 0) // there was no fieldbus board m_boardsFound
    {
        // do nothing
        return;
    }

    if ((productIndex == eProductIndex_Anybus_GW) ||
        (productIndex == eProductIndex_Kunbus_GW) ||
        (productIndex == eProductIndex_Fieldbus))
    {
        pthread_mutex_lock(&oFieldbusOutLock);
        // currently only one Fieldbus board possible
        if ((int)instance > m_boardsFound)
        {
            // falsche instance !
            wmLogTr(eError, "QnxMsg.VI.ECMWrongInst", "Wrong instance (%d) for output terminal %s\n", static_cast<int>(instance), "Fieldbus-Board");
            instance = eInstance1; // damit nicht falscher Speicher ueberschrieben wird
        }
        int oInstanceIndex = (int)instance - 1;

        for(unsigned int i = 0;i < size;i++)
        {
            m_oFieldbusOutputBuffer[oInstanceIndex][i] &= ~mask[i];
            m_oFieldbusOutputBuffer[oInstanceIndex][i] |= data[i];
        }

        pthread_mutex_unlock(&oFieldbusOutLock);
    }
    else
    {
        // falscher productIndex !
        wmLogTr(eError, "QnxMsg.VI.ECMWrongProd", "Wrong product index (%d) for %s\n", static_cast<int>(productIndex), "Fieldbus-Board");
        return;
    }

#if ECAT_DEBUG_OUTPUTS
    static uint8_t oBuffer[MAX_FIELDBUS_DATA_LENGTH] {};
    bool oChangeFlag = false;

    for(unsigned int i = 0;i < size;i++)
    {
        if (oBuffer[i] != mask[i]) oChangeFlag = true;
        oBuffer[i] = mask[i];
    }

    if (oChangeFlag)
    {
        printf("ecatGatewayOut: instance: %d size: %u\n", (int)instance, size);
        for (unsigned int i = 0;i < size;i++)
        {
            printf("%02X ", data[i]);
        }
        printf("\n");
        for (unsigned int i = 0;i < size;i++)
        {
            printf("%02X ", mask[i]);
        }
        printf("\n");
    }
#endif
}

void Fieldbus::ecatRequestSlaveInfo(void) // Interface EthercatOutputs
{
    // if there is no fieldbus board -> do nothing
    if ((!m_oFieldbusViaSeparateFieldbusBoard) && (!m_oFieldbusTwoSeparateFieldbusBoards))
    {
        return;
    }
    sendSlaveInfo();
}

void Fieldbus::sendSlaveInfo()
{
    SlaveInfo oAllSlaveInfos(m_oSlaveInfo4Service.size());
    oAllSlaveInfos.FillBuffer(m_oSlaveInfoArray);
    m_rFieldbusInputsToServiceProxy.fieldbusAllSlaveInfo(oAllSlaveInfos);
}

char * Fieldbus::InsertTimeStamp(char * p_pTimeStampStrg)
{
    struct timespec oTimeStamp;
    clock_gettime(CLOCK_REALTIME, &oTimeStamp);
    struct tm *pTmVar;
    pTmVar = localtime(&oTimeStamp.tv_sec);
    sprintf(p_pTimeStampStrg, "%02d:%02d:%02d:%03ld", pTmVar->tm_hour, pTmVar->tm_min, pTmVar->tm_sec, (oTimeStamp.tv_nsec / 1000000));
    return p_pTimeStampStrg;
}

void Fieldbus::operationState(OperationState state) // Interface systemStatus
{
    if (!m_firstOperationStateReceived)
    {
        m_firstOperationStateReceived = true;
        m_fieldbusInputsActive = true;
    }
}

} // namespace ethercat

} // namespace precitec

