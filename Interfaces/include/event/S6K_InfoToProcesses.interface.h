#pragma once

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

namespace precitec
{

namespace interface
{

using namespace  system;
using namespace  message;

enum S6K_CmdToProcesses {eS6K_CmdBatchID, eS6K_CmdProductNumber};

template <int mode>
class TS6K_InfoToProcesses;

template<>
class TS6K_InfoToProcesses<AbstractInterface>
{
public:
    TS6K_InfoToProcesses() {}
    virtual ~TS6K_InfoToProcesses() {}
public:
    virtual void systemReadyInfo (bool onoff) = 0;
    virtual void sendRequestQualityData (void) = 0;
    virtual void sendRequestInspectData (void) = 0;
    virtual void sendRequestStatusData (void) = 0;
    virtual void sendS6KCmdToProcesses (S6K_CmdToProcesses p_oCmd, uint64_t p_oValue) = 0;
};

struct TS6K_InfoToProcessesMessageDefinition
{
    EVENT_MESSAGE(SystemReadyInfo, bool);
    EVENT_MESSAGE(SendRequestQualityData, void);
    EVENT_MESSAGE(SendRequestInspectData, void);
    EVENT_MESSAGE(SendRequestStatusData, void);
    EVENT_MESSAGE(SendS6KCmdToProcesses, S6K_CmdToProcesses, uint64_t);
    MESSAGE_LIST(
        SystemReadyInfo,
        SendRequestQualityData,
        SendRequestInspectData,
        SendRequestStatusData,
        SendS6KCmdToProcesses
    );
};

template <>
class TS6K_InfoToProcesses<Messages> : public Server<Messages>, public TS6K_InfoToProcessesMessageDefinition
{
public:
    TS6K_InfoToProcesses<Messages>() : info(system::module::S6K_InfoToProcesses, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;
private:
    /// Konstanten wg Lesbarkeit, diese könnten auch in der Basisklasse stehen, würden dann aber wohl kaum verwendet
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 2000*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
};

} // namespace interface
} // namespace precitec
