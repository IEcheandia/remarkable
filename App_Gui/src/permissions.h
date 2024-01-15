#pragma once

#include <QtGlobal>
#include <QtCore/qobjectdefs.h>

namespace precitec
{
namespace gui
{

Q_NAMESPACE

/**
 * The Permissions exported to QtQuick as precitec.gui.App.
 *
 * To bind the visibility of an Item to whether the current User has a given Permission
 * use the following way:
 * @code
 * import QtQuick 2.5
 * import precitec.gui.components.userManagement 1.0
 * import precitec.gui 1.0
 *
 * Item {
 *     visible: UserManagement.currentUser && UserManagement.hasPermission(App.ResetSystemStatus)
 * }
 * @endcode
 *
 * The binding to currentUser is required to have the property evaluated whenever the currentUser changes.
 * As currentUser is a pointer property it always evaluates to @c true if a User is logged in.
 **/
enum class Permission
{
    /**
     * Permission whether the User is allowed to reset the system status when in Not Ready state.
     **/
    ResetSystemStatus = 1000,
    /**
     * Permission whether the User is allowed to shutdown/reboot the system.
     **/
    ShutdownSystem,
    /**
     * Permission whether the User is allowed to stop/restart the processes.
     * The system is not physically shutdown/rebooted unlike the ShutdownSystem permission.
     **/
    StopAllProcesses,
    ViewGrabberDeviceConfig,
    ViewCalibrationDeviceConfig,
    ViewVideoRecorderDeviceConfig,
    ViewWeldHeadDeviceConfig,
    ViewServiceDeviceConfig,
    ViewInspectionDeviceConfig,
    ViewWorkflowDeviceConfig,
    EditGrabberDeviceConfig,
    EditCalibrationDeviceConfig,
    EditVideoRecorderDeviceConfig,
    EditWeldHeadDeviceConfig,
    EditServiceDeviceConfig,
    EditInspectionDeviceConfig,
    EditWorkflowDeviceConfig,
    EditResultsConfig,
    /**
     * Permission whether the User is allowed to enter the hardware and product wizard.
     **/
    RunHardwareAndProductWizard,
    /**
     * Permission whether the User is allowed to mount portable devices
     **/
    MountPortableDevices,
    /**
     * Permission whether the User is allowed to perform a backup
     **/
    PerformBackup,
    /**
     * Permission whether the User is allowed to perform a restore.
     * Also requires ShutdownSystem
     **/
    PerformRestore,
    /**
     * Permission whether the User is allowed to perform updates.
     **/
    PerformUpdate,
    /**
     * Permission whether the User is allowed to set the tool center poin.
     **/
    SetToolCenterPoint,
    /**
     * Permission whether the User is allowed to download videos.
     * Also requires MountPortableDevices
     **/
    DownloadVideo,
    /**
     * Permission whether the User is allowed to view information about the Ethercat slaves.
     **/
    ViewEthercat,
    /**
     * Permission whether the User is allowed to modify the state of Ethercat slaves.
     **/
    ModifyEthercat,
    /**
     * Permission whether the User is allowed to view debug messages
     **/
    ViewDebugLogMessages,
    /**
     * Permission whether the User is allowed to clear log messages
     **/
    ClearLogMessages,
    /**
     * Permission whether the User is allowed to view the user log
     **/
    ViewUserLog,
    /**
     * Permissions to emulate user level of filter parameters
     **/
    ViewFilterParameterAdmin,
    ViewFilterParameterSuperUser,
    ViewFilterParameterGroupLeader,
    ViewFilterParameterOperator,
    ViewStorageDeviceConfig,
    EditStorageDeviceConfig,
    ViewIDMDeviceConfig,
    EditIDMDeviceConfig,
    /**
     * Permission to trigger the Hardware limits of y-Axis from Gui
     **/
    AxisTriggerHardwareLimits,
    /**
     * Permission to view the SpsSimulation page with the verification step
     **/
    ViewSpsSimulation,
    /**
     * Permission to configure localization settings.
     **/
    Localization,
    /**
     * Permission whether the User is allowed to upload videos.
     * Also requires MountPortableDevices
     **/
    UploadVideo,
    /**
     * Permission whether the User is allowed to change network devices, e.g. ip address.
     **/
    ConfigureNetworkDevices,
    /**
     * Permission whether the User is allowed to configure the Ups
     **/
    ConfigureUps,
    /**
     * Permission whether the User is allowed to create and restore hardware configuration backups
     **/
    BackupRestoreHardwareConfiguration,
    /**
     * Permission whether the User is allowed to export graphs
     **/
    ExportGraph,
    /**
     * Permission whether the User is allowed to export result data to xlsx
     **/
    ExportResults,
    ViewGuiDeviceConfig,
    EditGuiDeviceConfig,
    ViewSSHConfiguration,
    ImportSSHAuthorizedKey,
    RemoveSSHAuthorizedKey,
    /**
     * Permission to start remote desktop functionality
     **/
    RemoteDesktop,
    /**
     * Permission to configure screen (e.g. resolution).
     **/
    ConfigureScreen,
    /**
     * Permission whether the User is allowed to upload results.
     * Also requires MountPortableDevices
     **/
    UploadResults,
    /**
     * Permission whether the User is allowed to delede videos.
     **/
    DeleteVideo,
    /**
     * Permission whether the User is allowed to import legacy product xml from Weldmaster Full
     **/
    ImportProducts,
    /**
     * Permission whether the User is allowed to set a reference image
     **/
    EditReferenceImage,
    /**
     * Permission whether the User is allowed to create or change a graph with graph editor
     **/
    EditGraphsWithGrapheditor,
    /**
     * Permission whether the User is allowed to view and edit the task scheduler
     **/
    TaskScheduler,
    /**
     * Permission whether the User is allowed to extract a template from image and save it
     **/
    SaveTemplate,
    /**
     * Permission whether the User is allowed to open system network configuration.
     **/
    OpenSystemNetworkConfiguration,

    // following are the permissions from HeadMonitor - Weldmaster Permission need to go above
    // HeadMonitor registers the Permissions by itself. This is only syncing the enum
    HeadMonitorResetPermissions = 10000,
    HeadMonitorAddLaserHead,
    HeadMonitorDeleteLaserHead,
    HeadMonitorEditLaserHead,
    HeadMonitorEditMaxLimits,
    HeadMonitorEditService

};
Q_ENUM_NS(Permission)

}
}
