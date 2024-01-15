#include "CHRCommunication/InspectionServer.h"

namespace precitec
{

namespace grabber
{

InspectionServer::InspectionServer(CHRCommunication& p_rCHRCommunication)
    : m_rCHRCommunication(p_rCHRCommunication)
{
}

InspectionServer::~InspectionServer()
{
}

} // namespace grabber

} // namespace precitec;
