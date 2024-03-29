#pragma once

#include "event/schedulerEvents.interface.h"
#include "abstractTrigger.h"

#include <mutex>

namespace precitec
{
namespace scheduler
{

using namespace interface;

class ManualTrigger : public AbstractTrigger
{
public:
    ManualTrigger();
    SignalInfo getSignalInfo(const time_t &endObservationalPeriod) override;
    const std::string &name() const override;
    virtual ~ManualTrigger() override{};

    void schedulerEventFunction(SchedulerEvents event, const std::map<std::string, std::string> &info);

    static const std::string mName;

private:
    std::mutex m_mutex;
    uint32_t m_signalNumberDuringObservationalPeriod = 0;
    std::map<std::string, std::string> m_info;
};

class ManualTriggerFactory : public AbstractTriggerFactory
{
public:
    std::unique_ptr<AbstractTrigger> make(const std::map<std::string, std::string> &settings) const override;
    virtual ~ManualTriggerFactory() override {};
};

} // namespace scheduler
} // namespace precitec
