#pragma once

#include "event/S6K_InfoToProcesses.interface.h"
#include "server/eventHandler.h"

namespace precitec
{

namespace interface
{

using namespace  message;

template <>
class TS6K_InfoToProcesses<EventHandler> : public Server<EventHandler>, public TS6K_InfoToProcessesMessageDefinition
{
public:
    EVENT_HANDLER( TS6K_InfoToProcesses );
public:
    void registerCallbacks()
    {
        REGISTER_EVENT(SystemReadyInfo, systemReadyInfo);
        REGISTER_EVENT(SendRequestQualityData, sendRequestQualityData);
        REGISTER_EVENT(SendRequestInspectData, sendRequestInspectData);
        REGISTER_EVENT(SendRequestStatusData, sendRequestStatusData);
        REGISTER_EVENT(SendS6KCmdToProcesses, sendS6KCmdToProcesses);
    }

    void systemReadyInfo(Receiver &receiver)
    {
        bool onoff; receiver.deMarshal(onoff);
        getServer()->systemReadyInfo(onoff);
    }

    void sendRequestQualityData(Receiver &receiver)
    {
        getServer()->sendRequestQualityData();
    }

    void sendRequestInspectData(Receiver &receiver)
    {
        getServer()->sendRequestInspectData();
    }

    void sendRequestStatusData(Receiver &receiver)
    {
        getServer()->sendRequestStatusData();
    }

    void sendS6KCmdToProcesses(Receiver &receiver)
    {
        S6K_CmdToProcesses oCmd; receiver.deMarshal(oCmd);
        uint64_t oValue; receiver.deMarshal(oValue);
        getServer()->sendS6KCmdToProcesses(oCmd, oValue);
    }

private:
    TS6K_InfoToProcesses<AbstractInterface> * getServer()
    {
        return server_;
    }

};

} // namespace interface
} // namespace precitec
