#include "viInspectionControl/EthercatInputsServer.h"

namespace precitec
{

namespace ethercat
{

EthercatInputsServer::EthercatInputsServer(VI_InspectionControl& p_rInspectionControl)
	:m_rInspectionControl(p_rInspectionControl)
{
}

EthercatInputsServer::~EthercatInputsServer()
{
}

}	// namespace ethercat

} 	// namespace precitec;

