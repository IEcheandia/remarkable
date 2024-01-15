#include "Scheduler/resultExcelFileFromProductInstanceTask.h"
#include "Scheduler/resultExcelFileFromProductInstanceTaskProcess.h"
#include "Scheduler/eventTrigger.h"
#include "Scheduler/manualTrigger.h"
#include "Scheduler/metaDataHelper.h"
#include "json.hpp"

using Json = nlohmann::json;

namespace precitec
{
namespace scheduler
{

const std::string ResultExcelFileFromProductInstanceTask::mName = "ResultExcelFileFromProductInstanceTask";

std::set<std::pair<std::string, std::optional<precitec::interface::SchedulerEvents>>> ResultExcelFileFromProductInstanceTask::onlySupportedTriggers() const
{
    return {{EventTrigger::mName, precitec::interface::SchedulerEvents::ProductInstanceResultsStored}, 
            {ManualTrigger::mName, precitec::interface::SchedulerEvents::ProductManualTrigger}};
}

void ResultExcelFileFromProductInstanceTask::run()
{
    const auto &metaData = signalInfoMetaData();
    if (auto it = metaData.find(std::string{"path"}); it != metaData.end() && !it->second.empty())
    {
        auto settings = this->settings();
        settings[std::string{"ProductInstanceDirectory"}] = it->second;

        MetaDataHelper helper{it->second +  "metadata.json"};
        helper.updateTargetPath(settings);
        setSettings(settings);
    }
    if (!checkSettings())
    {
        log(false);
        return;
    }

    ResultExcelFileFromProductInstanceTaskProcess process;
    process.setInfo(settings());
    log(process.run());
}

const std::string &ResultExcelFileFromProductInstanceTask::name() const
{
    return mName;
}

AbstractTask *ResultExcelFileFromProductInstanceTask::clone() const
{
    return new ResultExcelFileFromProductInstanceTask(static_cast<ResultExcelFileFromProductInstanceTask const &>(*this));
}

bool ResultExcelFileFromProductInstanceTask::checkSettings() const
{
    const auto &map = settings();
    if (map.find("TargetIpAddress") == map.end() ||
        map.find("TargetUserName") == map.end() ||
        map.find("TargetPassword") == map.end() ||
        map.find("TargetDirectoryPath") == map.end() ||
        map.find("ProductInstanceDirectory") == map.end())
    {
        return false;
    }
    return true;

}

std::unique_ptr<AbstractTask> ResultExcelFileFromProductInstanceTaskFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto result = std::make_unique<ResultExcelFileFromProductInstanceTask>();
    result->setSettings(settings);
    return result;
}

}
}
