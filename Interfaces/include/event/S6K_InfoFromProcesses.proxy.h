#pragma once

#include "event/S6K_InfoFromProcesses.interface.h"
#include "server/eventProxy.h"

namespace precitec
{

namespace interface
{

template <>
class TS6K_InfoFromProcesses<EventProxy> : public Server<EventProxy>, public TS6K_InfoFromProcesses<AbstractInterface>, public TS6K_InfoFromProcessesMessageDefinition
{
public:
    TS6K_InfoFromProcesses() : EVENT_PROXY_CTOR(TS6K_InfoFromProcesses), TS6K_InfoFromProcesses<AbstractInterface>()
    {
    }

    ~TS6K_InfoFromProcesses() override {}

    void maxSouvisSpeed(uint32_t p_oSpeed) override
    {
        INIT_EVENT(MaxSouvisSpeed);
        signaler().marshal(p_oSpeed);
        signaler().send();
    }

    void souvisControlBits(bool p_oSouvisPresent, bool p_oSouvisSelected) override
    {
        INIT_EVENT(SouvisControlBits);
        signaler().marshal(p_oSouvisPresent);
        signaler().marshal(p_oSouvisSelected);
        signaler().send();
    }

    virtual void passS6K_CS_DataBlock_To_Inspect (uint32_t p_oProductNo, uint32_t p_oBatchID, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                      uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock) override
    {
        INIT_EVENT(PassS6K_CS_DataBlock_To_Inspect);
        signaler().marshal(p_oProductNo);
        signaler().marshal(p_oBatchID);
        signaler().marshal(p_oSeamNo);
        signaler().marshal(p_oBlockNo);
        signaler().marshal(p_oFirstMeasureInBlock);
        signaler().marshal(p_oMeasureCntInBlock);
        signaler().marshal(p_oMeasuresPerResult);
        signaler().marshal(p_oValuesPerMeasure);
        signaler().marshal(p_oCS_DataBlock);
        signaler().send();
    }

    void numberOfPresentSeams(uint32_t p_oSeams) override
    {
        INIT_EVENT(NumberOfPresentSeams);
        signaler().marshal(p_oSeams);
        signaler().send();
    }

    void productNoFromTCP(uint32_t p_oProductNo) override
    {
        INIT_EVENT(ProductNoFromTCP);
        signaler().marshal(p_oProductNo);
        signaler().send();
    }

};

} // namespace interface
} // namespace precitec
