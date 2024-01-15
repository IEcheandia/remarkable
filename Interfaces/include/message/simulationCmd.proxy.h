#pragma once

#include "message/simulationCmd.interface.h"
#include "server/proxy.h"

namespace precitec
{
namespace interface
{

template <>
class TSimulationCmd<MsgProxy> : public Server<MsgProxy>, public TSimulationCmd<AbstractInterface>, public TSimulationCmdMessageDefinition
{
public:
    TSimulationCmd()
        : PROXY_CTOR(TSimulationCmd)
        , TSimulationCmd<AbstractInterface>()
    {
    }

    ~TSimulationCmd() override {}

    SimulationInitStatus initSimulation(PocoUUID product, PocoUUID productInstance, PocoUUID dataProduct) override
    {
        INIT_MESSAGE(InitSimulation);
        sender().marshal(product);
        sender().marshal(productInstance);
        sender().marshal(dataProduct);
        return send<SimulationInitStatus>();
    }

    SimulationFrameStatus nextFrame() override
    {
        INIT_MESSAGE(NextFrame);
        return send<SimulationFrameStatus>();
    }

    SimulationFrameStatus previousFrame() override
    {
        INIT_MESSAGE(PreviousFrame);
        return send<SimulationFrameStatus>();
    }

    SimulationFrameStatus nextSeam() override
    {
        INIT_MESSAGE(NextSeam);
        return send<SimulationFrameStatus>();
    }

    SimulationFrameStatus previousSeam() override
    {
        INIT_MESSAGE(PreviousSeam);
        return send<SimulationFrameStatus>();
    }

    SimulationFrameStatus seamStart() override
    {
        INIT_MESSAGE(SeamStart);
        return send<SimulationFrameStatus>();
    }

    SimulationFrameStatus jumpToFrame(uint32_t index) override
    {
        INIT_MESSAGE(JumpToFrame);
        sender().marshal(index);
        return send<SimulationFrameStatus>();
    }

    SimulationFrameStatus sameFrame() override
    {
        INIT_MESSAGE(SameFrame);
        return send<SimulationFrameStatus>();
    }

    SimulationFrameStatus stop() override
    {
        INIT_MESSAGE(Stop);
        return send<SimulationFrameStatus>();
    }

private:
    template <typename T>
    T send()
    {
        T status;
        try
        {
            sender().send();
            sender().deMarshal(status);
        } catch (...)
        {
            wmLog(eError, "Caught an exception in SimulationCmd interface.\n");
        }
        return status;
    }
};

}
}
