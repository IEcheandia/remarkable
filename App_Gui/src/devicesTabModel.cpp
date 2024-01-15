#include "devicesTabModel.h"
#include "permissions.h"

#include <precitec/userManagement.h>

#include <common/systemConfiguration.h>

using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

DevicesTabModel::DevicesTabModel(QObject *parent)
    : SecurityTabModel({
        {tr("Gui"), QString(), {int(Permission::ViewGuiDeviceConfig)}},
        {tr("Grabber"), QString(), {int(Permission::ViewGrabberDeviceConfig)}},
        {tr("Calibration"), QString(), {int(Permission::ViewCalibrationDeviceConfig)}},
        {tr("Video Recorder"), QString(), {int(Permission::ViewVideoRecorderDeviceConfig)}},
        {tr("Weld Head"), QString(), {int(Permission::ViewWeldHeadDeviceConfig)}},
        {tr("Service"), QString(), {int(Permission::ViewServiceDeviceConfig)}},
        {tr("Workflow"), QString(), {int(Permission::ViewWorkflowDeviceConfig)}},
        {tr("InspectionControl"), QString(), {int(Permission::ViewInspectionDeviceConfig)}},
        {tr("Storage"), QString(), {int(Permission::ViewStorageDeviceConfig)}},
        {tr("IDM"), QString(), {int(Permission::ViewIDMDeviceConfig)}}
    }, parent)
    , m_editRoles({
                  int(Permission::EditGuiDeviceConfig),
                  int(Permission::EditGrabberDeviceConfig),
                  int(Permission::EditCalibrationDeviceConfig),
                  int(Permission::EditVideoRecorderDeviceConfig),
                  int(Permission::EditWeldHeadDeviceConfig),
                  int(Permission::EditServiceDeviceConfig),
                  int(Permission::EditWorkflowDeviceConfig),
                  int(Permission::EditInspectionDeviceConfig),
                  int(Permission::EditStorageDeviceConfig),
                  int(Permission::EditIDMDeviceConfig)
 
    }),
    m_available({
                  true,
                  interface::SystemConfiguration::instance().get(interface::SystemConfiguration::BooleanKey::HardwareCameraEnabled),
                  true,
                  true,
                  true,
                  true,
                  true,
                  true,
                  true,
                  (interface::SystemConfiguration::instance().get(interface::SystemConfiguration::BooleanKey::IDM_Device1Enable) || interface::SystemConfiguration::instance().get(interface::SystemConfiguration::BooleanKey::CLS2_Device1Enable))
        })
{
    connect(UserManagement::instance(), &UserManagement::currentUserChanged, this,
        [this] {
            emit dataChanged(index(0, 0), index(m_editRoles.size() - 1, 0), {Qt::UserRole + 1});
        }
    );
}

DevicesTabModel::~DevicesTabModel() = default;

QHash<int, QByteArray> DevicesTabModel::roleNames() const
{
    auto roles = SecurityTabModel::roleNames();
    roles[Qt::UserRole + 1] = QByteArrayLiteral("canEdit");
    roles[Qt::UserRole + 2] = QByteArrayLiteral("available");
    return roles;
}

QVariant DevicesTabModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    if (role == Qt::UserRole + 1)
    {
        return UserManagement::instance()->hasPermission(m_editRoles.at(index.row()));
    }
    if (role == Qt::UserRole + 2)
    {
        return m_available.at(index.row());
    }
    return SecurityTabModel::data(index, role);
}

}
}
