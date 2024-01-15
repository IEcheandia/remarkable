/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Handles the TCP/IP communication with SOURING machine
 */

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#include "module/moduleLogger.h"

#include "common/connectionConfiguration.h"

#include "TCPCommunication/TCPClientSRING.h"

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

///////////////////////////////////////////////////////////
// Prototyp fuer Thread Funktions
///////////////////////////////////////////////////////////

// Thread Funktion muss ausserhalb der Klasse sein
void* TCPClientThread (void *p_pArg);

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

TCPClientSRING::TCPClientSRING(TDb<MsgProxy>& p_rDbProxy):
        m_oTCPClientThread_ID(0),
        m_oIsSOUVIS6000_Application(false),
        m_oIPAddressOfMachine(0x0100007F), // 127.0.0.1 in network byte order
        m_oSendRequestQualityData(false),
        m_oSendRequestInspectData(false),
        m_oSendRequestStatusData(true),
        m_oSystemReadyStatus(true),
        m_oCycleDataBatchID(0),
        m_oCycleDataProductNo(0),
        m_oSouvisParamReq(true),
        m_rDbProxy(p_rDbProxy),
        m_oLastPresentSeam(0)
{
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
    m_oSOUVIS6000_TCPIP_Communication_On = SystemConfiguration::instance().getBool("SOUVIS6000_TCPIP_Communication_On", true);
    wmLog(eDebug, "m_oSOUVIS6000_TCPIP_Communication_On (bool): %d\n", m_oSOUVIS6000_TCPIP_Communication_On);
    m_oSOUVIS6000_IP_Address_MachineControl = SystemConfiguration::instance().getString("SOUVIS6000_IP_Address_MachineControl", "127.0.0.1");
    wmLog(eDebug, "m_oSOUVIS6000_IP_Address_MachineControl: <%s>\n", m_oSOUVIS6000_IP_Address_MachineControl.c_str());

    memset(&m_oSeamData, 0, sizeof(m_oSeamData));
    memset(&m_oQualResult, 0, sizeof(m_oQualResult));
    for(int i = 0;i < MAX_S6K_SEAM;i++)
    {
        m_oQualResult[i].m_oFirstErrorPos = -1;
    }

    clearSegmentTable();
    clearQualityGrades();

    int oRetValue = inet_pton(AF_INET, m_oSOUVIS6000_IP_Address_MachineControl.c_str(), &m_oIPAddressOfMachine);
    if (oRetValue != 1)
    {
        wmLogTr(eError, "QnxMsg.VI.TCPWrongIPFormat", "wrong format of IP address string\n");
        m_oIPAddressOfMachine = 0x0100007F; // 127.0.0.1 in network byte order
    }
    char oHelpStrg[21];
    sprintf(oHelpStrg, "%08X", m_oIPAddressOfMachine);
    wmLog(eDebug, "m_oIPAddressOfMachine: %s, retVal: %d\n", oHelpStrg, oRetValue); // in network byte order

    StartTCPClientThread();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

TCPClientSRING::~TCPClientSRING(void)
{
    if (m_oTCPClientThread_ID != 0)
    {
        if (pthread_cancel(m_oTCPClientThread_ID) != 0)
        {
            wmLog(eDebug, "TCPClientSRING: was not able to abort thread: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(101)");
        }
    }
}

void TCPClientSRING::SetSeamData(int p_oSeamNo, struct SeamDataStruct& p_oSeamData)
{
    Poco::Mutex::ScopedLock lock( m_oSeamDataMutex );

    m_oSeamData[p_oSeamNo] = p_oSeamData;
}

void TCPClientSRING::setS6K_QualityResults(int32_t p_oSeamNo, struct S6K_QualityData_S1S2 p_oQualityData)
{
    Poco::Mutex::ScopedLock lock( m_oQualResultMutex );

    m_oQualResult[p_oSeamNo] = p_oQualityData;
}

///////////////////////////////////////////////////////////
// private members
///////////////////////////////////////////////////////////

/***************************************************************************/
/* StartTCPClientThread                                                    */
/***************************************************************************/

void TCPClientSRING::StartTCPClientThread(void)
{
    if (isSOUVIS6000_Application())
    {
        ///////////////////////////////////////////////////////
        // Thread für TCPClient starten
        ///////////////////////////////////////////////////////
        pthread_attr_t oPthreadAttr;
        pthread_attr_init(&oPthreadAttr);

        m_oDataToTCPClientThread.m_pTCPClientSRING = this;

        if (pthread_create(&m_oTCPClientThread_ID, &oPthreadAttr, &TCPClientThread, &m_oDataToTCPClientThread) != 0)
        {
            wmLog(eDebug, "TCPClientSRING: was not able to create thread: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(102)");
        }
    }
}

int TCPClientSRING::run(void)
{
    m_oSouvisParamReq = true; // request parameter from machine
    m_oSendRequestStatusData = true; // send status data to machine

    /****** main loop ********************************************************/

    sleep(8);

    while (true)
    {
        // check if there has to be sent a block
        checkSendRequest();

        // check if there are new product data
        dbCheck();

        usleep(20*1000); // 20 ms
    }          /* end while !quit */
    return 0;
}

/***************************************************************************/
/* checkSendRequest                                                        */
/***************************************************************************/

void TCPClientSRING::checkSendRequest (void)
{
    int sockDesc;
    struct sockaddr_in serverAddr;

    static bool sendRequestAll = false;
    static bool sendRequestQualResult = false;
    static bool sendRequestInspectData = false;
    static bool sendRequestStatusData = false;

    /*************************************************************************/

    // determine if there is at least one send requested
    if (m_oSendRequestQualityData == true)
    {
        m_oSendRequestQualityData = false;
        sendRequestAll = true;
        sendRequestQualResult = true;
    }

    if (m_oSendRequestInspectData == true)
    {
        m_oSendRequestInspectData = false;
        sendRequestAll = true;
        sendRequestInspectData = true;
    }

    if (m_oSendRequestStatusData == true)
    {
        m_oSendRequestStatusData = false;
        sendRequestAll = true;
        sendRequestStatusData = true;
    }

    // return from function if there is no send request
    if (sendRequestAll == false) return;

    // sending of at least one block is requested
    if (isSOUVIS6000_TCPIP_Communication_On())
    {
        // create socket
        sockDesc = socket(AF_INET, SOCK_STREAM, 0);
        if (sockDesc == -1)
        {
            wmLog(eDebug, "TCPClientSRING: Error while creating socket: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(101)");
            // cannot open socket, all pending send requests are stopped
            sendRequestAll = false;
            sendRequestQualResult = false;
            sendRequestInspectData = false;
            sendRequestStatusData = false;
            return;
        }
        memset((char *)&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;

        memcpy(&serverAddr.sin_addr, &m_oIPAddressOfMachine, 4); // in network byte order

        if (isSOUVIS6000_Is_PreInspection())
        {
            serverAddr.sin_port = htons(SOURING_MC_SERVER_PORT_S1);
        }
        else if (isSOUVIS6000_Is_PostInspection_Top())
        {
            serverAddr.sin_port = htons(SOURING_MC_SERVER_PORT_S2U);
        }
        else if (isSOUVIS6000_Is_PostInspection_Bottom())
        {
            serverAddr.sin_port = htons(SOURING_MC_SERVER_PORT_S2L);
        }
        else
        {
            wmLogTr(eError, "QnxMsg.VI.TCPCommNoSubsys", "No subsystem of SOUVIS6000 system is configured !\n");
            serverAddr.sin_port = htons(SOURING_MC_SERVER_PORT_S1);
        }

        // connect to Server
        if (connect(sockDesc, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
        {
            if (errno == ECONNREFUSED)
            {
                wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(001)");
                close (sockDesc);
                sleep(2);
                return;
            }
            else if (errno == ETIMEDOUT)
            {
                wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(002)");
                close (sockDesc);
                return;
            }
            else
            {
                wmLog(eDebug, "TCPClientSRING: Error while connecting server: %s\n", strerror(errno));
                close (sockDesc);
                wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(102)");
                // cannot connect to server, all pending send requests are stopped
                sendRequestAll = false;
                sendRequestQualResult = false;
                sendRequestInspectData = false;
                sendRequestStatusData = false;
                return;
            }
        }
        wmLogTr(eInfo, "QnxMsg.VI.TCPCommStarted", "TCP/IP Connection started %s\n", "(Client)");

        // check if qualResult has to be sent
        if (sendRequestQualResult == true)
        {
            sendRequestQualResult = false;
            int oSeamsToSend{MAX_S6K_SEAM_SRING};
            if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
            {
                oSeamsToSend = MAX_S6K_SEAM_SSPEED;
            }
            for (int i = 0; i < oSeamsToSend; i++)
            {
                int oRetValue = sendQualResult(sockDesc, i);
                if (oRetValue == 1) // successful
                {
                }
                else if (oRetValue == -1) // fatal error
                {
                }
                else // repeat
                {
                    sendRequestQualResult = true;
                }
                usleep(3*1000);
            }
        }

        // check if inspectData has to be sent
        if (sendRequestInspectData == true)
        {
            sendRequestInspectData = false;
            int oSeamsToSend{MAX_S6K_SEAM_SRING};
            if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
            {
                oSeamsToSend = MAX_S6K_SEAM_SSPEED;
            }
            for (int i = 0; i < oSeamsToSend; i++)
            {
                int oRetValue = sendInspectData(sockDesc, i);
                if (oRetValue == 1) // successful
                {
                }
                else if (oRetValue == -1) // fatal error
                {
                }
                else // repeat
                {
                    sendRequestInspectData = true;
                }
                usleep(3*1000);
            }
        }

        // check if statusData has to be sent
        if (sendRequestStatusData == true)
        {
            int oRetValue = sendStatusData(sockDesc);
            if (oRetValue == 1) // successful
            {
                sendRequestStatusData = false;
            }
            else if (oRetValue == -1) // fatal error
            {
                sendRequestStatusData = false;
            }
            else // repeat
            {
                sendRequestStatusData = true;
            }
        }

        if ((sendRequestQualResult == false) &&
            (sendRequestInspectData == false) &&
            (sendRequestStatusData == false))
        {
            sendRequestAll = false;
        }

        // close connection and socket
        close (sockDesc);

        wmLogTr(eInfo, "QnxMsg.VI.TCPCommEnded", "TCP/IP Connection ended %s\n", "(Client)");
    }
    else // if(isSOUVIS6000_TCPIP_Communication_On)
    {
        sendRequestAll = false;
    }
}

/***************************************************************************/
/* sendQualResult                                                          */
/***************************************************************************/

int TCPClientSRING::sendQualResult(int sockDesc, int p_oSeamNo)
{
    char sendBuffer[1500];
    char recvBuffer[1500];
    char statusStrg[121];
    int lengthToSend;

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 1;
    oTcpTimeout.tv_usec = 0;
    setsockopt(sockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(sockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

    packQualResult(sendBuffer, &lengthToSend, p_oSeamNo);
    if (send(sockDesc, sendBuffer, lengthToSend, 0) == -1)
    {
        wmLog(eDebug, "TCPClientSRING: was not able to send via TCP/IP: %s\n", strerror(errno));
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(105)");
        return -1;
    }

    int16_t *type = (int16_t *) &sendBuffer[0];
    int16_t *version = (int16_t *) &sendBuffer[2];
    int16_t *length = (int16_t *) &sendBuffer[4];
    sprintf(statusStrg,"TCPClientSRING: blockType = %d , blockVersion = %04X , blockLength = %d\n",
            *type,*version,*length);
    wmLog(eDebug, statusStrg);

    if (recv(sockDesc, recvBuffer, sizeof(recvBuffer), 0) <= 0)
    {
        wmLog(eDebug, "TCPClientSRING: was not able to recv via TCP/IP: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(106)");
        return 0;
    }
    int32_t oAcknCode;
    int32_t oBlockType;
    int32_t oBlocksReceived;
    unpackAcknBlock(recvBuffer, oAcknCode, oBlockType, oBlocksReceived);

    sprintf(statusStrg,"TCPClientSRING: acknCode = %4d ; blockType = %d ; reserve1 = %d\n",
                    oAcknCode, oBlockType, oBlocksReceived);
    wmLog(eDebug, statusStrg);

    return 1;
}

/***************************************************************************/
/* sendInspectData                                                         */
/***************************************************************************/

int TCPClientSRING::sendInspectData(int sockDesc, int p_oSeamNo)
{
    char sendBuffer[1500];
    char recvBuffer[1500];
    char statusStrg[121];
    int lengthToSend;

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 1;
    oTcpTimeout.tv_usec = 0;
    setsockopt(sockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(sockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

    packInspectData(sendBuffer, &lengthToSend, p_oSeamNo);
    if (send(sockDesc, sendBuffer, lengthToSend, 0) == -1)
    {
        wmLog(eDebug, "TCPClientSRING: was not able to send via TCP/IP: %s\n", strerror(errno));
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(111)");
        return -1;
    }

    int16_t *type = (int16_t *) &sendBuffer[0];
    int16_t *version = (int16_t *) &sendBuffer[2];
    int16_t *length = (int16_t *) &sendBuffer[4];
    sprintf(statusStrg,"TCPClientSRING: blockType = %d , blockVersion = %04X , blockLength = %d\n",
            *type,*version,*length);
    wmLog(eDebug, statusStrg);

    if (recv(sockDesc, recvBuffer, sizeof(recvBuffer), 0) <= 0)
    {
        wmLog(eDebug, "TCPClientSRING: was not able to recv via TCP/IP: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(112)");
        return 0;
    }
    int32_t oAcknCode;
    int32_t oBlockType;
    int32_t oBlocksReceived;
    unpackAcknBlock(recvBuffer, oAcknCode, oBlockType, oBlocksReceived);

    sprintf(statusStrg,"TCPClientSRING: acknCode = %4d ; blockType = %d ; reserve1 = %d\n",
                    oAcknCode, oBlockType, oBlocksReceived);
    wmLog(eDebug, statusStrg);

    return 1;
}

/***************************************************************************/
/* sendStatusData                                                          */
/***************************************************************************/

int TCPClientSRING::sendStatusData(int sockDesc)
{
    char sendBuffer[1500];
    char recvBuffer[1500];
    char statusStrg[121];
    int lengthToSend;

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 1;
    oTcpTimeout.tv_usec = 0;
    setsockopt(sockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(sockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

    packStatusData(sendBuffer, &lengthToSend);
    if (send(sockDesc, sendBuffer, lengthToSend, 0) == -1)
    {
        wmLog(eDebug, "TCPClientSRING: was not able to send via TCP/IP: %s\n", strerror(errno));
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(117)");
        return -1;
    }

    int16_t *type = (int16_t *) &sendBuffer[0];
    int16_t *version = (int16_t *) &sendBuffer[2];
    int16_t *length = (int16_t *) &sendBuffer[4];
    sprintf(statusStrg,"TCPClientSRING: blockType = %d , blockVersion = %04X , blockLength = %d\n",
            *type,*version,*length);
    wmLog(eDebug, statusStrg);

    if (recv(sockDesc, recvBuffer, sizeof(recvBuffer), 0) <= 0)
    {
        wmLog(eDebug, "TCPClientSRING: was not able to recv via TCP/IP: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(118)");
        return 0;
    }
    int32_t oAcknCode;
    int32_t oBlockType;
    int32_t oBlocksReceived;
    unpackAcknBlock(recvBuffer, oAcknCode, oBlockType, oBlocksReceived);

    sprintf(statusStrg,"TCPClientSRING: acknCode = %4d ; blockType = %d ; reserve1 = %d\n",
                    oAcknCode, oBlockType, oBlocksReceived);
    wmLog(eDebug, statusStrg);

    return 1;
}

/***************************************************************************/
/* packQualResult                                                          */
/***************************************************************************/

void TCPClientSRING::packQualResult(char *sendBuffer, int *bufLength, int p_oSeamNo)
{
    int16_t *pword;
    uint32_t *pudword;
    int blockLength;
    uint32_t oErrS1 = 0x00;
    uint32_t oExcS1 = 0x00;
    int16_t oErrPosS1 = -1;
    uint32_t oErrS2U = 0x00;
    uint32_t oExcS2U = 0x00;
    int16_t oErrPosS2U = -1;
    uint32_t oErrS2L = 0x00;
    uint32_t oExcS2L = 0x00;
    int16_t oErrPosS2L = -1;

    // Write DATABLOCK-Header
    pword = (int16_t *) sendBuffer;

    *pword = (int16_t) (DBLOCK_SRING_QUALRESULT_1_TYPE + p_oSeamNo);
    pword++;
    *pword = (int16_t) DBLOCK_SRING_QUALRESULT_VERSION;
    pword++;
    *pword = 0;           // place for blockLength
    pword++;
    *pword = 0;           // reserve
    pword++;

    // Write DATABLOCK-Information

    Poco::Mutex::ScopedLock lock( m_oQualResultMutex );

    *pword = m_oCycleDataBatchID.load(); // Batch-ID wie ueber Profinet empfangen
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;

    // change datatype
    pudword = (uint32_t *) pword;

    *pudword = 0;
    pudword++;

    // change datatype
    pword = (int16_t *) pudword;

    *pword = 0;
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;

    // change datatype
    pudword = (uint32_t *) pword;

    if (isSOUVIS6000_Is_PreInspection())
    {
        *pudword = m_oQualResult[p_oSeamNo].m_oQualityError;    // Qualitaetsfehler Souvis1
    }
    else
    {
        *pudword = 0;
    }
    oErrS1 = *pudword;
    pudword++;
    if (isSOUVIS6000_Is_PreInspection())
    {
        *pudword = m_oQualResult[p_oSeamNo].m_oExceptions;      // Ausnahmebedingungen Souvis1
    }
    else
    {
        *pudword = 0;
    }
    oExcS1 = *pudword;
    pudword++;

    // change datatype
    pword = (int16_t *) pudword;

    if (isSOUVIS6000_Is_PreInspection())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oFirstErrorPos;     // erste Fehlerposition SOUVIS1
    }
    else
    {
        *pword = -1;
    }
    oErrPosS1 = *pword;
    pword++;
    if (isSOUVIS6000_Is_PreInspection())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgElementWidth;   // Mittelwert Spaltbreite
    }
    else
    {
        *pword = 0;
    }
    pword++;
    if (isSOUVIS6000_Is_PreInspection())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgHeightDiff;     // Mittelwert Hoehendifferenz
    }
    else
    {
        *pword = 0;
    }
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;

    // change datatype
    pudword = (uint32_t *) pword;

    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        *pudword = m_oQualResult[p_oSeamNo].m_oQualityError;    // Qualitaetsfehler Souvis2 oben
    }
    else
    {
        *pudword = 0;
    }
    oErrS2U = *pudword;
    pudword++;
    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        *pudword = m_oQualResult[p_oSeamNo].m_oExceptions;      // Ausnahmebedingungen Souvis2 oben
    }
    else
    {
        *pudword = 0;
    }
    oExcS2U = *pudword;
    pudword++;

    // change datatype
    pword = (int16_t *) pudword;

    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oFirstErrorPos;     // erste Fehlerposition SOUVIS2 oben
    }
    else
    {
        *pword = -1;
    }
    oErrPosS2U = *pword;
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgElementWidth;   // Mittelwert Nahtbreite S2 oben
    }
    else
    {
        *pword = 0;
    }
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgHeightDiff;     // Mittelwert Hoehendifferenz SOUVIS2 oben
    }
    else
    {
        *pword = 0;
    }
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgConcavity;      // Mittelwert Konkavitaet SOUVIS2 oben
    }
    else
    {
        *pword = 0;
    }
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgConvexity;      // Mittelwert Konvexitaet SOUVIS2 oben
    }
    else
    {
        *pword = 0;
    }
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Top())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oFirstPoreWidth;    // Porengrosse erster Pore SOUVIS2 oben
    }
    else
    {
        *pword = 0;
    }
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;

    // change datatype
    pudword = (uint32_t *) pword;

    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        *pudword = m_oQualResult[p_oSeamNo].m_oQualityError;    // Qualitaetsfehler Souvis2 unten
    }
    else
    {
        *pudword = 0;
    }
    oErrS2L = *pudword;
    pudword++;
    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        *pudword = m_oQualResult[p_oSeamNo].m_oExceptions;      // Ausnahmebedingungen Souvis2 unten
    }
    else
    {
        *pudword = 0;
    }
    oExcS2L = *pudword;
    pudword++;

    // change datatype
    pword = (int16_t *) pudword;

    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oFirstErrorPos;     // erste Fehlerposition SOUVIS2 unten
    }
    else
    {
        *pword = -1;
    }
    oErrPosS2L = *pword;
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgElementWidth;   // Mittelwert Nahtbreite S2 unten
    }
    else
    {
        *pword = 0;
    }
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgHeightDiff;     // Mittelwert Hoehendifferenz SOUVIS2 unten
    }
    else
    {
        *pword = 0;
    }
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgConcavity;      // Mittelwert Konkavitaet SOUVIS2 unten
    }
    else
    {
        *pword = 0;
    }
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oAvgConvexity;      // Mittelwert Konvexitaet SOUVIS2 unten
    }
    else
    {
        *pword = 0;
    }
    pword++;
    if (isSOUVIS6000_Is_PostInspection_Bottom())
    {
        *pword = m_oQualResult[p_oSeamNo].m_oFirstPoreWidth;    // Porengrosse erster Pore SOUVIS2 unten
    }
    else
    {
        *pword = 0;
    }
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;
    *pword = 0;
    pword++;

    blockLength = (char *)pword - (char *)sendBuffer;
    pword = (int16_t *) &sendBuffer[4];
    *pword = (int16_t) blockLength;
    *bufLength = blockLength;

    wmLog(eDebug, "packQualResult: blockType: %d, p_oSeamNo: %d, batchID:%d\n", (DBLOCK_SRING_QUALRESULT_1_TYPE + p_oSeamNo), p_oSeamNo, m_oCycleDataBatchID.load());
    char oErrStrg[21];
    char oExcStrg[21];
    sprintf(oErrStrg, "%08X", oErrS1);
    sprintf(oExcStrg, "%08X", oExcS1);
    wmLog(eDebug, "errS1: %s, excS1: %s, errPosS1: %d\n", 
          oErrStrg, oExcStrg, oErrPosS1);
    sprintf(oErrStrg, "%08X", oErrS2U);
    sprintf(oExcStrg, "%08X", oExcS2U);
    wmLog(eDebug, "errS2U:%s, excS2U:%s, errPosS2U:%d\n", 
          oErrStrg, oExcStrg, oErrPosS2U);
    sprintf(oErrStrg, "%08X", oErrS2L);
    sprintf(oExcStrg, "%08X", oExcS2L);
    wmLog(eDebug, "errS2L:%s, excS2L:%s, errPosS2L:%d\n", 
          oErrStrg, oExcStrg, oErrPosS2L);
}

/***************************************************************************/
/* packInspectData                                                         */
/***************************************************************************/

void TCPClientSRING::packInspectData(char *sendBuffer, int *bufLength, int p_oSeamNo)
{
    int16_t *pword;
    int32_t *pdword;
    int blockLength;

    // Write DATABLOCK-Header
    pword = (int16_t *) sendBuffer;

    *pword = (int16_t) (DBLOCK_SRING_INSPECTDATA_1_TYPE + p_oSeamNo);
    pword++;
    *pword = (int16_t) DBLOCK_SRING_INSPECTDATA_VERSION;
    pword++;
    *pword = 0;           // place for blockLength
    pword++;
    *pword = 0;           // reserve
    pword++;

    // Write DATABLOCK-Information

    // change datatype
    pdword = (int32_t *) pword;

    Poco::Mutex::ScopedLock lock( m_oSeamDataMutex );

//    *pdword = m_oCycleDataProductNo.load();      // Produktnummer
    *pdword = m_oSeamData[p_oSeamNo].m_oProductNumber;      // Produktnummer
    pdword++;

    // change datatype
    pword = (int16_t *) pdword;

    *pword = m_oSeamData[p_oSeamNo].m_oSeamPresent;             // Naht aktiv
    pword++;

    // change datatype
    pdword = (int32_t *) pword;

    *pdword = 0;                             // Reserve
    pdword++;
    *pdword = m_oSeamData[p_oSeamNo].m_oSeamLength;        // Nahtlaenge
    pdword++;

    // change datatype
    pword = (int16_t *) pdword;

    *pword = 0;                              // Reserve
    pword++;
    *pword = m_oSeamData[p_oSeamNo].m_oTargetDifference;             // Soll Differenz Dickensprung
    pword++;
    *pword = m_oSeamData[p_oSeamNo].m_oBlankThicknessLeft;           // Blechdicke links
    pword++;
    *pword = m_oSeamData[p_oSeamNo].m_oBlankThicknessRight;          // Blechdicke rechts
    pword++;
    *pword = m_oSeamData[p_oSeamNo].m_oSetupNumber;             // Setupnummer
    pword++;
    *pword = m_oSeamData[p_oSeamNo].m_oMaxWeldSpeed;               // Schweissgeschwindigkeit
    pword++;
    *pword = m_oSeamData[p_oSeamNo].m_oMaxGapWidth;         // max. Spaltbreite
    pword++;
    *pword = 0; //pprodNaht->minEdgeQuality;      // min. Kantenqualitaet
    pword++;

    // change datatype
    pdword = (int32_t *) pword;

    *pdword = 0;                             // Reserve
    pdword++;
    *pdword = 0;                             // Reserve
    pdword++;
    *pdword = 0;                             // Reserve
    pdword++;
    *pdword = 0;                             // Reserve
    pdword++;
    *pdword = 0;                             // Reserve
    pdword++;
    *pdword = 0;                             // Reserve
    pdword++;

    // change datatype
    pword = (int16_t *) pdword;

    for(int i = 0;i < MAX_S6K_SEGMENTS;i++) // fuer alle 12 Segmente
    {
        *pword = m_oInspData[p_oSeamNo].m_oSegment[i].m_oStart; // Startposition Segment
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oSegment[i].m_oGrade; // Inspektionsstufe Segment
        pword++;
    }
    for(int i = 0;i < MAX_S6K_QUALITY_GRADES;i++) // fuer alle 4 Inspektionsstufen
    {
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMaxConcavity;          // max. Konkavitaet
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMaxConvexity;          // max. Konvexitaet
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMaxPosMisalign;        // max. pos. Kantenversatz
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMaxNegMisalign;        // max. neg. Kantenversatz
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMinSeamWidth;          // min. Nahtbreite
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMaxPoreWidth;          // max. Porengroesse
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMaxGreyLevel;          // Grauwertlevel
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMinAblationWidth;      // min. Ablationsbreite
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMaxAblationBrightness; // max. Ablationshelligkeit
        pword++;
        *pword = m_oInspData[p_oSeamNo].m_oQualityGrade[i].m_oMinMillingPos;         // min. Fraeskantenposition
        pword++;
        *pword = 0;         // Reserve 1
        pword++;
    }

    blockLength = (char *)pword - (char *)sendBuffer;
    pword = (int16_t *) &sendBuffer[4];
    *pword = (int16_t) blockLength;
    *bufLength = blockLength;

    wmLog(eDebug, "packInspectData: blockType: %d, p_oSeamNo: %d\n", (DBLOCK_SRING_INSPECTDATA_1_TYPE + p_oSeamNo), p_oSeamNo);
    wmLog(eDebug, "ProdNr:%d, Pres:%d, Len:%d, Setup:%d\n", 
          m_oSeamData[p_oSeamNo].m_oProductNumber, m_oSeamData[p_oSeamNo].m_oSeamPresent, m_oSeamData[p_oSeamNo].m_oSeamLength, m_oSeamData[p_oSeamNo].m_oSetupNumber);
    wmLog(eDebug, "ThickL:%d, ThickR:%d, TDiff:%d, WSpeed:%d, MaxGap:%d\n", 
          m_oSeamData[p_oSeamNo].m_oBlankThicknessLeft, m_oSeamData[p_oSeamNo].m_oBlankThicknessRight, m_oSeamData[p_oSeamNo].m_oTargetDifference,
          m_oSeamData[p_oSeamNo].m_oMaxWeldSpeed, m_oSeamData[p_oSeamNo].m_oMaxGapWidth);
#if 0
    char oDebugStrg1[81];
    char oDebugStrg2[81];
    for(int oSegment = 0;oSegment < MAX_S6K_SEGMENTS;oSegment++)
    {
        sprintf(oDebugStrg1, "Seam: %2d, Segment: %2d, start: %5d", p_oSeamNo, oSegment, m_oInspData[p_oSeamNo].m_oSegment[oSegment].m_oStart);
        sprintf(oDebugStrg2, ", grade: %d", m_oInspData[p_oSeamNo].m_oSegment[oSegment].m_oGrade);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
    }
    for(int oQualGrade = 0;oQualGrade < MAX_S6K_QUALITY_GRADES;oQualGrade++)
    {
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMaxConcavity:     %d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMaxConcavity);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMaxConvexity:     %d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMaxConvexity);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMaxPosMisalign:   %d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMaxPosMisalign);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMaxNegMisalign:   %d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMaxNegMisalign);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMinSeamWidth:     %d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMinSeamWidth);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMaxPoreWidth:     %d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMaxPoreWidth);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMaxGreyLevel:     %d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMaxGreyLevel);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMinAblationWidth: %d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMinAblationWidth);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMaxAblationBright:%d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMaxAblationBrightness);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
        sprintf(oDebugStrg1, "Seam: %2d, Grade: %1d", p_oSeamNo, oQualGrade);
        sprintf(oDebugStrg2, ", m_oMinMillingPos:    %d", m_oInspData[p_oSeamNo].m_oQualityGrade[oQualGrade].m_oMinMillingPos);
        wmLog(eDebug, "%s%s\n", oDebugStrg1, oDebugStrg2);
    }
#endif
}

/***************************************************************************/
/* packStatusData                                                          */
/***************************************************************************/

void TCPClientSRING::packStatusData(char *sendBuffer, int *bufLength)
{
    uint8_t *pubyte;
    int16_t *pword;
    int32_t *pdword;
    uint32_t *pudword;
    int blockLength;
    int i;

    // Write DATABLOCK-Header
    pword = (int16_t *) sendBuffer;

    uint16_t oBlockType{DBLOCK_SRING_STATUSDATA_TYPE};
    if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
    {
        oBlockType = DBLOCK_SSPEED_STATUSDATA_TYPE;
    }

    *pword = (int16_t) oBlockType;
    pword++;
    *pword = (int16_t) DBLOCK_SRING_STATUSDATA_VERSION;
    pword++;
    *pword = 0;           // place for blockLength
    pword++;
    *pword = 0;           // reserve
    pword++;

    // Write DATABLOCK-Information

    // change datatype
    pubyte = (uint8_t *) pword;

    *pubyte = 0x00; // was Machine No.
    pubyte++;

    for(i = 0;i < 3;i++)
    {
        *pubyte = 0x00; // was Souvis_System
        pubyte++;
    }

    // Reserve
    for(i = 0;i < 10;i++)
    {
        *pubyte = 0x00;
        pubyte++;
    }

    // change datatype
    pudword = (uint32_t *) pubyte;

    *pudword = 0x0000;
    if (m_oSystemReadyStatus)
    {
        *pudword |= 0x00000001; // ready, no failure
    }
    else
    {
        *pudword |= 0x00000002; // not ready, failure
    }
    if (m_oSouvisParamReq)
    {
        *pudword |= 0x00010000; // request for parameters from machine
    }
    pudword++;

    // change datatype
    pdword = (int32_t *) pudword;

    *pdword = 0; //ptcpShMem->statusData.errorNo;
    pdword++;

    *pdword = 0;
    pdword++;
    *pdword = 0;
    pdword++;
    *pdword = 0;
    pdword++;
    *pdword = 0;
    pdword++;
    *pdword = 0;
    pdword++;
    *pdword = 0;
    pdword++;
    *pdword = 0;
    pdword++;
    *pdword = 0;
    pdword++;
    *pdword = 0;
    pdword++;

    blockLength = (char *)pdword - (char *)sendBuffer;
    pword = (int16_t *) &sendBuffer[4];
    *pword = (int16_t) blockLength;
    *bufLength = blockLength;

    wmLog(eDebug, "packStatusData: blockType: %d\n", oBlockType);
    wmLog(eDebug, "souvReady:%d, paramReq:%d\n", m_oSystemReadyStatus.load(), m_oSouvisParamReq.load());
}

/***************************************************************************/
/* unpackAcknBlock                                                         */
/***************************************************************************/

void TCPClientSRING::unpackAcknBlock(char *recvBuffer, int32_t& p_oAcknCode, int32_t& p_oBlockType, int32_t& p_oBlocksReceived)
{
    int32_t *pdword;
    int16_t *pword;
    int blockLength1;
    int blockLength2;

    // Read DATABLOCK-Header
    pword = (int16_t *) recvBuffer;

    //xyz = *pword;              // block type
    pword++;
    //xyz = *pword;              // block version
    pword++;
    blockLength1 = *pword;       // block length
    pword++;
    //xyz = *pword;              // reserve
    pword++;

    // Read DATABLOCK-Information
    // change datatype
    pdword = (int32_t *) pword;

    p_oAcknCode = *pdword;
    pdword++;
    p_oBlockType = *pdword;
    pdword++;
    p_oBlocksReceived = *pdword;
    pdword++;
    //packnBlock->reserve2 = *pdword;
    pdword++;
    //packnBlock->reserve3 = *pdword;
    pdword++;
    //packnBlock->reserve4 = *pdword;
    pdword++;

    blockLength2 = (char *)pdword - (char *)recvBuffer;
    if (blockLength1 != blockLength2)
    {
        wmLogTr(eError, "QnxMsg.VI.TCPLengthNotIdent", "%s: Length of datablock not identical %s\n", "TCPClientSRING", "(acknBlock)");
    }
}

/***************************************************************************/
/* dbAltered                                                               */
/***************************************************************************/

void TCPClientSRING::dbAltered( DbChange p_oChange )
{
    Poco::Mutex::ScopedLock lock( m_oDbChangesMutex );

    m_oDbChanges.push_back( p_oChange );
} // dbAltered

/***************************************************************************/
/* dbCheck                                                                 */
/***************************************************************************/

void TCPClientSRING::dbCheck()
{
    Poco::Mutex::ScopedLock lock( m_oDbChangesMutex );

    // Are there any DB changes that were not applied yet?
    if ( m_oDbChanges.size() == 0 )
    {
        return;
    }

    Poco::UUID oStationUUID = Poco::UUID( ConnectionConfiguration::instance().getString("Station.UUID", Poco::UUID::null().toString() ) );
    wmLog(eDebug, "TCPClientSRING::dbCheck: m_oStationUUID: %s\n", oStationUUID.toString().c_str());

    ProductList oProducts = m_rDbProxy.getProductList( oStationUUID );

    // OK, then lets apply the changes ...
    for ( auto oIter = m_oDbChanges.begin(); oIter != m_oDbChanges.end(); ++oIter )
    {
        // A product definition has changed.
        if ( oIter->getStatus() == eProduct )
        {
            for( ProductList::iterator oIterProd = oProducts.begin(); oIterProd!=oProducts.end(); ++oIterProd )
            {
                // Search for product ID. Attention: Only activated products are send to the QNX station, so it could be that the DB has changed
                // the definition of a product, we cannot update it here, as it is not activated and therefore not found ...
                if ( oIterProd->productID() == oIter->getProductID() )
                {
                    wmLog(eDebug, "TCPClientSRING::dbCheck: Product found: %s , %s\n", oIter->getProductID().toString().c_str(), *oIterProd->name().c_str());
                    clearSegmentTable();
                    fillSegmentTable(oStationUUID, oIterProd->productID());
                    clearQualityGrades();
                    if (isSOUVIS6000_Is_PostInspection_Top() || isSOUVIS6000_Is_PostInspection_Bottom())
                    {
                        fillQualityGrades(oIterProd->productID());
                    }
                    if (isSOUVIS6000_Is_PostInspection_Top() || isSOUVIS6000_Is_PostInspection_Bottom())
                    {
                        if (getSOUVIS6000MachineType() != SOUVIS6000MachineType::eS6K_SouBlate)
                        {
                            checkForCompleteness();
                        }
                    }

#if 0
// see also debug possibility at the end of packInspectData
                    printf("\n");
                    printf("InspectData DebugPrint:\n");
                    for(int oSeam = 0;oSeam < 10;oSeam++)
                    {
                        for(int oSegment = 0;oSegment < MAX_S6K_SEGMENTS;oSegment++)
                        {
                            printf("Seam: %2d, Segment: %2d, start: %5d, grade: %d\n",
                                oSeam, oSegment, m_oInspData[oSeam].m_oSegment[oSegment].m_oStart, m_oInspData[oSeam].m_oSegment[oSegment].m_oGrade);
                        }
                        printf("\n");
                        for(int oQualGrade = 0;oQualGrade < 4;oQualGrade++)
                        {
                            printf("Seam: %2d, Grade: %1d, m_oMaxConcavity:     %d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConcavity);
                            printf("Seam: %2d, Grade: %1d, m_oMaxConvexity:     %d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConvexity);
                            printf("Seam: %2d, Grade: %1d, m_oMaxPosMisalign:   %d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPosMisalign);
                            printf("Seam: %2d, Grade: %1d, m_oMaxNegMisalign:   %d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxNegMisalign);
                            printf("Seam: %2d, Grade: %1d, m_oMinSeamWidth:     %d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinSeamWidth);
                            printf("Seam: %2d, Grade: %1d, m_oMaxPoreWidth:     %d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPoreWidth);
                            printf("Seam: %2d, Grade: %1d, m_oMaxGreyLevel:     %d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxGreyLevel);
                            printf("Seam: %2d, Grade: %1d, m_oMinAblationWidth: %d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinAblationWidth);
                            printf("Seam: %2d, Grade: %1d, m_oMaxAblationBright:%d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxAblationBrightness);
                            printf("Seam: %2d, Grade: %1d, m_oMinMillingPos:    %d\n", oSeam, oQualGrade, m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinMillingPos);
                        }
                        printf("\n");
                    }
                    printf("\n");
#endif

                    m_oSendRequestInspectData = true;
                }
            }
        } // if (eProduct)
    } // for

    // Clear the array
    m_oDbChanges.clear();
} // dbCheck

void TCPClientSRING::clearSegmentTable(void)
{
    // clear existing segment table
    for(int oSeam = 0;oSeam < MAX_S6K_SEAM;oSeam++)
    {
        m_oInspData[oSeam].m_oSegment[0].m_oStart = 0;
        m_oInspData[oSeam].m_oSegment[0].m_oGrade = 3;
        for(int oSegment = 1;oSegment < MAX_S6K_SEGMENTS;oSegment++)
        {
            m_oInspData[oSeam].m_oSegment[oSegment].m_oStart = START_POS_OF_LAST_SEGMENT;
            m_oInspData[oSeam].m_oSegment[oSegment].m_oGrade = 3;
        }
    }
    m_oLastPresentSeam = 0;
}

void TCPClientSRING::fillSegmentTable(const Poco::UUID& p_oStationUUID, const Poco::UUID& p_oProductID)
{
    // getMeasureTasks and extract the necessary information
    MeasureTaskList oMeasureTasks = m_rDbProxy.getMeasureTasks(p_oStationUUID, p_oProductID);
    wmLog(eDebug, "TCPClientSRING::dbCheck: getMeasureTasks returns oMeasureTasks.size() <%d>\n", oMeasureTasks.size());
    for( MeasureTaskList::iterator oIterMeasTask = oMeasureTasks.begin(); oIterMeasTask!=oMeasureTasks.end(); ++oIterMeasTask )
    {
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

        if (oIterMeasTask->level() == 2)
        {
            static int oStartPosAccu = 0;
            if (oIterMeasTask->seaminterval() == 0)
            {
                m_oInspData[oIterMeasTask->seam()].m_oSegment[oIterMeasTask->seaminterval()].m_oStart = 0;
                oStartPosAccu = (oIterMeasTask->length() / 1000);
            }
            else
            {
                m_oInspData[oIterMeasTask->seam()].m_oSegment[oIterMeasTask->seaminterval()].m_oStart = oStartPosAccu;
                oStartPosAccu += (oIterMeasTask->length() / 1000);
            }
            m_oInspData[oIterMeasTask->seam()].m_oSegment[oIterMeasTask->seaminterval()].m_oGrade = oIterMeasTask->intervalLevel();
            if (oIterMeasTask->seam() > m_oLastPresentSeam)
            {
                m_oLastPresentSeam = oIterMeasTask->seam();
            }
        }
    }
}

void TCPClientSRING::clearQualityGrades(void)
{
    // clear existing quality grades
    for(int oSeam = 0;oSeam < MAX_S6K_SEAM;oSeam++)
    {
        for(int oQualGrade = 0;oQualGrade < MAX_S6K_QUALITY_GRADES;oQualGrade++)
        {
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConcavity = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConcavityWasSet = false;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConvexity = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConvexityWasSet = false;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPosMisalign = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPosMisalignWasSet = false;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxNegMisalign = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxNegMisalignWasSet = false;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinSeamWidth = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinSeamWidthWasSet = false;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPoreWidth = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPoreWidthWasSet = false;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxGreyLevel = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxGreyLevelWasSet = false;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinAblationWidth = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinAblationWidthWasSet = false;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxAblationBrightness = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxAblationBrightnessWasSet = false;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinMillingPos = 0;
            m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinMillingPosWasSet = false;
        }
    }
}

void TCPClientSRING::fillQualityGrades(const Poco::UUID& p_oProductID)
{
    // getProductParameter and extract the necessary information
    ParameterList oProductParameter = m_rDbProxy.getProductParameter(p_oProductID);
    wmLog(eDebug, "TCPClientSRING::dbCheck: getProductParameter returns oProductParameter.size() <%d>\n", oProductParameter.size());
    for( ParameterList::iterator oIterParameter = oProductParameter.begin(); oIterParameter!=oProductParameter.end(); ++oIterParameter )
    {
        static double oMaxValue = 0.0;
        static int oResultType = 0;
        static double oMinValue = 0.0;
        static std::string oScope("");
        static int oSeam = 0;
        static int oSeamInterval = 0;
        static int oSeamSerie;
        if ((*oIterParameter)->name().compare("Max") == 0)
        {
            oMaxValue = (*oIterParameter)->value<double>();
        }
        if ((*oIterParameter)->name().compare("Result") == 0)
        {
            oResultType = (*oIterParameter)->value<int>();
        }
        if ((*oIterParameter)->name().compare("Min") == 0)
        {
            oMinValue = (*oIterParameter)->value<double>();
        }
        if ((*oIterParameter)->name().compare("Scope") == 0)
        {
            oScope = (*oIterParameter)->value<std::string>();
        }
        if ((*oIterParameter)->name().compare("Seam") == 0)
        {
            oSeam = (*oIterParameter)->value<int>();
        }
        if ((*oIterParameter)->name().compare("SeamInterval") == 0)
        {
            oSeamInterval = (*oIterParameter)->value<int>();
        }
        if ((*oIterParameter)->name().compare("SeamSeries") == 0)
        {
            oSeamSerie = (*oIterParameter)->value<int>();
            char oHelpStrg1[81];
            char oHelpStrg2[81];
            char oHelpStrg3[81];
            char oHelpStrg4[81];
            sprintf(oHelpStrg1, "SeamSerie: %2d Seam: %2d SeamInt: %2d ", oSeamSerie, oSeam, oSeamInterval);
            sprintf(oHelpStrg2, "MessTyp: %4d Scope: %s ", oResultType, oScope.c_str());
            sprintf(oHelpStrg3, "Min: %9.3f Max: %9.3f ", oMinValue, oMaxValue);
            sprintf(oHelpStrg4, "oQualGrade: %d", m_oInspData[oSeam].m_oSegment[oSeamInterval].m_oGrade);
            wmLog(eDebug, "%s%s%s%s\n", oHelpStrg1, oHelpStrg2, oHelpStrg3, oHelpStrg4);

            if (oScope.compare("SeamInterval") == 0) // sum error is on level seam interval
            {
                int oQualGrade = m_oInspData[oSeam].m_oSegment[oSeamInterval].m_oGrade;
                if (oResultType == S6K_Concavity)
                {
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConcavity = static_cast<int16_t>(oMaxValue * 1000);
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConcavityWasSet = true;
                }
                else if (oResultType == S6K_Convexity)
                {
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConvexity = static_cast<int16_t>(oMaxValue * 1000);
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxConvexityWasSet = true;
                }
                else if (oResultType == S6K_HeightDifference)
                {
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPosMisalign = static_cast<int16_t>(oMaxValue * 1000);
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPosMisalignWasSet = true;
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxNegMisalign = std::abs(static_cast<int16_t>(oMinValue * 1000));
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxNegMisalignWasSet = true;
                }
                else if (oResultType == S6K_SeamWidth)
                {
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinSeamWidth = static_cast<int16_t>(oMinValue * 1000);
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinSeamWidthWasSet = true;
                }
                else if (oResultType == S6K_PoreWidth)
                {
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPoreWidth = static_cast<int16_t>(oMaxValue * 1000);
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxPoreWidthWasSet = true;
                }
                else if (oResultType == S6K_GreyLevel)
                {
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxGreyLevel = static_cast<int16_t>(oMaxValue);
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxGreyLevelWasSet = true;
                }
                else if (oResultType == S6K_AblationWidth)
                {
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinAblationWidth = static_cast<int16_t>(oMinValue * 1000);
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinAblationWidthWasSet = true;
                }
                else if (oResultType == S6K_AblationBrightness)
                {
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxAblationBrightness = static_cast<int16_t>(oMaxValue);
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMaxAblationBrightnessWasSet = true;
                }
                else if (oResultType == S6K_MillingEdgePosition)
                {
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinMillingPos = static_cast<int16_t>(oMinValue * 1000);
                    m_oInspData[oSeam].m_oQualityGrade[oQualGrade].m_oMinMillingPosWasSet = true;
                }
            } // if (oScope.compare("SeamInterval") == 0)
        } // if ((*oIterParameter)->name().compare("SeamSerie") == 0)
    } // for( ParameterList::iterator
}

void TCPClientSRING::checkForCompleteness(void)
{
    for(int oSeam = 0;oSeam <= m_oLastPresentSeam;oSeam++)
    {
        // currently there are only 2 quality grades used !
        if ((!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_1].m_oMaxConcavityWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_2].m_oMaxConcavityWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_3].m_oMaxConcavityWasSet))
        {
            wmLogTr(eError, "QnxMsg.VI.SumErrMissConcav", "There is a sum error for concavity missing. seam: %d\n", (oSeam + 1));
        }
        if ((!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_1].m_oMaxConvexityWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_2].m_oMaxConvexityWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_3].m_oMaxConvexityWasSet))
        {
            wmLogTr(eError, "QnxMsg.VI.SumErrMissConvex", "There is a sum error for convexity missing. seam: %d\n", (oSeam + 1));
        }
        if ((!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_1].m_oMaxPosMisalignWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_2].m_oMaxPosMisalignWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_3].m_oMaxPosMisalignWasSet))
        {
            wmLogTr(eError, "QnxMsg.VI.SumErrMissPosMis", "There is a sum error for positive misalignment missing. seam: %d\n", (oSeam + 1));
        }
        if ((!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_1].m_oMaxNegMisalignWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_2].m_oMaxNegMisalignWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_3].m_oMaxNegMisalignWasSet))
        {
            wmLogTr(eError, "QnxMsg.VI.SumErrMissNegMis", "There is a sum error for negative misalignment missing. seam: %d\n", (oSeam + 1));
        }
        if ((!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_1].m_oMinSeamWidthWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_2].m_oMinSeamWidthWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_3].m_oMinSeamWidthWasSet))
        {
            wmLogTr(eError, "QnxMsg.VI.SumErrMissSWidth", "There is a sum error for seam width missing. seam: %d\n", (oSeam + 1));
        }
        if ((!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_1].m_oMaxPoreWidthWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_2].m_oMaxPoreWidthWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_3].m_oMaxPoreWidthWasSet))
        {
            wmLogTr(eError, "QnxMsg.VI.SumErrMissPWidth", "There is a sum error for pore width missing. seam: %d\n", (oSeam + 1));
        }
        if ((!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_1].m_oMaxGreyLevelWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_2].m_oMaxGreyLevelWasSet) &&
            (!m_oInspData[oSeam].m_oQualityGrade[QUALITY_GRADE_3].m_oMaxGreyLevelWasSet))
        {
            wmLogTr(eError, "QnxMsg.VI.SumErrMissGreLev", "There is a sum error for grey level missing. seam: %d\n", (oSeam + 1));
        }
        // additional sum errors for ablation inspection should not be monitored !
    }
}

/***************************************************************************/
/* TCPClientThread                                                         */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void *TCPClientThread(void *p_pArg)
{
    auto pDataToTCPClientThread = static_cast<struct DataToTCPClientThread *>(p_pArg);
    auto pTCPClientSRING = pDataToTCPClientThread->m_pTCPClientSRING;

    wmLog(eDebug, "TCPClientThread is started\n");

    usleep(1000 * 1000); // 1 second delay before starting

    wmLog(eDebug, "TCPClientThread is active\n");

    usleep(100*1000);

    if (pTCPClientSRING != nullptr)
    {
        pTCPClientSRING->run();
    }

    return NULL;
}

} // namespace tcpcommunication

} // namespace precitec


