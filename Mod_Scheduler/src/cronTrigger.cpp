#include "Scheduler/cronTrigger.h"
#include "Scheduler/croncpp.h"
#include "module/moduleLogger.h"

#include <chrono>

namespace precitec
{
namespace scheduler
{
const std::string CronTrigger::mName = "CronTrigger";

SignalInfo CronTrigger::getSignalInfo(const time_t &endObservationalPeriod)
{
    SignalInfo signalInfo;
    signalInfo.triggerName = name();
    signalInfo.beginObservationalPeriod = beginObservationalPeriod();
    signalInfo.endObservationalPeriod = endObservationalPeriod;
    signalInfo.signalNumberDuringObservationalPeriod = 0;
    try
    {
        // Please see https://github.com/mariusbancila/croncpp
        auto cron = cron::make_cron(settings().at("cron"));
        std::time_t past = signalInfo.beginObservationalPeriod;
        std::time_t current = signalInfo.endObservationalPeriod;
        for (std::time_t next = cron::cron_next(cron, past); next < current; next = cron::cron_next(cron, next))
        {
            signalInfo.signalNumberDuringObservationalPeriod++;
            setBeginObservationalPeriod(next);
        }
    }
    catch (cron::bad_cronexpr const & ex)
    {
        wmLog(eDebug, "Trigger: %s\n", name());
        wmLog(eDebug, "Bad format: %s\n", ex.what());
    }
    return signalInfo;
}

const std::string &CronTrigger::name() const
{
    return mName;
}

CronTrigger::CronTrigger()
{
    setBeginObservationalPeriod(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

std::unique_ptr<AbstractTrigger> CronTriggerFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto cronTrigger =  std::make_unique<CronTrigger>();
    cronTrigger->setSettings(settings);
    return cronTrigger;
}
}
}