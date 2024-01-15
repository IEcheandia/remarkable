#pragma once

#include "abstractTrigger.h"

namespace precitec
{
namespace scheduler
{

class CronTrigger : public AbstractTrigger
{
public:
    CronTrigger();
    SignalInfo getSignalInfo(const time_t &endObservationalPeriod) override;
    const std::string &name() const override;
    virtual ~CronTrigger() override{};

    static const std::string mName;
};

class CronTriggerFactory : public AbstractTriggerFactory
{
public:
    std::unique_ptr<AbstractTrigger> make(const std::map<std::string, std::string> &settings) const override;
    virtual ~CronTriggerFactory() override{};
};

}
}
