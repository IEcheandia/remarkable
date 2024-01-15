#include "viInspectionControl/ControlSimulationServer.h"

#include "common/systemConfiguration.h"

namespace precitec
{

namespace ethercat
{

ControlSimulationServer::ControlSimulationServer(VI_InspectionControl& p_rVI_InspectionControl):
    m_rVI_InspectionControl(p_rVI_InspectionControl),
    m_oControlSimulationIsOn(false)
{
    m_oContinuouslyModeActive = SystemConfiguration::instance().getBool("ContinuouslyModeActive", false);
    wmLog(eDebug, "m_oContinuouslyModeActive (bool): %d\n", m_oContinuouslyModeActive);
}

ControlSimulationServer::~ControlSimulationServer()
{
}

void ControlSimulationServer::activateControlSimulation(bool p_oState)
{
    std::cout << "ControlSimualtion server reached!!!!" << std::endl;
    if (p_oState)
    {
        m_oControlSimulationIsOn = true;
    }
    else
    {
        m_oControlSimulationIsOn = false;
    }
    m_rVI_InspectionControl.activateControlSimulation(p_oState);
}

void ControlSimulationServer::setInspectionCycle(bool p_oState, uint32_t p_oProductType, uint32_t p_oProductNumber)
{
    if (m_oControlSimulationIsOn)
    {
        if (!m_oContinuouslyModeActive)
        {
            if (p_oProductType == 0) p_oProductType = 1; //ProductType 0 is live product !
            m_rVI_InspectionControl.TriggerAutomatic(p_oState, p_oProductType, p_oProductNumber, "no info");
        }
    }
}

void ControlSimulationServer::setSeamSeries(bool p_oState, uint32_t p_oSeamSeries)
{
    if (m_oControlSimulationIsOn)
    {
        if (!m_oContinuouslyModeActive)
        {
            if (p_oSeamSeries != 0) p_oSeamSeries--;
            m_rVI_InspectionControl.TriggerInspectInfo(p_oState, p_oSeamSeries);
        }
    }
}

void ControlSimulationServer::setSeam(bool p_oState, uint32_t p_oSeam)
{
    if (m_oControlSimulationIsOn)
    {
        // no locking with SCANMASTER_Application, the point is to trigger the seams manually
        if (!m_oContinuouslyModeActive)
        {
            if (p_oSeam != 0) p_oSeam--;
            m_rVI_InspectionControl.TriggerInspectStartStop(p_oState, p_oSeam);
        }
    }
}

void ControlSimulationServer::setCalibration(bool p_oState, uint32_t p_oMode)
{
    if (m_oControlSimulationIsOn)
    {
        // no calibration blocking, blocking is only used in conjunction with auto homing at startup
        m_rVI_InspectionControl.TriggerCalibration(p_oState, p_oMode);
    }
}

void ControlSimulationServer::genPurposeDigIn(uint8_t p_oAddress, int16_t p_oDigInValue)
{
    if (m_oControlSimulationIsOn)
    {
        m_rVI_InspectionControl.genPurposeDigIn(p_oAddress, p_oDigInValue);
    }
}

void ControlSimulationServer::quitSystemFault(bool p_oState)
{
    if (m_oControlSimulationIsOn)
    {
        m_rVI_InspectionControl.TriggerQuitSystemFault(p_oState);
    }
}

} // namespace ethercat
} // namespace precitec

