#include "Scheduler/signalManagerWriter.h"
#include "json.hpp"

namespace precitec
{
namespace scheduler
{

bool SignalManagerWriter::write(std::ostream &output, const std::set<SignalManager> &signalManagers)
{
    if (output.bad())
    {
        return false;
    }

    nlohmann::json j;

    for (auto &signalManager : signalManagers)
    {
        j.emplace_back(signalManager.toJson());
    }

    output.clear();
    output << j;

    return true;
}

}
}