#include "TCPCommunication/InspectionOutServer.h"

namespace precitec
{

namespace tcpcommunication
{

InspectionOutServer::InspectionOutServer(TCPCommunication& p_rTCPCommunication)
    :m_rTCPCommunication(p_rTCPCommunication)
{
}

InspectionOutServer::~InspectionOutServer()
{
}

} // namespace tcpcommunication

} // namespace precitec;

