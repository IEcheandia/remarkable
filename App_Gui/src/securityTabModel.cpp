#include "securityTabModel.h"
#include "permissions.h"

#include <precitec/userManagement.h>

using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

SecurityTabModel::SecurityTabModel(std::initializer_list<Data> init, QObject *parent)
    : QAbstractListModel(parent)
    , m_elements(init)
{
    connect(UserManagement::instance(), &UserManagement::currentUserChanged, this,
        [this] {
            emit dataChanged(index(0, 0), index(m_elements.size() - 1, 0), {Qt::UserRole});
        }
    );
}

SecurityTabModel::~SecurityTabModel() = default;

int SecurityTabModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_elements.size();
}

QVariant SecurityTabModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    const auto &element = m_elements.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return element.name;
    case Qt::UserRole:
    {
        const auto &permissions = element.permissions;
        if (permissions.empty())
        {
            return true;
        }
        for (int permission : permissions)
        {
            if (UserManagement::instance()->hasPermission(permission))
            {
                return true;
            }
        }
        return false;
    }
    case Qt::UserRole + 2:
        return element.available;
    case Qt::DecorationRole:
        return element.icon;
    }
    return QVariant();
}

QHash<int, QByteArray> SecurityTabModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
        {Qt::DecorationRole, "icon"}
    };
}

}
}
