/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Handles the TCP/IP communication with Soutec machine
 */

#include <unistd.h>

#include "module/moduleLogger.h"

#include "event/dbNotification.interface.h"

#include "TCPCommunication/TCPCommunication.h"

#include "common/connectionConfiguration.h"

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

TCPCommunication::TCPCommunication(TS6K_InfoFromProcesses<EventProxy>& p_rS6K_InfoFromProcessesProxy, TDb<MsgProxy>& p_rDbProxy):
        m_rS6K_InfoFromProcessesProxy(p_rS6K_InfoFromProcessesProxy),
        m_rDbProxy(p_rDbProxy),
        m_pTCPClientSRING(nullptr),
        m_pTCPServerSRING(nullptr),
        m_pTCPClientCrossSection(nullptr),
        m_pTCPServerCrossSection(nullptr),
        m_oSystemReadyStatusShadow(true)
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
    m_oSOUVIS6000_CrossSectionMeasurementEnable = SystemConfiguration::instance().getBool("SOUVIS6000_CrossSectionMeasurementEnable", false);
    wmLog(eDebug, "m_oSOUVIS6000_CrossSectionMeasurementEnable (bool): %d\n", m_oSOUVIS6000_CrossSectionMeasurementEnable);
    m_oSOUVIS6000_CrossSection_Leading_System = SystemConfiguration::instance().getBool("SOUVIS6000_CrossSection_Leading_System", false);
    wmLog(eDebug, "m_oSOUVIS6000_CrossSection_Leading_System (bool): %d\n", m_oSOUVIS6000_CrossSection_Leading_System);

    // check correct configuration of SOUVIS6000 system
    if (isSOUVIS6000_Application())
    {
        int oNumberOfSystemsConfigured = 0;
        if (SystemConfiguration::instance().getBool("SOUVIS6000_Is_PreInspection", false) == true)
        {
            oNumberOfSystemsConfigured++;
        }
        if (SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Top", false) == true)
        {
            oNumberOfSystemsConfigured++;
        }
        if (SystemConfiguration::instance().getBool("SOUVIS6000_Is_PostInspection_Bottom", false) == true)
        {
            oNumberOfSystemsConfigured++;
        }
        if (oNumberOfSystemsConfigured == 0)
        {
            wmLogTr(eError, "QnxMsg.VI.TCPCommNoSubsys", "No subsystem of SOUVIS6000 system is configured !\n");
        }
        if (oNumberOfSystemsConfigured > 1)
        {
            wmLogTr(eError, "QnxMsg.VI.TCPCommToManySub", "To many subsystems of SOUVIS6000 system are configured !\n");
        }
    }

    if (isSOUVIS6000_Application())
    {
        usleep(100*1000);
        m_pTCPClientSRING = new TCPClientSRING(m_rDbProxy);
        if (m_pTCPClientSRING == nullptr)
        {
            wmLog(eDebug, "can't create m_pTCPClientSRING !\n");
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(001)");
        }

        usleep(100*1000);
        m_pTCPServerSRING = new TCPServerSRING();
        if (m_pTCPServerSRING == nullptr)
        {
            wmLog(eDebug, "can't create m_pTCPServerSRING !\n");
            wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(002)");
        }

        usleep(100*1000);
        if (m_pTCPServerSRING != nullptr)
        {
            m_pTCPServerSRING->connectCallback_1([&](uint32_t i) { return GlobDataReceived(i); });
            m_pTCPServerSRING->connectCallback_2([&](uint32_t i) { return SeamDataReceived(i); });
        }

        if (isSOUVIS6000_CrossSectionMeasurementEnable() && isSOUVIS6000_CrossSection_Leading_System())
        {
            usleep(100*1000);
            m_pTCPClientCrossSection = new TCPClientCrossSection();
            if (m_pTCPClientCrossSection == nullptr)
            {
                wmLog(eDebug, "can't create m_pTCPClientCrossSection !\n");
                wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(003)");
            }
        }

        if (isSOUVIS6000_CrossSectionMeasurementEnable() && !isSOUVIS6000_CrossSection_Leading_System())
        {
            usleep(100*1000);
            m_pTCPServerCrossSection = new TCPServerCrossSection(p_rS6K_InfoFromProcessesProxy);
            if (m_pTCPServerCrossSection == nullptr)
            {
                wmLog(eDebug, "can't create m_pTCPServerCrossSection !\n");
                wmFatal(eBusSystem, "QnxMsg.VI.InitTCPCommFault", "Problem while initializing TCP/IP communication %s\n", "(004)");
            }
        }
    }
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

TCPCommunication::~TCPCommunication(void)
{
    if (isSOUVIS6000_Application())
    {
        if (m_pTCPServerCrossSection != nullptr)
        {
            delete m_pTCPServerCrossSection;
        }
        usleep(100 * 1000);

        if (m_pTCPClientCrossSection != nullptr)
        {
            delete m_pTCPClientCrossSection;
        }
        usleep(100 * 1000);

        if (m_pTCPServerSRING != nullptr)
        {
            delete m_pTCPServerSRING;
        }
        usleep(100 * 1000);

        if (m_pTCPClientSRING != nullptr)
        {
            delete m_pTCPClientSRING;
        }
        usleep(100 * 1000);
    }
}

///////////////////////////////////////////////////////////
// private members
///////////////////////////////////////////////////////////

void TCPCommunication::GlobDataReceived(uint32_t p_oBlockType) // Callback from TCPServer
{
    if (m_pTCPServerSRING != nullptr)
    {
        uint32_t oMaxSouvisSpeed;
        bool oSouvisPresent;
        bool oSouvisSelected;
        bool oSendStatusData;
        m_pTCPServerSRING->GetGlobData(oMaxSouvisSpeed, oSouvisPresent, oSouvisSelected, oSendStatusData);

        m_rS6K_InfoFromProcessesProxy.maxSouvisSpeed(oMaxSouvisSpeed);
        m_rS6K_InfoFromProcessesProxy.souvisControlBits(oSouvisPresent, oSouvisSelected);

        if (oSendStatusData)
        {
            if (m_pTCPClientSRING != nullptr)
            {
                m_pTCPClientSRING->SetSendRequestStatusData();
            }
        }
    }
}

void TCPCommunication::SeamDataReceived(uint32_t p_oBlockType) // Callback from TCPServer
{
    if (m_pTCPServerSRING != nullptr)
    {
        uint32_t oNumberOfPresentSeams = 0;
        int oSeamsToProcess{MAX_S6K_SEAM_SRING};
        if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
        {
            oSeamsToProcess = MAX_S6K_SEAM_SSPEED;
        }
        for (int i = 0; i < oSeamsToProcess; i++)
        {
            struct SeamDataStruct oSeamData;
            m_pTCPServerSRING->GetSeamData(i, oSeamData);
            if (m_pTCPClientSRING != nullptr)
            {
                m_pTCPClientSRING->SetSeamData(i, oSeamData);
            }
            if (oSeamData.m_oSeamPresent == 1)
            {
                oNumberOfPresentSeams++;
            }
        }
        m_rS6K_InfoFromProcessesProxy.numberOfPresentSeams(oNumberOfPresentSeams);

        if ((getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouSpeed) || (getSOUVIS6000MachineType() == SOUVIS6000MachineType::eS6K_SouBlate))
        {
            uint32_t oProductNo{0};
            if (m_pTCPServerSRING != nullptr)
            {
                // ask TCP/IP server for the actual product no
                oProductNo = m_pTCPServerSRING->GetProductNo();
            }
            m_rS6K_InfoFromProcessesProxy.productNoFromTCP(oProductNo);
        }
    }
    if (m_pTCPClientSRING != nullptr)
    {
        m_pTCPClientSRING->SetSouvisParamReq(false);
        m_pTCPClientSRING->SetSendRequestInspectData();
    }
}

void TCPCommunication::systemReadyInfo(bool onoff) // Interface S6K_InfoToProcesses
{
    // send request only if the status changed
    if (onoff != m_oSystemReadyStatusShadow)
    {
        if (m_pTCPClientSRING != nullptr)
        {
            m_pTCPClientSRING->SetSystemReadyStatus(onoff);
            m_pTCPClientSRING->SetSendRequestStatusData();
        }
        m_oSystemReadyStatusShadow = onoff;
    }
}

void TCPCommunication::sendRequestQualityData(void) // Interface S6K_InfoToProcesses
{
    if (m_pTCPClientSRING != nullptr)
    {
        m_pTCPClientSRING->SetSendRequestQualityData();
    }
}

void TCPCommunication::sendRequestInspectData(void) // Interface S6K_InfoToProcesses
{
    if (m_pTCPClientSRING != nullptr)
    {
        m_pTCPClientSRING->SetSendRequestInspectData();
    }
}

void TCPCommunication::sendRequestStatusData(void) // Interface S6K_InfoToProcesses
{
    if (m_pTCPClientSRING != nullptr)
    {
        m_pTCPClientSRING->SetSendRequestStatusData();
    }
}

void TCPCommunication::sendS6KCmdToProcesses(S6K_CmdToProcesses p_oCmd, uint64_t p_oValue) // Interface S6K_InfoToProcesses
{
    if (p_oCmd == eS6K_CmdBatchID)
    {
        if (m_pTCPClientSRING != nullptr)
        {
            // pass on the batch id to TCP/IP client for use in qualResult block
            m_pTCPClientSRING->SetCycleDataBatchID(p_oValue);
        }
        if (m_pTCPClientCrossSection != nullptr)
        {
            // pass on the batch id to TCP/IP client for use in CrossSection block
            m_pTCPClientCrossSection->SetCycleDataBatchID(p_oValue);
        }
    }
    if (p_oCmd == eS6K_CmdProductNumber)
    {
        // check if product no from fieldbus is identical to the product no from TCP/IP
        if (m_pTCPServerSRING != nullptr)
        {
            // ask TCP/IP server for the actual product no
            uint32_t oProductNo = m_pTCPServerSRING->GetProductNo();
            if (oProductNo != p_oValue)
            {
                wmLogTr(eError, "QnxMsg.VI.ProdNoNotEqual", "ProductNumber via Fieldbus is not equal to ProductNumber via TCP/IP !\n");
            }
        }
        if (m_pTCPClientSRING != nullptr)
        {
            // pass on the product no to TCP/IP client for use in inspData block
            m_pTCPClientSRING->SetCycleDataProductNo(p_oValue);
        }
        if (m_pTCPClientCrossSection != nullptr)
        {
            // pass on the product no to TCP/IP client for use in CrossSection block
            m_pTCPClientCrossSection->SetCycleDataProductNo(p_oValue);
        }
    }
}

void TCPCommunication::setupProduct(const Poco::UUID& p_rProductID)
{
    wmLog(eDebug, "TCPCommunication::setupProduct: %s\n", p_rProductID.toString().c_str());
    DbChange oChange( eProduct );
    oChange.setProductID( p_rProductID );

    // mark the db as altered
    if (m_pTCPClientSRING != nullptr)
    {
        m_pTCPClientSRING->dbAltered( oChange );
    }
}

void TCPCommunication::setS6K_QualityResults(int32_t p_oSeamNo, struct S6K_QualityData_S1S2 p_oQualityData) // interface InspectionOut
{
    if (m_pTCPClientSRING != nullptr)
    {
        m_pTCPClientSRING->setS6K_QualityResults(p_oSeamNo, p_oQualityData);
    }
}

void TCPCommunication::setS6K_CS_DataBlock (CS_TCP_MODE p_oSendCmd, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,  // interface InspectionOut
                                            uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock)
{
    if (m_pTCPClientCrossSection != nullptr)
    {
        m_pTCPClientCrossSection->setS6K_CS_DataBlock(p_oSendCmd, p_oSeamNo, p_oBlockNo, p_oFirstMeasureInBlock, p_oMeasureCntInBlock, p_oMeasuresPerResult, p_oValuesPerMeasure, p_oCS_DataBlock);
    }
}

} // namespace tcpcommunication

} // namespace precitec

