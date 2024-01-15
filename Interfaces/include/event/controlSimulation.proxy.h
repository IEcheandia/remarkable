#pragma once

#include "event/controlSimulation.interface.h"
#include "server/eventProxy.h"

namespace precitec
{

namespace interface
{

template <>
class TControlSimulation<EventProxy> : public Server<EventProxy>, public TControlSimulation<AbstractInterface>, public TControlSimulationMessageDefinition
{
public:
    TControlSimulation() : EVENT_PROXY_CTOR(TControlSimulation), TControlSimulation<AbstractInterface>()
    {
    }

    ~TControlSimulation() override {}

    void activateControlSimulation(bool p_oState) override
    {
        INIT_EVENT(ActivateControlSimulation);
        signaler().marshal(p_oState);
        signaler().send();
    }

    void setInspectionCycle(bool p_oState, uint32_t p_oProductType, uint32_t p_oProductNumber) override
    {
        INIT_EVENT(SetInspectionCycle);
        signaler().marshal(p_oState);
        signaler().marshal(p_oProductType);
        signaler().marshal(p_oProductNumber);
        signaler().send();
    }

    void setSeamSeries(bool p_oState, uint32_t p_oSeamSeries) override
    {
        INIT_EVENT(SetSeamSeries);
        signaler().marshal(p_oState);
        signaler().marshal(p_oSeamSeries);
        signaler().send();
    }

    void setSeam(bool p_oState, uint32_t p_oSeam) override
    {
        INIT_EVENT(SetSeam);
        signaler().marshal(p_oState);
        signaler().marshal(p_oSeam);
        signaler().send();
    }

    void setCalibration(bool p_oState, uint32_t p_oMode) override
    {
        INIT_EVENT(SetCalibration);
        signaler().marshal(p_oState);
        signaler().marshal(p_oMode);
        signaler().send();
    }

    void genPurposeDigIn(uint8_t p_oAddress, int16_t p_oDigInValue) override
    {
        INIT_EVENT(GenPurposeDigIn);
        signaler().marshal(p_oAddress);
        signaler().marshal(p_oDigInValue);
        signaler().send();
    }

    void quitSystemFault(bool p_oState) override
    {
        INIT_EVENT(QuitSystemFault);
        signaler().marshal(p_oState);
        signaler().send();
    }

};

} // namespace interface
} // namespace precitec
