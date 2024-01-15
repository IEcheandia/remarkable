#include "Scheduler/eventTrigger.h"

#include "module/moduleLogger.h"

namespace precitec
{

namespace scheduler
{

using namespace interface;

const std::string EventTrigger::mName = "EventTrigger";

EventTrigger::EventTrigger()
{
    setBeginObservationalPeriod(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

SignalInfo EventTrigger::getSignalInfo(const time_t &endObservationalPeriod)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    SignalInfo signalInfo;
    signalInfo.triggerName = name();
    signalInfo.beginObservationalPeriod = beginObservationalPeriod();
    signalInfo.endObservationalPeriod = endObservationalPeriod;
    signalInfo.signalNumberDuringObservationalPeriod = m_signalNumberDuringObservationalPeriod;
    m_signalNumberDuringObservationalPeriod = 0;
    signalInfo.cumulativeMetaData = std::move(m_info);
    m_info.clear();
    return signalInfo;
}

const std::string &EventTrigger::name() const
{
    return mName;
}

void EventTrigger::schedulerEventFunction(SchedulerEvents event, const std::map<std::string, std::string>& info)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    int eventNumber;
    try {
        eventNumber = std::stoi(settings().at("event"));
    }
    catch (const std::exception& exception)
    {
        wmLog(eDebug, "Trigger: %s\n", name());
        wmLog(eDebug, "Bad format: %s\n", exception.what());
        return;
    }
    if (eventNumber == static_cast<int>(event))
    {
        auto extendedInfo = info;
        m_signalNumberDuringObservationalPeriod++;
        extendedInfo["Source"] = "SchedulerEventsServer::schedulerEventFunction";
        extendedInfo["Event number"] = std::to_string(eventNumber);
        m_info.clear();
        m_info = std::move(extendedInfo);
    }
}

std::unique_ptr<AbstractTrigger> EventTriggerFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto eventTrigger =  std::make_unique<EventTrigger>();
    eventTrigger->setSettings(settings);
    return eventTrigger;
}

}
}

