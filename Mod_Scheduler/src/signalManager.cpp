#include "Scheduler/signalManager.h"
#include <chrono>

namespace precitec
{
namespace scheduler
{

const std::string SignalManager::idKey = "Uuid";
const std::string SignalManager::taskKey = "Task";
const std::string SignalManager::triggerKey = "Trigger";

void SignalManager::setTask(std::shared_ptr<AbstractTask> task)
{
    m_task = std::move(task);
}

void SignalManager::setTrigger(std::shared_ptr<AbstractTrigger> trigger)
{
    m_trigger = std::move(trigger);
}

void SignalManager::setId(const Poco::UUID &id)
{
    m_id = id;
}

AbstractTask *SignalManager::task() const
{
    return m_task.get();
}

AbstractTrigger *SignalManager::trigger() const
{
    return m_trigger.get();
}

const Poco::UUID &SignalManager::id() const
{
    return m_id;
}

JobDescription SignalManager::generate() const
{
    auto signalInfo = m_trigger->getSignalInfo(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
    signalInfo.cumulativeMetaData["SignalManagerId"] = m_id.toString();
    return JobDescription{std::unique_ptr<AbstractTask>(m_task->clone()), std::move(signalInfo)};
}

nlohmann::json SignalManager::toJson() const
{
    nlohmann::json jsonObject;

    jsonObject[idKey] = m_id.toString();
    jsonObject[taskKey] = (m_task) ? m_task->toJson() : "";
    jsonObject[triggerKey] = (m_trigger) ? m_trigger->toJson() : "";

    return jsonObject;
}

SignalManager SignalManagerFactory::make(const nlohmann::json &json)
{
    SignalManager signalManager;

    if (json.contains(SignalManager::idKey) && json.contains(SignalManager::taskKey) &&
        json.contains(SignalManager::triggerKey))
    {
        auto task = TaskFactory().make(json[SignalManager::taskKey]);
        auto trigger = TriggerFactory().make(json[SignalManager::triggerKey]);
        if (task && trigger)
        {
            signalManager.setTask(std::move(task));
            signalManager.setTrigger(std::move(trigger));
            signalManager.setId(Poco::UUID(json[SignalManager::idKey]));
        }
    }

    return signalManager;
}

SignalManager SignalManagerFactory::make(std::shared_ptr<AbstractTask> task,
                                         std::shared_ptr<AbstractTrigger> trigger,
                                         const Poco::UUID &id)
{
    SignalManager signalManager;

    signalManager.setTask(task);
    signalManager.setTrigger(trigger);
    signalManager.setId(id);

    return signalManager;
}

}
}
