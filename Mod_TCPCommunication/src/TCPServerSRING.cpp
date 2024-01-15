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
#include <iomanip>
#include <fstream>

#include "module/moduleLogger.h"

#include "Poco/DirectoryIterator.h"

#include "json.hpp"
#include "TCPCommunication/TCPServerSRING.h"

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

///////////////////////////////////////////////////////////
// Prototyp fuer Thread Funktions
///////////////////////////////////////////////////////////

// Thread Funktion muss ausserhalb der Klasse sein
void* TCPServerThread(void *p_pArg);

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

TCPServerSRING::TCPServerSRING(void):
        m_oTCPServerThread_ID(0),
        m_oIsSOUVIS6000_Application(false),
        m_oSOUVIS6000_TCPIP_Communication_On(true),
        m_oProductNo(0),
        m_oMaxSouvisSpeed(300), // [100mm/min] -> here: 30000 mm/min -> 30 m/min
        m_oSouvisPresent(true),
        m_oSouvisSelected(true),
        m_oSendStatusDataToMC(false)
{
    // SystemConfig Switches for SOUVIS6000 application and functions
    m_oIsSOUVIS6000_Application = SystemConfiguration::instance().getBool("SOUVIS6000_Application", false);
    wmLog(eDebug, "m_oIsSOUVIS6000_Application (bool): %d\n", m_oIsSOUVIS6000_Application);
    m_oSOUVIS6000_Machine_Type = static_cast<SOUVIS6000MachineType>(SystemConfiguration::instance().getInt("SOUVIS6000_Machine_Type", 0) );
    wmLog(eDebug, "m_oSOUVIS6000_Machine_Type (int): %d\n", static_cast<int>(m_oSOUVIS6000_Machine_Type));
    m_oSOUVIS6000_TCPIP_Communication_On = SystemConfiguration::instance().getBool("SOUVIS6000_TCPIP_Communication_On", true);
    wmLog(eDebug, "m_oSOUVIS6000_TCPIP_Communication_On (bool): %d\n", m_oSOUVIS6000_TCPIP_Communication_On);

    memset(&m_oSeamData, 0, sizeof(m_oSeamData));

    StartTCPServerThread();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

TCPServerSRING::~TCPServerSRING(void)
{
    if (m_oTCPServerThread_ID != 0)
    {
        if (pthread_cancel(m_oTCPServerThread_ID) != 0)
        {
            wmLog(eDebug, "TCPServerSRING: was not able to abort thread: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(201)");
        }
    }
}

void TCPServerSRING::connectCallback_1(TCPCallbackFunction_Type1 p_pTCPCallbackFunction_1)
{
    m_pTCPCallbackFunction_1 = p_pTCPCallbackFunction_1;
}

void TCPServerSRING::connectCallback_2(TCPCallbackFunction_Type1 p_pTCPCallbackFunction_2)
{
    m_pTCPCallbackFunction_2 = p_pTCPCallbackFunction_2;
}

void TCPServerSRING::GetGlobData(uint32_t& p_oSpeed, bool& p_oPresent, bool& p_oSelected, bool& p_oSendStatus)
{
    p_oSpeed = m_oMaxSouvisSpeed;
    p_oPresent = m_oSouvisPresent;
    p_oSelected = m_oSouvisSelected;
    p_oSendStatus = m_oSendStatusDataToMC;
}

void TCPServerSRING::GetSeamData(int p_oSeamNo, struct SeamDataStruct& p_oSeamData)
{
    Poco::Mutex::ScopedLock lock( m_oSeamDataMutex );

    p_oSeamData = m_oSeamData[p_oSeamNo];
}

///////////////////////////////////////////////////////////
// private members
///////////////////////////////////////////////////////////

/***************************************************************************/
/* StartTCPServerThread                                                    */
/***************************************************************************/

void TCPServerSRING::StartTCPServerThread(void)
{
    if (m_oIsSOUVIS6000_Application)
    {
        ///////////////////////////////////////////////////////
        // Thread für TCPServer starten
        ///////////////////////////////////////////////////////
        pthread_attr_t oPthreadAttr;
        pthread_attr_init(&oPthreadAttr);

        m_oDataToTCPServerThread.m_pTCPServerSRING = this;

        if (pthread_create(&m_oTCPServerThread_ID, &oPthreadAttr, &TCPServerThread, &m_oDataToTCPServerThread) != 0)
        {
            wmLog(eDebug, "TCPServerSRING: was not able to create thread: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(202)");
        }
    }
}

int TCPServerSRING::run(void)
{
    int sockDesc = 0;
    int actSockDesc;
    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;
    socklen_t addrLen;
    bool connectOk;
    bool newSeamDataReceived;
    char sendBuffer[1500];
    char recvBuffer[1500];
    int recvBytes;
    int32_t blockReceived = 0;
    int lengthToSend;

    int16_t *type;
    int16_t *version;
    int16_t *length;

    /*************************************************************************/

    if (m_oSOUVIS6000_TCPIP_Communication_On)
    {
        // create socket
        sockDesc = socket(AF_INET, SOCK_STREAM, 0);
        if (sockDesc == -1)
        {
            wmLog(eDebug, "TCPServerSRING: Error while creating socket: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(201)");
        }

        // bind socket
        memset((char *)&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        if (SystemConfiguration::instance().getBool("SOUVIS6000_Is_PreInspection", false) == true)
        {
            serverAddr.sin_port = htons(SOURING_S6K_SERVER_PORT_S1);
        }
        else if (SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Top", false) == true)
        {
            serverAddr.sin_port = htons(SOURING_S6K_SERVER_PORT_S2U);
        }
        else if (SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Bottom", false) == true)
        {
            serverAddr.sin_port = htons(SOURING_S6K_SERVER_PORT_S2L);
        }
        else
        {
            wmLogTr(eError, "QnxMsg.VI.TCPCommNoSubsys", "No subsystem of SOUVIS6000 system is configured !\n");
            serverAddr.sin_port = htons(SOURING_S6K_SERVER_PORT_S1);
        }

        if (bind(sockDesc, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
        {
            wmLog(eDebug, "TCPServerSRING: Error while binding socket: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(202)");
        }

        // start listening
        if (listen(sockDesc, 5) == -1)
        {
            wmLog(eDebug, "TCPServerSRING: Error while start listening: %s\n", strerror(errno));
            wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(203)");
        }
    }

    /****** main loop ********************************************************/

    while (true)
    {
        if (m_oSOUVIS6000_TCPIP_Communication_On)
        {
            addrLen = sizeof(clientAddr);
            actSockDesc = accept(sockDesc, (struct sockaddr *)&clientAddr, &addrLen);
            if (actSockDesc == -1)
            {
                wmLog(eDebug, "TCPServerSRING: Error while accepting connection: %s\n", strerror(errno));
                wmFatal(eBusSystem, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(204)");
            }
            wmLogTr(eInfo, "QnxMsg.VI.TCPCommStarted", "TCP/IP Connection started %s\n", "(Server)");
            connectOk = true;
            newSeamDataReceived = false;

            while (connectOk)
            {
                recvBytes = recv(actSockDesc, recvBuffer, sizeof(recvBuffer), 0);
                if ((recvBytes == 0)||(recvBytes == -1))
                {
                    wmLogTr(eInfo, "QnxMsg.VI.TCPCommEnded", "TCP/IP Connection ended %s\n", "(Server)");
                    connectOk = false;
                    if (newSeamDataReceived)
                    {
                        if (m_pTCPCallbackFunction_2)
                        {
                            m_pTCPCallbackFunction_2(*type);
                        }

                        // insert new seamData in product file
                        wmLog(eDebug, "TCPServerSRING: m_oProductNo: %d\n", m_oProductNo.load());
                        InsertDataInProductFile();

                        newSeamDataReceived = false;
                    }
                }
                else
                {
                    blockReceived++;
                    type = (int16_t *) &recvBuffer[0];
                    version = (int16_t *) &recvBuffer[2];
                    length = (int16_t *) &recvBuffer[4];

                    char oVersionStrg[21];
                    sprintf(oVersionStrg, "%X", (int16_t)*version);
                    wmLog(eDebug,"TCPServerSRING: blockType = %d , blockVersion = %s , blockLength = %d\n", (int16_t)*type, oVersionStrg, (int16_t)*length);

                    switch(*type)
                    {
                        case DBLOCK_SRING_GLOBDATA_TYPE:
                            unpackGlobData(recvBuffer);
                            if (m_pTCPCallbackFunction_1)
                            {
                                m_pTCPCallbackFunction_1(*type);
                            }
                            break;
                        case DBLOCK_SRING_SEAMDATA_1_TYPE:
                        case DBLOCK_SRING_SEAMDATA_2_TYPE:
                        case DBLOCK_SRING_SEAMDATA_3_TYPE:
                        case DBLOCK_SRING_SEAMDATA_4_TYPE:
                        case DBLOCK_SRING_SEAMDATA_5_TYPE:
                        case DBLOCK_SRING_SEAMDATA_6_TYPE:
                        case DBLOCK_SRING_SEAMDATA_7_TYPE:
                        case DBLOCK_SRING_SEAMDATA_8_TYPE:
                        case DBLOCK_SRING_SEAMDATA_9_TYPE:
                        case DBLOCK_SRING_SEAMDATA_10_TYPE:
                        case DBLOCK_SRING_SEAMDATA_11_TYPE:
                        case DBLOCK_SRING_SEAMDATA_12_TYPE:
                        case DBLOCK_SRING_SEAMDATA_13_TYPE:
                        case DBLOCK_SRING_SEAMDATA_14_TYPE:
                        case DBLOCK_SRING_SEAMDATA_15_TYPE:
                        case DBLOCK_SRING_SEAMDATA_16_TYPE:
                        case DBLOCK_SRING_SEAMDATA_17_TYPE:
                        case DBLOCK_SRING_SEAMDATA_18_TYPE:
                        case DBLOCK_SRING_SEAMDATA_19_TYPE:
                        case DBLOCK_SRING_SEAMDATA_20_TYPE:
                        case DBLOCK_SRING_SEAMDATA_21_TYPE:
                        case DBLOCK_SRING_SEAMDATA_22_TYPE:
                        case DBLOCK_SRING_SEAMDATA_23_TYPE:
                        case DBLOCK_SRING_SEAMDATA_24_TYPE:
                            unpackSeamData(recvBuffer, (*type - DBLOCK_SRING_SEAMDATA_1_TYPE));
                            newSeamDataReceived = true;
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

    if (m_oSOUVIS6000_TCPIP_Communication_On)
    {
        close (sockDesc);
    }
    return 0;
}

int TCPServerSRING::TestDataInsert(void)
{
#if 0
    m_oSeamData[0].m_oProductNumber = 14;
    m_oSeamData[0].m_oSeamPresent = 1;
    m_oSeamData[0].m_oSeamLength = 5000; // 500 mm
    m_oSeamData[0].m_oTargetDifference = 0;
    m_oSeamData[0].m_oBlankThicknessLeft = 1100;
    m_oSeamData[0].m_oBlankThicknessRight = 2100;
    m_oSeamData[0].m_oMaxWeldSpeed = 300; // 30 m/min
    m_oSeamData[1].m_oProductNumber = 14;
    m_oSeamData[1].m_oSeamPresent = 1;
    m_oSeamData[1].m_oSeamLength = 5000; // 500 mm
    m_oSeamData[1].m_oTargetDifference = 0;
    m_oSeamData[1].m_oBlankThicknessLeft = 1200;
    m_oSeamData[1].m_oBlankThicknessRight = 2200;
    m_oSeamData[1].m_oMaxWeldSpeed = 300; // 30 m/min

    sleep(20); // wait for 1 minute

    while(true)
    {
        InsertDataInProductFile();
//        InsertDataInProductFile();
//        InsertDataInProductFile();
//        InsertDataInProductFile();
//        m_oSeamData[0].m_oTargetDifference = 0;
//        m_oSeamData[0].m_oBlankThicknessLeft++;
//        m_oSeamData[0].m_oBlankThicknessRight++;
//        m_oSeamData[1].m_oTargetDifference = 0;
//        m_oSeamData[1].m_oBlankThicknessLeft++;
//        m_oSeamData[1].m_oBlankThicknessRight++;

        sleep(30);
    }
#endif

    sleep(20); // wait for 1 minute
    m_oProductNo.store(14);
    m_oSeamData[0].m_oProductNumber = 14;
    m_oSeamData[0].m_oSeamPresent = 1;
    m_oSeamData[0].m_oSeamLength = 5000; // Segmente: 5000 -> correct
    m_oSeamData[0].m_oTargetDifference = 0;
    m_oSeamData[0].m_oBlankThicknessLeft = 1100;
    m_oSeamData[0].m_oBlankThicknessRight = 2100;
    m_oSeamData[0].m_oMaxWeldSpeed = 240; // 24 m/min

    m_oSeamData[1].m_oProductNumber = 14;
    m_oSeamData[1].m_oSeamPresent = 1;
    m_oSeamData[1].m_oSeamLength = 5000; // Segmente: 4000 -> laenger
    m_oSeamData[1].m_oTargetDifference = 0;
    m_oSeamData[1].m_oBlankThicknessLeft = 1200;
    m_oSeamData[1].m_oBlankThicknessRight = 2200;
    m_oSeamData[1].m_oMaxWeldSpeed = 300; // 30 m/min
    InsertDataInProductFile();

    sleep(10); // wait for 1 minute
    m_oSeamData[1].m_oProductNumber = 14;
    m_oSeamData[1].m_oSeamPresent = 1;
    m_oSeamData[1].m_oSeamLength = 3500; // Segmente: 4000 -> kuerzer
    InsertDataInProductFile();

    sleep(10); // wait for 1 minute
    m_oSeamData[1].m_oProductNumber = 14;
    m_oSeamData[1].m_oSeamPresent = 1;
    m_oSeamData[1].m_oSeamLength = 2500; // Segmente: 4000 -> letzes Segment 0
    InsertDataInProductFile();

//    sleep(10); // wait for 1 minute
//    m_oSeamData[1].m_oProductNumber = 14;
//    m_oSeamData[1].m_oSeamPresent = 1;
//    m_oSeamData[1].m_oSeamLength = 1900; // Segmente: 4000 -> letzen 2 Segmente 0
//    InsertDataInProductFile();

    while(true)
    {
        sleep(10);
    }

    return 0;
}

/***************************************************************************/
/* unpackGlobData                                                          */
/***************************************************************************/

void TCPServerSRING::unpackGlobData(char *recvBuffer)
{
    int16_t *pword;
    int32_t *pdword;
    uint32_t *pudword;
    int16_t oBlockType = 0;
    int blockLength1;
    int blockLength2;

    // Read DATABLOCK-Header
    pword = (int16_t *) recvBuffer;
    oBlockType = *pword;     // block type
    pword++;
    //xyz = *pword;          // block version
    pword++;
    blockLength1 = *pword;   // block length
    pword++;
    //xyz = *pword;          // reserve
    pword++;

    // Read DATABLOCK-Information
    // change datatype
    pdword = (int32_t *) pword;

    // SOUVIS Geschwindigkeit
    m_oMaxSouvisSpeed = *pdword;
    pdword++;

    // Reserven
    pdword++;
    pdword++;
    pdword++;

    // change datatype
    pudword = (uint32_t *) pdword;

    m_oSouvisPresent  = (bool)(*pudword & BIT0);
    m_oSouvisSelected  = (bool)(*pudword & BIT1);
    pudword++;

    // change datatype
    pdword = (int32_t *) pudword;

    // Bitfeld fuer diverse Anfragen
    m_oSendStatusDataToMC  = (bool)(*pdword & BIT0);
    pdword++;

    // Reserven
    pdword++;
    pdword++;
    pdword++;
    pdword++;
    pdword++;
    pdword++;
    pdword++;

    blockLength2 = (char *)pdword - (char *)recvBuffer;
    if (blockLength1 != blockLength2)
    {
        wmLogTr(eError, "QnxMsg.VI.TCPLengthNotIdent", "%s: Length of datablock not identical %s\n", "TCPServerSRING", "(globData)");
    }

    wmLog(eDebug, "unpackGlobData: blockType: %d\n", oBlockType);
    wmLog(eDebug, "maxSpeed:%d, pres:%d, sel:%d, sendStatus:%d\n",
          m_oMaxSouvisSpeed.load(), m_oSouvisPresent.load(), m_oSouvisSelected.load(), m_oSendStatusDataToMC.load());
}

/***************************************************************************/
/* unpackSeamData                                                          */
/***************************************************************************/

void TCPServerSRING::unpackSeamData(char *recvBuffer, int seamNo)
{
    int16_t *pword;
    int32_t *pdword;
    int16_t oBlockType = 0;
    int blockLength1;
    int blockLength2;

    // Read DATABLOCK-Header
    pword = (int16_t *) recvBuffer;
    oBlockType = *pword;         // block type
    pword++;
    //xyz = *pword;              // block version
    pword++;
    blockLength1 = *pword;       // block length
    pword++;
    //xyz = *pword;              // reserve
    pword++;

    // change datatype
    pdword = (int32_t *) pword;

    Poco::Mutex::ScopedLock lock( m_oSeamDataMutex );

    // Produktnummer
//    if (*pdword != ptcpShMem->productNumber)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"productNumber", -1, seamNo, -1,
//                ptcpShMem->productNumber, *pdword);
//    }
    m_oSeamData[seamNo].m_oProductNumber = *pdword;
    uint32_t oLocalProductNo = *pdword; // product number to local memory, see m_oSeamPresent !
    pdword++;

    // change datatype
    pword = (int16_t *) pdword;

    ////////////////////
    // general seam data
    ////////////////////

    // Naht aktiv
//    if (*pword != pseamData->present)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"present", -1, seamNo, -1,
//                pseamData->present, *pword);
//    }
//    pseamData->present = *pword;
//    if ((pseamData->present == 1)&&(seamNo > (int)(phostShMem->configData[IDX_MAX_SEAM_VISIBLE].parameter - 1)))
//    {
//        // Naht Nummer zu gross
//        sprintf(statusStrg,"tcpServerSSPEED: %s: %d , max: %d (001)", getTextPtr(HOST_ERR,70), (seamNo + 1),
//                (int)phostShMem->configData[IDX_MAX_SEAM_VISIBLE].parameter);
//        statusMsg(STATUS_ERR,statusStrg);
//        pseamData->present = 0;
//        userLogMsg(-1,(char *)"no username",(char *)"present corrected", -1, seamNo, -1,
//                pseamData->present, *pword);
//    }
    m_oSeamData[seamNo].m_oSeamPresent = *pword;
    if (m_oSeamData[seamNo].m_oSeamPresent == 1)
    {
        m_oProductNo = oLocalProductNo; // use local product number only if blank is present !
    }
    pword++;

    // change datatype
    pdword = (int32_t *) pword;

    // Reserve
    pdword++;

    // Schweisslaenge
//    if (*pdword != pseamData->weldLength)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"weldLength", -1, seamNo, -1,
//                pseamData->weldLength, *pdword);
//    }
//    pseamData->weldLength = *pdword;
    m_oSeamData[seamNo].m_oSeamLength = *pdword;
    pdword++;

    // change datatype
    pword = (int16_t *) pdword;

    // Reserven
    pword++;

    // Soll Differenz Dickensprung
//    if (*pword != pseamData->m_oTargetDifference)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"targetDifference", -1, seamNo, -1,
//                pseamData->m_oTargetDifference, *pword);
//    }
//    pseamData->m_oTargetDifference = *pword;
    m_oSeamData[seamNo].m_oTargetDifference = *pword;
    pword++;

    // Blechdicke links
//    if (*pword != pseamData->gaugeLeft)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"gaugeLeft", -1, seamNo, -1,
//                pseamData->gaugeLeft, *pword);
//    }
//    pseamData->gaugeLeft = *pword;
    m_oSeamData[seamNo].m_oBlankThicknessLeft = *pword;
    pword++;

    // Blechdicke rechts
//    if (*pword != pseamData->gaugeRight)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"gaugeRight", -1, seamNo, -1,
//                pseamData->gaugeRight, *pword);
//    }
//    pseamData->gaugeRight = *pword;
    m_oSeamData[seamNo].m_oBlankThicknessRight = *pword;
    pword++;

    // Setup Nummer
//    if (*pword != pseamData->setupNo)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"setupNo", -1, seamNo, -1,
//                pseamData->setupNo, *pword);
//    }
//    pseamData->setupNo = *pword;
    m_oSeamData[seamNo].m_oSetupNumber = *pword;
    pword++;

    // Schweissgeschwindigkeit
//    if (*pword != pseamData->weldSpeed)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"weldSpeed [0.1m/min]", -1, seamNo, -1,
//                pseamData->weldSpeed, *pword);
//    }
//    pseamData->weldSpeed = *pword;
    m_oSeamData[seamNo].m_oMaxWeldSpeed = *pword;
    pword++;

    ///////////////
    // SOUVIS1 data
    ///////////////

    // max Spaltbreite
//    if (*pword != pseamData->gapWidth)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"gapWidth", -1, seamNo, -1,
//                pseamData->gapWidth, *pword);
//    }
//    pseamData->gapWidth = *pword;
    m_oSeamData[seamNo].m_oMaxGapWidth = *pword;
    pword++;

    // min Kantenqualitaet
//    if (*pword != pseamData->edgeQuality)
//    {
//        userLogMsg(-1,(char *)"no username",(char *)"edgeQuality", -1, seamNo, -1,
//                pseamData->edgeQuality, *pword);
//    }
//    pseamData->edgeQuality = *pword;
    pword++;

    // change datatype
    pdword = (int32_t *) pword;

    // Reserven
    pdword++;
    pdword++;
    pdword++;
    pdword++;
    pdword++;
    pdword++;

    m_oSeamData[seamNo].m_oReserved16_1 = 0;
    m_oSeamData[seamNo].m_oReserved16_2 = 0;
    m_oSeamData[seamNo].m_oReserved16_3 = 0;
    m_oSeamData[seamNo].m_oReserved32_1 = 0;
    m_oSeamData[seamNo].m_oReserved32_2 = 0;
    m_oSeamData[seamNo].m_oReserved32_3 = 0;

    blockLength2 = (char *)pdword - (char *)recvBuffer;
    if (blockLength1 != blockLength2)
    {
        wmLogTr(eError, "QnxMsg.VI.TCPLengthNotIdent", "%s: Length of datablock not identical %s\n", "TCPServerSRING", "(seamData)");
    }

    wmLog(eDebug, "unpackSeamData: blockType: %d, seamNo: %d\n", oBlockType, seamNo);
    wmLog(eDebug, "ProdNr:%d, Pres:%d, Len:%d, Setup:%d\n", 
          m_oSeamData[seamNo].m_oProductNumber, m_oSeamData[seamNo].m_oSeamPresent, m_oSeamData[seamNo].m_oSeamLength, m_oSeamData[seamNo].m_oSetupNumber);
    wmLog(eDebug, "ThickL:%d, ThickR:%d, TDiff:%d, WSpeed:%d, MaxGap:%d\n", 
          m_oSeamData[seamNo].m_oBlankThicknessLeft, m_oSeamData[seamNo].m_oBlankThicknessRight, m_oSeamData[seamNo].m_oTargetDifference,
          m_oSeamData[seamNo].m_oMaxWeldSpeed, m_oSeamData[seamNo].m_oMaxGapWidth);
}

/***************************************************************************/
/* packAcknBlock                                                           */
/***************************************************************************/

void TCPServerSRING::packAcknBlock(char *sendBuffer, int *bufLength, int32_t p_oAcknCode, int32_t p_oBlockType, int32_t p_oBlocksReceived)
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
/* InsertDataInProductFile                                                 */
/***************************************************************************/

void TCPServerSRING::InsertDataInProductFile(void)
{
    try
    {
        std::string oHomeDir(std::getenv("WM_BASE_DIR"));
        std::string oProductsDirectory = oHomeDir + "/config/products";;
        wmLog(eDebug, "InsertDataInProductFile: oProductsDirectory: %s\n", oProductsDirectory.c_str());

        Poco::Mutex::ScopedLock lock( m_oSeamDataMutex );
        bool oProductFound = false;
        Poco::DirectoryIterator it(oProductsDirectory);
        Poco::DirectoryIterator end;
        Poco::Path oFilenameWithPath;
        while (it != end)
        {
            std::string oFilename = it.name();
            if (oFilename.find("temp") == std::string::npos) // check only "regular" files, not temporary files !
            {
                oFilenameWithPath = it.path();

                std::string oProductName;
                int retVal = CheckForProductNumber(oFilenameWithPath, m_oProductNo.load(), oProductName);
                if(retVal == 1) // found the appropriate product file
                {
                    oProductFound = true;
                    wmLogTr(eInfo, "QnxMsg.VI.TCPRcvProdUpdate", "Product '%s' has been received for updating\n", oProductName.c_str());;
                    break;
                }
            }
            ++it;
        }
        if (!oProductFound)
        {
            wmLogTr(eWarning, "QnxMsg.VI.DataInsProdMiss", "Product file for inserting data not found, ProductNumber: %d\n", m_oProductNo.load());
            return;
        }
        // oFilenameWithPath: name of product file including path
        ModifyProductFile(oFilenameWithPath);
        AdjustSeamLength(oFilenameWithPath);
    }
    catch(...)
    {
        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(001)");
    }
}

int TCPServerSRING::CheckForProductNumber(const Poco::Path &p_oFilenameWithPath, const int p_oSearchType, std::string &p_oProductName)
{
    std::ifstream oJSONInputFile(p_oFilenameWithPath.toString());
    nlohmann::json oProductFileObject;
    try
    {
        oProductFileObject = nlohmann::json::parse(oJSONInputFile);
    }
    catch (...)
    {
        // file is not a json file, ignore it
        return -1; // falsches Fileformat
    }

    int oType;
    try
    {
        oType = oProductFileObject.at("type");
    }
    catch (...)
    {
        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(002)");
        return -1; // falsches Fileformat
    }
    std::string oName;
    try
    {
        oName = oProductFileObject.at("name");
        p_oProductName.assign(oName);
    }
    catch (...)
    {
        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(003)");
        return -1; // falsches Fileformat
    }

    if (oType == p_oSearchType)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int TCPServerSRING::ModifyProductFile(const Poco::Path &p_oFilenameWithPath)
{
    uint8_t oBlankPresentArray[MAX_S6K_SEAM]{};
    int oSeamsToProcess{MAX_S6K_SEAM_SRING};
    if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
    {
        oSeamsToProcess = MAX_S6K_SEAM_SSPEED;
    }
    for (int i = 0; i < oSeamsToProcess; i++)
    {
        if (m_oSeamData[i].m_oSeamPresent == 1)
        {
            oBlankPresentArray[i] = 1;
        }
    }

    //*************************************************************************

    std::ifstream oJSONInputFile(p_oFilenameWithPath.toString());
    nlohmann::json oProductFileObject = nlohmann::json::parse(oJSONInputFile);

    //*************************************************************************

    // separate the seamSeries object
    try
    {
        // test if object seamSeries is present
        nlohmann::json &oSeamSeriesObject = oProductFileObject.at("seamSeries");
        (void)oSeamSeriesObject;
    }
    catch (...)
    {
        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(004)");
        // wrong fileformat
        return -1;
    }

    nlohmann::json &oSeamSeriesObject = oProductFileObject.at("seamSeries");

    // seamSeries array
    for (auto itSeamSeriesArray = oSeamSeriesObject.begin(); itSeamSeriesArray != oSeamSeriesObject.end(); ++itSeamSeriesArray) // iterate through all seamSeries
    {
        // separate this seamSeries
        nlohmann::json &oSeamSeries = itSeamSeriesArray.value();

        // seamSeries items
        for (auto itSeamSeriesItems = oSeamSeries.begin(); itSeamSeriesItems != oSeamSeries.end(); ++itSeamSeriesItems) // iterate through all items in this seamSeries
        {
            // determine the key of the seamSeries item
            const std::string &oSeamSeriesKey = itSeamSeriesItems.key();

            // this item is the seams object
            if (oSeamSeriesKey == "seams")
            {
                // separate the seams object
                nlohmann::json &oSeamsObject = itSeamSeriesItems.value();

                // seams array
                for (auto itSeamsArray = oSeamsObject.begin(); itSeamsArray != oSeamsObject.end(); ++itSeamsArray) // iterate through all seams
                {
                    // separate this seam
                    nlohmann::json &oSeam = itSeamsArray.value();

                    int oSeamNumber = -1;
                    try
                    {
                        oSeamNumber = oSeam.at("number");
                    }
                    catch (...)
                    {
                        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(005)");
                        // wrong fileformat
                        return -1;
                    }

                    if (m_oSeamData[oSeamNumber].m_oSeamPresent == 1)
                    {
                        try
                        {
                            oSeam.at("targetDifference") = m_oSeamData[oSeamNumber].m_oTargetDifference;
                        }
                        catch (...)
                        {
                            oSeam += nlohmann::json::object_t::value_type("targetDifference", m_oSeamData[oSeamNumber].m_oTargetDifference);
                        }
                        try
                        {
                            oSeam.at("thicknessLeft") = m_oSeamData[oSeamNumber].m_oBlankThicknessLeft;
                        }
                        catch (...)
                        {
                            oSeam += nlohmann::json::object_t::value_type("thicknessLeft", m_oSeamData[oSeamNumber].m_oBlankThicknessLeft);
                        }
                        try
                        {
                            oSeam.at("thicknessRight") = m_oSeamData[oSeamNumber].m_oBlankThicknessRight;
                        }
                        catch (...)
                        {
                            oSeam += nlohmann::json::object_t::value_type("thicknessRight", m_oSeamData[oSeamNumber].m_oBlankThicknessRight);
                        }
                        uint64_t oVelocity = static_cast<uint64_t>(static_cast<double>(m_oSeamData[oSeamNumber].m_oMaxWeldSpeed) * 1000000.0 / 600.0);
                        try
                        {
                            oSeam.at("velocity") = oVelocity;
                        }
                        catch (...)
                        {
                            oSeam += nlohmann::json::object_t::value_type("velocity", oVelocity);
                        }
                        oBlankPresentArray[oSeamNumber] += 2;
                    }

                    // seam items
                    for (auto itSeamItems = oSeam.begin(); itSeamItems != oSeam.end(); ++itSeamItems) // iterate through all items in this seam
                    {
                        // determine the key of the seam item
                        const std::string &oSeamKey = itSeamItems.key();

                        // this item is the seamIntervals object
                        if (oSeamKey == "seamIntervals")
                        {
                            // separate the seamIntervals object
                            nlohmann::json &oSeamIntervalsObject = itSeamItems.value();

                            // seamInterval array
                            for (auto itSeamIntervalArray = oSeamIntervalsObject.begin(); itSeamIntervalArray != oSeamIntervalsObject.end(); ++itSeamIntervalArray) // iterate through all seamIntervals
                            {
                                // separate this seamInterval
                                nlohmann::json &oseamInterval = itSeamIntervalArray.value();

                                if (m_oSeamData[oSeamNumber].m_oSeamPresent == 1)
                                {
                                    try
                                    {
                                        oseamInterval.at("targetDifference") = m_oSeamData[oSeamNumber].m_oTargetDifference;
                                    }
                                    catch (...)
                                    {
                                        oseamInterval += nlohmann::json::object_t::value_type("targetDifference", m_oSeamData[oSeamNumber].m_oTargetDifference);
                                    }
                                    try
                                    {
                                        oseamInterval.at("thicknessLeft") = m_oSeamData[oSeamNumber].m_oBlankThicknessLeft;
                                    }
                                    catch (...)
                                    {
                                        oseamInterval += nlohmann::json::object_t::value_type("thicknessLeft", m_oSeamData[oSeamNumber].m_oBlankThicknessLeft);
                                    }
                                    try
                                    {
                                        oseamInterval.at("thicknessRight") = m_oSeamData[oSeamNumber].m_oBlankThicknessRight;
                                    }
                                    catch (...)
                                    {
                                        oseamInterval += nlohmann::json::object_t::value_type("thicknessRight", m_oSeamData[oSeamNumber].m_oBlankThicknessRight);
                                    }
                                    uint64_t oVelocity = static_cast<uint64_t>(static_cast<double>(m_oSeamData[oSeamNumber].m_oMaxWeldSpeed) * 1000000.0 / 600.0);
                                    try
                                    {
                                        oseamInterval.at("velocity") = oVelocity;
                                    }
                                    catch (...)
                                    {
                                        oseamInterval += nlohmann::json::object_t::value_type("velocity", oVelocity);
                                    }
                                }
                            } // for seamIntervals array
                        } // key == "seamIntervals"
                    } // for seam items
                } // for seams array
            } // key == "seams"
        } // for seamSeries items
    } // for seamSeries array

    //*************************************************************************

    for (int i = 0; i < oSeamsToProcess; i++)
    {
        if (oBlankPresentArray[i] == 3) // seam is present in TCP/IP and present in product file
        {
        }
        else if (oBlankPresentArray[i] == 0) // seam is not present in product file and not present in TCP/IP
        {
        }
        else if (oBlankPresentArray[i] == 1) // seam is present in TCP/IP but not present in product file
        {
            wmLogTr(eError, "QnxMsg.VI.DataInsNoSeam", "Seam number %d is missing in product file of SOUVIS6000 !\n", (i + 1));
        }
        else if (oBlankPresentArray[i] == 2) // seam is not present in TCP/IP but present in TCP/IP
        {
        }
    }

    //*************************************************************************

    std::string oTempString(p_oFilenameWithPath.toString());
    oTempString += ".temp";
    Poco::Path oFilenameTemporary(oTempString);
    std::ofstream oJSONOutputFile(oFilenameTemporary.toString());
    oJSONOutputFile << std::setw(4) << oProductFileObject << std::endl;
    oJSONOutputFile.close();

    Poco::File oTempFile2(oFilenameTemporary);
    try
    {
        oTempFile2.renameTo(p_oFilenameWithPath.toString());
    }
    catch (Poco::Exception& e)
    {
        wmLog(eDebug, "Exception in renameTo: %s\n", e.what());
        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(006)");
    }
    catch (...)
    {
        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(007)");
    }

    return 1;
}

int TCPServerSRING::AdjustSeamLength(const Poco::Path &p_oFilenameWithPath)
{
    std::ifstream oJSONInputFile(p_oFilenameWithPath.toString());
    nlohmann::json oProductFileObject = nlohmann::json::parse(oJSONInputFile);

    //*************************************************************************

    // separate the seamSeries object
    try
    {
        // test if object seamSeries is present
        nlohmann::json &oSeamSeriesObject = oProductFileObject.at("seamSeries");
        (void)oSeamSeriesObject;
    }
    catch (...)
    {
        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(010)");
        // wrong fileformat
        return -1;
    }

    nlohmann::json &oSeamSeriesObject = oProductFileObject.at("seamSeries");

    // seamSeries array
    for (auto itSeamSeriesArray = oSeamSeriesObject.begin(); itSeamSeriesArray != oSeamSeriesObject.end(); ++itSeamSeriesArray) // iterate through all seamSeries
    {
        // separate this seamSeries
        nlohmann::json &oSeamSeries = itSeamSeriesArray.value();

        // seamSeries items
        for (auto itSeamSeriesItems = oSeamSeries.begin(); itSeamSeriesItems != oSeamSeries.end(); ++itSeamSeriesItems) // iterate through all items in this seamSeries
        {
            // determine the key of the seamSeries item
            const std::string &oSeamSeriesKey = itSeamSeriesItems.key();

            // this item is the seams object
            if (oSeamSeriesKey == "seams")
            {
                // separate the seams object
                nlohmann::json &oSeamsObject = itSeamSeriesItems.value();

                // seams array
                for (auto itSeamsArray = oSeamsObject.begin(); itSeamsArray != oSeamsObject.end(); ++itSeamsArray) // iterate through all seams
                {
                    // separate this seam
                    nlohmann::json &oSeam = itSeamsArray.value();

                    std::vector<long> oSeamIntervalLength;
                    int lastLevel0Segment{-1};

                    int oSeamNumber = -1;
                    try
                    {
                        oSeamNumber = oSeam.at("number");
                    }
                    catch (...)
                    {
                        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(011)");
                        // wrong fileformat
                        return -1;
                    }

                    // seam items
                    for (auto itSeamItems = oSeam.begin(); itSeamItems != oSeam.end(); ++itSeamItems) // iterate through all items in this seam
                    {
                        // determine the key of the seam item
                        const std::string &oSeamKey = itSeamItems.key();

                        // this item is the seamIntervals object
                        if (oSeamKey == "seamIntervals")
                        {
                            // separate the seamIntervals object
                            nlohmann::json &oSeamIntervalsObject = itSeamItems.value();

                            // seamInterval array
                            for (auto itSeamIntervalArray = oSeamIntervalsObject.begin(); itSeamIntervalArray != oSeamIntervalsObject.end(); ++itSeamIntervalArray) // iterate through all seamIntervals
                            {
                                // separate this seamInterval
                                nlohmann::json &oseamInterval = itSeamIntervalArray.value();

                                int segmentNumber{-1};
                                try
                                {
                                    segmentNumber = oseamInterval.at("number");
                                }
                                catch (...)
                                {
                                    wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(012)");
                                    return -1;
                                }

                                long oSeamIntervalLengthLong = -1;
                                try
                                {
                                    std::string oSeamIntervalLengthStr = oseamInterval.at("length");
                                    oSeamIntervalLengthLong = std::stol(oSeamIntervalLengthStr);
                                }
                                catch (...)
                                {
                                    wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(013)");
                                    return -1;
                                }
                                // save the segment length in the segment length vector
                                oSeamIntervalLength.emplace_back(oSeamIntervalLengthLong);

                                try
                                {
                                    int seamIntervalLevelValue = oseamInterval.at("level");
                                    if (seamIntervalLevelValue == 0)
                                    {
                                        if (segmentNumber != -1)
                                        {
                                            lastLevel0Segment = segmentNumber;
                                        }
                                    }
                                }
                                catch (...)
                                {
                                    wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(014)");
                                    return -1;
                                }


                            } // for seamIntervals array, all seamIntervals have been seen
                        } // key == "seamIntervals"
                    } // for seam items

                    // copy the actual segment lengths to vector with corrected segment lengths
                    std::vector<long> oSeamIntervalLengthCorr(oSeamIntervalLength);
                    // do modification only if seam present bit is set
                    if (m_oSeamData[oSeamNumber].m_oSeamPresent == 1)
                    {
                        // calculate the sum of the single segment lengths
                        long oSeamLength = std::accumulate(oSeamIntervalLength.begin(), oSeamIntervalLength.end(), 0);
                        // check if there are no seam intervals present (should be never true !)
                        if (oSeamIntervalLength.empty())
                        {
                            wmLogTr(eError, "QnxMsg.VI.TCPNoSeamInterval", "There is no seam interval in product file !\n");
                            wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(015)");
                            // there is no correction of segment lengths possible
                            return -1;
                        }

                        // compare sum of the segment lengths with the seam length received from machine control
                        if (oSeamLength != (m_oSeamData[oSeamNumber].m_oSeamLength * 100))
                        {
                            // received seam length is different, correction must be done
                            if (lastLevel0Segment == -1)
                            {
                                wmLogTr(eError, "QnxMsg.VI.TCPNoIntervalLev0", "There is no seam interval with level 1 in product file !");
                            }

                            long oDifference = (m_oSeamData[oSeamNumber].m_oSeamLength * 100) - oSeamLength;
                            if (oDifference >= 0)
                            {
                                // seam length over TCP/IP is larger -> simply extend the last segment with level 0
                                if (lastLevel0Segment != -1)
                                {
                                    oSeamIntervalLengthCorr.at(lastLevel0Segment) += oDifference;
                                }
                            }
                            else
                            {
                                // seam length over TCP/IP is shorter -> shorten the last segment with level 0
                                if (lastLevel0Segment != -1)
                                {
                                    if (std::abs(oDifference) < oSeamIntervalLengthCorr.at(lastLevel0Segment))
                                    {
                                        oSeamIntervalLengthCorr.at(lastLevel0Segment) += oDifference;
                                    }
                                    else
                                    {
                                        wmLogTr(eError, "QnxMsg.VI.TCPInterLev0Short", "Length of last seam interval with level 1 is to short for seam length !");
                                    }
                                }
                            }
                        }
                    }

                    // now insert the corrected seamInterval lengths into the actual seam
                    // seam items
                    for (auto itSeamItems = oSeam.begin(); itSeamItems != oSeam.end(); ++itSeamItems) // iterate through all items in this seam
                    {
                        // determine the key of the seam item
                        const std::string &oSeamKey = itSeamItems.key();

                        // this item is the seamIntervals object
                        if (oSeamKey == "seamIntervals")
                        {
                            // separate the seamIntervals object
                            nlohmann::json &oSeamIntervalsObject = itSeamItems.value();

                            int oIndex = 0;
                            // seamInterval array
                            for (auto itSeamIntervalArray = oSeamIntervalsObject.begin(); itSeamIntervalArray != oSeamIntervalsObject.end(); ++itSeamIntervalArray) // iterate through all seamIntervals
                            {
                                // separate this seamInterval
                                nlohmann::json &oseamInterval = itSeamIntervalArray.value();

                                try
                                {
                                    std::string oSeamIntervalLengthStr = std::to_string(oSeamIntervalLengthCorr[oIndex]);
                                    oseamInterval.at("length") = oSeamIntervalLengthStr;
                                    oIndex++;
                                }
                                catch (...)
                                {
                                    wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(016)");
                                    // wrong fileformat
                                    return -1;
                                }
                            } // for seamIntervals array, all seamIntervals have been seen
                        } // key == "seamIntervals"
                    } // for seam items
                } // for seams array
            } // key == "seams"
        } // for seamSeries items
    } // for seamSeries array

    //*************************************************************************

    // now save the modified json object aka product file
    std::string oTempString(p_oFilenameWithPath.toString());
    oTempString += ".temp";
    Poco::Path oFilenameTemporary(oTempString);
    std::ofstream oJSONOutputFile(oFilenameTemporary.toString());
    oJSONOutputFile << std::setw(4) << oProductFileObject << std::endl;
    oJSONOutputFile.close();

    Poco::File oTempFile2(oFilenameTemporary);
    try
    {
        oTempFile2.renameTo(p_oFilenameWithPath.toString());
    }
    catch (Poco::Exception& e)
    {
        wmLog(eDebug, "Exception in renameTo: %s\n", e.what());
        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(017)");
    }
    catch (...)
    {
        wmLogTr(eError, "QnxMsg.VI.DataInsFailure", "Problem while inserting data into product file %s\n", "(018)");
    }

    return 1;
}

/***************************************************************************/
/* TCPServerThread                                                         */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void *TCPServerThread(void *p_pArg)
{
    auto pDataToTCPServerThread = static_cast<struct DataToTCPServerThread *>(p_pArg);
    auto pTCPServerSRING = pDataToTCPServerThread->m_pTCPServerSRING;

    wmLog(eDebug, "TCPServerThread is started\n");

    usleep(1000 * 1000); // 1 second delay before starting

    wmLog(eDebug, "TCPServerThread is active\n");

    usleep(100*1000);

    if (pTCPServerSRING != nullptr)
    {
#if 1
        pTCPServerSRING->run();
#else
        pTCPServerSRING->TestDataInsert();
#endif
    }

    return NULL;
}

} // namespace tcpcommunication

} // namespace precitec

