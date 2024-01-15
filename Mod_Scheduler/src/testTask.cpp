#include "Scheduler/testTask.h"
#include "json.hpp"

using Json = nlohmann::json;

namespace precitec
{
namespace scheduler
{

const std::string TestTask::mName = "TestTask";
unsigned int TestTask::callNumber = 0;

void TestTask::run()
{
    wmLog(eError, std::to_string(callNumber+=(settings().size() + 1)));
    log(true);
}

const std::string &TestTask::name() const
{
    return mName;
}

AbstractTask *TestTask::clone() const
{
    return new TestTask(static_cast<TestTask const &>(*this));
}

bool TestTask::checkSettings() const
{
    return true;
}

std::unique_ptr<AbstractTask> TestTaskFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto result = std::make_unique<TestTask>();
    result->setSettings(settings);
    return result;
}

}
}