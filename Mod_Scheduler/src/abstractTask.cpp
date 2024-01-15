#include "Scheduler/abstractTask.h"

namespace precitec
{
namespace scheduler
{

std::map<std::string, std::string> AbstractTask::settings() const
{
    return m_settings;
}

void AbstractTask::setSignalInfo(const SignalInfo &signalInfo)
{
    m_signalInfo = signalInfo;
}

void AbstractTask::setSettings(const std::map<std::string, std::string> &settings)
{
    m_settings = settings;
}

nlohmann::json AbstractTask::toJson() const
{
    return {{"Name", name()}, {"Settings", settings()}};
}

void AbstractTask::log(bool result) const
{
    wmLog(eDebug, "Task: %s\n", name());
    wmLog(eDebug, "Trigger: %s\n", m_signalInfo.triggerName);
    wmLog(eDebug, "Signal number: %d\n", m_signalInfo.signalNumberDuringObservationalPeriod);

    if (result)
    {
        wmLog(eDebug, "Result: success\n");
        for (auto &info : m_signalInfo.cumulativeMetaData)
        {
            wmLog(eDebug, "%s\n", info.first);
            wmLog(eDebug, "%s\n", info.second);
        }
    }
    else
    {
        wmLog(eDebug, "Result: fail\n");
    }
}

std::set<std::pair<std::string, std::optional<precitec::interface::SchedulerEvents>>> AbstractTask::onlySupportedTriggers() const
{
    // "all triggers" is related to empty set
    return {};
};

}
}
