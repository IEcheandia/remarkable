#include "Scheduler/configurationMonitor.h"
#include "Scheduler/signalManagerLoader.h"

#include <filesystem>
namespace fs = std::filesystem;
#include "Poco/Delegate.h"

#include <fstream>

namespace precitec
{
namespace scheduler
{

ConfigurationMonitor::ConfigurationMonitor(TaskScheduler &taskScheduler, const std::string &monitoringFolder,
                                           const std::string &fileName)
    : m_taskScheduler(taskScheduler), m_monitoringFolder(monitoringFolder), m_fileName(fileName)
{
}

void ConfigurationMonitor::monitor()
{
    if (fs::exists(m_monitoringFolder))
    {
        try
        {
            m_watcher = std::make_unique<Poco::DirectoryWatcher>(m_monitoringFolder,
                                                                Poco::DirectoryWatcher::DW_ITEM_MODIFIED,
                                                                Poco::DirectoryWatcher::DW_DEFAULT_SCAN_INTERVAL);
            m_watcher->itemModified += Poco::delegate(this, &ConfigurationMonitor::onFileModified);
        }
        catch (...)
        {
            system::logExcpetion(__FUNCTION__, std::current_exception());
        }
    }
}

void ConfigurationMonitor::onFileModified(const Poco::DirectoryWatcher::DirectoryEvent &changeEvent)
{
    std::string fullFileName = m_monitoringFolder + "/" + m_fileName;
    if (changeEvent.item.path() == fullFileName)
    {
        std::ifstream jsonSettingFile(fullFileName);
        if (jsonSettingFile.fail()) {
            m_taskScheduler.rewriteSignalManagers({});
            return;
        }
        auto newSignalManagers = SignalManagerLoader::load(jsonSettingFile);
        m_taskScheduler.rewriteSignalManagers(newSignalManagers);
    }
}

}
}