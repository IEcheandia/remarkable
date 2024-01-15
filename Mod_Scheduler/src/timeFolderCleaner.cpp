#include "Scheduler/timeFolderCleaner.h"

#include <filesystem>
#include <chrono>

#include<string>

namespace fs = std::filesystem;
using namespace std::chrono_literals;
using namespace std::chrono;

namespace precitec
{
namespace scheduler
{

void TimeFolderCleaner::setFolder(const std::string &fullName)
{
    m_fullFolderName = fullName;
}

void TimeFolderCleaner::setTimeToLive(std::int32_t timeToLiveSecs)
{
    m_timeToLiveSecs = timeToLiveSecs;
}

bool TimeFolderCleaner::cleanOldSubFolders()
{
    if (fs::exists(m_fullFolderName))
    {
        for (const auto &item: fs::directory_iterator(m_fullFolderName))
        {
            if (isTimeToDie(item, m_timeToLiveSecs) && item.is_directory())
            {
                fs::remove_all(item.path()); // deletes one or more file/directories recursively.
            }
        }
        return true;
    }
    return false;
}

bool TimeFolderCleaner::isTimeToDie(const fs::directory_entry &entry, std::int32_t timeToLiveSecs)
{
    auto now = fs::file_time_type::clock::now();
    auto lastWriteTime = fs::last_write_time(entry.path());
    return duration_cast<seconds>(now - lastWriteTime).count() > timeToLiveSecs ;
}

}
}