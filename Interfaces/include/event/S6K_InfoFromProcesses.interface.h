#pragma once

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "event/inspectionOut.h"

namespace precitec
{

namespace interface
{

using namespace  system;
using namespace  message;

template <int mode>
class TS6K_InfoFromProcesses;

template<>
class TS6K_InfoFromProcesses<AbstractInterface>
{
public:
    TS6K_InfoFromProcesses() {}
    virtual ~TS6K_InfoFromProcesses() {}
public:
    virtual void maxSouvisSpeed (uint32_t p_oSpeed) = 0;
    virtual void souvisControlBits (bool p_oSouvisPresent, bool p_oSouvisSelected) = 0;
    virtual void passS6K_CS_DataBlock_To_Inspect (uint32_t p_oProductNo, uint32_t p_oBatchID, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                      uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock) = 0;
    virtual void numberOfPresentSeams (uint32_t p_oSeams) = 0;
    virtual void productNoFromTCP(uint32_t p_oProductNo) = 0;
};

struct TS6K_InfoFromProcessesMessageDefinition
{
    EVENT_MESSAGE(MaxSouvisSpeed, uint32_t);
    EVENT_MESSAGE(SouvisControlBits, bool, bool);
    EVENT_MESSAGE(PassS6K_CS_DataBlock_To_Inspect, uint32_t, uint32_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, CS_BlockType);
    EVENT_MESSAGE(NumberOfPresentSeams, uint32_t);
    EVENT_MESSAGE(ProductNoFromTCP, uint32_t);
    MESSAGE_LIST(
        MaxSouvisSpeed,
        SouvisControlBits,
        PassS6K_CS_DataBlock_To_Inspect,
        NumberOfPresentSeams,
        ProductNoFromTCP
    );
};

template <>
class TS6K_InfoFromProcesses<Messages> : public Server<Messages>, public TS6K_InfoFromProcessesMessageDefinition
{
public:
    TS6K_InfoFromProcesses<Messages>() : info(system::module::S6K_InfoFromProcesses, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;
private:
    /// Konstanten wg Lesbarkeit, diese könnten auch in der Basisklasse stehen, würden dann aber wohl kaum verwendet
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 2000*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
};

} // namespace interface
} // namespace precitec
