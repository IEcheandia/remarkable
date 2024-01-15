#pragma once

#include "abstractTask.h"

#include <map>
#include <memory>
#include <map>

namespace precitec
{
namespace scheduler
{

class TaskFactory
{
    std::map<std::string, std::unique_ptr<AbstractTaskFactory>> m_taskFactories;

public:
    TaskFactory();
    std::unique_ptr<AbstractTask> make(const nlohmann::json &jsonObject);
    std::unique_ptr<AbstractTask> make(const std::string &taskName, const std::map<std::string, std::string> &settings);
};

}
}
