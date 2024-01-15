#pragma once

#include <memory>
#include <string>

#include <Poco/Runnable.h>

#include "json.hpp"

#include "abstractTrigger.h"
#include "module/moduleLogger.h"
#include "event/schedulerEvents.interface.h"
#include <map>
#include <set>
#include <optional>

namespace precitec
{
namespace scheduler
{

class AbstractTask : public Poco::Runnable
{
public:
    virtual const std::string &name() const = 0;
    virtual AbstractTask *clone() const = 0;
    virtual bool checkSettings() const = 0;
    // "all triggers" is related to empty set
    virtual std::set<std::pair<std::string, std::optional<precitec::interface::SchedulerEvents>>> onlySupportedTriggers() const;

    std::map<std::string, std::string> settings() const;
    void setSignalInfo(const SignalInfo &signalInfo);
    void setSettings(const std::map<std::string, std::string> &settings);

    nlohmann::json toJson() const;
    void log(bool result) const;

    virtual ~AbstractTask(){};

protected:
    const std::map<std::string, std::string> &signalInfoMetaData() const
    {
        return m_signalInfo.cumulativeMetaData;
    }

private:
    std::map<std::string, std::string> m_settings;
    SignalInfo m_signalInfo;
};

class AbstractTaskFactory
{
public:
    virtual std::unique_ptr<AbstractTask> make(const std::map<std::string, std::string> &settings) const = 0;
    virtual ~AbstractTaskFactory(){};
};

}
}
