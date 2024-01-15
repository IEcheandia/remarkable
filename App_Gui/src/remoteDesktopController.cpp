#include "remoteDesktopController.h"
#include "permissions.h"

#include "plugins/general/guiConfiguration.h"
#include <QProcess>

#include <precitec/userManagement.h>

#include <signal.h>

namespace precitec
{
namespace gui
{

Process::Process(QObject *parent)
    : QProcess(parent)
{
}

Process::~Process() = default;

void Process::setupChildProcess()
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    sigprocmask(SIG_UNBLOCK, &mask, nullptr);
}


using components::user::UserManagement;

RemoteDesktopController::RemoteDesktopController(QObject *parent)
    : QObject(parent)
    , m_process(new Process{this})
{
    m_process->setProgram(QStringLiteral("/usr/bin/krfb"));
    m_process->setArguments({QStringLiteral("--nodialog")});
    m_process->setProcessChannelMode(QProcess::ForwardedErrorChannel);
    m_process->setReadChannel(QProcess::StandardOutput);

    connect(m_process, &QProcess::stateChanged, this, &RemoteDesktopController::enabledChanged);

    if (GuiConfiguration::instance()->remoteDesktopOnStartup())
    {
        performStart();
    }
}

RemoteDesktopController::~RemoteDesktopController()
{
    if (m_process->state() == QProcess::Running)
    {
        m_process->terminate();
        m_process->waitForFinished();
    }
}

bool RemoteDesktopController::isEnabled() const
{
    return m_process->state() == QProcess::Running;
}

void RemoteDesktopController::start()
{
    if (!UserManagement::instance()->hasPermission(int(Permission::RemoteDesktop)))
    {
        return;
    }
    performStart();
}

void RemoteDesktopController::performStart()
{
    if (m_process->state() == QProcess::NotRunning)
    {
        m_process->start();
    }
}

void RemoteDesktopController::stop()
{
    if (!UserManagement::instance()->hasPermission(int(Permission::RemoteDesktop)))
    {
        return;
    }
    if (m_process->state() != QProcess::NotRunning)
    {
        m_process->terminate();
    }
}


}
}
