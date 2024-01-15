#pragma once

#include "event/triggerCmd.interface.h"
#include "common/defines.h"

class MockTriggerCmd : public precitec::interface::TTriggerCmd<precitec::interface::AbstractInterface>
{
public:
    void single(UNUSED const std::vector<int>& p_rSensorIds, precitec::interface::TriggerContext const& context) override
    {
        m_lastContext = context;
    }
    void burst(UNUSED const std::vector<int>& p_rSensorIds, UNUSED precitec::interface::TriggerContext const& context, UNUSED int source, UNUSED precitec::interface::TriggerInterval const& interval) override
    {
    }
    void cancel(UNUSED const std::vector<int>& p_rSensorIds) override
    {
    }

    const precitec::interface::TriggerContext &lastContext() const
    {
        return m_lastContext;
    }

private:
    precitec::interface::TriggerContext m_lastContext;
};
