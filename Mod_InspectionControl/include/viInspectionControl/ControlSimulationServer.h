#pragma once

#include "event/controlSimulation.handler.h"
#include "VI_InspectionControl.h"

namespace precitec
{
    using namespace interface;

namespace ethercat
{

/**
 * ControlSimulationServer
 **/
class ControlSimulationServer : public TControlSimulation<AbstractInterface>
{

public:
    ControlSimulationServer(VI_InspectionControl& p_rVI_InspectionControl);
    virtual ~ControlSimulationServer();

    virtual void activateControlSimulation(bool p_oState);
    virtual void setInspectionCycle(bool p_oState, uint32_t p_oProductType, uint32_t p_oProductNumber);
    virtual void setSeamSeries(bool p_oState, uint32_t p_oSeamSeries);
    virtual void setSeam(bool p_oState, uint32_t p_oSeam);
    virtual void setCalibration(bool p_oState, uint32_t p_oMode);
    virtual void genPurposeDigIn(uint8_t p_oAddress, int16_t p_oDigInValue);
    virtual void quitSystemFault(bool p_oState);

private:
    VI_InspectionControl &m_rVI_InspectionControl;
    bool m_oControlSimulationIsOn;
    bool m_oContinuouslyModeActive;

};

} // namespace ethercat
} // namespace precitec

