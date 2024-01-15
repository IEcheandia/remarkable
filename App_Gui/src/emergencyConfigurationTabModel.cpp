#include "emergencyConfigurationTabModel.h"
#include "permissions.h"

#include <precitec/userManagement.h>

using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

EmergencyConfigurationTabModel::EmergencyConfigurationTabModel(QObject *parent)
    : SecurityTabModel({
        {tr("Shutdown"), QStringLiteral("system-shutdown"), {int(Permission::ShutdownSystem), int(Permission::StopAllProcesses)}},
        {tr("Backup"), QStringLiteral("document-save-all"), {int(Permission::PerformBackup)}}
    }, parent)
{
}

EmergencyConfigurationTabModel::~EmergencyConfigurationTabModel() = default;

}
}

