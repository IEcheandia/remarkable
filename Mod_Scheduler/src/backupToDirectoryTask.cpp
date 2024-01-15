#include "Scheduler/backupToDirectoryTask.h"
#include "Scheduler/backupToDirectoryTaskProcess.h"

#include "json.hpp"

namespace precitec
{
namespace scheduler
{
void BackupToDirectoryTask::run()
{
    if (!checkSettings())
    {
        log(false);
        return;
    }
    BackupToDirectoryTaskProcess process;
    process.setInfo(settings());
    log(process.run());
}

const std::string &BackupToDirectoryTask::name() const
{
    return mName;
}

const std::string BackupToDirectoryTask::mName = "BackupToDirectoryTask";

AbstractTask *BackupToDirectoryTask::clone() const
{
    return new BackupToDirectoryTask(static_cast<BackupToDirectoryTask const &>(*this));
}

bool BackupToDirectoryTask::checkSettings() const
{
    const auto &map = settings();
    const auto it = map.find("backupPath");
    return it != map.end() && !it->second.empty();
}

std::unique_ptr<AbstractTask> BackupToDirectoryTaskFactory::make(const std::map<std::string, std::string> &settings) const
{
    auto result = std::make_unique<BackupToDirectoryTask>();
    result->setSettings(settings);
    return result;
}

}
}
