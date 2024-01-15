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
#include <iomanip>

#include "module/moduleLogger.h"
#include "common/systemConfiguration.h"

#include "TCPCommunication/TCPServerCrossSection.h"

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

///////////////////////////////////////////////////////////
// Prototyp fuer Thread Funktions
///////////////////////////////////////////////////////////

// Thread Funktion muss ausserhalb der Klasse sein
void* TCPServerCrossSectionThread(void *p_pArg);

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

TCPServerCrossSection::TCPServerCrossSection(TS6K_InfoFromProcesses<EventProxy>& p_rS6K_InfoFromProcessesProxy):
        m_rS6K_InfoFromProcessesProxy(p_rS6K_InfoFromProcessesProxy),
        m_oTCPServerCrossSectionThread_ID(0),
        m_oProductNo(0),
        m_oBatchID(0),
        m_oSeamNo(0),
        m_oBlockNo(0),
        m_oFirstMeasureInBlock(0),
        m_oMeasureCntInBlock(0),
        m_oMeasuresPerResult(1),
        m_oValuesPerMeasure(3)
{
    // SystemConfig Switches for SOUVIS6000 application and functions
    m_oSOUVIS6000_TCPIP_Communication_On = SystemConfiguration::instance().getBool("SOUVIS6000_TCPIP_Communication_On", true);
    wmLog(eDebug, "m_oSOUVIS6000_TCPIP_Communication_On (bool): %d\n", m_oSOUVIS6000_TCPIP_Communication_On);

    StartTCPServerCrossSectionThread();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

TCPServerCrossSection::~TCPServerCrossSection(void)
{
    if (m_oTCPServerCrossSectionThread_ID != 0)
    {
        if (pthread_cancel(m_oTCPServerCrossSectionThread_ID) != 0)
        {
            wmLog(eDebug, "TCPServerCrossSection: was not able to abort thread: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(ServerCrossSection)(201)");
        }
    }
}

///////////////////////////////////////////////////////////
// private members
///////////////////////////////////////////////////////////

/***************************************************************************/
/* StartTCPServerCrossSectionThread                                        */
/***************************************************************************/

void TCPServerCrossSection::StartTCPServerCrossSectionThread(void)
{
    ///////////////////////////////////////////////////////
    // Thread für TCPServerCrossSection starten
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToTCPServerCrossSectionThread.m_pTCPServerCrossSection = this;

    if (pthread_create(&m_oTCPServerCrossSectionThread_ID, &oPthreadAttr, &TCPServerCrossSectionThread, &m_oDataToTCPServerCrossSectionThread) != 0)
    {
        wmLog(eDebug, "TCPServerCrossSection: was not able to create thread: %s\n", strerror(errno));
        wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(ServerCrossSection)(202)");
    }
}

int TCPServerCrossSection::run(void)
{
    int sockDesc = 0;
    int actSockDesc;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    socklen_t addrLen;
    bool connectOk;
    char sendBuffer[1500];
    char recvBuffer[1500];
    int recvBytes;
    int32_t blockReceived = 0;
    int lengthToSend;

    int16_t *type;
    int16_t *version;
    int16_t *length;

    /*************************************************************************/

    if (isSOUVIS6000_TCPIP_Communication_On())
    {
        // create socket
        sockDesc = socket(AF_INET, SOCK_STREAM, 0);
        if (sockDesc == -1)
        {
            wmLog(eDebug, "TCPServerCrossSection: Error while creating socket: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(ServerCrossSection)(201)");
        }

        // bind socket
        memset((char *)&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(CROSS_SECTION_TRAILING_SERVER_PORT);

        if (bind(sockDesc, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
        {
            wmLog(eDebug, "TCPServerCrossSection: Error while binding socket: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(ServerCrossSection)(202)");
        }

        // start listening
        if (listen(sockDesc, 5) == -1)
        {
            wmLog(eDebug, "TCPServerCrossSection: Error while start listening: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(ServerCrossSection)(203)");
        }
    }

    /****** main loop ********************************************************/

    while (true)
    {
        if (isSOUVIS6000_TCPIP_Communication_On())
        {
            addrLen = sizeof(clientAddr);
            actSockDesc = accept(sockDesc, (struct sockaddr *)&clientAddr, &addrLen);
            if (actSockDesc == -1)
            {
                wmLog(eDebug, "TCPServerCrossSection: Error while accepting connection: %s\n", strerror(errno));
                wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(ServerCrossSection)(204)");
            }
            wmLogTr(eInfo, "QnxMsg.VI.TCPCommStarted", "TCP/IP Connection started %s\n", "(ServerCrossSection)");
            connectOk = true;

            while (connectOk)
            {
                recvBytes = recv(actSockDesc, recvBuffer, sizeof(recvBuffer), 0);
                if ((recvBytes == 0)||(recvBytes == -1))
                {
                    wmLogTr(eInfo, "QnxMsg.VI.TCPCommEnded", "TCP/IP Connection ended %s\n", "(ServerCrossSection)");
                    connectOk = false;
                }
                else
                {
                    blockReceived++;
                    type = (int16_t *) &recvBuffer[0];
                    version = (int16_t *) &recvBuffer[2];
                    length = (int16_t *) &recvBuffer[4];

                    char oVersionStrg[21];
                    sprintf(oVersionStrg, "%X", (int16_t)*version);
                    wmLog(eDebug,"TCPServerCrossSection: blockType = %d , blockVersion = %s , blockLength = %d\n", (int16_t)*type, oVersionStrg, (int16_t)*length);

                    switch(*type)
                    {
                        case DBLOCK_CROSS_SECTION_TYPE:
                            unpackCrossSectionData(recvBuffer);
                            break;
                        default:
                            wmLogTr(eWarning, "QnxMsg.VI.TCPUnknownBlock", "unknown blocktype via TCP/IP received: %d\n", *type);
                            break;
                    }

                    // Acknowledge senden
                    packAcknBlock(sendBuffer, &lengthToSend, DBLOCK_ACKN_OK, *type, blockReceived);
                    send(actSockDesc, sendBuffer, lengthToSend, 0);
                }
            }
            close (actSockDesc);
        }
        else
        {
            usleep(1000*1000);
        }
    }          /* end while !quit */

    if (isSOUVIS6000_TCPIP_Communication_On())
    {
        close (sockDesc);
    }
    return 0;
}

/***************************************************************************/
/* unpackCrossSectionData                                                  */
/***************************************************************************/

void TCPServerCrossSection::unpackCrossSectionData(char *recvBuffer)
{
    uint8_t *pubyte;
    int16_t *pword;
    uint16_t *puword;
    uint32_t *pudword;
    int16_t oBlockType = 0;
    int blockLength1;
    int blockLength2;

    // Read DATABLOCK-Header
    pword = (int16_t *) recvBuffer;

    // bytes 0,1
    oBlockType = *pword;         // block type
    pword++;
    // bytes 2,3
    //xyz = *pword;              // block version
    pword++;
    // bytes 4,5
    blockLength1 = *pword;       // block length
    pword++;
    // bytes 6,7
    //xyz = *pword;              // reserve
    pword++;

    // Read DATABLOCK-Information

    // change datatype
    pudword = (uint32_t *) pword;

    // bytes 8-11
    m_oProductNo = *pudword; // product number
    pudword++;

    // bytes 12-15
    m_oBatchID = *pudword; // cycle ID
    pudword++;

    // change datatype
    puword = (uint16_t *) pudword;

    // bytes 16,17
    m_oSeamNo = *puword; // seam number
    puword++;

    // bytes 18,19
    m_oBlockNo = *puword; // block number
    puword++;

    // bytes 20,21
    m_oFirstMeasureInBlock = *puword; // first measure number in this block
    puword++;

    // bytes 22,23
    m_oMeasureCntInBlock = *puword; // measure count in block
    puword++;

    // bytes 24,25
    m_oMeasuresPerResult = *puword; // measure count per result
    puword++;

    // bytes 26,27
    m_oValuesPerMeasure = *puword; // value count per measure
    puword++;

    // change datatype
    pubyte = (uint8_t *) puword;

    // bytes 28 ff
    memcpy(m_oCS_DataBlock.data(), pubyte, sizeof(m_oCS_DataBlock));
    pubyte += sizeof(m_oCS_DataBlock);

    blockLength2 = (char *)pubyte - (char *)recvBuffer;
    if (blockLength1 != blockLength2)
    {
        wmLogTr(eError, "QnxMsg.VI.TCPLengthNotIdent", "%s: Length of datablock not identical %s\n", "TCPServerCrossSection", "(CrossSectionData)");
    }

    wmLog(eDebug, "unpackCrossSectionData: blockType: %d, blockLength: %d\n", oBlockType, blockLength2);
    char oHelpStrg1[81];
    char oHelpStrg2[81];
    char oHelpStrg3[81];
    char oHelpStrg4[81];
    sprintf(oHelpStrg1, "Prod: %2d, Batch: %3d, ", m_oProductNo, m_oBatchID);
    sprintf(oHelpStrg2, "Seam: %2d, Block: %2d, ", m_oSeamNo, m_oBlockNo);
    sprintf(oHelpStrg3, "FirstMeas: %2d, MeasCnt: %2d, ", m_oFirstMeasureInBlock, m_oMeasureCntInBlock);
    sprintf(oHelpStrg4, "MeasRes: %2d, Values: %2d", m_oMeasuresPerResult, m_oValuesPerMeasure);
    wmLog(eDebug, "%s%s%s%s\n", oHelpStrg1, oHelpStrg2, oHelpStrg3, oHelpStrg4);

    m_rS6K_InfoFromProcessesProxy.passS6K_CS_DataBlock_To_Inspect(m_oProductNo, m_oBatchID, m_oSeamNo, m_oBlockNo, m_oFirstMeasureInBlock, m_oMeasureCntInBlock, m_oMeasuresPerResult, m_oValuesPerMeasure, m_oCS_DataBlock);
}

/***************************************************************************/
/* packAcknBlock                                                           */
/***************************************************************************/

void TCPServerCrossSection::packAcknBlock(char *sendBuffer, int *bufLength, int32_t p_oAcknCode, int32_t p_oBlockType, int32_t p_oBlocksReceived)
{
    int32_t *pdword;
    int16_t *pword;
    int blockLength;

    // Write DATABLOCK-Header
    pword = (int16_t *) sendBuffer;

    *pword = (int16_t) DBLOCK_ACKN_TYPE;
    pword++;
    *pword = (int16_t) DBLOCK_ACKN_VERSION;
    pword++;
    *pword = 0;           // place for blockLength
    pword++;
    *pword = 0;           // reserve
    pword++;

    // Write DATABLOCK-Information
    // change datatype
    pdword = (int32_t *) pword;

    *pdword = p_oAcknCode;
    pdword++;
    *pdword = p_oBlockType;
    pdword++;
    *pdword = p_oBlocksReceived;
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
}

/***************************************************************************/
/* TCPServerCrossSectionThread                                             */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void *TCPServerCrossSectionThread(void *p_pArg)
{
    auto pDataToTCPServerCrossSectionThread = static_cast<struct DataToTCPServerCrossSectionThread *>(p_pArg);
    auto pTCPServerCrossSection = pDataToTCPServerCrossSectionThread->m_pTCPServerCrossSection;

    wmLog(eDebug, "TCPServerThread <CrossSection> is started\n");

    usleep(1000 * 1000); // 1 second delay before starting

    wmLog(eDebug, "TCPServerThread <CrossSection>  is active\n");

    usleep(100*1000);

    if (pTCPServerCrossSection != nullptr)
    {
        pTCPServerCrossSection->run();
    }

    return NULL;
}

} // namespace tcpcommunication

} // namespace precitec

