/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Handles the TCP/IP communication with SOURING machine
 */

#ifndef TCPSERVERSRING_H_
#define TCPSERVERSRING_H_

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#include <stdint.h>
#include <atomic>
#include <functional>

#include "common/systemConfiguration.h"

#include "TCPCommunication/TCPDefinesSRING.h"

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

namespace precitec
{

namespace tcpcommunication
{

#define  BIT0  0x00000001
#define  BIT1  0x00000002
#define  BIT2  0x00000004
#define  BIT3  0x00000008
#define  BIT4  0x00000010
#define  BIT5  0x00000020
#define  BIT6  0x00000040
#define  BIT7  0x00000080
#define  BIT8  0x00000100
#define  BIT9  0x00000200
#define  BIT10 0x00000400
#define  BIT11 0x00000800
#define  BIT12 0x00001000
#define  BIT13 0x00002000
#define  BIT14 0x00004000
#define  BIT15 0x00008000
#define  BIT16 0x00010000
#define  BIT17 0x00020000
#define  BIT18 0x00040000
#define  BIT19 0x00080000
#define  BIT20 0x00100000
#define  BIT21 0x00200000
#define  BIT22 0x00400000
#define  BIT23 0x00800000
#define  BIT24 0x01000000
#define  BIT25 0x02000000
#define  BIT26 0x04000000
#define  BIT27 0x08000000
#define  BIT28 0x10000000
#define  BIT29 0x20000000
#define  BIT30 0x40000000
#define  BIT31 0x80000000

///////////////////////////////////////////////////////////
// typedefs
///////////////////////////////////////////////////////////

class TCPServerSRING; // forward declaration

struct DataToTCPServerThread
{
    TCPServerSRING* m_pTCPServerSRING;
};

typedef std::function<void(uint32_t)> TCPCallbackFunction_Type1;

class TCPServerSRING
{

public:
    TCPServerSRING(void);
    virtual ~TCPServerSRING(void);

    void connectCallback_1(TCPCallbackFunction_Type1 p_pTCPCallbackFunction_1);
    void connectCallback_2(TCPCallbackFunction_Type1 p_pTCPCallbackFunction_2);

    int run(void);
    int TestDataInsert(void);

    uint32_t GetProductNo(void) { return m_oProductNo; };
    void GetGlobData(uint32_t& p_oSpeed, bool& p_oPresent, bool& p_oSelected, bool& p_oSendStatus);
    void GetSeamData(int p_oSeamNo, struct SeamDataStruct& p_oSeamData);

private:
    void StartTCPServerThread(void);

    interface::SOUVIS6000MachineType getSOUVIS6000MachineType(void) { return m_oSOUVIS6000_Machine_Type; }

    /****************************************************************************/

    void unpackGlobData(char *recvBuffer);
    void unpackSeamData(char *recvBuffer, int seamNo);
    void packAcknBlock(char *sendBuffer, int *bufLength, int32_t p_oAcknCode, int32_t p_oBlockType, int32_t p_oBlocksReceived);

    void InsertDataInProductFile(void);
    int CheckForProductNumber(const Poco::Path &oFilenameWithPath, const int p_oSearchType, std::string &p_oProductName);
    int ModifyProductFile(const Poco::Path &p_oFilenameWithPath);
    int AdjustSeamLength(const Poco::Path &p_oFilenameWithPath);

    /****************************************************************************/

    TCPCallbackFunction_Type1 m_pTCPCallbackFunction_1;
    TCPCallbackFunction_Type1 m_pTCPCallbackFunction_2;

    /****************************************************************************/

    pthread_t m_oTCPServerThread_ID;
    struct DataToTCPServerThread m_oDataToTCPServerThread;

    bool m_oIsSOUVIS6000_Application;
    interface::SOUVIS6000MachineType m_oSOUVIS6000_Machine_Type;
    bool m_oSOUVIS6000_TCPIP_Communication_On;

    /****************************************************************************/

    std::atomic<uint32_t> m_oProductNo;

    std::atomic<uint32_t> m_oMaxSouvisSpeed;
    std::atomic<bool> m_oSouvisPresent;
    std::atomic<bool> m_oSouvisSelected;
    std::atomic<bool> m_oSendStatusDataToMC;

    Poco::Mutex m_oSeamDataMutex;  ///< Protect the db changes array ...
    SeamDataStruct m_oSeamData[MAX_S6K_SEAM];
};

} // namespace tcpcommunication

} // namespace precitec

#endif /* TCPSERVERSRING_H_ */



