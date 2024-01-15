#include "Scheduler/transferMetaDataTask.h"
#include "Scheduler/eventTrigger.h"
#include "Scheduler/manualTrigger.h"
#include "Scheduler/metaDataHelper.h"

#include "Scheduler/transferFileTaskProcess.h"

namespace precitec
{
namespace scheduler
{

const std::string TransferMetaDataTask::mName = "TransferMetaDataTask";

std::set<std::pair<std::string, std::optional<precitec::interface::SchedulerEvents>>> TransferMetaDataTask::onlySupportedTriggers() const
{
    return {
        {EventTrigger::mName, precitec::interface::SchedulerEvents::ProductInstanceResultsStored},
        {ManualTrigger::mName, precitec::interface::SchedulerEvents::ProductManualTrigger}
    };
}

void TransferMetaDataTask::run()
{
    const auto &metaData = signalInfoMetaData();
    if (auto it = metaData.find(std::string{"path"}); it != metaData.end() && !it->second.empty())
    {
        auto settings = this->settings();
        settings[std::string{"SourceFileName"}] = "metadata.json";
        settings[std::string{"SourceDirectoryPath"}] = it->second;

        MetaDataHelper helper{it->second +  "metadata.json"};
        helper.updateTargetPath(settings);

        if (auto it = settings.find("TargetFileName"); it != settings.end() && !it->second.empty())
        {
            helper.updateString(it->second);
        }
        else
        {
            settings[std::string{"TargetFileName"}] = "metadata.json";
        }

        setSettings(settings);
    }
    if (!checkSettings())
    {
        log(false);
        return;
    }
    TransferFileTaskProcess process;
    process.setInfo(settings());
    log(process.run());
}

const std::string &TransferMetaDataTask::name() const
{
    return mName;
}

AbstractTask *TransferMetaDataTask::clone() const
{
    return new TransferMetaDataTask(static_cast<TransferMetaDataTask const &>(*this));
}

bool TransferMetaDataTask::checkSettings() const
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

std::unique_ptr<AbstractTask> TransferMetaDataTaskFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto result = std::make_unique<TransferMetaDataTask>();
    result->setSettings(settings);
    return result;
}

}
}
