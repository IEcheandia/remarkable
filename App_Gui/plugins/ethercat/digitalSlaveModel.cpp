#include "digitalSlaveModel.h"

#include <bitset>

namespace precitec
{
namespace gui
{
namespace components
{
namespace ethercat
{

DigitalSlaveModel::DigitalSlaveModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(this, &DigitalSlaveModel::byteDataChanged, this,
        [this]
        {
            emit dataChanged(index(0, 0), index(7, 0), {Qt::DisplayRole});
        }
    );
}

DigitalSlaveModel::~DigitalSlaveModel() = default;

QHash<int, QByteArray> DigitalSlaveModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("bit")}
    };
}

int DigitalSlaveModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return 8;
}

QVariant DigitalSlaveModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (role != Qt::DisplayRole)
    {
        return {};
    }
    if (m_data.isEmpty())
    {
        return false;
    }
    std::bitset<8> mask{};
    mask.set(index.row());
    return bool(m_data.at(0) & mask.to_ulong());
}

void DigitalSlaveModel::setByteData(const QByteArray &data)
{
    if (m_data == data)
    {
        return;
    }
    m_data = data;
    emit byteDataChanged();
}

}
}
}
}
