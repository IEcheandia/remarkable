/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2020
 *  @brief      Handles the TCP/IP communication with the other PostInspection system
 */

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "module/moduleLogger.h"

#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"

#include "TCPCommunication/TCPClientCrossSection.h"

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

///////////////////////////////////////////////////////////
// Prototyp fuer Thread Funktions
///////////////////////////////////////////////////////////

// Thread Funktion muss ausserhalb der Klasse sein
void* TCPClientCrossSectionThread (void *p_pArg);

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

TCPClientCrossSection::TCPClientCrossSection():
        m_oTCPClientCrossSectionThread_ID(0),
        m_oIPAddressOfCrossSectionOther(0x0100007F), // 127.0.0.1 in network byte order
        m_oSockDesc(-1),
        m_oTCPConnectionIsOn(false),
        m_oOpenRequestCrossSectionConnection(false),
        m_oSendRequestCrossSectionData(false),
        m_oCloseRequestCrossSectionConnection(false),
        m_oCycleDataBatchID(0),
        m_oCycleDataProductNo(0),
        m_oSeamNo(1),
        m_oBlockNo(1),
        m_oFirstMeasureInBlock(1),
        m_oMeasureCntInBlock(10),
        m_oMeasuresPerResult(1),
        m_oValuesPerMeasure(3)
{
    // SystemConfig Switches for SOUVIS6000 application and functions
    m_oSOUVIS6000_TCPIP_Communication_On = SystemConfiguration::instance().getBool("SOUVIS6000_TCPIP_Communication_On", true);
    wmLog(eDebug, "m_oSOUVIS6000_TCPIP_Communication_On (bool): %d\n", m_oSOUVIS6000_TCPIP_Communication_On);
    m_oSOUVIS6000_IP_Address_CrossSection_Other = SystemConfiguration::instance().getString("SOUVIS6000_IP_Address_CrossSection_Other", "127.0.0.1");
    wmLog(eDebug, "m_oSOUVIS6000_IP_Address_CrossSection_Other: <%s>\n", m_oSOUVIS6000_IP_Address_CrossSection_Other.c_str());

    int oRetValue = inet_pton(AF_INET, m_oSOUVIS6000_IP_Address_CrossSection_Other.c_str(), &m_oIPAddressOfCrossSectionOther);
    if (oRetValue != 1)
    {
        wmLogTr(eError, "QnxMsg.VI.TCPWrongIPFormat", "wrong format of IP address string\n");
        m_oIPAddressOfCrossSectionOther = 0x0100007F; // 127.0.0.1 in network byte order
    }
    char oHelpStrg[21];
    sprintf(oHelpStrg, "%08X", m_oIPAddressOfCrossSectionOther);
    wmLog(eDebug, "m_oIPAddressOfCrossSectionOther: %s, retVal: %d\n", oHelpStrg, oRetValue); // in network byte order

    StartTCPClientCrossSectionThread();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

TCPClientCrossSection::~TCPClientCrossSection(void)
{
    if (m_oTCPClientCrossSectionThread_ID != 0)
    {
        if (pthread_cancel(m_oTCPClientCrossSectionThread_ID) != 0)
        {
            wmLog(eDebug, "TCPClientCrossSection: was not able to abort thread: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(ClientCrossSection)(101)");
        }
    }
}

void TCPClientCrossSection::setS6K_CS_DataBlock (CS_TCP_MODE p_oSendCmd, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,  // interface InspectionOut
                                                 uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock)
{
    if (p_oSendCmd == eCS_TCP_OPEN_CONNECTION)
    {
        m_oOpenRequestCrossSectionConnection.store(true);
    }
    else if (p_oSendCmd == eCS_TCP_SEND_DATABLOCK)
    {
        m_oSeamNo = p_oSeamNo;
        m_oBlockNo = p_oBlockNo;
        m_oFirstMeasureInBlock = p_oFirstMeasureInBlock;
        m_oMeasureCntInBlock = p_oMeasureCntInBlock;
        m_oMeasuresPerResult = p_oMeasuresPerResult;
        m_oValuesPerMeasure = p_oValuesPerMeasure;
        m_oCS_DataBlock1 = p_oCS_DataBlock;
        m_oSendRequestCrossSectionData.store(true);
    }
    else if (p_oSendCmd == eCS_TCP_CLOSE_CONNECTION)
    {
        m_oCloseRequestCrossSectionConnection.store(true);
    }
}

///////////////////////////////////////////////////////////
// private members
///////////////////////////////////////////////////////////

/***************************************************************************/
/* StartTCPClientCrossSectionThread                                        */
/***************************************************************************/

void TCPClientCrossSection::StartTCPClientCrossSectionThread(void)
{
    ///////////////////////////////////////////////////////
    // Thread für TCPClientCrossSection starten
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToTCPClientCrossSectionThread.m_pTCPClientCrossSection = this;

    if (pthread_create(&m_oTCPClientCrossSectionThread_ID, &oPthreadAttr, &TCPClientCrossSectionThread, &m_oDataToTCPClientCrossSectionThread) != 0)
    {
        wmLog(eDebug, "TCPClientCrossSection: was not able to create thread: %s\n", strerror(errno));
        wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(ClientCrossSection)(102)");
    }
}

int TCPClientCrossSection::run(void)
{
    /****** main loop ********************************************************/

    sleep(8);

    while (true)
    {
        // check if there has to be sent a block
        checkSendRequest();

        usleep(5*1000); // 5 ms
    }          /* end while !quit */
    return 0;
}

/***************************************************************************/
/* checkSendRequest                                                        */
/***************************************************************************/

void TCPClientCrossSection::checkSendRequest (void)
{
    static bool oSendRequestAll = false;
    static bool oOpenRequestCrossSectionConnection = false;
    static int  oOpenRequestErrorCounter = 0;
    static bool oSendRequestCrossSectionData = false;
    static bool oCloseRequestCrossSectionConnection = false;

    /*************************************************************************/

    // determine if there is at least one request
    if (m_oOpenRequestCrossSectionConnection.load() == true)
    {
        m_oOpenRequestCrossSectionConnection.store(false);
        oSendRequestAll = true;
        oOpenRequestCrossSectionConnection = true;
        oOpenRequestErrorCounter = 0;
    }
    if (m_oSendRequestCrossSectionData.load() == true)
    {
        m_oSendRequestCrossSectionData.store(false);
        oSendRequestAll = true;
        oSendRequestCrossSectionData = true;
    }
    if (m_oCloseRequestCrossSectionConnection.load() == true)
    {
        m_oCloseRequestCrossSectionConnection.store(false);
        oSendRequestAll = true;
        oCloseRequestCrossSectionConnection = true;
    }

    // return from function if there is no send request
    if (oSendRequestAll == false) return;

    // sending of at least one block is requested
    if (isSOUVIS6000_TCPIP_Communication_On())
    {
        // check if open CrossSection connection is requested
        if (oOpenRequestCrossSectionConnection == true)
        {
            oOpenRequestCrossSectionConnection = false;
            int oRetValue = openCrossSectionConnection();
            if (oRetValue == 1) // successful
            {
                m_oTCPConnectionIsOn = true;
            }
            else if (oRetValue == -1) // fatal error
            {
                oSendRequestAll = false;
                oOpenRequestCrossSectionConnection = false;
                oSendRequestCrossSectionData = false;
                oCloseRequestCrossSectionConnection = false;
            }
            else // repeat
            {
                oOpenRequestErrorCounter++;
                if (oOpenRequestErrorCounter < 5)
                {
                    oOpenRequestCrossSectionConnection = true;
                }
                else
                {
                    wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(ClientCrossSection)(111)");
                    oOpenRequestCrossSectionConnection = false;
                    oCloseRequestCrossSectionConnection = true;
                }
            }
        }

        // check if CrossSection data block has to be sent
        if (oSendRequestCrossSectionData == true)
        {
            oSendRequestCrossSectionData = false;
            int oRetValue = sendCrossSectionData();
            if (oRetValue == 1) // successful
            {
            }
            else if (oRetValue == -1) // fatal error
            {
                oSendRequestAll = false;
                oOpenRequestCrossSectionConnection = false;
                oSendRequestCrossSectionData = false;
                oCloseRequestCrossSectionConnection = false;
            }
            else // repeat
            {
                oSendRequestCrossSectionData = true;
            }
        }

        // check if close CrossSection connection is requested
        if (oCloseRequestCrossSectionConnection == true)
        {
            oCloseRequestCrossSectionConnection = false;
            int oRetValue = closeCrossSectionConnection();
            if (oRetValue == 1) // successful
            {
                m_oTCPConnectionIsOn = false;
            }
            else // fatal error
            {
                oSendRequestAll = false;
                oOpenRequestCrossSectionConnection = false;
                oSendRequestCrossSectionData = false;
                oCloseRequestCrossSectionConnection = false;
            }
        }

        if ((oOpenRequestCrossSectionConnection == false) &&
            (oSendRequestCrossSectionData == false) &&
            (oCloseRequestCrossSectionConnection == false))
        {
            oSendRequestAll = false;
        }
    }
    else // if(isSOUVIS6000_TCPIP_Communication_On)
    {
        oSendRequestAll = false;
        oOpenRequestCrossSectionConnection = false;
        oSendRequestCrossSectionData = false;
        oCloseRequestCrossSectionConnection = false;
    }
}

/***************************************************************************/
/* openCrossSectionConnection                                              */
/***************************************************************************/

int TCPClientCrossSection::openCrossSectionConnection(void)
{
    struct sockaddr_in serverAddr;

    // create socket
    m_oSockDesc = socket(AF_INET, SOCK_STREAM, 0);
    if (m_oSockDesc == -1)
    {
        wmLog(eDebug, "TCPClientCrossSection: Error while creating socket: %s\n", strerror(errno));
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(ClientCrossSection)(101)");
        // cannot open socket, all pending send requests are stopped
        return -1;
    }

    memset((char *)&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    memcpy(&serverAddr.sin_addr, &m_oIPAddressOfCrossSectionOther, 4); // in network byte order
    serverAddr.sin_port = htons(CROSS_SECTION_TRAILING_SERVER_PORT);

    // connect to Server
    if (connect(m_oSockDesc, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        if (errno == ECONNREFUSED)
        {
            wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(ClientCrossSection)(001)");
            close (m_oSockDesc);
            sleep(2);
            return 0;
        }
        else if (errno == ETIMEDOUT)
        {
            wmLogTr(eWarning, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(ClientCrossSection)(002)");
            close (m_oSockDesc);
            return 0;
        }
        else
        {
            wmLog(eDebug, "TCPClientCrossSection: Error while connecting server: %s\n", strerror(errno));
            close (m_oSockDesc);
            wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(ClientCrossSection)(102)");
            // cannot connect to server, all pending send requests are stopped
            return -1;
        }
    }
    wmLogTr(eInfo, "QnxMsg.VI.TCPCommStarted", "TCP/IP Connection started %s\n", "(ClientCrossSection)");

    return 1;
}

/***************************************************************************/
/* closeCrossSectionConnection                                             */
/***************************************************************************/

int TCPClientCrossSection::closeCrossSectionConnection(void)
{
    // close connection and socket
    close (m_oSockDesc);
    m_oSockDesc = -1;

    wmLogTr(eInfo, "QnxMsg.VI.TCPCommEnded", "TCP/IP Connection ended %s\n", "(ClientCrossSection)");

    return 1;
}

/***************************************************************************/
/* sendCrossSectionData                                                    */
/***************************************************************************/

int TCPClientCrossSection::sendCrossSectionData(void)
{
    char sendBuffer[1500];
    char recvBuffer[1500];
    char statusStrg[121];
    int lengthToSend;

    if ( !m_oTCPConnectionIsOn )
    {
        wmLogTr(eError, "QnxMsg.VI.TCPCommNoConn", "connection to server not possible %s\n", "(ClientCrossSection)(003)");
        return -1;
    }

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 1;
    oTcpTimeout.tv_usec = 0;
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

    packCrossSectionData(sendBuffer, &lengthToSend);
    if (send(m_oSockDesc, sendBuffer, lengthToSend, 0) == -1)
    {
        wmLog(eDebug, "TCPClientCrossSection: was not able to send via TCP/IP: %s\n", strerror(errno));
        wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(ClientCrossSection)(106)");
        return -1;
    }

    int16_t *type = (int16_t *) &sendBuffer[0];
    int16_t *version = (int16_t *) &sendBuffer[2];
    int16_t *length = (int16_t *) &sendBuffer[4];
    sprintf(statusStrg,"TCPClientCrossSection: blockType = %d , blockVersion = %04X , blockLength = %d\n",
            *type,*version,*length);
    wmLog(eDebug, statusStrg);

    if (recv(m_oSockDesc, recvBuffer, sizeof(recvBuffer), 0) <= 0)
    {
        wmLog(eDebug, "TCPClientCrossSection: was not able to recv via TCP/IP: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(ClientCrossSection)(108)");
        return 0;
    }
    int32_t oAcknCode;
    int32_t oBlockType;
    int32_t oBlocksReceived;
    unpackAcknBlock(recvBuffer, oAcknCode, oBlockType, oBlocksReceived);

    sprintf(statusStrg,"TCPClientCrossSection: acknCode = %4d ; blockType = %d ; reserve1 = %d\n",
                    oAcknCode, oBlockType, oBlocksReceived);
    wmLog(eDebug, statusStrg);

    return 1;
}

/***************************************************************************/
/* packCrossSectionData                                                    */
/***************************************************************************/

void TCPClientCrossSection::packCrossSectionData(char *sendBuffer, int *bufLength)
{
    uint8_t *pubyte;
    int16_t *pword;
    uint16_t *puword;
    uint32_t *pudword;
    int blockLength;

    // Write DATABLOCK-Header
    pword = (int16_t *) sendBuffer;

    // bytes 0,1
    *pword = (int16_t) DBLOCK_CROSS_SECTION_TYPE;
    pword++;
    // bytes 2,3
    *pword = (int16_t) DBLOCK_CROSS_SECTION_VERSION;
    pword++;
    // bytes 4,5
    *pword = 0;           // place for blockLength
    pword++;
    // bytes 6,7
    *pword = 0;           // reserve
    pword++;

    // Write DATABLOCK-Information

    // change datatype
    pudword = (uint32_t *) pword;

    // bytes 8-11
    *pudword = m_oCycleDataProductNo.load(); // product number
    pudword++;

    // bytes 12-15
    *pudword = m_oCycleDataBatchID.load(); // cycle ID
    pudword++;

    // change datatype
    puword = (uint16_t *) pudword;

    // bytes 16,17
    *puword = m_oSeamNo; // seam number
    puword++;

    // bytes 18,19
    *puword = m_oBlockNo; // block number
    puword++;

    // bytes 20,21
    *puword = m_oFirstMeasureInBlock; // first measure number in this block
    puword++;

    // bytes 22,23
    *puword = m_oMeasureCntInBlock; // measure count in block
    puword++;

    // bytes 24,25
    *puword = m_oMeasuresPerResult; // measure count per result
    puword++;

    // bytes 26,27
    *puword = m_oValuesPerMeasure; // value count per measure
    puword++;

    // change datatype
    pubyte = (uint8_t *) puword;

    // bytes 28 ff
    memcpy(pubyte, m_oCS_DataBlock1.data(), sizeof(m_oCS_DataBlock1));
    pubyte += sizeof(m_oCS_DataBlock1);

    blockLength = (char *)pubyte - (char *)sendBuffer;
    pword = (int16_t *) &sendBuffer[4];
    *pword = (int16_t) blockLength;
    *bufLength = blockLength;

    wmLog(eDebug, "packCrossSectionData: blockType: %d, blockLength: %d\n", DBLOCK_CROSS_SECTION_TYPE, blockLength);
}

/***************************************************************************/
/* unpackAcknBlock                                                         */
/***************************************************************************/

void TCPClientCrossSection::unpackAcknBlock(char *recvBuffer, int32_t& p_oAcknCode, int32_t& p_oBlockType, int32_t& p_oBlocksReceived)
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
        wmLogTr(eError, "QnxMsg.VI.TCPLengthNotIdent", "%s: Length of datablock not identical %s\n", "TCPClientCrossSection", "(acknBlock)");
    }
}

/***************************************************************************/
/* TCPClientCrossSectionThread                                             */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void *TCPClientCrossSectionThread(void *p_pArg)
{
    auto pDataToTCPClientCrossSectionThread = static_cast<struct DataToTCPClientCrossSectionThread *>(p_pArg);
    auto pTCPClientCrossSection = pDataToTCPClientCrossSectionThread->m_pTCPClientCrossSection;

    wmLog(eDebug, "TCPClientThread <CrossSection> is started\n");

    usleep(1000 * 1000); // 1 second delay before starting

    wmLog(eDebug, "TCPClientThread <CrossSection> is active\n");

    usleep(100*1000);

    if (pTCPClientCrossSection != nullptr)
    {
        pTCPClientCrossSection->run();
    }

    return NULL;
}

} // namespace tcpcommunication

} // namespace precitec


