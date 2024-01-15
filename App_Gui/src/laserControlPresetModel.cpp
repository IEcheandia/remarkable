#include "laserControlPresetModel.h"
#include "laserControlPreset.h"

using precitec::storage::LaserControlPreset;

namespace precitec
{
namespace gui
{

LaserControlPresetModel::LaserControlPresetModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

LaserControlPresetModel::~LaserControlPresetModel() = default;

QHash<int, QByteArray> LaserControlPresetModel::roleNames() const
{
    return {
        {Qt::UserRole, QByteArrayLiteral("power")},
        {Qt::UserRole + 1, QByteArrayLiteral("offset")}
    };
}

QVariant LaserControlPresetModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !m_preset)
    {
        return {};
    }

    if (role == Qt::UserRole)
    {
        return m_preset->power(index.row() + (m_displayChannel == Channel::Two ? 4 : 0));
    }
    if (role == Qt::UserRole + 1)
    {
        return m_preset->offset(index.row() + (m_displayChannel == Channel::Two ? 4 : 0));
    }

    return {};
}

int LaserControlPresetModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() || !m_preset)
    {
        return 0;
    }
    return m_displayChannel == Channel::Both ? 8 : 4;
}

bool LaserControlPresetModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || !m_preset || index.row() >= 8)
    {
        return false;
    }

    if (role == Qt::UserRole)
    {
        m_preset->setPower(index.row() + (m_displayChannel == Channel::Two ? 4 : 0), value.toInt());
        return true;
    }

    if (role == Qt::UserRole + 1)
    {
        m_preset->setOffset(index.row() + (m_displayChannel == Channel::Two ? 4 : 0), value.toInt());
        return true;
    }

    return false;
}

Qt::ItemFlags LaserControlPresetModel::flags(const QModelIndex& index) const
{
    Q_UNUSED(index)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void LaserControlPresetModel::setPreset(LaserControlPreset* preset)
{
    if (m_preset == preset)
    {
        return;
    }

    beginResetModel();
    m_preset = preset;

    disconnect(m_destroyedConnection);
    disconnect(m_powerConnection);
    disconnect(m_offsetConnection);

    if (m_preset)
    {
        m_destroyedConnection = connect(m_preset, &LaserControlPreset::destroyed, this, std::bind(&LaserControlPresetModel::setPreset, this, nullptr));
        m_powerConnection = connect(m_preset, &LaserControlPreset::powerChanged, this, [this] (int idx) {
            const auto i = index(idx - (m_displayChannel == Channel::Two ? 4 : 0));
            emit dataChanged(i, i, {Qt::UserRole});
        });
        m_offsetConnection = connect(m_preset, &LaserControlPreset::offsetChanged, this, [this] (int idx) {
            const auto i = index(idx - (m_displayChannel == Channel::Two ? 4 : 0));
            emit dataChanged(i, i, {Qt::UserRole + 1});
        });
    } else
    {
        m_destroyedConnection = {};
        m_powerConnection = {};
        m_offsetConnection = {};
    }
    endResetModel();
    emit presetChanged();
}

void LaserControlPresetModel::setDisplayChannel(Channel displayChannel)
{
    if (m_displayChannel == displayChannel)
    {
        return;
    }
    beginResetModel();
    m_displayChannel = displayChannel;
    endResetModel();
    emit displayChannelChanged();
}

}
}
