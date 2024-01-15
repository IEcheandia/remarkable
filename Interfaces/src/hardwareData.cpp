#include "filter/hardwareData.h"
#include <module/moduleLogger.h>

namespace precitec
{
namespace analyzer
{

std::optional<double> HardwareData::get(unsigned int appId, const std::string& parameterName) const
{
    std::optional<double> value;
    const auto itAppName = m_parameterMap.find(appId);
    if (itAppName != m_parameterMap.end())
    {
        const auto itParameterName = itAppName->second.find(parameterName);
        if (itParameterName != itAppName->second.end())
        {
            value = itParameterName->second;
        }
    }
   return value;
}

void HardwareData::set(unsigned int appId, const std::string& parameterName, double value)
{
    m_parameterMap[appId].insert(std::make_pair(parameterName, value));
}

void HardwareData::remove(unsigned int appId, const std::string& parameterName)
{
    const auto itAppName = m_parameterMap.find(appId);
    if (itAppName != m_parameterMap.end())
    {
        itAppName->second.erase(parameterName);
    }
}

}
}
