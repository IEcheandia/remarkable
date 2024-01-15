#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;
namespace precitec
{
namespace scheduler
{

class TimeFolderCleaner
{
public:
    TimeFolderCleaner() = default;
    ~TimeFolderCleaner() = default;

    void setFolder(const std::string &fullName);
    void setTimeToLive(std::int32_t timeToLeaveSecs);

    bool cleanOldSubFolders();

    static bool isTimeToDie(const fs::directory_entry &entry, std::int32_t timeToLiveSecs);
private:

    std::string m_fullFolderName;
    std::int32_t m_timeToLiveSecs;
};

}
}