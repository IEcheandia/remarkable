#include "Scheduler/exportProductTask.h"
#include "Scheduler/exportProductTaskProcess.h"
#include <Scheduler/eventTrigger.h>

namespace precitec
{
namespace scheduler
{

const std::string ExportProductTask::mName = "ExportProductTask";

std::set<std::pair<std::string, std::optional<precitec::interface::SchedulerEvents>>> ExportProductTask::onlySupportedTriggers() const
{
    return {
        {EventTrigger::mName, precitec::interface::SchedulerEvents::ProductAdded},
        {EventTrigger::mName, precitec::interface::SchedulerEvents::ProductModified}
    };
}

void ExportProductTask::run()
{
    const auto &metaData = signalInfoMetaData();
    if (auto it = metaData.find(std::string{"uuid"}); it != metaData.end() && !it->second.empty())
    {
        auto settings = this->settings();
        settings[std::string{"uuid"}] = it->second;
        setSettings(settings);
    }
    if (!checkSettings())
    {
        wmLog(precitec::eError, "settings false\n");
        log(false);
        return;
    }

    ExportProductTaskProcess process;
    process.setInfo(settings());
    log(process.run());
}

const std::string &ExportProductTask::name() const
{
    return mName;
}

AbstractTask *ExportProductTask::clone() const
{
    return new ExportProductTask(static_cast<ExportProductTask const &>(*this));
}

bool ExportProductTask::checkSettings() const
{
    const auto &map = settings();
    if (map.find("uuid") == map.end() ||
        map.find("TargetIpAddress") == map.end() ||
        map.find("TargetUserName") == map.end() ||
        map.find("TargetPassword") == map.end() ||
        map.find("TargetDirectoryPath") == map.end())
    {
        return false;
    }
    return true;
}

std::unique_ptr<AbstractTask> ExportProductTaskFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto result = std::make_unique<ExportProductTask>();
    result->setSettings(settings);
    return result;
}

}
}
