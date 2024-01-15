#include "viWeldHead/InspectionServer.h"

namespace precitec
{

namespace ethercat
{

InspectionServer::InspectionServer(WeldingHeadControl& p_rWeldingHeadControl)
	:m_rWeldingHeadControl(p_rWeldingHeadControl)
{
}

InspectionServer::~InspectionServer()
{
}

}	// namespace ethercat

} 	// namespace precitec;

