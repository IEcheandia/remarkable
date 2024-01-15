#include "analyzer/centralDeviceManager.h"

namespace precitec
{
namespace analyzer
{

const Poco::UUID g_oCameraID( "1f50352e-a92a-4521-b184-e16809345026" );

CentralDeviceManager::CentralDeviceManager(bool)
{
}

void CentralDeviceManager::force(const Poco::UUID &p_rDeviceId, SmpKeyValue p_pKeyValue)
{
}

void CentralDeviceManager::lock(bool p_oLocked)
{
}

SmpKeyValue CentralDeviceManager::get( const Poco::UUID &p_rDeviceId, std::string key)
{
    return SmpKeyValue{};
}

}
}
