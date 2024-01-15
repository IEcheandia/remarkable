#pragma once

#include "Poco/DirectoryWatcher.h"

#include "taskScheduler.h"

namespace precitec
{
namespace scheduler
{

class ConfigurationMonitor
{
public:
    ConfigurationMonitor(TaskScheduler &taskScheduler, const std::string &monitoringFolder,
                         const std::string &fileName);
    void monitor();
private:
    void onFileModified(const Poco::DirectoryWatcher::DirectoryEvent& changeEvent);

    TaskScheduler &m_taskScheduler;
    const std::string m_monitoringFolder;

    std::unique_ptr<Poco::DirectoryWatcher> m_watcher = nullptr;
    const std::string m_fileName;
};

}
}