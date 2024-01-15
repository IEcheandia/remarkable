#pragma once

#include "server/interface.h"
#include "message/simulationCmd.h"
#include "module/interfaces.h"
#include "protocol/protocol.info.h"

namespace precitec
{
namespace interface
{

template <int mode>
class TSimulationCmd;

/**
 * The simulation command interface.
 *
 * Allows to send simulation specific commands to the App_Simulation.
 **/
template<>
class TSimulationCmd<AbstractInterface>
{
public:
    TSimulationCmd() {}
    virtual ~TSimulationCmd() {}

    typedef Poco::UUID PocoUUID;

    /**
     * Initialize simulation for @p product with simulation data from @p productInstance of @p dataProduct.
     **/
    virtual SimulationInitStatus initSimulation(PocoUUID product, PocoUUID productInstance, PocoUUID dataProduct) = 0;
    /**
     * Proceed to next image and simulate it.
     **/
    virtual SimulationFrameStatus nextFrame() = 0;
    /**
     * Go back to previous image and simulate it.
     **/
    virtual SimulationFrameStatus previousFrame() = 0;
    /**
     * Fast forward to the next seam and simulate the first image of the seam.
     **/
    virtual SimulationFrameStatus nextSeam() = 0;
    /**
     * Go back to the start of the current seam and simulate the first image of the seam.
     * If the current image is the start of a seam, it goes back to the previous seam.
     **/
    virtual SimulationFrameStatus previousSeam() = 0;
    /**
     * Go back to the start of the current seam and simulate the first image of the seam.
     * If the current image is the start of a seam, do not change the current image.
    */
    virtual SimulationFrameStatus seamStart() = 0;
    /**
     * Forward to the image identified by @p index and simulate it.
     **/
    virtual SimulationFrameStatus jumpToFrame(uint32_t index) = 0;
    /**
     * Simulate the current frame again. Will trigger an end of automatic mode when the seam changes.
     **/
    virtual SimulationFrameStatus sameFrame() = 0;

    /**
     * Stops the simulation (end of automatic mode) and restart at frame 0.
     **/
    virtual SimulationFrameStatus stop() = 0;
};

struct TSimulationCmdMessageDefinition
{
    MESSAGE(SimulationInitStatus, InitSimulation, Poco::UUID, Poco::UUID, Poco::UUID);
    MESSAGE(SimulationFrameStatus, NextFrame, void);
    MESSAGE(SimulationFrameStatus, PreviousFrame, void);
    MESSAGE(SimulationFrameStatus, NextSeam, void);
    MESSAGE(SimulationFrameStatus, PreviousSeam, void);
    MESSAGE(SimulationFrameStatus, SeamStart, void);
    MESSAGE(SimulationFrameStatus, JumpToFrame, uint32_t);
    MESSAGE(SimulationFrameStatus, SameFrame, void);
    MESSAGE(SimulationFrameStatus, Stop, void);

    MESSAGE_LIST(
        InitSimulation,
        NextFrame,
        PreviousFrame,
        NextSeam,
        PreviousSeam,
        SeamStart,
        JumpToFrame,
        SameFrame,
        Stop
    );
};

template<>
class TSimulationCmd<Messages> : public Server<Messages>, public TSimulationCmdMessageDefinition
{
public:
    TSimulationCmd<Messages>() : info(system::module::SimulationCmd, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
    MessageInfo info;

private:
    enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
    enum { sendBufLen  = 500*Bytes, replyBufLen = 10*MBytes, NumBuffers=128 };
};

}
}
