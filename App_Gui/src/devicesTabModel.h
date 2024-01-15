#pragma once
#include "securityTabModel.h"

namespace precitec
{
namespace gui
{

/**
 * Model exposing the names of the devices (as Qt::DisplayRole)
 * and whether the current user is allowed to access the tab (as Qt::UserRole).
 * In addition there is a "canEdit" role (Qt::UserRole+1) which provides information
 * whether the current user has permission to edit the device.
 **/
class DevicesTabModel : public SecurityTabModel
{
    Q_OBJECT
public:
    explicit DevicesTabModel(QObject *parent = nullptr);
    ~DevicesTabModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    std::vector<int> m_editRoles;
    std::vector<bool> m_available;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::DevicesTabModel*)
