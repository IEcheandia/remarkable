#include "EtherCATMaster/EthercatOutputsServer.h"

namespace precitec
{

namespace ethercat
{

EthercatOutputsServer::EthercatOutputsServer(EtherCATMaster& p_rEtherCATMaster)
	:m_rEtherCATMaster(p_rEtherCATMaster)
{
}

EthercatOutputsServer::~EthercatOutputsServer()
{
}

}	// namespace ethercat

} 	// namespace precitec;

