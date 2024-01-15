#include "CHRCommunication/TriggerCmdServer.h"

namespace precitec
{

namespace grabber
{

TriggerCmdServer::TriggerCmdServer(CHRCommunication& p_rCHRCommunication)
    : m_rCHRCommunication(p_rCHRCommunication)
{
}

TriggerCmdServer::~TriggerCmdServer()
{
}

} // namespace grabber
} // namespace precitec;
