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

template <int mode>
class TEthercatInputsToService;

template<>
class TEthercatInputsToService<AbstractInterface>
{
public:
    TEthercatInputsToService() {}
    virtual ~TEthercatInputsToService() {}
public:
    virtual void ecatAllDataIn (uint16_t size, stdVecUINT8 data) = 0;
    virtual void ecatAllSlaveInfo (SlaveInfo p_oSlaveInfo) = 0;
    virtual void fieldbusAllDataIn (uint16_t size, stdVecUINT8 data) = 0;
    virtual void fieldbusAllSlaveInfo (SlaveInfo p_oSlaveInfo) = 0;
};

struct TEthercatInputsToServiceMessageDefinition
{
    EVENT_MESSAGE(EcatAllDataIn, uint16_t, stdVecUINT8);
    EVENT_MESSAGE(EcatAllSlaveInfo, SlaveInfo);
    EVENT_MESSAGE(FieldbusAllDataIn, uint16_t, stdVecUINT8);
    EVENT_MESSAGE(FieldbusAllSlaveInfo, SlaveInfo);
    MESSAGE_LIST(
        EcatAllDataIn,
        EcatAllSlaveInfo,
        FieldbusAllDataIn,
        FieldbusAllSlaveInfo
    );
};

template <>
class TEthercatInputsToService<Messages> : public Server<Messages>, public TEthercatInputsToServiceMessageDefinition
{
public:
    TEthercatInputsToService<Messages>() : info(system::module::EthercatInputsToService, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;
private:
    /// Konstanten wg Lesbarkeit, diese könnten auch in der Basisklasse stehen, würden dann aber wohl kaum verwendet
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 2000*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
};

} // namespace interface
} // namespace precitec
