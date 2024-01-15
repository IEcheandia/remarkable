#include "Scheduler/testTrigger.h"
#include <chrono>

namespace precitec
{
namespace scheduler
{
const std::string TestTrigger::mName = "TestTrigger";

SignalInfo TestTrigger::getSignalInfo(const time_t &endObservationalPeriod)
{
    m_updateNumber++;
    SignalInfo signalInfo;
    signalInfo.endObservationalPeriod = endObservationalPeriod;
    signalInfo.signalNumberDuringObservationalPeriod = m_updateNumber;
    signalInfo.cumulativeMetaData.emplace(std::pair{"Test", "Data"});
    signalInfo.triggerName = name();
    return signalInfo;
}

const std::string &TestTrigger::name() const
{
    return mName;
}

TestTrigger::TestTrigger()
{
    setBeginObservationalPeriod(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

std::unique_ptr<AbstractTrigger> TestTriggerFactory::make(const std::map<std::string, std::string> &settings) const
{
    return std::make_unique<TestTrigger>();
}

}
}