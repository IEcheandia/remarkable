#include "../include/viService/serviceFromGuiServer.h"

namespace precitec {

namespace ethercat {

#define DEBUG_SERVICEFROMGUISERVER     0

ServiceFromGuiServer::ServiceFromGuiServer(ServiceToGuiServer &serviceToGuiServer, TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy):
        m_serviceToGuiServer(serviceToGuiServer),
        m_rEthercatOutputsProxy(p_rEthercatOutputsProxy)
{
}

ServiceFromGuiServer::~ServiceFromGuiServer()
{
}

void ServiceFromGuiServer::OutputProcessData(short physAddr, ProcessData& data, ProcessData& mask, short type)
{
    for (unsigned int i = 0; i < m_senderList.size(); ++i)
    {
        SENDER_INFORMATION theSenderInfo = m_senderList[i];
        EC_T_GET_SLAVE_INFO theInfo = theSenderInfo.info;

        if(theInfo.wCfgPhyAddress == physAddr && theSenderInfo.type == type)
        {
            //sender gefunden...
            if ((theInfo.dwProductCode == PRODUCTCODE_EL2008) && (theInfo.dwVendorId == VENDORID_BECKHOFF))
            {
                m_rEthercatOutputsProxy.ecatDigitalOut(theSenderInfo.m_oEcatProductIndex,
                                                       theSenderInfo.m_oEcatInstance,
                                                       (uint8_t) data.GetData()[0],
                                                       (uint8_t) mask.GetData()[0]);
            }
            else if ((theInfo.dwProductCode == PRODUCTCODE_EL4102) && (theInfo.dwVendorId == VENDORID_BECKHOFF))
            {
                uint16_t oChannelValue = 0;
                memcpy(&oChannelValue, data.GetData(), sizeof(uint16_t));
                m_rEthercatOutputsProxy.ecatAnalogOut(theSenderInfo.m_oEcatProductIndex,
                                                      theSenderInfo.m_oEcatInstance,
                                                      theSenderInfo.m_oEcatChannel,
                                                      oChannelValue);
            }
            else if ((theInfo.dwProductCode == PRODUCTCODE_EL4132) && (theInfo.dwVendorId == VENDORID_BECKHOFF))
            {
                uint16_t oChannelValue = 0;
                memcpy(&oChannelValue, data.GetData(), sizeof(uint16_t));
                m_rEthercatOutputsProxy.ecatAnalogOut(theSenderInfo.m_oEcatProductIndex,
                                                      theSenderInfo.m_oEcatInstance,
                                                      theSenderInfo.m_oEcatChannel,
                                                      oChannelValue);
            }
            else if((theInfo.dwProductCode == PRODUCTCODE_ANYBUS_GW) && (theInfo.dwVendorId == VENDORID_HMS))
            {
                int oBytesCount = data.GetSize();
                stdVecUINT8 oValues;
                stdVecUINT8 oMask;
                for(int i = 0;i < oBytesCount;i++)
                {
                    oValues.push_back(data.GetData()[i]);
                    oMask.push_back(mask.GetData()[i]);
                }
                m_rEthercatOutputsProxy.ecatGatewayOut(theSenderInfo.m_oEcatProductIndex,
                                                       theSenderInfo.m_oEcatInstance,
                                                       (uint8_t) oBytesCount,
                                                       oValues,
                                                       oMask);
            }
            else if((theInfo.dwProductCode == PRODUCTCODE_FIELDBUS) && (theInfo.dwVendorId == VENDORID_HILSCHER))
            {
                int oBytesCount = data.GetSize();
                stdVecUINT8 oValues;
                stdVecUINT8 oMask;
                for(int i = 0;i < oBytesCount;i++)
                {
                    oValues.push_back(data.GetData()[i]);
                    oMask.push_back(mask.GetData()[i]);
                }
                m_rEthercatOutputsProxy.ecatGatewayOut(theSenderInfo.m_oEcatProductIndex,
                                                       theSenderInfo.m_oEcatInstance,
                                                       (uint8_t) oBytesCount,
                                                       oValues,
                                                       oMask);
            }
        }
    }
}

void ServiceFromGuiServer::SetTransferMode(bool onOff)
{
    m_serviceToGuiServer.SetTransferModus(onOff);
    m_rEthercatOutputsProxy.sendAllData(onOff);
}

void ServiceFromGuiServer::requestSlaveInfo()
{
    m_serviceToGuiServer.sendSlaveInfo();
}

/**
 * Verbindungen zu den einzelnen Proxies aufbauen.
 * @param slaveInfo die Liste mit allen Slaves
 */
void ServiceFromGuiServer::Init(SlaveInfo slaveInfo)
{
    std::vector<EC_T_GET_SLAVE_INFO> slaveInfos = slaveInfo.GetSlaveInfoVector();
    m_senderList.clear();

    int nSlaveInfos = slaveInfos.size();
    for (int i = 0; i < nSlaveInfos; ++i)
    {
        EC_T_GET_SLAVE_INFO info = slaveInfos[i];

#if DEBUG_SERVICEFROMGUISERVER
        printf("-----> ServiceFromGuiServer::Init: abyDeviceName: (%s)\n", info.abyDeviceName);
        printf("-----> ServiceFromGuiServer::Init: wCfgPhyAddress:(%d)\n", info.wCfgPhyAddress);
        printf("-----> ServiceFromGuiServer::Init: dwVendorId:    (0x%04X)\n", info.dwVendorId);
        printf("-----> ServiceFromGuiServer::Init: dwProductCode: (0x%04X)\n", info.dwProductCode);
        printf("-----> ServiceFromGuiServer::Init: dwPdOffsIn:    (%d)\n", info.dwPdOffsIn);
        printf("-----> ServiceFromGuiServer::Init: dwPdSizeIn:    (%d)\n", info.dwPdSizeIn);
        printf("-----> ServiceFromGuiServer::Init: dwPdOffsOut:   (%d)\n", info.dwPdOffsOut);
        printf("-----> ServiceFromGuiServer::Init: dwPdSizeOut:   (%d)\n", info.dwPdSizeOut);
#endif

        // sender nur fuer Slaves mit OutputProzessdaten erstellen
        if(info.dwPdSizeOut > 0)
        {
            EC_T_DWORD productCode = info.dwProductCode;
            EC_T_DWORD vendorID = info.dwVendorId;
            int instanceID = 1;

            //Instanz-ID erstellen
            for (unsigned int j = 0; j < m_senderList.size(); ++j)
            {
                SENDER_INFORMATION theInfo = m_senderList[j];

                if(theInfo.info.dwProductCode == productCode && theInfo.info.dwVendorId == vendorID)
                {
                    //naechste Instanz gefunden...
                    if (theInfo.m_oEcatChannel == eChannel1)
                    {
                        instanceID++;
                    }
                }
            }

            //Sender erstellen
            if ((productCode == PRODUCTCODE_EL2008) && (vendorID == VENDORID_BECKHOFF))
            {
                AddSenderInfo(info, instanceID);
            } 
            else if ((productCode == PRODUCTCODE_EL4102) && (vendorID == VENDORID_BECKHOFF))
            {
                AddSenderInfo(info, instanceID, 1);
                AddSenderInfo(info, instanceID, 2);
            } 
            else if ((productCode == PRODUCTCODE_EL4132) && (vendorID == VENDORID_BECKHOFF))
            {
                AddSenderInfo(info, instanceID, 1);
                AddSenderInfo(info, instanceID, 2);
            } 
            else if((productCode == PRODUCTCODE_ANYBUS_GW) && (vendorID == VENDORID_HMS))
            {
                AddSenderInfo(info, instanceID);
            }
            else if((productCode == PRODUCTCODE_FIELDBUS) && (vendorID == VENDORID_HILSCHER))
            {
                AddSenderInfo(info, instanceID);
            }
        }
    }
}

void ServiceFromGuiServer::AddSenderInfo(EC_T_GET_SLAVE_INFO info, int instanceID, short type)
{
    //in Sender-Liste ablegen
    SENDER_INFORMATION senderInfo;
    senderInfo.info = info;
    senderInfo.instanceID = instanceID;
    senderInfo.type = type;
    senderInfo.m_oEcatInstance = (EcatInstance)instanceID;
    if (info.dwProductCode == PRODUCTCODE_EL2008)
    {
        senderInfo.m_oEcatProductIndex = eProductIndex_EL2008;
    }
    else if (info.dwProductCode == PRODUCTCODE_EL4102)
    {
        senderInfo.m_oEcatProductIndex = eProductIndex_EL4102;
    }
    else if (info.dwProductCode == PRODUCTCODE_EL4132)
    {
        senderInfo.m_oEcatProductIndex = eProductIndex_EL4132;
    }
    else if (info.dwProductCode == PRODUCTCODE_ANYBUS_GW)
    {
        senderInfo.m_oEcatProductIndex = eProductIndex_Anybus_GW;
    }
    else if (info.dwProductCode == PRODUCTCODE_FIELDBUS)
    {
        senderInfo.m_oEcatProductIndex = eProductIndex_Fieldbus;
    }
    else
    {
        // falscher ProductCode
        senderInfo.m_oEcatProductIndex = eProductIndex_EL2008;
    }
    senderInfo.m_oEcatChannel = (EcatChannel)type;

    m_senderList.push_back(senderInfo);
}

} // namespace ethercat

} // namespace precitec

