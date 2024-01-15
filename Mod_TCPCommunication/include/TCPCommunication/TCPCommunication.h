/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Handles the TCP/IP communication with Soutec machine
 */

#ifndef TCPCOMMUNICATION_H_
#define TCPCOMMUNICATION_H_

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#include "event/S6K_InfoToProcesses.handler.h"
#include "event/S6K_InfoFromProcesses.proxy.h"

#include "message/db.proxy.h"

#include "event/inspectionOut.h"

#include "common/systemConfiguration.h"

#include "TCPCommunication/TCPDefinesSRING.h"
#include "TCPCommunication/TCPClientSRING.h"
#include "TCPCommunication/TCPServerSRING.h"
#include "TCPCommunication/TCPClientCrossSection.h"
#include "TCPCommunication/TCPServerCrossSection.h"

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

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

class TCPCommunication
{

public:
    TCPCommunication(TS6K_InfoFromProcesses<EventProxy>& p_rS6K_InfoFromProcessesProxy, TDb<MsgProxy>& p_rDbProxy);
    virtual ~TCPCommunication(void);

    bool isSOUVIS6000_Application(void) { return m_oIsSOUVIS6000_Application; }
    SOUVIS6000MachineType getSOUVIS6000MachineType(void) { return m_oSOUVIS6000_Machine_Type; }
    bool isSOUVIS6000_Is_PreInspection(void) { return m_oSOUVIS6000_Is_PreInspection; }
    bool isSOUVIS6000_Is_PostInspection_Top(void) { return m_oSOUVIS6000_Is_PostInspection_Top; }
    bool isSOUVIS6000_Is_PostInspection_Bottom(void) { return m_oSOUVIS6000_Is_PostInspection_Bottom; }
    bool isSOUVIS6000_CrossSectionMeasurementEnable(void) { return m_oSOUVIS6000_CrossSectionMeasurementEnable; }
    bool isSOUVIS6000_CrossSection_Leading_System(void) { return m_oSOUVIS6000_CrossSection_Leading_System; }

    void GlobDataReceived(uint32_t p_oBlockType); // Callback from TCPServer
    void SeamDataReceived(uint32_t p_oBlockType); // Callback from TCPServer

    void systemReadyInfo(bool onoff); // Interface S6K_InfoToProcesses
    void sendRequestQualityData(void); // Interface S6K_InfoToProcesses
    void sendRequestInspectData(void); // Interface S6K_InfoToProcesses
    void sendRequestStatusData(void); // Interface S6K_InfoToProcesses
    void sendS6KCmdToProcesses(S6K_CmdToProcesses p_oCmd, uint64_t p_oValue); // Interface S6K_InfoToProcesses

    void setupProduct(const Poco::UUID& p_rProductID); // Interface dbNotification

    void setS6K_QualityResults(int32_t p_oSeamNo, struct S6K_QualityData_S1S2 p_oQualityData); // interface InspectionOut
    void setS6K_CS_DataBlock (CS_TCP_MODE p_oSendCmd, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,  // interface InspectionOut
                              uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock);

private:
    /****************************************************************************/

    TS6K_InfoFromProcesses<EventProxy>& m_rS6K_InfoFromProcessesProxy;
    TDb<MsgProxy>& m_rDbProxy;

    /****************************************************************************/

    bool m_oIsSOUVIS6000_Application;
    SOUVIS6000MachineType m_oSOUVIS6000_Machine_Type;
    bool m_oSOUVIS6000_Is_PreInspection;
    bool m_oSOUVIS6000_Is_PostInspection_Top;
    bool m_oSOUVIS6000_Is_PostInspection_Bottom;
    bool m_oSOUVIS6000_CrossSectionMeasurementEnable;
    bool m_oSOUVIS6000_CrossSection_Leading_System;

    TCPClientSRING *m_pTCPClientSRING;
    TCPServerSRING *m_pTCPServerSRING;

    TCPClientCrossSection *m_pTCPClientCrossSection;
    TCPServerCrossSection *m_pTCPServerCrossSection;

    /****************************************************************************/

    bool m_oSystemReadyStatusShadow;

    /****************************************************************************/

};

} // namespace tcpcommunication

} // namespace precitec

#endif /* TCPCOMMUNICATION_H_ */

