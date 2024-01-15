#include "Scheduler/deleteOldBackupsTask.h"

#include "json.hpp"

#include "Scheduler/deleteOldBackupsTaskProcess.h"

using Json = nlohmann::json;

namespace precitec
{
namespace scheduler
{

const std::string DeleteOldBackupsTask::mName = "DeleteOldBackupsTask";

void DeleteOldBackupsTask::run()
{

    if (!checkSettings())
    {
        log(false);
        return;
    }

    DeleteOldBackupsTaskProcess process;
    process.setInfo(settings());
    log(process.run());
}

const std::string &DeleteOldBackupsTask::name() const
{
    return mName;
}

AbstractTask *DeleteOldBackupsTask::clone() const
{
    return new DeleteOldBackupsTask(static_cast<DeleteOldBackupsTask const &>(*this));
}

bool DeleteOldBackupsTask::checkSettings() const
{
    const auto &map = settings();
    if (map.find("backupPath") == map.end() ||
        map.find("TimeToLiveDays") == map.end())
    {
        return false;
    }
    return true;
}

std::unique_ptr<AbstractTask> DeleteOldBackupsTaskFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto result = std::make_unique<DeleteOldBackupsTask>();
    result->setSettings(settings);
    return result;
}

}
}
