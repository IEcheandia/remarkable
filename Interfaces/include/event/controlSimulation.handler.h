#pragma once

#include "event/controlSimulation.interface.h"
#include "server/eventHandler.h"

namespace precitec
{

namespace interface
{

using namespace message;

template <>
class TControlSimulation<EventHandler> : public Server<EventHandler>, public TControlSimulationMessageDefinition
{
public:
    EVENT_HANDLER( TControlSimulation );
public:
    void registerCallbacks()
    {
        REGISTER_EVENT(ActivateControlSimulation, activateControlSimulation);
        REGISTER_EVENT(SetInspectionCycle, setInspectionCycle);
        REGISTER_EVENT(SetSeamSeries, setSeamSeries);
        REGISTER_EVENT(SetSeam, setSeam);
        REGISTER_EVENT(SetCalibration, setCalibration);
        REGISTER_EVENT(GenPurposeDigIn, genPurposeDigIn);
        REGISTER_EVENT(QuitSystemFault, quitSystemFault);
    }

    void activateControlSimulation(Receiver &receiver)
    {
        bool oState; receiver.deMarshal(oState);
        getServer()->activateControlSimulation(oState);
    }

    void setInspectionCycle(Receiver &receiver)
    {
        bool oState; receiver.deMarshal(oState);
        uint32_t oProductType; receiver.deMarshal(oProductType);
        uint32_t oProductNumber; receiver.deMarshal(oProductNumber);
        getServer()->setInspectionCycle(oState, oProductType, oProductNumber);
    }

    void setSeamSeries(Receiver &receiver)
    {
        bool oState; receiver.deMarshal(oState);
        uint32_t oSeamSeries; receiver.deMarshal(oSeamSeries);
        getServer()->setSeamSeries(oState, oSeamSeries);
    }

    void setSeam(Receiver &receiver)
    {
        bool oState; receiver.deMarshal(oState);
        uint32_t oSeam; receiver.deMarshal(oSeam);
        getServer()->setSeam(oState, oSeam);
    }

    void setCalibration(Receiver &receiver)
    {
        bool oState; receiver.deMarshal(oState);
        uint32_t oMode; receiver.deMarshal(oMode);
        getServer()->setCalibration(oState, oMode);
    }

    void genPurposeDigIn(Receiver &receiver)
    {
        uint8_t oAddress; receiver.deMarshal(oAddress);
        int16_t oDigInValue; receiver.deMarshal(oDigInValue);
        getServer()->genPurposeDigIn(oAddress, oDigInValue);
    }

    void quitSystemFault(Receiver &receiver)
    {
        bool oState; receiver.deMarshal(oState);
        getServer()->quitSystemFault(oState);
    }

private:
    TControlSimulation<AbstractInterface> * getServer()
    {
        return server_;
    }

};

} // namespace interface
} // namespace precitec
