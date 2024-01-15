#pragma once

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

namespace precitec
{

namespace interface
{

using namespace system;
using namespace message;

template <int mode>
class TControlSimulation;

template<>
class TControlSimulation<AbstractInterface>
{
public:
    TControlSimulation() {}
    virtual ~TControlSimulation() {}
public:
    virtual void activateControlSimulation (bool p_oState) = 0;
    virtual void setInspectionCycle (bool p_oState, uint32_t p_oProductType, uint32_t oProductNumber) = 0;
    virtual void setSeamSeries (bool p_oState, uint32_t p_oSeamSeries) = 0;
    virtual void setSeam (bool p_oState, uint32_t p_oSeam) = 0;
    virtual void setCalibration (bool p_oState, uint32_t p_oMode) = 0;
    virtual void genPurposeDigIn (uint8_t p_oAddress, int16_t p_oDigInValue) = 0;
    virtual void quitSystemFault (bool p_oState) = 0;
};

struct TControlSimulationMessageDefinition
{
    EVENT_MESSAGE(ActivateControlSimulation, bool);
    EVENT_MESSAGE(SetInspectionCycle, bool, uint32_t, uint32_t);
    EVENT_MESSAGE(SetSeamSeries, bool, uint32_t);
    EVENT_MESSAGE(SetSeam, bool, uint32_t);
    EVENT_MESSAGE(SetCalibration, bool, uint32_t);
    EVENT_MESSAGE(GenPurposeDigIn, uint8_t, int16_t);
    EVENT_MESSAGE(QuitSystemFault, bool);
    MESSAGE_LIST(
        ActivateControlSimulation,
        SetInspectionCycle,
        SetSeamSeries,
        SetSeam,
        SetCalibration,
        GenPurposeDigIn,
        QuitSystemFault
    );
};

template <>
class TControlSimulation<Messages> : public Server<Messages>, public TControlSimulationMessageDefinition
{
public:
    TControlSimulation<Messages>() : info(system::module::ControlSimulation, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;
private:
    /// Konstanten wg Lesbarkeit, diese könnten auch in der Basisklasse stehen, würden dann aber wohl kaum verwendet
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 2000*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
};

} // namespace interface
} // namespace precitec
