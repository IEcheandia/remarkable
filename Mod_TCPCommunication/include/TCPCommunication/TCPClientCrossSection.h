/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2020
 *  @brief      Handles the TCP/IP communication with the other PostInspection system
 */

#pragma once

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#include <atomic>

#include "event/inspectionOut.h"

#include "TCPCommunication/TCPDefinesSRING.h"

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

///////////////////////////////////////////////////////////
// typedefs
///////////////////////////////////////////////////////////

class TCPClientCrossSection; // forward declaration

struct DataToTCPClientCrossSectionThread
{
    TCPClientCrossSection* m_pTCPClientCrossSection;
};

///////////////////////////////////////////////////////////
// class TCPClientCrossSection
///////////////////////////////////////////////////////////

class TCPClientCrossSection
{

public:
    TCPClientCrossSection(void);
    virtual ~TCPClientCrossSection(void);

    int run(void);

    void SetSendRequestCrossSectionData(void) { m_oSendRequestCrossSectionData.store(true); };
    void SetCycleDataBatchID(uint32_t p_oBatchID) { m_oCycleDataBatchID.store(p_oBatchID); };
    void SetCycleDataProductNo(uint32_t p_oProductNo) { m_oCycleDataProductNo.store(p_oProductNo); };

    void setS6K_CS_DataBlock (CS_TCP_MODE p_oSendCmd, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,  // interface InspectionOut
                              uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock);

private:
    bool isSOUVIS6000_TCPIP_Communication_On(void) { return m_oSOUVIS6000_TCPIP_Communication_On; }

    void StartTCPClientCrossSectionThread(void);

    /****************************************************************************/

    void checkSendRequest (void);
    int openCrossSectionConnection(void);
    int closeCrossSectionConnection(void);
    int sendCrossSectionData(void);
    void packCrossSectionData(char *sendBuffer, int *bufLength);
    void unpackAcknBlock(char *recvBuffer, int32_t& p_oAcknCode, int32_t& p_oBlockType, int32_t& p_oBlocksReceived);

    /****************************************************************************/

    pthread_t m_oTCPClientCrossSectionThread_ID;
    struct DataToTCPClientCrossSectionThread m_oDataToTCPClientCrossSectionThread;

    bool m_oSOUVIS6000_TCPIP_Communication_On;
    std::string m_oSOUVIS6000_IP_Address_CrossSection_Other;
    uint32_t m_oIPAddressOfCrossSectionOther; // in network byte order

    int m_oSockDesc;
    bool m_oTCPConnectionIsOn;

    /****************************************************************************/

    std::atomic<bool> m_oOpenRequestCrossSectionConnection;
    std::atomic<bool> m_oSendRequestCrossSectionData;
    std::atomic<bool> m_oCloseRequestCrossSectionConnection;
    std::atomic<uint32_t> m_oCycleDataBatchID;
    std::atomic<uint32_t> m_oCycleDataProductNo;

    uint16_t m_oSeamNo;
    uint16_t m_oBlockNo;
    uint16_t m_oFirstMeasureInBlock;
    uint16_t m_oMeasureCntInBlock;
    uint16_t m_oMeasuresPerResult;
    uint16_t m_oValuesPerMeasure;
    CS_BlockType m_oCS_DataBlock1;
};

} // namespace tcpcommunication

} // namespace precitec


