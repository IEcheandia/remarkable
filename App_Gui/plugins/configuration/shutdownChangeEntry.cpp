#include "shutdownChangeEntry.h"

namespace precitec
{
namespace gui
{

ShutdownChangeEntry::ShutdownChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}

ShutdownChangeEntry::ShutdownChangeEntry(Operation operation, QObject *parent)
    : components::userLog::Change(parent)
{
    switch (operation)
    {
    case Operation::Shutdown:
        setMessage(tr("System shutdown"));
        break;
    case Operation::Restart:
        setMessage(tr("System restart"));
        break;
    case Operation::StopProcesses:
        setMessage(tr("Stop of all processes"));
        break;
    case Operation::RestartProcesses:
        setMessage(tr("Restart of all processes"));
        break;
    default:
        Q_UNREACHABLE();
    }
}

ShutdownChangeEntry::~ShutdownChangeEntry() = default;

QJsonObject ShutdownChangeEntry::data() const
{
    return {};
}

void ShutdownChangeEntry::initFromJson(const QJsonObject &data)
{
    Q_UNUSED(data)
}

}
}
