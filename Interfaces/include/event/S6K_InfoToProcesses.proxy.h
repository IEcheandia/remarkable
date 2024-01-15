#pragma once

#include "event/S6K_InfoToProcesses.interface.h"
#include "server/eventProxy.h"

namespace precitec
{

namespace interface
{

template <>
class TS6K_InfoToProcesses<EventProxy> : public Server<EventProxy>, public TS6K_InfoToProcesses<AbstractInterface>, public TS6K_InfoToProcessesMessageDefinition
{
public:
    TS6K_InfoToProcesses() : EVENT_PROXY_CTOR(TS6K_InfoToProcesses), TS6K_InfoToProcesses<AbstractInterface>()
    {
    }

    ~TS6K_InfoToProcesses() override {}

    void systemReadyInfo(bool onoff) override
    {
        INIT_EVENT(SystemReadyInfo);
        signaler().marshal(onoff);
        signaler().send();
    }

    void sendRequestQualityData(void) override
    {
        INIT_EVENT(SendRequestQualityData);
        signaler().send();
    }

    void sendRequestInspectData(void) override
    {
        INIT_EVENT(SendRequestInspectData);
        signaler().send();
    }

    void sendRequestStatusData(void) override
    {
        INIT_EVENT(SendRequestStatusData);
        signaler().send();
    }

    void sendS6KCmdToProcesses(S6K_CmdToProcesses p_oCmd, uint64_t p_oValue) override
    {
        INIT_EVENT(SendS6KCmdToProcesses);
        signaler().marshal(p_oCmd);
        signaler().marshal(p_oValue);
        signaler().send();
    }

};

} // namespace interface
} // namespace precitec
