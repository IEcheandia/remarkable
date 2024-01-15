#include "Scheduler/taskFactory.h"

#include "Scheduler/testTask.h"
#include "Scheduler/backupToDirectoryTask.h"
#include "Scheduler/backupToRemoteTask.h"
#include "Scheduler/transferFileTask.h"
#include "Scheduler/transferDirectoryTask.h"
#include "Scheduler/deleteOldBackupsTask.h"
#include "Scheduler/resultExcelFileFromProductInstanceTask.h"
#include "Scheduler/transferMetaDataTask.h"
#include "Scheduler/exportProductTask.h"

namespace precitec
{
namespace scheduler
{

TaskFactory::TaskFactory()
{
    m_taskFactories[TestTask::mName] = std::make_unique<TestTaskFactory>();
    m_taskFactories[BackupToDirectoryTask::mName] = std::make_unique<BackupToDirectoryTaskFactory>();
    m_taskFactories[BackupToRemoteTask::mName] = std::make_unique<BackupToRemoteTaskFactory>();
    m_taskFactories[TransferFileTask::mName] = std::make_unique<TransferFileTaskFactory>();
    m_taskFactories[TransferDirectoryTask::mName] = std::make_unique<TransferDirectoryTaskFactory>();
    m_taskFactories[DeleteOldBackupsTask::mName] = std::make_unique<DeleteOldBackupsTaskFactory>();
    m_taskFactories[ResultExcelFileFromProductInstanceTask::mName] = std::make_unique<ResultExcelFileFromProductInstanceTaskFactory>();
    m_taskFactories[TransferMetaDataTask::mName] = std::make_unique<TransferMetaDataTaskFactory>();
    m_taskFactories[ExportProductTask::mName] = std::make_unique<ExportProductTaskFactory>();
}

std::unique_ptr<AbstractTask> TaskFactory::make(const nlohmann::json &jsonObject)
{
    std::map<std::string, std::string> settings;
    try
    {
        settings = jsonObject.at("Settings").get<std::map<std::string, std::string>>();
    }
    catch (std::exception const &ex)
    {
        wmLog(eDebug, "Bad json task format\n");
        wmLog(eDebug, "Bad format: %s\n", ex.what());
        return {};
    }

    std::string name;
    try
    {
        name = jsonObject.at("Name").get<std::string>();
    }
    catch (std::exception const &ex)
    {
        wmLog(eDebug, "Bad json task format\n");
        wmLog(eDebug, "Bad format: %s\n", ex.what());
        return {};
    }

    std::unique_ptr<AbstractTask> abstractTask = make(name, settings);
    if (!abstractTask)
    {
        wmLog(eDebug, "Bad json task format\n");
        wmLog(eDebug, "Bad format: there is no task with name %s\n", name);
        return {};
    }

    return abstractTask;
}

std::unique_ptr<AbstractTask> TaskFactory::make(const std::string &taskName, const std::map<std::string, std::string> &settings)
{
    if (m_taskFactories.find(taskName) == m_taskFactories.end())
    {
        wmLog(eError, "not found %s", taskName);
        return nullptr;
    }

    return m_taskFactories[taskName]->make(settings);
}

}
}
