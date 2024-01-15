#ifndef S6KINFOTOPROCESSESSERVER_H_
#define S6KINFOTOPROCESSESSERVER_H_

#include "event/S6K_InfoToProcesses.handler.h"
#include "TCPCommunication.h"

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

/**
 * S6K_InfoToProcessesServer
 **/
class S6K_InfoToProcessesServer : public TS6K_InfoToProcesses<AbstractInterface>
{
    public:

    /**
     * Ctor.
     * @param _service Service
     * @return void
     **/
    S6K_InfoToProcessesServer(TCPCommunication& p_rTCPCommunication);
    virtual ~S6K_InfoToProcessesServer();

    virtual void systemReadyInfo(bool onoff)
    {
        m_rTCPCommunication.systemReadyInfo(onoff);
    }

    virtual void sendRequestQualityData(void)
    {
        m_rTCPCommunication.sendRequestQualityData();
    }

    virtual void sendRequestInspectData(void)
    {
        m_rTCPCommunication.sendRequestInspectData();
    }

    virtual void sendRequestStatusData(void)
    {
        m_rTCPCommunication.sendRequestStatusData();
    }

    virtual void sendS6KCmdToProcesses(S6K_CmdToProcesses p_oCmd, uint64_t p_oValue)
    {
        m_rTCPCommunication.sendS6KCmdToProcesses(p_oCmd, p_oValue);
    }

    private:
        TCPCommunication &m_rTCPCommunication;
};

} // namespace tcpcommunication

} // namespace precitec

#endif // S6KINFOTOPROCESSESSERVER_H_

