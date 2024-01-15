#include "Scheduler/transferFileTask.h"
#include "Scheduler/eventTrigger.h"
#include "Scheduler/manualTrigger.h"
#include "json.hpp"

#include "Scheduler/transferFileTaskProcess.h"
#include <filesystem>

using Json = nlohmann::json;
namespace fs = std::filesystem;

namespace precitec
{
namespace scheduler
{

const std::string TransferFileTask::mName = "TransferFileTask";

std::set<std::pair<std::string, std::optional<precitec::interface::SchedulerEvents>>> TransferFileTask::onlySupportedTriggers() const
{
    return {
        {EventTrigger::mName, precitec::interface::SchedulerEvents::ProductAdded},
        {EventTrigger::mName, precitec::interface::SchedulerEvents::ProductModified}, 
        {ManualTrigger::mName, precitec::interface::SchedulerEvents::ProductManualTrigger}
    };
}

void TransferFileTask::run()
{
    const auto &metaData = signalInfoMetaData();
    if (auto it = metaData.find(std::string{"filePath"}); it != metaData.end() && !it->second.empty())
    {
        std::string filePath{it->second};
        fs::path path{filePath};
        auto settings = this->settings();
        settings[std::string{"SourceFileName"}] = path.filename();
        settings[std::string{"TargetFileName"}] = path.filename();
        path.remove_filename();
        settings[std::string{"SourceDirectoryPath"}] = path;
        setSettings(settings);
    }
   if( !checkSettings())
    {
        log(false);
        return;
    }
    TransferFileTaskProcess process;
    process.setInfo(settings());
    log(process.run());
}

const std::string &TransferFileTask::name() const
{
    return mName;
}

AbstractTask *TransferFileTask::clone() const
{
    return new TransferFileTask(static_cast<TransferFileTask const &>(*this));
}

bool TransferFileTask::checkSettings() const
{
    const auto &map = settings();
    if (map.find("TargetIpAddress") == map.end() ||
        map.find("TargetUserName") == map.end() ||
        map.find("TargetPassword") == map.end() ||
        map.find("SourceDirectoryPath") == map.end() ||
        map.find("SourceFileName") == map.end() ||
        map.find("TargetDirectoryPath") == map.end() ||
        map.find("TargetFileName") == map.end())
        {
            return false;
        }
    return true;
}

std::unique_ptr<AbstractTask> TransferFileTaskFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto result = std::make_unique<TransferFileTask>();
    result->setSettings(settings);
    return result;
}

}
}
