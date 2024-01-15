#include "viService/EthercatInputsServer.h"

namespace precitec
{

namespace viService
{

#define DEBUG_CALLBACK_FUNCTION        0

EthercatInputsServer::EthercatInputsServer(ServiceToGuiServer& p_rServiceToGuiServer, ServiceFromGuiServer& p_rServiceFromGuiServer):
	 m_rServiceToGuiServer(p_rServiceToGuiServer),
	 m_rServiceFromGuiServer(p_rServiceFromGuiServer)
{
    m_rServiceToGuiServer.connectCallback([&](SlaveInfo i) { return NewSlaveInfo(i); });
}

EthercatInputsServer::~EthercatInputsServer()
{
}

void EthercatInputsServer::NewSlaveInfo(SlaveInfo p_oSlaveInfo) // Callback from serviceToGuiServer
{
#if DEBUG_CALLBACK_FUNCTION
    printf("-----> EthercatInputsServer::NewSlaveInfo: oSlaveInfoComplete.GetSize(%d)\n", p_oSlaveInfo.GetSize());
    for(int i = 0;i < p_oSlaveInfo.GetSize();i++)
    {
        EC_T_GET_SLAVE_INFO oSlaveInfo = p_oSlaveInfo.GetInfoAt(i);
        printf("-----> EthercatInputsServer::NewSlaveInfo: abyDeviceName: (%s)\n", oSlaveInfo.abyDeviceName);
        printf("-----> EthercatInputsServer::NewSlaveInfo: wCfgPhyAddress:(%d)\n", oSlaveInfo.wCfgPhyAddress);
        printf("-----> EthercatInputsServer::NewSlaveInfo: dwVendorId:    (0x%04X)\n", oSlaveInfo.dwVendorId);
        printf("-----> EthercatInputsServer::NewSlaveInfo: dwProductCode: (0x%04X)\n", oSlaveInfo.dwProductCode);
        printf("-----> EthercatInputsServer::NewSlaveInfo: dwPdOffsIn:    (%d)\n", oSlaveInfo.dwPdOffsIn);
        printf("-----> EthercatInputsServer::NewSlaveInfo: dwPdSizeIn:    (%d)\n", oSlaveInfo.dwPdSizeIn);
        printf("-----> EthercatInputsServer::NewSlaveInfo: dwPdOffsOut:   (%d)\n", oSlaveInfo.dwPdOffsOut);
        printf("-----> EthercatInputsServer::NewSlaveInfo: dwPdSizeOut:   (%d)\n", oSlaveInfo.dwPdSizeOut);
    }
#endif

    m_rServiceFromGuiServer.Init(p_oSlaveInfo);
}

}	// namespace viService

} 	// namespace precitec;

