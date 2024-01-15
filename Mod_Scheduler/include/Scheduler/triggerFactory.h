#pragma once

#include "abstractTrigger.h"

#include <map>
#include <memory>

namespace precitec
{
namespace scheduler
{

class TriggerFactory
{
    std::map<std::string, std::unique_ptr<AbstractTriggerFactory>> triggerFactories;

public:
    TriggerFactory();
    std::unique_ptr<AbstractTrigger> make(const nlohmann::json &jsonObject);
    std::unique_ptr<AbstractTrigger> make(const std::string &triggerName, const std::map<std::string, std::string> &settings);
};

}
}
