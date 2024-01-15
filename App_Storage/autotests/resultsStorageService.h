#pragma once

namespace precitec
{
namespace storage
{

/**
 * Mock class
 **/
class ResultsStorageService
{
public:
    bool isShutdown() const
    {
        return m_shutdown;
    }
    void setShutdown(bool shutdown)
    {
        m_shutdown = shutdown;
    }
    void setEnabled(bool enabled)
    {
        m_enabled = enabled;
    }
    bool isEnabled() const
    {
        return m_enabled;
    }
    void setMaxRelativeDiskUsage(double diskUsage)
    {
        m_maxRelativeDiskUsage = qBound(0.0, diskUsage, 1.0);
    }
    double maxRelativeDiskUsage() const
    {
        return m_maxRelativeDiskUsage;
    }
    void setMaxCacheEntries(std::size_t cacheEntries)
    {
        m_maxCacheEntries = qBound(std::size_t{0}, cacheEntries, std::size_t{999999});
    }
    std::size_t maxCacheEntries() const
    {
        return m_maxCacheEntries;
    }

private:
    bool m_enabled = true;
    bool m_shutdown = false;
    double m_maxRelativeDiskUsage = 0.9;
    std::size_t m_maxCacheEntries = 500;
};

}
}
