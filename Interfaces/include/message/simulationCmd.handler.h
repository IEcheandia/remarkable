#pragma once

#include "message/simulationCmd.interface.h"
#include "server/handler.h"

namespace precitec
{
namespace interface
{

template <>
class TSimulationCmd<MsgHandler> : public Server<MsgHandler>, public TSimulationCmdMessageDefinition
{
public:
    MSG_HANDLER( TSimulationCmd );

public:
    typedef Poco::UUID PocoUUID;

    void registerCallbacks()
    {
        REGISTER_MESSAGE(InitSimulation, initSimulation);
        REGISTER_MESSAGE(NextFrame, nextFrame);
        REGISTER_MESSAGE(PreviousFrame, previousFrame);
        REGISTER_MESSAGE(NextSeam, nextSeam);
        REGISTER_MESSAGE(PreviousSeam, previousSeam);
        REGISTER_MESSAGE(SeamStart, seamStart);
        REGISTER_MESSAGE(JumpToFrame, jumpToFrame);
        REGISTER_MESSAGE(SameFrame, sameFrame);
        REGISTER_MESSAGE(Stop, stopSimulation);
    }

    void initSimulation(Receiver &receiver)
    {
        Poco::UUID product;
        Poco::UUID productInstance;
        Poco::UUID dataProduct;
        receiver.deMarshal(product);
        receiver.deMarshal(productInstance);
        receiver.deMarshal(dataProduct);
        const auto ret = getServer()->initSimulation(product, productInstance, dataProduct);
        receiver.marshal(ret);
        receiver.reply();
    }

    void nextFrame(Receiver &receiver)
    {
        callback(receiver, &TSimulationCmd<AbstractInterface>::nextFrame);
    }

    void previousFrame(Receiver &receiver)
    {
        callback(receiver, &TSimulationCmd<AbstractInterface>::previousFrame);
    }

    void nextSeam(Receiver &receiver)
    {
        callback(receiver, &TSimulationCmd<AbstractInterface>::nextSeam);
    }

    void previousSeam(Receiver &receiver)
    {
        callback(receiver, &TSimulationCmd<AbstractInterface>::previousSeam);
    }

    void seamStart(Receiver &receiver)
    {
        callback(receiver, &TSimulationCmd<AbstractInterface>::seamStart);
    }

    void jumpToFrame(Receiver &receiver)
    {
        uint32_t index = 0;
        receiver.deMarshal(index);
        const auto ret = getServer()->jumpToFrame(index);
        receiver.marshal(ret);
        receiver.reply();
    }

    void sameFrame(Receiver &receiver)
    {
        callback(receiver, &TSimulationCmd<AbstractInterface>::sameFrame);
    }

    void stopSimulation(Receiver &receiver)
    {
        callback(receiver, &TSimulationCmd<AbstractInterface>::stop);
    }

private:
    TSimulationCmd<AbstractInterface> *getServer() const
    {
        return server_;
    }

    void callback(Receiver &receiver, SimulationFrameStatus (TSimulationCmd<AbstractInterface>::* method)())
    {
        const auto ret = (getServer()->*method)();
        receiver.marshal(ret);
        receiver.reply();
    }
};

}
}
