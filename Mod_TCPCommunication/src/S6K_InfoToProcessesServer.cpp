#include "TCPCommunication/S6K_InfoToProcessesServer.h"

namespace precitec
{

namespace tcpcommunication
{

S6K_InfoToProcessesServer::S6K_InfoToProcessesServer(TCPCommunication& p_rTCPCommunication)
    :m_rTCPCommunication(p_rTCPCommunication)
{
}

S6K_InfoToProcessesServer::~S6K_InfoToProcessesServer()
{
}

} // namespace tcpcommunication

} // namespace precitec;

