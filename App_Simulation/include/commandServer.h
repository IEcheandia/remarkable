#pragma once

#include "message/simulationCmd.server.h"
#include "event/triggerCmd.proxy.h"
#include "workflow/stateMachine/stateContext.h"

#include <Poco/SharedPtr.h>

#include <memory.h>

class CommandServerTest;

namespace precitec
{

using namespace precitec::interface;

class SequenceInformation;
class VdrFileInfo;

namespace analyzer
{
class CentralDeviceManager;
}

namespace workflow
{
class ProductListProvider;
typedef Poco::SharedPtr<ProductListProvider> SharedProductListProvider;
}

namespace Simulation
{

class CommandServer : public TSimulationCmd<MsgServer>
{
public:
    explicit CommandServer(TTriggerCmd<AbstractInterface> *triggerCmdProxy, analyzer::CentralDeviceManager *deviceManager, const workflow::SharedProductListProvider& stateContext);
    ~CommandServer() override;

    SimulationInitStatus initSimulation(PocoUUID product, PocoUUID productInstance, PocoUUID dataProduct) override;
    SimulationFrameStatus nextFrame() override;
    SimulationFrameStatus previousFrame() override;
    SimulationFrameStatus nextSeam() override;
    SimulationFrameStatus previousSeam() override;
    SimulationFrameStatus seamStart() override;
    SimulationFrameStatus jumpToFrame(uint32_t index) override;
    SimulationFrameStatus sameFrame() override;
    SimulationFrameStatus stop() override;

private:
    bool hasNext() const;
    bool hasPrevious() const;
    bool hasPreviousSeam() const;
    bool hasNextSeam() const;
    void sendCurrentImage();
    SimulationFrameStatus currentStatus() const;
    std::deque<VdrFileInfo>::const_iterator nextSeamPos() const;
    std::deque<VdrFileInfo>::const_iterator previousSeamPos() const;
    std::deque<VdrFileInfo>::const_iterator seamStartPos(std::deque<VdrFileInfo>::const_iterator start, std::deque<VdrFileInfo>::const_iterator end) const;

    TTriggerCmd<AbstractInterface> *m_triggerCmdProxy;
    analyzer::CentralDeviceManager *m_deviceManager;
    workflow::SharedProductListProvider m_stateContext;
    std::unique_ptr<SequenceInformation> m_sequenceInformation;
    std::deque<VdrFileInfo>::const_iterator m_imageIterator;
    std::deque<VdrFileInfo>::const_iterator m_prevIterator;
    std::deque<VdrFileInfo>::const_iterator m_startIterator;
    Poco::UUID m_productInstance;
    Poco::UUID m_product;
    uint32_t m_productType;
    Poco::UUID m_prevProductInstance;
    Poco::UUID m_previousProduct;
    bool m_forceStartAuto = false;
    bool m_forceStartAutoOnSeamChange = false;
    friend CommandServerTest;
};

}
}
