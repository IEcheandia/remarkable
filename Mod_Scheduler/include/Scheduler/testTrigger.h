#pragma once

#include "abstractTrigger.h"

namespace precitec
{
namespace scheduler
{

class TestTrigger : public AbstractTrigger
{
public:
    TestTrigger();
    SignalInfo getSignalInfo(const time_t &endObservationalPeriod) override;
    const std::string &name() const override;
    virtual ~TestTrigger() override{};


    static const std::string mName;
    uint32_t m_updateNumber = 0;
};

class TestTriggerFactory : public AbstractTriggerFactory
{
public:
    std::unique_ptr<AbstractTrigger> make(const std::map<std::string, std::string> &settings) const override;
    virtual ~TestTriggerFactory() override{};
};

}
}
