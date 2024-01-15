#include "viWeldHead/InspectionCmdServer.h"

namespace precitec
{

namespace ethercat
{

InspectionCmdServer::InspectionCmdServer(WeldingHeadControl& p_rWeldingHeadControl)
	:m_rWeldingHeadControl(p_rWeldingHeadControl)
{
}

InspectionCmdServer::~InspectionCmdServer()
{
}

}	// namespace ethercat

} 	// namespace precitec;

