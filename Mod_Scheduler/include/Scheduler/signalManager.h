#pragma once

#include <memory>

#include <Poco/DateTime.h>
#include <Poco/UUID.h>

#include "abstractTask.h"
#include "abstractTrigger.h"
#include "taskFactory.h"
#include "triggerFactory.h"

#include "json.hpp"

namespace precitec
{
namespace scheduler
{

struct JobDescription
{
    std::unique_ptr<AbstractTask> task;
    SignalInfo signalInfo;
};

class SignalManager
{
public:
    void setTask(std::shared_ptr<AbstractTask> task);
    void setTrigger(std::shared_ptr<AbstractTrigger> trigger);
    void setId(const Poco::UUID &id);

    AbstractTask *task() const;
    AbstractTrigger *trigger() const;
    const Poco::UUID &id() const;

    nlohmann::json toJson() const;

    JobDescription generate() const;

    friend bool operator<(const SignalManager &lhs, const SignalManager &rhs)
    {
        return lhs.id().toString() < rhs.id().toString();
    }

    friend bool operator==(const SignalManager &lhs, const SignalManager &rhs)
    {
        return lhs.id() == rhs.id();
    };

    static const std::string idKey;
    static const std::string taskKey;
    static const std::string triggerKey;

private:
    std::shared_ptr<AbstractTask> m_task = nullptr;
    std::shared_ptr<AbstractTrigger> m_trigger = nullptr;
    Poco::UUID m_id;
};

class SignalManagerFactory
{
public:
static SignalManager make(const nlohmann::json &json);
static SignalManager make(std::shared_ptr<AbstractTask> task,
                          std::shared_ptr<AbstractTrigger> trigger,
                          const Poco::UUID &id);;
};

}
}