/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2023
 *  @brief      Handles the TCP/IP communication with LWM
 */

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <chrono>

#include "module/moduleLogger.h"
#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"

#include "viInspectionControl/TCPClientLWM.h"

namespace precitec
{

namespace ethercat
{

using namespace interface;
const int TCPClientLWM::runFunctionTimeout = 20; // milliseconds
///////////////////////////////////////////////////////////
// prototypes for thread functions
///////////////////////////////////////////////////////////

// Thread function must be outside of the class
void* TCPClientSendThread (void *p_pArg);
void* TCPClientReceiveThread (void *p_pArg);

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

TCPClientLWM::TCPClientLWM(void)
    : m_TCPClientSendThreadId(0)
    , m_LWMDeviceIpAddressInt(0x17AAA8C0) // 192.168.170.23 in network byte order
{
    // SystemConfig Switches for LWM communication
    m_LWMDeviceIpAddress = SystemConfiguration::instance().getString("LWM_Device_IP_Address", "192.168.170.23");
    wmLog(eDebug, "m_LWMDeviceIpAddress: <%s>\n", m_LWMDeviceIpAddress.c_str());
    m_LWMDeviceTCPPort = SystemConfiguration::instance().getInt("LWM_Device_TCP_Port", 2400);
    wmLog(eDebug, "m_LWMDeviceTCPPort: <%04X>\n", m_LWMDeviceTCPPort);

    int returnValue = inet_pton(AF_INET, m_LWMDeviceIpAddress.c_str(), &m_LWMDeviceIpAddressInt);
    if (returnValue != 1)
    {
        wmLogTr(eError, "QnxMsg.VI.TCPWrongIPFormat", "wrong format of IP address string\n");
        m_LWMDeviceIpAddressInt = 0x17AAA8C0; // 192.168.170.23 in network byte order
    }
    char helpStrg[21];
    sprintf(helpStrg, "%08X", m_LWMDeviceIpAddressInt);
    wmLog(eDebug, "m_LWMDeviceIpAddressInt: %s, retVal: %d\n", helpStrg, returnValue); // in network byte order

    startTCPClientSendThread();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

TCPClientLWM::~TCPClientLWM(void)
{
    if (m_TCPClientReceiveThreadId != 0)
    {
        if (pthread_cancel(m_TCPClientReceiveThreadId) != 0)
        {
            wmLog(eDebug, "TCPClientLWM: was not able to abort thread: %s (%d)\n", strerror(errno), errno);
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(101)");
        }
        else
        {
            pthread_join(m_TCPClientReceiveThreadId, nullptr);
            m_TCPClientReceiveThreadId = 0;
        }
    }

    if (m_TCPClientSendThreadId != 0)
    {
        if (pthread_cancel(m_TCPClientSendThreadId) != 0)
        {
            wmLog(eDebug, "TCPClientLWM: was not able to abort thread: %s (%d)\n", strerror(errno), errno);
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(102)");
        }
        else
        {
            pthread_join(m_TCPClientSendThreadId, nullptr);
            m_TCPClientSendThreadId = 0;
        }
    }
}

void TCPClientLWM::connectSimpleIOSelectionCallback(TCPCallbackFunctionType1 simpleIOSelectionCallback)
{
    m_simpleIOSelectionCallback = simpleIOSelectionCallback;
}

void TCPClientLWM::connectSimpleIOStoppCallback(TCPCallbackFunctionType1 simpleIOStoppCallback)
{
    m_simpleIOStoppCallback = simpleIOStoppCallback;
}

void TCPClientLWM::connectSimpleIOTriggerCallback(TCPCallbackFunctionType1 simpleIOTriggerCallback)
{
    m_simpleIOTriggerCallback = simpleIOTriggerCallback;
}

void TCPClientLWM::connectSimpleIOErrorCallback(TCPCallbackFunctionType1 simpleIOErrorCallback)
{
    m_simpleIOErrorCallback = simpleIOErrorCallback;
}

void TCPClientLWM::connectResultsRangesCallback(TCPCallbackFunctionType1 resultsRangesCallback)
{
    m_resultsRangesCallback = resultsRangesCallback;
}

void TCPClientLWM::connectConnectionOKCallback(TCPCallbackFunctionType1 connectionOKCallback)
{
    m_connectionOKCallback = connectionOKCallback;
}

void TCPClientLWM::connectConnectionNOKCallback(TCPCallbackFunctionType1 connectionNOKCallback)
{
    m_connectionNOKCallback = connectionNOKCallback;
}

/***************************************************************************/
/* startTCPClientSendThread                                                */
/***************************************************************************/

void TCPClientLWM::startTCPClientSendThread(void)
{
    ///////////////////////////////////////////////////////
    // Thread für TCPClientSend starten
    ///////////////////////////////////////////////////////
    pthread_attr_t pthreadAttr;
    pthread_attr_init(&pthreadAttr);

    m_dataToTCPClientSendThread.m_TCPClientLWM = this;

    if (pthread_create(&m_TCPClientSendThreadId, &pthreadAttr, &TCPClientSendThread, &m_dataToTCPClientSendThread) != 0)
    {
        wmLog(eDebug, "TCPClientLWM: was not able to create thread: %s (%d)\n", strerror(errno), errno);
        wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(103)");
    }
}

/***************************************************************************/
/* startTCPClientReceiveThread                                             */
/***************************************************************************/

void TCPClientLWM::startTCPClientReceiveThread(void)
{
    ///////////////////////////////////////////////////////
    // Thread für TCPClientReceive starten
    ///////////////////////////////////////////////////////
    pthread_attr_t pthreadAttr;
    pthread_attr_init(&pthreadAttr);

    m_dataToTCPClientReceiveThread.m_TCPClientLWM = this;

    if (pthread_create(&m_TCPClientReceiveThreadId, &pthreadAttr, &TCPClientReceiveThread, &m_dataToTCPClientReceiveThread) != 0)
    {
        wmLog(eDebug, "TCPClientLWM: was not able to create thread: %s (%d)\n", strerror(errno), errno);
        wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(104)");
    }
}

/***************************************************************************/
/* stopTCPClientReceiveThread                                              */
/***************************************************************************/

void TCPClientLWM::stopTCPClientReceiveThread(void)
{
    if (m_TCPClientReceiveThreadId != 0)
    {
        if (pthread_cancel(m_TCPClientReceiveThreadId) != 0)
        {
            wmLog(eDebug, "TCPClientLWM: was not able to cancel thread: %s (%d)\n", strerror(errno), errno);
            wmFatal(eBusSystem, "QnxMsg.VI.RestartTCPCommFault", "Problem while restarting TCP/IP communication %s\n", "(001)");
        }
        else
        {
            pthread_join(m_TCPClientReceiveThreadId, nullptr);
            m_TCPClientReceiveThreadId = 0;
        }
    }
}

int TCPClientLWM::run(void)
{
    /****** main loop ********************************************************/

    sleep(8);

    while (true)
    {
        while (!m_connectionIsOn)
        {
            [[maybe_unused]] int returnValue = openCommunicationToLWM();
        }

        {
            std::unique_lock<std::mutex> lock(*m_LWMSendRequestMutex);
            if (m_LWMSendRequestCondVar->wait_for(lock, std::chrono::milliseconds(runFunctionTimeout), [this]{return *m_LWMSendRequestVariable;}))
            {
                // got request, reset SendRequestVariable
                *m_LWMSendRequestVariable = false;
            }
        }

        // check if a block has to be sent
        checkSendRequests();

        // check if watchdog answer is received in time
        checkLWMWatchog();

        // check if a reconnect trial is necessary
        if (m_backToReconnect.load())
        {
            // close active socket
            shutdown(m_socket, SHUT_RDWR);
            stopTCPClientReceiveThread();
            close(m_socket);
            // start new connect
            m_connectionIsOn = false;
            // inform InspectionControl
            if (m_connectionNOKCallback)
            {
                m_connectionNOKCallback();
            }
            m_backToReconnect.store(false);
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////
// openCommunicationToLWM
///////////////////////////////////////////////////////////

int TCPClientLWM::openCommunicationToLWM(void)
{
    static int trialCounter{0};

    // create socket
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == -1)
    {
        wmLog(eDebug, "TCPClientLWM: Error while creating socket: %s (%d)\n", strerror(errno), errno);
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(101)");
        return -1;
    }

    struct linger lingerStruct;
    lingerStruct.l_onoff = 1;
    lingerStruct.l_linger = 2; // 2 seconds
    setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (const char*)&lingerStruct, sizeof lingerStruct);

    struct timeval tcpTimeout;
    tcpTimeout.tv_sec = 2;
    tcpTimeout.tv_usec = 0;
    setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tcpTimeout, sizeof tcpTimeout);
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tcpTimeout, sizeof tcpTimeout);

    struct sockaddr_in serverAddr;
    memset((char *)&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    memcpy(&serverAddr.sin_addr, &m_LWMDeviceIpAddressInt, 4); // in network byte order
    serverAddr.sin_port = htons(m_LWMDeviceTCPPort);

    trialCounter++;
    // connect to Server
    if (connect(m_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        if (errno == ECONNREFUSED)
        {
            wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(001)");
            if (trialCounter > 10)
            {
                wmLogTr(eError, "QnxMsg.VI.NoConnToLWM", "There is no connection to the LWM device %s\n", "(001)");
                trialCounter = 0;
            }
            close (m_socket);
            sleep(2);
            return -2;
        }
        else if (errno == ETIMEDOUT)
        {
            wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(002)");
            if (trialCounter > 10)
            {
                wmLogTr(eError, "QnxMsg.VI.NoConnToLWM", "There is no connection to the LWM device %s\n", "(002)");
                trialCounter = 0;
            }
            close (m_socket);
            return -2;
        }
        else if (errno == EINPROGRESS)
        {
            wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(003)");
            if (trialCounter > 10)
            {
                wmLogTr(eError, "QnxMsg.VI.NoConnToLWM", "There is no connection to the LWM device %s\n", "(003)");
                trialCounter = 0;
            }
            close (m_socket);
            return -2;
        }
        else if (errno == EHOSTUNREACH)
        {
            wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(004)");
            if (trialCounter > 10)
            {
                wmLogTr(eError, "QnxMsg.VI.NoConnToLWM", "There is no connection to the LWM device %s\n", "(004)");
                trialCounter = 0;
            }
            close (m_socket);
            return -2;
        }
        else
        {
            wmLog(eDebug, "TCPClientLWM: Error while connecting server: %s (%d)\n", strerror(errno), errno);
            close (m_socket);
            wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(102)");
            return -2;
        }
    }
    wmLogTr(eInfo, "QnxMsg.VI.TCPCommStarted", "TCP/IP Connection started %s\n", "(Client)");

    tcpTimeout.tv_sec = 1;
    tcpTimeout.tv_usec = 0;
    setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tcpTimeout, sizeof tcpTimeout);
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tcpTimeout, sizeof tcpTimeout);

    trialCounter = 0;
    startTCPClientReceiveThread();
    m_connectionIsOn = true;
    if (m_connectionOKCallback)
    {
        m_connectionOKCallback();
    }
    return 0;
}

///////////////////////////////////////////////////////////
// checkSendRequests
///////////////////////////////////////////////////////////

void TCPClientLWM::checkSendRequests(void)
{
    if (!m_connectionIsOn)
    {
        return;
    }

    if (m_sendRequestSimpleIOSelection.load())
    {
        sendSimpleIOSelection();
        m_sendRequestSimpleIOSelection.store(false);
    }

    if (m_sendRequestSimpleIOStopp.load())
    {
        sendSimpleIOStopp();
        m_sendRequestSimpleIOStopp.store(false);
    }

    if (m_sendRequestWatchdog.load())
    {
        sendWatchdog();
        m_sendRequestWatchdog.store(false);
    }

    if (m_sendRequestSignOffClient.load())
    {
        sendSignOffClient();
        m_sendRequestSignOffClient.store(false);
    }
}

void TCPClientLWM::setSendRequestSimpleIOSelection(const SimpleIOSelectionDataType& simpleIOSelectionData)
{
    {
        const std::lock_guard<std::mutex> lock(m_simpleIOSelectionMutex);
        m_simpleIOSelectionData = simpleIOSelectionData;
    }
    m_sendRequestSimpleIOSelection.store(true);
}

void TCPClientLWM::setSendRequestSimpleIOStopp(const bool requestAcknowledge)
{
    {
        const std::lock_guard<std::mutex> lock(m_simpleIOStoppMutex);
        m_simpleIOStoppRequestAcknowledge = requestAcknowledge;
    }
    m_sendRequestSimpleIOStopp.store(true);
}

void TCPClientLWM::setSendRequestWatchdog(void)
{
    m_sendRequestWatchdog.store(true);
}

void TCPClientLWM::setSendRequestSignOffClient(void)
{
    m_sendRequestSignOffClient.store(true);
}

int32_t TCPClientLWM::getErrorStatus(void)
{
    return m_errorStatus;
}

void TCPClientLWM::getResultsRanges(ResultsRangesDataType& resultsRanges)
{
    resultsRanges = m_resultsRanges;
}

///////////////////////////////////////////////////////////
// sendSimpleIOSelection
///////////////////////////////////////////////////////////

void TCPClientLWM::sendSimpleIOSelection(void)
{
    TCPDataBlockType sendBuffer;
    char statusStrg[121];
    int lengthToSend;

    int32_t* pint32;
    char* pchar;
    int32_t* datablockStart;

    SimpleIOSelectionDataType simpleIOSelectionData{};
    {
        const std::lock_guard<std::mutex> lock(m_simpleIOSelectionMutex);
        simpleIOSelectionData = m_simpleIOSelectionData;
    }

    // write header

    pint32 = reinterpret_cast<int32_t*>(sendBuffer.data());
    *pint32 = LWM_SIMPLE_IO_SELECTION;  // Telegram-ID
    pint32++;
    *pint32 = 0;                        // status
    pint32++;
    *pint32 = 24;                       // placeholder for length, will be correctly filled later
    pint32++;

    // write datablock

    datablockStart = pint32;

    if (simpleIOSelectionData.m_requestAcknowledge)
    {
        *pint32 = 1;                    // Request Acknowledge
    }
    else
    {
        *pint32 = 0;                    // Request Acknowledge
    }
    pint32++;
    if (simpleIOSelectionData.m_systemActivated)
    {
        *pint32 = 1;                    // System Activated
    }
    else
    {
        *pint32 = 0;                    // System Activated
    }
    pint32++;
    if ((simpleIOSelectionData.m_programNumber >= 0) && (simpleIOSelectionData.m_programNumber <= 255))
    {
        *pint32 = simpleIOSelectionData.m_programNumber; // Program Nummer
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.WrongPrgNoLWM", "Wrong program number for LWM device (%d)\n", simpleIOSelectionData.m_programNumber);
        *pint32 = 0;                    // Program Nummer
    }
    pint32++;

    if (simpleIOSelectionData.m_comment.length() <= 78)
    {
        *pint32 = simpleIOSelectionData.m_comment.length(); // length of comment
        pint32++;

        pchar = reinterpret_cast<char*>(pint32);
        for (unsigned int i = 0; i < simpleIOSelectionData.m_comment.length(); i++)
        {
            *pchar = simpleIOSelectionData.m_comment[i]; // comment
            pchar++;
        }
        pint32 = reinterpret_cast<int32_t*>(pchar);
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.CommentLenLWM", "Comment string for LWM device is too long\n");
        *pint32 = 78; // length of comment
        pint32++;

        pchar = reinterpret_cast<char*>(pint32);
        for (unsigned int i = 0; i < 78; i++)
        {
            *pchar = simpleIOSelectionData.m_comment[i]; // comment
            pchar++;
        }
        pint32 = reinterpret_cast<int32_t*>(pchar);
    }

    *pint32 = 0;                        // use sequences
    pint32++;
    *pint32 = 0;                        // length of sequences space
    pint32++;

    int32_t dataBlockLength = (char *)pint32 - (char *)datablockStart;

    pint32 = (int32_t *) &sendBuffer[8];
    *pint32 = (int32_t) dataBlockLength; // insert correctly length

    lengthToSend = dataBlockLength + 12;

    if (send(m_socket, sendBuffer.data(), lengthToSend, 0) == -1)
    {
        wmLog(eDebug, "TCPClientLWM: was not able to send via TCP/IP: %s (%d)\n", strerror(errno), errno);
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(103)");
        return;
    }

    int32_t *telegram = (int32_t *) &sendBuffer[0];
    int32_t *status = (int32_t *) &sendBuffer[4];
    int32_t *length = (int32_t *) &sendBuffer[8];
    sprintf(statusStrg, "TCPClientLWM: sent:  telegram = %08X , status = %08X , length = %d\n", *telegram, *status, *length);
    wmLog(eDebug, statusStrg);
}

///////////////////////////////////////////////////////////
// sendSimpleIOStopp
///////////////////////////////////////////////////////////

void TCPClientLWM::sendSimpleIOStopp(void)
{
    TCPDataBlockType sendBuffer;
    char statusStrg[121];
    int lengthToSend;

    int32_t* pint32;
    int32_t* datablockStart;

    bool simpleIOStoppRequestAcknowledge;
    {
        const std::lock_guard<std::mutex> lock(m_simpleIOSelectionMutex);
        simpleIOStoppRequestAcknowledge = m_simpleIOStoppRequestAcknowledge;
    }

    // write header

    pint32 = reinterpret_cast<int32_t*>(sendBuffer.data());
    *pint32 = LWM_SIMPLE_IO_STOPP;      // Telegram-ID
    pint32++;
    *pint32 = 0;                        // status
    pint32++;
    *pint32 = 4;                        // placeholder for length, will be correctly filled later
    pint32++;

    // write datablock

    datablockStart = pint32;

    if (simpleIOStoppRequestAcknowledge)
    {
        *pint32 = 1;                    // Request Acknowledge
    }
    else
    {
        *pint32 = 0;                    // Request Acknowledge
    }
    pint32++;

    int32_t dataBlockLength = (char *)pint32 - (char *)datablockStart;

    pint32 = (int32_t *) &sendBuffer[8];
    *pint32 = (int32_t) dataBlockLength; // insert correctly length

    lengthToSend = dataBlockLength + 12;

    if (send(m_socket, sendBuffer.data(), lengthToSend, 0) == -1)
    {
        wmLog(eDebug, "TCPClientLWM: was not able to send via TCP/IP: %s (%d)\n", strerror(errno), errno);
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(104)");
        return;
    }

    int32_t *telegram = (int32_t *) &sendBuffer[0];
    int32_t *status = (int32_t *) &sendBuffer[4];
    int32_t *length = (int32_t *) &sendBuffer[8];
    sprintf(statusStrg, "TCPClientLWM: sent:  telegram = %08X , status = %08X , length = %d\n", *telegram, *status, *length);
    wmLog(eDebug, statusStrg);
}

///////////////////////////////////////////////////////////
// sendWatchdog
///////////////////////////////////////////////////////////

void TCPClientLWM::sendWatchdog(void)
{
    TCPDataBlockType sendBuffer;
    char statusStrg[121];
    int lengthToSend;

    int32_t* pint32;

    // write header

    pint32 = reinterpret_cast<int32_t*>(sendBuffer.data());
    *pint32 = LWM_WATCHDOG;             // Telegram-ID
    pint32++;
    *pint32 = 0;                        // status
    pint32++;
    *pint32 = 0;                        // placeholder for length, will be correctly filled later
    pint32++;

    lengthToSend = 12;

    if (send(m_socket, sendBuffer.data(), lengthToSend, 0) == -1)
    {
        wmLog(eDebug, "TCPClientLWM: was not able to send via TCP/IP: %s (%d)\n", strerror(errno), errno);
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(105)");
        return;
    }

    m_watchdogTelegramReceived.store(false);
    m_watchdogTelegramSent.store(true);

    int32_t *telegram = (int32_t *) &sendBuffer[0];
    int32_t *status = (int32_t *) &sendBuffer[4];
    int32_t *length = (int32_t *) &sendBuffer[8];
    sprintf(statusStrg, "TCPClientLWM: sent:  telegram = %08X , status = %08X , length = %d\n", *telegram, *status, *length);
    wmLog(eDebug, statusStrg);
}

void TCPClientLWM::checkLWMWatchog(void)
{
    static int loopCounter{0};

    if (m_watchdogTelegramSent.load())
    {
        loopCounter++;
        if (loopCounter > (40 / runFunctionTimeout)) // after 40ms watchdog answer should be arrived
        {
            wmLogTr(eError, "QnxMsg.VI.NoWatchdogAnsLWM", "Can't receive watchdog answer from LWM\n");
            m_watchdogTelegramSent.store(false);
            loopCounter = 0;
            m_backToReconnect.store(true);
        }
    }
    if (loopCounter != 0)
    {
        if (m_watchdogTelegramReceived.load())
        {
            loopCounter = 0;
        }
    }
}

///////////////////////////////////////////////////////////
// sendSignOffClient
///////////////////////////////////////////////////////////

void TCPClientLWM::sendSignOffClient(void)
{
    TCPDataBlockType sendBuffer;
    char statusStrg[121];
    int lengthToSend;

    int32_t* pint32;

    // write header

    pint32 = reinterpret_cast<int32_t*>(sendBuffer.data());
    *pint32 = LWM_SIGN_OFF_CLIENT;      // Telegram-ID
    pint32++;
    *pint32 = 0;                        // status
    pint32++;
    *pint32 = 0;                        // placeholder for length, will be correctly filled later
    pint32++;

    lengthToSend = 12;

    if (send(m_socket, sendBuffer.data(), lengthToSend, 0) == -1)
    {
        wmLog(eDebug, "TCPClientLWM: was not able to send via TCP/IP: %s (%d)\n", strerror(errno), errno);
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(106)");
        return;
    }

    int32_t *telegram = (int32_t *) &sendBuffer[0];
    int32_t *status = (int32_t *) &sendBuffer[4];
    int32_t *length = (int32_t *) &sendBuffer[8];
    sprintf(statusStrg, "TCPClientLWM: sent:  telegram = %08X , status = %08X , length = %d\n", *telegram, *status, *length);
    wmLog(eDebug, statusStrg);
}

///////////////////////////////////////////////////////////
// receiveFunction
///////////////////////////////////////////////////////////

int TCPClientLWM::receiveFunction(void)
{
    while (true)
    {
        TCPDataBlockType recvBuffer{};

        int retValue = recv(m_socket, recvBuffer.data(), recvBuffer.size(), 0);
        if (retValue < 0)
        {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
            {
                // Receive timeout has expired, no further action
                continue;
            }
            else
            {
                wmLog(eDebug, "TCPClientLWM: was not able to recv via TCP/IP: %s (%d)\n", strerror(errno), errno);
                wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(107)");
                continue;
            }
        }
        else if (retValue == 0) // EOF from LWM
        {
            m_backToReconnect.store(true);
            return 1; // this ends the TCPClientReceiveThread
        }

        char statusStrg[80]{};
        int32_t *telegram = (int32_t *) &recvBuffer[0];
        int32_t *status = (int32_t *) &recvBuffer[4];
        int32_t *length = (int32_t *) &recvBuffer[8];
        sprintf(statusStrg, "TCPClientLWM: recvd: telegram = %08X , status = %08X , length = %d\n", *telegram, *status, *length);
        wmLog(eDebug, statusStrg);

        switch (*telegram)
        {
            case LWM_RESULT_RANGES: // Receive results with ranges
                unpackResultsRanges(recvBuffer);
                if (m_resultsRangesCallback)
                {
                    m_resultsRangesCallback();
                }
                break;
            case LWM_RESULT_VALUES: // Receive results with values
                break;
            case LWM_RESULT_VALUES_RANGES: // Receive results with values and ranges
                unpackResultsValuesRanges(recvBuffer);
                break;

            case LWM_SIMPLE_IO_SELECTION: // Acknowledge Simple I/O Selection
                unpackAcknSimpleIOSelection(recvBuffer);
                if (m_simpleIOSelectionCallback)
                {
                    m_simpleIOSelectionCallback();
                }
                break;
            case LWM_SIMPLE_IO_STOPP: // Acknowledge Simple I/O Stopp
                unpackAcknSimpleIOStopp(recvBuffer);
                if (m_simpleIOStoppCallback)
                {
                    m_simpleIOStoppCallback();
                }
                break;
            case LWM_SIMPLE_IO_ERROR: // Receive Simple I/O Error
                unpackSimpleIOError(recvBuffer);
                if (m_simpleIOErrorCallback)
                {
                    m_simpleIOErrorCallback();
                }
                break;
            case LWM_SIMPLE_IO_TRIGGER: // Receive Simple I/O Trigger OK
                unpackSimpleIOTrigger(recvBuffer);
                if (m_simpleIOTriggerCallback)
                {
                    m_simpleIOTriggerCallback();
                }
                break;

            case LWM_WATCHDOG: // Acknowledge Watchdog
                unpackWatchdog(recvBuffer);
                break;

            default:
                {
                    char telegramString[21];
                    sprintf(telegramString, "%08X", *telegram);
                    wmLog(eDebug, "unknown telegram via TCP/IP received: %s\n", telegramString);
                }
                break;
        }
    }

    return 0;
}

///////////////////////////////////////////////////////////
// unpackAcknSimpleIOSelection
///////////////////////////////////////////////////////////

void TCPClientLWM::unpackAcknSimpleIOSelection(TCPDataBlockType& recvBuffer)
{
    int32_t* pint32;

    pint32 = reinterpret_cast<int32_t*>(recvBuffer.data());

    int32_t telegram = *pint32;
    pint32++;
    int32_t status = *pint32;
    pint32++;
    int32_t length = *pint32;
    pint32++;

    evaluateStatus(status);

    wmLog(eDebug, "unpackAcknSimpleIOSelection: tele: %X, stat: %x, len: %d\n", telegram, status, length);
}

///////////////////////////////////////////////////////////
// unpackAcknSimpleIOStopp
///////////////////////////////////////////////////////////

void TCPClientLWM::unpackAcknSimpleIOStopp(TCPDataBlockType& recvBuffer)
{
    int32_t* pint32;

    pint32 = reinterpret_cast<int32_t*>(recvBuffer.data());

    int32_t telegram = *pint32;
    pint32++;
    int32_t status = *pint32;
    pint32++;
    int32_t length = *pint32;
    pint32++;

    evaluateStatus(status);

    wmLog(eDebug, "unpackAcknSimpleIOStopp: tele: %X, stat: %x, len: %d\n", telegram, status, length);
}

///////////////////////////////////////////////////////////
// unpackSimpleIOTrigger
///////////////////////////////////////////////////////////

void TCPClientLWM::unpackSimpleIOTrigger(TCPDataBlockType& recvBuffer)
{
    int32_t* pint32;
    char* pchar;

    pint32 = reinterpret_cast<int32_t*>(recvBuffer.data());

    int32_t telegram = *pint32;
    pint32++;
    int32_t status = *pint32;
    pint32++;
    int32_t length = *pint32;
    pint32++;

    int32_t stepNumber = *pint32;
    pint32++;
    int32_t commentLength = *pint32;
    pint32++;

    std::string comment{""};
    pchar = reinterpret_cast<char*>(pint32);
    for (int i = 0; i < commentLength; i++)
    {
        comment.push_back(*pchar);
        pchar++;
    }
    pint32 = reinterpret_cast<int32_t*>(pchar);

    evaluateStatus(status);

    wmLog(eDebug, "unpackSimpleIOTrigger: tele: %X, stat: %x, len: %d\n", telegram, status, length);
    wmLog(eDebug, "unpackSimpleIOTrigger: step: %d, comment: <%s>\n", stepNumber, comment.c_str());
}

///////////////////////////////////////////////////////////
// unpackSimpleIOError
///////////////////////////////////////////////////////////

void TCPClientLWM::unpackSimpleIOError(TCPDataBlockType& recvBuffer)
{
    int32_t* pint32;

    pint32 = reinterpret_cast<int32_t*>(recvBuffer.data());

    int32_t telegram = *pint32;
    pint32++;
    m_errorStatus = *pint32;
    pint32++;
    int32_t length = *pint32;
    pint32++;

    evaluateStatus(m_errorStatus);

    wmLog(eDebug, "unpackSimpleIOError: tele: %X, stat: %x, len: %d\n", telegram, m_errorStatus, length);
}

///////////////////////////////////////////////////////////
// unpackWatchdog
///////////////////////////////////////////////////////////

void TCPClientLWM::unpackWatchdog(TCPDataBlockType& recvBuffer)
{
    int32_t* pint32;

    pint32 = reinterpret_cast<int32_t*>(recvBuffer.data());

    int32_t telegram = *pint32;
    pint32++;
    int32_t status = *pint32;
    pint32++;
    int32_t length = *pint32;
    pint32++;

    evaluateStatus(status);

    m_watchdogTelegramSent.store(false);
    m_watchdogTelegramReceived.store(true);

    wmLog(eDebug, "unpackWatchdog: tele: %X, stat: %x, len: %d\n", telegram, status, length);
}

///////////////////////////////////////////////////////////
// unpackResultsRanges
///////////////////////////////////////////////////////////

void TCPClientLWM::unpackResultsRanges(TCPDataBlockType& recvBuffer)
{
    int32_t* pint32;
    float* pfloat;
    char* pchar;
    int16_t* pint16;

    pint32 = reinterpret_cast<int32_t*>(recvBuffer.data());

    int32_t telegram = *pint32;
    pint32++;
    int32_t status = *pint32;
    pint32++;
    int32_t length = *pint32;
    pint32++;

    m_resultsRanges.m_programNumber = *pint32;
    pint32++;
    m_resultsRanges.m_configId = *pint32;
    pint32++;
    m_resultsRanges.m_configVersion = *pint32;
    pint32++;
    m_resultsRanges.m_overallResult = *pint32;
    pint32++;
    m_resultsRanges.m_extendedResult = *pint32;
    pint32++;
    pfloat = reinterpret_cast<float*>(pint32);
    m_resultsRanges.m_errorProbability = *pfloat;
    pfloat++;
    pint32 = reinterpret_cast<int32_t*>(pfloat);
    m_resultsRanges.m_errorSignals = *pint32;
    pint32++;
    m_resultsRanges.m_measurementId = *pint32;
    pint32++;

    int32_t commentLength = *pint32;
    pint32++;

    m_resultsRanges.m_comment.clear();
    pchar = reinterpret_cast<char*>(pint32);
    for (int i = 0; i < 80; i++)
    {
        if (i < commentLength)
        {
            m_resultsRanges.m_comment.push_back(*pchar);
        }
        pchar++;
    }

    pint16 = reinterpret_cast<int16_t*>(pchar);
    m_resultsRanges.m_year = *pint16;
    pint16++;
    m_resultsRanges.m_month = *pint16;
    pint16++;
    m_resultsRanges.m_day = *pint16;
    pint16++;
    m_resultsRanges.m_hour = *pint16;
    pint16++;
    m_resultsRanges.m_minute = *pint16;
    pint16++;
    m_resultsRanges.m_second = *pint16;
    pint16++;

    pint32 = reinterpret_cast<int32_t*>(pint16);
    m_resultsRanges.m_numberOfRanges = *pint32;
    pint32++;

    // In the case of ScanMaster, m_numberOfRanges should normally be 0
    // if m_numberOfRanges is 0, then LWM don't add results for ranges

    evaluateStatus(status);
    evaluateExtendedResult(m_resultsRanges.m_extendedResult);

    wmLog(eDebug, "unpackResultsRanges: tele: %X, stat: %x, len: %d\n", telegram, status, length);
}

///////////////////////////////////////////////////////////
// unpackResultsValuesRanges
///////////////////////////////////////////////////////////

void TCPClientLWM::unpackResultsValuesRanges(TCPDataBlockType& recvBuffer)
{
    int32_t* pint32;
    float* pfloat;
    char* pchar;
    int16_t* pint16;

    pint32 = reinterpret_cast<int32_t*>(recvBuffer.data());

    int32_t telegram = *pint32;
    pint32++;
    int32_t status = *pint32;
    pint32++;
    int32_t length = *pint32;
    pint32++;

    m_resultsRanges.m_programNumber = *pint32;
    pint32++;
    m_resultsRanges.m_configId = *pint32;
    pint32++;
    m_resultsRanges.m_configVersion = *pint32;
    pint32++;
    m_resultsRanges.m_overallResult = *pint32;
    pint32++;
    m_resultsRanges.m_extendedResult = *pint32;
    pint32++;
    pfloat = reinterpret_cast<float*>(pint32);
    m_resultsRanges.m_errorProbability = *pfloat;
    pfloat++;
    pint32 = reinterpret_cast<int32_t*>(pfloat);
    m_resultsRanges.m_errorSignals = *pint32;
    pint32++;
    m_resultsRanges.m_measurementId = *pint32;
    pint32++;

    int32_t commentLength = *pint32;
    pint32++;

    m_resultsRanges.m_comment.clear();
    pchar = reinterpret_cast<char*>(pint32);
    for (int i = 0; i < 80; i++)
    {
        if (i < commentLength)
        {
            m_resultsRanges.m_comment.push_back(*pchar);
        }
        pchar++;
    }

    pint16 = reinterpret_cast<int16_t*>(pchar);
    m_resultsRanges.m_year = *pint16;
    pint16++;
    m_resultsRanges.m_month = *pint16;
    pint16++;
    m_resultsRanges.m_day = *pint16;
    pint16++;
    m_resultsRanges.m_hour = *pint16;
    pint16++;
    m_resultsRanges.m_minute = *pint16;
    pint16++;
    m_resultsRanges.m_second = *pint16;
    pint16++;

    pint32 = reinterpret_cast<int32_t*>(pint16);
    int32_t numberOfSensors = *pint32;
    pint32++;

    for (int i = 0; i < numberOfSensors; i++)
    {
        int32_t detectionNameLength = *pint32;
        pint32++;

        std::string detectionName{};
        pchar = reinterpret_cast<char*>(pint32);
        for (int i = 0; i < 20; i++)
        {
            if (i < detectionNameLength)
            {
                detectionName.push_back(*pchar);
            }
            pchar++;
        }

        pint32 = reinterpret_cast<int32_t*>(pchar);
        [[maybe_unused]] int32_t detectionID = *pint32;
        pint32++;
        [[maybe_unused]] int32_t detectionResult = *pint32;
        pint32++;
        [[maybe_unused]] int32_t errorIntegral = *pint32;
        pint32++;
        [[maybe_unused]] int32_t errorFrequencyTop = *pint32;
        pint32++;
        [[maybe_unused]] int32_t errorFrequencyBottom = *pint32;
        pint32++;
        [[maybe_unused]] int32_t errorDeviation = *pint32;
        pint32++;

        int32_t numberOfRanges = *pint32;
        pint32++;

        for (int j = 0; j < numberOfRanges; j++)
        {
            for (int k = 0; k < 16; k++)
            {
                pint32++;
            }
        }
    }

    evaluateStatus(status);
    evaluateExtendedResult(m_resultsRanges.m_extendedResult);

    wmLog(eDebug, "unpackResultsValuesRanges: tele: %X, stat: %x, len: %d\n", telegram, status, length);
}

void TCPClientLWM::evaluateStatus(const int32_t status)
{
    switch (status)
    {
        case 0x01: // unknown telegram
            break;
        case 0x02: // unsupported telegram
            break;
        case 0x09: // no data present
            break;
        case 0x0C: // invalid length
            break;
        case 0x0D: // unknown data
            break;
        case 0x0F: // incomplete data
            break;
        case 0x10: // maximum clients reached
            break;
        case 0x11: // no clear result
            break;
        case 0x2A: // problems with rework
        case 0x2C:
        case 0x2E:
        case 0x30:
        case 0x33:
            break;
        case 0x35: // wrong timing
            break;
        case 0x40: // wrong program
            wmLogTr(eError, "QnxMsg.VI.PrgNoNotExistLWM", "The program number does not exist on LWM device\n");
            break;
        case 0x41: // wrong sequence
            break;
        case 0x42: // wrong length of comment
            break;
        case 0x50: // system locked
            // LWM isn't able to process input signals
            wmLogTr(eError, "QnxMsg.VI.DeviceLockedLWM", "LWM device is locked\n");
            break;
        default:
            break;
    }
}

void TCPClientLWM::evaluateExtendedResult(const int32_t extendedResult)
{
    if (extendedResult & 0x01)
    {
        // Measurement was aborted by 'SYSTEM ACTIVATED' (=LOW)
    }
    if (extendedResult & 0x02)
    {
        // Measurement was aborted by 'AUTOMATIC MODE STOP' (=HIGH).
    }
    if (extendedResult & 0x04)
    {
        // Set number of evaluation areas was not reached
    }
    if (extendedResult & 0x08)
    {
        // Set number of steps (seams) was not reached
    }
    if (extendedResult & 0x10)
    {
        // IDM serial connection lost (IDM only)
    }
    if (extendedResult & 0x20)
    {
        // UPS has detected a power failure
    }
    if (extendedResult & 0x40)
    {
        // Measurement incomplete (result)
    }
}

/***************************************************************************/
/* TCPClientSendThread                                                     */
/***************************************************************************/

// Thread function must be outside of the class
void *TCPClientSendThread(void *p_pArg)
{
    auto pDataToTCPClientSendThread = static_cast<struct DataToTCPClientSendThread *>(p_pArg);
    auto pTCPClientLWM = pDataToTCPClientSendThread->m_TCPClientLWM;

    wmLog(eDebug, "TCPClientSendThread is started\n");

    usleep(1000 * 1000); // 1 second delay before starting

    wmLog(eDebug, "TCPClientSendThread is active\n");

    usleep(100*1000);

    if (pTCPClientLWM != nullptr)
    {
        pTCPClientLWM->run();
    }

    return NULL;
}

/***************************************************************************/
/* TCPClientReceiveThread                                                  */
/***************************************************************************/

// Thread function must be outside of the class
void *TCPClientReceiveThread(void *p_pArg)
{
    auto pDataToTCPClientReceiveThread = static_cast<struct DataToTCPClientReceiveThread *>(p_pArg);
    auto pTCPClientLWM = pDataToTCPClientReceiveThread->m_TCPClientLWM;

    wmLog(eDebug, "TCPClientReceiveThread is started\n");

    wmLog(eDebug, "TCPClientReceiveThread is active\n");

    usleep(100*1000);

    if (pTCPClientLWM != nullptr)
    {
        pTCPClientLWM->receiveFunction();
    }

    pTCPClientLWM->m_TCPClientReceiveThreadId = 0;

    return NULL;
}

}
}

