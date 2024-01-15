/**
 *     @file
 *     @copyright    Precitec Vision GmbH & Co. KG
 *     @author       EA
 *     @date         2017/2022
 *     @brief        serves the interface to CHRocodile device
 */

#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include <sys/prctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include "module/moduleLogger.h"
#include "common/systemConfiguration.h"

#include "common/defines.h"
#include "image/ipImage.h"
#include "system/timer.h"
#include "system/realTimeSupport.h"

#include "CHRocodileLibSpecialFunc.h"

#include "CHRCommunication/CHRCommunication.h"
#include "CHRCommunication/displayHelper.h"

#include "common/definesIDM.h"

using namespace precitec::interface;

namespace precitec
{

using namespace interface;

namespace grabber
{

// folgende CLOCK... verwenden
#define CLOCK_TO_USE CLOCK_MONOTONIC

// folgendes definiert die Anzahl ns pro Sekunde
#define NSEC_PER_SEC (1000000000)

#define IDM_IS_PRESENT 1

///////////////////////////////////////////////////////////
// Prototyp fuer Thread Funktions
///////////////////////////////////////////////////////////

// Thread Funktion muss ausserhalb der Klasse sein
void* CHRReadThread(void* p_pArg);

void* CHRCyclicTaskThread(void* p_pArg);

void* CHRSpectrumTransferThread(void* p_pArg);

void* CHRWriteThread(void* p_pArg);

void GeneralResponseCB(const TRspCallbackInfo p_oInfo, Rsp_h p_oResponse);
void DebugGeneralResponse(const Rsp_h p_oResponse, const TResponseInfo& p_oResponseInfo);
void ProcessFirmwareVersion(const Rsp_h p_oResponse, const TResponseInfo& p_oResponseInfo);
void ProcessSpectrumData(const Rsp_h p_oResponse, const TResponseInfo& p_oResponseInfo, CHRCommunication* p_pCHRCommunication);
void ProcessFullScale(const Rsp_h p_oResponse, const TResponseInfo& p_oResponseInfo, CHRCommunication* p_pCHRCommunication);

void SampleDataCB(void* p_pUser, int32_t p_oState, int64_t p_oSampleCount, const double* p_pSampleBuffer, LibSize_t p_oSizePerSample,
                  TSampleSignalGeneralInfo p_oSignalGeneralInfo, TSampleSignalInfo* p_pSignalInfo);

///////////////////////////////////////////////////////////
// global variables for debugging purposes
///////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

CHRCommunication::CHRCommunication(TSensor<AbstractInterface>& p_rSensorProxy)
    : m_oTriggerDistance_ns(10000000)
    , m_IDMDistancePeak1RingBuffer(70000)
    , m_IDMQualityPeak1RingBuffer(70000)
    , m_oSpectrumActive(false)
    , m_oCHRReadThread_ID(0)
    , m_oCHRCyclicTaskThread_ID(0)
    , m_oCHRSpectrumTransferThread_ID(0)
    , m_oCHRWriteThread_ID(0)
    , m_oResultsOnOff(false)
    , m_rSensorProxy(p_rSensorProxy)
    , m_oImageNr_IDMWeldDepth(0)
    , m_pValues_IDMWeldDepth(nullptr)
    , m_imageNoIDMQualityPeak1(0)
    , m_valuesIDMQualityPeak1(nullptr)
    , m_oSendSamples_CHRSpectrumLine(0)
    , m_oImageNr_CHRSpectrumLine(0)
    , m_pValues_CHRSpectrumLine(nullptr)
    , m_oSendSamples_CLSLines(0)
    , m_oImageNr_CLSLines(0)
    , m_oCycleIsOn(false)
    , m_oSeamIsOn(false)
    , m_oLiveModeIsOn(false)
    , m_oSLDDimmerOnOff(false)
    , m_oAdaptiveExposureOnOff(false)
    , m_oAdaptiveExposureBasicValue(50)
    , m_oRescaleFactorCHRResults(10)
    , m_oWaitingForCommandResponse(false)
    , m_oWaitingForCommandResponseCounter(0)
    , m_oWaitingForCommandResponseLimit(10)
{
    // SystemConfig Switches for CHRocodile devices
    m_oIDMDevice1Enable = SystemConfiguration::instance().getBool("IDM_Device1Enable", false);
    wmLog(eDebug, "m_oIDMDevice1Enable (bool): %d\n", m_oIDMDevice1Enable);
    m_oIDMDevice1IpAddress = SystemConfiguration::instance().getString("IDM_Device1_IP_Address", "192.168.170.2");
    wmLog(eDebug, "m_oIDMDevice1IpAdress (string): %s\n", m_oIDMDevice1IpAddress.c_str());
    m_oCLS2Device1Enable = SystemConfiguration::instance().getBool("CLS2_Device1Enable", false);
    wmLog(eDebug, "m_oCLS2Device1Enable (bool): %d\n", m_oCLS2Device1Enable);
    m_oCLS2Device1IpAddress = SystemConfiguration::instance().getString("CLS2_Device1_IP_Address", "192.168.170.3");
    wmLog(eDebug, "m_oCLS2Device1IpAddress (string): %s\n", m_oCLS2Device1IpAddress.c_str());

    m_oSensorIdsEnabled.insert(std::make_pair(static_cast<precitec::interface::Sensor>(CamLaserLine), false));
    m_oSensorIdsEnabled.insert(std::make_pair(eIDMWeldingDepth, false));
    m_oSensorIdsEnabled.insert(std::make_pair(eIDMTrackingLine, false));
    m_oSensorIdsEnabled.insert(std::make_pair(eIDMSpectrumLine, false));
    m_oSensorIdsEnabled.insert(std::make_pair(eIDMQualityPeak1, false));
    poco_assert_dbg(m_oSensorIdsEnabled.size() == eIDMQualityPeak1 + 1 - eExternSensorMin);

    //************************************************************************

    char oStrErrorBuffer[256];

    if (sem_init(&m_oSpectrumTransferSemaphore, 0, 0) != 0)
    {
        wmLog(eDebug, "sem_init failed (%s)(%s)\n", "003", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMSysErr", "Error while executing system call (%s)\n", "003");
    }

    //************************************************************************

    for (int i = 0; i < m_oNumberOfLines; i++)
    {
        for (int j = 0; j < m_oMaxNumberOfValues; j++)
        {
            m_oSpectrumArray[i][j] = 0;
        }
        m_oSpectrumLength[i] = 0;
        m_oSpectrumIsFree[i] = true;
    }
    m_oSpectrumIdxToSend = 0;
    m_oSpectrumIdxToWrite = m_oNumberOfLines - 1;

    //************************************************************************

    if (sem_init(&m_commandsToCHRSemaphore, 0, 0) != 0)
    {
        wmLog(eDebug, "sem_init failed (%s)(%s)\n", "003", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMSysErr", "Error while executing system call (%s)\n", "003");
    }

    //************************************************************************
    if (isIDMDevice1Enabled())
    {
        queryScale();
    }

    // read parameters from configuraiotn file
    initConfiguration();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

CHRCommunication::~CHRCommunication(void)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        if (m_oCHRWriteThread_ID != 0)
        {
            if (pthread_cancel(m_oCHRWriteThread_ID) != 0)
            {
                wmLog(eDebug, "was not able to abort thread\n");
            }
        }

        if (m_oCHRSpectrumTransferThread_ID != 0)
        {
            if (pthread_cancel(m_oCHRSpectrumTransferThread_ID) != 0)
            {
                wmLog(eDebug, "was not able to abort thread\n");
            }
        }

        if (m_oCHRCyclicTaskThread_ID != 0)
        {
            if (pthread_cancel(m_oCHRCyclicTaskThread_ID) != 0)
            {
                wmLog(eDebug, "was not able to abort thread\n");
            }
        }

        if (m_oCHRReadThread_ID)
        {
            if (pthread_cancel(m_oCHRReadThread_ID) != 0)
            {
                wmLog(eDebug, "was not able to abort thread\n");
            }
        }

        usleep(100 * 1000);

        //************************************************************************

#if IDM_IS_PRESENT
        CloseConnection(m_oCHRHandle);
#endif

        //************************************************************************
    }

    char oStrErrorBuffer[256];

    if (sem_destroy(&m_commandsToCHRSemaphore) != 0)
    {
        wmLog(eDebug, "sem_destroy failed (%s)(%s)\n", "006", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMSysErr", "Error while executing system call (%s)\n", "006");
    }

    if (sem_destroy(&m_oSpectrumTransferSemaphore) != 0)
    {
        wmLog(eDebug, "sem_destroy failed (%s)(%s)\n", "006", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMSysErr", "Error while executing system call (%s)\n", "006");
    }
}

///////////////////////////////////////////////////////////
// private members
///////////////////////////////////////////////////////////

//****************************************************************************
//* more detailed error text                                                 *
//****************************************************************************

void CHRCommunication::ErrorInCHRFunction(const Res_t p_oRes, const std::string& p_oFunctionName)
{
    wmLog(eError, p_oFunctionName + "\n");
    char oErrorString[100]{};
    LibSize_t oSize = static_cast<LibSize_t>(sizeof(oErrorString));
    ErrorCodeToString(p_oRes, oErrorString, &oSize);
    strcat(oErrorString, "\n");
    wmLog(eError, oErrorString);
}

void ErrorInCHRFunction(const Res_t p_oRes, const std::string& p_oFunctionName)
{
    wmLog(eError, p_oFunctionName + "\n");
    char oErrorString[100]{};
    LibSize_t oSize = static_cast<LibSize_t>(sizeof(oErrorString));
    ErrorCodeToString(p_oRes, oErrorString, &oSize);
    strcat(oErrorString, "\n");
    wmLog(eError, oErrorString);
}

//****************************************************************************
//* InitCHRCommunication                                                     *
//****************************************************************************

void CHRCommunication::InitCHRCommunication(void)
{
    if (isIDMDevice1Enabled() && isCLS2Device1Enabled())
    {
        wmFatal(eExtEquipment, "QnxMsg.VI.IDMCLSBoth", "Either IDM_Device1Enable or CLS2_Device1Enable is possible !\n");
        m_oIDMDevice1Enable = false;
        m_oCLS2Device1Enable = false;
        return;
    }

    Res_t oRes = SetLibLogFileDirectory(".", 500, 30); // current dir, max 500KiB per file, max 30 files
    if (!CHR_SUCCESS(oRes))
    {
        ErrorInCHRFunction(oRes, "SetLibLogFileDirectory");
    }
    oRes = SetLibLogLevel(99); // very high logging level
    if (!CHR_SUCCESS(oRes))
    {
        ErrorInCHRFunction(oRes, "SetLibLogLevel");
    }

    //************************************************************************

    if (isIDMDevice1Enabled())
    {
#if IDM_IS_PRESENT
        // IDM
        wmLog(eDebug, "OpenConnection to IDM Device1: %s\n", m_oIDMDevice1IpAddress.c_str());
        oRes = OpenConnection(m_oIDMDevice1IpAddress.c_str(), CHR_2_Device, Connection_Asynchronous, 0, &m_oCHRHandle); //buffer size 0, use default buffer size 32MB
        if (!CHR_SUCCESS(oRes))
        {
            wmFatal(eExtEquipment, "QnxMsg.VI.IDMErrConnOpen", "Unable to open connection to the IDM device\n", "001");
            ErrorInCHRFunction(oRes, "OpenConnection");
            m_oCHRHandle = std::numeric_limits<Conn_h>::max();
            m_oIDMDevice1Enable = false;
            return;
        }
#endif
    }
    else if (isCLS2Device1Enabled())
    {
        // CLS2
        wmLog(eDebug, "OpenConnection to CLS2 Device1: %s\n", m_oCLS2Device1IpAddress.c_str());
        oRes = OpenConnection(m_oCLS2Device1IpAddress.c_str(), CHR_Multi_Channel_Device, Connection_Asynchronous, 0, &m_oCHRHandle); //buffer size 0, use default buffer size 32MB
        if (!CHR_SUCCESS(oRes))
        {
            wmFatal(eExtEquipment, "QnxMsg.VI.CLSErrConnOpen", "Unable to open connection to the CLS device\n", "001");
            ErrorInCHRFunction(oRes, "OpenConnection");
            m_oCHRHandle = std::numeric_limits<Conn_h>::max();
            m_oCLS2Device1Enable = false;
            return;
        }
        wmLog(eInfo, "Opened connection to CLS2 device with handle: %d\n", oRes);
    }
    else
    {
        wmLog(eError, "Wrong CHRocodile device\n");
    }

#if IDM_IS_PRESENT
    int32_t oDeviceType = GetDeviceType(m_oCHRHandle);
    if (!CHR_SUCCESS(oDeviceType))
    {
        ErrorInCHRFunction(oDeviceType, "GetDeviceType");
    }
    wmLog(eDebug, "Device Type: %d\n", oDeviceType);

    int32_t oChannelCount = GetDeviceChannelCount(m_oCHRHandle);
    if (!CHR_SUCCESS(oChannelCount))
    {
        ErrorInCHRFunction(oChannelCount, "GetDeviceChannelCount");
    }
    wmLog(eDebug, "Channel Count: %d\n", oChannelCount);

    oRes = RegisterGeneralResponseAndUpdateCallback(m_oCHRHandle, this, &GeneralResponseCB);
    if (!CHR_SUCCESS(oRes))
    {
        ErrorInCHRFunction(oRes, "RegisterGeneralResponseAndUpdateCallback");
    }

    oRes = RegisterSampleDataCallback(m_oCHRHandle, 4000, 0, this, &SampleDataCB);
    if (!CHR_SUCCESS(oRes))
    {
        ErrorInCHRFunction(oRes, "RegisterSampleDataCallback");
    }
#endif

    //************************************************************************

    if (m_oCHRHandle)
    {
        // Stopp der Messdatenausgabe
        usleep(50 * 1000);
        m_checkSampleCounter.store(false);
        SetResultsOnOff(false);

        // warten bis nach Verbindungsaufbau alle Parameter gesendet wurden
        usleep(1000 * 1000);

        AskVersionString();
        usleep(200 * 1000);

        // in case of IDM: switch off detailed logging
        if (!isCLS2Device1Enabled())
        {
            setLoggingVerbosity(0);
            usleep(200 * 1000);
        }

        //********************************************************************
        // send basic parameters to Chrocodile device
        //********************************************************************

        if (isCLS2Device1Enabled())
        {
            SetNumberOfChannels(1200);
            usleep(200 * 1000);

            SetNumberOfPeaks(1);
        }

        //************************************************************************

        SetSampleFrequency(m_oConfiguration.m_oSampleFrequency);
        usleep(200 * 1000);

        //************************************************************************

        SetLampIntensity(m_oConfiguration.m_oLampIntensity);
        usleep(200 * 1000);

        //************************************************************************

        if (!isCLS2Device1Enabled())
        {
            //************************************************************************

            SetDetectionWindow(eDWD_Window_Left, m_oConfiguration.m_oDetectionWindow_Left);
            usleep(200 * 1000);

            //************************************************************************

            SetDetectionWindow(eDWD_Window_Right, m_oConfiguration.m_oDetectionWindow_Right);
            usleep(200 * 1000);

            SetQualityThreshold(m_oConfiguration.m_oQualityThreshold);
            usleep(200 * 1000);

            //************************************************************************

            SetDataAveraging(m_oConfiguration.m_oDataAveraging);
            usleep(200 * 1000);

            //************************************************************************

            SetSpectralAveraging(m_oConfiguration.m_oSpectralAveraging);
            usleep(200 * 1000);
        }

        //************************************************************************

        SetSignalSubscription();
        usleep(200 * 1000);
    }
}

void GeneralResponseCB(const TRspCallbackInfo p_oInfo, Rsp_h p_oResponse)
{
    TResponseInfo oResponseInfo;
    Res_t oRes = GetResponseInfo(p_oResponse, &oResponseInfo);
    if (!CHR_SUCCESS(oRes))
    {
        ErrorInCHRFunction(oRes, "GetResponseInfo");
    }
    CHRCommunication* pCHRCommunication = static_cast<CHRCommunication*>(p_oInfo.User);

    if (pCHRCommunication->m_oDebugInfoCHRResponse)
    {
        DebugGeneralResponse(p_oResponse, oResponseInfo);
    }

    if (oResponseInfo.CmdID == CmdID_Firmware_Version)
    {
        ProcessFirmwareVersion(p_oResponse, oResponseInfo);
    }

    if (oResponseInfo.CmdID == CmdID_Download_Spectrum)
    {
        ProcessSpectrumData(p_oResponse, oResponseInfo, pCHRCommunication);
    }

    if (oResponseInfo.CmdID == CmdID_Start_Data_Stream)
    {
        pCHRCommunication->CommandResponseOK();
    }
    if (oResponseInfo.CmdID == CmdID_Full_Scale)
    {
        ProcessFullScale(p_oResponse, oResponseInfo, pCHRCommunication);
    }
}

void DebugGeneralResponse(const Rsp_h p_oResponse, const TResponseInfo& p_oResponseInfo)
{
    char oCmdStrg[10]{};
    oCmdStrg[0] = (p_oResponseInfo.CmdID & 0x000000FF) >> 0;
    oCmdStrg[1] = (p_oResponseInfo.CmdID & 0x0000FF00) >> 8;
    oCmdStrg[2] = (p_oResponseInfo.CmdID & 0x00FF0000) >> 16;
    oCmdStrg[3] = (p_oResponseInfo.CmdID & 0xFF000000) >> 24;
    oCmdStrg[4] = 0x00;
    printf("CmdID: %4s, Ticket: %4d, Flag: 0x%04x, ParamCount: %d\n", oCmdStrg, p_oResponseInfo.Ticket, p_oResponseInfo.Flag, p_oResponseInfo.ParamCount);

    for (uint32_t i = 0; i < p_oResponseInfo.ParamCount; i++)
    {
        int32_t oArgType;
        Res_t oRes = GetResponseArgType(p_oResponse, i, &oArgType);
        if (!CHR_SUCCESS(oRes))
        {
            ErrorInCHRFunction(oRes, "GetResponseArgType");
        }
        printf("oArgType: %d\n", oArgType);
        switch (oArgType)
        {
        case Rsp_Param_Type_Integer:
        {
            int32_t oIntParameter;
            oRes = GetResponseIntArg(p_oResponse, i, &oIntParameter);
            if (!CHR_SUCCESS(oRes))
            {
                ErrorInCHRFunction(oRes, "GetResponseIntArg");
            }
            printf("%d\n", oIntParameter);
            break;
        }
        case Rsp_Param_Type_Float:
        {
            float oFloatParameter;
            oRes = GetResponseFloatArg(p_oResponse, i, &oFloatParameter);
            if (!CHR_SUCCESS(oRes))
            {
                ErrorInCHRFunction(oRes, "GetResponseFloatArg");
            }
            printf("%f\n", oFloatParameter);
            break;
        }
        case Rsp_Param_Type_String:
        {
            const char* oStringArg;
            int32_t oStringLength;
            oRes = GetResponseStringArg(p_oResponse, i, &oStringArg, &oStringLength);
            if (!CHR_SUCCESS(oRes))
            {
                ErrorInCHRFunction(oRes, "GetResponseStringArg");
            }
            printf("%s\n", oStringArg);
            break;
        }
        case Rsp_Param_Type_Byte_Array: // Blob
        {
            const int8_t* oByteArray;
            int32_t oArrayLength;
            oRes = GetResponseBlobArg(p_oResponse, i, &oByteArray, &oArrayLength);
            if (!CHR_SUCCESS(oRes))
            {
                ErrorInCHRFunction(oRes, "GetResponseBlobArg");
            }
            for (int i = 0; i < oArrayLength; i++)
            {
                printf("%d, ", *oByteArray);
                oByteArray++;
            }
            printf("\n");
            break;
        }
        case Rsp_Param_Type_Integer_Array:
        {
            const int32_t* oIntArray;
            int32_t oArrayLength;
            oRes = GetResponseIntArrayArg(p_oResponse, i, &oIntArray, &oArrayLength);
            if (!CHR_SUCCESS(oRes))
            {
                ErrorInCHRFunction(oRes, "GetResponseIntArrayArg");
            }
            for (int i = 0; i < oArrayLength; i++)
            {
                printf("%d\n", *oIntArray);
                oIntArray++;
            }
            break;
        }
        case Rsp_Param_Type_Float_Array:
        {
            const float* oFloatArray;
            int32_t oArrayLength;
            oRes = GetResponseFloatArrayArg(p_oResponse, i, &oFloatArray, &oArrayLength);
            if (!CHR_SUCCESS(oRes))
            {
                ErrorInCHRFunction(oRes, "GetResponseFloatArrayArg");
            }
            for (int i = 0; i < oArrayLength; i++)
            {
                printf("%f\n", *oFloatArray);
                oFloatArray++;
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

void ProcessFirmwareVersion(const Rsp_h p_oResponse, const TResponseInfo& p_oResponseInfo)
{
    for (uint32_t i = 0; i < p_oResponseInfo.ParamCount; i++)
    {
        int32_t oParameterType{};
        Res_t oRes = GetResponseArgType(p_oResponse, i, &oParameterType);
        if (!CHR_SUCCESS(oRes))
        {
            ErrorInCHRFunction(oRes, "GetResponseArgType");
        }
        if (oParameterType == Rsp_Param_Type_String)
        {
            const char* oStringArg;
            int32_t oStringLength;
            oRes = GetResponseStringArg(p_oResponse, i, &oStringArg, &oStringLength);
            if (!CHR_SUCCESS(oRes))
            {
                ErrorInCHRFunction(oRes, "GetResponseStringArg");
            }

            std::string oVersionString{oStringArg};
            std::string oDelimiter{"\r\n"};
            size_t pos = 0;
            std::string oVersionLine;
            while ((pos = oVersionString.find(oDelimiter)) != std::string::npos)
            {
                oVersionLine = oVersionString.substr(0, pos);
                wmLog(eInfo, oVersionLine + "\n");
                oVersionString.erase(0, pos + oDelimiter.length());
            }
            wmLog(eInfo, oVersionString + "\n");
        }
    }
}

void ProcessSpectrumData(const Rsp_h p_oResponse, const TResponseInfo& p_oResponseInfo, CHRCommunication* p_pCHRCommunication)
{
    for (uint32_t i = 0; i < p_oResponseInfo.ParamCount; i++)
    {
        int32_t oParameterType{};
        Res_t oRes = GetResponseArgType(p_oResponse, i, &oParameterType);
        if (!CHR_SUCCESS(oRes))
        {
            ErrorInCHRFunction(oRes, "GetResponseArgType");
        }

        if (oParameterType == Rsp_Param_Type_Byte_Array) // spectrum blob
        {
            const int8_t* pBlobArg{nullptr};
            int32_t oLength{};
            Res_t oRes = GetResponseBlobArg(p_oResponse, i, &pBlobArg, &oLength);
            if (!CHR_SUCCESS(oRes))
            {
                ErrorInCHRFunction(oRes, "GetResponseBlobArg");
            }

            const std::lock_guard<std::mutex> spectrumTransferLock(p_pCHRCommunication->m_oSpectrumTransferMutex);
            p_pCHRCommunication->m_oSpectrumIdxToWrite++;
            p_pCHRCommunication->m_oSpectrumIdxToWrite = (p_pCHRCommunication->m_oSpectrumIdxToWrite % p_pCHRCommunication->m_oNumberOfLines);
            if (p_pCHRCommunication->m_oSpectrumIsFree[p_pCHRCommunication->m_oSpectrumIdxToWrite] != true)
            {
                wmLogTr(eError, "QnxMsg.VI.IDMErrBufNotFree", "Buffer for extracted line is not free (%s)\n", "001");
            }
            int16_t* pShortPtr = (int16_t*)(pBlobArg);
            for (int32_t i = 0; i < (oLength / 2); i++)
            {
                p_pCHRCommunication->m_oSpectrumArray[p_pCHRCommunication->m_oSpectrumIdxToWrite][i] = (unsigned int)(pShortPtr[i]);
            }
            p_pCHRCommunication->m_oSpectrumLength[p_pCHRCommunication->m_oSpectrumIdxToWrite] = (oLength / 2);
            p_pCHRCommunication->m_oSpectrumIsFree[p_pCHRCommunication->m_oSpectrumIdxToWrite] = false;
            p_pCHRCommunication->m_oSpectrumIdxToSend = p_pCHRCommunication->m_oSpectrumIdxToWrite;

            if (sem_post(&p_pCHRCommunication->m_oSpectrumTransferSemaphore) != 0)
            {
                char oStrErrorBuffer[256];

                wmLog(eDebug, "sem_post failed (%s)(%s)\n", "102", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
                wmFatal(eInternalError, "QnxMsg.VI.IDMSysErr", "Error while executing system call (%s)\n", "102");
            }

            if (p_pCHRCommunication->m_oDebugInfoSpectrumFile)
            {
                FILE* m_pSpectrumFile;
                char oFilename[81];
                static int oFileIndex = 1;
                sprintf(oFilename, "CHR_SpectrumFile%02d.txt", oFileIndex);
                oFileIndex++;
                if (oFileIndex > 20)
                {
                    oFileIndex = 1;
                }
                m_pSpectrumFile = fopen(oFilename, "w");
                if (m_pSpectrumFile == nullptr)
                {
                    wmLog(eDebug, "Unable to open file for saving spectrum line\n");
                }
                else
                {
                    int16_t* pShortPtr = (int16_t*)(pBlobArg);
                    for (int32_t i = 0; i < (oLength / 2); i++)
                    {
                        fprintf(m_pSpectrumFile, "%d\n", pShortPtr[i]);
                    }
                }
                fclose(m_pSpectrumFile);
            }
        }
    }
}

void ProcessFullScale(const Rsp_h p_oResponse, const TResponseInfo& p_oResponseInfo, CHRCommunication* p_pCHRCommunication)
{
    for (uint32_t i = 0; i < p_oResponseInfo.ParamCount; i++)
    {
        int32_t oParameterType{};
        Res_t oRes = GetResponseArgType(p_oResponse, i, &oParameterType);
        if (!CHR_SUCCESS(oRes))
        {
            ErrorInCHRFunction(oRes, "GetResponseArgType");
        }
        if (oParameterType == Rsp_Param_Type_Integer)
        {
            int32_t oIntParameter;
            oRes = GetResponseIntArg(p_oResponse, i, &oIntParameter);
            if (!CHR_SUCCESS(oRes))
            {
                ErrorInCHRFunction(oRes, "GetResponseIntArg");
            }
            p_pCHRCommunication->setScale(oIntParameter);
        }
    }
}

void SampleDataCB(void* p_pUser, int32_t p_oState, int64_t p_oSampleCount, const double* p_pSampleBuffer, LibSize_t p_oSizePerSample,
                  TSampleSignalGeneralInfo p_oSignalGeneralInfo, TSampleSignalInfo* p_pSignalInfo)
{
    CHRCommunication* pCHRCommunication = static_cast<CHRCommunication*>(p_pUser);

    int oSignalCount = p_oSignalGeneralInfo.GlobalSignalCount + p_oSignalGeneralInfo.PeakSignalCount;
    int oResultsPerSample = p_oSignalGeneralInfo.GlobalSignalCount + (p_oSignalGeneralInfo.PeakSignalCount * p_oSignalGeneralInfo.ChannelCount);

    if (pCHRCommunication->m_sampleSigGenInfo.InfoIndex != p_oSignalGeneralInfo.InfoIndex)
    {
        pCHRCommunication->m_sampleSigGenInfo = p_oSignalGeneralInfo;

        pCHRCommunication->m_signalInfo.clear();
        for (int i = 0; i < oSignalCount; i++)
        {
            pCHRCommunication->m_signalInfo.emplace_back(p_pSignalInfo[i]);
        }

        if (pCHRCommunication->isCLS2Device1Enabled())
        {
            pCHRCommunication->m_pLineBuffer.reset(new CHRCircularLineBuffer(8192, p_oSizePerSample));
        }
    }

    auto& oSignalInfo = pCHRCommunication->m_signalInfo;

    if (pCHRCommunication->m_oDebugInfoSampleSignals)
    {
        printf("State: %d, SampleCnt: %ld, SizePerSample: %ld, DataForm: %u, GlobSig: %d, PeakSig: %d, ChCnt: %d, SignalCount: %d, ResPerSample: %d\n",
               p_oState, p_oSampleCount, p_oSizePerSample, p_oSignalGeneralInfo.InfoIndex, p_oSignalGeneralInfo.GlobalSignalCount,
               p_oSignalGeneralInfo.PeakSignalCount, p_oSignalGeneralInfo.ChannelCount, oSignalCount, oResultsPerSample);

        for (int i = 0; i < oSignalCount; i++)
        {
            printf("SigID%-4d: %5u, DataType%-4d: %3d ", (i + 1), p_pSignalInfo->SignalID, (i + 1), p_pSignalInfo->DataType);
            p_pSignalInfo++;
            if ((i % 8) == 7)
            {
                printf("\n");
            }
        }
        printf("\n");
        printf("\n");
    }

    if (pCHRCommunication->m_oDebugInfoSampleData)
    {
        static uint32_t oLoop{0};
        oLoop++;
        if ((oLoop % 100) == 99)
        {
            const double* pDebug1{p_pSampleBuffer};
            for (int i = 0; i < p_oSampleCount; i++) // do this for all samples in this data block
            {
                // global signals
                for (int j = 0; j < p_oSignalGeneralInfo.GlobalSignalCount; j++)
                {
                    //printf("%5d, ", static_cast<int>(*pDebug1));
                    printf("%7.2f, ", *pDebug1);
                    if (pCHRCommunication->m_pResultDebugFile != nullptr)
                    {
                        //fprintf(pCHRCommunication->m_pResultDebugFile, "%5d,", static_cast<int>(*pDebug1));
                        fprintf(pCHRCommunication->m_pResultDebugFile, "%7.2f,", *pDebug1);
                    }
                    pDebug1++;
                }
                // for all channels
                for (int j = 0; j < p_oSignalGeneralInfo.ChannelCount; j++)
                {
                    for (int k = 0; k < p_oSignalGeneralInfo.PeakSignalCount; k++)
                    {
                        // peak signals
                        if (j < 10) // limit the printed signals
                        {
                            printf("%7.2f, ", *pDebug1);
                        }
                        if (pCHRCommunication->m_pResultDebugFile != nullptr)
                        {
                            fprintf(pCHRCommunication->m_pResultDebugFile, "%7.2f,", *pDebug1);
                        }
                        pDebug1++;
                    }
                }
                printf("\n");
                if (pCHRCommunication->m_pResultDebugFile != nullptr)
                {
                    fprintf(pCHRCommunication->m_pResultDebugFile, "\n");
                }
            }
            printf("\n");
        }
    }

    if (pCHRCommunication->isIDMDevice1Enabled())
    {
        double oDistanceValue1 = 0.0;
        double oOldDistanceValue1 = 0.0;
        unsigned int oDistanceValueInt1 = 0;

        double qualityValue1{0.0};
        static double oldQualityValue1{0.0};
        unsigned int qualityValueInt1{0};

        uint16_t sampleCounter{0};
        static uint16_t oldSampleCounter{0};
        static int32_t debugConsecutiveSamplesCounter{0};

        const std::lock_guard<std::mutex> IDMDistancePeak1Lock(pCHRCommunication->m_IDMDistancePeak1RingBufferMutex);
        const std::lock_guard<std::mutex> IDMQualityPeak1Lock(pCHRCommunication->m_IDMQualityPeak1RingBufferMutex);

        for (int i = 0; i < p_oSampleCount; i++) // do this for all samples in this data block
        {
            for (int j = 0; j < oResultsPerSample; j++) // extract signals of one sample
            {
                // extract now the distance signal of peak 1
                if (oSignalInfo[j].SignalID == SIGNAL_ID_DISTANCE1)
                {
                    oDistanceValue1 = p_pSampleBuffer[(i * oResultsPerSample) + j];
                    if (!std::isnan(oDistanceValue1))
                    {
                        // the distance1 value ist not a NAN, save the distance1 value for possible following NAN
                        oOldDistanceValue1 = oDistanceValue1;
                    }
                    else
                    {
                        // the distance1 value is a NAN, value is not valid, use the last valid distance1 value
                        oDistanceValue1 = oOldDistanceValue1;
                    }
                    oDistanceValueInt1 = (unsigned int)(oDistanceValue1);
                    if (oDistanceValueInt1 == 0)
                    {
                        // the oDistanceValueInt1 is 0, there is still a problem with the distance1 value, use the last valid distance1 value to cast
                        oDistanceValueInt1 = (unsigned int)(oOldDistanceValue1);
                    }
                    pCHRCommunication->m_IDMDistancePeak1RingBuffer.Write(oDistanceValueInt1);

                    if (!pCHRCommunication->m_firstCallBackArrived.load())
                    {
                        pCHRCommunication->m_firstCallBackArrived.store(true);
                    }
                }                                                              // end of: if (SIGNAL_ID_DISTANCE1)

                // extract now the quality signal of peak 1
                if (oSignalInfo[j].SignalID == SIGNAL_ID_QUALITY1)
                {
                    qualityValue1 = p_pSampleBuffer[(i * oResultsPerSample) + j];
                    if (!std::isnan(qualityValue1))
                    {
                        // the qualityValue1 value ist not a NAN, save the qualityValue1 value for possible following NAN
                        oldQualityValue1 = qualityValue1;
                    }
                    else
                    {
                        // the qualityValue1 value is a NAN, value is not valid, use the last valid qualityValue1 value
                        qualityValue1 = oldQualityValue1;
                    }
                    qualityValueInt1 = (unsigned int)(qualityValue1);
                    if (qualityValueInt1 == 0)
                    {
                        // the qualityValueInt1 is 0, there is still a problem with the qualityValue1 value, use the last valid qualityValue1 value to cast
                        qualityValueInt1 = (unsigned int)(oldQualityValue1);
                    }
                    pCHRCommunication->m_IDMQualityPeak1RingBuffer.Write(qualityValueInt1);
                }

                // extract the sample counter of IDM, needed for checking of missed results
                if (oSignalInfo[j].SignalID == SIGNAL_ID_SAMPLE_COUNTER)
                {
                    sampleCounter = static_cast<uint16_t>(p_pSampleBuffer[(i * oResultsPerSample) + j]);
                } // end of: if (SIGNAL_ID_SAMPLE_COUNTER)
            }

            // check for missing results
            auto onExpectedSampleCounter = [&]()
            {
                if (debugConsecutiveSamplesCounter != 0)
                {
                    wmLog(eDebug, "Expected SampleCounter %d received (ConsecutiveSamplesCounter was %d)\n", 
                        sampleCounter, debugConsecutiveSamplesCounter);
                    debugConsecutiveSamplesCounter = 0;
                }
            };
            
            if (pCHRCommunication->m_checkSampleCounter.load())
            {
                if (pCHRCommunication->m_firstSampleCounter)
                {
                    oldSampleCounter = sampleCounter - pCHRCommunication->m_dataAveragingBuffer;
                    pCHRCommunication->m_firstSampleCounter.store(false);
                }
                if (sampleCounter != (oldSampleCounter + pCHRCommunication->m_dataAveragingBuffer))
                {
                    if (pCHRCommunication->m_dataAveragingBuffer - (65535 - oldSampleCounter) - 1 == sampleCounter) //check overflow
                    {
                        onExpectedSampleCounter();
                    }
                    else
                    {
                        if (debugConsecutiveSamplesCounter % 1000 == 0)
                        {
                            wmLog(eDebug, "oSampleCounter: %d, oOldSampleCounter: %d , debugConsecutiveSamplesCounter0: %d\n", 
                                sampleCounter, oldSampleCounter, debugConsecutiveSamplesCounter);
                            wmLogTr(eError, "QnxMsg.VI.IDMErrResNotCons", "Results of IDM are not consecutive\n");
                        }
                        debugConsecutiveSamplesCounter++;
                    }
                }
                else
                {
                    onExpectedSampleCounter();                
                }
            }
            else
            {
                //first data block, reset debug counter
                debugConsecutiveSamplesCounter = 0;
            }
            oldSampleCounter = sampleCounter;
        }
    }

    if (pCHRCommunication->isCLS2Device1Enabled())
    {
        auto pBuffer = pCHRCommunication->m_pLineBuffer.get();
        pBuffer->AddLines(p_oSampleCount, p_pSampleBuffer);
    }
}

//*****************************************************************************
//* CHRReadFunction                                                           *
//*****************************************************************************

void CHRCommunication::CHRReadFunction(void)
{
    while (true)
    {
        if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
        {
#if IDM_IS_PRESENT
            if (m_oCHRHandle != std::numeric_limits<Conn_h>::max())
            {
                Res_t oRes = ProcessDeviceOutput(m_oCHRHandle);
                if (oRes == Read_Data_Not_Enough)
                {
                    usleep(1 * 1000); // 1ms
                }
                else if (CHR_ERROR(oRes))
                {
                    wmLog(eError, "Error in ProcessDeviceOutput\n");
                    usleep(10 * 1000); // 10ms
                }
            }
            else
            {
                usleep(20 * 1000); // 20ms
            }
#endif
        }
        else
        {
            usleep(20 * 1000); // 20ms
        }
    }
}

//*****************************************************************************
//*                                                                           *
//*****************************************************************************

interface::SmpKeyValue CHRCommunication::getKeyValue(std::string key) const
{
    return m_oConfiguration.get(key);
}

interface::Configuration CHRCommunication::makeConfiguration() const
{
    return m_oConfiguration.makeConfiguration();
}

//*****************************************************************************
//*                                                                           *
//*****************************************************************************

void CHRCommunication::insertInCommandsToCHR(const std::string& commandString)
{
    const std::lock_guard<std::mutex> lock(m_oCommandsToCHRMutex);
    m_oCommandsToCHR.emplace(commandString);
    if (sem_post(&m_commandsToCHRSemaphore) != 0)
    {
        char oStrErrorBuffer[256];

        wmLog(eDebug, "sem_post failed (%s)(%s)\n", "102", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMSysErr", "Error while executing system call (%s)\n", "102");
    }
}

//*****************************************************************************
//* SetNumberOfChannels                                                       *
//*****************************************************************************

void CHRCommunication::SetNumberOfChannels(int p_oValue)
{
    if (isCLS2Device1Enabled()) // command only with CLS2 possible
    {
        std::string oCmdStrg{"NCH "};
        oCmdStrg.append(std::to_string(p_oValue));
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* SetSampleFrequency                                                        *
//*****************************************************************************

void CHRCommunication::SetSampleFrequency(int p_oValue)
{
    m_oConfigurationModified |= p_oValue != m_oConfiguration.m_oSampleFrequency;
    m_oConfiguration.m_oSampleFrequency = p_oValue;

    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        std::string oCmdStrg{"SHZ "};
        oCmdStrg.append(std::to_string(p_oValue));
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* SetLampIntensity                                                          *
//*****************************************************************************

void CHRCommunication::SetLampIntensity(int p_oValue)
{
    m_oConfigurationModified |= p_oValue != m_oConfiguration.m_oLampIntensity;
    m_oConfiguration.m_oLampIntensity = p_oValue;

    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        std::string oCmdStrg{"LAI "};
        oCmdStrg.append(std::to_string(p_oValue));
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* SetResultsOnOff                                                           *
//*****************************************************************************

void CHRCommunication::SetResultsOnOff(bool p_oOnOff)
{
    wmLog(eDebug, "CHRCommunication::SetResultsOnOff: %d\n", p_oOnOff);

    m_oResultsOnOff = p_oOnOff;

    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        if (m_oResultsOnOff) // switch on m_oResultsOnOff
        {
            if (m_oDebugInfoSampleData)
            {
                m_pResultDebugFile = fopen("ResultDebugFile.txt", "w");
                if (m_pResultDebugFile == nullptr)
                {
                    wmLog(eDebug, "Unable to open file for saving results\n");
                }
            }
            if (isCLS2Device1Enabled())
            {
                // TODO - should this be configurable?
                if (m_pLineBuffer)
                    m_pLineBuffer->Reset();

                FlushConnectionBuffer(m_oCHRHandle);
            }
            insertInCommandsToCHR("STA");
            uint32_t oHelpInt = (100000000 / m_oTriggerDistance_ns.load()) + 1;
            m_oWaitingForCommandResponseLimit.store(oHelpInt);
            m_oWaitingForCommandResponseCounter.store(0);
            m_oWaitingForCommandResponse.store(true);
        }
        else // switch off m_oResultsOnOff
        {
            insertInCommandsToCHR("STO");
            if (m_oDebugInfoSampleData)
            {
                if (m_pResultDebugFile != nullptr)
                {
                    fclose(m_pResultDebugFile);
                }
                m_pResultDebugFile = nullptr;
            }
        }
    }
}

void CHRCommunication::AskVersionString(void)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        insertInCommandsToCHR("VER");
    }
}

void CHRCommunication::setLoggingVerbosity(int value)
{
    std::string oCmdStrg{"VLV "};
    oCmdStrg.append(std::to_string(value));
    insertInCommandsToCHR(oCmdStrg);
}

void CHRCommunication::SetDetectionWindow(DWD_Window_Type p_oType, int p_oValue)
{
    if (p_oType == eDWD_Window_Left)
    {
        m_oConfigurationModified |= (m_oConfiguration.m_oDetectionWindow_Left != p_oValue);
        m_oConfiguration.m_oDetectionWindow_Left = p_oValue;
    }
    else if (p_oType == eDWD_Window_Right)
    {
        m_oConfigurationModified |= (m_oConfiguration.m_oDetectionWindow_Right != p_oValue);
        m_oConfiguration.m_oDetectionWindow_Right = p_oValue;
    }

    if (isIDMDevice1Enabled()) // command only with IDM possible
    {
        std::string oCmdStrg{"DWD "};
        oCmdStrg.append(std::to_string(m_oConfiguration.m_oDetectionWindow_Left));
        oCmdStrg.append(" ");
        oCmdStrg.append(std::to_string(m_oConfiguration.m_oDetectionWindow_Right));
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* SetQualityThreshold                                                       *
//*****************************************************************************

void CHRCommunication::SetQualityThreshold(int p_oValue)
{
    m_oConfigurationModified |= (m_oConfiguration.m_oQualityThreshold != p_oValue);
    m_oConfiguration.m_oQualityThreshold = p_oValue;

    if (isIDMDevice1Enabled()) // command only with IDM possible
    {
        std::string oCmdStrg{"QTH "};
        oCmdStrg.append(std::to_string(m_oConfiguration.m_oQualityThreshold));
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* SetDataAveraging                                                          *
//*****************************************************************************

void CHRCommunication::SetDataAveraging(int p_oValue)
{
    m_oConfigurationModified |= m_oConfiguration.m_oDataAveraging != p_oValue;
    m_oConfiguration.m_oDataAveraging = p_oValue;
    m_dataAveragingBuffer.store(p_oValue);
    if (isIDMDevice1Enabled()) // command only with IDM possible
    {
        std::string oCmdStrg{"AVD "};
        oCmdStrg.append(std::to_string(m_oConfiguration.m_oDataAveraging));
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* SetSpectralAveraging                                                      *
//*****************************************************************************

void CHRCommunication::SetSpectralAveraging(int p_oValue)
{
    m_oConfigurationModified |= m_oConfiguration.m_oSpectralAveraging != p_oValue;
    m_oConfiguration.m_oSpectralAveraging = p_oValue;

    if (isIDMDevice1Enabled()) // command only with IDM possible
    {
        std::string oCmdStrg{"AVS "};
        oCmdStrg.append(std::to_string(m_oConfiguration.m_oSpectralAveraging));
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* SetNumberOfPeaks                                                          *
//*****************************************************************************

void CHRCommunication::SetNumberOfPeaks(int p_oValue)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        std::string oCmdStrg{"NOP "};
        oCmdStrg.append(std::to_string(p_oValue));
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* SetDarkReference                                                          *
//*****************************************************************************

void CHRCommunication::SetDarkReference(void)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        insertInCommandsToCHR("DRK");
    }
}

//*****************************************************************************
//* SetFastDarkReference                                                      *
//*****************************************************************************

void CHRCommunication::SetFastDarkReference(void)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        insertInCommandsToCHR("FDK");
    }
}

//*****************************************************************************
//* SetEqualizeNoise                                                          *
//*****************************************************************************

void CHRCommunication::SetEqualizeNoise(void)
{
    if (isIDMDevice1Enabled()) // command only with IDM possible
    {
        insertInCommandsToCHR("EQN 1");
    }
}

//*****************************************************************************
//* SetRefreshOfDarkRef                                                       *
//*****************************************************************************

void CHRCommunication::SetRefreshOfDarkRef(void)
{
    if (isIDMDevice1Enabled()) // command only with IDM possible
    {
        insertInCommandsToCHR("CRDK 250");
    }
}

//*****************************************************************************
//* SetSaveSetup                                                              *
//*****************************************************************************

void CHRCommunication::SetSaveSetup(void)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        insertInCommandsToCHR("SSU");
    }
}

//*****************************************************************************
//* PerformDarkReference                                                              *
//*****************************************************************************

void CHRCommunication::PerformDarkReference(void)
{
    SetDarkReference();
    usleep(4 * 1000 * 1000); // 4 seconds delay after starting command DRK
    SetFastDarkReference();
    usleep(2 * 1000 * 1000); // 2 seconds delay after starting command FDK
    SetEqualizeNoise();
    usleep(1 * 1000 * 1000); // 1 seconds delay after starting command EQN
    SetRefreshOfDarkRef();
    usleep(1 * 1000 * 1000); // 1 seconds delay after starting command CRDK
    SetSaveSetup();
    usleep(1 * 1000 * 1000); // 1 seconds delay after starting command SSU
}

//*****************************************************************************
//* SetSLDDimmerOnOff                                                         *
//*****************************************************************************

void CHRCommunication::SetSLDDimmerOnOff(bool p_oOnOff)
{
    m_oSLDDimmerOnOff = p_oOnOff;

    if (isIDMDevice1Enabled()) // command only with IDM possible
    {
        if (m_oSLDDimmerOnOff)
        {
            insertInCommandsToCHR("SVF 3 8192 1");
        }
        else
        {
            insertInCommandsToCHR("SVF 3 8192 0");
        }
    }
}

bool CHRCommunication::GetSLDDimmerOnOff(void)
{
    return m_oSLDDimmerOnOff;
}

//*****************************************************************************
//* SetAdaptiveExposureOnOff                                                  *
//*****************************************************************************

void CHRCommunication::SetAdaptiveExposureOnOff(bool p_oOnOff)
{
    m_oAdaptiveExposureOnOff = p_oOnOff;
    SendAdaptiveExposureData();
}

bool CHRCommunication::GetAdaptiveExposureOnOff(void)
{
    return m_oAdaptiveExposureOnOff;
}

//*****************************************************************************
//* SetAdaptiveExposureBasicValue                                             *
//*****************************************************************************

void CHRCommunication::SetAdaptiveExposureBasicValue(int p_oValue)
{
    m_oAdaptiveExposureBasicValue = p_oValue;
    SendAdaptiveExposureData();
}

int CHRCommunication::GetAdaptiveExposureBasicValue(void)
{
    return m_oAdaptiveExposureBasicValue;
}

void CHRCommunication::SendAdaptiveExposureData(void)
{
    if (isIDMDevice1Enabled()) // command only with IDM possible
    {
        if (m_oAdaptiveExposureOnOff)
        {
            std::string oCmdStrg{"AAL 1 "};
            oCmdStrg.append(std::to_string(m_oAdaptiveExposureBasicValue));
            insertInCommandsToCHR(oCmdStrg);
        }
        else
        {
            insertInCommandsToCHR("AAL 0");
        }
    }
}

void CHRCommunication::queryScale()
{
    if (isIDMDevice1Enabled())
    {
        insertInCommandsToCHR("SCA");
    }
}

void CHRCommunication::setScale(int value)
{
    if (isIDMDevice1Enabled() && (m_scale != value))
    {
        m_scale = value;
    }
}

//*****************************************************************************
//* SetSignalSubscription                                                     *
//*****************************************************************************

void CHRCommunication::SetSignalSubscription(void)
{
    if (isIDMDevice1Enabled())
    {
        std::string oCmdStrg{"SODX "};
        oCmdStrg.append(std::to_string(SIGNAL_ID_SAMPLE_COUNTER));
        oCmdStrg.append(" ");
        oCmdStrg.append(std::to_string(SIGNAL_ID_DISTANCE1));
        oCmdStrg.append(" ");
        oCmdStrg.append(std::to_string(SIGNAL_ID_QUALITY1));
        insertInCommandsToCHR(oCmdStrg);
    }
    else if (isCLS2Device1Enabled())
    {
        // append global signals first !
        std::string oCmdStrg{"SODX "};
        oCmdStrg.append(std::to_string(SIGNAL_ID_SAMPLE_COUNTER));
        oCmdStrg.append(" ");
        oCmdStrg.append(std::to_string(SIGNAL_ID_DISTANCE1_INT));
        //oCmdStrg.append(" ");
        //oCmdStrg.append(std::to_string(SIGNAL_ID_QUALITY1_INT));
        insertInCommandsToCHR(oCmdStrg);
    }
    else
    {
        wmLog(eError, "Wrong CHRocodile device\n");
    }
}

//*****************************************************************************
//* Set the maximum number of profiles to send                                *
//*****************************************************************************

void CHRCommunication::SetMaxNumProfilesToSend(int p_oValue)
{
    m_oConfiguration.m_oNumProfilesToSend = p_oValue;
}

//*****************************************************************************
//* Set the maximum number of lines in buffer                                 *
//*****************************************************************************

void CHRCommunication::SetMaxBufferLines(int p_oValue)
{
    m_oConfiguration.m_oNumBuferLines = p_oValue;
}

//*****************************************************************************
//* DownloadRawSpectrum                                                       *
//*****************************************************************************

void CHRCommunication::DownloadRawSpectrum(void)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        std::string oCmdStrg{"DNLD 0"};
        if (isCLS2Device1Enabled())
        {
            oCmdStrg.append(" ");
            oCmdStrg.append(std::to_string(50)); // for CLS2 add channel number
        }
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* DownloadChromaticSpectrum                                                 *
//*****************************************************************************

void CHRCommunication::DownloadChromaticSpectrum(void)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        std::string oCmdStrg{"DNLD 1"};
        if (isCLS2Device1Enabled())
        {
            oCmdStrg.append(" ");
            oCmdStrg.append(std::to_string(50)); // for CLS2 add channel number
        }
        insertInCommandsToCHR(oCmdStrg);
    }
}

//*****************************************************************************
//* DownloadFFTSpectrum                                                       *
//*****************************************************************************

void CHRCommunication::DownloadFFTSpectrum(void)
{
    if (isIDMDevice1Enabled()) // command only with IDM possible
    {
        insertInCommandsToCHR("DNLD 2");
    }
    if (isCLS2Device1Enabled()) // hack until the raw spectrum can be selected via the GUI
    {
        DownloadRawSpectrum();
    }
}

/***************************************************************************/
/* StartCHRCyclicTaskThread                                                */
/***************************************************************************/

void CHRCommunication::StartCHRCyclicTaskThread(bool isFirstStart)
{
    ///////////////////////////////////////////////////////
    // Thread für zyklischen Ablauf starten
    ///////////////////////////////////////////////////////
    pthread_attr_t pthreadAttr;
    if (pthread_attr_init(&pthreadAttr) != 0)
    {
        char errorBuffer[256];
        wmLog(eDebug, "pthread_attr_init failed (%s)(%s)\n", "001", strerror_r(errno, errorBuffer, sizeof(errorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.SetThreadAttrFail", "Cannot set thread attributes (%s)\n", "001");
    }

    system::makeThreadRealTime(system::Priority::Sensors, &pthreadAttr);

    m_oDataToCHRCyclicTaskThread.m_pCHRCommunication = this;
    m_oDataToCHRCyclicTaskThread.isFirstStart = isFirstStart;

    if (pthread_create(&m_oCHRCyclicTaskThread_ID, &pthreadAttr, &CHRCyclicTaskThread, &m_oDataToCHRCyclicTaskThread) != 0)
    {
        char oStrErrorBuffer[256];

        wmLog(eDebug, "pthread_create failed (%s)(%s)\n", "001", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMCreateThreadFail", "Cannot start thread (%s)\n", "001");
    }
}

/***************************************************************************/
/* StopCHRCyclicTaskThread                                                 */
/***************************************************************************/

void CHRCommunication::StopCHRCyclicTaskThread(void)
{
    ///////////////////////////////////////////////////////
    // stop thread for cyclic processing
    ///////////////////////////////////////////////////////
    if (m_oCHRCyclicTaskThread_ID != 0)
    {
        if (pthread_cancel(m_oCHRCyclicTaskThread_ID) != 0)
        {
            wmLog(eDebug, "was not able to abort thread\n");
        }
        else
        {
            m_oCHRCyclicTaskThread_ID = 0;
        }
    }
}

void CHRCommunication::IncomingIDMWeldingDepth(void)
{
    const std::lock_guard<std::mutex> burstDataLock(m_oBurstDataMutex);
    if ((m_oImageNr_IDMWeldDepth < (int)m_oTriggerInterval.nbTriggers()) && (m_oSensorIdsEnabled[eIDMWeldingDepth] == true))
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            const std::lock_guard<std::mutex> IDMDistancePeak1Lock(m_IDMDistancePeak1RingBufferMutex);
            if (m_IDMDistancePeak1RingBuffer.GetWriteIndex() >= m_IDMDistancePeak1RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_IDMDistancePeak1RingBuffer.GetWriteIndex()
                           - m_IDMDistancePeak1RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_IDMDistancePeak1RingBuffer.GetBufferSize()
                           - m_IDMDistancePeak1RingBuffer.GetReadIndex()
                           + m_IDMDistancePeak1RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_IDMDistancePeak1RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_IDMDistancePeak1RingBuffer.GetValuesPerImageCycle();
            }

            for (unsigned int i = 0; i < oValuesToSend; i++)
            {
                int value = (int)m_IDMDistancePeak1RingBuffer.Read();
                if (value != 0)
                {
                    if (m_firstCallBackArrived.load())
                    {
                        m_pValues_IDMWeldDepth[0][i] = (int)value - m_oConfiguration.m_oWeldingDepthSystemOffset;
                    }
                    else
                    {
                        m_pValues_IDMWeldDepth[0][i] = IDMWELDINGDEPTH_BADVALUE;
                    }
                }
                else
                {
                    m_pValues_IDMWeldDepth[0][i] = IDMWELDINGDEPTH_BADVALUE;
                }
            }
        } // mutex can be released
        m_oTriggerContext.setImageNumber(m_oImageNr_IDMWeldDepth);
        m_rSensorProxy.data(eIDMWeldingDepth, m_oTriggerContext, image::Sample(m_pValues_IDMWeldDepth[0], oValuesToSend));
        ++m_oImageNr_IDMWeldDepth;
    }
}

void CHRCommunication::incomingIDMQualityPeak1(void)
{
    const std::lock_guard<std::mutex> burstDataLock(m_oBurstDataMutex);
    if ((m_imageNoIDMQualityPeak1 < (int)m_oTriggerInterval.nbTriggers()) && (m_oSensorIdsEnabled[eIDMQualityPeak1] == true))
    {
        unsigned int oIndexDiff;
        unsigned int oValuesToSend;
        { // scope for mutex
            const std::lock_guard<std::mutex> IDMQualityPeak1Lock(m_IDMQualityPeak1RingBufferMutex);
            if (m_IDMQualityPeak1RingBuffer.GetWriteIndex() >= m_IDMQualityPeak1RingBuffer.GetReadIndex())
            {
                oIndexDiff = m_IDMQualityPeak1RingBuffer.GetWriteIndex()
                           - m_IDMQualityPeak1RingBuffer.GetReadIndex();
            }
            else
            {
                oIndexDiff = m_IDMQualityPeak1RingBuffer.GetBufferSize()
                           - m_IDMQualityPeak1RingBuffer.GetReadIndex()
                           + m_IDMQualityPeak1RingBuffer.GetWriteIndex();
            }
            if (oIndexDiff <= m_IDMQualityPeak1RingBuffer.GetValuesPerImageCycle())
            {
                oValuesToSend = (oIndexDiff - 1);
            }
            else
            {
                oValuesToSend = m_IDMQualityPeak1RingBuffer.GetValuesPerImageCycle();
            }

            for (unsigned int i = 0; i < oValuesToSend; i++)
            {
                m_valuesIDMQualityPeak1[0][i] = (int)m_IDMQualityPeak1RingBuffer.Read();
            }
        } // mutex can be released
        m_oTriggerContext.setImageNumber(m_imageNoIDMQualityPeak1);
        m_rSensorProxy.data(eIDMQualityPeak1, m_oTriggerContext, image::Sample(m_valuesIDMQualityPeak1[0], oValuesToSend));
        ++m_imageNoIDMQualityPeak1;
    }
}

void CHRCommunication::IncomingCLSLines(void)
{
    const std::lock_guard<std::mutex> burstDataLock(m_oBurstDataMutex);
    if (m_oSendSamples_CLSLines < int(m_oTriggerInterval.nbTriggers()) && m_oSensorIdsEnabled[static_cast<interface::Sensor>(CamLaserLine)] == true)
    {
        // Nur zum Testen
        static int lastSampleCtr = -1;

        std::vector<geo2d::Doublearray> oVecDoublearray;
        if (m_pLineBuffer)
        {
            auto lines = m_pLineBuffer->GetLines(m_oConfiguration.m_oNumProfilesToSend);
            // Daten verarbeiten
            if (!lines.empty())
            {
                for (const auto& line : lines)
                {
                    // printf("%f %f %f\n", line[0], line[1], line[2]);
                    std::vector<geo2d::Doublearray> peakArrays(m_sampleSigGenInfo.PeakSignalCount);
                    for (int i = 0; i < m_sampleSigGenInfo.PeakSignalCount; ++i)
                        peakArrays[i].resize(m_sampleSigGenInfo.ChannelCount);

                    // TODO - handle global signals dynamically.
                    int currSampleCtr = static_cast<int>(line[0]);
                    if (lastSampleCtr != -1 && (currSampleCtr - lastSampleCtr) > 1)
                    {
                        printf("Jump in sample ctr %d %d\n", lastSampleCtr, currSampleCtr);
                    }
                    lastSampleCtr = currSampleCtr;

                    for (int i = 0; i < m_sampleSigGenInfo.ChannelCount; ++i)
                    {
                        for (int j = 0; j < m_sampleSigGenInfo.PeakSignalCount; ++j)
                        {
                            peakArrays[j].getData().data()[i] = line[m_sampleSigGenInfo.GlobalSignalCount + i + j];
                            peakArrays[j].getRank().data()[i] = filter::eRankMax;
                        }
                    }

                    //printf("%d %f\n", currSampleCtr, oVecDoublearray[0].getData().data()[0]);
                    for (int i = 0; i < m_sampleSigGenInfo.PeakSignalCount; ++i)
                        oVecDoublearray.emplace_back(std::move(peakArrays[i]));
                }
                m_pLineBuffer->AdvanceReader(lines.size());
            }
        }
        m_oTriggerContext.setImageNumber(m_oImageNr_CLSLines++);
        GeoVecDoublearray oGeoVecDoublearray(m_oTriggerContext, oVecDoublearray, AnalysisOK, 1.0); // default parameter 3 and 4: p_oRes=AnalysisOK, p_oRank = NotPresent
        //m_rSensorProxy.data(CamLaserLine, m_oTriggerContext, oGeoVecDoublearray);
        ++m_oSendSamples_CLSLines;
    }
}

/***************************************************************************/
/* StartCHRSpectrumTransferThread                                          */
/***************************************************************************/

void CHRCommunication::StartCHRSpectrumTransferThread(void)
{
    ///////////////////////////////////////////////////////
    // Thread für Spectrum Line starten
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToCHRSpectrumTransferThread.m_pCHRCommunication = this;
    m_oDataToCHRSpectrumTransferThread.m_pSpectrumTransferSemaphore = &m_oSpectrumTransferSemaphore;

    if (pthread_create(&m_oCHRSpectrumTransferThread_ID, &oPthreadAttr, &CHRSpectrumTransferThread, &m_oDataToCHRSpectrumTransferThread) != 0)
    {
        char oStrErrorBuffer[256];

        wmLog(eDebug, "pthread_create failed (%s)(%s)\n", "003", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMCreateThreadFail", "Cannot start thread (%s)\n", "003");
    }
}

void CHRCommunication::IncomingCHRSpectrumLine(void)
{
    const std::lock_guard<std::mutex> spectrumTransferLock(m_oSpectrumTransferMutex);
    const std::lock_guard<std::mutex> burstDataLock(m_oBurstDataMutex);
    if (m_oSendSamples_CHRSpectrumLine < int(m_oTriggerInterval.nbTriggers()) && m_oSensorIdsEnabled[eIDMSpectrumLine] == true)
    {
        for (unsigned int i = 0; i < m_oSpectrumLength[m_oSpectrumIdxToSend]; i++)
        {
            // divide by 10: bring values into the range of image pixels
            m_pValues_CHRSpectrumLine[0][i] = (m_oSpectrumArray[m_oSpectrumIdxToSend][i] / 10);
        }
        m_oTriggerContext.setImageNumber(m_oImageNr_CHRSpectrumLine++);
        m_rSensorProxy.data(eIDMSpectrumLine, m_oTriggerContext, image::Sample(m_pValues_CHRSpectrumLine[0], m_oSpectrumLength[m_oSpectrumIdxToSend]));
        ++m_oSendSamples_CHRSpectrumLine;
        m_oSpectrumIsFree[m_oSpectrumIdxToSend] = true;
    }
    else
    {
        // if there are spectrum lines received, but there ist no need to send them to InspectManager, drop them
        m_oSpectrumIsFree[m_oSpectrumIdxToSend] = true;
    }
}

/***************************************************************************/
/* StartCHRWriteThread                                                     */
/***************************************************************************/

void CHRCommunication::StartCHRWriteThread(void)
{
    ///////////////////////////////////////////////////////
    // start thread for sending commands to CHRocodile
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToCHRWriteThread.m_pCHRCommunication = this;
    m_oDataToCHRWriteThread.m_commandsToCHRSemaphore = &m_commandsToCHRSemaphore;

    if (pthread_create(&m_oCHRWriteThread_ID, &oPthreadAttr, &CHRWriteThread, &m_oDataToCHRWriteThread) != 0)
    {
        char oStrErrorBuffer[256];

        wmLog(eDebug, "pthread_create failed (%s)(%s)\n", "004", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMCreateThreadFail", "Cannot start thread (%s)\n", "004");
    }
}

void CHRCommunication::CHRWriteFunction(void)
{
    bool oEmpty{true};
    {
        const std::lock_guard<std::mutex> lock(m_oCommandsToCHRMutex);
        oEmpty = m_oCommandsToCHR.empty();
    }
    while (!oEmpty)
    {
        std::string oString{};
        {
            const std::lock_guard<std::mutex> lock(m_oCommandsToCHRMutex);
            oString = m_oCommandsToCHR.front();
            m_oCommandsToCHR.pop();
        }
        Cmd_h oCHRCommand;
        wmLog(eDebug, "Command: oString: <%s>\n", oString.c_str());
        Res_t oRes = NewCommandFromString(oString.c_str(), &oCHRCommand);
        if (!CHR_SUCCESS(oRes))
        {
            ErrorInCHRFunction(oRes, "NewCommandFromString");
        }

#if IDM_IS_PRESENT
        oRes = ExecCommandAsync(m_oCHRHandle, oCHRCommand, nullptr, nullptr, nullptr);
        if (!CHR_SUCCESS(oRes))
        {
            ErrorInCHRFunction(oRes, "ExecCommandAsync");
        }
#endif

        {
            const std::lock_guard<std::mutex> lock(m_oCommandsToCHRMutex);
            oEmpty = m_oCommandsToCHR.empty();
        }
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
void CHRCommunication::burst(const std::vector<int>& ids, TriggerContext const& context, TriggerInterval const& interval)
{
    const std::lock_guard<std::mutex> burstDataLock(m_oBurstDataMutex);
    wmLog(eDebug, "burst: nbTrig:%d, Delta[um]%d, Dist[ns]:%d, State:%d\n", interval.nbTriggers(), interval.triggerDelta(), interval.triggerDistance(), interval.state());

    m_firstCallBackArrived.store(false);

    m_oTriggerContext = context;
    m_oTriggerInterval = interval;

    m_oImageNr_IDMWeldDepth = 0;
    m_imageNoIDMQualityPeak1 = 0;
    m_oSendSamples_CHRSpectrumLine = 0;
    m_oImageNr_CHRSpectrumLine = 0;
    m_oSendSamples_CLSLines = 0;
    m_oImageNr_CLSLines = 0;

    m_oTriggerDistance_ns = m_oTriggerInterval.triggerDistance();

    cleanBuffer();

    // enable all extern sensors which are within sensor id list
    int enabledCHRsensors = 0;
    for (int id : ids)
    {
        const Sensor oId = Sensor(id);
        switch (oId)
        {
        case interface::eIDMSpectrumLine:
        case interface::eIDMTrackingLine:
        case interface::eIDMWeldingDepth:
        case interface::eIDMQualityPeak1:
            enabledCHRsensors++;
            m_oSensorIdsEnabled[oId] = true;
            break;
        //case static_cast<interface::Sensor>(CamLaserLine):
        //    enabledCHRsensors++;
        //    m_oSensorIdsEnabled[oId] = true;
        //    break;
        default:
            break;
        }

    } // for

    m_IDMDistancePeak1RingBuffer.SetValuesPerECATCycle(m_oConfiguration.m_oSampleFrequency / 1000); // values per 1ms, like with EtherCAT communication
    m_IDMDistancePeak1RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_IDMDistancePeak1RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eIDMWeldingDepth])
    {
        m_IDMDistancePeak1RingBuffer.SetReadIsActive(true);
    }

    m_IDMQualityPeak1RingBuffer.SetValuesPerECATCycle(m_oConfiguration.m_oSampleFrequency / 1000); // values per 1ms, like with EtherCAT communication
    m_IDMQualityPeak1RingBuffer.SetTriggerDist_ns(interval.triggerDistance());
    m_IDMQualityPeak1RingBuffer.StartReadOut();
    if (m_oSensorIdsEnabled[eIDMQualityPeak1])
    {
        m_IDMQualityPeak1RingBuffer.SetReadIsActive(true);
    }

    StopCHRCyclicTaskThread();
    StartCHRCyclicTaskThread(false); // repetitive start

    if (enabledCHRsensors > 1)
    {
        if (!((enabledCHRsensors == 2) &&
            (m_oSensorIdsEnabled[eIDMWeldingDepth]) &&
            ((m_oSensorIdsEnabled[eIDMQualityPeak1] || m_oSensorIdsEnabled[eIDMSpectrumLine]))))
        {
            wmLogTr(eWarning, "QnxMsg.VI.IDMSensNoWrong", "The number of CHRocodile sensors is invalid\n");
        }
    }
    if (enabledCHRsensors == 0)
    {
        m_checkSampleCounter.store(false);
        SetResultsOnOff(false);
        return;
    }
    else
    {
        m_firstSampleCounter.store(true);
        m_checkSampleCounter.store(true);
        SetResultsOnOff(true);
    }
    // if spectrum line is requested
    if (m_oSensorIdsEnabled[eIDMSpectrumLine] == true)
    {
        initBufferSpectrum();
        // reset data structures for spectrum transfer in live mode
        for (int i = 0; i < m_oNumberOfLines; i++)
        {
            for (int j = 0; j < m_oMaxNumberOfValues; j++)
            {
                m_oSpectrumArray[i][j] = 0;
            }
            m_oSpectrumLength[i] = 0;
            m_oSpectrumIsFree[i] = true;
        }
        m_oSpectrumIdxToSend = 0;
        m_oSpectrumIdxToWrite = m_oNumberOfLines - 1;
        // activate spectrum transfer in live mode
        m_oSpectrumActive = true;
    }
    if (m_oSensorIdsEnabled[eIDMWeldingDepth] == true)
    {
        initBufferWeldDepth();

        if ((!m_oCycleIsOn) && (!m_oSeamIsOn))
        {
            m_oLiveModeIsOn = true;
            startAutomaticmode(0, 0);
            start(0);
        }
    }
    // eIDMQualityPeak1 only makes sense if eIDMWeldingDepth is activated also
    if (m_oSensorIdsEnabled[eIDMQualityPeak1] == true)
    {
        initBufferQualityPeak1();
    }
} //release lock

void CHRCommunication::cancel(int id)
{
    if (m_oLiveModeIsOn)
    {
        end(0);
        stopAutomaticmode();
        m_oLiveModeIsOn = false;
    }

    m_checkSampleCounter.store(false);
    SetResultsOnOff(false); // deactivate results output of CHRocodile device

    m_IDMDistancePeak1RingBuffer.SetReadIsActive(false);
    m_IDMQualityPeak1RingBuffer.SetReadIsActive(false);

    const std::lock_guard<std::mutex> burstDataLock(m_oBurstDataMutex);
    m_oTriggerContext = TriggerContext(0, 0, 0);
    m_oTriggerInterval = TriggerInterval(0, 0);

    m_oImageNr_IDMWeldDepth = 0;
    m_imageNoIDMQualityPeak1 = 0;
    m_oSendSamples_CHRSpectrumLine = 0;
    m_oImageNr_CHRSpectrumLine = 0;
    m_oSendSamples_CLSLines = 0;
    m_oImageNr_CLSLines = 0;

    cleanBuffer();

    // reset enabled states for all extern sensors
    for (auto& rSensorIdEnabled : m_oSensorIdsEnabled)
    {
        rSensorIdEnabled.second = false;
    } // for

    m_oSpectrumActive = false;
}

//clean buffer
void CHRCommunication::cleanBuffer()
{
    if (m_pValues_IDMWeldDepth != nullptr)
    {
        for (unsigned int i = 0; i < 1; ++i)
        {
            m_pValues_IDMWeldDepth[i] = 0; // decrement smart pointer reference counter
        }
        delete[] m_pValues_IDMWeldDepth;
        m_pValues_IDMWeldDepth = nullptr;
    }
    if (m_valuesIDMQualityPeak1 != nullptr)
    {
        for (unsigned int i = 0; i < 1; ++i)
        {
            m_valuesIDMQualityPeak1[i] = 0; // decrement smart pointer reference counter
        }
        delete[] m_valuesIDMQualityPeak1;
        m_valuesIDMQualityPeak1 = nullptr;
    }
    if (m_pValues_CHRSpectrumLine != nullptr)
    {
        for (unsigned int i = 0; i < 1; ++i)
        {
            m_pValues_CHRSpectrumLine[i] = 0; // decrement smart pointer reference counter
        }
        delete[] m_pValues_CHRSpectrumLine;
        m_pValues_CHRSpectrumLine = nullptr;
    }
}

void CHRCommunication::initBufferWeldDepth()
{
    m_pValues_IDMWeldDepth = new system::TSmartArrayPtr<int>::ShArrayPtr[1];
    for (unsigned int i = 0; i < 1; ++i)
    {
        float floatHelper = static_cast<float>(m_oTriggerDistance_ns) / 1000000.0;
        unsigned int intHelper = std::ceil(floatHelper);
        m_pValues_IDMWeldDepth[i] = new int[100 * intHelper];
        for (unsigned int j = 0; j < (100 * intHelper); j++)
        {
            m_pValues_IDMWeldDepth[i][j] = 0;
        }
    }
}

void CHRCommunication::initBufferQualityPeak1()
{
    m_valuesIDMQualityPeak1 = new system::TSmartArrayPtr<int>::ShArrayPtr[1];
    for (unsigned int i = 0; i < 1; ++i)
    {
        float floatHelper = static_cast<float>(m_oTriggerDistance_ns) / 1000000.0;
        unsigned int intHelper = std::ceil(floatHelper);
        m_valuesIDMQualityPeak1[i] = new int[100 * intHelper];
        for (unsigned int j = 0; j < (100 * intHelper); j++)
        {
            m_valuesIDMQualityPeak1[i][j] = 0;
        }
    }
}

void CHRCommunication::initBufferSpectrum()
{
    m_pValues_CHRSpectrumLine = new system::TSmartArrayPtr<int>::ShArrayPtr[1];
    for (unsigned int i = 0; i < 1; ++i)
    {
        m_pValues_CHRSpectrumLine[i] = new int[5000];
        for (unsigned int j = 0; j < 5000; j++)
        {
            m_pValues_CHRSpectrumLine[i][j] = 0;
        }
    }
}

///////////////////////////////////////////////////////////
// interface functions (public member)
///////////////////////////////////////////////////////////

//*****************************************************************************
//* inspection Interface (Event) (nur Teile der Funktionen)                   *
//*****************************************************************************

void CHRCommunication::startAutomaticmode(uint32_t producttype, uint32_t productnumber) // Interface inspection
{
    wmLog(eDebug, "CHRCommunication::startAutomaticmode\n");

    m_oCycleIsOn = true;

    //tracking line it's available only with the newson scanner
}

void CHRCommunication::stopAutomaticmode() // Interface inspection
{
    wmLog(eDebug, "CHRCommunication::stopAutomaticmode\n");
    m_checkSampleCounter.store(false);
    SetResultsOnOff(false); // deactivate results output of CHRocodile device

    m_oCycleIsOn = false;
}

void CHRCommunication::start(int seamnumber) // Interface inspection
{
    wmLog(eDebug, "CHRCommunication::start\n");

    m_oSeamIsOn = true;
}

void CHRCommunication::end(int seamnumber) // Interface inspection
{
    wmLog(eDebug, "CHRCommunication::end\n");

    m_oSeamIsOn = false;
}

/***************************************************************************/
/* StartCHRReadThread                                                    */
/***************************************************************************/

void CHRCommunication::StartCHRReadThread(void)
{
    ///////////////////////////////////////////////////////
    // Thread für Empfang der IDm Daten starten
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToCHRReadThread.m_pCHRCommunication = this;

    if (pthread_create(&m_oCHRReadThread_ID, &oPthreadAttr, &CHRReadThread, &m_oDataToCHRReadThread) != 0)
    {
        char oStrErrorBuffer[256];

        wmLog(eDebug, "pthread_create failed (%s)(%s)\n", "004", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMCreateThreadFail", "Cannot start thread (%s)\n", "004");
    }
}

/***************************************************************************/
/* CHRReadThread                                                         */
/***************************************************************************/

void* CHRReadThread(void* p_pArg)
{
    prctl(PR_SET_NAME, "Answer");
    system::makeThreadRealTime(system::Priority::Sensors);
    auto pDataToCHRReadThread = static_cast<struct DataToCHRReadThread*>(p_pArg);
    auto pCHRCommunication = pDataToCHRReadThread->m_pCHRCommunication;

    wmLog(eDebug, "CHRReadThread is started\n");

    usleep(5000 * 1000); // 5 seconds delay before starting to read from device

    wmLog(eDebug, "CHRReadThread is active\n");

    pCHRCommunication->CHRReadFunction();

    return nullptr;
}

/***************************************************************************/
/* CHRCyclicTaskThread                                                     */
/***************************************************************************/

void CHRCyclicTaskThreadWork(CHRCommunication* pCHRCommunication)
{
    // send value for Welding Depth via sensor interface to InspectManager
    if (pCHRCommunication->isIDMDevice1Enabled())
    {
        pCHRCommunication->IncomingIDMWeldingDepth();
        pCHRCommunication->incomingIDMQualityPeak1();
    }

    // send lines from CLS2
    if (pCHRCommunication->isCLS2Device1Enabled())
    {
        pCHRCommunication->IncomingCLSLines();
    }

    if (pCHRCommunication->m_oSpectrumActive)
    {
        pCHRCommunication->DownloadFFTSpectrum();
    }

    pCHRCommunication->CheckCommandResponse();
}

// Thread Funktion muss ausserhalb der Klasse sein
void* CHRCyclicTaskThread(void* p_pArg)
{
    prctl(PR_SET_NAME, "CyclicTask");
    pthread_detach(pthread_self());
    struct timespec oWakeupTime;
    int retValue;

    auto pDataToCHRCyclicTaskThread = static_cast<struct DataToCHRCyclicTaskThread*>(p_pArg);
    auto pCHRCommunication = pDataToCHRCyclicTaskThread->m_pCHRCommunication;
    bool isFirstStart = pDataToCHRCyclicTaskThread->isFirstStart;

    wmLog(eDebug, "CHRCyclicTaskThread is started\n");

    if (isFirstStart)
    {
        usleep(2000 * 1000); // 2 seconds delay before starting to feed interfaces

        clock_gettime(CLOCK_TO_USE, &oWakeupTime);
        oWakeupTime.tv_sec += 1; // start in future
        oWakeupTime.tv_nsec = 0;
    }
    else
    {
        // calculate time for next send
        clock_gettime(CLOCK_TO_USE, &oWakeupTime);
        oWakeupTime.tv_nsec += pCHRCommunication->m_oTriggerDistance_ns;
        while(oWakeupTime.tv_nsec >= NSEC_PER_SEC)
        {
            oWakeupTime.tv_nsec -= NSEC_PER_SEC;
            oWakeupTime.tv_sec++;
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
        CHRCyclicTaskThreadWork(pCHRCommunication);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
    }

    wmLog(eDebug, "CHRCyclicTaskThread is active\n");

    while (true)
    {
        retValue = clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &oWakeupTime, nullptr);
        if (retValue)
        {
            char oStrErrorBuffer[256];

            wmLog(eDebug, "clock_nanosleep failed (%s)\n", strerror_r(retValue, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
            wmLogTr(eError, "QnxMsg.VI.IDMCyclicSleepFail", "Sleeping time for cycle loop failed\n");
            break;
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
        CHRCyclicTaskThreadWork(pCHRCommunication);
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);

        // calculate time for next send
        oWakeupTime.tv_nsec += pCHRCommunication->m_oTriggerDistance_ns;
        while (oWakeupTime.tv_nsec >= NSEC_PER_SEC)
        {
            oWakeupTime.tv_nsec -= NSEC_PER_SEC;
            oWakeupTime.tv_sec++;
        }
    }

    return nullptr;
}

/***************************************************************************/
/* CHRSpectrumTransferThread                                               */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void* CHRSpectrumTransferThread(void* p_pArg)
{
    prctl(PR_SET_NAME, "SpectrumTransfer");
    system::makeThreadRealTime(system::Priority::Sensors);
    struct DataToCHRSpectrumTransferThread* pDataToCHRSpectrumTransferThread;
    CHRCommunication* pCHRCommunication;
    sem_t* pSpectrumTransferSemaphore;

    pDataToCHRSpectrumTransferThread = static_cast<struct DataToCHRSpectrumTransferThread*>(p_pArg);
    pCHRCommunication = pDataToCHRSpectrumTransferThread->m_pCHRCommunication;
    pSpectrumTransferSemaphore = pDataToCHRSpectrumTransferThread->m_pSpectrumTransferSemaphore;

    wmLog(eDebug, "CHRSpectrumTransferThread is started\n");

    usleep(2500 * 1000);

    wmLog(eDebug, "CHRSpectrumTransferThread is active\n");

    while (true)
    {
        if (sem_wait(pSpectrumTransferSemaphore) != 0)
        {
            char oStrErrorBuffer[256];

            wmLog(eDebug, "sem_wait failed (%s)(%s)\n", "014", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
            wmFatal(eInternalError, "QnxMsg.VI.IDMSysErr", "Error while executing system call (%s)\n", "014");
        }
        // send values for Spectrum Line via sensor interface to InspectManager
        pCHRCommunication->IncomingCHRSpectrumLine();
    }

    return nullptr;
}

/***************************************************************************/
/* CHRWriteThread                                                          */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void* CHRWriteThread(void* p_pArg)
{
    prctl(PR_SET_NAME, "CHRWriteThread");
    system::makeThreadRealTime(system::Priority::Sensors);
    struct DataToCHRWriteThread* pDataToCHRWriteThread;
    CHRCommunication* pCHRCommunication;
    sem_t* commandsToCHRSemaphore;

    pDataToCHRWriteThread = static_cast<struct DataToCHRWriteThread*>(p_pArg);
    pCHRCommunication = pDataToCHRWriteThread->m_pCHRCommunication;
    commandsToCHRSemaphore = pDataToCHRWriteThread->m_commandsToCHRSemaphore;

    wmLog(eDebug, "CHRWriteThread is started\n");

    usleep(1000 * 1000); // 1s

    wmLog(eDebug, "CHRWriteThread is active\n");

    while (true)
    {
        if (sem_wait(commandsToCHRSemaphore) != 0)
        {
            char oStrErrorBuffer[256];

            wmLog(eDebug, "sem_wait failed (%s)(%s)\n", "014", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
            wmFatal(eInternalError, "QnxMsg.VI.IDMSysErr", "Error while executing system call (%s)\n", "014");
        }
        pCHRCommunication->CHRWriteFunction();
    }

    return nullptr;
}

void CHRCommunication::initConfiguration()
{
    std::string oWMBaseDir = getenv("WM_BASE_DIR");
    if (oWMBaseDir.empty())
    {
        wmLog(eError, "WM Base Dir not defined\n");
        oWMBaseDir = "/tmp/precitec";
    }
    m_oConfigFile = oWMBaseDir + "/config/OCT.xml";
    bool readConfigOk = m_oConfiguration.initFromFile(m_oConfigFile);
    if (!readConfigOk)
    {
        //error when reading the file, m_oConfiguration now is set to the default values
        // we create a config file
        m_oConfiguration.writeToFile(m_oConfigFile);
        wmLog(eInfo, "Create config file %s with default values \n", m_oConfigFile);
    }
    m_oConfigurationModified = false;
}

void CHRCommunication::saveConfigurationToFile()
{
    if (m_oConfigurationModified)
    {
        m_oConfiguration.writeToFile(m_oConfigFile);
        m_oConfigurationModified = false;
    }
}

void CHRCommunication::SetDirectCHRCommand(const PvString& p_oCommand)
{
    if (p_oCommand.size() != 0)
    {
        printf("Direct CHR command: <%s>\n", p_oCommand.c_str());
        if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
        {
            insertInCommandsToCHR(p_oCommand);
        }
    }
    else
    {
        printf("Direct CHR command is empty\n");
    }
}

void CHRCommunication::TestFunction2(void)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        insertInCommandsToCHR("IDE");
    }
}

void CHRCommunication::TestFunction3(void)
{
    if ((isIDMDevice1Enabled()) || (isCLS2Device1Enabled()))
    {
        insertInCommandsToCHR("CONF");
    }
}

void CHRCommunication::TestFunction4(void)
{
}

void CHRCommunication::TestFunction5(void)
{
}

void CHRCommunication::TestFunction6(void)
{
}

void CHRCommunication::SetRescaleFactorCHRResults(int p_oValue)
{
    m_oConfigurationModified |= (m_oRescaleFactorCHRResults != p_oValue);
    m_oRescaleFactorCHRResults = p_oValue;
}

void CHRCommunication::SetWeldingDepthSystemOffset(int p_oValue)
{
    m_oConfigurationModified |= (m_oConfiguration.m_oWeldingDepthSystemOffset != p_oValue);
    m_oConfiguration.m_oWeldingDepthSystemOffset = p_oValue;
}

void CHRCommunication::CommandResponseOK(void)
{
    m_oWaitingForCommandResponse.store(false);
}

void CHRCommunication::CheckCommandResponse(void)
{
    if (m_oWaitingForCommandResponse.load())
    {
        m_oWaitingForCommandResponseCounter++;
        if (m_oWaitingForCommandResponseCounter.load() > m_oWaitingForCommandResponseLimit.load())
        {
            wmFatal(eExtEquipment, "QnxMsg.VI.IDMErrTCPSend", "Unable to send data to the IDM device (%s)\n", "006");
            m_oWaitingForCommandResponse.store(false);
        }
    }
}

} // namespace grabber

} // namespace precitec
