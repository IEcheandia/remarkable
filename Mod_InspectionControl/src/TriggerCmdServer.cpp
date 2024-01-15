#include "viInspectionControl/TriggerCmdServer.h"

namespace precitec
{

namespace ethercat
{

TriggerCmdServer::TriggerCmdServer(VI_InspectionControl& p_rVI_InspectionControl):
    m_rVI_InspectionControl(p_rVI_InspectionControl)
{
}

TriggerCmdServer::~TriggerCmdServer()
{
}

} // namespace ethercat
} // namespace precitec;

