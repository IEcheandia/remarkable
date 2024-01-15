#include "viWeldHead/TriggerCmdServer.h"
namespace precitec
{
namespace ethercat
{


TriggerCmdServer::TriggerCmdServer(WeldingHeadControl& rWeldingHeadControl)
	:m_rWeldingHeadControl(rWeldingHeadControl)
{

}

TriggerCmdServer::~TriggerCmdServer()
{
}

}	// namespace ethercat
} 	// namespace precitec;

