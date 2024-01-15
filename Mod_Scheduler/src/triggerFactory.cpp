#include "Scheduler/triggerFactory.h"

#include "Scheduler/testTrigger.h"
#include "Scheduler/cronTrigger.h"
#include "Scheduler/eventTrigger.h"
#include "Scheduler/manualTrigger.h"

#include "module/moduleLogger.h"

namespace precitec
{
namespace scheduler
{

TriggerFactory::TriggerFactory()
{
    triggerFactories[TestTrigger::mName] = std::make_unique<TestTriggerFactory>();
    triggerFactories[CronTrigger::mName] = std::make_unique<CronTriggerFactory>();
    triggerFactories[EventTrigger::mName] = std::make_unique<EventTriggerFactory>();
    triggerFactories[ManualTrigger::mName] = std::make_unique<ManualTriggerFactory>();
}

std::unique_ptr<AbstractTrigger> TriggerFactory::make(const nlohmann::json &jsonObject)
{
   std::map<std::string, std::string> settings;
    try
    {
        settings = jsonObject.at("Settings").get<std::map<std::string, std::string>>();
    }
    catch (std::exception const &ex)
    {
        wmLog(eDebug, "Bad json trigger format\n");
        wmLog(eDebug, "Bad format: %s\n", ex.what());
        return {};
    }

    std::string name;
    try
    {
        name = jsonObject.at("Name").get<std::string>();
    }
    catch (std::exception const &ex)
    {
        wmLog(eDebug, "Bad json trigger format\n");
        wmLog(eDebug, "Bad format: %s\n", ex.what());
        return {};
    }

    std::unique_ptr<AbstractTrigger> abstractTrigger = make(name, settings);
    if (!abstractTrigger)
    {
        wmLog(eDebug, "Bad json trigger format\n");
        wmLog(eDebug, "Bad format: there is no trigger with name %s\n", name);
        return {};
    }

    return abstractTrigger;
}

std::unique_ptr<AbstractTrigger> TriggerFactory::make(const std::string &triggerName, const std::map<std::string, std::string> &settings)
{
    if (triggerFactories.find(triggerName) == triggerFactories.end())
    {
        return nullptr;
    }

    return triggerFactories[triggerName]->make(settings);
}

}
}
