#include "shutdownService.h"
#include "shutdownChangeEntry.h"

#include <precitec/userManagement.h>
#include <precitec/userLog.h>

#include "common/connectionConfiguration.h"

#include <signal.h>

using precitec::gui::components::user::UserManagement;
using precitec::gui::components::userLog::UserLog;
using precitec::interface::ConnectionConfiguration;

namespace precitec
{
namespace gui
{

ShutdownService::ShutdownService(QObject *parent)
    : QObject(parent)
{
}

ShutdownService::~ShutdownService() = default;

void ShutdownService::shutdownSystem()
{
    if (!UserManagement::instance()->hasPermission(shutdownSystemPermission()))
    {
        return;
    }
    // stop all simulation and shutdown real system
    UserLog::instance()->addChange(new ShutdownChangeEntry{ShutdownChangeEntry::Operation::Shutdown});
    sendRequest(Request::StopAll, Station::Simulation);
    sendRequest(Request::ShutdownSystem, Station::Hardware);
}

void ShutdownService::restartSystem()
{
    if (!UserManagement::instance()->hasPermission(restartSystemPermission()))
    {
        return;
    }
    // stop all simulation and restart real system
    UserLog::instance()->addChange(new ShutdownChangeEntry{ShutdownChangeEntry::Operation::Restart});
    sendRequest(Request::StopAll, Station::Simulation);
    sendRequest(Request::RestartSystem, Station::Hardware);
}

void ShutdownService::stopAllProcesses()
{
    if (!UserManagement::instance()->hasPermission(stopAllProcessesPermission()))
    {
        return;
    }
    // stop all in both systems
    UserLog::instance()->addChange(new ShutdownChangeEntry{ShutdownChangeEntry::Operation::StopProcesses});
    sendRequest(Request::StopAll, Station::Simulation);
    sendRequest(Request::StopAll, Station::Hardware);
}

void ShutdownService::sendRequest(Request request, Station station)
{
    static std::map<Request, int> s_requests{
        {Request::StopAll, SIGTERM},
        {Request::ShutdownSystem, SIGUSR1},
        {Request::RestartSystem, SIGUSR2}
    };
    auto it = s_requests.find(request);
    if (it == s_requests.end())
    {
        return;
    }
    const int pid = getPid(station);
    if (pid == 0)
    {
        return;
    }
    ::kill(pid, it->second);
}

int ShutdownService::getPid(precitec::gui::ShutdownService::Station station) const
{
    try
    {
        ConnectionConfiguration config(getName(station));
        return config.getInt(std::string("ConnectServerPid"), 0);
    }
    catch (const std::exception& e)
    {
        // Implementation tries to open shared memory for some stations (e.g. simulation)
        // which throws in case it doesn't exist, do not crash in this case
        return 0;
    }
}

std::string ShutdownService::getName(precitec::gui::ShutdownService::Station station) const
{
    switch (station)
    {
    case Station::Hardware:
        return std::string(getenv("WM_STATION_NAME"));
    case Station::Simulation:
        return std::string("SIMULATION");
    default:
        Q_UNREACHABLE();
    }
}

void ShutdownService::setShutdownSystemPermission(int permission)
{
    if (m_shutdownSystemPermission == permission)
    {
        return;
    }
    m_shutdownSystemPermission = permission;
    emit shutdownSystemPermissionChanged();
}

void ShutdownService::setRestartSystemPermission(int permission)
{
    if (m_restartSystemPermission == permission)
    {
        return;
    }
    m_restartSystemPermission = permission;
    emit restartSystemPermissionChanged();
}

void ShutdownService::setStopAllProcessesPermission(int permission)
{
    if (m_stopAllProcessesPermission == permission)
    {
        return;
    }
    m_stopAllProcessesPermission = permission;
    emit stopAllProcessesPermissionChanged();
}

}
}
