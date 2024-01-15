/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Handles the TCP/IP communication with SOURING machine
 */

#ifndef TCPCLIENTSRING_H_
#define TCPCLIENTSRING_H_

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#include <atomic>

#include "common/systemConfiguration.h"

#include "event/dbNotification.interface.h"
#include "message/db.proxy.h"
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

class TCPClientSRING; // forward declaration

struct DataToTCPClientThread
{
    TCPClientSRING* m_pTCPClientSRING;
};

struct segmentDefinition
{
    int16_t m_oStart;
    int16_t m_oGrade;
};
typedef segmentDefinition *psegmentDefinition;

struct qualityDefinition
{
    int16_t m_oMaxConcavity;
    bool m_oMaxConcavityWasSet;
    int16_t m_oMaxConvexity;
    bool m_oMaxConvexityWasSet;
    int16_t m_oMaxPosMisalign;
    bool m_oMaxPosMisalignWasSet;
    int16_t m_oMaxNegMisalign;
    bool m_oMaxNegMisalignWasSet;
    int16_t m_oMinSeamWidth;
    bool m_oMinSeamWidthWasSet;
    int16_t m_oMaxPoreWidth;
    bool m_oMaxPoreWidthWasSet;
    int16_t m_oMaxGreyLevel;
    bool m_oMaxGreyLevelWasSet;
    int16_t m_oMinAblationWidth;
    bool m_oMinAblationWidthWasSet;
    int16_t m_oMaxAblationBrightness;
    bool m_oMaxAblationBrightnessWasSet;
    int16_t m_oMinMillingPos;
    bool m_oMinMillingPosWasSet;
};
typedef qualityDefinition *pqualityDefinition;

struct seamDefinition
{
    segmentDefinition m_oSegment[MAX_S6K_SEGMENTS];
    qualityDefinition m_oQualityGrade[MAX_S6K_QUALITY_GRADES];
};
typedef seamDefinition *pseamDefinition;

///////////////////////////////////////////////////////////
// class TCPClientSRING
///////////////////////////////////////////////////////////

class TCPClientSRING
{

public:
    TCPClientSRING(TDb<MsgProxy>& p_rDbProxy);
    virtual ~TCPClientSRING(void);

    int run(void);

    void SetSendRequestQualityData(void) { m_oSendRequestQualityData = true; };
    void SetSendRequestInspectData(void) { m_oSendRequestInspectData = true; };
    void SetSendRequestStatusData(void) { m_oSendRequestStatusData = true; };
    void SetSystemReadyStatus(bool p_oStatus) { m_oSystemReadyStatus = p_oStatus; };
    void SetCycleDataBatchID(uint32_t p_oBatchID) { m_oCycleDataBatchID.store(p_oBatchID); };
    void SetCycleDataProductNo(uint32_t p_oProductNo) { m_oCycleDataProductNo.store(p_oProductNo); };
    void SetSouvisParamReq(bool p_oState) { m_oSouvisParamReq = p_oState; };

    /**
    * @brief Inform the state machine, that the user has altered the database. The state machine might not apply the changes immediately, though. If the system is inspecting a part, the state machine will wait until the end of the cycle.
    * @param p_oDbChange the db-change object that identifies the parts that have been changed.
    */
    void dbAltered( DbChange p_oDbChange );

    void SetSeamData(int p_oSeamNo, struct SeamDataStruct& p_oSeamData);

    void setS6K_QualityResults(int32_t p_oSeamNo, struct S6K_QualityData_S1S2 p_oQualityData);

private:
    /****************************************************************************/

    bool isSOUVIS6000_Application(void) { return m_oIsSOUVIS6000_Application; }
    SOUVIS6000MachineType getSOUVIS6000MachineType(void) { return m_oSOUVIS6000_Machine_Type; }
    bool isSOUVIS6000_Is_PreInspection(void) { return m_oSOUVIS6000_Is_PreInspection; }
    bool isSOUVIS6000_Is_PostInspection_Top(void) { return m_oSOUVIS6000_Is_PostInspection_Top; }
    bool isSOUVIS6000_Is_PostInspection_Bottom(void) { return m_oSOUVIS6000_Is_PostInspection_Bottom; }
    bool isSOUVIS6000_TCPIP_Communication_On(void) { return m_oSOUVIS6000_TCPIP_Communication_On; }

    void StartTCPClientThread(void);

    /****************************************************************************/

    void checkSendRequest (void);
    int sendQualResult(int sockDesc, int p_oSeamNo);
    int sendInspectData(int sockDesc, int p_oSeamNo);
    int sendStatusData(int sockDesc);
    void packQualResult(char *sendBuffer, int *bufLength, int p_oSeamNo);
    void packInspectData(char *sendBuffer, int *bufLength, int p_oSeamNo);
    void packStatusData(char *sendBuffer, int *bufLength);
    void unpackAcknBlock(char *recvBuffer, int32_t& p_oAcknCode, int32_t& p_oBlockType, int32_t& p_oBlocksReceived);

    /****************************************************************************/

    /**
     * @brief We have to check if the DB has changed, and for example inform the analyzer that a product definition was updated. This function has to be called by the state machine at state transitions when it is save to update the internal data structures.
     */
    void dbCheck();

    void clearSegmentTable(void);
    void fillSegmentTable(const Poco::UUID& p_oStationUUID, const Poco::UUID& p_oProductID);
    void clearQualityGrades(void);
    void fillQualityGrades(const Poco::UUID& p_oProductID);
    void checkForCompleteness(void);
    void createSumErrors(ParameterList &p_rParamList) const;

    /****************************************************************************/

    pthread_t m_oTCPClientThread_ID;
    struct DataToTCPClientThread m_oDataToTCPClientThread;

    bool m_oIsSOUVIS6000_Application;
    SOUVIS6000MachineType m_oSOUVIS6000_Machine_Type;
    bool m_oSOUVIS6000_Is_PreInspection;
    bool m_oSOUVIS6000_Is_PostInspection_Top;
    bool m_oSOUVIS6000_Is_PostInspection_Bottom;
    bool m_oSOUVIS6000_TCPIP_Communication_On;
    std::string m_oSOUVIS6000_IP_Address_MachineControl;
    uint32_t m_oIPAddressOfMachine; // in network byte order

    /****************************************************************************/

    std::atomic<bool> m_oSendRequestQualityData;
    std::atomic<bool> m_oSendRequestInspectData;
    std::atomic<bool> m_oSendRequestStatusData;
    std::atomic<bool> m_oSystemReadyStatus;
    std::atomic<uint32_t> m_oCycleDataBatchID;
    std::atomic<uint32_t> m_oCycleDataProductNo;
    std::atomic<bool> m_oSouvisParamReq;

    std::vector<DbChange> m_oDbChanges;  ///< We have to see if the db was altered and have to inform other components whenever possible.
    Poco::Mutex m_oDbChangesMutex;  ///< Protect the db changes array ...
    TDb<MsgProxy>& m_rDbProxy;

    Poco::Mutex m_oSeamDataMutex;  ///< Protect the seamData array
    SeamDataStruct m_oSeamData[MAX_S6K_SEAM];

    Poco::Mutex m_oQualResultMutex;  ///< Protect the qualResult array
    struct S6K_QualityData_S1S2 m_oQualResult[MAX_S6K_SEAM];

    seamDefinition m_oInspData[MAX_S6K_SEAM];
    int m_oLastPresentSeam;
};

} // namespace tcpcommunication

} // namespace precitec

#endif /* TCPCLIENTSRING_H_ */

