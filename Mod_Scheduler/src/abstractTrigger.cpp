#include "Scheduler/abstractTrigger.h"

namespace precitec
{
namespace scheduler
{

nlohmann::json AbstractTrigger::toJson() const
{
    return {{"Name", name()}, {"Settings", settings()}};
}

std::map<std::string, std::string> AbstractTrigger::settings() const
{
    return m_settings;
}

void AbstractTrigger::setSettings(const std::map<std::string, std::string> &settings)
{
    m_settings = settings;
}

std::time_t AbstractTrigger::beginObservationalPeriod() const
{
    return m_beginObservationalPeriod;
}

void AbstractTrigger::setBeginObservationalPeriod(const std::time_t &beginObservationalPeriod)
{
    m_beginObservationalPeriod = beginObservationalPeriod;
}

}
}
