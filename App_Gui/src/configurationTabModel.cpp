#include "configurationTabModel.h"
#include "permissions.h"

#include "common/systemConfiguration.h"

#include <precitec/userManagement.h>

using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

ConfigurationTabModel::ConfigurationTabModel(QObject *parent)
    : SecurityTabModel({
        {tr("Shutdown"), QStringLiteral("system-shutdown"), {int(Permission::ShutdownSystem), int(Permission::StopAllProcesses)}},
        {tr("Change Password"), QStringLiteral("user-properties"), {}},
        {tr("User Management"), QStringLiteral("system-users"), {int(UserManagement::UserManagementPermission::AddUser),
                                 int(UserManagement::UserManagementPermission::ChangeUser),
                                 int(UserManagement::UserManagementPermission::DeleteUser)}},
        {tr("Roles"), QStringLiteral("emblem-locked"), {int(UserManagement::UserManagementPermission::AddRole),
                                 int(UserManagement::UserManagementPermission::ChangeRole),
                                 int(UserManagement::UserManagementPermission::DeleteRole)}},
        {tr("Devices"), QStringLiteral("configure-devices"), {
            int(Permission::ViewGrabberDeviceConfig),
            int(Permission::ViewCalibrationDeviceConfig),
            int(Permission::ViewWeldHeadDeviceConfig),
            int(Permission::ViewServiceDeviceConfig),
            int(Permission::ViewWorkflowDeviceConfig),
            int(Permission::ViewInspectionDeviceConfig),
            int(Permission::ViewStorageDeviceConfig),
            int(Permission::ViewIDMDeviceConfig)
        }},
        {tr("Head Monitor"), QStringLiteral("laserhead"), {
            int(Permission::HeadMonitorAddLaserHead),
            int(Permission::HeadMonitorDeleteLaserHead),
            int(Permission::HeadMonitorEditLaserHead),
        }, interface::SystemConfiguration::instance().get(interface::SystemConfiguration::BooleanKey::HeadMonitorGatewayEnable)},
        {tr("Results"), QStringLiteral("configure-results"), {int(Permission::EditResultsConfig)}},
        {tr("Errors"), QStringLiteral("wizard-sumerror"), {int(Permission::EditResultsConfig)}},
        {tr("Sensors"), QStringLiteral("configure-results"), {int(Permission::EditResultsConfig)}},
        {tr("Backup"), QStringLiteral("document-save-all"), {int(Permission::PerformBackup)}},
        {tr("Restore"), QStringLiteral("edit-redo"), {int(Permission::PerformRestore)}},
        {tr("Updates"), QStringLiteral("update-none"), {int(Permission::PerformUpdate)}},
        {tr("Open Source"), QStringLiteral("license"), {}},
        {tr("I/O"), QStringLiteral("view-input-output"), {int(Permission::ViewEthercat)}},
        {tr("User log"), QStringLiteral("view-list-text"), {int(Permission::ViewUserLog)}},
        {tr("Localization"), QStringLiteral("configure-localization"), {int(Permission::Localization)}},
        {tr("Scheduler"), QStringLiteral("clock"), {int(Permission::TaskScheduler)}},
        {tr("System"), QStringLiteral("gtk-preferences"), {int(Permission::ConfigureNetworkDevices), int(Permission::ConfigureUps), int(Permission::BackupRestoreHardwareConfiguration), int(Permission::ViewSSHConfiguration)}},
        {tr("Remote desktop"), QStringLiteral("network-wired"), {int(Permission::RemoteDesktop)}}
    }, parent)
{
}

ConfigurationTabModel::~ConfigurationTabModel() = default;

}
}
