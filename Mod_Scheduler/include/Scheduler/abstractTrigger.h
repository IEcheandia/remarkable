#pragma once

#include <map>
#include <memory>
#include <ctime>

#include "json.hpp"

namespace precitec
{
namespace scheduler
{

struct SignalInfo
{
    uint32_t signalNumberDuringObservationalPeriod = 0;
    std::time_t beginObservationalPeriod;
    std::time_t endObservationalPeriod;
    std::map<std::string, std::string> cumulativeMetaData;
    std::string triggerName{};
};

class AbstractTrigger
{
public:
    virtual SignalInfo getSignalInfo(const time_t &endObservationalPeriod) = 0;
    virtual const std::string &name() const = 0;
    virtual ~AbstractTrigger(){};

    nlohmann::json toJson() const;

    std::map<std::string, std::string> settings() const;
    void setSettings(const std::map<std::string, std::string> &settings);

    std::time_t beginObservationalPeriod() const;

    void setBeginObservationalPeriod(const std::time_t &beginObservationalPeriod);

private:
    std::map<std::string, std::string> m_settings;
    std::time_t m_beginObservationalPeriod;
};

class AbstractTriggerFactory
{
public:
    virtual std::unique_ptr<AbstractTrigger> make(const std::map<std::string, std::string> &settings) const = 0;
    virtual ~AbstractTriggerFactory(){};
};

}
}
