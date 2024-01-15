#include "Scheduler/signalManagerLoader.h"
#include "Scheduler/signalManager.h"

namespace precitec
{
namespace scheduler
{

std::set<SignalManager> SignalManagerLoader::load(std::istream &input)
{
    std::set<SignalManager> signalManagers;
    if (input.bad())
    {
        return signalManagers;
    }

    nlohmann::json js;

    try
    {
        js = nlohmann::json::parse(input);
    }
    catch (const std::exception &e)
    {
        return signalManagers;
    }

    for (auto it = js.begin(); it != js.end(); ++it)
    {
        auto signalManager = SignalManagerFactory::make(*it);
        if (signalManager.task() && signalManager.trigger())
        {
            signalManagers.insert(std::move(signalManager));
        }
    }

    return signalManagers;
}

}
}
