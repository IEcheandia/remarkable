#include "Scheduler/backupToRemoteTask.h"
#include "Scheduler/backupToRemoteTaskProcess.h"

#include "json.hpp"

namespace precitec
{
namespace scheduler
{
void BackupToRemoteTask::run()
{
    if (!checkSettings())
    {
        log(false);
        return;
    }
    BackupToRemoteTaskProcess process;
    process.setInfo(settings());
    log(process.run());
}

const std::string &BackupToRemoteTask::name() const
{
    return mName;
}

const std::string BackupToRemoteTask::mName = "BackupToRemoteTask";

AbstractTask *BackupToRemoteTask::clone() const
{
    return new BackupToRemoteTask(static_cast<BackupToRemoteTask const &>(*this));
}

bool BackupToRemoteTask::checkSettings() const
{
    const auto &map = settings();
    if (map.find("TargetIpAddress") == map.end() ||
        map.find("TargetUserName") == map.end() ||
        map.find("TargetPassword") == map.end() ||
        map.find("TargetDirectoryPath") == map.end())
    {
        return false;
    }
    return true;
}

std::unique_ptr<AbstractTask> BackupToRemoteTaskFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto result = std::make_unique<BackupToRemoteTask>();
    result->setSettings(settings);
    return result;
}

}
}

