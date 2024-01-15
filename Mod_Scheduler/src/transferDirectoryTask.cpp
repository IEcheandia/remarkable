#include "Scheduler/transferDirectoryTask.h"
#include "Scheduler/transferDirectoryTaskProcess.h"
#include "Scheduler/eventTrigger.h"
#include "Scheduler/manualTrigger.h"
#include "Scheduler/metaDataHelper.h"
#include <filesystem>

#include "json.hpp"

using Json = nlohmann::json;
namespace fs = std::filesystem;

namespace precitec
{
namespace scheduler
{

const std::string TransferDirectoryTask::mName = "TransferDirectoryTask";

std::set<std::pair<std::string, std::optional<precitec::interface::SchedulerEvents>>> TransferDirectoryTask::onlySupportedTriggers() const
{
    return {
        {EventTrigger::mName, precitec::interface::SchedulerEvents::ProductInstanceResultsStored},
        {EventTrigger::mName, precitec::interface::SchedulerEvents::ProductInstanceVideoStored},
        {ManualTrigger::mName, precitec::interface::SchedulerEvents::ProductManualTrigger}
    };
}

void TransferDirectoryTask::run()
{
    const auto &metaData = signalInfoMetaData();
    if (auto it = metaData.find(std::string{"path"}); it != metaData.end() && !it->second.empty())
    {
        std::string filePath{it->second};
        if (filePath.back() == '/')
        {
            filePath.pop_back();
        }
        fs::path path{filePath};
        auto settings = this->settings();
        settings[std::string{"SourceDirectoryName"}] = path.filename();
        settings[std::string{"TargetDirectoryName"}] = path.filename();
        path.remove_filename();
        settings[std::string{"SourceDirectoryPath"}] = path;

        MetaDataHelper helper{it->second +  "metadata.json"};
        helper.updateTargetPath(settings);
        setSettings(settings);
    }
    if (!checkSettings())
    {
       log(false);
       return;
    }

    TransferDirectoryTaskProcess process;
    process.setInfo(settings());
    log(process.run());
}

const std::string &TransferDirectoryTask::name() const
{
    return mName;
}

AbstractTask *TransferDirectoryTask::clone() const
{
    return new TransferDirectoryTask(static_cast<TransferDirectoryTask const &>(*this));
}

bool TransferDirectoryTask::checkSettings() const
{
    const auto &map = settings();
    if (map.find("TargetIpAddress") == map.end() ||
        map.find("TargetUserName") == map.end() ||
        map.find("TargetPassword") == map.end() ||
        map.find("SourceDirectoryPath") == map.end() ||
        map.find("SourceDirectoryName") == map.end() ||
        map.find("TargetDirectoryPath") == map.end() ||
        map.find("TargetDirectoryName") == map.end())
    {
        return false;
    }
    return true;
}

std::unique_ptr<AbstractTask> TransferDirectoryTaskFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto result = std::make_unique<TransferDirectoryTask>();
    result->setSettings(settings);
    return result;
}

}
}
