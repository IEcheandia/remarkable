#include "common/connectionConfiguration.h"

namespace precitec
{
namespace interface
{

ConnectionConfiguration::ConnectionConfiguration()
    : ConnectionConfiguration(std::string(getenv("WM_STATION_NAME") ? getenv("WM_STATION_NAME") : ""))
{
}

ConnectionConfiguration::ConnectionConfiguration(const std::string &stationName)
    : m_stationName(stationName)
    , m_mutex(m_stationName + "WMConnConfMutex")
{
}

ConnectionConfiguration &ConnectionConfiguration::instance()
{
    static ConnectionConfiguration s_config;
    return s_config;
}

const std::string &ConnectionConfiguration::stationName() const
{
    return m_stationName;
}

}
}
